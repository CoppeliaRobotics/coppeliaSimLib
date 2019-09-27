
#pragma once

#include "3Vector.h"
#include "4Vector.h"
#include "7Vector.h"

class CHolonomicPathNode  
{
public:
    CHolonomicPathNode(const C4Vector& rotAxisRot,const C4Vector& rotAxisRotInv);
    CHolonomicPathNode(int theType,const C7Vector& conf,const C4Vector& rotAxisRot,const C4Vector& rotAxisRotInv);
    CHolonomicPathNode(int theType,float searchMin[4],float searchRange[4],const C4Vector& rotAxisRot,const C4Vector& rotAxisRotInv);
    virtual ~CHolonomicPathNode();

    CHolonomicPathNode* copyYourself();
    int getSize();
    void setAllValues(float* v);
    void setAllValues(const C3Vector& pos,const C4Vector& orient);
    void getAllValues(C3Vector& pos,C4Vector& orient);

    CHolonomicPathNode* parent;
    float* values;
protected:
    int _nodeType;
    C4Vector _rotAxisRot;
    C4Vector _rotAxisRotInv;
};
