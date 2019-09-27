
#pragma once

#include "dummyClasses.h"
#include "vrepConfig.h"

typedef void* (*ptr_createNonHolonomicPathPlanningObject)(int theStartDummyID,int theGoalDummyID,
                            int theRobotCollectionID,int theObstacleCollectionID,float theAngularCoeff,
                            float theSteeringAngleCoeff,float theMaxSteeringAngleVariation,float theMaxSteeringAngle,
                            float theStepSize,const float theSearchMinVal[2],const float theSearchRange[2],
                            const int theDirectionConstraints[2],const float clearanceAndMaxDistance[2]);
typedef void* (*ptr_createHolonomicPathPlanningObject)(int theStartDummyID,int theGoalDummyID,
                            int theRobotCollectionID,int theObstacleCollectionID,int thePlanningType,
                            float theAngularCoeff,float theStepSize,const float theSearchMinVal[4],const float theSearchRange[4],
                            const int theDirectionConstraints[4],const float clearanceAndMaxDistance[2],const float gammaAxis[3]);
typedef void (*ptr_destroyPathPlanningObject)(CDummyPathPlanning* obj);
typedef int (*ptr_searchPath)(CDummyPathPlanning* obj,int maxTimeInMsPerPass);
typedef int (*ptr_getPathNodeCount)(CDummyPathPlanning* obj,char fromStart);
typedef char (*ptr_setPartialPath)(CDummyPathPlanning* obj);
typedef int (*ptr_smoothFoundPath)(CDummyPathPlanning* obj,int steps,int maxTimePerPass);
typedef float* (*ptr_getFoundPath)(CDummyPathPlanning* obj,int* nodeCount);
typedef float* (*ptr_getSearchTree)(CDummyPathPlanning* obj,int* segmentCount,char fromStart);
typedef void (*ptr_releaseBuffer)(void* buff);

typedef CDummyMpObject* (*ptr_createMpObject)();
typedef void (*ptr_destroyMpObject)(CDummyMpObject* mpObject);
typedef void (*ptr_setMpObjectData)(CDummyMpObject* mpObject,int jointCnt,const int* jointHandles,const int* jointStepCount,int ikGroupId,int baseFrameId,int tipFrameId,int robotSelfCollEntity1,int robotSelfCollEntity2,int robotEntity,int obstacleEntity,float distanceThreshold,const float* tipMetricAndIkElConstraints,const float* robotMetric);
typedef char (*ptr_calculateMpNodesPhase1)(CDummyMpObject* mpObject,const char* serializationData,int serializationDataSize);
typedef int (*ptr_getMpPhase1NodesRenderData)(CDummyMpObject* mpObject,int index,float** pos);
typedef int (*ptr_getMpPhase2NodesRenderData)(CDummyMpObject* mpObject,unsigned char what,int index,float** pos1,float** pos2);
typedef char* (*ptr_getMpSerializationData)(CDummyMpObject* mpObject,int* cnt);
typedef int (*ptr_getMpPhase1NodeCnt)(CDummyMpObject* mpObject,char collidingNodesOnly);
typedef int (*ptr_getMpRobotConfigFromTipPose)(CDummyMpObject* mpObject,const float* tipPos,const float* tipQuat,int options,float* robotJoints,const float* constraints,const float* configConstraints,int trialCount,float tipCloseDistance,const float* referenceConfigs,int configCount,const int* jointBehaviour,int correctionPasses,int maxTimeInMs);
typedef float* (*ptr_mpFindPath)(CDummyMpObject* mpObject,const float* startConfig,const float* goalConfig,int options,float stepSize,int* outputConfigsCnt,int maxTimeInMs,const int* auxIntParams,const float* auxFloatParams,int randomSeed);
typedef float* (*ptr_mpFindIkPath)(CDummyMpObject* mpObject,const float* startConfig,const float* goalPos,const float* goalQuat,int options,float stepSize,int* outputConfigsCnt,const int* auxIntParams,const float* auxFloatParams);
typedef float* (*ptr_mpSimplifyPath)(CDummyMpObject* mpObject,const float* pathBuffer,int configCnt,int options,float stepSize,int increment,int* outputConfigsCnt,int maxTimeInMs,const int* auxIntParams,const float* auxFloatParams);
typedef float* (*ptr_mpGetConfigTransition)(CDummyMpObject* mpObject,const float* startConfig,const float* goalConfig,int options,const int* select,float calcStepSize,float maxOutStepSize,int wayPointCnt,const float* wayPoints,int* outputConfigsCnt,const int* auxIntParams,const float* auxFloatParams);

// FULLY STATIC CLASS
class CPathPlanningInterface  
{
public:
    static bool initializeFunctionsIfNeeded();
    static void setSystemPluginsLoadPhaseOver();
    static CDummyNonHolonomicPathPlanning* createNonHolonomicPathPlanningObject(int theStartDummyID,int theGoalDummyID,
                            int theRobotCollectionID,int theObstacleCollectionID,float theAngularCoeff,
                            float theSteeringAngleCoeff,float theMaxSteeringAngleVariation,float theMaxSteeringAngle,
                            float theStepSize,const float theSearchMinVal[2],const float theSearchRange[2],
                            const int theDirectionConstraints[2],const float clearanceAndMaxDistance[2]);

    static CDummyHolonomicPathPlanning* createHolonomicPathPlanningObject(int theStartDummyID,int theGoalDummyID,
                            int theRobotCollectionID,int theObstacleCollectionID,int thePlanningType,
                            float theAngularCoeff,float theStepSize,const float theSearchMinVal[4],const float theSearchRange[4],
                            const int theDirectionConstraints[4],const float clearanceAndMaxDistance[2],const float gammaAxis[3]);
    static void destroyPathPlanningObject(CDummyPathPlanning* obj);
    static int searchPath(CDummyPathPlanning* obj,int maxTimeInMsPerPass);
    static int getPathNodeCount(CDummyPathPlanning* obj,char fromStart);
    static char setPartialPath(CDummyPathPlanning* obj);
    static int smoothFoundPath(CDummyPathPlanning* obj,int steps,int maxTimePerPass);
    static float* getFoundPath(CDummyPathPlanning* obj,int* nodeCount);
    static float* getSearchTree(CDummyPathPlanning* obj,int* segmentCount,char fromStart);
    static void releaseBuffer(void* buff);

    static CDummyMpObject* createMpObject();
    static void destroyMpObject(CDummyMpObject* mpObject);
    static void setMpObjectData(CDummyMpObject* mpObject,int jointCnt,const int* jointHandles,const int* jointStepCount,int ikGroupId,int baseFrameId,int tipFrameId,int robotSelfCollEntity1,int robotSelfCollEntity2,int robotEntity,int obstacleEntity,float distanceThreshold,const float* tipMetricAndIkElConstraints,const float* robotMetric);
    static char calculateMpNodesPhase1(CDummyMpObject* mpObject,const char* serializationData,int serializationDataSize);
    static int getMpPhase1NodesRenderData(CDummyMpObject* mpObject,int index,float** pos);
    static int getMpPhase2NodesRenderData(CDummyMpObject* mpObject,unsigned char what,int index,float** pos1,float** pos2);
    static char* getMpSerializationData(CDummyMpObject* mpObject,int* cnt);
    static int getMpPhase1NodeCnt(CDummyMpObject* mpObject,char collidingNodesOnly);
    static int getMpRobotConfigFromTipPose(CDummyMpObject* mpObject,const float* tipPos,const float* tipQuat,int options,float* robotJoints,const float* constraints,const float* configConstraints,int trialCount,float tipCloseDistance,const float* referenceConfigs,int configCount,const int* jointBehaviour,int correctionPasses,int maxTimeInMs);
    static float* mpFindPath(CDummyMpObject* mpObject,const float* startConfig,const float* goalConfig,int options,float stepSize,int* outputConfigsCnt,int maxTimeInMs,const int* auxIntParams,const float* auxFloatParams,int randomSeed);
    static float* mpFindIkPath(CDummyMpObject* mpObject,const float* startConfig,const float* goalPos,const float* goalQuat,int options,float stepSize,int* outputConfigsCnt,const int* auxIntParams,const float* auxFloatParams);
    static float* mpSimplifyPath(CDummyMpObject* mpObject,const float* pathBuffer,int configCnt,int options,float stepSize,int increment,int* outputConfigsCnt,int maxTimeInMs,const int* auxIntParams,const float* auxFloatParams);
    static float* mpGetConfigTransition(CDummyMpObject* mpObject,const float* startConfig,const float* goalConfig,int options,const int* select,float calcStepSize,float maxOutStepSize,int wayPointCnt,const float* wayPoints,int* outputConfigsCnt,const int* auxIntParams,const float* auxFloatParams);

    static ptr_createNonHolonomicPathPlanningObject _createNonHolonomicPathPlanningObject;
    static ptr_createHolonomicPathPlanningObject _createHolonomicPathPlanningObject;
    static ptr_destroyPathPlanningObject _destroyPathPlanningObject;
    static ptr_searchPath _searchPath;
    static ptr_getPathNodeCount _getPathNodeCount;
    static ptr_setPartialPath _setPartialPath;
    static ptr_smoothFoundPath _smoothFoundPath;
    static ptr_getFoundPath _getFoundPath;
    static ptr_getSearchTree _getSearchTree;
    static ptr_releaseBuffer _releaseBuffer;

    static ptr_createMpObject _createMpObject;
    static ptr_destroyMpObject _destroyMpObject;
    static ptr_setMpObjectData _setMpObjectData;
    static ptr_calculateMpNodesPhase1 _calculateMpNodesPhase1;
    static ptr_getMpPhase1NodesRenderData _getMpPhase1NodesRenderData;
    static ptr_getMpPhase2NodesRenderData _getMpPhase2NodesRenderData;
    static ptr_getMpSerializationData _getMpSerializationData;
    static ptr_getMpPhase1NodeCnt _getMpPhase1NodeCnt;
    static ptr_getMpRobotConfigFromTipPose _getMpRobotConfigFromTipPose;
    static ptr_mpFindPath _mpFindPath;
    static ptr_mpFindIkPath _mpFindIkPath;
    static ptr_mpSimplifyPath _mpSimplifyPath;
    static ptr_mpGetConfigTransition _mpGetConfigTransition;


    static bool _pathPlanningInterfaceInitialized;
    static bool _pathPlanningSystemPluginsLoadPhaseOver;

    static float getNormalizedAngle(float angle);
};
