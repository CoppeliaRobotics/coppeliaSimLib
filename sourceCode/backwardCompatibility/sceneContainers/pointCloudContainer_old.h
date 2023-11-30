#pragma once

#include <ptCloud_old.h>
#include <simMath/7Vector.h>

class CViewableBase;

class CPointCloudContainer_old
{
  public:
    CPointCloudContainer_old();
    virtual ~CPointCloudContainer_old();

    void simulationEnded();
    int addObject(CPtCloud_old *it);
    CPtCloud_old *getObject(int objectID);
    void eraseAllObjects(bool onlyNonPersistentOnes);
    void announceObjectWillBeErased(int objID);
    bool removeObject(int objectID);

    void pushGenesisEvents();

  private:
    std::vector<CPtCloud_old *> _allObjects;

#ifdef SIM_WITH_GUI
  public:
    void renderYour3DStuff_nonTransparent(CViewableBase *renderingObject, int displayAttrib);
    void renderYour3DStuff_transparent(CViewableBase *renderingObject, int displayAttrib);
    void renderYour3DStuff_overlay(CViewableBase *renderingObject, int displayAttrib);
    void drawAll(
        int displayAttrib); // bool overlay,bool transparentObject,int displayAttrib,const C4X4Matrix& cameraCTM);
#endif
};
