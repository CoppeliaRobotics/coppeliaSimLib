
#include "mpPhase2Node.h"
#include "v_rep_internal.h"

CmpPhase2Node::CmpPhase2Node(int jointCount,const float* _jointPositions,const C7Vector& _tipTransf)
{
    jointPositions=new float[jointCount];
    tipTransf=_tipTransf;
    for (int i=0;i<jointCount;i++)
        jointPositions[i]=_jointPositions[i];
    parentNode=nullptr;
}

CmpPhase2Node::~CmpPhase2Node()
{
    delete[] jointPositions;
}


CmpPhase2Node* CmpPhase2Node::copyYourself(int jointCount)
{
    CmpPhase2Node* newNode=new CmpPhase2Node(jointCount,jointPositions,tipTransf);
    return(newNode);
}

