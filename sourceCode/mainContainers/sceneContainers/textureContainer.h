#pragma once

#include <textureObject.h>

class CTextureContainer 
{
public:
    CTextureContainer();
    virtual ~CTextureContainer();

    CTextureObject* getObject(int objectID);
    CTextureObject* getObject(const char* objectName);
    CTextureObject* getObjectAtIndex(int index);
    int addObject(CTextureObject* anObject,bool objectIsACopy);
    int addObjectWithSuffixOffset(CTextureObject* anObject,bool objectIsACopy,int suffixOffset);
    void getMinAndMaxNameSuffixes(int& minSuffix,int& maxSuffix) const;
    bool canSuffix1BeSetToSuffix2(int suffix1,int suffix2) const;
    void setSuffix1ToSuffix2(int suffix1,int suffix2);
    int getSameObjectID(CTextureObject* anObject);
    void removeObject(int objectID);
    void eraseAllObjects();

    void storeTextureObject(CSer& ar,CTextureObject* it);
    CTextureObject* loadTextureObject(CSer& ar,std::string theName,bool &noHit);

    void announceGeneralObjectWillBeErased(int generalObjectID,int subID);
    void clearAllDependencies();
    void updateAllDependencies();

    // Variable that need to be serialized on an individual basis:
    std::vector<CTextureObject*> _allTextureObjects;
protected:
    CTextureObject* _getEquivalentTextureObject(CTextureObject* theData);
};
