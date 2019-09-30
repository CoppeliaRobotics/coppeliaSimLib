
#pragma once

#include "mainCont.h"
#include "constraintSolverObject.h"

class CConstraintSolverContainer : public CMainCont 
{
public:
    CConstraintSolverContainer();
    virtual ~CConstraintSolverContainer();

    void simulationAboutToStart();
    void simulationEnded();
    void renderYour3DStuff(CViewableBase* renderingObject,int displayAttrib);


    CConstraintSolverObject* getObject(int objectID);
    CConstraintSolverObject* getObject(const char* objectName);
    CConstraintSolverObject* getObjectFromMechanismID(int mechanismID);
    void addObject(CConstraintSolverObject* anObject,bool objectIsACopy);
    void addObjectWithSuffixOffset(CConstraintSolverObject* anObject,bool objectIsACopy,int suffixOffset);
    void getMinAndMaxNameSuffixes(int& minSuffix,int& maxSuffix);
    bool canSuffix1BeSetToSuffix2(int suffix1,int suffix2);
    void setSuffix1ToSuffix2(int suffix1,int suffix2);
    void removeObject(int objectID);
    void removeAllObjects();
    void removeMultipleDefinedObjects();
    void announceObjectWillBeErased(int objID);

    int computeAllMechanisms(bool exceptExplicitHandling);

    // Variable that need to be serialized on an individual basis:
    std::vector<CConstraintSolverObject*> allGcsObjects;
};
