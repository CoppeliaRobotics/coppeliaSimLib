#include "mpPhase1Node.h"
#include "v_rep_internal.h"

CmpPhase1Node::CmpPhase1Node(int jointCount,const float* _jointPositions,const C7Vector& _tipPose)
{
    jointPositions=new float[jointCount];
// KEEP!!   neighbourIndices=new int[jointCount*2];
    tipPose=_tipPose;
    attributes=0;
    for (int i=0;i<jointCount;i++)
        jointPositions[i]=_jointPositions[i];
// KEEP!!   for (int i=0;i<jointCount*2;i++)
// KEEP!!       neighbourIndices[i]=-1;
}

CmpPhase1Node::~CmpPhase1Node()
{
// KEEP!!   delete[] neighbourIndices;
    delete[] jointPositions;
}

CmpPhase1Node* CmpPhase1Node::copyYourself(int jointCount)
{
    CmpPhase1Node* newNode=new CmpPhase1Node(jointCount,jointPositions,tipPose);
// KEEP!!   for (int i=0;i<jointCount*2;i++)
// KEEP!!       newNode->neighbourIndices[i]=neighbourIndices[i];
    newNode->attributes=attributes;
    return(newNode);
}

/* KEEP!!
void CmpPhase1Node::setNeighbours(int index,int theNeighbourIndex)
{
    neighbourIndices[index]=theNeighbourIndex;
}
*/

/*
void CmpPhase1Node::setColliding(bool colliding)
{
    if (colliding)
        attributes&=254;
    else
        attributes|=1;
}
*/
