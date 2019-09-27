
#pragma once

#include "iKGraphNode.h"
#include "iKGraphObject.h"

class CIKGraphJoint : public CIKGraphNode
{
public:
    CIKGraphJoint(bool isRevolute,float theParameter,float theMinVal,float theRange,float theScrewPitch,bool isCyclic,float theWeight);
    CIKGraphJoint(C4Vector& theSphericalTr,float theRange,float theWeight);
    virtual ~CIKGraphJoint();

    C7Vector getDownToTopTransformation();
    CIKGraphObject* getTopIKGraphObject();
    CIKGraphObject* getDownIKGraphObject();
    void constrainJointToOtherJoint(CIKGraphJoint* it,float constantVal,float coefficient);
    CIKGraphObject* topObject;
    CIKGraphObject* downObject;
    int jointType;// IK_GRAPH_REVOLUTE_JOINT_TYPE,IK_GRAPH_PRISMATIC_JOINT_TYPE,IK_GRAPH_SPHERICAL_JOINT_TYPE,IK_GRAPH_SCREW_JOINT_TYPE
    float parameter;
    float screwPitch;
    C4Vector sphericalTransformation;
    float minValue;
    float range;
    bool cyclic;
    float weight;
    bool disabled;

    float constantValue;
    float coefficientValue;
    CIKGraphJoint* followedJoint;

    CIKGraphNode* getUnexplored(int pos);
    CIKGraphNode* getNeighbour(int pos,bool noLinkNeighbour);
    CIKGraphNode* getExploredWithSmallestExplorationID();
    CIKGraphNode* getNeighbourWithExplorationID(int theID);
    int getNumberOfUnexplored();
    int getConnectionNumber();
};
