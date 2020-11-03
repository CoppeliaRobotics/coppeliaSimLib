#pragma once

#include "_collection_.h"

class CCollection : public _CCollection_
{
public:
    CCollection();
    virtual ~CCollection();

    // Overridden from CSyncObject:
    void buildUpdateAndPopulateSynchronizationObject(const std::vector<SSyncRoute>* parentRouting);
    void connectSynchronizationObject();
    void removeSynchronizationObject(bool localReferencesToItOnly);

    void initializeInitialValues(bool simulationAlreadyRunning);
    void simulationAboutToStart();
    void simulationEnded();

    bool actualizeCollection(bool full=true);
    bool isObjectInCollection(int objectHandle) const;
    void addCollectionElement(CCollectionElement* collectionElement);
    void removeCollectionElementFromHandle(int collectionElementHandle);
    void serialize(CSer& ar);
    void performObjectLoadingMapping(const std::vector<int>* map);
    bool announceObjectWillBeErased(int objectHandle,bool copyBuffer);
    CCollection* copyYourself() const;
    void emptyCollection();

    // Overridden from _CCollection_:
    bool setCollectionName(const char* newName,bool check);

    std::string getCollectionLoadName() const;
    std::string getUniquePersistentIdString() const;

    size_t getSceneObjectCountInCollection() const;
    int getSceneObjectHandleFromIndex(size_t index) const;

protected:
    // Overridden from _CCollection_:
    void _addCollectionElement(CCollectionElement* collectionElement);
    void _removeCollectionElementFromHandle(int collectionElementHandle);


private:
    void _updateCollectionObjects_(const std::vector<int>& sceneObjectHandles);

    std::string _uniquePersistentIdString;
    std::vector<int> _collectionObjects;
    std::string _collectionLoadName;
};
