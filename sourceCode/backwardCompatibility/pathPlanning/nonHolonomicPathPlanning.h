
#pragma once

#include "pathPlanning.h"
#include "nonHolonomicPathNode.h"
#include "dummyClasses.h"
#include <vector>
#include "7Vector.h"

class CNonHolonomicPathPlanning : public CPathPlanning  
{
public:
    CNonHolonomicPathPlanning(int theStartDummyID,int theGoalDummyID,
                            int theRobotCollectionID,int theObstacleCollectionID,int ikGroupID,float theAngularCoeff,
                            float theSteeringAngleCoeff,float theMaxSteeringAngleVariation,float theMaxSteeringAngle,
                            float theStepSize,const float theSearchMinVal[2],const float theSearchRange[2],
                            const int theDirectionConstraints[2],const float clearanceAndMaxDistance[2]);
    virtual ~CNonHolonomicPathPlanning();

    // Following functions are inherited from CPathPlanning:
    int searchPath(int maxTimePerPass);
    bool setPartialPath();
    int smoothFoundPath(int steps,int maxTimePerPass);
    void getPathData(std::vector<float>& data);
    void getSearchTreeData(std::vector<float>& data,bool fromStart);

    void setStepSize(float size);

    std::vector<CNonHolonomicPathNode*> fromStart;
    std::vector<CNonHolonomicPathNode*> fromGoal;
    std::vector<CNonHolonomicPathNode*> foundPath;

private:
    bool doCollide(float* dist);

    CNonHolonomicPathNode* getClosestNode(std::vector<CNonHolonomicPathNode*>& nodes,CNonHolonomicPathNode* sample,bool forward,bool forConnection);
    CNonHolonomicPathNode* extend(std::vector<CNonHolonomicPathNode*>* currentList,CNonHolonomicPathNode* toBeExtended,CNonHolonomicPathNode* extention,bool forward,CDummyDummy* startDummy);
    CNonHolonomicPathNode* connect(std::vector<CNonHolonomicPathNode*>* currentList,std::vector<CNonHolonomicPathNode*>* nextList,CNonHolonomicPathNode* toBeExtended,CNonHolonomicPathNode* extention,bool forward,bool connect,bool test,CDummyDummy* startDummy);

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
