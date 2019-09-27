
#pragma once

#include "7Vector.h"

class CmpPhase2Node
{
public:
    CmpPhase2Node(int jointCount,const float* _jointPositions,const C7Vector& _tipTransf);
    virtual ~CmpPhase2Node();

    CmpPhase2Node* copyYourself(int jointCount);

    float* jointPositions;
    C7Vector tipTransf; // relative to base object!
    CmpPhase2Node* parentNode;
};
