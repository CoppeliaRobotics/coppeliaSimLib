
#pragma once

#include "MyMath.h"
#include "jointObject.h"
#include "dummy.h"
#include "ikEl.h"
#include "ikGroup.h"

// FULLY STATIC CLASS
class CIkRoutine  
{
public:
    static void multiply(C4X4FullMatrix& d0,C4X4FullMatrix& dp,int index,
                        std::vector<C4X4FullMatrix*>& allMatrices);
    static void buildDeltaZRotation(C4X4FullMatrix& d0,C4X4FullMatrix& dp,float screwCoeff);
    static void buildDeltaZTranslation(C4X4FullMatrix& d0,C4X4FullMatrix& dp);
    static CMatrix* getJacobian(CikEl* ikElement,C4X4Matrix& tooltipTransf,std::vector<int>* rowJointIDs=nullptr,std::vector<int>* rowJointStages=nullptr);
    static void performGroupIK(CikGroup* ikGroup);
    static CMatrix* getAvoidanceJacobian(C3DObject* base,C3DObject* avoidingObject,const C4X4Matrix& relPos,std::vector<int>* rowJointIDs=nullptr,std::vector<int>* rowJointStages=nullptr);
};
