#pragma once

#include "_collisionObjectContainer_.h"

class CViewableBase;

class CCollisionObjectContainer : public _CCollisionObjectContainer_
{
public:
    CCollisionObjectContainer();
    virtual ~CCollisionObjectContainer();

    void buildUpdateAndPopulateSynchronizationObjects();
    void connectSynchronizationObjects();
    void removeSynchronizationObjects(bool localReferencesToItOnly);

    void simulationAboutToStart();
    void simulationEnded();
    void renderYour3DStuff(CViewableBase* renderingObject,int displayAttrib);
    void displayCollisionContours();
    int addNewObject(int entity1Handle,int entity2Handle,const char* objName);
    void addObject(CCollisionObject* newCollObj,bool objectIsACopy);
    void addObjectWithSuffixOffset(CCollisionObject* newCollObj,bool objectIsACopy,int suffixOffset);
    void getMinAndMaxNameSuffixes(int& minSuffix,int& maxSuffix) const;
    bool canSuffix1BeSetToSuffix2(int suffix1,int suffix2) const;
    void setSuffix1ToSuffix2(int suffix1,int suffix2);
    void removeObject(int objectHandle);

    void removeAllCollisionObjects();
    void announceObjectWillBeErased(int objectHandle);
    void announceCollectionWillBeErased(int collectionHandle);
    void setUpDefaultValues();
    int handleAllCollisions(bool exceptExplicitHandling);
    void resetAllCollisions(bool exceptExplicitHandling);
    int getCollisionColor(int entityID);

protected:
    // Overridden from _CCollisionObjectContainer_:
    void _addObject(CCollisionObject* newCollObj);
    void _removeObject(int objectHandle);
};
