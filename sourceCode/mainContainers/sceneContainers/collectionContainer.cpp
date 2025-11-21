#include <collectionContainer.h>
#include <app.h>
#include <tt.h>
#include <utils.h>
#ifdef SIM_WITH_GUI
#include <guiApp.h>
#endif

CCollectionContainer::CCollectionContainer()
{
}

CCollectionContainer::~CCollectionContainer()
{ // beware, the current world could be nullptr
    while (_allCollections.size() != 0)
        _removeCollection(_allCollections[0]->getCollectionHandle());
}

void CCollectionContainer::simulationAboutToStart()
{
    for (size_t i = 0; i < getObjectCount(); i++)
        getObjectFromIndex(i)->simulationAboutToStart();
}

void CCollectionContainer::simulationEnded()
{
    for (size_t i = 0; i < getObjectCount(); i++)
        getObjectFromIndex(i)->simulationEnded();
}

void CCollectionContainer::newScene()
{
    removeAllCollections();
}

void CCollectionContainer::announceObjectWillBeErased(int objectHandle)
{ // Never called from copy buffer!
    size_t i = 0;
    while (i < getObjectCount())
    {
        CCollection* coll = getObjectFromIndex(i);
        if (coll->announceObjectWillBeErased(objectHandle, false))
        {
            if (coll->getCreatorHandle() == -2)                // Only old-type collections will be removed (those created via the GUI)
                removeCollection(coll->getCollectionHandle()); // This will call announceCollectionWillBeErased!!
            else
                i++;
        }
        else
            i++;
    }
}

void CCollectionContainer::announceScriptStateWillBeErased(int scriptHandle, bool simulationScript,
                                                           bool sceneSwitchPersistentScript)
{
    size_t i = 0;
    while (i < getObjectCount())
    {
        CCollection* coll = getObjectFromIndex(i);
        if (coll->announceScriptStateWillBeErased(scriptHandle, simulationScript, sceneSwitchPersistentScript))
            removeCollection(coll->getCollectionHandle()); // This will call announceCollectionWillBeErased!!
        else
            i++;
    }
}

void CCollectionContainer::actualizeAllCollections()
{
    size_t i = 0;
    while (i < getObjectCount())
    {
        CCollection* coll = getObjectFromIndex(i);
        if (!coll->actualizeCollection())
        {
            if (coll->getCreatorHandle() == -2) // Only old-type collections will be removed (those created via the GUI)
                removeCollection(coll->getCollectionHandle());
            else
                i++;
        }
        else
            i++;
    }
}

void CCollectionContainer::getCollidableObjectsFromCollection(int collectionHandle,
                                                              std::vector<CSceneObject*>& objects) const
{
    objects.clear();
    CCollection* theGroup = getObjectFromHandle(collectionHandle);
    if (theGroup != nullptr)
    {
        bool overridePropertyFlags = theGroup->getOverridesObjectMainProperties();
        for (size_t i = 0; i < theGroup->getSceneObjectCountInCollection(); i++)
        {
            CSceneObject* anObject =
                App::currentWorld->sceneObjects->getObjectFromHandle(theGroup->getSceneObjectHandleFromIndex(i));
            if ((anObject != nullptr) && (anObject->isPotentiallyCollidable()))
            {
                if ((anObject->getCumulativeObjectSpecialProperty() & sim_objectspecialproperty_collidable) ||
                    overridePropertyFlags)
                    objects.push_back(anObject);
            }
        }
    }
}

void CCollectionContainer::getMeasurableObjectsFromCollection(int collectionHandle,
                                                              std::vector<CSceneObject*>& objects) const
{
    objects.clear();
    CCollection* theGroup = getObjectFromHandle(collectionHandle);
    if (theGroup != nullptr)
    {
        bool overridePropertyFlags = theGroup->getOverridesObjectMainProperties();
        for (size_t i = 0; i < theGroup->getSceneObjectCountInCollection(); i++)
        {
            CSceneObject* anObject =
                App::currentWorld->sceneObjects->getObjectFromHandle(theGroup->getSceneObjectHandleFromIndex(i));
            if ((anObject != nullptr) && (anObject->isPotentiallyMeasurable()))
            {
                if ((anObject->getCumulativeObjectSpecialProperty() & sim_objectspecialproperty_measurable) ||
                    overridePropertyFlags)
                    objects.push_back(anObject);
            }
        }
    }
}

void CCollectionContainer::getDetectableObjectsFromCollection(int collectionHandle,
                                                              std::vector<CSceneObject*>& objects,
                                                              int detectableMask) const
{
    objects.clear();
    CCollection* theGroup = getObjectFromHandle(collectionHandle);
    if (theGroup != nullptr)
    {
        bool overridePropertyFlags = theGroup->getOverridesObjectMainProperties() || (detectableMask == -1);
        for (size_t i = 0; i < theGroup->getSceneObjectCountInCollection(); i++)
        {
            CSceneObject* anObject =
                App::currentWorld->sceneObjects->getObjectFromHandle(theGroup->getSceneObjectHandleFromIndex(i));
            if ((anObject != nullptr) && (anObject->isPotentiallyDetectable()))
            {
                if ((anObject->getCumulativeObjectSpecialProperty() & detectableMask) || overridePropertyFlags)
                    objects.push_back(anObject);
            }
        }
    }
}

void CCollectionContainer::removeAllCollections()
{
    while (getObjectCount() != 0)
        removeCollection(getObjectFromIndex(0)->getCollectionHandle());
}

void CCollectionContainer::setUpDefaultValues()
{
    removeAllCollections();
}

void CCollectionContainer::removeCollection(int collectionHandle)
{
    App::currentWorld->announceCollectionWillBeErased(collectionHandle);
    _removeCollection(collectionHandle);
#ifdef SIM_WITH_GUI
    GuiApp::setFullDialogRefreshFlag();
#endif
}

void CCollectionContainer::getMinAndMaxNameSuffixes(int& minSuffix, int& maxSuffix) const
{
    minSuffix = -1;
    maxSuffix = -1;
    for (size_t i = 0; i < getObjectCount(); i++)
    {
        int s = tt::getNameSuffixNumber(getObjectFromIndex(i)->getCollectionName().c_str(), true);
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

bool CCollectionContainer::canSuffix1BeSetToSuffix2(int suffix1, int suffix2) const
{
    for (size_t i = 0; i < getObjectCount(); i++)
    {
        int s1 = tt::getNameSuffixNumber(getObjectFromIndex(i)->getCollectionName().c_str(), true);
        if (s1 == suffix1)
        {
            std::string name1(tt::getNameWithoutSuffixNumber(getObjectFromIndex(i)->getCollectionName().c_str(), true));
            for (size_t j = 0; j < getObjectCount(); j++)
            {
                int s2 = tt::getNameSuffixNumber(getObjectFromIndex(j)->getCollectionName().c_str(), true);
                if (s2 == suffix2)
                {
                    std::string name2(
                        tt::getNameWithoutSuffixNumber(getObjectFromIndex(j)->getCollectionName().c_str(), true));
                    if (name1 == name2)
                        return false; // NO! We would have a name clash!
                }
            }
        }
    }
    return true;
}

void CCollectionContainer::setSuffix1ToSuffix2(int suffix1, int suffix2)
{
    for (size_t i = 0; i < getObjectCount(); i++)
    {
        int s1 = tt::getNameSuffixNumber(getObjectFromIndex(i)->getCollectionName().c_str(), true);
        if (s1 == suffix1)
        {
            std::string name1(tt::getNameWithoutSuffixNumber(getObjectFromIndex(i)->getCollectionName().c_str(), true));
            getObjectFromIndex(i)->setCollectionName(tt::generateNewName_hash(name1.c_str(), suffix2 + 1).c_str(),
                                                     false);
        }
    }
}

void CCollectionContainer::addCollection(CCollection* collection, bool objectIsACopy)
{
    addCollectionWithSuffixOffset(collection, objectIsACopy, 1);
}

void CCollectionContainer::addCollectionWithSuffixOffset(CCollection* collection, bool objectIsACopy, int suffixOffset)
{
    std::string theName = collection->getCollectionName();
    if (theName.length() == 0)
        theName = "Collection";
    if (objectIsACopy)
        theName = tt::generateNewName_hash(theName.c_str(), suffixOffset);
    while (getObjectFromName(theName.c_str()) != nullptr)
        theName = tt::generateNewName_hashOrNoHash(theName.c_str(), objectIsACopy);
    collection->setCollectionName(theName.c_str(), false);

    int handle = SIM_IDSTART_COLLECTION;
    while (getObjectFromHandle(handle) != nullptr)
        handle++;
    collection->setCollectionHandle(handle);

    _addCollection(collection);

#ifdef SIM_WITH_GUI
    GuiApp::setFullDialogRefreshFlag();
#endif
}

void CCollectionContainer::addCollectionToSelection(int collectionHandle) const
{
    CCollection* it = getObjectFromHandle(collectionHandle);
    if (it != nullptr)
    {
        for (size_t i = 0; i < it->getSceneObjectCountInCollection(); i++)
            App::currentWorld->sceneObjects->addObjectToSelection(it->getSceneObjectHandleFromIndex(i));
    }
}

int CCollectionContainer::getStringProperty(long long int target, const char* pName, std::string& pState) const
{
    int retVal = -1;
    if (target == -1)
    {
    }
    else
    {
        CCollection* it = getObjectFromHandle(int(target));
        if (it != nullptr)
            return it->getStringProperty(pName, pState);
        retVal = -2; // collection does not exist
    }
    return retVal;
}

int CCollectionContainer::getHandleArrayProperty(long long int target, const char* pName, std::vector<long long int>& pState) const
{
    int retVal = -1;
    pState.clear();
    if (target == -1)
    {
        if (strcmp(pName, propCollCont_collections.name) == 0)
        {
            for (size_t i = 0; i < _allCollections.size(); i++)
                pState.push_back(_allCollections[i]->getCollectionHandle());
            retVal = 1;
        }
    }
    else
    {
        CCollection* it = getObjectFromHandle(int(target));
        if (it != nullptr)
            return it->getHandleArrayProperty(pName, pState);
        retVal = -2; // collection does not exist
    }
    return retVal;
}

int CCollectionContainer::getPropertyName(long long int target, int& index, std::string& pName, std::string& appartenance, int excludeFlags) const
{
    int retVal = -1;
    if (target == -1)
    {
        for (size_t i = 0; i < allProps_collCont.size(); i++)
        {
            if ((pName.size() == 0) || utils::startsWith(allProps_collCont[i].name, pName.c_str()))
            {
                if ((allProps_collCont[i].flags & excludeFlags) == 0)
                {
                    index--;
                    if (index == -1)
                    {
                        pName = allProps_collCont[i].name;
                        retVal = 1;
                        break;
                    }
                }
            }
        }
    }
    else
    {
        CCollection* it = getObjectFromHandle(int(target));
        if (it != nullptr)
        {
            appartenance = "collection";
            return it->getPropertyName(index, pName, appartenance, excludeFlags);
        }
        retVal = -2; // object does not exist
    }
    return retVal;
}

int CCollectionContainer::getPropertyInfo(long long int target, const char* pName, int& info, std::string& infoTxt) const
{
    int retVal = -1;
    if (target == -1)
    {
        for (size_t i = 0; i < allProps_collCont.size(); i++)
        {
            if (strcmp(allProps_collCont[i].name, pName) == 0)
            {
                retVal = allProps_collCont[i].type;
                info = allProps_collCont[i].flags;
                if ((infoTxt == "") && (strcmp(allProps_collCont[i].infoTxt, "") != 0))
                    infoTxt = allProps_collCont[i].infoTxt;
                else
                    infoTxt = allProps_collCont[i].shortInfoTxt;
                break;
            }
        }
    }
    else
    {
        CCollection* it = getObjectFromHandle(int(target));
        if (it != nullptr)
            return it->getPropertyInfo(pName, info, infoTxt);
        retVal = -2; // object does not exist
    }
    return retVal;
}

void CCollectionContainer::performObjectLoadingMapping(const std::map<int, int>* map)
{
    for (size_t i = 0; i < getObjectCount(); i++)
        getObjectFromIndex(i)->performObjectLoadingMapping(map, 0);
}

void CCollectionContainer::_removeCollection(int collectionHandle)
{
    for (size_t i = 0; i < _allCollections.size(); i++)
    {
        if (_allCollections[i]->getCollectionHandle() == collectionHandle)
        {
            if (App::worldContainer->getEventsEnabled())
            {
                App::worldContainer->createEvent(EVENTTYPE_COLLECTIONREMOVED, -1, collectionHandle, nullptr, false);
                App::worldContainer->pushEvent();
            }
            delete _allCollections[i];
            _allCollections.erase(_allCollections.begin() + i);
            break;
        }
    }
}

void CCollectionContainer::_addCollection(CCollection* collection)
{
    _allCollections.push_back(collection);
    collection->pushCreationEvent();

    if (App::worldContainer->getEventsEnabled())
    {
        std::vector<int> handles;
        for (size_t i = 0; i < _allCollections.size(); i++)
            handles.push_back(_allCollections[i]->getCollectionHandle());
        const char* cmd = propCollCont_collections.name;
        CCbor* ev = App::worldContainer->createObjectChangedEvent(sim_handle_scene, cmd, true);
        ev->appendKeyIntArray(cmd, handles.data(), handles.size());
        App::worldContainer->pushEvent();
    }
}

size_t CCollectionContainer::getObjectCount() const
{
    return _allCollections.size();
}

CCollection* CCollectionContainer::getObjectFromIndex(size_t index) const
{
    CCollection* retVal = nullptr;
    if (index < _allCollections.size())
        retVal = _allCollections[index];
    return retVal;
}

CCollection* CCollectionContainer::getObjectFromHandle(int collectionHandle) const
{
    CCollection* retVal = nullptr;
    for (size_t i = 0; i < _allCollections.size(); i++)
    {
        if (_allCollections[i]->getCollectionHandle() == collectionHandle)
        {
            retVal = _allCollections[i];
            break;
        }
    }
    return retVal;
}

CCollection* CCollectionContainer::getObjectFromName(const char* collectionName) const
{
    CCollection* retVal = nullptr;
    for (size_t i = 0; i < _allCollections.size(); i++)
    {
        if (_allCollections[i]->getCollectionName().compare(collectionName) == 0)
        {
            retVal = _allCollections[i];
            break;
        }
    }
    return retVal;
}

void CCollectionContainer::pushGenesisEvents() const
{
    std::vector<int> addedCollections;
    for (size_t i = 0; i < _allCollections.size(); i++)
    {
        CCollection* coll = _allCollections[i];
        coll->pushCreationEvent();

        // We need to "fake" adding that collection:
        addedCollections.push_back(coll->getCollectionHandle());
        const char* cmd = propCollCont_collections.name;
        CCbor* ev = App::worldContainer->createObjectChangedEvent(sim_handle_scene, cmd, true);
        ev->appendKeyIntArray(cmd, addedCollections.data(), addedCollections.size());
        App::worldContainer->pushEvent();
    }
}

