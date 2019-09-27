
#pragma once

#include "MyMath.h"
#include "iKObject.h"

class CIKDummy : public CIKObject  
{
public:
    CIKDummy(C7Vector& tipLocalTransformation,C7Vector& targetCumulTransformation);
    virtual ~CIKDummy();
    C7Vector targetCumulativeTransformation;
    C7Vector baseReorient;
    int constraints;
    float dampingFactor;
    bool loopClosureDummy;
    int chainSize;

    int computeChainSize();
    bool withinTolerance(float positionTolerance,float orientationTolerance);
};
