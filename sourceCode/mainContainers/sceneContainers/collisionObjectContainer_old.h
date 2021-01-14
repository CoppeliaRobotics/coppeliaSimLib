#pragma once

#include "_collisionObjectContainer_old.h"

class CViewableBase;

class CCollisionObjectContainer_old : public _CCollisionObjectContainer_old
{
public:
    CCollisionObjectContainer_old();
    virtual ~CCollisionObjectContainer_old();

    void buildUpdateAndPopulateSynchronizationObjects();
    void connectSynchronizationObjects();
    void removeSynchronizationObjects(bool localReferencesToItOnly);

    void simulationAboutToStart();
    void simulationEnded();
    void renderYour3DStuff(CViewableBase* renderingObject,int displayAttrib);
    void displayCollisionContours();
    int addNewObject(int entity1Handle,int entity2Handle,const char* objName);
    void addObject(CCollisionObject_old* newCollObj,bool objectIsACopy);
    void addObjectWithSuffixOffset(CCollisionObject_old* newCollObj,bool objectIsACopy,int suffixOffset);
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
    // Overridden from _CCollisionObjectContainer_old:
    void _addObject(CCollisionObject_old* newCollObj);
    void _removeObject(int objectHandle);
};
