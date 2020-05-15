#pragma once

#include "ptCloud_old.h"
#include "7Vector.h"
#include "vMutex.h"

class CViewableBase;

class CPointCloudContainer_old
{
public:
    CPointCloudContainer_old();
    virtual ~CPointCloudContainer_old();

    void simulationEnded();
    void renderYour3DStuff_nonTransparent(CViewableBase* renderingObject,int displayAttrib);
    void renderYour3DStuff_transparent(CViewableBase* renderingObject,int displayAttrib);
    void renderYour3DStuff_overlay(CViewableBase* renderingObject,int displayAttrib);
    void drawAll(int displayAttrib);//bool overlay,bool transparentObject,int displayAttrib,const C4X4Matrix& cameraCTM);
    int addObject(CPtCloud_old* it);
    CPtCloud_old* getObject(int objectID);
    void removeAllObjects(bool onlyNonPersistentOnes);
    void announceObjectWillBeErased(int objID);
    bool removeObject(int objectID);

private:
    std::vector<CPtCloud_old*> _allObjects;
    VMutex _objectMutex;
};
