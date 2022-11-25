#pragma once

#include "simTypes.h"

class CNonHolonomicPathNode_old
{
public:
    CNonHolonomicPathNode_old();
    CNonHolonomicPathNode_old(CNonHolonomicPathNode_old* aNode);
    CNonHolonomicPathNode_old(floatDouble a,floatDouble b,floatDouble c);
    virtual ~CNonHolonomicPathNode_old();

    CNonHolonomicPathNode_old* parent;
    floatDouble values[3];
};
