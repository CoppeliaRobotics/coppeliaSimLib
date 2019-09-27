
#pragma once

#include "7Vector.h"

class CmpPhase1Node
{
public:
    CmpPhase1Node(int jointCount,const float* _jointPositions,const C7Vector& _tipPose);
    virtual ~CmpPhase1Node();

    CmpPhase1Node* copyYourself(int jointCount);

    float* jointPositions;
//  KEEP!! int* neighbourIndices;
    C7Vector tipPose;
    unsigned char attributes; // bit2 set--> node already tried
};
