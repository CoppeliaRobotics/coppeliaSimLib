#pragma once

#include "simTypes.h"

class CNonHolonomicPathNode_old
{
public:
    CNonHolonomicPathNode_old();
    CNonHolonomicPathNode_old(CNonHolonomicPathNode_old* aNode);
    CNonHolonomicPathNode_old(double a,double b,double c);
    virtual ~CNonHolonomicPathNode_old();

    CNonHolonomicPathNode_old* parent;
    double values[3];
};
