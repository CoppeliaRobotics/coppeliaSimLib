#pragma once

#include "collection.h"
#include "shape.h"
#include "dummy.h"
#include "_collectionContainer_.h"

class CCollectionContainer : public _CCollectionContainer_
{
public:
    CCollectionContainer();
    virtual ~CCollectionContainer();

    // Overridden from CSyncObject
    void buildUpdateAndPopulateSynchronizationObjects();
    void connectSynchronizationObjects();

    void simulationAboutToStart();
    void simulationEnded();
    void newScene();
    void actualizeAllCollections();
    void announceObjectWillBeErased(int objectHandle);
    void setUpDefaultValues();
    void removeAllCollections();

    void addCollection(CCollection* collection,bool objectIsACopy);
    void addCollectionWithSuffixOffset(CCollection* theGroup,bool objectIsACopy,int suffixOffset);
    void getMinAndMaxNameSuffixes(int& minSuffix,int& maxSuffix) const;
    bool canSuffix1BeSetToSuffix2(int suffix1,int suffix2) const;
    void setSuffix1ToSuffix2(int suffix1,int suffix2);
    void removeCollection(int collectionHandle);

    bool getShapesAndDummiesFromCollection(int collectionHandle,std::vector<CSceneObject*>* objInCollection,int propMask,bool pathPlanningRoutineCalling) const;
    bool getShapesAndVolumesFromCollection(int collectionHandle,std::vector<CSceneObject*>* objInCollection,int propMask,bool pathPlanningRoutineCalling) const;

    void getCollidableObjectsFromCollection(int collectionHandle,std::vector<CSceneObject*>& objects) const;
    void getMeasurableObjectsFromCollection(int collectionHandle,std::vector<CSceneObject*>& objects) const;
    void getDetectableObjectsFromCollection(int collectionHandle,std::vector<CSceneObject*>& objects,int detectableMask) const;

    void performObjectLoadingMapping(const std::vector<int>* map);

    void addCollectionToSelection(int collectionHandle) const;

protected:
    // Overridden from _CCollectionContainer_:
    void _addCollection(CCollection* collection);
    void _removeCollection(int collectionHandle);

};
