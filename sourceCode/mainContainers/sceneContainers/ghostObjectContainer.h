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
    int addGhost(int theGroupId,int theObjectHandle,int theOptions,floatDouble theStartTime,floatDouble theEndTime,const floatDouble theColor[12]);
    int removeGhost(int groupId,int ghostId); // -1,-1 to remove all objects
    int modifyGhost(int groupId,int ghostId,int operation,floatDouble floatValue,int theOptions,int theOptionsMask,const floatDouble* colorOrTransformation);
    void announceObjectWillBeErased(int objID);
    void performObjectLoadingMapping(const std::map<int,int>* map);
    void serialize(CSer& ar);

protected:
    std::vector<CGhostObject*> _allObjects;
};
