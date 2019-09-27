
#include "holonomicPathNode.h"
#include "pathPlanningInterface.h"
#include "v_rep_internal.h"

CHolonomicPathNode::CHolonomicPathNode(const C4Vector& rotAxisRot,const C4Vector& rotAxisRotInv)
{
    _rotAxisRot=rotAxisRot;
    _rotAxisRotInv=rotAxisRotInv;
    parent=nullptr;
    _nodeType=-1;
    values=nullptr;
}

CHolonomicPathNode::CHolonomicPathNode(int theType,const C7Vector& conf,const C4Vector& rotAxisRot,const C4Vector& rotAxisRotInv)
{
    _rotAxisRot=rotAxisRot;
    _rotAxisRotInv=rotAxisRotInv;
    parent=nullptr;
    _nodeType=theType;
    values=nullptr;
    int s=getSize();
    values=new float[s];
    setAllValues(conf.X,conf.Q);
}

CHolonomicPathNode::CHolonomicPathNode(int theType,float searchMin[4],float searchRange[4],const C4Vector& rotAxisRot,const C4Vector& rotAxisRotInv)
{
    _rotAxisRot=rotAxisRot;
    _rotAxisRotInv=rotAxisRotInv;
    parent=nullptr;
    _nodeType=theType;
    values=nullptr;
    if (theType==sim_holonomicpathplanning_xy)
    {
        values=new float[2];
        values[0]=searchMin[0]+searchRange[0]*SIM_RAND_FLOAT;
        values[1]=searchMin[1]+searchRange[1]*SIM_RAND_FLOAT;
    }
    if (theType==sim_holonomicpathplanning_xg)
    {
        values=new float[2];
        values[0]=searchMin[0]+searchRange[0]*SIM_RAND_FLOAT;
        values[1]=CPathPlanningInterface::getNormalizedAngle(searchMin[3]+searchRange[3]*SIM_RAND_FLOAT);
    }
    if (theType==sim_holonomicpathplanning_xyz)
    {
        values=new float[3];
        values[0]=searchMin[0]+searchRange[0]*SIM_RAND_FLOAT;
        values[1]=searchMin[1]+searchRange[1]*SIM_RAND_FLOAT;
        values[2]=searchMin[2]+searchRange[2]*SIM_RAND_FLOAT;
    }
    if (theType==sim_holonomicpathplanning_xyg)
    {
        values=new float[3];
        values[0]=searchMin[0]+searchRange[0]*SIM_RAND_FLOAT;
        values[1]=searchMin[1]+searchRange[1]*SIM_RAND_FLOAT;
        values[2]=CPathPlanningInterface::getNormalizedAngle(searchMin[3]+searchRange[3]*SIM_RAND_FLOAT);
    }
    if (theType==sim_holonomicpathplanning_abg)
    {
        values=new float[4];
        C4Vector d;
        d.buildRandomOrientation();
        values[0]=d(0);
        values[1]=d(1);
        values[2]=d(2);
        values[3]=d(3);
    }
    if (theType==sim_holonomicpathplanning_xyzg)
    {
        values=new float[4];
        values[0]=searchMin[0]+searchRange[0]*SIM_RAND_FLOAT;
        values[1]=searchMin[1]+searchRange[1]*SIM_RAND_FLOAT;
        values[2]=searchMin[2]+searchRange[2]*SIM_RAND_FLOAT;
        values[3]=CPathPlanningInterface::getNormalizedAngle(searchMin[3]+searchRange[3]*SIM_RAND_FLOAT);
    }
    if (theType==sim_holonomicpathplanning_xabg)
    {
        values=new float[5];
        values[0]=searchMin[0]+searchRange[0]*SIM_RAND_FLOAT;
        C4Vector d;
        d.buildRandomOrientation();
        values[1]=d(0);
        values[2]=d(1);
        values[3]=d(2);
        values[4]=d(3);
    }
    if (theType==sim_holonomicpathplanning_xyabg)
    {
        values=new float[6];
        values[0]=searchMin[0]+searchRange[0]*SIM_RAND_FLOAT;
        values[1]=searchMin[1]+searchRange[1]*SIM_RAND_FLOAT;
        C4Vector d;
        d.buildRandomOrientation();
        values[2]=d(0);
        values[3]=d(1);
        values[4]=d(2);
        values[5]=d(3);
    }
    if (theType==sim_holonomicpathplanning_xyzabg)
    {
        values=new float[7];
        values[0]=searchMin[0]+searchRange[0]*SIM_RAND_FLOAT;
        values[1]=searchMin[1]+searchRange[1]*SIM_RAND_FLOAT;
        values[2]=searchMin[2]+searchRange[2]*SIM_RAND_FLOAT;
        C4Vector d;
        d.buildRandomOrientation();
        values[3]=d(0);
        values[4]=d(1);
        values[5]=d(2);
        values[6]=d(3);
    }
}

CHolonomicPathNode::~CHolonomicPathNode()
{
    delete[] values;
}

void CHolonomicPathNode::setAllValues(const C3Vector& pos,const C4Vector& orient)
{
    if (_nodeType==sim_holonomicpathplanning_xy)
    {
        values[0]=pos(0);
        values[1]=pos(1);
    }
    if (_nodeType==sim_holonomicpathplanning_xg)
    {
        values[0]=pos(0);
        C4Vector o(_rotAxisRotInv*orient*_rotAxisRot);
        values[1]=o.getEulerAngles()(2);
    }
    if (_nodeType==sim_holonomicpathplanning_xyz)
    {
        values[0]=pos(0);
        values[1]=pos(1);
        values[2]=pos(2);
    }
    if (_nodeType==sim_holonomicpathplanning_xyg)
    {
        values[0]=pos(0);
        values[1]=pos(1);
        C4Vector o(_rotAxisRotInv*orient*_rotAxisRot);
        values[2]=o.getEulerAngles()(2);
    }
    if (_nodeType==sim_holonomicpathplanning_abg)
    {
        values[0]=orient(0);
        values[1]=orient(1);
        values[2]=orient(2);
        values[3]=orient(3);
    }
    if (_nodeType==sim_holonomicpathplanning_xyzg)
    {
        values[0]=pos(0);
        values[1]=pos(1);
        values[2]=pos(2);
        C4Vector o(_rotAxisRotInv*orient*_rotAxisRot);
        values[3]=o.getEulerAngles()(2);
    }
    if (_nodeType==sim_holonomicpathplanning_xabg)
    {
        values[0]=pos(0);
        values[1]=orient(0);
        values[2]=orient(1);
        values[3]=orient(2);
        values[4]=orient(3);
    }
    if (_nodeType==sim_holonomicpathplanning_xyabg)
    {
        values[0]=pos(0);
        values[1]=pos(1);
        values[2]=orient(0);
        values[3]=orient(1);
        values[4]=orient(2);
        values[5]=orient(3);
    }
    if (_nodeType==sim_holonomicpathplanning_xyzabg)
    {
        values[0]=pos(0);
        values[1]=pos(1);
        values[2]=pos(2);
        values[3]=orient(0);
        values[4]=orient(1);
        values[5]=orient(2);
        values[6]=orient(3);
    }
}

void CHolonomicPathNode::getAllValues(C3Vector& pos,C4Vector& orient)
{
    pos.clear();
    orient.setIdentity();
    if (_nodeType==sim_holonomicpathplanning_xy)
    {
        pos(0)=values[0];
        pos(1)=values[1];
    }
    if (_nodeType==sim_holonomicpathplanning_xg)
    {
        pos(0)=values[0];
        orient=_rotAxisRot*(C4Vector(C3Vector(0.0f,0.0f,values[1]))*_rotAxisRotInv);
    }
    if (_nodeType==sim_holonomicpathplanning_xyz)
    {
        pos(0)=values[0];
        pos(1)=values[1];
        pos(2)=values[2];
    }
    if (_nodeType==sim_holonomicpathplanning_xyg)
    {
        pos(0)=values[0];
        pos(1)=values[1];
        orient=_rotAxisRot*(C4Vector(C3Vector(0.0f,0.0f,values[2]))*_rotAxisRotInv);
    }
    if (_nodeType==sim_holonomicpathplanning_abg)
    {
        orient(0)=values[0];
        orient(1)=values[1];
        orient(2)=values[2];
        orient(3)=values[3];
    }
    if (_nodeType==sim_holonomicpathplanning_xyzg)
    {
        pos(0)=values[0];
        pos(1)=values[1];
        pos(2)=values[2];
        orient=_rotAxisRot*(C4Vector(C3Vector(0.0f,0.0f,values[3]))*_rotAxisRotInv);
    }
    if (_nodeType==sim_holonomicpathplanning_xabg)
    {
        pos(0)=values[0];
        orient(0)=values[1];
        orient(1)=values[2];
        orient(2)=values[3];
        orient(3)=values[4];
    }
    if (_nodeType==sim_holonomicpathplanning_xyabg)
    {
        pos(0)=values[0];
        pos(1)=values[1];
        orient(0)=values[2];
        orient(1)=values[3];
        orient(2)=values[4];
        orient(3)=values[5];
    }
    if (_nodeType==sim_holonomicpathplanning_xyzabg)
    {
        pos(0)=values[0];
        pos(1)=values[1];
        pos(2)=values[2];
        orient(0)=values[3];
        orient(1)=values[4];
        orient(2)=values[5];
        orient(3)=values[6];
    }
}

CHolonomicPathNode* CHolonomicPathNode::copyYourself()
{
    CHolonomicPathNode* newNode=new CHolonomicPathNode(_rotAxisRot,_rotAxisRotInv);
    newNode->_nodeType=_nodeType;
    int s=getSize();
    newNode->values=new float[s];
    for (int i=0;i<s;i++)
        newNode->values[i]=values[i];
    return(newNode);
}

void CHolonomicPathNode::setAllValues(float* v)
{
    int s=getSize();
    for (int i=0;i<s;i++)
        values[i]=v[i];
}

int CHolonomicPathNode::getSize()
{
    if (_nodeType==sim_holonomicpathplanning_xy)
        return(2);
    if (_nodeType==sim_holonomicpathplanning_xg)
        return(2);
    if (_nodeType==sim_holonomicpathplanning_xyz)
        return(3);
    if (_nodeType==sim_holonomicpathplanning_xyg)
        return(3);
    if (_nodeType==sim_holonomicpathplanning_abg)
        return(4);
    if (_nodeType==sim_holonomicpathplanning_xyzg)
        return(4);
    if (_nodeType==sim_holonomicpathplanning_xabg)
        return(5);
    if (_nodeType==sim_holonomicpathplanning_xyabg)
        return(6);
    if (_nodeType==sim_holonomicpathplanning_xyzabg)
        return(7);
    return(0);
}
