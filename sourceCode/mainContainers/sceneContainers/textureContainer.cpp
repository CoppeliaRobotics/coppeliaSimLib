
#include "simInternal.h"
#include "textureContainer.h"
#include "tt.h"

#include "app.h"

CTextureContainer::CTextureContainer()
{
}

CTextureContainer::~CTextureContainer()
{ // beware, the current world could be nullptr
    eraseAllObjects();
}

CTextureObject* CTextureContainer::getObject(int objectID)
{
    for (size_t i=0;i<_allTextureObjects.size();i++)
    {
        if (_allTextureObjects[i]->getObjectID()==objectID)
            return(_allTextureObjects[i]);
    }
    return(nullptr);
}

CTextureObject* CTextureContainer::getObject(const char* objectName)
{
    for (int i=0;i<int(_allTextureObjects.size());i++)
    {
        if (_allTextureObjects[i]->getObjectName()==objectName)
            return(_allTextureObjects[i]);
    }
    return(nullptr);
}

CTextureObject* CTextureContainer::getObjectAtIndex(int index)
{
    if ( (index<0)||(index>=int(_allTextureObjects.size())) )
        return(nullptr);
    return(_allTextureObjects[index]);
}

void CTextureContainer::getMinAndMaxNameSuffixes(int& minSuffix,int& maxSuffix) const
{
    minSuffix=-1;
    maxSuffix=-1;
    for (int i=0;i<int(_allTextureObjects.size());i++)
    {
        int s=tt::getNameSuffixNumber(_allTextureObjects[i]->getObjectName().c_str(),true);
        if (i==0)
        {
            minSuffix=s;
            maxSuffix=s;
        }
        else
        {
            if (s<minSuffix)
                minSuffix=s;
            if (s>maxSuffix)
                maxSuffix=s;
        }
    }
}

bool CTextureContainer::canSuffix1BeSetToSuffix2(int suffix1,int suffix2) const
{
    for (int i=0;i<int(_allTextureObjects.size());i++)
    {
        int s1=tt::getNameSuffixNumber(_allTextureObjects[i]->getObjectName().c_str(),true);
        if (s1==suffix1)
        {
            std::string name1(tt::getNameWithoutSuffixNumber(_allTextureObjects[i]->getObjectName().c_str(),true));
            for (int j=0;j<int(_allTextureObjects.size());j++)
            {
                int s2=tt::getNameSuffixNumber(_allTextureObjects[j]->getObjectName().c_str(),true);
                if (s2==suffix2)
                {
                    std::string name2(tt::getNameWithoutSuffixNumber(_allTextureObjects[j]->getObjectName().c_str(),true));
                    if (name1==name2)
                        return(false); // NO! We would have a name clash!
                }
            }
        }
    }
    return(true);
}

void CTextureContainer::setSuffix1ToSuffix2(int suffix1,int suffix2)
{
    for (int i=0;i<int(_allTextureObjects.size());i++)
    {
        int s1=tt::getNameSuffixNumber(_allTextureObjects[i]->getObjectName().c_str(),true);
        if (s1==suffix1)
        {
            std::string name1(tt::getNameWithoutSuffixNumber(_allTextureObjects[i]->getObjectName().c_str(),true));
            _allTextureObjects[i]->setObjectName(tt::generateNewName_hash(name1.c_str(),suffix2+1).c_str());
        }
    }
}

int CTextureContainer::addObject(CTextureObject* anObject,bool objectIsACopy)
{ // If object already exists (well, similar object), it is destroyed in here!
    return(addObjectWithSuffixOffset(anObject,objectIsACopy,1));
}

int CTextureContainer::addObjectWithSuffixOffset(CTextureObject* anObject,bool objectIsACopy,int suffixOffset)
{ // If object already exists (well, similar object), it is destroyed in here!
//    printf("TextCnt: %i\n",_allTextureObjects.size());
    CTextureObject* theOldData=_getEquivalentTextureObject(anObject);
    if (theOldData!=nullptr)
    { // we already have a similar object!!
  //      printf("a\n");
        // We transfer the dependencies (since 10/2/2012 (was forgotten before)):
        anObject->transferDependenciesToThere(theOldData);

        delete anObject;
        return(theOldData->getObjectID());
    }

    int newID=SIM_IDSTART_TEXTURE;
    while (getObject(newID)!=nullptr)
        newID++;
    anObject->setObjectID(newID);
    std::string newName(anObject->getObjectName());
    while (getObject(newName.c_str())!=nullptr)
    {
        // TEXTURE OBJECTS SHOULDn'T HAVE A HASHED NAME!!
        newName=tt::generateNewName_noHash(newName.c_str());
    }
    anObject->setObjectName(newName.c_str());
    _allTextureObjects.push_back(anObject);
    return(newID);
}

CTextureObject* CTextureContainer::_getEquivalentTextureObject(CTextureObject *theData)
{
    for (size_t i=0;i<_allTextureObjects.size();i++)
    {
        if (_allTextureObjects[i]->isSame(theData))
            return(_allTextureObjects[i]);
    }
    return(nullptr);
}

void CTextureContainer::removeObject(int objectID)
{
    for (size_t i=0;i<_allTextureObjects.size();i++)
    {
        if (_allTextureObjects[i]->getObjectID()==objectID)
        {
            delete _allTextureObjects[i];
            _allTextureObjects.erase(_allTextureObjects.begin()+i);
            App::setFullDialogRefreshFlag();
            return;
        }
    }
}

void CTextureContainer::clearAllDependencies()
{
    for (int i=0;i<int(_allTextureObjects.size());i++)
        _allTextureObjects[i]->clearAllDependencies();
}

void CTextureContainer::updateAllDependencies()
{ // should not be called from "ct::objCont->addObjectsToSceneAndPerformMapping" routine!!
    clearAllDependencies();
    App::currentWorld->buttonBlockContainer->setTextureDependencies();
    for (size_t i=0;i<App::currentWorld->sceneObjects->getShapeCount();i++)
    {
        CShape* sh=App::currentWorld->sceneObjects->getShapeFromIndex(i);
        if (sh->getMeshWrapper()!=nullptr)
            sh->getMeshWrapper()->setTextureDependencies(sh->getObjectHandle());
    }
}

void CTextureContainer::announceGeneralObjectWillBeErased(int generalObjectID,int subID)
{
    size_t i=0;
    while (i<_allTextureObjects.size())
    {
        if (_allTextureObjects[i]->announceGeneralObjectWillBeErased(generalObjectID,subID))
        {
            removeObject(_allTextureObjects[i]->getObjectID());
            i=0; // ordering may have changed!
        }
        else
            i++;
    }
}

int CTextureContainer::getSameObjectID(CTextureObject* anObject)
{
    for (int i=0;i<int(_allTextureObjects.size());i++)
    {
        if (_allTextureObjects[i]->isSame(anObject))
            return(_allTextureObjects[i]->getObjectID());
    }   
    return(-1);
}

void CTextureContainer::eraseAllObjects()
{
    for (int i=0;i<int(_allTextureObjects.size());i++)
        delete _allTextureObjects[i];
    _allTextureObjects.clear();
}

void CTextureContainer::storeTextureObject(CSer& ar,CTextureObject* it)
{
    if (ar.isBinary())
    {
        ar.storeDataName(SER_TEXTURE);
        ar.setCountingMode();
        it->serialize(ar);
        if (ar.setWritingMode())
            it->serialize(ar);
    }
    else
        it->serialize(ar);
}

CTextureObject* CTextureContainer::loadTextureObject(CSer& ar,std::string theName,bool &noHit)
{
    if (ar.isBinary())
    {
        int byteNumber;
        if (theName.compare(SER_TEXTURE)==0)
        {
            noHit=false;
            ar >> byteNumber; 
            CTextureObject* myNewObject=new CTextureObject();
            myNewObject->serialize(ar);
            return(myNewObject);
        }
    }
    else
    {
        CTextureObject* myNewObject=new CTextureObject();
        myNewObject->serialize(ar);
        return(myNewObject);
    }
    return(nullptr);
}
