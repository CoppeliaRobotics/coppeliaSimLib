#pragma once

#include "dummyClasses.h"

// FULLY STATIC CLASS
class CPathPlanningInterface  
{
public:
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

    static float getNormalizedAngle(float angle);
};
