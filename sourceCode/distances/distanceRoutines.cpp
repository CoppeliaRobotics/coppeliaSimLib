#include "distanceRoutines.h"
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

unsigned long long int CDistanceRoutine::getExtendedCacheValue(int id)
{
    for (size_t i=0;i<_extendedCacheBuffer.size();i++)
    {
        if (_extendedCacheBuffer[i].id==id)
            return(_extendedCacheBuffer[i].cache);
    }
    return(0);
}

int CDistanceRoutine::insertExtendedCacheValue(unsigned long long int value)
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
    C7Vector tr1,tr2;
    C3Vector hs1,hs2;
    octree1->getTransfAndHalfSizeOfBoundingBox(tr1,hs1);
    octree2->getTransfAndHalfSizeOfBoundingBox(tr2,hs2);
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
    CSceneObject* object1=App::currentWorld->sceneObjects->getObjectFromHandle(entity1ID);
    CSceneObject* object2=App::currentWorld->sceneObjects->getObjectFromHandle(entity2ID);
    CCollection* collection1=nullptr;
    CCollection* collection2=nullptr;
    if (object1==nullptr)
    {
        collection1=App::currentWorld->collections->getObjectFromHandle(entity1ID);
        if (collection1==nullptr)
            return(false);
    }
    if (object2==nullptr)
    {
        collection2=App::currentWorld->collections->getObjectFromHandle(entity2ID);
        if ( (collection2==nullptr)&&(entity2ID!=-1) )
            return(false);
    }

    if (object1!=nullptr)
    { // We have an object against...
        if (object2!=nullptr)
        { // ...another object

            bool doTheCheck=true;
            if ( (entity1ID==cache1[0])&&(entity2ID==cache2[0]) )
            { // check against cached values:
                bool cachedPairWasProcessed=false;
                returnValue=_getCachedDistanceIfSmaller(dist,ray,cache1,cache2,overrideMeasurableFlagIfNonCollection1,overrideMeasurableFlagIfNonCollection2,cachedPairWasProcessed)||returnValue;
                doTheCheck=!cachedPairWasProcessed;
            }
            if (doTheCheck)
                returnValue=_getObjectObjectDistanceIfSmaller(object1,object2,dist,ray,cache1,cache2,overrideMeasurableFlagIfNonCollection1,overrideMeasurableFlagIfNonCollection2)||returnValue;
        }
        else
        { // an objects VS a collection or all other objects
            std::vector<CSceneObject*> group;
            if (entity2ID==-1)
            { // Special group here (all objects except the object):
                std::vector<CSceneObject*> exception;
                exception.push_back(object1);
                App::currentWorld->sceneObjects->getAllMeasurableObjectsFromSceneExcept(&exception,group);
            }
            else
            { // Regular group here:
                App::currentWorld->collections->getMeasurableObjectsFromCollection(entity2ID,group);
            }
            if (group.size()!=0)
            {
                std::vector<CSceneObject*> pairs;
                _generateValidPairsFromObjectGroup(object1,group,pairs);

                if (entity1ID==cache1[0]) // we can only check for the object
                { // check against cached values:
                    bool cachedPairWasProcessed=false;
                    returnValue=_getCachedDistanceIfSmaller_pairs(pairs,dist,ray,cache1,cache2,overrideMeasurableFlagIfNonCollection1,overrideMeasurableFlagIfNonCollection2,cachedPairWasProcessed)||returnValue;
                }
                returnValue=_getObjectPairsDistanceIfSmaller(pairs,dist,ray,cache1,cache2,overrideMeasurableFlagIfNonCollection1,true)||returnValue;
            }
        }
    }
    else
    { // We have a group against...
        std::vector<CSceneObject*> group1;
        App::currentWorld->collections->getMeasurableObjectsFromCollection(entity1ID,group1);
        if (group1.size()!=0)
        {
            if (object2!=nullptr)
            { //...an object
                std::vector<CSceneObject*> pairs;
                _generateValidPairsFromGroupObject(group1,object2,pairs);

                if (entity2ID==cache2[0]) // we can only check for the object
                { // check against cached values:
                    bool cachedPairWasProcessed=false;
                    returnValue=_getCachedDistanceIfSmaller_pairs(pairs,dist,ray,cache1,cache2,overrideMeasurableFlagIfNonCollection1,overrideMeasurableFlagIfNonCollection2,cachedPairWasProcessed)||returnValue;
                }
                returnValue=_getObjectPairsDistanceIfSmaller(pairs,dist,ray,cache1,cache2,true,overrideMeasurableFlagIfNonCollection2)||returnValue;
            }
            else
            { // ...another group or all other objects (entity2ID could be -1)
                std::vector<CSceneObject*> group2;
                if (entity2ID==-1)
                { // Special group here
                    App::currentWorld->sceneObjects->getAllMeasurableObjectsFromSceneExcept(&group1,group2);
                }
                else
                { // Regular group here:
                    App::currentWorld->collections->getMeasurableObjectsFromCollection(entity2ID,group2);
                }
                if (group2.size()!=0)
                {
                    std::vector<CSceneObject*> pairs;
                    _generateValidPairsFromGroupGroup(group1,group2,pairs,entity1ID==entity2ID);

                    bool cachedPairWasProcessed=false;
                    returnValue=_getCachedDistanceIfSmaller_pairs(pairs,dist,ray,cache1,cache2,overrideMeasurableFlagIfNonCollection1,overrideMeasurableFlagIfNonCollection2,cachedPairWasProcessed)||returnValue;
                    returnValue=_getObjectPairsDistanceIfSmaller(pairs,dist,ray,cache1,cache2,true,true)||returnValue;
                }
            }
        }
    }
    return(returnValue);
}

float CDistanceRoutine::_getApproxBoundingBoxDistance(CSceneObject* obj1,CSceneObject* obj2)
{ // the returned distance is always same or smaller than the real distance!
    bool isPt[2]={false,false};
    CSceneObject* objs[2]={obj1,obj2};
    C3Vector halfSizes[2];
    C7Vector m[2];
    for (size_t cnt=0;cnt<2;cnt++)
    {
        CSceneObject* obj=objs[cnt];
        if (obj->getObjectType()==sim_object_shape_type)
        {
            halfSizes[cnt]=((CShape*)obj)->getBoundingBoxHalfSizes();
            m[cnt]=obj->getFullCumulativeTransformation();
        }
        if (obj->getObjectType()==sim_object_dummy_type)
        {
            isPt[cnt]=true;
            m[cnt]=obj->getFullCumulativeTransformation();
        }
        if (obj->getObjectType()==sim_object_octree_type)
            ((COctree*)obj)->getTransfAndHalfSizeOfBoundingBox(m[cnt],halfSizes[cnt]);
        if (obj->getObjectType()==sim_object_pointcloud_type)
            ((CPointCloud*)obj)->getTransfAndHalfSizeOfBoundingBox(m[cnt],halfSizes[cnt]);
    }
    if (isPt[0])
    { // pt vs ...
        if (isPt[1])
            return((m[0].X-m[1].X).getLength()); // pt vs pt
        else
            return(CPluginContainer::geomPlugin_getBoxPointDistance(m[1],halfSizes[1],true,m[0].X)); // pt vs box
    }
    else
    { // box vs ...
        if (isPt[1])
            return(CPluginContainer::geomPlugin_getBoxPointDistance(m[0],halfSizes[0],true,m[1].X)); // box vs pt
        else
        {
            // box box is too slow here
            return(CPluginContainer::geomPlugin_getApproxBoxBoxDistance(m[0],halfSizes[0],m[1],halfSizes[1])); // box vs box
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

    C3Vector v0(dummy1->getCumulativeTransformation().X);
    C3Vector v1(dummy2->getCumulativeTransformation().X);
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

    shape->initializeMeshCalculationStructureIfNeeded();

    C3Vector dummyPos(dummy->getFullCumulativeTransformation().X);
    C3Vector rayPart0;
    C3Vector rayPart1(dummyPos);
    int buffer=0;
    if (CPluginContainer::geomPlugin_getMeshPointDistanceIfSmaller(shape->_meshCalculationStructure,shape->getFullCumulativeTransformation(),dummyPos,dist,&rayPart0,&buffer))
    {
        rayPart0.getData(ray);
        rayPart1.getData(ray+3);
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

    shape1->initializeMeshCalculationStructureIfNeeded();
    shape2->initializeMeshCalculationStructureIfNeeded();

    C3Vector minDistPt1,minDistPt2;
    if (CPluginContainer::geomPlugin_getMeshMeshDistanceIfSmaller(shape1->_meshCalculationStructure,shape1->getFullCumulativeTransformation(),shape2->_meshCalculationStructure,shape2->getFullCumulativeTransformation(),dist,&minDistPt1,&minDistPt2,cache1+1,cache2+1))
    {
        minDistPt1.getData(ray+0);
        minDistPt2.getData(ray+3);
        ray[6]=dist;
        cache1[0]=shape1->getObjectHandle();
        cache2[0]=shape2->getObjectHandle();
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

    unsigned long long int cacheV=getExtendedCacheValue(cache1[1]);
    C3Vector dummyPos(dummy->getFullCumulativeTransformation().X);
    C3Vector minDistPt;
    if (CPluginContainer::geomPlugin_getOctreePointDistanceIfSmaller(octree->getOctreeInfo(),octree->getFullCumulativeTransformation(),dummyPos,dist,&minDistPt,&cacheV))
    {
        minDistPt.getData(ray+0);
        dummyPos.getData(ray+3);
        ray[6]=dist;
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

    shape->initializeMeshCalculationStructureIfNeeded();

    unsigned long long int cache1V=getExtendedCacheValue(cache1[1]);
    C3Vector distPt1;
    C3Vector distPt2;
    if (CPluginContainer::geomPlugin_getMeshOctreeDistanceIfSmaller(shape->_meshCalculationStructure,shape->getFullCumulativeTransformation(),octree->getOctreeInfo(),octree->getFullCumulativeTransformation(),dist,&distPt2,&distPt1,cache2+1,&cache1V))
    {
        distPt1.getData(ray+0);
        distPt2.getData(ray+3);
        ray[6]=dist;
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

    unsigned long long int cache1V=getExtendedCacheValue(cache1[1]);
    unsigned long long int cache2V=getExtendedCacheValue(cache2[1]);
    if (_distanceCachingOff)
    {
        cache1V=0;
        cache2V=0;
    }
    bool hasCoherency=getOctreesHaveCoherentMovement(octree1,octree2);
    C3Vector distPt1;
    C3Vector distPt2;
    if (CPluginContainer::geomPlugin_getOctreeOctreeDistanceIfSmaller(octree1->getOctreeInfo(),octree1->getFullCumulativeTransformation(),octree2->getOctreeInfo(),octree2->getFullCumulativeTransformation(),dist,&distPt1,&distPt2,&cache1V,&cache2V))
    {
        distPt1.getData(ray+0);
        distPt2.getData(ray+3);
        ray[6]=dist;
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

    unsigned long long int cacheV=getExtendedCacheValue(cache1[1]);
    C3Vector dummyPos(dummy->getFullCumulativeTransformation().X);
    C3Vector distPt;
    if (CPluginContainer::geomPlugin_getPtcloudPointDistanceIfSmaller(pointCloud->getPointCloudInfo(),pointCloud->getFullCumulativeTransformation(),dummyPos,dist,&distPt,&cacheV))
    {
        distPt.getData(ray+0);
        dummyPos.getData(ray+3);
        ray[6]=dist;
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

    shape->initializeMeshCalculationStructureIfNeeded();

    unsigned long long int cache1V=getExtendedCacheValue(cache1[1]);
    C3Vector distPt1;
    C3Vector distPt2;
    if (CPluginContainer::geomPlugin_getMeshPtcloudDistanceIfSmaller(shape->_meshCalculationStructure,shape->getFullCumulativeTransformation(),pointCloud->getPointCloudInfo(),pointCloud->getFullCumulativeTransformation(),dist,&distPt2,&distPt1,cache2+1,&cache1V))
    {
        distPt1.getData(ray+0);
        distPt2.getData(ray+3);
        ray[6]=dist;
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

    unsigned long long int cache1V=getExtendedCacheValue(cache1[1]);
    unsigned long long int cache2V=getExtendedCacheValue(cache2[1]);
    C3Vector distPt1;
    C3Vector distPt2;
    if (CPluginContainer::geomPlugin_getOctreePtcloudDistanceIfSmaller(octree->getOctreeInfo(),octree->getFullCumulativeTransformation(),pointCloud->getPointCloudInfo(),pointCloud->getFullCumulativeTransformation(),dist,&distPt1,&distPt2,&cache1V,&cache2V))
    {
        distPt1.getData(ray+0);
        distPt2.getData(ray+3);
        ray[6]=dist;
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

    unsigned long long int cache1V=getExtendedCacheValue(cache1[1]);
    unsigned long long int cache2V=getExtendedCacheValue(cache2[1]);
    C3Vector distPt1;
    C3Vector distPt2;
    if (CPluginContainer::geomPlugin_getPtcloudPtcloudDistanceIfSmaller(pointCloud1->getPointCloudInfo(),pointCloud1->getFullCumulativeTransformation(),pointCloud2->getPointCloudInfo(),pointCloud2->getFullCumulativeTransformation(),dist,&distPt1,&distPt2,&cache1V,&cache2V))
    {
        distPt1.getData(ray+0);
        distPt2.getData(ray+3);
        ray[6]=dist;
        cache1[0]=pointCloud1->getObjectHandle();
        cache1[1]=insertExtendedCacheValue(cache1V);
        cache2[0]=pointCloud2->getObjectHandle();
        cache2[1]=insertExtendedCacheValue(cache2V);
        return(true);
    }
    return(false);
}

bool CDistanceRoutine::_getObjectPairsDistanceIfSmaller(const std::vector<CSceneObject*>& unorderedPairs,float& dist,float ray[7],int cache1[2],int cache2[2],bool overrideMeasurableFlagObject1,bool overrideMeasurableFlagObject2)
{
    std::vector<CSceneObject*> pairs(unorderedPairs);
    float approxDist=_orderPairsAccordingToApproxBoundingBoxDistance(pairs);
    if (approxDist>=dist)
        return(false);
    bool retVal=false;
    for (size_t i=0;i<pairs.size()/2;i++)
        retVal=_getObjectObjectDistanceIfSmaller(pairs[2*i+0],pairs[2*i+1],dist,ray,cache1,cache2,overrideMeasurableFlagObject1,overrideMeasurableFlagObject2)||retVal;

    return(retVal);
}

bool CDistanceRoutine::_getObjectObjectDistanceIfSmaller(CSceneObject* object1,CSceneObject* object2,float& dist,float ray[7],int cache1[2],int cache2[2],bool overrideMeasurableFlagObject1,bool overrideMeasurableFlagObject2)
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

void CDistanceRoutine::_generateValidPairsFromObjectGroup(CSceneObject* obj,const std::vector<CSceneObject*>& group,std::vector<CSceneObject*>& pairs)
{
    for (size_t i=0;i<group.size();i++)
    {
        CSceneObject* obj2=group[i];
        if (obj!=obj2)
        { // We never check an object against itself!
            pairs.push_back(obj);
            pairs.push_back(obj2);
        }
    }
}

void CDistanceRoutine::_generateValidPairsFromGroupObject(const std::vector<CSceneObject*>& group,CSceneObject* obj,std::vector<CSceneObject*>& pairs)
{
    for (size_t i=0;i<group.size();i++)
    {
        CSceneObject* obj1=group[i];
        if (obj!=obj1)
        { // We never check an object against itself!
            pairs.push_back(obj1);
            pairs.push_back(obj);
        }
    }
}

void CDistanceRoutine::_generateValidPairsFromGroupGroup(const std::vector<CSceneObject*>& group1,const std::vector<CSceneObject*>& group2,std::vector<CSceneObject*>& pairs,bool collectionSelfDistanceCheck)
{
    for (size_t i=0;i<group1.size();i++)
    {
        CSceneObject* obj1=group1[i];
        int csci1=obj1->getCollectionSelfCollisionIndicator();
        for (size_t j=0;j<group2.size();j++)
        {
            CSceneObject* obj2=group2[j];
            int csci2=obj2->getCollectionSelfCollisionIndicator();
            if (obj1!=obj2)
            { // We never check an object against itself!
                if ( (!collectionSelfDistanceCheck)||((abs(csci1-csci2)!=1)&&(abs(csci1-csci2)!=10)&&(abs(csci1-csci2)!=100)&&(abs(csci1-csci2)!=1000)&&(abs(csci1-csci2)!=10000)&&(abs(csci1-csci2)!=100000)) )
                { // the collection self collision indicators differences is not 1, 10, 100 or 1000
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

float CDistanceRoutine::_orderPairsAccordingToApproxBoundingBoxDistance(std::vector<CSceneObject*>& pairs)
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

    std::vector<CSceneObject*> _pairs(pairs);
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
    if (_distanceCachingOff)
        return(false); // Distance caching is turned off
    if ( (cache1[0]<0)||(cache2[0]<0) )
        return(false);
    CSceneObject* object1=App::currentWorld->sceneObjects->getObjectFromHandle(cache1[0]);
    if (object1==nullptr)
        return(false);
    CSceneObject* object2=App::currentWorld->sceneObjects->getObjectFromHandle(cache2[0]);
    if (object2==nullptr)
        return(false);

    bool retVal=_getObjectObjectDistanceIfSmaller(object1,object2,dist,ray,cache1,cache2,overrideMeasurableFlagObject1,overrideMeasurableFlagObject2);
    cachedPairWasProcessed=true;
    return(retVal);
}

bool CDistanceRoutine::_getCachedDistanceIfSmaller_pairs(std::vector<CSceneObject*>& unorderedPairsCanBeModified,float& dist,float ray[7],int cache1[2],int cache2[2],bool overrideMeasurableFlagObject1,bool overrideMeasurableFlagObject2,bool& cachedPairWasProcessed)
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
        CSceneObject* object1=unorderedPairsCanBeModified[2*i+0];
        CSceneObject* object2=unorderedPairsCanBeModified[2*i+1];
        if ( (object1->getObjectHandle()==cache1[0])&&(object2->getObjectHandle()==cache2[0]) )
        { // we have found the pair!
            isSmaller=_getCachedDistanceIfSmaller(dist,ray,cache1,cache2,overrideMeasurableFlagObject1,overrideMeasurableFlagObject2,cachedPairWasProcessed)||isSmaller;
            unorderedPairsCanBeModified.erase(unorderedPairsCanBeModified.begin()+2*i,unorderedPairsCanBeModified.begin()+2*i+2);
            break;
        }
    }
    if (!cachedPairWasProcessed)
        cachedPairWasProcessed=(unorderedPairsCanBeModified.size()==0);
    return(isSmaller);
}
