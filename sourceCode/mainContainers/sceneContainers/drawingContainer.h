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
    void renderYour3DStuff_nonTransparent(CViewableBase* renderingObject,int displayAttrib);
    void renderYour3DStuff_transparent(CViewableBase* renderingObject,int displayAttrib);
    void renderYour3DStuff_overlay(CViewableBase* renderingObject,int displayAttrib);
    void drawAll(bool overlay,bool transparentObject,int displayAttrib,const C4X4Matrix& cameraCTM);
    void drawObjectsParentedWith(bool overlay,bool transparentObject,int parentObjectId,int displayAttrib,const C4X4Matrix& cameraCTM);
    int addObject(CDrawingObject* it);
    CDrawingObject* getObject(int objectId);
    void announceObjectWillBeErased(const CSceneObject* object);
    void announceScriptStateWillBeErased(int scriptHandle,bool simulationScript,bool sceneSwitchPersistentScript);
    void removeObject(int objectId);
    void adjustForFrameChange(int objectId,const C7Vector& preCorrection);
    void adjustForScaling(int objectId,double xScale,double yScale,double zScale);

    void pushGenesisEvents();
    void pushAppendNewPointEvents();



private:
    std::vector<CDrawingObject*> _allObjects;
    VMutex _objectMutex;
};
