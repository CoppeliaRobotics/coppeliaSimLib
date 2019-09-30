
#pragma once

#include "regCollection.h"
#include "shape.h"
#include "dummy.h"
#include "mainCont.h"

class CRegisteredCollections : public CMainCont
{
public:
    CRegisteredCollections();
    virtual ~CRegisteredCollections();

    void simulationAboutToStart();
    void simulationEnded();
    void renderYour3DStuff(CViewableBase* renderingObject,int displayAttrib);
    void newScene();
    void actualizeAllGroups();
    void announceObjectWillBeErased(int objID);
    void setUpDefaultValues();
    void removeAllCollections();
    CRegCollection* getCollection(int groupID);
    CRegCollection* getCollection(std::string groupName);
    void addCollection(CRegCollection* theGroup,bool objectIsACopy);
    void addCollectionWithSuffixOffset(CRegCollection* theGroup,bool objectIsACopy,int suffixOffset);
    void getMinAndMaxNameSuffixes(int& minSuffix,int& maxSuffix);
    bool canSuffix1BeSetToSuffix2(int suffix1,int suffix2);
    void setSuffix1ToSuffix2(int suffix1,int suffix2);
    bool removeCollection(int groupID);
    void unmarkAll();
    void mark(int objID);
    void unmark(int objID);
    bool isObjectInMarkedCollection(int objID);
    void repairMarking();
    bool getShapesAndDummiesFromCollection(int groupID,std::vector<C3DObject*>* objInGroup,int propMask,bool pathPlanningRoutineCalling);
    bool getShapesAndVolumesFromCollection(int groupID,std::vector<C3DObject*>* objInGroup,int propMask,bool pathPlanningRoutineCalling);

    void getCollidableObjectsFromCollection(int collectionID,std::vector<C3DObject*>& objects);
    void getMeasurableObjectsFromCollection(int collectionID,std::vector<C3DObject*>& objects);
    void getDetectableObjectsFromCollection(int collectionID,std::vector<C3DObject*>& objects,int detectableMask);



    void performObjectLoadingMapping(std::vector<int>* map);
    void addCollectionToSelection(int groupID);
    void removeCollectionFromSelection(int groupID);
    void deselectAllCollections();
    bool isCollectionInSelection(int groupID);

    // Variable that need to be serialized on an individual basis:
    std::vector<CRegCollection*> allCollections;
    // Various
    std::vector<int> selectedCollections;
};
