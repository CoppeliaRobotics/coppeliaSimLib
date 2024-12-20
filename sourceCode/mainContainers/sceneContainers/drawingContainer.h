#pragma once

#include <drawingObject.h>
#include <simMath/7Vector.h>

class CViewableBase;

class CDrawingContainer
{
  public:
    CDrawingContainer();
    virtual ~CDrawingContainer();

    void simulationEnded();
    void eraseAllObjects();
    int addObject(CDrawingObject* it);
    CDrawingObject* getObject(int objectId);
    void announceObjectWillBeErased(const CSceneObject* object);
    void announceScriptStateWillBeErased(int scriptHandle, bool simulationScript, bool sceneSwitchPersistentScript);
    void removeObject(int objectId);

    void pushGenesisEvents();
    void pushAppendNewPointEvents();

  private:
    std::vector<CDrawingObject*> _allObjects;

#ifdef SIM_WITH_GUI
  public:
    void renderYour3DStuff_nonTransparent(CViewableBase* renderingObject, int displayAttrib);
    void renderYour3DStuff_transparent(CViewableBase* renderingObject, int displayAttrib);
    void renderYour3DStuff_overlay(CViewableBase* renderingObject, int displayAttrib);
    void drawAll(bool overlay, bool transparentObject, int displayAttrib, const C4X4Matrix& cameraCTM);
    void drawObjectsParentedWith(bool overlay, bool transparentObject, int parentObjectId, int displayAttrib,
                                 const C4X4Matrix& cameraCTM);
#endif
};
