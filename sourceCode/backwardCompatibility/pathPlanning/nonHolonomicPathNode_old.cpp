#include "nonHolonomicPathNode_old.h"
#include "pathPlanningInterface.h"
#include "simInternal.h"

CNonHolonomicPathNode_old::CNonHolonomicPathNode_old()
{
    parent=0;
}

CNonHolonomicPathNode_old::CNonHolonomicPathNode_old(CNonHolonomicPathNode_old* aNode)
{
    parent=0;
    values[0]=aNode->values[0];
    values[1]=aNode->values[1];
    values[2]=aNode->values[2];
}

CNonHolonomicPathNode_old::CNonHolonomicPathNode_old(floatDouble a,floatDouble b,floatDouble c)
{
    parent=0;
    values[0]=a;
    values[1]=b;
    values[2]=c;
}

CNonHolonomicPathNode_old::~CNonHolonomicPathNode_old()
{
}
