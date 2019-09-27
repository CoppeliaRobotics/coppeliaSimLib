
#pragma once

class CNonHolonomicPathNode  
{
public:
    CNonHolonomicPathNode();
    CNonHolonomicPathNode(CNonHolonomicPathNode* aNode);
    CNonHolonomicPathNode(float a,float b,float c);
    virtual ~CNonHolonomicPathNode();

    CNonHolonomicPathNode* parent;
    float values[3];
};
