#pragma once

#include "distanceObject_old.h"

class CViewableBase;

class CDistanceObjectContainer_old
{
public:
    CDistanceObjectContainer_old();
    virtual ~CDistanceObjectContainer_old();

    void simulationAboutToStart();
    void simulationEnded();
    void renderYour3DStuff(CViewableBase* renderingObject,int displayAttrib);
    void displayDistanceSegments();
    int addNewObject(int entity1Handle,int entity2Handle,const char* objName);
    void addObject(CDistanceObject_old* newDistObj,bool objectIsACopy);
    void addObjectWithSuffixOffset(CDistanceObject_old* newDistObj,bool objectIsACopy,int suffixOffset);
    void getMinAndMaxNameSuffixes(int& minSuffix,int& maxSuffix) const;
    bool canSuffix1BeSetToSuffix2(int suffix1,int suffix2) const;
    void setSuffix1ToSuffix2(int suffix1,int suffix2);
    void removeObject(int objectHandle);
    void removeAllDistanceObjects();
    void resetAllDistances(bool exceptExplicitHandling);
    void announceObjectWillBeErased(int objectHandle);
    void announceCollectionWillBeErased(int collectionHandle);
    void setUpDefaultValues();
    double handleAllDistances(bool exceptExplicitHandling);

    size_t getObjectCount() const;
    CDistanceObject_old* getObjectFromIndex(size_t index) const;
    CDistanceObject_old* getObjectFromHandle(int objectHandle) const;
    CDistanceObject_old* getObjectFromName(const char* objName) const;

protected:
    void _addObject(CDistanceObject_old* newDistObj);
    void _removeObject(int objectHandle);

    std::vector<CDistanceObject_old*> _distanceObjects;
};

