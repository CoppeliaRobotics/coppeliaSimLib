#pragma once

#include "pathPlanning_old.h"
#include "holonomicPathNode_old.h"
#include "dummyClasses.h"
#include <vector>
#include "4Vector.h"
#include "7Vector.h"

class CHolonomicPathPlanning_old : public CPathPlanning_old
{
public:
    CHolonomicPathPlanning_old(int theStartDummyID,int theGoalDummyID,
                            int theRobotCollectionID,int theObstacleCollectionID,int ikGroupID,
                            int thePlanningType,floatDouble theAngularCoeff,
                            floatDouble theStepSize,
                            const floatDouble theSearchMinVal[4],const floatDouble theSearchRange[4],
                            const int theDirectionConstraints[4],const floatDouble clearanceAndMaxDistance[2],const C3Vector& gammaAxis);
    virtual ~CHolonomicPathPlanning_old();

    // Following functions are inherited from CPathPlanning:
    int searchPath(int maxTimePerPass);
    bool setPartialPath();
    int smoothFoundPath(int steps,int maxTimePerPass);
    void getPathData(std::vector<floatDouble>& data);

    void getSearchTreeData(std::vector<floatDouble>& data,bool fromStart);

    void setAngularCoefficient(floatDouble coeff);
    void setStepSize(floatDouble size);

    std::vector<CHolonomicPathNode_old*> fromStart;
    std::vector<CHolonomicPathNode_old*> fromGoal;
    std::vector<CHolonomicPathNode_old*> foundPath;

private:
    bool doCollide(floatDouble* dist);

    CHolonomicPathNode_old* getClosestNode(std::vector<CHolonomicPathNode_old*>& nodes,CHolonomicPathNode_old* sample);
    CHolonomicPathNode_old* extend(std::vector<CHolonomicPathNode_old*>* nodeList,CHolonomicPathNode_old* toBeExtended,CHolonomicPathNode_old* extention,bool connect,CXDummy* dummy);
    int getVector(CHolonomicPathNode_old* fromPoint,CHolonomicPathNode_old* toPoint,floatDouble vect[7],floatDouble e,floatDouble& artificialLength,bool dontDivide);
    bool addVector(C3Vector& pos,C4Vector& orient,floatDouble vect[7]);
    bool areDirectionConstraintsRespected(floatDouble vect[7]);
    bool areSomeValuesForbidden(floatDouble values[7]);


    int startDummyID;
    int goalDummyID;
    int planningType;
    floatDouble angularCoeff;
    floatDouble stepSize;
    floatDouble _searchMinVal[4];
    floatDouble _searchRange[4];
    int _directionConstraints[4];
    bool _directionConstraintsOn;

    C4Vector _gammaAxisRotation;
    C4Vector _gammaAxisRotationInv;

    floatDouble _startConfInterferenceState;

    C7Vector _startDummyCTM;
    C7Vector _startDummyLTM;

    int numberOfRandomConnectionTries_forSteppedSmoothing;
    int numberOfRandomConnectionTriesLeft_forSteppedSmoothing;
    std::vector<int> foundPathSameStraightLineID_forSteppedSmoothing;
    int sameStraightLineNextID_forSteppedSmoothing;
    int nextIteration_forSteppedSmoothing;
};
