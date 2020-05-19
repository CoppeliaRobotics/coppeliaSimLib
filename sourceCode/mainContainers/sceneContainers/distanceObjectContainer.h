#pragma once

#include "_distanceObjectContainer_.h"

class CViewableBase;

class CDistanceObjectContainer : public _CDistanceObjectContainer_
{
public:
    CDistanceObjectContainer();
    virtual ~CDistanceObjectContainer();

    void buildUpdateAndPopulateSynchronizationObjects();
    void connectSynchronizationObjects();
    void removeSynchronizationObjects(bool localReferencesToItOnly);

    void simulationAboutToStart();
    void simulationEnded();
    void renderYour3DStuff(CViewableBase* renderingObject,int displayAttrib);
    void displayDistanceSegments();
    int addNewObject(int entity1Handle,int entity2Handle,const char* objName);
    void addObject(CDistanceObject* newDistObj,bool objectIsACopy);
    void addObjectWithSuffixOffset(CDistanceObject* newDistObj,bool objectIsACopy,int suffixOffset);
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
    // Overridden from _CDistanceObjectContainer_:
    void _addObject(CDistanceObject* newDistObj);
    void _removeObject(int objectHandle);

};

