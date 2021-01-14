
#pragma once

class CNonHolonomicPathNode_old
{
public:
    CNonHolonomicPathNode_old();
    CNonHolonomicPathNode_old(CNonHolonomicPathNode_old* aNode);
    CNonHolonomicPathNode_old(float a,float b,float c);
    virtual ~CNonHolonomicPathNode_old();

    CNonHolonomicPathNode_old* parent;
    float values[3];
};
