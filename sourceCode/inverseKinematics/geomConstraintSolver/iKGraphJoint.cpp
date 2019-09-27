
#include "vrepMainHeader.h"
#include "iKGraphJoint.h"
#include "gCSDefs.h"

CIKGraphJoint::CIKGraphJoint(bool isRevolute,float theParameter,float theMinVal,float theRange,float theScrewPitch,bool isCyclic,float theWeight)
{
    topObject=nullptr;
    downObject=nullptr;
    type=IK_GRAPH_JOINT_TYPE;
    if (isRevolute)
        jointType=IK_GRAPH_REVOLUTE_JOINT_TYPE;
    else
        jointType=IK_GRAPH_PRISMATIC_JOINT_TYPE;
    parameter=theParameter;
    minValue=theMinVal;
    range=theRange;
    cyclic=isCyclic;
    weight=theWeight;
    screwPitch=theScrewPitch;
    followedJoint=nullptr;
    disabled=false;
}
CIKGraphJoint::CIKGraphJoint(C4Vector& theSphericalTr,float theRange,float theWeight)
{
    topObject=nullptr;
    downObject=nullptr;
    type=IK_GRAPH_JOINT_TYPE;
    jointType=IK_GRAPH_SPHERICAL_JOINT_TYPE;
    screwPitch=0.0f;
    parameter=0.0f;
    minValue=-piValue_f;
    range=theRange;
    cyclic=false;
    weight=theWeight;
    sphericalTransformation=theSphericalTr;
    followedJoint=nullptr;
    disabled=false;
}

CIKGraphJoint::~CIKGraphJoint()
{
}

C7Vector CIKGraphJoint::getDownToTopTransformation()
{
    C7Vector retVal;
    retVal.setIdentity();
    if (jointType==IK_GRAPH_SPHERICAL_JOINT_TYPE)
        retVal.Q=sphericalTransformation;
    else if (jointType==IK_GRAPH_REVOLUTE_JOINT_TYPE)
        retVal.Q.setAngleAndAxis(parameter,C3Vector(0.0f,0.0f,1.0f));
    else if (jointType==IK_GRAPH_PRISMATIC_JOINT_TYPE)
        retVal.X(2)=parameter;
    return(retVal);
}

CIKGraphObject* CIKGraphJoint::getTopIKGraphObject()
{
    return(topObject);
}

CIKGraphObject* CIKGraphJoint::getDownIKGraphObject()
{
    return(downObject);
}

CIKGraphNode* CIKGraphJoint::getUnexplored(int pos)
{
    if (downObject->explorationID==-1)
    {
        if (pos==0)
            return(downObject);
        pos--;
    }
    if (topObject->explorationID==-1)
    {
        if (pos==0)
            return(topObject);
        pos--;
    }
    return(nullptr);
}

CIKGraphNode* CIKGraphJoint::getNeighbour(int pos,bool noLinkNeighbour)
{
    if (pos==0)
        return(downObject);
    if (pos==1)
        return(topObject);
    return(nullptr);
}

int CIKGraphJoint::getNumberOfUnexplored()
{   
    int retVal=0;
    if (downObject->explorationID==-1)
        retVal++;
    if (topObject->explorationID==-1)
        retVal++;
    return(retVal);
}

CIKGraphNode* CIKGraphJoint::getNeighbourWithExplorationID(int theID)
{
    if (downObject->explorationID==theID)
        return(downObject);
    if (topObject->explorationID==theID)
        return(topObject);
    return(nullptr);
}


CIKGraphNode* CIKGraphJoint::getExploredWithSmallestExplorationID()
{
    int smallest=999999;
    CIKGraphObject* retVal=nullptr;
    if (downObject->explorationID!=-1)
    {
        if (downObject->explorationID<smallest)
        {
            smallest=downObject->explorationID;
            retVal=downObject;
        }
    }
    if (topObject->explorationID!=-1)
    {
        if (topObject->explorationID<smallest)
        {
            smallest=topObject->explorationID;
            retVal=topObject;
        }
    }
    return(retVal);
}

int CIKGraphJoint::getConnectionNumber()
{
    return(2);
}

void CIKGraphJoint::constrainJointToOtherJoint(CIKGraphJoint* it,float constantVal,float coefficient)
{
    if (it!=nullptr)
    {
        if ( (it->jointType!=IK_GRAPH_SPHERICAL_JOINT_TYPE)&&(jointType!=IK_GRAPH_SPHERICAL_JOINT_TYPE) )
        {
            followedJoint=it;
            constantValue=constantVal;
            coefficientValue=coefficient;
        }
    }
    else
        followedJoint=nullptr;
}
