
#pragma once

#include "MyMath.h"
#include "iKDummy.h"

class CIKChain  
{
public:
    CIKChain(CIKDummy* tip,float interpolFact,int jointNbToExclude);
    virtual ~CIKChain();

    CIKDummy* tooltip;

    // various:
    CMatrix* matrix;
    CMatrix* errorVector;
    std::vector<CIKJoint*> rowJoints;
    bool chainIsValid;

private:
    CMatrix* getJacobian(CIKDummy* tooltip,C4X4Matrix& tooltipTransf,std::vector<CIKJoint*>& theRowJoints,int jointNbToExclude);
    void multiply(C4X4FullMatrix& d0,C4X4FullMatrix& dp,int index,std::vector<C4X4FullMatrix*>& allMatrices);
    void buildDeltaZRotation(C4X4FullMatrix& d0,C4X4FullMatrix& dp,float screwCoeff);
    void buildDeltaZTranslation(C4X4FullMatrix& d0,C4X4FullMatrix& dp);
};
