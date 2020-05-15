#pragma once

#include "ghostObject.h"
#include "ser.h"

class CViewableBase;

class CGhostObjectContainer
{
public:
    CGhostObjectContainer();
    virtual ~CGhostObjectContainer();

    void renderYour3DStuff_nonTransparent(CViewableBase* renderingObject,int displayAttrib);
    void renderYour3DStuff_transparent(CViewableBase* renderingObject,int displayAttrib);
    void renderYour3DStuff_overlay(CViewableBase* renderingObject,int displayAttrib);
    int addGhost(int theGroupId,int theObjectHandle,int theOptions,float theStartTime,float theEndTime,const float theColor[12]);
    int removeGhost(int groupId,int ghostId); // -1,-1 to remove all objects
    int modifyGhost(int groupId,int ghostId,int operation,float floatValue,int theOptions,int theOptionsMask,const float* colorOrTransformation);
    void announceObjectWillBeErased(int objID);
    void performObjectLoadingMapping(const std::vector<int>* map);
    void serialize(CSer& ar);

protected:
    std::vector<CGhostObject*> _allObjects;
};
