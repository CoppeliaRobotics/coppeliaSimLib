
#include "vrepMainHeader.h"
#include "distanceRoutine.h"
#include "pluginContainer.h"
#include "tt.h"
#include "app.h"

bool CDistanceRoutine::_distanceCachingOff=false;
std::vector<SExtCache> CDistanceRoutine::_extendedCacheBuffer;
int CDistanceRoutine::_nextExtendedCacheId=1;
std::vector<SMovementCoherency> CDistanceRoutine::_objectCoherency;

bool CDistanceRoutine::getDistanceCachingEnabled()
{
    return(!_distanceCachingOff);
}

void CDistanceRoutine::setDistanceCachingEnabled(bool e)
{
    _distanceCachingOff=!e;
}

long long int CDistanceRoutine::getExtendedCacheValue(int id)
{
    for (size_t i=0;i<_extendedCacheBuffer.size();i++)
    {
        if (_extendedCacheBuffer[i].id==id)
            return(_extendedCacheBuffer[i].cache);
    }
    return(0);
}

int CDistanceRoutine::insertExtendedCacheValue(long long int value)
{
    if (_extendedCacheBuffer.size()>100)
        _extendedCacheBuffer.pop_back();
    SExtCache c;
    c.id=_nextExtendedCacheId;
    c.cache=value;
    _extendedCacheBuffer.insert(_extendedCacheBuffer.begin(),c);
    return(_nextExtendedCacheId++);
}

bool CDistanceRoutine::getOctreesHaveCoherentMovement(COctree* octree1,COctree* octree2)
{
    bool retVal=false;
    C4X4Matrix m1,m2;
    C3Vector hs1,hs2;
    octree1->getMatrixAndHalfSizeOfBoundingBox(m1,hs1);
    octree2->getMatrixAndHalfSizeOfBoundingBox(m2,hs2);
    C7Vector tr1(m1);
    C7Vector tr2(m2);
    for (size_t i=0;i<_objectCoherency.size();i++)
    {
        if ( (_objectCoherency[i].object1Id==octree1->getObjectHandle())&&(_objectCoherency[i].object2Id==octree2->getObjectHandle()) )
        {
            float s1=0.2*sqrt(pow(hs1(0),2.0)+pow(hs1(1),2.0)+pow(hs1(2),2.0));
            float s2=0.2*sqrt(pow(hs2(0),2.0)+pow(hs2(1),2.0)+pow(hs2(2),2.0));
            if ( ((tr1.X-_objectCoherency[i].object1Tr.X).getLength()<s1)&&((tr2.X-_objectCoherency[i].object2Tr.X).getLength()<s2) )
            { // we have positional coherency
                C4Vector q1(tr1.Q.getInverse()*tr2.Q);
                C4Vector q2(_objectCoherency[i].object1Tr.Q.getInverse()*_objectCoherency[i].object2Tr.Q);
                retVal=q1.getAngleBetweenQuaternions(q2)<20.0f*piValue_f/180.0f; // this is angular coherency
            }
            _objectCoherency.erase(_objectCoherency.begin()+i);
            break;
        }
    }
    SMovementCoherency co;
    co.object1Id=octree1->getObjectHandle();
    co.object2Id=octree2->getObjectHandle();
    co.object1Tr=tr1;
    co.object2Tr=tr2;
    _objectCoherency.push_back(co);
    return(retVal);
}


//---------------------------- GENERAL DISTANCE QUERIES ---------------------------
bool CDistanceRoutine::getDistanceBetweenEntitiesIfSmaller(int entity1ID,int entity2ID,float& dist,float ray[7],int cache1[2],int cache2[2],bool overrideMeasurableFlagIfNonCollection1,bool overrideMeasurableFlagIfNonCollection2)
{ // entity2ID can be -1, in which case all objects are tested against entity1
    // cache1/2 is: obj1Id,obj1TriangleId

    bool returnValue=false;
    // We first check if objects are valid:
    C3DObject* object1=App::ct->objCont->getObjectFromHandle(entity1ID);
    C3DObject* object2=App::ct->objCont->getObjectFromHandle(entity2ID);
    CRegCollection* collection1=nullptr;
    CRegCollection* collection2=nullptr;
    if (object1==nullptr)
    {
        collection1=App::ct->collections->getCollection(entity1ID);
        if (collection1==nullptr)
            return(false);
    }
    if (object2==nullptr)
    {
        collection2=App::ct->collections->getCollection(entity2ID);
        if ( (collection2==nullptr)&&(entity2ID!=-1) )
            return(false);
    }

    App::ct->calcInfo->distanceCalculationStart();
    if (object1!=nullptr)
    { // We have an object against...
        if (object2!=nullptr)
        { // ...another object

            bool doTheCheck=true;
            if ( (entity1ID==cache1[0])&&(entity2ID==cache2[0]) )
            { // check against cached values:
                bool cachedPairWasProcessed=false;
                returnValue|=_getCachedDistanceIfSmaller(dist,ray,cache1,cache2,overrideMeasurableFlagIfNonCollection1,overrideMeasurableFlagIfNonCollection2,cachedPairWasProcessed);
                doTheCheck=!cachedPairWasProcessed;
            }
            if (doTheCheck)
                returnValue|=_getObjectObjectDistanceIfSmaller(object1,object2,dist,ray,cache1,cache2,overrideMeasurableFlagIfNonCollection1,overrideMeasurableFlagIfNonCollection2);
        }
        else
        { // an objects VS a collection or all other objects
            std::vector<C3DObject*> group;
            if (entity2ID==-1)
            { // Special group here (all objects except the object):
                std::vector<C3DObject*> exception;
                exception.push_back(object1);
                App::ct->objCont->getAllMeasurableObjectsFromSceneExcept(&exception,group);
            }
            else
            { // Regular group here:
                App::ct->collections->getMeasurableObjectsFromCollection(entity2ID,group);
            }
            if (group.size()!=0)
            {
                std::vector<C3DObject*> pairs;
                _generateValidPairsFromObjectGroup(object1,group,pairs);

                if (entity1ID==cache1[0]) // we can only check for the object
                { // check against cached values:
                    bool cachedPairWasProcessed=false;
                    returnValue|=_getCachedDistanceIfSmaller_pairs(pairs,dist,ray,cache1,cache2,overrideMeasurableFlagIfNonCollection1,overrideMeasurableFlagIfNonCollection2,cachedPairWasProcessed);
                }
                returnValue|=_getObjectPairsDistanceIfSmaller(pairs,dist,ray,cache1,cache2,overrideMeasurableFlagIfNonCollection1,true);
            }
        }
    }
    else
    { // We have a group against...
        std::vector<C3DObject*> group1;
        App::ct->collections->getMeasurableObjectsFromCollection(entity1ID,group1);
        if (group1.size()!=0)
        {
            if (object2!=nullptr)
            { //...an object
                std::vector<C3DObject*> pairs;
                _generateValidPairsFromGroupObject(group1,object2,pairs);

                if (entity2ID==cache2[0]) // we can only check for the object
                { // check against cached values:
                    bool cachedPairWasProcessed=false;
                    returnValue|=_getCachedDistanceIfSmaller_pairs(pairs,dist,ray,cache1,cache2,overrideMeasurableFlagIfNonCollection1,overrideMeasurableFlagIfNonCollection2,cachedPairWasProcessed);
                }
                returnValue|=_getObjectPairsDistanceIfSmaller(pairs,dist,ray,cache1,cache2,true,overrideMeasurableFlagIfNonCollection2);
            }
            else
            { // ...another group or all other objects (entity2ID could be -1)
                std::vector<C3DObject*> group2;
                if (entity2ID==-1)
                { // Special group here
                    App::ct->objCont->getAllMeasurableObjectsFromSceneExcept(&group1,group2);
                }
                else
                { // Regular group here:
                    App::ct->collections->getMeasurableObjectsFromCollection(entity2ID,group2);
                }
                if (group2.size()!=0)
                {
                    std::vector<C3DObject*> pairs;
                    _generateValidPairsFromGroupGroup(group1,group2,pairs,entity1ID==entity2ID);

                    bool cachedPairWasProcessed=false;
                    returnValue|=_getCachedDistanceIfSmaller_pairs(pairs,dist,ray,cache1,cache2,overrideMeasurableFlagIfNonCollection1,overrideMeasurableFlagIfNonCollection2,cachedPairWasProcessed);
                    returnValue|=_getObjectPairsDistanceIfSmaller(pairs,dist,ray,cache1,cache2,true,true);
                }
            }
        }
    }
    App::ct->calcInfo->distanceCalculationEnd();
    return(returnValue);
}

float CDistanceRoutine::_getApproxBoundingBoxDistance(C3DObject* obj1,C3DObject* obj2)
{ // the returned distance is always same or smaller than the real distance!
    bool isPt[2]={false,false};
    C3DObject* objs[2]={obj1,obj2};
    C3Vector halfSizes[2];
    C4X4Matrix m[2];
    for (size_t cnt=0;cnt<2;cnt++)
    {
        C3DObject* obj=objs[cnt];
        if (obj->getObjectType()==sim_object_shape_type)
        {
            halfSizes[cnt]=((CShape*)obj)->geomData->getBoundingBoxHalfSizes();
            m[cnt]=obj->getCumulativeTransformation().getMatrix();
        }
        if (obj->getObjectType()==sim_object_dummy_type)
        {
            isPt[cnt]=true;
            m[cnt]=obj->getCumulativeTransformation().getMatrix();
        }
        if (obj->getObjectType()==sim_object_octree_type)
            ((COctree*)obj)->getMatrixAndHalfSizeOfBoundingBox(m[cnt],halfSizes[cnt]);
        if (obj->getObjectType()==sim_object_pointcloud_type)
            ((CPointCloud*)obj)->getMatrixAndHalfSizeOfBoundingBox(m[cnt],halfSizes[cnt]);
    }
    if (isPt[0])
    { // pt vs ...
        if (isPt[1])
            return((m[0].X-m[1].X).getLength()); // pt vs pt
        else
            return(CPluginContainer::mesh_getBoxPointDistance(m[1],halfSizes[1],m[0].X)); // pt vs box
    }
    else
    { // box vs ...
        if (isPt[1])
            return(CPluginContainer::mesh_getBoxPointDistance(m[0],halfSizes[0],m[1].X)); // box vs pt
        else
        {
// This is apparently too slow!         return(CPluginContainer::mesh_getBoxBoxDistance(m[0],halfSizes[0],m[1],halfSizes[1])); // box vs box
            return(CPluginContainer::mesh_getApproximateBoxBoxDistance(m[0],halfSizes[0],m[1],halfSizes[1])); // box vs box
        }
    }
}

void CDistanceRoutine::_copyInvertedRay(float originRay[7],float destinationRay[7])
{
    destinationRay[0]=originRay[3];
    destinationRay[1]=originRay[4];
    destinationRay[2]=originRay[5];
    destinationRay[3]=originRay[0];
    destinationRay[4]=originRay[1];
    destinationRay[5]=originRay[2];
    destinationRay[6]=originRay[6];
}

bool CDistanceRoutine::_getDummyDummyDistanceIfSmaller(CDummy* dummy1,CDummy* dummy2,float& dist,float ray[7],int cache1[2],int cache2[2],bool overrideMeasurableFlagDummy1,bool overrideMeasurableFlagDummy2)
{   // If the distance is smaller than 'dist', 'dist' is replaced and the return value is true
    // If the distance is bigger, 'dist' doesn't change and the return value is false
    // ray contains the point on object1 (0-2), the point on object2 (3-5) and the distance (6)

    if (dummy1==dummy2)
        return(false);
    if ( ( (dummy1->getCumulativeObjectSpecialProperty()&sim_objectspecialproperty_measurable)==0 )&&(!overrideMeasurableFlagDummy1) )
        return(false);
    if ( ( (dummy2->getCumulativeObjectSpecialProperty()&sim_objectspecialproperty_measurable)==0 )&&(!overrideMeasurableFlagDummy2) )
        return(false);

    C3Vector v0(dummy1->getCumulativeTransformationPart1().X);
    C3Vector v1(dummy2->getCumulativeTransformationPart1().X);
    float newDist=(v0-v1).getLength();
    if (newDist<dist)
    {
        ray[0]=v0(0);
        ray[1]=v0(1);
        ray[2]=v0(2);
        ray[3]=v1(0);
        ray[4]=v1(1);
        ray[5]=v1(2);
        ray[6]=newDist;
        dist=newDist;
        cache1[0]=dummy1->getObjectHandle();
        cache1[1]=-1;
        cache2[0]=dummy2->getObjectHandle();
        cache2[1]=-1;
        return(true);
    }
    return(false);
}

bool CDistanceRoutine::_getShapeDummyDistanceIfSmaller(CShape* shape,CDummy* dummy,float& dist,float ray[7],int cache1[2],int cache2[2],bool overrideMeasurableFlagShape,bool overrideMeasurableFlagDummy)
{   // If the distance is smaller than 'dist', 'dist' is replaced and the return value is true
    // If the distance is bigger, 'dist' doesn't change and the return value is false
    // ray contains the point on object1 (0-2), the point on object2 (3-5) and the distance (6)

    if ( ( (shape->getCumulativeObjectSpecialProperty()&sim_objectspecialproperty_measurable)==0 )&&(!overrideMeasurableFlagShape) )
        return(false);
    if ( ( (dummy->getCumulativeObjectSpecialProperty()&sim_objectspecialproperty_measurable)==0 )&&(!overrideMeasurableFlagDummy) )
        return(false);

    // Do a preliminary test:
    float bbDist=_getApproxBoundingBoxDistance(shape,dummy);
    if (bbDist>=dist)
        return(false);

    shape->initializeCalculationStructureIfNeeded();

    C3Vector dummyPos(dummy->getCumulativeTransformation().X);
    C4X4Matrix shapePCTM(shape->getCumulativeTransformation());
    C3Vector rayPart0;
    C3Vector rayPart1;
    int buffer=0;
    if (CPluginContainer::mesh_getDistanceAgainstDummy_ifSmaller(shape->geomData->collInfo,dummyPos,shapePCTM,dist,rayPart0,rayPart1,buffer))
    {
        rayPart0.copyTo(ray);
        rayPart1.copyTo(ray+3);
        ray[6]=dist;
        cache1[0]=shape->getObjectHandle();
        cache1[1]=buffer;
        cache2[0]=dummy->getObjectHandle();
        cache2[1]=-1;
        return(true);
    }
    return(false);
}

bool CDistanceRoutine::_getDummyShapeDistanceIfSmaller(CDummy* dummy,CShape* shape,float& dist,float ray[7],int cache1[2],int cache2[2],bool overrideMeasurableFlagDummy,bool overrideMeasurableFlagShape)
{   // If the distance is smaller than 'dist', 'dist' is replaced and the return value is true
    // If the distance is bigger, 'dist' doesn't change and the return value is false
    // ray contains the point on object1 (0-2), the point on object2 (3-5) and the distance (6)

    float _ray[7];
    if (_getShapeDummyDistanceIfSmaller(shape,dummy,dist,_ray,cache2,cache1,overrideMeasurableFlagShape,overrideMeasurableFlagDummy))
    {
        _copyInvertedRay(_ray,ray);
        return(true);
    }
    return(false);
}

bool CDistanceRoutine::_getShapeShapeDistanceIfSmaller(CShape* shape1,CShape* shape2,float& dist,float ray[7],int cache1[2],int cache2[2],bool overrideMeasurableFlagShape1,bool overrideMeasurableFlagShape2)
{   // If the distance is smaller than 'dist', 'dist' is replaced and the return value is true
    // If the distance is bigger, 'dist' doesn't change and the return value is false
    // ray contains the point on object1 (0-2), the point on object2 (3-5) and the distance (6)

    if (shape1==shape2)
        return(false);
    if ( ( (shape1->getCumulativeObjectSpecialProperty()&sim_objectspecialproperty_measurable)==0 )&&(!overrideMeasurableFlagShape1) )
        return(false);
    if ( ( (shape2->getCumulativeObjectSpecialProperty()&sim_objectspecialproperty_measurable)==0 )&&(!overrideMeasurableFlagShape2) )
        return(false);

    // Do a preliminary test:
    float bbDist=_getApproxBoundingBoxDistance(shape1,shape2);
    if (bbDist>=dist)
        return(false);

    shape1->initializeCalculationStructureIfNeeded();
    shape2->initializeCalculationStructureIfNeeded();

    C4X4Matrix shape1PCTM(shape1->getCumulativeTransformation());
    C4X4Matrix shape2PCTM(shape2->getCumulativeTransformation());

    float distances[7];
    distances[6]=dist;
    C4X4Matrix distObjMatr[2]={shape1PCTM,shape2PCTM};
    const void* collInfos[2]={shape1->geomData->collInfo,shape2->geomData->collInfo};
    int buffer[2];
    CPluginContainer::mesh_getMeshMeshDistance(shape1->geomData->collInfo,shape2->geomData->collInfo,distObjMatr,collInfos,false,distances,buffer);
    if (distances[6]<dist)
    {
        dist=distances[6];
        ray[0]=distances[0];
        ray[1]=distances[1];
        ray[2]=distances[2];
        ray[3]=distances[3];
        ray[4]=distances[4];
        ray[5]=distances[5];
        ray[6]=distances[6];
        cache1[0]=shape1->getObjectHandle();
        cache1[1]=buffer[0];
        cache2[0]=shape2->getObjectHandle();
        cache2[1]=buffer[1];
        return(true);
    }
    return(false);
}

bool CDistanceRoutine::_getOctreeDummyDistanceIfSmaller(COctree* octree,CDummy* dummy,float& dist,float ray[7],int cache1[2],int cache2[2],bool overrideMeasurableFlagOctree,bool overrideMeasurableFlagDummy)
{   // If the distance is smaller than 'dist', 'dist' is replaced and the return value is true
    // If the distance is bigger, 'dist' doesn't change and the return value is false
    // ray contains the point on object1 (0-2), the point on object2 (3-5) and the distance (6)

    if (octree->getOctreeInfo()==nullptr)
        return(false); // Octree is empty
    if ( ( (octree->getCumulativeObjectSpecialProperty()&sim_objectspecialproperty_measurable)==0 )&&(!overrideMeasurableFlagOctree) )
        return(false);
    if ( ( (dummy->getCumulativeObjectSpecialProperty()&sim_objectspecialproperty_measurable)==0 )&&(!overrideMeasurableFlagDummy) )
        return(false);

    // Do a preliminary test:
    float bbDist=_getApproxBoundingBoxDistance(octree,dummy);
    if (bbDist>=dist)
        return(false);

    long long int cacheV=getExtendedCacheValue(cache1[1]);
    float _ray[7];
    _ray[6]=dist;
    if (CPluginContainer::mesh_getOctreeDistanceToPointIfSmaller(octree->getOctreeInfo(),octree->getCumulativeTransformation().getMatrix(),dummy->getCumulativeTransformation().X,_ray,cacheV))
    {
        ray[0]=_ray[0];
        ray[1]=_ray[1];
        ray[2]=_ray[2];
        ray[3]=_ray[3];
        ray[4]=_ray[4];
        ray[5]=_ray[5];
        ray[6]=_ray[6];
        dist=ray[6];
        cache1[0]=octree->getObjectHandle();
        cache1[1]=insertExtendedCacheValue(cacheV);
        cache2[0]=dummy->getObjectHandle();
        cache2[1]=-1;
        return(true);
    }

    return(false);
}

bool CDistanceRoutine::_getDummyOctreeDistanceIfSmaller(CDummy* dummy,COctree* octree,float& dist,float ray[7],int cache1[2],int cache2[2],bool overrideMeasurableFlagDummy,bool overrideMeasurableFlagOctree)
{   // If the distance is smaller than 'dist', 'dist' is replaced and the return value is true
    // If the distance is bigger, 'dist' doesn't change and the return value is false
    // ray contains the point on object1 (0-2), the point on object2 (3-5) and the distance (6)

    float _ray[7];
    if (_getOctreeDummyDistanceIfSmaller(octree,dummy,dist,_ray,cache2,cache1,overrideMeasurableFlagOctree,overrideMeasurableFlagDummy))
    {
        _copyInvertedRay(_ray,ray);
        return(true);
    }
    return(false);
}

bool CDistanceRoutine::_getOctreeShapeDistanceIfSmaller(COctree* octree,CShape* shape,float& dist,float ray[7],int cache1[2],int cache2[2],bool overrideMeasurableFlagOctree,bool overrideMeasurableFlagShape)
{   // If the distance is smaller than 'dist', 'dist' is replaced and the return value is true
    // If the distance is bigger, 'dist' doesn't change and the return value is false
    // ray contains the point on object1 (0-2), the point on object2 (3-5) and the distance (6)

    if (octree->getOctreeInfo()==nullptr)
        return(false); // Octree is empty
    if ( ( (octree->getCumulativeObjectSpecialProperty()&sim_objectspecialproperty_measurable)==0 )&&(!overrideMeasurableFlagOctree) )
        return(false);
    if ( ( (shape->getCumulativeObjectSpecialProperty()&sim_objectspecialproperty_measurable)==0 )&&(!overrideMeasurableFlagShape) )
        return(false);

    // Do a preliminary test:
    float bbDist=_getApproxBoundingBoxDistance(octree,shape);
    if (bbDist>=dist)
        return(false);

    shape->initializeCalculationStructureIfNeeded();

    long long int cache1V=getExtendedCacheValue(cache1[1]);
    float _ray[7];
    _ray[6]=dist;
    if (CPluginContainer::mesh_getOctreeDistanceToShapeIfSmaller(octree->getOctreeInfo(),shape->geomData->collInfo,octree->getCumulativeTransformation().getMatrix(),shape->getCumulativeTransformation().getMatrix(),_ray,cache1V,cache2[1]))
    {
        ray[0]=_ray[0];
        ray[1]=_ray[1];
        ray[2]=_ray[2];
        ray[3]=_ray[3];
        ray[4]=_ray[4];
        ray[5]=_ray[5];
        ray[6]=_ray[6];
        dist=ray[6];
        cache1[0]=octree->getObjectHandle();
        cache1[1]=insertExtendedCacheValue(cache1V);
        cache2[0]=shape->getObjectHandle();
        return(true);
    }
    return(false);
}

bool CDistanceRoutine::_getShapeOctreeDistanceIfSmaller(CShape* shape,COctree* octree,float& dist,float ray[7],int cache1[2],int cache2[2],bool overrideMeasurableFlagShape,bool overrideMeasurableFlagOctree)
{   // If the distance is smaller than 'dist', 'dist' is replaced and the return value is true
    // If the distance is bigger, 'dist' doesn't change and the return value is false
    // ray contains the point on object1 (0-2), the point on object2 (3-5) and the distance (6)

    float _ray[7];
    if (_getOctreeShapeDistanceIfSmaller(octree,shape,dist,_ray,cache2,cache1,overrideMeasurableFlagOctree,overrideMeasurableFlagShape))
    {
        _copyInvertedRay(_ray,ray);
        return(true);
    }
    return(false);
}

bool CDistanceRoutine::_getOctreeOctreeDistanceIfSmaller(COctree* octree1,COctree* octree2,float& dist,float ray[7],int cache1[2],int cache2[2],bool overrideMeasurableFlagOctree1,bool overrideMeasurableFlagOctree2)
{   // If the distance is smaller than 'dist', 'dist' is replaced and the return value is true
    // If the distance is bigger, 'dist' doesn't change and the return value is false
    // ray contains the point on object1 (0-2), the point on object2 (3-5) and the distance (6)

    if (octree1==octree2)
        return(false);
    if (octree1->getOctreeInfo()==nullptr)
        return(false); // Octree is empty
    if (octree2->getOctreeInfo()==nullptr)
        return(false); // Octree is empty
    if ( ( (octree1->getCumulativeObjectSpecialProperty()&sim_objectspecialproperty_measurable)==0 )&&(!overrideMeasurableFlagOctree1) )
        return(false);
    if ( ( (octree2->getCumulativeObjectSpecialProperty()&sim_objectspecialproperty_measurable)==0 )&&(!overrideMeasurableFlagOctree2) )
        return(false);

    // Do a preliminary test:
    float bbDist=_getApproxBoundingBoxDistance(octree1,octree2);
    if (bbDist>=dist)
        return(false);

    long long int cache1V=getExtendedCacheValue(cache1[1]);
    long long int cache2V=getExtendedCacheValue(cache2[1]);
    if (_distanceCachingOff)
    { // we do this here since caching is handled directly in mesh_getOctreeDistanceToOctreeIfSmaller
        cache1V=0;
        cache2V=0;
    }
    bool hasCoherency=getOctreesHaveCoherentMovement(octree1,octree2);
    float _ray[7];
    _ray[6]=dist;
    if (CPluginContainer::mesh_getOctreeDistanceToOctreeIfSmaller(octree1->getOctreeInfo(),octree2->getOctreeInfo(),octree1->getCumulativeTransformation().getMatrix(),octree2->getCumulativeTransformation().getMatrix(),_ray,cache1V,cache2V,hasCoherency))
    {
        ray[0]=_ray[0];
        ray[1]=_ray[1];
        ray[2]=_ray[2];
        ray[3]=_ray[3];
        ray[4]=_ray[4];
        ray[5]=_ray[5];
        ray[6]=_ray[6];
        dist=ray[6];
        cache1[0]=octree1->getObjectHandle();
        cache1[1]=insertExtendedCacheValue(cache1V);
        cache2[0]=octree2->getObjectHandle();
        cache2[1]=insertExtendedCacheValue(cache2V);
        return(true);
    }

    return(false);
}

bool CDistanceRoutine::_getPointCloudDummyDistanceIfSmaller(CPointCloud* pointCloud,CDummy* dummy,float& dist,float ray[7],int cache1[2],int cache2[2],bool overrideMeasurableFlagPointCloud,bool overrideMeasurableFlagDummy)
{   // If the distance is smaller than 'dist', 'dist' is replaced and the return value is true
    // If the distance is bigger, 'dist' doesn't change and the return value is false
    // ray contains the point on object1 (0-2), the point on object2 (3-5) and the distance (6)

    if (pointCloud->getPointCloudInfo()==nullptr)
        return(false); // PointCloud is empty
    if ( ( (pointCloud->getCumulativeObjectSpecialProperty()&sim_objectspecialproperty_measurable)==0 )&&(!overrideMeasurableFlagPointCloud) )
        return(false);
    if ( ( (dummy->getCumulativeObjectSpecialProperty()&sim_objectspecialproperty_measurable)==0 )&&(!overrideMeasurableFlagDummy) )
        return(false);

    // Do a preliminary test:
    float bbDist=_getApproxBoundingBoxDistance(pointCloud,dummy);
    if (bbDist>=dist)
        return(false);

    long long int cacheV=getExtendedCacheValue(cache1[1]);
    float _ray[7];
    _ray[6]=dist;
    if (CPluginContainer::mesh_getPointCloudDistanceToPointIfSmaller(pointCloud->getPointCloudInfo(),pointCloud->getCumulativeTransformation().getMatrix(),dummy->getCumulativeTransformation().X,_ray,cacheV))
    {
        ray[0]=_ray[0];
        ray[1]=_ray[1];
        ray[2]=_ray[2];
        ray[3]=_ray[3];
        ray[4]=_ray[4];
        ray[5]=_ray[5];
        ray[6]=_ray[6];
        dist=ray[6];
        cache1[0]=pointCloud->getObjectHandle();
        cache1[1]=insertExtendedCacheValue(cacheV);
        cache2[0]=dummy->getObjectHandle();
        cache2[1]=-1;
        return(true);
    }

    return(false);
}

bool CDistanceRoutine::_getDummyPointCloudDistanceIfSmaller(CDummy* dummy,CPointCloud* pointCloud,float& dist,float ray[7],int cache1[2],int cache2[2],bool overrideMeasurableFlagDummy,bool overrideMeasurableFlagPointCloud)
{   // If the distance is smaller than 'dist', 'dist' is replaced and the return value is true
    // If the distance is bigger, 'dist' doesn't change and the return value is false
    // ray contains the point on object1 (0-2), the point on object2 (3-5) and the distance (6)

    float _ray[7];
    if (_getPointCloudDummyDistanceIfSmaller(pointCloud,dummy,dist,_ray,cache2,cache1,overrideMeasurableFlagPointCloud,overrideMeasurableFlagDummy))
    {
        _copyInvertedRay(_ray,ray);
        return(true);
    }
    return(false);
}

bool CDistanceRoutine::_getPointCloudShapeDistanceIfSmaller(CPointCloud* pointCloud,CShape* shape,float& dist,float ray[7],int cache1[2],int cache2[2],bool overrideMeasurableFlagPointCloud,bool overrideMeasurableFlagShape)
{   // If the distance is smaller than 'dist', 'dist' is replaced and the return value is true
    // If the distance is bigger, 'dist' doesn't change and the return value is false
    // ray contains the point on object1 (0-2), the point on object2 (3-5) and the distance (6)

    if (pointCloud->getPointCloudInfo()==nullptr)
        return(false); // Point cloud is empty
    if ( ( (pointCloud->getCumulativeObjectSpecialProperty()&sim_objectspecialproperty_measurable)==0 )&&(!overrideMeasurableFlagPointCloud) )
        return(false);
    if ( ( (shape->getCumulativeObjectSpecialProperty()&sim_objectspecialproperty_measurable)==0 )&&(!overrideMeasurableFlagShape) )
        return(false);

    // Do a preliminary test:
    float bbDist=_getApproxBoundingBoxDistance(pointCloud,shape);
    if (bbDist>=dist)
        return(false);

    shape->initializeCalculationStructureIfNeeded();

    long long int cache1V=getExtendedCacheValue(cache1[1]);
    float _ray[7];
    _ray[6]=dist;
    if (CPluginContainer::mesh_getPointCloudDistanceToShapeIfSmaller(pointCloud->getPointCloudInfo(),shape->geomData->collInfo,pointCloud->getCumulativeTransformation().getMatrix(),shape->getCumulativeTransformation().getMatrix(),_ray,cache1V,cache2[1]))
    {
        ray[0]=_ray[0];
        ray[1]=_ray[1];
        ray[2]=_ray[2];
        ray[3]=_ray[3];
        ray[4]=_ray[4];
        ray[5]=_ray[5];
        ray[6]=_ray[6];
        dist=ray[6];
        cache1[0]=pointCloud->getObjectHandle();
        cache1[1]=insertExtendedCacheValue(cache1V);
        cache2[0]=shape->getObjectHandle();
        return(true);
    }
    return(false);
}

bool CDistanceRoutine::_getShapePointCloudDistanceIfSmaller(CShape* shape,CPointCloud* pointCloud,float& dist,float ray[7],int cache1[2],int cache2[2],bool overrideMeasurableFlagShape,bool overrideMeasurableFlagPointCloud)
{   // If the distance is smaller than 'dist', 'dist' is replaced and the return value is true
    // If the distance is bigger, 'dist' doesn't change and the return value is false
    // ray contains the point on object1 (0-2), the point on object2 (3-5) and the distance (6)

    float _ray[7];
    if (_getPointCloudShapeDistanceIfSmaller(pointCloud,shape,dist,_ray,cache2,cache1,overrideMeasurableFlagPointCloud,overrideMeasurableFlagShape))
    {
        _copyInvertedRay(_ray,ray);
        return(true);
    }
    return(false);
}

bool CDistanceRoutine::_getOctreePointCloudDistanceIfSmaller(COctree* octree,CPointCloud* pointCloud,float& dist,float ray[7],int cache1[2],int cache2[2],bool overrideMeasurableFlagOctree,bool overrideMeasurableFlagPointCloud)
{   // If the distance is smaller than 'dist', 'dist' is replaced and the return value is true
    // If the distance is bigger, 'dist' doesn't change and the return value is false
    // ray contains the point on object1 (0-2), the point on object2 (3-5) and the distance (6)

    if (octree->getOctreeInfo()==nullptr)
        return(false); // Octree is empty
    if (pointCloud->getPointCloudInfo()==nullptr)
        return(false); // PointCloud is empty
    if ( ( (octree->getCumulativeObjectSpecialProperty()&sim_objectspecialproperty_measurable)==0 )&&(!overrideMeasurableFlagOctree) )
        return(false);
    if ( ( (pointCloud->getCumulativeObjectSpecialProperty()&sim_objectspecialproperty_measurable)==0 )&&(!overrideMeasurableFlagPointCloud) )
        return(false);

    // Do a preliminary test:
    float bbDist=_getApproxBoundingBoxDistance(octree,pointCloud);
    if (bbDist>=dist)
        return(false);

    long long int cache1V=getExtendedCacheValue(cache1[1]);
    long long int cache2V=getExtendedCacheValue(cache2[1]);

    float _ray[7];
    _ray[6]=dist;
    if (CPluginContainer::mesh_getOctreeDistanceToPointCloudIfSmaller(octree->getOctreeInfo(),pointCloud->getPointCloudInfo(),octree->getCumulativeTransformation().getMatrix(),pointCloud->getCumulativeTransformation().getMatrix(),_ray,cache1V,cache2V))
    {
        ray[0]=_ray[0];
        ray[1]=_ray[1];
        ray[2]=_ray[2];
        ray[3]=_ray[3];
        ray[4]=_ray[4];
        ray[5]=_ray[5];
        ray[6]=_ray[6];
        dist=ray[6];
        cache1[0]=octree->getObjectHandle();
        cache1[1]=insertExtendedCacheValue(cache1V);
        cache2[0]=pointCloud->getObjectHandle();
        cache2[1]=insertExtendedCacheValue(cache2V);
        return(true);
    }

    return(false);
}

bool CDistanceRoutine::_getPointCloudOctreeDistanceIfSmaller(CPointCloud* pointCloud,COctree* octree,float& dist,float ray[7],int cache1[2],int cache2[2],bool overrideMeasurableFlagPointCloud,bool overrideMeasurableFlagOctree)
{   // If the distance is smaller than 'dist', 'dist' is replaced and the return value is true
    // If the distance is bigger, 'dist' doesn't change and the return value is false
    // ray contains the point on object1 (0-2), the point on object2 (3-5) and the distance (6)

    float _ray[7];
    if (_getOctreePointCloudDistanceIfSmaller(octree,pointCloud,dist,_ray,cache2,cache1,overrideMeasurableFlagOctree,overrideMeasurableFlagPointCloud))
    {
        _copyInvertedRay(_ray,ray);
        return(true);
    }
    return(false);
}

bool CDistanceRoutine::_getPointCloudPointCloudDistanceIfSmaller(CPointCloud* pointCloud1,CPointCloud* pointCloud2,float& dist,float ray[7],int cache1[2],int cache2[2],bool overrideMeasurableFlagPointCloud1,bool overrideMeasurableFlagPointCloud2)
{   // If the distance is smaller than 'dist', 'dist' is replaced and the return value is true
    // If the distance is bigger, 'dist' doesn't change and the return value is false
    // ray contains the point on object1 (0-2), the point on object2 (3-5) and the distance (6)

    if (pointCloud1==pointCloud2)
        return(false);
    if (pointCloud1->getPointCloudInfo()==nullptr)
        return(false); // Octree is empty
    if (pointCloud2->getPointCloudInfo()==nullptr)
        return(false); // Octree is empty
    if ( ( (pointCloud1->getCumulativeObjectSpecialProperty()&sim_objectspecialproperty_measurable)==0 )&&(!overrideMeasurableFlagPointCloud1) )
        return(false);
    if ( ( (pointCloud2->getCumulativeObjectSpecialProperty()&sim_objectspecialproperty_measurable)==0 )&&(!overrideMeasurableFlagPointCloud2) )
        return(false);

    // Do a preliminary test:
    float bbDist=_getApproxBoundingBoxDistance(pointCloud1,pointCloud2);
    if (bbDist>=dist)
        return(false);

    long long int cache1V=getExtendedCacheValue(cache1[1]);
    long long int cache2V=getExtendedCacheValue(cache2[1]);
    float _ray[7];
    _ray[6]=dist;
    if (CPluginContainer::mesh_getPointCloudDistanceToPointCloudIfSmaller(pointCloud1->getPointCloudInfo(),pointCloud2->getPointCloudInfo(),pointCloud1->getCumulativeTransformation().getMatrix(),pointCloud2->getCumulativeTransformation().getMatrix(),_ray,cache1V,cache2V))
    {
        ray[0]=_ray[0];
        ray[1]=_ray[1];
        ray[2]=_ray[2];
        ray[3]=_ray[3];
        ray[4]=_ray[4];
        ray[5]=_ray[5];
        ray[6]=_ray[6];
        dist=ray[6];
        cache1[0]=pointCloud1->getObjectHandle();
        cache1[1]=insertExtendedCacheValue(cache1V);
        cache2[0]=pointCloud2->getObjectHandle();
        cache2[1]=insertExtendedCacheValue(cache2V);
        return(true);
    }
    return(false);
}

bool CDistanceRoutine::_getObjectPairsDistanceIfSmaller(const std::vector<C3DObject*>& unorderedPairs,float& dist,float ray[7],int cache1[2],int cache2[2],bool overrideMeasurableFlagObject1,bool overrideMeasurableFlagObject2)
{
    std::vector<C3DObject*> pairs(unorderedPairs);
    float approxDist=_orderPairsAccordingToApproxBoundingBoxDistance(pairs);
    if (approxDist>=dist)
        return(false);
    bool retVal=false;
    for (size_t i=0;i<pairs.size()/2;i++)
        retVal|=_getObjectObjectDistanceIfSmaller(pairs[2*i+0],pairs[2*i+1],dist,ray,cache1,cache2,overrideMeasurableFlagObject1,overrideMeasurableFlagObject2);

    return(retVal);
}

bool CDistanceRoutine::_getObjectObjectDistanceIfSmaller(C3DObject* object1,C3DObject* object2,float& dist,float ray[7],int cache1[2],int cache2[2],bool overrideMeasurableFlagObject1,bool overrideMeasurableFlagObject2)
{
    if (object1->getObjectType()==sim_object_dummy_type)
    {
        if (object2->getObjectType()==sim_object_dummy_type)
            return(_getDummyDummyDistanceIfSmaller((CDummy*)object1,(CDummy*)object2,dist,ray,cache1,cache2,overrideMeasurableFlagObject1,overrideMeasurableFlagObject2));
        if (object2->getObjectType()==sim_object_shape_type)
            return(_getDummyShapeDistanceIfSmaller((CDummy*)object1,(CShape*)object2,dist,ray,cache1,cache2,overrideMeasurableFlagObject1,overrideMeasurableFlagObject2));
        if (object2->getObjectType()==sim_object_octree_type)
            return(_getDummyOctreeDistanceIfSmaller((CDummy*)object1,(COctree*)object2,dist,ray,cache1,cache2,overrideMeasurableFlagObject1,overrideMeasurableFlagObject2));
        if (object2->getObjectType()==sim_object_pointcloud_type)
            return(_getDummyPointCloudDistanceIfSmaller((CDummy*)object1,(CPointCloud*)object2,dist,ray,cache1,cache2,overrideMeasurableFlagObject1,overrideMeasurableFlagObject2));
    }
    if (object1->getObjectType()==sim_object_shape_type)
    {
        if (object2->getObjectType()==sim_object_dummy_type)
            return(_getShapeDummyDistanceIfSmaller((CShape*)object1,(CDummy*)object2,dist,ray,cache1,cache2,overrideMeasurableFlagObject1,overrideMeasurableFlagObject2));
        if (object2->getObjectType()==sim_object_shape_type)
            return(_getShapeShapeDistanceIfSmaller((CShape*)object1,(CShape*)object2,dist,ray,cache1,cache2,overrideMeasurableFlagObject1,overrideMeasurableFlagObject2));
        if (object2->getObjectType()==sim_object_octree_type)
            return(_getShapeOctreeDistanceIfSmaller((CShape*)object1,(COctree*)object2,dist,ray,cache1,cache2,overrideMeasurableFlagObject1,overrideMeasurableFlagObject2));
        if (object2->getObjectType()==sim_object_pointcloud_type)
            return(_getShapePointCloudDistanceIfSmaller((CShape*)object1,(CPointCloud*)object2,dist,ray,cache1,cache2,overrideMeasurableFlagObject1,overrideMeasurableFlagObject2));
    }
    if (object1->getObjectType()==sim_object_octree_type)
    {
        if (object2->getObjectType()==sim_object_dummy_type)
            return(_getOctreeDummyDistanceIfSmaller((COctree*)object1,(CDummy*)object2,dist,ray,cache1,cache2,overrideMeasurableFlagObject1,overrideMeasurableFlagObject2));
        if (object2->getObjectType()==sim_object_shape_type)
            return(_getOctreeShapeDistanceIfSmaller((COctree*)object1,(CShape*)object2,dist,ray,cache1,cache2,overrideMeasurableFlagObject1,overrideMeasurableFlagObject2));
        if (object2->getObjectType()==sim_object_octree_type)
            return(_getOctreeOctreeDistanceIfSmaller((COctree*)object1,(COctree*)object2,dist,ray,cache1,cache2,overrideMeasurableFlagObject1,overrideMeasurableFlagObject2));
        if (object2->getObjectType()==sim_object_pointcloud_type)
            return(_getOctreePointCloudDistanceIfSmaller((COctree*)object1,(CPointCloud*)object2,dist,ray,cache1,cache2,overrideMeasurableFlagObject1,overrideMeasurableFlagObject2));
    }
    if (object1->getObjectType()==sim_object_pointcloud_type)
    {
        if (object2->getObjectType()==sim_object_dummy_type)
            return(_getPointCloudDummyDistanceIfSmaller((CPointCloud*)object1,(CDummy*)object2,dist,ray,cache1,cache2,overrideMeasurableFlagObject1,overrideMeasurableFlagObject2));
        if (object2->getObjectType()==sim_object_shape_type)
            return(_getPointCloudShapeDistanceIfSmaller((CPointCloud*)object1,(CShape*)object2,dist,ray,cache1,cache2,overrideMeasurableFlagObject1,overrideMeasurableFlagObject2));
        if (object2->getObjectType()==sim_object_octree_type)
            return(_getPointCloudOctreeDistanceIfSmaller((CPointCloud*)object1,(COctree*)object2,dist,ray,cache1,cache2,overrideMeasurableFlagObject1,overrideMeasurableFlagObject2));
        if (object2->getObjectType()==sim_object_pointcloud_type)
            return(_getPointCloudPointCloudDistanceIfSmaller((CPointCloud*)object1,(CPointCloud*)object2,dist,ray,cache1,cache2,overrideMeasurableFlagObject1,overrideMeasurableFlagObject2));
    }
    return(false);
}

void CDistanceRoutine::_generateValidPairsFromObjectGroup(C3DObject* obj,const std::vector<C3DObject*>& group,std::vector<C3DObject*>& pairs)
{
    for (size_t i=0;i<group.size();i++)
    {
        C3DObject* obj2=group[i];
        if (obj!=obj2)
        { // We never check an object against itself!
            pairs.push_back(obj);
            pairs.push_back(obj2);
        }
    }
}

void CDistanceRoutine::_generateValidPairsFromGroupObject(const std::vector<C3DObject*>& group,C3DObject* obj,std::vector<C3DObject*>& pairs)
{
    for (size_t i=0;i<group.size();i++)
    {
        C3DObject* obj1=group[i];
        if (obj!=obj1)
        { // We never check an object against itself!
            pairs.push_back(obj1);
            pairs.push_back(obj);
        }
    }
}

void CDistanceRoutine::_generateValidPairsFromGroupGroup(const std::vector<C3DObject*>& group1,const std::vector<C3DObject*>& group2,std::vector<C3DObject*>& pairs,bool collectionSelfDistanceCheck)
{
    for (size_t i=0;i<group1.size();i++)
    {
        C3DObject* obj1=group1[i];
        int csci1=obj1->getCollectionSelfCollisionIndicator();
        for (size_t j=0;j<group2.size();j++)
        {
            C3DObject* obj2=group2[j];
            int csci2=obj2->getCollectionSelfCollisionIndicator();
            if (obj1!=obj2)
            { // We never check an object against itself!
                if ( (abs(csci1-csci2)!=1)||(!collectionSelfDistanceCheck) )
                { // the collection self collision indicators differences is not 1
                    // We now check if these partners are already present in objPairs
                    bool absent=true;
                    for (size_t k=0;k<pairs.size()/2;k++)
                    {
                        if ( (pairs[2*k+0]==obj2)&&(pairs[2*k+1]==obj1) )
                        {
                            absent=false;
                            break;
                        }
                    }
                    if (absent)
                    {
                        pairs.push_back(obj1);
                        pairs.push_back(obj2);
                    }
                }
            }
        }
    }
}

float CDistanceRoutine::_orderPairsAccordingToApproxBoundingBoxDistance(std::vector<C3DObject*>& pairs)
{ // returns the smallest approx box-box distance
    float retVal=0;
    std::vector<float> distances;
    std::vector<int> indexes;
    for (size_t i=0;i<pairs.size()/2;i++)
    {
        indexes.push_back(int(i));
        float d=_getApproxBoundingBoxDistance(pairs[2*i+0],pairs[2*i+1]);
        if ( (i==0)||(d<retVal) )
            retVal=d;
        distances.push_back(d);
    }

    tt::orderAscending(distances,indexes);

    std::vector<C3DObject*> _pairs(pairs);
    for (size_t i=0;i<indexes.size();i++)
    {
        pairs[2*i+0]=_pairs[2*indexes[i]+0];
        pairs[2*i+1]=_pairs[2*indexes[i]+1];
    }

    return(retVal);
}

bool CDistanceRoutine::_getCachedDistanceIfSmaller(float& dist,float ray[7],int cache1[2],int cache2[2],bool overrideMeasurableFlagObject1,bool overrideMeasurableFlagObject2,bool& cachedPairWasProcessed)
{   // If the distance is smaller than 'dist', 'dist' is replaced and return is true
    // If the distance is bigger, 'dist' doesn't change and the return is false
    // ray contains the point on object1 (0-2), the point on object2 (3-5) and the distance (6)
    // cachedPairWasProcessed tells the called whether he still needs to process the incriminated pair
    // In each cache we have: objectID, triangleID (if shape)
    cachedPairWasProcessed=false;
    bool isSmaller=false;
    if (_distanceCachingOff)
        return(isSmaller); // Distance caching is turned off
    if ( (cache1[0]<0)||(cache2[0]<0) )
        return(isSmaller);
    C3DObject* object1=App::ct->objCont->getObjectFromHandle(cache1[0]);
    if (object1==nullptr)
        return(isSmaller);
    C3DObject* object2=App::ct->objCont->getObjectFromHandle(cache2[0]);
    if (object2==nullptr)
        return(isSmaller);

    if (object1->getObjectType()==sim_object_shape_type)
    { // object1 is a shape
        if (object2->getObjectType()==sim_object_shape_type)
        { // object2 is a shape
            isSmaller=_getCachedDistanceShapeShapeIfSmaller((CShape*)object1,(CShape*)object2,dist,ray,cache1,cache2,overrideMeasurableFlagObject1,overrideMeasurableFlagObject2,cachedPairWasProcessed);
        }
        else if (object2->getObjectType()==sim_object_dummy_type)
        { // object2 is a dummy. We have shape-dummy
            isSmaller=_getCachedDistanceShapeDummyIfSmaller((CShape*)object1,(CDummy*)object2,dist,ray,cache1,cache2,overrideMeasurableFlagObject1,overrideMeasurableFlagObject2,cachedPairWasProcessed);
        }
        else if (object2->getObjectType()==sim_object_octree_type)
        { // object2 is an octree. We have shape-octree
            isSmaller=_getCachedDistanceShapeOctreeIfSmaller((CShape*)object1,(COctree*)object2,dist,ray,cache1,cache2,overrideMeasurableFlagObject1,overrideMeasurableFlagObject2,cachedPairWasProcessed);
        }
        else if (object2->getObjectType()==sim_object_pointcloud_type)
        { // object2 is a point cloud. We have shape-point cloud
            isSmaller=_getCachedDistanceShapePointCloudIfSmaller((CShape*)object1,(CPointCloud*)object2,dist,ray,cache1,cache2,overrideMeasurableFlagObject1,overrideMeasurableFlagObject2,cachedPairWasProcessed);
        }
    }
    else if (object1->getObjectType()==sim_object_dummy_type)
    { // object1 is a dummy
        if (object2->getObjectType()==sim_object_shape_type)
        { // object2 is a shape. We have dummy-shape
            isSmaller=_getCachedDistanceDummyShapeIfSmaller((CDummy*)object1,(CShape*)object2,dist,ray,cache1,cache2,overrideMeasurableFlagObject1,overrideMeasurableFlagObject2,cachedPairWasProcessed);
        }
        else if (object2->getObjectType()==sim_object_pointcloud_type)
        { // object2 is a pointCloud
            isSmaller=_getCachedDistanceDummyPointCloudIfSmaller((CDummy*)object1,(CPointCloud*)object2,dist,ray,cache1,cache2,overrideMeasurableFlagObject1,overrideMeasurableFlagObject2,cachedPairWasProcessed);
        }
        else if (object2->getObjectType()==sim_object_octree_type)
        { // object2 is an octree
            isSmaller=_getCachedDistanceDummyOctreeIfSmaller((CDummy*)object1,(COctree*)object2,dist,ray,cache1,cache2,overrideMeasurableFlagObject1,overrideMeasurableFlagObject2,cachedPairWasProcessed);
        }
    }
    else if (object1->getObjectType()==sim_object_pointcloud_type)
    { // object1 is a point cloud
        if (object2->getObjectType()==sim_object_pointcloud_type)
        { // object2 is a point cloud
            isSmaller=_getCachedDistancePointCloudPointCloudIfSmaller((CPointCloud*)object1,(CPointCloud*)object2,dist,ray,cache1,cache2,overrideMeasurableFlagObject1,overrideMeasurableFlagObject2,cachedPairWasProcessed);
        }
        else if (object2->getObjectType()==sim_object_dummy_type)
        { // object2 is a dummy
            isSmaller=_getCachedDistancePointCloudDummyIfSmaller((CPointCloud*)object1,(CDummy*)object2,dist,ray,cache1,cache2,overrideMeasurableFlagObject1,overrideMeasurableFlagObject2,cachedPairWasProcessed);
        }
        else if (object2->getObjectType()==sim_object_octree_type)
        { // object2 is an octree
            isSmaller=_getCachedDistancePointCloudOctreeIfSmaller((CPointCloud*)object1,(COctree*)object2,dist,ray,cache1,cache2,overrideMeasurableFlagObject1,overrideMeasurableFlagObject2,cachedPairWasProcessed);
        }
        else if (object2->getObjectType()==sim_object_shape_type)
        { // object2 is a shape. We have point cloud-shape
            isSmaller=_getCachedDistancePointCloudShapeIfSmaller((CPointCloud*)object1,(CShape*)object2,dist,ray,cache1,cache2,overrideMeasurableFlagObject1,overrideMeasurableFlagObject2,cachedPairWasProcessed);
        }
    }
    else if (object1->getObjectType()==sim_object_octree_type)
    { // object1 is an octree
        if (object2->getObjectType()==sim_object_dummy_type)
        { // object2 is a dummy
            isSmaller=_getCachedDistanceOctreeDummyIfSmaller((COctree*)object1,(CDummy*)object2,dist,ray,cache1,cache2,overrideMeasurableFlagObject1,overrideMeasurableFlagObject2,cachedPairWasProcessed);
        }
        else if (object2->getObjectType()==sim_object_pointcloud_type)
        { // object2 is a pointCloud
            isSmaller=_getCachedDistanceOctreePointCloudIfSmaller((COctree*)object1,(CPointCloud*)object2,dist,ray,cache1,cache2,overrideMeasurableFlagObject1,overrideMeasurableFlagObject2,cachedPairWasProcessed);
        }
        else if (object2->getObjectType()==sim_object_shape_type)
        { // object2 is an octree. We have octree-shape
            isSmaller=_getCachedDistanceOctreeShapeIfSmaller((COctree*)object1,(CShape*)object2,dist,ray,cache1,cache2,overrideMeasurableFlagObject1,overrideMeasurableFlagObject2,cachedPairWasProcessed);
        }
    }
    if (!cachedPairWasProcessed)
    {
        isSmaller|=_getObjectObjectDistanceIfSmaller(object1,object2,dist,ray,cache1,cache2,overrideMeasurableFlagObject1,overrideMeasurableFlagObject2);
        cachedPairWasProcessed=true;
    }
    return(isSmaller);
}

bool CDistanceRoutine::_getCachedDistanceIfSmaller_pairs(std::vector<C3DObject*>& unorderedPairsCanBeModified,float& dist,float ray[7],int cache1[2],int cache2[2],bool overrideMeasurableFlagObject1,bool overrideMeasurableFlagObject2,bool& cachedPairWasProcessed)
{   // If the distance is smaller than 'dist', 'dist' is replaced and return is true
    // If the distance is bigger, 'dist' doesn't change and the return is false
    // ray contains the point on object1 (0-2), the point on object2 (3-5) and the distance (6)
    // cachedPairWasProcessed tells the called whether he still needs to process the incriminated pair
    // In each cache we have: objectID, triangleID (if shape)

    // We check if we find the cached ids in the pairs:
    cachedPairWasProcessed=false;
    bool isSmaller=false;
    if (_distanceCachingOff)
        return(isSmaller); // Distance caching is turned off
    for (size_t i=0;i<unorderedPairsCanBeModified.size()/2;i++)
    {
        C3DObject* object1=unorderedPairsCanBeModified[2*i+0];
        C3DObject* object2=unorderedPairsCanBeModified[2*i+1];
        if ( (object1->getObjectHandle()==cache1[0])&&(object2->getObjectHandle()==cache2[0]) )
        { // we have found the pair!
            isSmaller|=_getCachedDistanceIfSmaller(dist,ray,cache1,cache2,overrideMeasurableFlagObject1,overrideMeasurableFlagObject2,cachedPairWasProcessed);
            unorderedPairsCanBeModified.erase(unorderedPairsCanBeModified.begin()+2*i,unorderedPairsCanBeModified.begin()+2*i+2);
            break;
        }
    }
    if (!cachedPairWasProcessed)
        cachedPairWasProcessed=(unorderedPairsCanBeModified.size()==0);
    return(isSmaller);
}

bool CDistanceRoutine::_getCachedDistanceShapeShapeIfSmaller(CShape* shape1,CShape* shape2,float& dist,float ray[7],int cache1[2],int cache2[2],bool overrideMeasurableFlagObject1,bool overrideMeasurableFlagObject2,bool& cachedPairWasProcessed)
{   // If the distance is smaller than 'dist', 'dist' is replaced and return is true
    // If the distance is bigger, 'dist' doesn't change and the return is false
    // ray contains the point on object1 (0-2), the point on object2 (3-5) and the distance (6)
    // cachedPairWasProcessed tells the called whether he still needs to process the incriminated pair
    // In each cache we have: objectID, triangleID (if shape)

    cachedPairWasProcessed=false;
    bool isSmaller=false;

    if (!shape1->isCollisionInformationInitialized())
        return(isSmaller);
    if ((cache1[1]<0)||(CPluginContainer::mesh_getCalculatedTriangleCount(shape1->geomData->collInfo)*3<=(cache1[1]*3+2)))
        return(isSmaller); // that index doesn't exist
    if (!shape2->isCollisionInformationInitialized())
        return(isSmaller);
    if ((cache2[1]<0)||(CPluginContainer::mesh_getCalculatedTriangleCount(shape2->geomData->collInfo)*3<=(cache2[1]*3+2)))
        return(isSmaller); // that index doesn't exist
    C3Vector t1a;
    C3Vector t1b;
    C3Vector t1c;
    if (!CPluginContainer::mesh_getCalculatedTriangleAt(shape1->geomData->collInfo,t1a,t1b,t1c,cache1[1]))
        return(isSmaller); // triangle was disabled
    C3Vector t2a;
    C3Vector t2b;
    C3Vector t2c;
    if (!CPluginContainer::mesh_getCalculatedTriangleAt(shape2->geomData->collInfo,t2a,t2b,t2c,cache2[1]))
        return(isSmaller); // triangle was disabled
    C7Vector b1CTM(shape1->getCumulativeTransformationPart1());
    C7Vector b2CTM(shape2->getCumulativeTransformationPart1());
    t1a*=b1CTM;
    t1b*=b1CTM;
    t1c*=b1CTM;
    t2a*=b2CTM;
    t2b*=b2CTM;
    t2c*=b2CTM;
    t1b-=t1a;
    t1c-=t1a;
    t2b-=t2a;
    t2c-=t2a;
    C3Vector segA;
    C3Vector segB;
    isSmaller|=CPluginContainer::mesh_getTriangleTriangleDistance_ifSmaller(t1a,t1b,t1c,t2a,t2b,t2c,dist,segA,segB);
    if (isSmaller)
    {
        ray[0]=segA(0);
        ray[1]=segA(1);
        ray[2]=segA(2);
        ray[3]=segB(0);
        ray[4]=segB(1);
        ray[5]=segB(2);
        ray[6]=dist;
    }
    isSmaller|=_getShapeShapeDistanceIfSmaller(shape1,shape2,dist,ray,cache1,cache2,overrideMeasurableFlagObject1,overrideMeasurableFlagObject2);
    cachedPairWasProcessed=true;
    return(isSmaller);
}

bool CDistanceRoutine::_getCachedDistanceShapeDummyIfSmaller(CShape* shape,CDummy* dummy,float& dist,float ray[7],int cache1[2],int cache2[2],bool overrideMeasurableFlagObject1,bool overrideMeasurableFlagObject2,bool& cachedPairWasProcessed)
{   // If the distance is smaller than 'dist', 'dist' is replaced and return is true
    // If the distance is bigger, 'dist' doesn't change and the return is false
    // ray contains the point on object1 (0-2), the point on object2 (3-5) and the distance (6)
    // cachedPairWasProcessed tells the called whether he still needs to process the incriminated pair
    // In each cache we have: objectID, triangleID (if shape)

    cachedPairWasProcessed=false;
    bool isSmaller=false;

    if (!shape->isCollisionInformationInitialized())
        return(isSmaller);
    if (CPluginContainer::mesh_getCalculatedTriangleCount(shape->geomData->collInfo)*3<=(cache1[1]*3+2))
        return(isSmaller); // that index doesn't exist
    C3Vector t1a;
    C3Vector t1b;
    C3Vector t1c;
    if (!CPluginContainer::mesh_getCalculatedTriangleAt(shape->geomData->collInfo,t1a,t1b,t1c,cache1[1]))
        return(isSmaller); // triangle was disabled
    C7Vector bCTM(shape->getCumulativeTransformationPart1());
    t1a*=bCTM;
    t1b*=bCTM;
    t1c*=bCTM;
    t1b-=t1a;
    t1c-=t1a;
    C7Vector dCTM(dummy->getCumulativeTransformationPart1());
    C3Vector dummyPos(dCTM.X);
    C3Vector tmpA;
    isSmaller|=CPluginContainer::mesh_getTrianglePointDistance_ifSmaller(t1a,t1b,t1c,dummyPos,dist,tmpA);
    if (isSmaller)
    {
        ray[0]=tmpA(0);
        ray[1]=tmpA(1);
        ray[2]=tmpA(2);
        ray[3]=dummyPos(0);
        ray[4]=dummyPos(1);
        ray[5]=dummyPos(2);
        ray[6]=dist;
    }
    isSmaller|=_getShapeDummyDistanceIfSmaller(shape,dummy,dist,ray,cache1,cache2,overrideMeasurableFlagObject1,overrideMeasurableFlagObject2);
    cachedPairWasProcessed=true;
    return(isSmaller);
}

bool CDistanceRoutine::_getCachedDistanceDummyShapeIfSmaller(CDummy* dummy,CShape* shape,float& dist,float ray[7],int cache1[2],int cache2[2],bool overrideMeasurableFlagObject1,bool overrideMeasurableFlagObject2,bool& cachedPairWasProcessed)
{   // If the distance is smaller than 'dist', 'dist' is replaced and return is true
    // If the distance is bigger, 'dist' doesn't change and the return is false
    // ray contains the point on object1 (0-2), the point on object2 (3-5) and the distance (6)
    // cachedPairWasProcessed tells the called whether he still needs to process the incriminated pair
    // In each cache we have: objectID, triangleID (if shape)

    float _ray[7];
    bool isSmaller=_getCachedDistanceShapeDummyIfSmaller(shape,dummy,dist,_ray,cache2,cache1,overrideMeasurableFlagObject2,overrideMeasurableFlagObject1,cachedPairWasProcessed);
    if (isSmaller)
        _copyInvertedRay(_ray,ray);
    return(isSmaller);
}

bool CDistanceRoutine::_getCachedDistancePointCloudPointCloudIfSmaller(CPointCloud* ptCloud1,CPointCloud* ptCloud2,float& dist,float ray[7],int cache1[2],int cache2[2],bool overrideMeasurableFlagObject1,bool overrideMeasurableFlagObject2,bool& cachedPairWasProcessed)
{   // If the distance is smaller than 'dist', 'dist' is replaced and return is true
    // If the distance is bigger, 'dist' doesn't change and the return is false
    // ray contains the point on object1 (0-2), the point on object2 (3-5) and the distance (6)
    // cachedPairWasProcessed tells the called whether he still needs to process the incriminated pair
    // In each cache we have: objectID, triangleID (if shape)

    cachedPairWasProcessed=false;
    bool isSmaller=false;

    long long int extCache1=getExtendedCacheValue(cache1[1]);
    long long int extCache2=getExtendedCacheValue(cache2[1]);
    if ( (extCache1==0)||(extCache2==0)||(ptCloud1->getPointCloudInfo()==nullptr)||(ptCloud2->getPointCloudInfo()==nullptr) )
        return(isSmaller);
    C4X4Matrix pts1M;
    C4X4Matrix pts2M;
    int pts1Cnt;
    int pts2Cnt;
    const float* pts1=CPluginContainer::mesh_getPointCloudPointsFromCache(ptCloud1->getPointCloudInfo(),ptCloud1->getCumulativeTransformation().getMatrix(),extCache1,pts1Cnt,pts1M);
    const float* pts2=CPluginContainer::mesh_getPointCloudPointsFromCache(ptCloud2->getPointCloudInfo(),ptCloud2->getCumulativeTransformation().getMatrix(),extCache2,pts2Cnt,pts2M);
    if ( (pts1==nullptr)||(pts2==nullptr) )
        return(isSmaller);
    for (int i=0;i<pts1Cnt;i++)
    {
        C3Vector p1(pts1+3*i);
        p1*=pts1M;
        for (int j=0;j<pts2Cnt;j++)
        {
            C3Vector p2(pts2+3*j);
            p2*=pts2M;
            float d=(p1-p2).getLength();
            if (d<dist)
            {
                dist=d;
                ray[0]=p1(0);
                ray[1]=p1(1);
                ray[2]=p1(2);
                ray[3]=p2(0);
                ray[4]=p2(1);
                ray[5]=p2(2);
                ray[6]=dist;
                isSmaller=true;
            }
        }
    }
    isSmaller|=_getPointCloudPointCloudDistanceIfSmaller(ptCloud1,ptCloud2,dist,ray,cache1,cache2,overrideMeasurableFlagObject1,overrideMeasurableFlagObject2);
    cachedPairWasProcessed=true;
    return(isSmaller);
}

bool CDistanceRoutine::_getCachedDistancePointCloudDummyIfSmaller(CPointCloud* pointCloud,CDummy* dummy,float& dist,float ray[7],int cache1[2],int cache2[2],bool overrideMeasurableFlagObject1,bool overrideMeasurableFlagObject2,bool& cachedPairWasProcessed)
{   // If the distance is smaller than 'dist', 'dist' is replaced and return is true
    // If the distance is bigger, 'dist' doesn't change and the return is false
    // ray contains the point on object1 (0-2), the point on object2 (3-5) and the distance (6)
    // cachedPairWasProcessed tells the called whether he still needs to process the incriminated pair
    // In each cache we have: objectID, triangleID (if shape)

    cachedPairWasProcessed=false;
    bool isSmaller=false;

    long long int extCache1=getExtendedCacheValue(cache1[1]);
    if ( (extCache1==0)||(pointCloud->getPointCloudInfo()==nullptr) )
        return(isSmaller);
    C4X4Matrix pts1M;
    int pts1Cnt;
    const float* pts1=CPluginContainer::mesh_getPointCloudPointsFromCache(pointCloud->getPointCloudInfo(),pointCloud->getCumulativeTransformation().getMatrix(),extCache1,pts1Cnt,pts1M);
    if (pts1==nullptr)
        return(isSmaller);
    C3Vector theSinglePoint(dummy->getCumulativeTransformation().X);
    for (int i=0;i<pts1Cnt;i++)
    {
        C3Vector p1(pts1+3*i);
        p1*=pts1M;
        float d=(p1-theSinglePoint).getLength();
        if (d<dist)
        {
            dist=d;
            ray[0]=p1(0);
            ray[1]=p1(1);
            ray[2]=p1(2);
            ray[3]=theSinglePoint(0);
            ray[4]=theSinglePoint(1);
            ray[5]=theSinglePoint(2);
            ray[6]=dist;
            isSmaller=true;
        }
    }
    isSmaller|=_getPointCloudDummyDistanceIfSmaller(pointCloud,dummy,dist,ray,cache1,cache2,overrideMeasurableFlagObject1,overrideMeasurableFlagObject2);
    cachedPairWasProcessed=true;
    return(isSmaller);
}

bool CDistanceRoutine::_getCachedDistanceOctreeDummyIfSmaller(COctree* octree,CDummy* dummy,float& dist,float ray[7],int cache1[2],int cache2[2],bool overrideMeasurableFlagObject1,bool overrideMeasurableFlagObject2,bool& cachedPairWasProcessed)
{   // If the distance is smaller than 'dist', 'dist' is replaced and return is true
    // If the distance is bigger, 'dist' doesn't change and the return is false
    // ray contains the point on object1 (0-2), the point on object2 (3-5) and the distance (6)
    // cachedPairWasProcessed tells the called whether he still needs to process the incriminated pair
    // In each cache we have: objectID, triangleID (if shape)
    cachedPairWasProcessed=false;
    bool isSmaller=false;

    long long int extCache1=getExtendedCacheValue(cache1[1]);
    if ( (extCache1==0)||(octree->getOctreeInfo()==nullptr) )
        return(isSmaller);
    C4X4Matrix cellM;
    float cellSize;

    if (!CPluginContainer::mesh_getOctreeCellFromCache(octree->getOctreeInfo(),octree->getCumulativeTransformation().getMatrix(),extCache1,cellSize,cellM))
        return(isSmaller);
    C3Vector theSinglePoint(dummy->getCumulativeTransformation().X);

    C3Vector _pt(theSinglePoint);
    _pt*=cellM.getInverse(); // _pt is now relative to cell that is at origin

    C3Vector pt(_pt);
    float boxHalfSize=cellSize*0.5;
    if (pt(0)>boxHalfSize)
        pt(0)=boxHalfSize;
    if (pt(0)<-boxHalfSize)
        pt(0)=-boxHalfSize;
    if (pt(1)>boxHalfSize)
        pt(1)=boxHalfSize;
    if (pt(1)<-boxHalfSize)
        pt(1)=-boxHalfSize;
    if (pt(2)>boxHalfSize)
        pt(2)=boxHalfSize;
    if (pt(2)<-boxHalfSize)
        pt(2)=-boxHalfSize;
    float d=(pt-_pt).getLength();
    if (d<dist)
    {
        dist=d;
        pt*=cellM; // pt is now absolute
        ray[0]=pt(0);
        ray[1]=pt(1);
        ray[2]=pt(2);
        ray[3]=theSinglePoint(0);
        ray[4]=theSinglePoint(1);
        ray[5]=theSinglePoint(2);
        ray[6]=dist;
        isSmaller=true;
    }
    isSmaller|=_getOctreeDummyDistanceIfSmaller(octree,dummy,dist,ray,cache1,cache2,overrideMeasurableFlagObject1,overrideMeasurableFlagObject2);
    cachedPairWasProcessed=true;
    return(isSmaller);
}

bool CDistanceRoutine::_getCachedDistanceDummyPointCloudIfSmaller(CDummy* dummy,CPointCloud* pointCloud,float& dist,float ray[7],int cache1[2],int cache2[2],bool overrideMeasurableFlagObject1,bool overrideMeasurableFlagObject2,bool& cachedPairWasProcessed)
{   // If the distance is smaller than 'dist', 'dist' is replaced and return is true
    // If the distance is bigger, 'dist' doesn't change and the return is false
    // ray contains the point on object1 (0-2), the point on object2 (3-5) and the distance (6)
    // cachedPairWasProcessed tells the called whether he still needs to process the incriminated pair
    // In each cache we have: objectID, triangleID (if shape)

    float _ray[7];
    bool isSmaller=_getCachedDistancePointCloudDummyIfSmaller(pointCloud,dummy,dist,_ray,cache2,cache1,overrideMeasurableFlagObject2,overrideMeasurableFlagObject1,cachedPairWasProcessed);
    if (isSmaller)
        _copyInvertedRay(_ray,ray);
    return(isSmaller);
}

bool CDistanceRoutine::_getCachedDistanceDummyOctreeIfSmaller(CDummy* dummy,COctree* octree,float& dist,float ray[7],int cache1[2],int cache2[2],bool overrideMeasurableFlagObject1,bool overrideMeasurableFlagObject2,bool& cachedPairWasProcessed)
{   // If the distance is smaller than 'dist', 'dist' is replaced and return is true
    // If the distance is bigger, 'dist' doesn't change and the return is false
    // ray contains the point on object1 (0-2), the point on object2 (3-5) and the distance (6)
    // cachedPairWasProcessed tells the called whether he still needs to process the incriminated pair
    // In each cache we have: objectID, triangleID (if shape)

    float _ray[7];
    bool isSmaller=_getCachedDistanceOctreeDummyIfSmaller(octree,dummy,dist,_ray,cache2,cache1,overrideMeasurableFlagObject2,overrideMeasurableFlagObject1,cachedPairWasProcessed);
    if (isSmaller)
        _copyInvertedRay(_ray,ray);
    return(isSmaller);
}

bool CDistanceRoutine::_getCachedDistancePointCloudOctreeIfSmaller(CPointCloud* pointCloud,COctree* octree,float& dist,float ray[7],int cache1[2],int cache2[2],bool overrideMeasurableFlagObject1,bool overrideMeasurableFlagObject2,bool& cachedPairWasProcessed)
{   // If the distance is smaller than 'dist', 'dist' is replaced and return is true
    // If the distance is bigger, 'dist' doesn't change and the return is false
    // ray contains the point on object1 (0-2), the point on object2 (3-5) and the distance (6)
    // cachedPairWasProcessed tells the called whether he still needs to process the incriminated pair
    // In each cache we have: objectID, triangleID (if shape)

    float _ray[7];
    bool isSmaller=_getCachedDistanceOctreePointCloudIfSmaller(octree,pointCloud,dist,_ray,cache2,cache1,overrideMeasurableFlagObject2,overrideMeasurableFlagObject1,cachedPairWasProcessed);
    if (isSmaller)
        _copyInvertedRay(_ray,ray);
    return(isSmaller);
}

bool CDistanceRoutine::_getCachedDistanceOctreePointCloudIfSmaller(COctree* octree,CPointCloud* pointCloud,float& dist,float ray[7],int cache1[2],int cache2[2],bool overrideMeasurableFlagObject1,bool overrideMeasurableFlagObject2,bool& cachedPairWasProcessed)
{   // If the distance is smaller than 'dist', 'dist' is replaced and return is true
    // If the distance is bigger, 'dist' doesn't change and the return is false
    // ray contains the point on object1 (0-2), the point on object2 (3-5) and the distance (6)
    // cachedPairWasProcessed tells the called whether he still needs to process the incriminated pair
    // In each cache we have: objectID, triangleID (if shape)
    cachedPairWasProcessed=false;
    bool isSmaller=false;

    long long int extCache1=getExtendedCacheValue(cache1[1]);
    long long int extCache2=getExtendedCacheValue(cache2[1]);
    if ( (extCache1==0)||(octree->getOctreeInfo()==nullptr) )
        return(isSmaller);
    if ( (extCache2==0)||(pointCloud->getPointCloudInfo()==nullptr) )
        return(isSmaller);
    C4X4Matrix cellM;
    float cellSize;

    if (!CPluginContainer::mesh_getOctreeCellFromCache(octree->getOctreeInfo(),octree->getCumulativeTransformation().getMatrix(),extCache1,cellSize,cellM))
        return(isSmaller);

    C4X4Matrix ptCloudM;
    int ptCnt;
    float* pts=CPluginContainer::mesh_getPointCloudPointsFromCache(pointCloud->getPointCloudInfo(),pointCloud->getCumulativeTransformation().getMatrix(),extCache2,ptCnt,ptCloudM);
    if (pts==nullptr)
        return(isSmaller);
    C4X4Matrix ptCloudMRelToCellM(cellM.getInverse()*ptCloudM);
    for (int i=0;i<ptCnt;i++)
    {
        C3Vector _pt(&pts[3*i]);
        _pt*=ptCloudMRelToCellM;
        C3Vector pt(_pt);
        float boxHalfSize=cellSize*0.5;
        if (pt(0)>boxHalfSize)
            pt(0)=boxHalfSize;
        if (pt(0)<-boxHalfSize)
            pt(0)=-boxHalfSize;
        if (pt(1)>boxHalfSize)
            pt(1)=boxHalfSize;
        if (pt(1)<-boxHalfSize)
            pt(1)=-boxHalfSize;
        if (pt(2)>boxHalfSize)
            pt(2)=boxHalfSize;
        if (pt(2)<-boxHalfSize)
            pt(2)=-boxHalfSize;
        float d=(pt-_pt).getLength();
        if (d<dist)
        {
            dist=d;
            pt*=cellM; // pt is now absolute
            ray[0]=pt(0);
            ray[1]=pt(1);
            ray[2]=pt(2);
            _pt*=cellM;
            ray[3]=_pt(0);
            ray[4]=_pt(1);
            ray[5]=_pt(2);
            ray[6]=dist;
            isSmaller=true;
        }
    }

    isSmaller|=_getOctreePointCloudDistanceIfSmaller(octree,pointCloud,dist,ray,cache1,cache2,overrideMeasurableFlagObject1,overrideMeasurableFlagObject2);
    cachedPairWasProcessed=true;
    return(isSmaller);
}

bool CDistanceRoutine::_getCachedDistanceShapeOctreeIfSmaller(CShape* shape,COctree* octree,float& dist,float ray[7],int cache1[2],int cache2[2],bool overrideMeasurableFlagObject1,bool overrideMeasurableFlagObject2,bool& cachedPairWasProcessed)
{   // If the distance is smaller than 'dist', 'dist' is replaced and return is true
    // If the distance is bigger, 'dist' doesn't change and the return is false
    // ray contains the point on object1 (0-2), the point on object2 (3-5) and the distance (6)
    // cachedPairWasProcessed tells the called whether he still needs to process the incriminated pair
    // In each cache we have: objectID, triangleID (if shape)

    float _ray[7];
    bool isSmaller=_getCachedDistanceOctreeShapeIfSmaller(octree,shape,dist,_ray,cache2,cache1,overrideMeasurableFlagObject2,overrideMeasurableFlagObject1,cachedPairWasProcessed);
    if (isSmaller)
        _copyInvertedRay(_ray,ray);
    return(isSmaller);
}

bool CDistanceRoutine::_getCachedDistanceOctreeShapeIfSmaller(COctree* octree,CShape* shape,float& dist,float ray[7],int cache1[2],int cache2[2],bool overrideMeasurableFlagObject1,bool overrideMeasurableFlagObject2,bool& cachedPairWasProcessed)
{   // If the distance is smaller than 'dist', 'dist' is replaced and return is true
    // If the distance is bigger, 'dist' doesn't change and the return is false
    // ray contains the point on object1 (0-2), the point on object2 (3-5) and the distance (6)
    // cachedPairWasProcessed tells the called whether he still needs to process the incriminated pair
    // In each cache we have: objectID, triangleID (if shape)
    cachedPairWasProcessed=false;
    bool isSmaller=false;
    if (!shape->isCollisionInformationInitialized())
        return(isSmaller);

    long long int extCache1=getExtendedCacheValue(cache1[1]);
    if ( (extCache1==0)||(octree->getOctreeInfo()==nullptr) )
        return(isSmaller);
    C4X4Matrix cellM;
    float cellSize;

    if (!CPluginContainer::mesh_getOctreeCellFromCache(octree->getOctreeInfo(),octree->getCumulativeTransformation().getMatrix(),extCache1,cellSize,cellM))
        return(isSmaller);

    if ( (cache2[1]<0)||(CPluginContainer::mesh_getCalculatedTriangleCount(shape->geomData->collInfo)*3<=(cache2[1]*3+2)) )
        return(isSmaller); // that index doesn't exist
    C3Vector t1a;
    C3Vector t1b;
    C3Vector t1c;
    if (!CPluginContainer::mesh_getCalculatedTriangleAt(shape->geomData->collInfo,t1a,t1b,t1c,cache2[1]))
        return(isSmaller); // triangle was disabled
    C7Vector bCTM(cellM.getInverse()*shape->getCumulativeTransformationPart1());
    t1a*=bCTM;
    t1b*=bCTM;
    t1c*=bCTM;
    t1b-=t1a;
    t1c-=t1a;
    // The triangle is now relative to the cell ref. frame
    C3Vector distPt1;
    C3Vector distPt2;
    if (CPluginContainer::mesh_getMinDistBetweenCubeAndTriangleIfSmaller(cellSize,t1a,t1b,t1c,dist,distPt1,distPt2))
    {
        distPt1*=cellM;
        distPt2*=cellM;
        ray[0]=distPt1(0);
        ray[1]=distPt1(1);
        ray[2]=distPt1(2);
        ray[3]=distPt2(0);
        ray[4]=distPt2(1);
        ray[5]=distPt2(2);
        ray[6]=dist;
        isSmaller=true;
    }
    isSmaller|=_getOctreeShapeDistanceIfSmaller(octree,shape,dist,ray,cache1,cache2,overrideMeasurableFlagObject1,overrideMeasurableFlagObject2);
    cachedPairWasProcessed=true;
    return(isSmaller);
}

bool CDistanceRoutine::_getCachedDistanceShapePointCloudIfSmaller(CShape* shape,CPointCloud* pointCloud,float& dist,float ray[7],int cache1[2],int cache2[2],bool overrideMeasurableFlagObject1,bool overrideMeasurableFlagObject2,bool& cachedPairWasProcessed)
{   // If the distance is smaller than 'dist', 'dist' is replaced and return is true
    // If the distance is bigger, 'dist' doesn't change and the return is false
    // ray contains the point on object1 (0-2), the point on object2 (3-5) and the distance (6)
    // cachedPairWasProcessed tells the called whether he still needs to process the incriminated pair
    // In each cache we have: objectID, triangleID (if shape)

    float _ray[7];
    bool isSmaller=_getCachedDistancePointCloudShapeIfSmaller(pointCloud,shape,dist,_ray,cache2,cache1,overrideMeasurableFlagObject2,overrideMeasurableFlagObject1,cachedPairWasProcessed);
    if (isSmaller)
        _copyInvertedRay(_ray,ray);
    return(isSmaller);
}

bool CDistanceRoutine::_getCachedDistancePointCloudShapeIfSmaller(CPointCloud* pointCloud,CShape* shape,float& dist,float ray[7],int cache1[2],int cache2[2],bool overrideMeasurableFlagObject1,bool overrideMeasurableFlagObject2,bool& cachedPairWasProcessed)
{   // If the distance is smaller than 'dist', 'dist' is replaced and return is true
    // If the distance is bigger, 'dist' doesn't change and the return is false
    // ray contains the point on object1 (0-2), the point on object2 (3-5) and the distance (6)
    // cachedPairWasProcessed tells the called whether he still needs to process the incriminated pair
    // In each cache we have: objectID, triangleID (if shape)
    cachedPairWasProcessed=false;
    bool isSmaller=false;
    if (!shape->isCollisionInformationInitialized())
        return(isSmaller);

    long long int extCache1=getExtendedCacheValue(cache1[1]);
    if ( (extCache1==0)||(pointCloud->getPointCloudInfo()==nullptr) )
        return(isSmaller);
    C4X4Matrix pts1M;
    int pts1Cnt;
    const float* pts1=CPluginContainer::mesh_getPointCloudPointsFromCache(pointCloud->getPointCloudInfo(),pointCloud->getCumulativeTransformation().getMatrix(),extCache1,pts1Cnt,pts1M);
    if (pts1==nullptr)
        return(isSmaller);

    if ( (cache2[1]<0)||(CPluginContainer::mesh_getCalculatedTriangleCount(shape->geomData->collInfo)*3<=(cache2[1]*3+2)) )
        return(isSmaller); // that index doesn't exist
    C3Vector t1a;
    C3Vector t1b;
    C3Vector t1c;
    if (!CPluginContainer::mesh_getCalculatedTriangleAt(shape->geomData->collInfo,t1a,t1b,t1c,cache2[1]))
        return(isSmaller); // triangle was disabled
    C7Vector bCTM(shape->getCumulativeTransformationPart1());
    t1a*=bCTM;
    t1b*=bCTM;
    t1c*=bCTM;
    t1b-=t1a;
    t1c-=t1a;

    C3Vector distPt2;
    for (int i=0;i<pts1Cnt;i++)
    {
        C3Vector pt(pts1M*C3Vector(pts1+3*i));
        if (CPluginContainer::mesh_getMinDistBetweenPointAndTriangleIfSmaller(pt,t1a,t1b,t1c,dist,distPt2))
        {
            ray[0]=pt(0);
            ray[1]=pt(1);
            ray[2]=pt(2);
            ray[3]=distPt2(0);
            ray[4]=distPt2(1);
            ray[5]=distPt2(2);
            ray[6]=dist;
            isSmaller=true;
        }
    }

    isSmaller|=_getPointCloudShapeDistanceIfSmaller(pointCloud,shape,dist,ray,cache1,cache2,overrideMeasurableFlagObject1,overrideMeasurableFlagObject2);
    cachedPairWasProcessed=true;
    return(isSmaller);
}
