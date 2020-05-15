#include "proxSensorRoutine.h"
#include "pluginContainer.h"
#include "app.h"
#include "tt.h"


bool CProxSensorRoutine::detectEntity(int sensorID,int entityID,bool closestFeatureMode,bool angleLimitation,float maxAngle,C3Vector& detectedPt,float& dist,bool frontFace,bool backFace,int& detectedObject,float minThreshold,C3Vector& triNormal,bool overrideDetectableFlagIfNonCollection)
{ // entityID==-1 --> checks all objects in the scene
    bool returnValue=false;
    detectedObject=-1;
    CProxSensor* sensor=App::currentWorld->sceneObjects->getProximitySensorFromHandle(sensorID);
    CSceneObject* object=App::currentWorld->sceneObjects->getObjectFromHandle(entityID);
    if (sensor==nullptr)
        return(false); // should never happen!
    App::worldContainer->calcInfo->proximitySensorSimulationStart();
    if (sensor->getRandomizedDetection())
    {
        if (sensor->getSensorType()!=sim_proximitysensor_ray_subtype)
            return(false); // probably not needed
        sensor->calculateFreshRandomizedRays();
    }

    if (object!=nullptr)
    { // Detecting one object:
        if ( ((object->getCumulativeObjectSpecialProperty()&sensor->getSensableType())!=0)||overrideDetectableFlagIfNonCollection )
        {
            int detectObjId=_detectObject(sensor,object,detectedPt,dist,triNormal,closestFeatureMode,angleLimitation,maxAngle,frontFace,backFace,minThreshold);
            returnValue=(detectObjId>=0);
            if (detectObjId>=0)
                detectedObject=entityID;
        }
    }
    else
    {
        std::vector<CSceneObject*> group;
        if (entityID==-1)
        { // Special group here (all detectable objects):
            std::vector<CSceneObject*> exception;
            App::currentWorld->sceneObjects->getAllDetectableObjectsFromSceneExcept(&exception,group,sensor->getSensableType());
        }
        else
        { // Regular group here:
            App::currentWorld->collections->getDetectableObjectsFromCollection(entityID,group,sensor->getSensableType());
        }
        if (group.size()!=0)
        {
            _orderGroupAccordingToApproxDistanceToSensingPoint(sensor,group);
            for (size_t i=0;i<group.size();i++)
            {
                int detectObjId=_detectObject(sensor,group[i],detectedPt,dist,triNormal,closestFeatureMode,angleLimitation,maxAngle,frontFace,backFace,minThreshold);
                returnValue=returnValue||(detectObjId>=0);
                if (detectObjId>=0)
                    detectedObject=group[i]->getObjectHandle();
                if (detectObjId==-2)
                {
                    returnValue=false;
                    break; // we detected something in the forbidden zone
                }
            }
        }
    }

    if (returnValue)
        triNormal.normalize();
    App::worldContainer->calcInfo->proximitySensorSimulationEnd(returnValue);
    return(returnValue);
}

bool CProxSensorRoutine::detectPrimitive(int sensorID,float* vertexPointer,int itemType,int itemCount,
        bool closestFeatureMode,bool angleLimitation,float maxAngle,C3Vector& detectedPt,
        float& dist,bool frontFace,bool backFace,float minThreshold,C3Vector& triNormal)
{
    bool returnValue=false;
    CProxSensor* sens=App::currentWorld->sceneObjects->getProximitySensorFromHandle(sensorID);
    if (sens==nullptr)
        return(false); // should never happen!

    if (sens->getRandomizedDetection())
    {
        if (sens->getSensorType()!=sim_proximitysensor_ray_subtype)
            return(false); // probably not needed
        sens->calculateFreshRandomizedRays();
    }
    App::worldContainer->calcInfo->proximitySensorSimulationStart();


    // Now prepare for detection
    C7Vector sensInv(sens->getCumulativeTransformation().getInverse());

    if (sens->getRandomizedDetection())
    {
        if (itemType==2)
        { // detecting triangles (points and segments cannot be detected by (randomized) rays!)
            int normalDetectionCnt=0;
            C3Vector averageDetectionVector;
            C3Vector averageNormalVector;
            averageDetectionVector.clear();
            float averageDetectionDist=0.0f;

            const std::vector<C3Vector> normalizedRays=sens->getPointerToRandomizedRays()[0];
            std::vector<float> individualRayDetectionState=sens->getPointerToRandomizedRayDetectionStates()[0];
            int requiredDetectionCount=sens->getRandomizedDetectionCountForDetection();

            for (size_t j=0;j<normalizedRays.size();j++)
            {
                C3Vector lp(normalizedRays[j]*sens->convexVolume->getRadius()); // Here we have radius instead of offset! Special with randomized detection!!
                C3Vector lvFar(normalizedRays[j]*sens->convexVolume->getRange());

                float distTmp=dist;
                C3Vector detectedPtTmp;
                C3Vector triNormalNotNormalizedTmp;
                bool detect=false;
                bool leave=false;
                for (int i=0;i<itemCount;i++)
                {
                    C3Vector a0(vertexPointer[9*i+0],vertexPointer[9*i+1],vertexPointer[9*i+2]);
                    C3Vector e0(vertexPointer[9*i+3],vertexPointer[9*i+4],vertexPointer[9*i+5]);
                    C3Vector e1(vertexPointer[9*i+6],vertexPointer[9*i+7],vertexPointer[9*i+8]);
                    a0*=sensInv;
                    e0*=sensInv;
                    e1*=sensInv;
                    e0-=a0;
                    e1-=a0;
                    float _maxAngle=0.0f;
                    if (angleLimitation)
                        _maxAngle=maxAngle;
                    if (CPluginContainer::geomPlugin_volumeSensorDetectTriangleIfSmaller(sens->convexVolume->planesInside,sens->convexVolume->planesOutside,a0,e0,e1,dist,frontFace,backFace,_maxAngle,&detectedPt,&triNormal))
                    {
                        if (sens->convexVolume->getSmallestDistanceEnabled())
                        {
                            if (dist<sens->convexVolume->getSmallestDistanceAllowed())
                            { // We triggered the sensor in the forbiden zone
                                leave=true;
                                detect=false;
                                break;
                            }
                        }
                        detect=true;
                        if (!closestFeatureMode)
                        {
                            leave=true;
                            break; // No fast detection, we leave here..
                        }
                    }
                }
                if (detect)
                {
                    normalDetectionCnt++;
                    individualRayDetectionState[j]=distTmp;
                    averageDetectionVector+=detectedPtTmp;
                    averageDetectionDist+=distTmp;
                    averageNormalVector+=triNormalNotNormalizedTmp.getNormalized();
                }
                if (leave)
                    break;
            }
            if ( (normalDetectionCnt>=requiredDetectionCount)||((normalDetectionCnt>0)&&(!closestFeatureMode)) )
            {
                returnValue=true;
                dist=averageDetectionDist/float(normalDetectionCnt);
                detectedPt=averageDetectionVector/float(normalDetectionCnt);
                triNormal=averageNormalVector/float(normalDetectionCnt);
            }
        }
    }
    else
    {
        for (int i=0;i<itemCount;i++)
        {
            bool detect=false;
            if (sens->getSensorType()!=sim_proximitysensor_ray_subtype)
            { // ray-type sensors cannot detect points or segments!
                if (itemType==0)
                { // detecting points
                    C3Vector p0(vertexPointer[3*i+0],vertexPointer[3*i+1],vertexPointer[3*i+2]);
                    p0*=sensInv;
                    float l=p0.getLength();
                    if (l<=dist)
                    { // ok, the point is closer..
                        detect=CPluginContainer::geomPlugin_isPointInVolume1AndOutVolume2(sens->convexVolume->planesInside,sens->convexVolume->planesOutside,p0);
                        if (detect)
                            detectedPt=p0;
                    }
                }
                if (itemType==1)
                { // detecting segments
                    C3Vector p0(vertexPointer[6*i+0],vertexPointer[6*i+1],vertexPointer[6*i+2]);
                    C3Vector p1(vertexPointer[6*i+3],vertexPointer[6*i+4],vertexPointer[6*i+5]);
                    p0*=sensInv;
                    p1*=sensInv;
                    float _maxAngle=0.0f;
                    if (angleLimitation)
                        _maxAngle=maxAngle;
                    detect=CPluginContainer::geomPlugin_volumeSensorDetectSegmentIfSmaller(sens->convexVolume->planesInside,sens->convexVolume->planesOutside,p0,p1-p0,dist,_maxAngle,&detectedPt);
                }
            }
            if (itemType==2)
            { // detecting triangles
                C3Vector a0(vertexPointer[9*i+0],vertexPointer[9*i+1],vertexPointer[9*i+2]);
                C3Vector e0(vertexPointer[9*i+3],vertexPointer[9*i+4],vertexPointer[9*i+5]);
                C3Vector e1(vertexPointer[9*i+6],vertexPointer[9*i+7],vertexPointer[9*i+8]);
                a0*=sensInv;
                e0*=sensInv;
                e1*=sensInv;
                e0-=a0;
                e1-=a0;
                float _maxAngle=0.0f;
                if (angleLimitation)
                    _maxAngle=maxAngle;
                detect=CPluginContainer::geomPlugin_volumeSensorDetectTriangleIfSmaller(sens->convexVolume->planesInside,sens->convexVolume->planesOutside,a0,e0,e1,dist,frontFace,backFace,_maxAngle,&detectedPt,&triNormal);
            }
            if (detect)
            {
                returnValue=true;
                if (sens->convexVolume->getSmallestDistanceEnabled())
                {
                    if (dist<sens->convexVolume->getSmallestDistanceAllowed())
                    { // We triggered the sensor in the forbiden zone
                        returnValue=false;
                        break;
                    }
                }
                if (!closestFeatureMode)
                    break; // No fast detection, we leave here..
            }
        }
    }

    if (returnValue)
        triNormal.normalize();
        
    App::worldContainer->calcInfo->proximitySensorSimulationEnd(returnValue);
    return(returnValue);
}

int CProxSensorRoutine::_detectDummy(CProxSensor* sensor,CDummy* dummy,C3Vector& detectedPt,float& dist,C3Vector& triNormalNotNormalized,bool closestFeatureMode,bool angleLimitation,float maxAngle,bool frontFace,bool backFace,float minThreshold)
{ // -2: sensor triggered in the forbidden zone, -1: sensor didn't trigger. Otherwise the object handle that triggered the sensor
    if (dist==0.0)
        return(-1);
    if (sensor->getSensorType()==sim_proximitysensor_ray_subtype)
        return(-1); // ray sensors never detect dummies
    if (sensor->getRandomizedDetection())
        return(-1); // probably not needed
    if (!_doesSensorVolumeOverlapWithObjectBoundingBox(sensor,dummy))
        return(-1);
    float d=_getApproxPointObjectBoundingBoxDistance(sensor->getFullCumulativeTransformation().X,dummy);
    if (d>dist)
        return(-1);

    C7Vector inv(sensor->getFullCumulativeTransformation().getInverse());
    C4X4Matrix dummyCTM((inv*dummy->getFullCumulativeTransformation()).getMatrix());
    float theDistance=dummyCTM.X.getLength();
    if (CPluginContainer::geomPlugin_isPointInVolume1AndOutVolume2(sensor->convexVolume->planesInside,sensor->convexVolume->planesOutside,dummyCTM.X) )
    {
        if (theDistance<dist)
        {
            if (minThreshold<theDistance)
            {
                dist=theDistance;
                detectedPt=dummyCTM.X;
                triNormalNotNormalized=detectedPt;
                return(dummy->getObjectHandle());
            }
            else
                return(-2);
        }
    }
    return(-1);
}

int CProxSensorRoutine::_detectShape(CProxSensor* sensor,CShape* shape,C3Vector& detectedPt,float& dist,C3Vector& triNormalNotNormalized,bool closestFeatureMode,bool angleLimitation,float maxAngle,bool frontFace,bool backFace,float minThreshold)
{ // -2: sensor triggered in the forbidden zone, -1: sensor didn't trigger. Otherwise the object handle that triggered the sensor
    if (dist==0.0)
        return(-1);
    if (!_doesSensorVolumeOverlapWithObjectBoundingBox(sensor,shape))
        return(-1);
    float d=_getApproxPointObjectBoundingBoxDistance(sensor->getFullCumulativeTransformation().X,shape);
    if (d>dist)
        return(-1);

    shape->initializeCalculationStructureIfNeeded();

    int retVal=-1;

    C7Vector inv(sensor->getFullCumulativeTransformation().getInverse());

    C7Vector shapeITr(inv*shape->getFullCumulativeTransformation());

    if (sensor->getRandomizedDetection())
    {
        bool* closeDetectionTriggered=nullptr;
        bool dummyVal=0;
        if (sensor->convexVolume->getSmallestDistanceEnabled())
            closeDetectionTriggered=&dummyVal;
        int normalDetectionCnt=0;
        C3Vector averageDetectionVector;
        C3Vector averageNormalVector;
        averageDetectionVector.clear();
        averageNormalVector.clear();
        float averageDetectionDist=0.0f;
        const std::vector<C3Vector> normalizedRays=sensor->getPointerToRandomizedRays()[0];
        std::vector<float> individualRayDetectionState=sensor->getPointerToRandomizedRayDetectionStates()[0];
        int requiredDetectionCount=sensor->getRandomizedDetectionCountForDetection();
        for (size_t i=0;i<normalizedRays.size();i++)
        {
            C3Vector lp(normalizedRays[i]*sensor->convexVolume->getRadius()); // Here we have radius instead of offset! Special with randomized detection!!
            C3Vector lvFar(normalizedRays[i]*sensor->convexVolume->getRange());
            float distTmp=dist;
            C3Vector detectedPtTmp;
            C3Vector triNormalNotNormalizedTmp;
            float _maxAngle=0.0f;
            if (angleLimitation)
                _maxAngle=maxAngle;
            bool result=CPluginContainer::geomPlugin_raySensorDetectMeshIfSmaller(lp,lvFar,shape->geomData->collInfo,shapeITr,distTmp,sensor->convexVolume->getSmallestDistanceAllowed(),!closestFeatureMode,frontFace,backFace,_maxAngle,&detectedPtTmp,&triNormalNotNormalizedTmp,closeDetectionTriggered);
            if ((closeDetectionTriggered!=nullptr)&&closeDetectionTriggered[0])
            { // We triggered the sensor in the forbiden zone
                normalDetectionCnt=0;
                retVal=-2;
                break;
            }
            if (result)
            { // We triggered the sensor normally
                normalDetectionCnt++;
                individualRayDetectionState[i]=distTmp;
                averageDetectionVector+=detectedPtTmp;
                averageDetectionDist+=distTmp;
                averageNormalVector+=triNormalNotNormalizedTmp.getNormalized();
            }
        }

        if (normalDetectionCnt>=requiredDetectionCount)
        {
            retVal=shape->getObjectHandle();
            dist=averageDetectionDist/float(normalDetectionCnt);
            detectedPt=averageDetectionVector/float(normalDetectionCnt);
            triNormalNotNormalized=averageNormalVector/float(normalDetectionCnt);
        }
    }
    else
    {
        if (sensor->getSensorType()==sim_proximitysensor_ray_subtype)
        { // ray-type sensor here:
            C3Vector lp(0.0f,0.0f,sensor->convexVolume->getOffset());
            C3Vector lvFar(0.0f,0.0f,sensor->convexVolume->getRange());
            bool* closeDetectionTriggered=nullptr;
            bool dummy=0;
            if (sensor->convexVolume->getSmallestDistanceEnabled())
                closeDetectionTriggered=&dummy;
            float _maxAngle=0.0f;
            if (angleLimitation)
                _maxAngle=maxAngle;
            bool result=CPluginContainer::geomPlugin_raySensorDetectMeshIfSmaller(lp,lvFar,shape->geomData->collInfo,shapeITr,dist,sensor->convexVolume->getSmallestDistanceAllowed(),!closestFeatureMode,frontFace,backFace,_maxAngle,&detectedPt,&triNormalNotNormalized,closeDetectionTriggered);
            if ((closeDetectionTriggered!=nullptr)&&closeDetectionTriggered[0])
                retVal=-2; // We triggered the sensor in the forbiden zone: to inform the calling routine, we do following:
            else
            {
                if (result)
                    retVal=shape->getObjectHandle();
            }
        }
        else
        { // non-ray-type sensors here:
            float _maxAngle=0.0f;
            if (angleLimitation)
                _maxAngle=maxAngle;
            if (CPluginContainer::geomPlugin_volumeSensorDetectMeshIfSmaller(sensor->convexVolume->planesInside,sensor->convexVolume->planesOutside,shape->geomData->collInfo,shapeITr,dist,!closestFeatureMode,frontFace,backFace,_maxAngle,&detectedPt,&triNormalNotNormalized))
            {
                if (sensor->convexVolume->getSmallestDistanceEnabled())
                {
                    if (dist<sensor->convexVolume->getSmallestDistanceAllowed())
                        retVal=-2;
                    else
                        retVal=shape->getObjectHandle();
                }
                else
                    retVal=shape->getObjectHandle();
            }
        }
    }

    return(retVal);
}

int CProxSensorRoutine::_detectOctree(CProxSensor* sensor,COctree* octree,C3Vector& detectedPt,float& dist,C3Vector& triNormalNotNormalized,bool closestFeatureMode,bool angleLimitation,float maxAngle,bool frontFace,bool backFace,float minThreshold)
{ // -2: sensor triggered in the forbidden zone, -1: sensor didn't trigger. Otherwise the object handle that triggered the sensor
    if (dist==0.0)
        return(-1);
    if (octree->getOctreeInfo()==nullptr)
        return(-1);
    if (!_doesSensorVolumeOverlapWithObjectBoundingBox(sensor,octree))
        return(-1);
    float d=_getApproxPointObjectBoundingBoxDistance(sensor->getFullCumulativeTransformation().X,octree);
    if (d>dist)
        return(-1);

    int retVal=-1;

    C7Vector sensTr(sensor->getFullCumulativeTransformation());
    C7Vector sensTrInv(sensTr.getInverse());
    C7Vector octreeITr(sensTrInv*octree->getFullCumulativeTransformation());

    float cosAngle=(float)cos(maxAngle);
    if (!angleLimitation)
        cosAngle=2.0f; // This means we don't want to check for a max angle!

    if (sensor->getRandomizedDetection())
    {
        int normalDetectionCnt=0;
        C3Vector averageDetectionVector;
        C3Vector averageNormalVector;
        averageDetectionVector.clear();
        averageNormalVector.clear();
        float averageDetectionDist=0.0f;
        const std::vector<C3Vector> normalizedRays=sensor->getPointerToRandomizedRays()[0];
        std::vector<float> individualRayDetectionState=sensor->getPointerToRandomizedRayDetectionStates()[0];
        int requiredDetectionCount=sensor->getRandomizedDetectionCountForDetection();
        for (size_t i=0;i<normalizedRays.size();i++)
        {
            C3Vector lp(normalizedRays[i]*sensor->convexVolume->getRadius()); // Here we have radius instead of offset! Special with randomized detection!!
            C3Vector lvFar(normalizedRays[i]*sensor->convexVolume->getRange());
            float distTmp=dist;
            C3Vector detectedPtTmp;
            C3Vector triNormalNotNormalizedTmp;
            float _maxAngle=0.0f;
            if (angleLimitation)
                _maxAngle=maxAngle;
            bool result=CPluginContainer::geomPlugin_raySensorDetectOctreeIfSmaller(lp,lvFar,octree->getOctreeInfo(),octreeITr,distTmp,0.0f,!closestFeatureMode,frontFace,backFace,_maxAngle,&detectedPtTmp,&triNormalNotNormalizedTmp,nullptr);
            if (result)
            { // We triggered the sensor
                if (sensor->convexVolume->getSmallestDistanceEnabled())
                {
                    if (distTmp<sensor->convexVolume->getSmallestDistanceAllowed())
                    {
                        normalDetectionCnt=0;
                        retVal=-2;
                        break;
                    }
                }
                normalDetectionCnt++;
                individualRayDetectionState[i]=distTmp;
                averageDetectionVector+=detectedPtTmp;
                averageDetectionDist+=distTmp;
                averageNormalVector+=triNormalNotNormalizedTmp.getNormalized();
            }
        }

        if (normalDetectionCnt>=requiredDetectionCount)
        {
            retVal=octree->getObjectHandle();
            dist=averageDetectionDist/float(normalDetectionCnt);
            detectedPt=averageDetectionVector/float(normalDetectionCnt);
            triNormalNotNormalized=averageNormalVector/float(normalDetectionCnt);
        }
    }
    else
    {
        if (sensor->getSensorType()==sim_proximitysensor_ray_subtype)
        { // ray-type sensor here:
            C3Vector lp(0.0f,0.0f,sensor->convexVolume->getOffset());
            C3Vector lvFar(0.0f,0.0f,sensor->convexVolume->getRange());

            float _maxAngle=0.0f;
            if (angleLimitation)
                _maxAngle=maxAngle;
            bool result=CPluginContainer::geomPlugin_raySensorDetectOctreeIfSmaller(lp,lvFar,octree->getOctreeInfo(),octreeITr,dist,0.0f,!closestFeatureMode,frontFace,backFace,_maxAngle,&detectedPt,&triNormalNotNormalized,nullptr);
            if (result)
            {
                if (sensor->convexVolume->getSmallestDistanceEnabled())
                {
                    if (dist<sensor->convexVolume->getSmallestDistanceAllowed())
                        retVal=-2;
                }
                if (retVal!=-2)
                    retVal=octree->getObjectHandle();
            }
        }
        else
        { // non-ray-type sensors here:

            float _maxAngle=0.0f;
            if (angleLimitation)
                _maxAngle=maxAngle;
            if ( CPluginContainer::geomPlugin_volumeSensorDetectOctreeIfSmaller(sensor->convexVolume->planesInside,sensor->convexVolume->planesOutside,octree->getOctreeInfo(),octreeITr,dist,!closestFeatureMode,frontFace,backFace,_maxAngle,&detectedPt,&triNormalNotNormalized) )
            {
                if (sensor->convexVolume->getSmallestDistanceEnabled())
                {
                    if (dist<sensor->convexVolume->getSmallestDistanceAllowed())
                        retVal=-2;
                    else
                        retVal=octree->getObjectHandle();
                }
                else
                    retVal=octree->getObjectHandle();
            }
        }
    }
    return(retVal);
}

int CProxSensorRoutine::_detectPointCloud(CProxSensor* sensor,CPointCloud* pointCloud,C3Vector& detectedPt,float& dist,C3Vector& triNormalNotNormalized,bool closestFeatureMode,bool angleLimitation,float maxAngle,bool frontFace,bool backFace,float minThreshold)
{ // -2: sensor triggered in the forbidden zone, -1: sensor didn't trigger. Otherwise the object handle that triggered the sensor
    if (dist==0.0)
        return(-1);
    if (pointCloud->getPointCloudInfo()==nullptr)
        return(-1);
    if (sensor->getRandomizedDetection())
        return(-1); // randomized detection doesn't work with points!
    if (sensor->getSensorType()==sim_proximitysensor_ray_subtype)
        return(-1); // ray-type proximity sensors don't work with points!

    if (!_doesSensorVolumeOverlapWithObjectBoundingBox(sensor,pointCloud))
        return(-1);
    float d=_getApproxPointObjectBoundingBoxDistance(sensor->getFullCumulativeTransformation().X,pointCloud);
    if (d>dist)
        return(-1);

    int retVal=-1;

    C7Vector sensTr(sensor->getFullCumulativeTransformation());
    C7Vector sensTrInv(sensTr.getInverse());
    C7Vector pointCloudITr(sensTrInv*pointCloud->getFullCumulativeTransformation());

    if (CPluginContainer::geomPlugin_volumeSensorDetectPtcloudIfSmaller(sensor->convexVolume->planesInside,sensor->convexVolume->planesOutside,pointCloud->getPointCloudInfo(),pointCloudITr,dist,!closestFeatureMode,&detectedPt))
    {
        bool doIt=true;
        if (sensor->convexVolume->getSmallestDistanceEnabled())
        {
            if (dist<sensor->convexVolume->getSmallestDistanceAllowed())
            {
                doIt=false;
                retVal=-2;
            }
        }
        if (doIt)
        {
            triNormalNotNormalized=detectedPt;
            retVal=pointCloud->getObjectHandle();
        }
    }

    return(retVal);
}

void CProxSensorRoutine::_orderGroupAccordingToApproxDistanceToSensingPoint(const CProxSensor* sensor,std::vector<CSceneObject*>& group)
{
    std::vector<float> distances;
    std::vector<int> indexes;
    std::vector<CSceneObject*> _group(group);
    group.clear();
    C3Vector pt(sensor->getFullCumulativeTransformation().X);
    for (size_t i=0;i<_group.size();i++)
    {
        indexes.push_back((int)i);
        float d=_getApproxPointObjectBoundingBoxDistance(pt,_group[i]);
        distances.push_back(d);
    }
    tt::orderAscending(distances,indexes);
    for (size_t i=0;i<indexes.size();i++)
        group.push_back(_group[indexes[i]]);
}

float CProxSensorRoutine::_getApproxPointObjectBoundingBoxDistance(const C3Vector& point,CSceneObject* obj)
{ // the returned distance is always same or smaller than the real distance!
    C3Vector halfSize;
    C7Vector tr;
    bool isPointToo=false;

    if (obj->getObjectType()==sim_object_shape_type)
    {
        halfSize=((CShape*)obj)->geomData->getBoundingBoxHalfSizes();
        tr=obj->getFullCumulativeTransformation();
    }
    if (obj->getObjectType()==sim_object_dummy_type)
    {
        isPointToo=true;
        tr=obj->getFullCumulativeTransformation();
    }
    if (obj->getObjectType()==sim_object_octree_type)
        ((COctree*)obj)->getTransfAndHalfSizeOfBoundingBox(tr,halfSize);
    if (obj->getObjectType()==sim_object_pointcloud_type)
        ((CPointCloud*)obj)->getTransfAndHalfSizeOfBoundingBox(tr,halfSize);
    if (isPointToo)
        return((tr.X-point).getLength()); // pt vs pt
    else
        return(CPluginContainer::geomPlugin_getBoxPointDistance(tr,halfSize,true,point,nullptr));
}

bool CProxSensorRoutine::_doesSensorVolumeOverlapWithObjectBoundingBox(CProxSensor* sensor,CSceneObject* obj)
{
    C3Vector sensorHalfSize;
    C7Vector sensorTr;
    sensor->getSensingVolumeOBB(sensorTr,sensorHalfSize);

    C3Vector objectHalfSize;
    C7Vector objectTr;
    bool objectIsPoint=false;

    if (obj->getObjectType()==sim_object_shape_type)
    {
        objectHalfSize=((CShape*)obj)->geomData->getBoundingBoxHalfSizes();
        objectTr=obj->getFullCumulativeTransformation();
    }
    if (obj->getObjectType()==sim_object_dummy_type)
    {
        objectIsPoint=true;
        objectTr=obj->getFullCumulativeTransformation();
    }
    if (obj->getObjectType()==sim_object_octree_type)
        ((COctree*)obj)->getTransfAndHalfSizeOfBoundingBox(objectTr,objectHalfSize);
    if (obj->getObjectType()==sim_object_pointcloud_type)
        ((CPointCloud*)obj)->getTransfAndHalfSizeOfBoundingBox(objectTr,objectHalfSize);
    if (objectIsPoint)
        return(CPluginContainer::geomPlugin_getBoxPointCollision(sensorTr,sensorHalfSize,objectTr.X));
    else
        return(CPluginContainer::geomPlugin_getBoxBoxCollision(sensorTr,sensorHalfSize,objectTr,objectHalfSize,true));
}

int CProxSensorRoutine::_detectObject(CProxSensor* sensor,CSceneObject* object,C3Vector& detectedPt,float& dist,C3Vector& triNormalNotNormalized,bool closestFeatureMode,bool angleLimitation,float maxAngle,bool frontFace,bool backFace,float minThreshold)
{
    int retVal=-1;
    if (object->getObjectType()==sim_object_dummy_type)
        retVal=_detectDummy(sensor,(CDummy*)object,detectedPt,dist,triNormalNotNormalized,closestFeatureMode,angleLimitation,maxAngle,frontFace,backFace,minThreshold);
    if (object->getObjectType()==sim_object_shape_type)
        retVal=_detectShape(sensor,(CShape*)object,detectedPt,dist,triNormalNotNormalized,closestFeatureMode,angleLimitation,maxAngle,frontFace,backFace,minThreshold);
    if (object->getObjectType()==sim_object_octree_type)
        retVal=_detectOctree(sensor,(COctree*)object,detectedPt,dist,triNormalNotNormalized,closestFeatureMode,angleLimitation,maxAngle,frontFace,backFace,minThreshold);
    if (object->getObjectType()==sim_object_pointcloud_type)
        retVal=_detectPointCloud(sensor,(CPointCloud*)object,detectedPt,dist,triNormalNotNormalized,closestFeatureMode,angleLimitation,maxAngle,frontFace,backFace,minThreshold);
    return(retVal);
}

