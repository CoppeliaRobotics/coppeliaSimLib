
#pragma once

#include "iKJoint.h"

class CIKObjCont  
{
public:
    CIKObjCont();
    virtual ~CIKObjCont();
    void removeAllObjects();
    void addChild(CIKJoint* parent,CIKObject* child);
    void addRoot(CIKJoint* root);
    std::vector<CIKObject*> allObjects;
    int nextID;
    CIKObject* getObjectWithID(int theID);
    CIKJoint* getJointWithData(int theData);
};
