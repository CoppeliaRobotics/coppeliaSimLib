#pragma once

#include <textureObject.h>

class CTextureContainer
{
  public:
    CTextureContainer();
    virtual ~CTextureContainer();

    CTextureObject* getObject(int objectID) const;
    CTextureObject* getObject(const char* objectName) const;
    CTextureObject* getObjectAtIndex(int index) const;
    int addObject(CTextureObject* anObject, bool objectIsACopy);
    int addObjectWithSuffixOffset(CTextureObject* anObject, bool objectIsACopy, int suffixOffset);
    void getMinAndMaxNameSuffixes(int& minSuffix, int& maxSuffix) const;
    bool canSuffix1BeSetToSuffix2(int suffix1, int suffix2) const;
    void setSuffix1ToSuffix2(int suffix1, int suffix2);
    int getSameObjectID(CTextureObject* anObject);
    void removeObject(int objectID);
    void eraseAllObjects();
    void pushGenesisEvents(const std::vector<CTextureObject*>* ObjectsToConsider = nullptr) const;

    void storeTextureObject(CSer& ar, CTextureObject* it);
    CTextureObject* loadTextureObject(CSer& ar, std::string theName, bool& noHit);

    void announceSceneObjectWillBeErased(int objectHandle, int meshHandle);
    void clearAllDependencies();
    void updateAllDependencies();

    int getBoolProperty_t(int64_t target, const char* pName, bool& pState) const;
    int getLongProperty_t(int64_t target, const char* pName, int64_t& pState) const;
    int getStringProperty_t(int64_t target, const char* pName, std::string& pState) const;
    int getStringArrayProperty_t(int64_t target, const char* pName, std::vector<std::string>& pState) const;
    int getBufferProperty_t(int64_t target, const char* pName, std::string& pState) const;
    int getIntArray2Property_t(int64_t target, const char* pName, int* pState) const;
    int getHandleArrayProperty_t(int64_t target, const char* pName, std::vector<int64_t>& pState) const;
    int getPropertyName_t(int64_t target, int& index, std::string& pName, std::string& appartenance, int excludeFlags) const;
    int getPropertyInfo_t(int64_t target, const char* pName, int& info, std::string& infoTxt) const;

    // Variable that need to be serialized on an individual basis:
    std::vector<CTextureObject*> _allTextureObjects;

  protected:
    CTextureObject* _getEquivalentTextureObject(CTextureObject* theData);
};
