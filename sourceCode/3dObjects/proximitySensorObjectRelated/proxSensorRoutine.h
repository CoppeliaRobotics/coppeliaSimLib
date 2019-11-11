#pragma once

#include "shape.h"
#include "proximitySensor.h"
#include "octree.h"
#include "pointCloud.h"


//FULLY STATIC CLASS
class CProxSensorRoutine  
{
public:
    // The main general routine:
    static bool detectEntity(int sensorID,int entityID,bool closestFeatureMode,bool angleLimitation,float maxAngle,C3Vector& detectedPt,float& dist,bool frontFace,bool backFace,int& detectedObject,float minThreshold,C3Vector& triNormal,bool overrideDetectableFlagIfNonCollection);

    static bool detectPrimitive(int sensorID,float* vertexPointer,int itemType,int itemCount,bool closestFeatureMode,bool angleLimitation,float maxAngle,C3Vector& detectedPt,float& dist,bool frontFace,bool backFace,float minThreshold,C3Vector& triNormal);


private:

    static int _detectDummy(CProxSensor* sensor,CDummy* dummy,C3Vector& detectedPt,float& dist,C3Vector& triNormalNotNormalized,bool closestFeatureMode,bool angleLimitation,float maxAngle,bool frontFace,bool backFace,float minThreshold);
    static int _detectShape(CProxSensor* sensor,CShape* shape,C3Vector& detectedPt,float& dist,C3Vector& triNormalNotNormalized,bool closestFeatureMode,bool angleLimitation,float maxAngle,bool frontFace,bool backFace,float minThreshold);
    static int _detectOctree(CProxSensor* sensor,COctree* octree,C3Vector& detectedPt,float& dist,C3Vector& triNormalNotNormalized,bool closestFeatureMode,bool angleLimitation,float maxAngle,bool frontFace,bool backFace,float minThreshold);
    static int _detectPointCloud(CProxSensor* sensor,CPointCloud* pointCloud,C3Vector& detectedPt,float& dist,C3Vector& triNormalNotNormalized,bool closestFeatureMode,bool angleLimitation,float maxAngle,bool frontFace,bool backFace,float minThreshold);
    static int _detectObject(CProxSensor* sensor,C3DObject* object,C3Vector& detectedPt,float& dist,C3Vector& triNormalNotNormalized,bool closestFeatureMode,bool angleLimitation,float maxAngle,bool frontFace,bool backFace,float minThreshold);

    static void _orderGroupAccordingToApproxDistanceToSensingPoint(const CProxSensor* sensor,std::vector<C3DObject*>& group);
    static float _getApproxPointObjectBoundingBoxDistance(const C3Vector& point,C3DObject* obj);
    static bool _doesSensorVolumeOverlapWithObjectBoundingBox(CProxSensor* sensor,C3DObject* obj);
};
