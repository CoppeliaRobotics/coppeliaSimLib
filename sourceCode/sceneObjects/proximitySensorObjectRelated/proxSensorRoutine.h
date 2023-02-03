#pragma once

#include <shape.h>
#include <proximitySensor.h>
#include <octree.h>
#include <pointCloud.h>


//FULLY STATIC CLASS
class CProxSensorRoutine  
{
public:
    // The main general routine:
    static bool detectEntity(int sensorID,int entityID,bool closestFeatureMode,bool angleLimitation,double maxAngle,C3Vector& detectedPt,double& dist,bool frontFace,bool backFace,int& detectedObject,double minThreshold,C3Vector& triNormal,bool overrideDetectableFlagIfNonCollection);

    static bool detectPrimitive(int sensorID,double* vertexPointer,int itemType,int itemCount,bool closestFeatureMode,bool angleLimitation,double maxAngle,C3Vector& detectedPt,double& dist,bool frontFace,bool backFace,double minThreshold,C3Vector& triNormal);


private:

    static int _detectDummy(CProxSensor* sensor,CDummy* dummy,C3Vector& detectedPt,double& dist,C3Vector& triNormalNotNormalized,bool closestFeatureMode,bool angleLimitation,double maxAngle,bool frontFace,bool backFace,double minThreshold);
    static int _detectShape(CProxSensor* sensor,CShape* shape,C3Vector& detectedPt,double& dist,C3Vector& triNormalNotNormalized,bool closestFeatureMode,bool angleLimitation,double maxAngle,bool frontFace,bool backFace,double minThreshold);
    static int _detectOctree(CProxSensor* sensor,COctree* octree,C3Vector& detectedPt,double& dist,C3Vector& triNormalNotNormalized,bool closestFeatureMode,bool angleLimitation,double maxAngle,bool frontFace,bool backFace,double minThreshold);
    static int _detectPointCloud(CProxSensor* sensor,CPointCloud* pointCloud,C3Vector& detectedPt,double& dist,C3Vector& triNormalNotNormalized,bool closestFeatureMode,bool angleLimitation,double maxAngle,bool frontFace,bool backFace,double minThreshold);
    static int _detectObject(CProxSensor* sensor,CSceneObject* object,C3Vector& detectedPt,double& dist,C3Vector& triNormalNotNormalized,bool closestFeatureMode,bool angleLimitation,double maxAngle,bool frontFace,bool backFace,double minThreshold);

    static void _orderGroupAccordingToApproxDistanceToSensingPoint(const CProxSensor* sensor,std::vector<CSceneObject*>& group);
    static double _getApproxPointObjectBoundingBoxDistance(const C3Vector& point,CSceneObject* obj);
    static bool _doesSensorVolumeOverlapWithObjectBoundingBox(CProxSensor* sensor,CSceneObject* obj);
};
