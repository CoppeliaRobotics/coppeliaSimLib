#pragma once

#include "_distanceObjectContainer_old.h"

class CViewableBase;

class CDistanceObjectContainer_old : public _CDistanceObjectContainer_old
{
public:
    CDistanceObjectContainer_old();
    virtual ~CDistanceObjectContainer_old();

    void buildUpdateAndPopulateSynchronizationObjects();
    void connectSynchronizationObjects();
    void removeSynchronizationObjects(bool localReferencesToItOnly);

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
    float handleAllDistances(bool exceptExplicitHandling);

protected:
    // Overridden from _CDistanceObjectContainer_old:
    void _addObject(CDistanceObject_old* newDistObj);
    void _removeObject(int objectHandle);

};

