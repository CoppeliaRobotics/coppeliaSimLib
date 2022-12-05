#pragma once

#include "dummyClasses.h"
#include "simTypes.h"

// FULLY STATIC CLASS
class CPathPlanningInterface  
{
public:
    static void* createNonHolonomicPathPlanningObject(int theStartDummyID,int theGoalDummyID,
                            int theRobotCollectionID,int theObstacleCollectionID,double theAngularCoeff,
                            double theSteeringAngleCoeff,double theMaxSteeringAngleVariation,double theMaxSteeringAngle,
                            double theStepSize,const double theSearchMinVal[2],const double theSearchRange[2],
                            const int theDirectionConstraints[2],const double clearanceAndMaxDistance[2]);
    static void* createHolonomicPathPlanningObject(int theStartDummyID,int theGoalDummyID,
                            int theRobotCollectionID,int theObstacleCollectionID,int thePlanningType,
                            double theAngularCoeff,double theStepSize,const double theSearchMinVal[4],const double theSearchRange[4],
                            const int theDirectionConstraints[4],const double clearanceAndMaxDistance[2],const double gammaAxis[3]);
    static void destroyPathPlanningObject(void* obj);
    static int searchPath(void* obj,int maxTimeInMsPerPass);
    static int getPathNodeCount(void* obj,char fromStart);
    static char setPartialPath(void* obj);
    static int smoothFoundPath(void* obj,int steps,int maxTimePerPass);
    static double* getFoundPath(void* obj,int* nodeCount);
    static double* getSearchTree(void* obj,int* segmentCount,char fromStart);
    static void releaseBuffer(void* buff);

    static double getNormalizedAngle(double angle);
};
