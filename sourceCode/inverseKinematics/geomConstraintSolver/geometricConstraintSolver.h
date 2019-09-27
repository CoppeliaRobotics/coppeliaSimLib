
#pragma once

#include "iKObjCont.h"
#include "iKChain.h"
#include "iKGraphObjCont.h"
#include "iKChainCont.h"

struct SGeomConstrSolverParam
{
    int maxIterations;
    float interpolation;
    float generalDamping;
    float maxAngularVariation;
    float maxLinearVariation;
    float loopClosurePositionTolerance;
    float loopClosureOrientationTolerance;
};

//FULLY STATIC CLASS
class CGeometricConstraintSolver  
{
public:
    static bool solve(CIKGraphObjCont& graphContainer,SGeomConstrSolverParam& parameters);

private:
    static bool solveHierarchy(CIKObjCont* objects,SGeomConstrSolverParam& parameters);
    static bool performMainIterationLoop(std::vector<CIKDummy*>& tipContainer,int& maxIterations,float interpolation,SGeomConstrSolverParam& parameters);
    static bool tryIncrementalLoopSolve(std::vector<CIKDummy*>& tipContainer,SGeomConstrSolverParam& parameters);
    static int performOnePass(CIKChainCont& chainCont,bool& limitOrAvoidanceNeedMoreCalculation,float interpolFact,float& nextInterpol,SGeomConstrSolverParam& parameters);
    
    static void orderLoopsFromSmallestToBiggest(std::vector<CIKDummy*>& tipContainer);
    static void getSmallestLoops(std::vector<CIKDummy*>& tipContainer,std::vector<CIKDummy*>& returnContainer,int numberOfLoops);
    static bool areLoopsClosedWithinTolerance(std::vector<CIKDummy*>& tipContainer,SGeomConstrSolverParam& parameters);
};
