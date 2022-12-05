#include "pathPlanningInterface.h"
#include "simConst.h"
#include <math.h>
#include "pluginContainer.h"
#include "nonHolonomicPathPlanning_old.h"
#include "holonomicPathPlanning_old.h"

void* CPathPlanningInterface::createNonHolonomicPathPlanningObject(int theStartDummyID,int theGoalDummyID,
                            int theRobotCollectionID,int theObstacleCollectionID,double theAngularCoeff,
                            double theSteeringAngleCoeff,double theMaxSteeringAngleVariation,double theMaxSteeringAngle,
                            double theStepSize,const double theSearchMinVal[2],const double theSearchRange[2],
                            const int theDirectionConstraints[2],const double clearanceAndMaxDistance[2])
{
    CNonHolonomicPathPlanning_old* p=new CNonHolonomicPathPlanning_old(theStartDummyID,theGoalDummyID,theRobotCollectionID,
            theObstacleCollectionID,-1,theAngularCoeff,theSteeringAngleCoeff,theMaxSteeringAngleVariation,theMaxSteeringAngle,theStepSize,
            theSearchMinVal,theSearchRange,theDirectionConstraints,clearanceAndMaxDistance);
    return((void*)p);
}

void* CPathPlanningInterface::createHolonomicPathPlanningObject(int theStartDummyID,int theGoalDummyID,
                            int theRobotCollectionID,int theObstacleCollectionID,int thePlanningType,
                            double theAngularCoeff,double theStepSize,const double theSearchMinVal[4],const double theSearchRange[4],
                            const int theDirectionConstraints[4],const double clearanceAndMaxDistance[2],const double gammaAxis[3])
{
    CHolonomicPathPlanning_old* p=new CHolonomicPathPlanning_old(theStartDummyID,theGoalDummyID,theRobotCollectionID,
            theObstacleCollectionID,-1,thePlanningType,theAngularCoeff,theStepSize,
            theSearchMinVal,theSearchRange,theDirectionConstraints,clearanceAndMaxDistance,C3Vector(gammaAxis));
    return((void*)p);
}


void CPathPlanningInterface::destroyPathPlanningObject(void* obj)
{
    delete (CPathPlanning_old*)obj;
}

int CPathPlanningInterface::searchPath(void* obj,int maxTimeInMsPerPass)
{
    return(((CPathPlanning_old*)obj)->searchPath(maxTimeInMsPerPass));
}

int CPathPlanningInterface::getPathNodeCount(void* obj,char fromStart)
{
    if (((CPathPlanning_old*)obj)->isHolonomic)
    {
        if (fromStart!=0)
            return(int(((CHolonomicPathPlanning_old*)obj)->fromStart.size()));
        return(int(((CHolonomicPathPlanning_old*)obj)->fromGoal.size()));
    }
    else
    {
        if (fromStart!=0)
            return(int(((CNonHolonomicPathPlanning_old*)obj)->fromStart.size()));
        return(int(((CNonHolonomicPathPlanning_old*)obj)->fromGoal.size()));
    }
}

char CPathPlanningInterface::setPartialPath(void* obj)
{
    return(((CPathPlanning_old*)obj)->setPartialPath());
}

int CPathPlanningInterface::smoothFoundPath(void* obj,int steps,int maxTimePerPass)
{
    return(((CPathPlanning_old*)obj)->smoothFoundPath(steps,maxTimePerPass));
}

double* CPathPlanningInterface::getFoundPath(void* obj,int* nodeCount)
{
    double* retVal=nullptr;
    std::vector<double> data;
    ((CPathPlanning_old*)obj)->getPathData(data);
    nodeCount[0]=(int)data.size()/7;
    if (nodeCount[0]!=0)
    {
        retVal=new double[nodeCount[0]*7];
        for (int i=0;i<nodeCount[0]*7;i++)
            retVal[i]=data[i];
    }
    return(retVal);
}

double* CPathPlanningInterface::getSearchTree(void* obj,int* segmentCount,char fromStart)
{
    double* retVal=nullptr;
    std::vector<double> data;
    ((CPathPlanning_old*)obj)->getSearchTreeData(data,fromStart!=0);
    segmentCount[0]=(int)data.size()/6;
    if (segmentCount[0]!=0)
    {
        retVal=new double[segmentCount[0]*6];
        for (int i=0;i<segmentCount[0]*6;i++)
            retVal[i]=data[i];
    }
    return(retVal);
}

void CPathPlanningInterface::releaseBuffer(void* buff)
{
    delete[] ((char*)buff);
}

double CPathPlanningInterface::getNormalizedAngle(double angle)
{ // Returns an angle between -PI and +PI
    angle=fmod(angle,6.28318531);
    if (angle<-3.14159266)
        angle+=6.28318531;
    else if (angle>+3.14159266)
        angle-=6.28318531;
    return(angle);
}






