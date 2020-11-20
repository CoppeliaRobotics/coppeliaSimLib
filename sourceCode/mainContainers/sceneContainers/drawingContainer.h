#pragma once

#include "drawingObject.h"
#include "7Vector.h"

class CViewableBase;

class CDrawingContainer
{
public:
    CDrawingContainer();
    virtual ~CDrawingContainer();

    void simulationEnded();
    void removeAllObjects();
    void renderYour3DStuff_nonTransparent(CViewableBase* renderingObject,int displayAttrib);
    void renderYour3DStuff_transparent(CViewableBase* renderingObject,int displayAttrib);
    void renderYour3DStuff_overlay(CViewableBase* renderingObject,int displayAttrib);
    void drawAll(bool overlay,bool transparentObject,int displayAttrib,const C4X4Matrix& cameraCTM);
    void drawObjectsParentedWith(bool overlay,bool transparentObject,int parentObjectID,int displayAttrib,const C4X4Matrix& cameraCTM);
    int addObject(CDrawingObject* it);
    CDrawingObject* getObject(int objectID);
    void announceObjectWillBeErased(int objID);
    void announceScriptStateWillBeErased(int scriptHandle,bool simulationScript,bool sceneSwitchPersistentScript);
    void removeObject(int objectID);
    void adjustForFrameChange(int objectID,const C7Vector& preCorrection);
    void adjustForScaling(int objectID,float xScale,float yScale,float zScale);

    bool getExportableMeshAtIndex(int parentObjectID,int index,std::vector<float>& vertices,std::vector<int>& indices);

private:
    std::vector<CDrawingObject*> _allObjects;
    VMutex _objectMutex;
};
