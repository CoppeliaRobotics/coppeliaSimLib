#pragma once

#include <simLib/simConst.h>
#include <collectionElement.h>
#include <propertiesAndMethods.h>

// ----------------------------------------------------------------------------------------------
#define FUNCX(name, str, v1, v2, t1, t2) const SProperty name = {str, v1, v2, t1, t2};
COLLECTION_PROPERTIES
#undef FUNCX
#define FUNCX(name, str, v1, v2, t1, t2) name,
    const std::vector<SProperty> allProps_collection = {COLLECTION_PROPERTIES};
#undef FUNCX
// ----------------------------------------------------------------------------------------------

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
    void performCollectionLoadingMapping(const std::map<int, int>* map, int opType);
    void performObjectLoadingMapping(const std::map<int, int>* map, int opType);
    bool announceObjectWillBeErased(int objectHandle, bool copyBuffer);
    bool announceScriptStateWillBeErased(int scriptHandle, bool simulationScript, bool sceneSwitchPersistentScript);
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
    bool setCollectionName(const char* newName, bool check);
    bool setOverridesObjectMainProperties(bool o);

    std::string getCollectionLoadName() const;
    std::string getUniquePersistentIdString() const;

    size_t getSceneObjectCountInCollection() const;
    int getSceneObjectHandleFromIndex(size_t index) const;
    void pushCreationEvent() const;

    int getHandleProperty(const char* pName, long long int& pState) const;
    int getStringProperty(const char* pName, std::string& pState) const;
    int getHandleArrayProperty(const char* pName, std::vector<long long int>& pState) const;
    static int getPropertyName(int& index, std::string& pName, std::string& appartenance, int excludeFlags);
    static int getPropertyInfo(const char* pName, int& info, std::string& infoTxt);

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
