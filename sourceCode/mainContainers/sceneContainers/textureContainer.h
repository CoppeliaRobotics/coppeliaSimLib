
#pragma once

#include "mainCont.h"
#include "textureObject.h"

class CTextureContainer : public CMainCont 
{
public:
    CTextureContainer();
    virtual ~CTextureContainer();
    void newSceneProcedure();
    void simulationAboutToStart();
    void simulationEnded();
    void renderYour3DStuff(CViewableBase* renderingObject,int displayAttrib);

    CTextureObject* getObject(int objectID);
    CTextureObject* getObject(const char* objectName);
    CTextureObject* getObjectAtIndex(int index);
    int addObject(CTextureObject* anObject,bool objectIsACopy);
    int addObjectWithSuffixOffset(CTextureObject* anObject,bool objectIsACopy,int suffixOffset);
    void getMinAndMaxNameSuffixes(int& minSuffix,int& maxSuffix);
    bool canSuffix1BeSetToSuffix2(int suffix1,int suffix2);
    void setSuffix1ToSuffix2(int suffix1,int suffix2);
    int getSameObjectID(CTextureObject* anObject);
    void removeObject(int objectID);
    void removeAllObjects();

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
