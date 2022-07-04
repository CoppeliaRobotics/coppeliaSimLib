#pragma once

#include "dummyClasses.h"

// FULLY STATIC CLASS
class CPathPlanningInterface  
{
public:
    static void* createNonHolonomicPathPlanningObject(int theStartDummyID,int theGoalDummyID,
                            int theRobotCollectionID,int theObstacleCollectionID,float theAngularCoeff,
                            float theSteeringAngleCoeff,float theMaxSteeringAngleVariation,float theMaxSteeringAngle,
                            float theStepSize,const float theSearchMinVal[2],const float theSearchRange[2],
                            const int theDirectionConstraints[2],const float clearanceAndMaxDistance[2]);
    static void* createHolonomicPathPlanningObject(int theStartDummyID,int theGoalDummyID,
                            int theRobotCollectionID,int theObstacleCollectionID,int thePlanningType,
                            float theAngularCoeff,float theStepSize,const float theSearchMinVal[4],const float theSearchRange[4],
                            const int theDirectionConstraints[4],const float clearanceAndMaxDistance[2],const float gammaAxis[3]);
    static void destroyPathPlanningObject(void* obj);
    static int searchPath(void* obj,int maxTimeInMsPerPass);
    static int getPathNodeCount(void* obj,char fromStart);
    static char setPartialPath(void* obj);
    static int smoothFoundPath(void* obj,int steps,int maxTimePerPass);
    static float* getFoundPath(void* obj,int* nodeCount);
    static float* getSearchTree(void* obj,int* segmentCount,char fromStart);
    static void releaseBuffer(void* buff);

    static float getNormalizedAngle(float angle);
};
