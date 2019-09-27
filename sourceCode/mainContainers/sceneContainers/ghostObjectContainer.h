
#pragma once

#include "vrepMainHeader.h"
#include "ghostObject.h"
#include "mainCont.h"
#include "ser.h"

class CGhostObjectContainer : public CMainCont
{
public:
    CGhostObjectContainer();
    virtual ~CGhostObjectContainer();

    void emptySceneProcedure();
    void simulationAboutToStart();
    void simulationEnded();
    void renderYour3DStuff_nonTransparent(CViewableBase* renderingObject,int displayAttrib);
    void renderYour3DStuff_transparent(CViewableBase* renderingObject,int displayAttrib);
    void renderYour3DStuff_overlay(CViewableBase* renderingObject,int displayAttrib);
    int addGhost(int theGroupId,int theObjectHandle,int theOptions,float theStartTime,float theEndTime,const float theColor[12]);
    int removeGhost(int groupId,int ghostId); // -1,-1 to remove all objects
    int modifyGhost(int groupId,int ghostId,int operation,float floatValue,int theOptions,int theOptionsMask,const float* colorOrTransformation);
    void announceObjectWillBeErased(int objID);
    void performObjectLoadingMapping(std::vector<int>* map);
    void serialize(CSer& ar);

protected:
    std::vector<CGhostObject*> _allObjects;
};
