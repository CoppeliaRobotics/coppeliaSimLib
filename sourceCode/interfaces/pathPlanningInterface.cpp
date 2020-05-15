#include "pathPlanningInterface.h"
#include "simConst.h"
#include <math.h>
#include "pluginContainer.h"
#include "nonHolonomicPathPlanning.h"
#include "holonomicPathPlanning.h"

CDummyNonHolonomicPathPlanning* CPathPlanningInterface::createNonHolonomicPathPlanningObject(int theStartDummyID,int theGoalDummyID,
                            int theRobotCollectionID,int theObstacleCollectionID,float theAngularCoeff,
                            float theSteeringAngleCoeff,float theMaxSteeringAngleVariation,float theMaxSteeringAngle,
                            float theStepSize,const float theSearchMinVal[2],const float theSearchRange[2],
                            const int theDirectionConstraints[2],const float clearanceAndMaxDistance[2])
{
    CNonHolonomicPathPlanning* p=new CNonHolonomicPathPlanning(theStartDummyID,theGoalDummyID,theRobotCollectionID,
            theObstacleCollectionID,-1,theAngularCoeff,theSteeringAngleCoeff,theMaxSteeringAngleVariation,theMaxSteeringAngle,theStepSize,
            theSearchMinVal,theSearchRange,theDirectionConstraints,clearanceAndMaxDistance);
    return((CDummyNonHolonomicPathPlanning*)p);
}

CDummyHolonomicPathPlanning* CPathPlanningInterface::createHolonomicPathPlanningObject(int theStartDummyID,int theGoalDummyID,
                            int theRobotCollectionID,int theObstacleCollectionID,int thePlanningType,
                            float theAngularCoeff,float theStepSize,const float theSearchMinVal[4],const float theSearchRange[4],
                            const int theDirectionConstraints[4],const float clearanceAndMaxDistance[2],const float gammaAxis[3])
{
    CHolonomicPathPlanning* p=new CHolonomicPathPlanning(theStartDummyID,theGoalDummyID,theRobotCollectionID,
            theObstacleCollectionID,-1,thePlanningType,theAngularCoeff,theStepSize,
            theSearchMinVal,theSearchRange,theDirectionConstraints,clearanceAndMaxDistance,C3Vector(gammaAxis));
    return((CDummyHolonomicPathPlanning*)p);
}


void CPathPlanningInterface::destroyPathPlanningObject(CDummyPathPlanning* obj)
{
    delete (CPathPlanning*)obj;
}

int CPathPlanningInterface::searchPath(CDummyPathPlanning* obj,int maxTimeInMsPerPass)
{
    return(((CPathPlanning*)obj)->searchPath(maxTimeInMsPerPass));
}

int CPathPlanningInterface::getPathNodeCount(CDummyPathPlanning* obj,char fromStart)
{
    if (((CPathPlanning*)obj)->isHolonomic)
    {
        if (fromStart!=0)
            return(int(((CHolonomicPathPlanning*)obj)->fromStart.size()));
        return(int(((CHolonomicPathPlanning*)obj)->fromGoal.size()));
    }
    else
    {
        if (fromStart!=0)
            return(int(((CNonHolonomicPathPlanning*)obj)->fromStart.size()));
        return(int(((CNonHolonomicPathPlanning*)obj)->fromGoal.size()));
    }
}

char CPathPlanningInterface::setPartialPath(CDummyPathPlanning* obj)
{
    return(((CPathPlanning*)obj)->setPartialPath());
}

int CPathPlanningInterface::smoothFoundPath(CDummyPathPlanning* obj,int steps,int maxTimePerPass)
{
    return(((CPathPlanning*)obj)->smoothFoundPath(steps,maxTimePerPass));
}

float* CPathPlanningInterface::getFoundPath(CDummyPathPlanning* obj,int* nodeCount)
{
    float* retVal=nullptr;
    std::vector<float> data;
    ((CPathPlanning*)obj)->getPathData(data);
    nodeCount[0]=(int)data.size()/7;
    if (nodeCount[0]!=0)
    {
        retVal=new float[nodeCount[0]*7];
        for (int i=0;i<nodeCount[0]*7;i++)
            retVal[i]=data[i];
    }
    return(retVal);
}

float* CPathPlanningInterface::getSearchTree(CDummyPathPlanning* obj,int* segmentCount,char fromStart)
{
    float* retVal=nullptr;
    std::vector<float> data;
    ((CPathPlanning*)obj)->getSearchTreeData(data,fromStart!=0);
    segmentCount[0]=(int)data.size()/6;
    if (segmentCount[0]!=0)
    {
        retVal=new float[segmentCount[0]*6];
        for (int i=0;i<segmentCount[0]*6;i++)
            retVal[i]=data[i];
    }
    return(retVal);
}

void CPathPlanningInterface::releaseBuffer(void* buff)
{
    delete[] ((char*)buff);
}

float CPathPlanningInterface::getNormalizedAngle(float angle)
{ // Returns an angle between -PI and +PI
    angle=fmod(angle,6.28318531f);
    if (angle<-3.14159266f)
        angle+=6.28318531f;
    else if (angle>+3.14159266f)
        angle-=6.28318531f;
    return(angle);
}






