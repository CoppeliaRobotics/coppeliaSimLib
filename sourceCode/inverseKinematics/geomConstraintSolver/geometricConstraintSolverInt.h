
#pragma once

#include "iKGraphObjCont.h"
#include "3DObject.h"

class CGeometricConstraintSolverInt  
{
public:
    CGeometricConstraintSolverInt(int threeDObjID,int constraintSolverObjectID);
    virtual ~CGeometricConstraintSolverInt();
    bool solve();
    int getMechanismID();

private:
    int createGraph(CIKGraphObjCont& graphContainer,C3DObject* partOfMechanism,std::vector<CIKGraphNode*>& treeBases);
    CIKGraphNode* createTree(CIKGraphObjCont& graphContainer,C3DObject* objectOnTree,std::vector<C3DObject*>& exploredObjs,std::vector<C3DObject*>& links,bool keepShapes,int& baseObjectID);

    CIKGraphObjCont* graphContainer;
    std::vector<CIKGraphNode*> treeBases;
    int lastMechanismID;
    int _constraintSolverObjectID;
};
