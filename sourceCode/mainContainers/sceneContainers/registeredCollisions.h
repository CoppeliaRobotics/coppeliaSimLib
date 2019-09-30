
#pragma once

#include "regCollision.h"
#include "mainCont.h"

class CRegisteredCollisions : public CMainCont 
{
public:
    CRegisteredCollisions();
    virtual ~CRegisteredCollisions();

    void simulationAboutToStart();
    void simulationEnded();
    void renderYour3DStuff(CViewableBase* renderingObject,int displayAttrib);
    void displayCollisionContours();
    int addNewObject(int obj1ID,int obj2ID,std::string objName);
    void addObject(CRegCollision* newCollObj,bool objectIsACopy);
    void addObjectWithSuffixOffset(CRegCollision* newCollObj,bool objectIsACopy,int suffixOffset);
    void getMinAndMaxNameSuffixes(int& minSuffix,int& maxSuffix);
    bool canSuffix1BeSetToSuffix2(int suffix1,int suffix2);
    void setSuffix1ToSuffix2(int suffix1,int suffix2);
    bool removeObject(int objID);
    CRegCollision* getObject(int objID);
    CRegCollision* getObject(std::string objName);
    void removeAllCollisionObjects();
    void announceObjectWillBeErased(int objID);
    void announceCollectionWillBeErased(int groupID);
    void setUpDefaultValues();
    int handleAllCollisions(bool exceptExplicitHandling);
    void resetAllCollisions(bool exceptExplicitHandling);
    bool setObjectName(int objID,std::string newName);
    int getCollisionColor(int entityID);

    // Variable that need to be serialized on an individual basis:
    std::vector<CRegCollision*> collisionObjects;
};
