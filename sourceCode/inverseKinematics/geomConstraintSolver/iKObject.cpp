
#include "vrepMainHeader.h"
#include "iKObject.h"
#include "iKJoint.h"
#include "iKDummy.h"
#include "gCSDefs.h"

CIKObject::CIKObject()
{
    parent=nullptr;
    data=-1;
}

CIKObject::~CIKObject()
{

}

C7Vector CIKObject::getLocalTransformation(bool useTempVals)
{
    if (objectType==IK_JOINT_TYPE)
    {
        C7Vector jointTr;
        jointTr.setIdentity();
        float param=((CIKJoint*)this)->parameter;
        if (useTempVals)
            param=((CIKJoint*)this)->tempParameter;
        if (((CIKJoint*)this)->revolute)
            jointTr.Q.setAngleAndAxis(param,C3Vector(0.0f,0.0f,1.0f));
        else
            jointTr.X(2)=param;
        return(transformation*jointTr);
    }
    else
        return(transformation);
}
C7Vector CIKObject::getLocalTransformationPart1(bool useTempVals)
{
    return(transformation);
}
C7Vector CIKObject::getCumulativeTransformation(bool useTempVals)
{
    if (objectType==IK_JOINT_TYPE)
    {
        C7Vector jointTr;
        jointTr.setIdentity();
        float param=((CIKJoint*)this)->parameter;
        if (useTempVals)
            param=((CIKJoint*)this)->tempParameter;
        if (((CIKJoint*)this)->revolute)
            jointTr.Q.setAngleAndAxis(param,C3Vector(0.0f,0.0f,1.0f));
        else
            jointTr.X(2)=param;
        return(getParentCumulativeTransformation(useTempVals)*transformation*jointTr);
    }
    else
        return(getParentCumulativeTransformation(useTempVals)*transformation);
}
C7Vector CIKObject::getCumulativeTransformationPart1(bool useTempVals)
{
    return(getParentCumulativeTransformation(useTempVals)*transformation);  
}
C7Vector CIKObject::getParentCumulativeTransformation(bool useTempVals)
{
    if (parent==nullptr)
    {
        C7Vector tr;
        tr.setIdentity();
        return(tr);
    }
    else
        return(parent->getCumulativeTransformation(useTempVals));
}
