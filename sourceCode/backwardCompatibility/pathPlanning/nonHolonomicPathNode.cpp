
#include "nonHolonomicPathNode.h"
#include "pathPlanningInterface.h"
#include "v_rep_internal.h"

CNonHolonomicPathNode::CNonHolonomicPathNode()
{
    parent=0;
}

CNonHolonomicPathNode::CNonHolonomicPathNode(CNonHolonomicPathNode* aNode)
{
    parent=0;
    values[0]=aNode->values[0];
    values[1]=aNode->values[1];
    values[2]=aNode->values[2];
}

CNonHolonomicPathNode::CNonHolonomicPathNode(float a,float b,float c)
{
    parent=0;
    values[0]=a;
    values[1]=b;
    values[2]=c;
}

CNonHolonomicPathNode::~CNonHolonomicPathNode()
{
}
