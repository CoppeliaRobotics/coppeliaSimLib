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
                            int thePlanningType,float theAngularCoeff,
                            float theStepSize,
                            const float theSearchMinVal[4],const float theSearchRange[4],
                            const int theDirectionConstraints[4],const float clearanceAndMaxDistance[2],const C3Vector& gammaAxis);
    virtual ~CHolonomicPathPlanning_old();

    // Following functions are inherited from CPathPlanning:
    int searchPath(int maxTimePerPass);
    bool setPartialPath();
    int smoothFoundPath(int steps,int maxTimePerPass);
    void getPathData(std::vector<float>& data);

    void getSearchTreeData(std::vector<float>& data,bool fromStart);

    void setAngularCoefficient(float coeff);
    void setStepSize(float size);

    std::vector<CHolonomicPathNode_old*> fromStart;
    std::vector<CHolonomicPathNode_old*> fromGoal;
    std::vector<CHolonomicPathNode_old*> foundPath;

private:
    bool doCollide(float* dist);

    CHolonomicPathNode_old* getClosestNode(std::vector<CHolonomicPathNode_old*>& nodes,CHolonomicPathNode_old* sample);
    CHolonomicPathNode_old* extend(std::vector<CHolonomicPathNode_old*>* nodeList,CHolonomicPathNode_old* toBeExtended,CHolonomicPathNode_old* extention,bool connect,CXDummy* dummy);
    int getVector(CHolonomicPathNode_old* fromPoint,CHolonomicPathNode_old* toPoint,float vect[7],float e,float& artificialLength,bool dontDivide);
    bool addVector(C3Vector& pos,C4Vector& orient,float vect[7]);
    bool areDirectionConstraintsRespected(float vect[7]);
    bool areSomeValuesForbidden(float values[7]);


    int startDummyID;
    int goalDummyID;
    int planningType;
    float angularCoeff;
    float stepSize;
    float _searchMinVal[4];
    float _searchRange[4];
    int _directionConstraints[4];
    bool _directionConstraintsOn;

    C4Vector _gammaAxisRotation;
    C4Vector _gammaAxisRotationInv;

    float _startConfInterferenceState;

    C7Vector _startDummyCTM;
    C7Vector _startDummyLTM;

    int numberOfRandomConnectionTries_forSteppedSmoothing;
    int numberOfRandomConnectionTriesLeft_forSteppedSmoothing;
    std::vector<int> foundPathSameStraightLineID_forSteppedSmoothing;
    int sameStraightLineNextID_forSteppedSmoothing;
    int nextIteration_forSteppedSmoothing;
};
