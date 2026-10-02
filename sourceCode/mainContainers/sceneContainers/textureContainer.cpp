#include <simInternal.h>
#include <textureContainer.h>
#include <tt.h>
#include <app.h>
#ifdef SIM_WITH_GUI
#include <guiApp.h>
#endif

CTextureContainer::CTextureContainer()
{
}

CTextureContainer::~CTextureContainer()
{ // beware, the current world could be nullptr
    eraseAllObjects();
}

CTextureObject* CTextureContainer::getObject(int objectID) const
{
    for (size_t i = 0; i < _allTextureObjects.size(); i++)
    {
        if (int(_allTextureObjects[i]->getObjectHandle()) == objectID)
            return (_allTextureObjects[i]);
    }
    return nullptr;
}

CTextureObject* CTextureContainer::getObject(const char* objectName) const
{
    for (int i = 0; i < int(_allTextureObjects.size()); i++)
    {
        if (_allTextureObjects[i]->getObjectName() == objectName)
            return (_allTextureObjects[i]);
    }
    return nullptr;
}

CTextureObject* CTextureContainer::getObjectAtIndex(int index) const
{
    if ((index < 0) || (index >= int(_allTextureObjects.size())))
        return nullptr;
    return _allTextureObjects[index];
}

void CTextureContainer::getMinAndMaxNameSuffixes(int& minSuffix, int& maxSuffix) const
{
    minSuffix = -1;
    maxSuffix = -1;
    for (int i = 0; i < int(_allTextureObjects.size()); i++)
    {
        int s = tt::getNameSuffixNumber(_allTextureObjects[i]->getObjectName().c_str(), true);
        if (i == 0)
        {
            minSuffix = s;
            maxSuffix = s;
        }
        else
        {
            if (s < minSuffix)
                minSuffix = s;
            if (s > maxSuffix)
                maxSuffix = s;
        }
    }
}

bool CTextureContainer::canSuffix1BeSetToSuffix2(int suffix1, int suffix2) const
{
    for (int i = 0; i < int(_allTextureObjects.size()); i++)
    {
        int s1 = tt::getNameSuffixNumber(_allTextureObjects[i]->getObjectName().c_str(), true);
        if (s1 == suffix1)
        {
            std::string name1(tt::getNameWithoutSuffixNumber(_allTextureObjects[i]->getObjectName().c_str(), true));
            for (int j = 0; j < int(_allTextureObjects.size()); j++)
            {
                int s2 = tt::getNameSuffixNumber(_allTextureObjects[j]->getObjectName().c_str(), true);
                if (s2 == suffix2)
                {
                    std::string name2(
                        tt::getNameWithoutSuffixNumber(_allTextureObjects[j]->getObjectName().c_str(), true));
                    if (name1 == name2)
                        return (false); // NO! We would have a name clash!
                }
            }
        }
    }
    return (true);
}

void CTextureContainer::setSuffix1ToSuffix2(int suffix1, int suffix2)
{
    for (int i = 0; i < int(_allTextureObjects.size()); i++)
    {
        int s1 = tt::getNameSuffixNumber(_allTextureObjects[i]->getObjectName().c_str(), true);
        if (s1 == suffix1)
        {
            std::string name1(tt::getNameWithoutSuffixNumber(_allTextureObjects[i]->getObjectName().c_str(), true));
            _allTextureObjects[i]->setObjectName(tt::generateNewName_hash(name1.c_str(), suffix2 + 1).c_str());
        }
    }
}

int CTextureContainer::addObject(CTextureObject* anObject, bool objectIsACopy)
{ // If object already exists (well, similar object), it is destroyed in here!
    return (addObjectWithSuffixOffset(anObject, objectIsACopy, 1));
}

int CTextureContainer::addObjectWithSuffixOffset(CTextureObject* anObject, bool objectIsACopy, int suffixOffset)
{ // If object already exists (well, similar object), it is destroyed in here!
    CTextureObject* theOldData = _getEquivalentTextureObject(anObject);
    if (theOldData != nullptr)
    { // we already have a similar object!!
        // We transfer the dependencies (since 10/2/2012 (was forgotten before)):
        anObject->transferDependenciesToThere(theOldData);

        delete anObject;
        return int(theOldData->getObjectHandle());
    }

    int newID = sim_object_texturestart;
    while (getObject(newID) != nullptr)
        newID++;
    anObject->setObjectHandle(newID);
    std::string newName(anObject->getObjectName());
    while (getObject(newName.c_str()) != nullptr)
    {
        // TEXTURE OBJECTS SHOULDn'T HAVE A HASHED NAME!!
        newName = tt::generateNewName_noHash(newName.c_str());
    }
    anObject->setObjectName(newName.c_str());
    _allTextureObjects.push_back(anObject);

    anObject->pushCreationEvent();
    if (App::scenes->getEventsEnabled())
    {
        std::vector<int64_t> handles;
        for (size_t i = 0; i < _allTextureObjects.size(); i++)
            handles.push_back(_allTextureObjects[i]->getObjectHandle());
        const char* cmd = prop(PropScene::textures).name;
        CCbor* ev = App::scenes->createObjectChangedEvent(sim_handle_scene, cmd, true);
        ev->appendKeyHandleArray(cmd, handles.data(), handles.size());
        App::scenes->pushEvent();
    }
    return newID;
}

CTextureObject* CTextureContainer::_getEquivalentTextureObject(CTextureObject* theData)
{
    for (size_t i = 0; i < _allTextureObjects.size(); i++)
    {
        if (_allTextureObjects[i]->isSame(theData))
            return (_allTextureObjects[i]);
    }
    return (nullptr);
}

void CTextureContainer::removeObject(int objectID)
{
    for (size_t i = 0; i < _allTextureObjects.size(); i++)
    {
        if (int(_allTextureObjects[i]->getObjectHandle()) == objectID)
        {
            App::scene->announceTextureWillBeErased(_allTextureObjects[i]);
            delete _allTextureObjects[i];
            _allTextureObjects.erase(_allTextureObjects.begin() + i);
            std::vector<int> remainingT;
            for (size_t j = 0; j < _allTextureObjects.size(); j++)
                remainingT.push_back(_allTextureObjects[j]->getObjectHandle());
            if (App::scenes->getEventsEnabled())
            {
                const char* cmd = prop(PropScene::textures).name;
                CCbor* ev = App::scenes->createObjectChangedEvent(sim_handle_scene, cmd, true);
                ev->appendKeyHandleArray(cmd, remainingT.data(), remainingT.size());
                App::scenes->pushEvent();
                App::scenes->pushRemoveEvent(objectID);
            }
#ifdef SIM_WITH_GUI
            GuiApp::setFullDialogRefreshFlag();
#endif
            break;
        }
    }
}

void CTextureContainer::clearAllDependencies()
{
    for (int i = 0; i < int(_allTextureObjects.size()); i++)
        _allTextureObjects[i]->clearAllDependencies();
}

void CTextureContainer::updateAllDependencies()
{ // should not be called from "ct::objCont->addObjectsToSceneAndPerformMapping" routine!!
    clearAllDependencies();
    for (size_t i = 0; i < App::scene->sceneObjects->getObjectCount(sim_sceneobject_shape); i++)
    {
        CShape* sh = App::scene->sceneObjects->getShapeFromIndex(i);
        if (sh->getMesh() != nullptr)
            sh->getMesh()->setTextureDependencies(sh->getObjectHandle());
    }
}

void CTextureContainer::announceSceneObjectWillBeErased(int objectHandle, int meshHandle)
{
    size_t i = 0;
    while (i < _allTextureObjects.size())
    {
        if (_allTextureObjects[i]->announceSceneObjectWillBeErased(objectHandle, meshHandle))
        {
            removeObject(int(_allTextureObjects[i]->getObjectHandle()));
            i = 0; // ordering may have changed!
        }
        else
            i++;
    }
}

int CTextureContainer::getSameObjectID(CTextureObject* anObject)
{
    for (int i = 0; i < int(_allTextureObjects.size()); i++)
    {
        if (_allTextureObjects[i]->isSame(anObject))
            return int(_allTextureObjects[i]->getObjectHandle());
    }
    return (-1);
}

void CTextureContainer::eraseAllObjects()
{
    for (int i = 0; i < int(_allTextureObjects.size()); i++)
        delete _allTextureObjects[i];
    _allTextureObjects.clear();
}

void CTextureContainer::storeTextureObject(CSer& ar, CTextureObject* it)
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

CTextureObject* CTextureContainer::loadTextureObject(CSer& ar, std::string theName, bool& noHit)
{
    if (ar.isBinary())
    {
        int byteNumber;
        if (theName.compare(SER_TEXTURE) == 0)
        {
            noHit = false;
            ar >> byteNumber;
            CTextureObject* myNewObject = new CTextureObject(16, 16);
            myNewObject->serialize(ar);
            return (myNewObject);
        }
    }
    else
    {
        CTextureObject* myNewObject = new CTextureObject(16, 16);
        myNewObject->serialize(ar);
        return (myNewObject);
    }
    return (nullptr);
}

int CTextureContainer::getBoolProperty_t(int64_t target, const char* pName, bool& pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CTextureObject* it = getObject(int(target));
    if (it != nullptr)
        retVal = it->getBoolProperty(pName, pState);
    return retVal;
}

int CTextureContainer::getLongProperty_t(int64_t target, const char* pName, int64_t& pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CTextureObject* it = getObject(int(target));
    if (it != nullptr)
        retVal = it->getLongProperty(pName, pState);
    return retVal;
}

int CTextureContainer::getStringProperty_t(int64_t target, const char* pName, std::string& pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CTextureObject* it = getObject(int(target));
    if (it != nullptr)
        retVal = it->getStringProperty(pName, pState);
    return retVal;
}

int CTextureContainer::getStringArrayProperty_t(int64_t target, const char* pName, std::vector<std::string>& pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CTextureObject* it = getObject(int(target));
    if (it != nullptr)
        retVal = it->getStringArrayProperty(pName, pState);
    return retVal;
}

int CTextureContainer::getBufferProperty_t(int64_t target, const char* ppName, std::string& pState) const
{
    int retVal = sim_propertyret_unknowntarget;

    CTextureObject* it = getObject(int(target));
    if (it != nullptr)
        retVal = it->getBufferProperty(ppName, pState);
    return retVal;
}

int CTextureContainer::getIntArray2Property_t(int64_t target, const char* ppName, int* pState) const
{
    int retVal = sim_propertyret_unknowntarget;

    CTextureObject* it = getObject(int(target));
    if (it != nullptr)
        retVal = it->getIntArray2Property(ppName, pState);
    return retVal;
}

int CTextureContainer::getHandleArrayProperty_t(int64_t target, const char* pName, std::vector<int64_t>& pState) const
{
    int retVal = sim_propertyret_unknownproperty;
    pState.clear();
    if (target == -1)
    {
        if (strcmp(pName, prop(PropScene::textures).name) == 0)
        {
            for (size_t i = 0; i < _allTextureObjects.size(); i++)
                pState.push_back(_allTextureObjects[i]->getObjectHandle());
            retVal = sim_propertyret_ok;
        }
    }
    return retVal;
}

int CTextureContainer::getPropertyName_t(int64_t target, int& index, std::string& pName, std::string& appartenance, int excludeFlags) const
{
    int retVal = sim_propertyret_unknownproperty;
    if (target != -1)
    {
        CTextureObject* it = getObject(int(target));
        if (it != nullptr)
        {
            appartenance = "textureData";
            return it->getPropertyName(index, pName, appartenance, excludeFlags);
        }
        retVal = -2; // object does not exist
    }
    return retVal;
}

int CTextureContainer::getPropertyInfo_t(int64_t target, const char* pName, int& info, std::string& infoTxt) const
{
    int retVal = sim_propertyret_unknownproperty;
    if (target != -1)
    {
        CTextureObject* it = getObject(int(target));
        if (it != nullptr)
            return it->getPropertyInfo(pName, info, infoTxt);
        retVal = -2; // object does not exist
    }
    return retVal;
}

void CTextureContainer::pushGenesisEvents(const std::vector<CTextureObject*>* ObjectsToConsider /*= nullptr*/) const
{
    if (App::scenes->getEventsEnabled())
    {
        std::vector<CTextureObject*> toConsider;
        std::vector<int> addedTexture;
        std::set<CTextureObject*> alreadyThere;
        if (ObjectsToConsider != nullptr)
        {
            std::set<CTextureObject*> newT;
            for (size_t i = 0; i < ObjectsToConsider->size(); i++)
                newT.insert(ObjectsToConsider->at(i));
            for (size_t i = 0; i < _allTextureObjects.size(); i++)
            {
                CTextureObject* o = _allTextureObjects[i];
                if (newT.find(o) == newT.end())
                {
                    alreadyThere.insert(o);
                    addedTexture.push_back(o->getObjectHandle());
                }
                else
                    toConsider.push_back(o);
            }
        }
        else
            toConsider = _allTextureObjects;
        for (size_t i = 0; i < toConsider.size(); i++)
        {
            CTextureObject* tobj = toConsider[i];
            if (alreadyThere.find(tobj) == alreadyThere.end())
            {
                alreadyThere.insert(tobj);
                tobj->pushCreationEvent();
                addedTexture.push_back(tobj->getObjectHandle());
                const char* cmd = prop(PropScene::textures).name;
                CCbor* ev = App::scenes->createObjectChangedEvent(sim_handle_scene, cmd, true);
                ev->appendKeyHandleArray(cmd, addedTexture.data(), addedTexture.size());
                App::scenes->pushEvent();
            }
        }
    }
}

