#pragma once

#include "collectionElement.h"

class CCollection
{
public:
    CCollection(int creatorHandle);
    virtual ~CCollection();

    void initializeInitialValues(bool simulationAlreadyRunning);
    void simulationAboutToStart();
    void simulationEnded();

    bool actualizeCollection();
    bool isObjectInCollection(int objectHandle) const;
    void addCollectionElement(CCollectionElement* collectionElement);
    void removeCollectionElementFromHandle(int collectionElementHandle);
    void serialize(CSer& ar);
    void performObjectLoadingMapping(const std::vector<int>* map);
    bool announceObjectWillBeErased(int objectHandle,bool copyBuffer);
    bool announceScriptStateWillBeErased(int scriptHandle,bool simulationScript,bool sceneSwitchPersistentScript);
    CCollection* copyYourself() const;
    void emptyCollection();
    int getCreatorHandle() const;
    size_t getElementCount() const;
    CCollectionElement* getElementFromIndex(size_t index) const;
    CCollectionElement* getElementFromHandle(int collectionElementHandle) const;

    int getCollectionHandle() const;
    std::string getCollectionName() const;
    bool getOverridesObjectMainProperties() const;

    bool setCollectionHandle(int newHandle);
    bool setCollectionName(const char* newName,bool check);
    bool setOverridesObjectMainProperties(bool o);

    std::string getCollectionLoadName() const;
    std::string getUniquePersistentIdString() const;

    size_t getSceneObjectCountInCollection() const;
    int getSceneObjectHandleFromIndex(size_t index) const;

protected:
    void _addCollectionElement(CCollectionElement* collectionElement);
    void _removeCollectionElementFromHandle(int collectionElementHandle);

    int _collectionHandle;
    std::string _collectionName;
    bool _overridesObjectMainProperties;

private:
    void _updateCollectionObjects_(const std::vector<int>& sceneObjectHandles);

    std::vector<CCollectionElement*> _collectionElements;
    int _creatorHandle; // -2: old gui items, -1: c-side, otherwise script handle
    std::string _uniquePersistentIdString;
    std::vector<int> _collectionObjects;
    std::string _collectionLoadName;
};
