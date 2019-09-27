
#pragma once

#include "MyMath.h"

class CIKJoint;

class CIKObject  
{
public:
    CIKObject();
    virtual ~CIKObject();

    C7Vector getLocalTransformation(bool useTempVals);
    C7Vector getLocalTransformationPart1(bool useTempVals);
    C7Vector getCumulativeTransformation(bool useTempVals);
    C7Vector getCumulativeTransformationPart1(bool useTempVals);
    C7Vector getParentCumulativeTransformation(bool useTempVals);

    int realObjectIdentifier;
    int identifier;
    int data;
    int objectType;
    CIKJoint* parent;
    C7Vector transformation; // Quaternion and position
};
