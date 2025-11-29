#pragma once

#include <collection.h>
#include <shape.h>
#include <dummy.h>

// ----------------------------------------------------------------------------------------------
#define FUNCX(name, str, v1, v2, t1, t2) const SProperty name = {str, v1, v2, t1, t2};
COLLECTIONCONT_PROPERTIES
#undef FUNCX
#define FUNCX(name, str, v1, v2, t1, t2) name,
    const std::vector<SProperty> allProps_collCont = {COLLECTIONCONT_PROPERTIES};
#undef FUNCX
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
    void announceObjectWillBeErased(int objectHandle);
    void announceScriptStateWillBeErased(int scriptHandle, bool simulationScript, bool sceneSwitchPersistentScript);
    void setUpDefaultValues();
    void removeAllCollections();

    void addCollection(CCollection* collection, bool objectIsACopy);
    void addCollectionWithSuffixOffset(CCollection* theGroup, bool objectIsACopy, int suffixOffset);
    void getMinAndMaxNameSuffixes(int& minSuffix, int& maxSuffix) const;
    bool canSuffix1BeSetToSuffix2(int suffix1, int suffix2) const;
    void setSuffix1ToSuffix2(int suffix1, int suffix2);
    void removeCollection(int collectionHandle);

    size_t getObjectCount() const;
    CCollection* getObjectFromIndex(size_t index) const;
    CCollection* getObjectFromHandle(int collectionHandle) const;
    CCollection* getObjectFromName(const char* collectionName) const;
    void getCollidableObjectsFromCollection(int collectionHandle, std::vector<CSceneObject*>& objects) const;
    void getMeasurableObjectsFromCollection(int collectionHandle, std::vector<CSceneObject*>& objects) const;
    void getDetectableObjectsFromCollection(int collectionHandle, std::vector<CSceneObject*>& objects,
                                            int detectableMask) const;

    void performObjectLoadingMapping(const std::map<int, int>* map);

    void addCollectionToSelection(int collectionHandle) const;
    void pushGenesisEvents() const;

    int getHandleProperty(long long int target, const char* pName, long long int& pState) const;
    int getStringProperty(long long int target, const char* pName, std::string& pState) const;
    int getHandleArrayProperty(long long int target, const char* pName, std::vector<long long int>& pState) const;
    int getPropertyName(long long int target, int& index, std::string& pName, std::string& appartenance, int excludeFlags) const;
    int getPropertyInfo(long long int target, const char* pName, int& info, std::string& infoTxt) const;

  protected:
    void _addCollection(CCollection* collection);
    void _removeCollection(int collectionHandle);

    std::vector<CCollection*> _allCollections;
};
