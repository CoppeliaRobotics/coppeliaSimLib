#pragma once

#include "bannerObject.h"
#include "7Vector.h"
#include "4X4Matrix.h"

class CViewableBase;

class CBannerContainer
{
public:
    CBannerContainer();
    virtual ~CBannerContainer();

    void simulationEnded();
    void renderYour3DStuff_nonTransparent(CViewableBase* renderingObject,int displayAttrib,int windowSize[2],float verticalViewSizeOrAngle,bool perspective);
    void renderYour3DStuff_transparent(CViewableBase* renderingObject,int displayAttrib,int windowSize[2],float verticalViewSizeOrAngle,bool perspective);
    void renderYour3DStuff_overlay(CViewableBase* renderingObject,int displayAttrib,int windowSize[2],float verticalViewSizeOrAngle,bool perspective);
    void drawAll3DStuff(bool overlay,bool transparentObject,int displayAttrib,const C4X4Matrix& cameraCTM,int windowSize[2],float verticalViewSizeOrAngle,bool perspective);
    int addObject(CBannerObject* it);
    CBannerObject* getObject(int objectID);
    void eraseAllObjects(bool onlyThoseCreatedFromScripts);
    void announceObjectWillBeErased(int objID);
    void removeObject(int objectID);
    void adjustForFrameChange(int objectID,const C7Vector& preCorrection);
    void adjustForScaling(int objectID,float xScale,float yScale,float zScale);

private:
    std::vector<CBannerObject*> _allObjects;
};
