
#pragma once

#include "shape.h"
#include "3Vector.h"
#include "4X4Matrix.h"

class CStatDistObj  
{
public:
    CStatDistObj(CShape* theShapeA,CShape* theShapeB);
    virtual ~CStatDistObj();
    bool measure(float& dist,int theCaching[2]);
    void checkDistance(int triaIndex,C3Vector& a0,C3Vector& e0,C3Vector& e1,int triAIndex,C3Vector& A0,C3Vector& E0,C3Vector& E1);

    C7Vector shapeACTM;
    C7Vector shapeBCTM;
    CShape* shapeA;
    CShape* shapeB;
    C3Vector ptOnShapeA;
    C3Vector ptOnShapeB;
};
