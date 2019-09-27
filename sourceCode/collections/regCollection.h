
#pragma once

#include "vrepMainHeader.h"
#include "regCollectionEl.h"

class CRegCollection
{
public:
    CRegCollection(std::string grName);
    virtual ~CRegCollection();

    void initializeInitialValues(bool simulationIsRunning);
    void simulationAboutToStart();
    void simulationEnded();

    bool actualizeCollection(bool full=true);
    bool isObjectInCollection(int objID);
    CRegCollectionEl* getSubCollection(int subGroupID);
    void addSubCollection(CRegCollectionEl* subGr);
    bool removeSubCollection(int subGroupID);
    void unmarkAll();
    void mark(int objID);
    void unmark(int objID);
    bool areAllMarked();
    void serialize(CSer& ar);
    void performObjectLoadingMapping(std::vector<int>* map);
    bool announceObjectWillBeErased(int objID,bool copyBuffer);
    int getCollectionID();
    void setCollectionID(int newID);
    std::string getCollectionName();
    void setCollectionName(std::string newName);
    bool getOverridesObjectMainProperties();
    void setOverridesObjectMainProperties(bool o);
    CRegCollection* copyYourself();
    void emptyCollection();
    std::string getUniquePersistentIdString() const;

    // Variables which need to be serialized
    std::vector<CRegCollectionEl*> subCollectionList;
    // Other
    std::vector<int> collectionObjects;

private:
    // Variables which need to be serialized
    std::string groupName;
    std::string _uniquePersistentIdString;
    int groupID;
    bool _overridesObjectMainProperties;
    // Other
    std::vector<int> markedObjects;
};
