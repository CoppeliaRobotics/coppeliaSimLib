#pragma once

#include <collection.h>
#include <shape.h>
#include <dummy.h>

// ----------------------------------------------------------------------------------------------
#define FUNCX(name, str, v1, v2, t1, t2) extern const SProperty name;
COLLECTIONCONT_PROPERTIES
#undef FUNCX
extern const std::vector<SProperty> allProps_collCont;
// ----------------------------------------------------------------------------------------------

class CCollectionContainer
{
  public:
    CCollectionContainer();
    virtual ~CCollectionContainer();

    void simulationAboutToStart();
    void simulationEnded();
    void newScene();
    void actualizeAllCollections();
    void announceObjectWillBeErased(int64_t objectHandle);
    void announceScriptStateWillBeErased(int64_t detachedScriptHandle, bool simulationScript, bool sceneSwitchPersistentScript);
    void setUpDefaultValues();
    void removeAllCollections();

    void addCollection(CCollection* collection, bool objectIsACopy);
    void addCollectionWithSuffixOffset(CCollection* theGroup, bool objectIsACopy, int suffixOffset);
    void getMinAndMaxNameSuffixes(int& minSuffix, int& maxSuffix) const;
    bool canSuffix1BeSetToSuffix2(int suffix1, int suffix2) const;
    void setSuffix1ToSuffix2(int suffix1, int suffix2);
    void removeCollection(int64_t collectionHandle);

    size_t getObjectCount() const;
    CCollection* getObjectFromIndex(size_t index) const;
    CCollection* getObjectFromHandle(int64_t collectionHandle) const;
    CCollection* getObjectFromName(const char* collectionName) const;
    void getCollidableObjectsFromCollection(int64_t collectionHandle, std::vector<CSceneObject*>& objects) const;
    void getMeasurableObjectsFromCollection(int64_t collectionHandle, std::vector<CSceneObject*>& objects) const;
    void getDetectableObjectsFromCollection(int64_t collectionHandle, std::vector<CSceneObject*>& objects, int detectableMask) const;

    void performObjectLoadingMapping(const std::map<int, int>* map);

    void addCollectionToSelection(int64_t collectionHandle) const;
    void pushGenesisEvents() const;

    int getBoolProperty_t(int64_t target, const char* pName, bool& pState) const;
    int getLongProperty_t(int64_t target, const char* pName, int64_t& pState) const;
    int getHandleProperty_t(int64_t target, const char* pName, int64_t& pState) const;
    int getStringProperty_t(int64_t target, const char* pName, std::string& pState) const;
    int getHandleArrayProperty_t(int64_t target, const char* pName, std::vector<int64_t>& pState) const;
    int getStringArrayProperty_t(int64_t target, const char* pName, std::vector<std::string>& pState) const;
    int getPropertyName_t(int64_t target, int& index, std::string& pName, std::string& appartenance, int excludeFlags) const;
    int getPropertyInfo_t(int64_t target, const char* pName, int& info, std::string& infoTxt) const;

  protected:
    void _addCollection(CCollection* collection);
    void _removeCollection(int64_t collectionHandle);

    std::vector<CCollection*> _allCollections;
};
