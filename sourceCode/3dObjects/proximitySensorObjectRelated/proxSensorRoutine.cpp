
#include "vrepMainHeader.h"
#include "proxSensorRoutine.h"
#include "pluginContainer.h"
#include "app.h"
#include "tt.h"

SOcclusionCheckDat CProxSensorRoutine::_occlusionCheckDat;

bool CProxSensorRoutine::detectEntity(int sensorID,int entityID,bool closestFeatureMode,bool angleLimitation,float maxAngle,C3Vector& detectedPt,float& dist,bool frontFace,bool backFace,int& detectedObject,float minThreshold,C3Vector& triNormal,bool overrideDetectableFlagIfNonCollection,bool checkOcclusions)
{ // entityID==-1 --> checks all objects in the scene
    bool returnValue=false;
    detectedObject=-1;
    CProxSensor* sensor=App::ct->objCont->getProximitySensor(sensorID);
    C3DObject* object=App::ct->objCont->getObjectFromHandle(entityID);
    if (sensor==nullptr)
        return(false); // should never happen!
    App::ct->calcInfo->proximitySensorSimulationStart();
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
            std::vector<C3DObject*> objsToDetect;
            objsToDetect.push_back(object);
            OCCLUSION_CHECK_CALLBACK occlusionCheckCallback=_prepareOcclusionCheck(sensor,objsToDetect,frontFace,backFace,checkOcclusions);
            int detectObjId=_detectObject(sensor,object,detectedPt,dist,triNormal,closestFeatureMode,angleLimitation,maxAngle,frontFace,backFace,minThreshold,occlusionCheckCallback);
            returnValue=(detectObjId>=0);
            if (detectObjId>=0)
                detectedObject=entityID;
            _cleanUpOcclusionCheck();
        }
    }
    else
    {
        std::vector<C3DObject*> group;
        if (entityID==-1)
        { // Special group here (all detectable objects):
            std::vector<C3DObject*> exception;
            App::ct->objCont->getAllDetectableObjectsFromSceneExcept(&exception,group,sensor->getSensableType());
        }
        else
        { // Regular group here:
            App::ct->collections->getDetectableObjectsFromCollection(entityID,group,sensor->getSensableType());
        }
        if (group.size()!=0)
        {
            _orderGroupAccordingToApproxDistanceToSensingPoint(sensor,group);
            OCCLUSION_CHECK_CALLBACK occlusionCheckCallback=_prepareOcclusionCheck(sensor,group,frontFace,backFace,checkOcclusions);
            for (size_t i=0;i<group.size();i++)
            {
                int detectObjId=_detectObject(sensor,group[i],detectedPt,dist,triNormal,closestFeatureMode,angleLimitation,maxAngle,frontFace,backFace,minThreshold,occlusionCheckCallback);
                returnValue|=(detectObjId>=0);
                if (detectObjId>=0)
                    detectedObject=group[i]->getObjectHandle();
                if (detectObjId==-2)
                {
                    returnValue=false;
                    break; // we detected something in the forbidden zone
                }
            }
            _cleanUpOcclusionCheck();
        }
    }

    if (returnValue)
        triNormal.normalize();
    App::ct->calcInfo->proximitySensorSimulationEnd(returnValue);
    return(returnValue);
}

bool CProxSensorRoutine::detectPrimitive(int sensorID,float* vertexPointer,int itemType,int itemCount,
        bool closestFeatureMode,bool angleLimitation,float maxAngle,C3Vector& detectedPt,
        float& dist,bool frontFace,bool backFace,float minThreshold,C3Vector& triNormal)
{
    bool returnValue=false;
    CProxSensor* sens=App::ct->objCont->getProximitySensor(sensorID);
    if (sens==nullptr)
        return(false); // should never happen!

    if (sens->getRandomizedDetection())
    {
        if (sens->getSensorType()!=sim_proximitysensor_ray_subtype)
            return(false); // probably not needed
        sens->calculateFreshRandomizedRays();
    }
    App::ct->calcInfo->proximitySensorSimulationStart();


    // Now prepare for detection
    float cosAngle=(float)cos(maxAngle);
    if (!angleLimitation)
        cosAngle=2.0f; // This means we don't want to check for a max angle!
    C7Vector sensInv(sens->getCumulativeTransformationPart1().getInverse());

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
                    if (CPluginContainer::mesh_getProxSensorDistanceToTriangle_ifSmaller(a0,e0,e1,dist,&sens->convexVolume->planesInside,&sens->convexVolume->planesOutside,cosAngle,detectedPt,frontFace,backFace,triNormal))
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
                        detect=CPluginContainer::mesh_isPointInsideVolume1AndOutsideVolume2(p0,&sens->convexVolume->planesInside,&sens->convexVolume->planesOutside);
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
                    detect=CPluginContainer::mesh_getProxSensorDistanceToSegment_ifSmaller(p0,p1,dist,&sens->convexVolume->planesInside,&sens->convexVolume->planesOutside,cosAngle,detectedPt);
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
                detect=CPluginContainer::mesh_getProxSensorDistanceToTriangle_ifSmaller(a0,e0,e1,dist,&sens->convexVolume->planesInside,&sens->convexVolume->planesOutside,cosAngle,detectedPt,frontFace,backFace,triNormal);
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
        
    App::ct->calcInfo->proximitySensorSimulationEnd(returnValue);
    return(returnValue);
}

int CProxSensorRoutine::_detectDummy(CProxSensor* sensor,CDummy* dummy,C3Vector& detectedPt,float& dist,C3Vector& triNormalNotNormalized,bool closestFeatureMode,bool angleLimitation,float maxAngle,bool frontFace,bool backFace,float minThreshold,OCCLUSION_CHECK_CALLBACK occlusionCheckCallback)
{ // -2: sensor triggered in the forbidden zone, -1: sensor didn't trigger. Otherwise the object handle that triggered the sensor
    if (dist==0.0)
        return(-1);
    if (sensor->getSensorType()==sim_proximitysensor_ray_subtype)
        return(-1); // ray sensors never detect dummies
    if (sensor->getRandomizedDetection())
        return(-1); // probably not needed
    if (!_doesSensorVolumeOverlapWithObjectBoundingBox(sensor,dummy))
        return(-1);
    float d=_getApproxPointObjectBoundingBoxDistance(sensor->getCumulativeTransformation().X,dummy);
    if (d>dist)
        return(-1);

    C7Vector inv(sensor->getCumulativeTransformation().getInverse());
    C4X4Matrix dummyCTM((inv*dummy->getCumulativeTransformation()).getMatrix());
    float theDistance=dummyCTM.X.getLength();
    if (CPluginContainer::mesh_isPointInsideVolume1AndOutsideVolume2(dummyCTM.X,&sensor->convexVolume->planesInside,&sensor->convexVolume->planesOutside) )
    {
        if (theDistance<dist)
        {
            bool doIt=true;
            if (occlusionCheckCallback!=nullptr)
                doIt=!occlusionCheckCallback(dummyCTM.X.data);
            if (doIt)
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
    }
    return(-1);
}

int CProxSensorRoutine::_detectShape(CProxSensor* sensor,CShape* shape,C3Vector& detectedPt,float& dist,C3Vector& triNormalNotNormalized,bool closestFeatureMode,bool angleLimitation,float maxAngle,bool frontFace,bool backFace,float minThreshold,OCCLUSION_CHECK_CALLBACK occlusionCheckCallback)
{ // -2: sensor triggered in the forbidden zone, -1: sensor didn't trigger. Otherwise the object handle that triggered the sensor
    if (dist==0.0)
        return(-1);
    if (!_doesSensorVolumeOverlapWithObjectBoundingBox(sensor,shape))
        return(-1);
    float d=_getApproxPointObjectBoundingBoxDistance(sensor->getCumulativeTransformation().X,shape);
    if (d>dist)
        return(-1);

    shape->initializeCalculationStructureIfNeeded();

    int retVal=-1;

    C7Vector inv(sensor->getCumulativeTransformation().getInverse());
    C4X4Matrix shapeRTM((inv*shape->getCumulativeTransformation()).getMatrix());

    float cosAngle=(float)cos(maxAngle);
    if (!angleLimitation)
        cosAngle=2.0f; // This means we don't want to check for a max angle!

    if (sensor->getRandomizedDetection())
    {
        char* closeDetectionTriggered=nullptr;
        char dummyVal=0;
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
            bool result=CPluginContainer::mesh_getRayProxSensorDistance_ifSmaller(
                            shape->geomData->collInfo,(inv*shape->getCumulativeTransformation()).getMatrix(),distTmp,lp,sensor->convexVolume->getSmallestDistanceAllowed(),lvFar,cosAngle,
                            detectedPtTmp,!closestFeatureMode,frontFace,backFace,closeDetectionTriggered,triNormalNotNormalizedTmp,(void*)occlusionCheckCallback);
            if ((closeDetectionTriggered!=nullptr)&&(closeDetectionTriggered[0]!=0))
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
            char* closeDetectionTriggered=nullptr;
            char dummy=0;
            if (sensor->convexVolume->getSmallestDistanceEnabled())
                closeDetectionTriggered=&dummy;

            bool result=CPluginContainer::mesh_getRayProxSensorDistance_ifSmaller(
                    shape->geomData->collInfo,shapeRTM,dist,lp,sensor->convexVolume->getSmallestDistanceAllowed(),lvFar,cosAngle,
                    detectedPt,!closestFeatureMode,frontFace,backFace,closeDetectionTriggered,triNormalNotNormalized,(void*)occlusionCheckCallback);

            if ((closeDetectionTriggered!=nullptr)&&(closeDetectionTriggered[0]!=0))
                retVal=-2; // We triggered the sensor in the forbiden zone: to inform the calling routine, we do following:
            else
            {
                if (result)
                    retVal=shape->getObjectHandle();
            }
        }
        else
        { // non-ray-type sensors here:
            if ( CPluginContainer::mesh_getProxSensorDistance_ifSmaller(
                    shape->geomData->collInfo,shapeRTM,dist,&sensor->convexVolume->planesInside,
                    &sensor->convexVolume->planesOutside,cosAngle,
                    detectedPt,!closestFeatureMode,frontFace,backFace,nullptr,triNormalNotNormalized,(void*)occlusionCheckCallback) )
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

int CProxSensorRoutine::_detectOctree(CProxSensor* sensor,COctree* octree,C3Vector& detectedPt,float& dist,C3Vector& triNormalNotNormalized,bool closestFeatureMode,bool angleLimitation,float maxAngle,bool frontFace,bool backFace,float minThreshold,OCCLUSION_CHECK_CALLBACK occlusionCheckCallback)
{ // -2: sensor triggered in the forbidden zone, -1: sensor didn't trigger. Otherwise the object handle that triggered the sensor
    if (dist==0.0)
        return(-1);
    if (octree->getOctreeInfo()==nullptr)
        return(-1);
    if (!_doesSensorVolumeOverlapWithObjectBoundingBox(sensor,octree))
        return(-1);
    float d=_getApproxPointObjectBoundingBoxDistance(sensor->getCumulativeTransformation().X,octree);
    if (d>dist)
        return(-1);

    int retVal=-1;

    C7Vector sensTr(sensor->getCumulativeTransformation());
    C7Vector sensTrInv(sensTr.getInverse());
    C4X4Matrix octreeRTM((sensTrInv*octree->getCumulativeTransformation()).getMatrix());

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
            bool result=CPluginContainer::mesh_getRayProxSensorOctreeDistanceIfSmaller(octree->getOctreeInfo(),octreeRTM,distTmp,lp,lvFar,cosAngle,detectedPtTmp,!closestFeatureMode,frontFace,backFace,triNormalNotNormalizedTmp,(void*)occlusionCheckCallback);
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

            bool result=CPluginContainer::mesh_getRayProxSensorOctreeDistanceIfSmaller(octree->getOctreeInfo(),octreeRTM,dist,lp,lvFar,cosAngle,detectedPt,!closestFeatureMode,frontFace,backFace,triNormalNotNormalized,(void*)occlusionCheckCallback);
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

            if ( CPluginContainer::mesh_getProxSensorOctreeDistanceIfSmaller(octree->getOctreeInfo(),octreeRTM,dist,&sensor->convexVolume->planesInside,&sensor->convexVolume->planesOutside,cosAngle,detectedPt,!closestFeatureMode,frontFace,backFace,triNormalNotNormalized,(void*)occlusionCheckCallback) )
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

int CProxSensorRoutine::_detectPointCloud(CProxSensor* sensor,CPointCloud* pointCloud,C3Vector& detectedPt,float& dist,C3Vector& triNormalNotNormalized,bool closestFeatureMode,bool angleLimitation,float maxAngle,bool frontFace,bool backFace,float minThreshold,OCCLUSION_CHECK_CALLBACK occlusionCheckCallback)
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
    float d=_getApproxPointObjectBoundingBoxDistance(sensor->getCumulativeTransformation().X,pointCloud);
    if (d>dist)
        return(-1);

    int retVal=-1;

    C7Vector sensTr(sensor->getCumulativeTransformation());
    C7Vector sensTrInv(sensTr.getInverse());
    C4X4Matrix pointCloudRTM((sensTrInv*pointCloud->getCumulativeTransformation()).getMatrix());

    if (CPluginContainer::mesh_getProxSensorPointCloudDistanceIfSmaller(pointCloud->getPointCloudInfo(),pointCloudRTM,dist,&sensor->convexVolume->planesInside,&sensor->convexVolume->planesOutside,detectedPt,!closestFeatureMode,(void*)occlusionCheckCallback))
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

void CProxSensorRoutine::_orderGroupAccordingToApproxDistanceToSensingPoint(const CProxSensor* sensor,std::vector<C3DObject*>& group)
{
    std::vector<float> distances;
    std::vector<int> indexes;
    std::vector<C3DObject*> _group(group);
    group.clear();
    C3Vector pt(sensor->getCumulativeTransformation().X);
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

float CProxSensorRoutine::_getApproxPointObjectBoundingBoxDistance(const C3Vector& point,C3DObject* obj)
{ // the returned distance is always same or smaller than the real distance!
    C3Vector halfSize;
    C4X4Matrix m;
    bool isPointToo=false;

    if (obj->getObjectType()==sim_object_shape_type)
    {
        halfSize=((CShape*)obj)->geomData->getBoundingBoxHalfSizes();
        m=obj->getCumulativeTransformation().getMatrix();
    }
    if (obj->getObjectType()==sim_object_dummy_type)
    {
        isPointToo=true;
        m=obj->getCumulativeTransformation().getMatrix();
    }
    if (obj->getObjectType()==sim_object_octree_type)
        ((COctree*)obj)->getMatrixAndHalfSizeOfBoundingBox(m,halfSize);
    if (obj->getObjectType()==sim_object_pointcloud_type)
        ((CPointCloud*)obj)->getMatrixAndHalfSizeOfBoundingBox(m,halfSize);

    if (isPointToo)
        return((m.X-point).getLength()); // pt vs pt
    else
        return(CPluginContainer::mesh_getBoxPointDistance(m,halfSize,point)); // pt vs box
}

bool CProxSensorRoutine::_doesSensorVolumeOverlapWithObjectBoundingBox(CProxSensor* sensor,C3DObject* obj)
{
    C3Vector sensorHalfSize;
    C4X4Matrix sensorM;
    sensor->getSensingVolumeOBB(sensorM,sensorHalfSize);

    C3Vector objectHalfSize;
    C4X4Matrix objectM;
    bool objectIsPoint=false;

    if (obj->getObjectType()==sim_object_shape_type)
    {
        objectHalfSize=((CShape*)obj)->geomData->getBoundingBoxHalfSizes();
        objectM=obj->getCumulativeTransformation().getMatrix();
    }
    if (obj->getObjectType()==sim_object_dummy_type)
    {
        objectIsPoint=true;
        objectM=obj->getCumulativeTransformation().getMatrix();
    }
    if (obj->getObjectType()==sim_object_octree_type)
        ((COctree*)obj)->getMatrixAndHalfSizeOfBoundingBox(objectM,objectHalfSize);
    if (obj->getObjectType()==sim_object_pointcloud_type)
        ((CPointCloud*)obj)->getMatrixAndHalfSizeOfBoundingBox(objectM,objectHalfSize);


    if (objectIsPoint)
        return(CPluginContainer::mesh_getBoxPointCollision(sensorM,sensorHalfSize,objectM.X));
    else
        return(CPluginContainer::mesh_getBoxBoxCollision(sensorM,sensorHalfSize,objectM,objectHalfSize));
}

int CProxSensorRoutine::_detectObject(CProxSensor* sensor,C3DObject* object,C3Vector& detectedPt,float& dist,C3Vector& triNormalNotNormalized,bool closestFeatureMode,bool angleLimitation,float maxAngle,bool frontFace,bool backFace,float minThreshold,OCCLUSION_CHECK_CALLBACK occlusionCheckCallback)
{
    int retVal=-1;
    if (object->getObjectType()==sim_object_dummy_type)
        retVal=_detectDummy(sensor,(CDummy*)object,detectedPt,dist,triNormalNotNormalized,closestFeatureMode,angleLimitation,maxAngle,frontFace,backFace,minThreshold,occlusionCheckCallback);
    if (object->getObjectType()==sim_object_shape_type)
        retVal=_detectShape(sensor,(CShape*)object,detectedPt,dist,triNormalNotNormalized,closestFeatureMode,angleLimitation,maxAngle,frontFace,backFace,minThreshold,occlusionCheckCallback);
    if (object->getObjectType()==sim_object_octree_type)
        retVal=_detectOctree(sensor,(COctree*)object,detectedPt,dist,triNormalNotNormalized,closestFeatureMode,angleLimitation,maxAngle,frontFace,backFace,minThreshold,occlusionCheckCallback);
    if (object->getObjectType()==sim_object_pointcloud_type)
        retVal=_detectPointCloud(sensor,(CPointCloud*)object,detectedPt,dist,triNormalNotNormalized,closestFeatureMode,angleLimitation,maxAngle,frontFace,backFace,minThreshold,occlusionCheckCallback);
    return(retVal);
}

OCCLUSION_CHECK_CALLBACK CProxSensorRoutine::_prepareOcclusionCheck(const CProxSensor* sensor,const std::vector<C3DObject*>& objectsToDetect,bool frontFace,bool backFace,bool checkOcclusions)
{
    if ( (!checkOcclusions)||(objectsToDetect.size()==0) )
        return(nullptr);
    _occlusionCheckDat.occlusionCheck=true;

    for (size_t i=0;i<objectsToDetect.size();i++)
    {
        C7Vector inv(sensor->getCumulativeTransformation().getInverse());
        C4X4Matrix objectRTM((inv*objectsToDetect[i]->getCumulativeTransformation()).getMatrix());

        _occlusionCheckDat.objects.push_back(objectsToDetect[i]);
        _occlusionCheckDat.objectRelToSensorM.push_back(objectRTM);
        _occlusionCheckDat.sensorClose=0.0;

        if ( (sensor->getSensorType()==sim_proximitysensor_cone_subtype)||(sensor->getSensorType()==sim_proximitysensor_disc_subtype) )
            _occlusionCheckDat.sensorClose=sensor->convexVolume->getRadius();
        else
        {
            if ( (sensor->getSensorType()==sim_proximitysensor_ray_subtype)&&sensor->getRandomizedDetection() )
                _occlusionCheckDat.sensorClose=sensor->convexVolume->getRadius(); // Special!!
            else
                _occlusionCheckDat.sensorClose=sensor->convexVolume->getOffset();
        }
        _occlusionCheckDat.frontFace=frontFace;
        _occlusionCheckDat.backFace=backFace;
    }
    return(CProxSensorRoutine::_checkForOcclusion);
}

void CProxSensorRoutine::_cleanUpOcclusionCheck()
{
    _occlusionCheckDat.occlusionCheck=false;
    _occlusionCheckDat.objects.clear();
    _occlusionCheckDat.objectRelToSensorM.clear();
}

bool CProxSensorRoutine::_checkForOcclusion(const float _detectedPt[])
{ // True means there are occlusions
    bool retVal=false;
    if (!_occlusionCheckDat.occlusionCheck)
        return(false);
    C3Vector detectedPt(_detectedPt);
    _occlusionCheckDat.occlusionCheck=false; // Disable the flag temporarily, otherwise this routine might call itself infinitely!
    C3Vector lp(0.0f,0.0f,0.0f);
    C3Vector lvFar(detectedPt);
    float l=detectedPt.getLength();
    if (l!=0.0f)
        lp=detectedPt*(_occlusionCheckDat.sensorClose/l);
    lvFar-=lp;

    C3Vector triNormalNotNormalized;
    C3Vector detectedPtTmp;

    for (size_t j=0;j<_occlusionCheckDat.objects.size();j++)
    { // only surface-type objects can potentially be an occlusion
        float dist=l;
        bool result=false;
        if (_occlusionCheckDat.objects[j]->getObjectType()==sim_object_shape_type)
        {
            ((CShape*)_occlusionCheckDat.objects[j])->geomData->initializeCalculationStructureIfNeeded();
            result=CPluginContainer::mesh_getRayProxSensorDistance_ifSmaller(((CShape*)_occlusionCheckDat.objects[j])->geomData->collInfo,_occlusionCheckDat.objectRelToSensorM[j],dist,lp,0.0f,lvFar,2.0f,detectedPtTmp,false,_occlusionCheckDat.frontFace,_occlusionCheckDat.backFace,nullptr,triNormalNotNormalized,nullptr);
        }
        if (_occlusionCheckDat.objects[j]->getObjectType()==sim_object_octree_type)
        {
            if (((COctree*)_occlusionCheckDat.objects[j])->getOctreeInfo()!=nullptr)
                result=CPluginContainer::mesh_getRayProxSensorOctreeDistanceIfSmaller(((COctree*)_occlusionCheckDat.objects[j])->getOctreeInfo(),_occlusionCheckDat.objectRelToSensorM[j],dist,lp,lvFar,2.0f,detectedPtTmp,false,_occlusionCheckDat.frontFace,_occlusionCheckDat.backFace,triNormalNotNormalized,nullptr);
        }
        if (result)
        {
            if (dist<l*0.999f)
            {
                retVal=true;
                break;
            }
        }
    }
    _occlusionCheckDat.occlusionCheck=true; // Restore the flag before leaving

    return(retVal);
}
