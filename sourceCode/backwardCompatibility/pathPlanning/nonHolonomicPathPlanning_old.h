#pragma once

#include "pathPlanning_old.h"
#include "nonHolonomicPathNode_old.h"
#include "dummyClasses.h"
#include <vector>
#include "7Vector.h"

class CNonHolonomicPathPlanning_old : public CPathPlanning_old
{
public:
    CNonHolonomicPathPlanning_old(int theStartDummyID,int theGoalDummyID,
                            int theRobotCollectionID,int theObstacleCollectionID,int ikGroupID,float theAngularCoeff,
                            float theSteeringAngleCoeff,float theMaxSteeringAngleVariation,float theMaxSteeringAngle,
                            float theStepSize,const float theSearchMinVal[2],const float theSearchRange[2],
                            const int theDirectionConstraints[2],const float clearanceAndMaxDistance[2]);
    virtual ~CNonHolonomicPathPlanning_old();

    // Following functions are inherited from CPathPlanning:
    int searchPath(int maxTimePerPass);
    bool setPartialPath();
    int smoothFoundPath(int steps,int maxTimePerPass);
    void getPathData(std::vector<float>& data);
    void getSearchTreeData(std::vector<float>& data,bool fromStart);

    void setStepSize(float size);

    std::vector<CNonHolonomicPathNode_old*> fromStart;
    std::vector<CNonHolonomicPathNode_old*> fromGoal;
    std::vector<CNonHolonomicPathNode_old*> foundPath;

private:
    bool doCollide(float* dist);

    CNonHolonomicPathNode_old* getClosestNode(std::vector<CNonHolonomicPathNode_old*>& nodes,CNonHolonomicPathNode_old* sample,bool forward,bool forConnection);
    CNonHolonomicPathNode_old* extend(std::vector<CNonHolonomicPathNode_old*>* currentList,CNonHolonomicPathNode_old* toBeExtended,CNonHolonomicPathNode_old* extention,bool forward,CDummyDummy* startDummy);
    CNonHolonomicPathNode_old* connect(std::vector<CNonHolonomicPathNode_old*>* currentList,std::vector<CNonHolonomicPathNode_old*>* nextList,CNonHolonomicPathNode_old* toBeExtended,CNonHolonomicPathNode_old* extention,bool forward,bool connect,bool test,CDummyDummy* startDummy);

    int _startDummyID;
    float angularCoeff;
    float steeringAngleCoeff;
    float maxSteeringAngleVariation;
    float maxSteeringAngle;
    float minTurningRadius;
    float stepSize;
    int DoF;
    float searchMinVal[2];
    float searchRange[2];
    C7Vector _startDummyCTM;
    C7Vector _startDummyLTM;

    int numberOfRandomConnectionTries_forSteppedSmoothing;
    int numberOfRandomConnectionTriesLeft_forSteppedSmoothing;
    std::vector<int> foundPathSameStraightLineID_forSteppedSmoothing;
    int sameStraightLineNextID_forSteppedSmoothing;
    int nextIteration_forSteppedSmoothing;




    float _startConfInterferenceState;

    int directionConstraints[2]; // WRONG!!! NOT USED NOW!!! 0 is for vehicle direction, 1 is for steering direction
};
