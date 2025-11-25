#include <collection.h>
#include <app.h>
#include <base64.h>
#include <tt.h>
#include <utils.h>
#include <global.h>
#ifdef SIM_WITH_GUI
#include <guiApp.h>
#endif

static std::string OBJECT_TYPE = "collection";
static std::string OBJECT_META_INFO = R"(
{
    "superclass": "object",
    "namespaces": {
    },
    "methods": {
        "addItem": "sim-2.addToCollection",
        "removeItem": "sim-2.removeFromCollection",
        "checkCollision": "sim-2.checkCollision",
        "checkDistance": "sim-2.checkDistance",
        "remove": "sim-2.removeCollection",
        )" APP_META_METHODS R"(
    }
}
)";

CCollection::CCollection(int creatorHandle)
{
    _collectionHandle = -1;
    _creatorHandle = creatorHandle;
    _overridesObjectMainProperties = false;
    _uniquePersistentIdString = utils::generateUniqueAlphaNumericString();
}

CCollection::~CCollection()
{
    while (_collectionElements.size() > 0)
        _removeCollectionElementFromHandle(_collectionElements[0]->getElementHandle());
}

void CCollection::initializeInitialValues(bool simulationAlreadyRunning)
{ // is called at simulation start, but also after object(s) have been copied into a scene!
    //_initialValuesInitialized=true;
}

void CCollection::simulationAboutToStart()
{
    initializeInitialValues(false);
}

void CCollection::simulationEnded()
{ // Remember, this is not guaranteed to be run! (the object can be copied during simulation, and pasted after it
    // ended). For thoses situations there is the initializeInitialValues routine!
    // if (_initialValuesInitialized)
    //{
    //}
    //_initialValuesInitialized=false;
}

bool CCollection::isObjectInCollection(int objectHandle) const
{
    for (size_t i = 0; i < _collectionObjects.size(); i++)
    {
        if (_collectionObjects[i] == objectHandle)
            return true;
    }
    return false;
}

void CCollection::addCollectionElement(CCollectionElement* collectionElement)
{
    int i = 0;
    while (getElementFromHandle(i) != nullptr)
        i++;
    collectionElement->setElementHandle(i);
    _addCollectionElement(collectionElement);
    actualizeCollection();
#ifdef SIM_WITH_GUI
    GuiApp::setFullDialogRefreshFlag();
#endif
}

bool CCollection::actualizeCollection()
{ // return value false means that this collection is empty
    // First we remove all collection elements which are not valid anymore:
    {
        size_t i = 0;
        while (i < getElementCount())
        {
            CSceneObject* it =
                App::currentWorld->sceneObjects->getObjectFromHandle(getElementFromIndex(i)->getMainObject());
            if (it == nullptr)
            {
                if (getElementFromIndex(i)->getElementType() != sim_collectionelement_all)
                    _removeCollectionElementFromHandle(getElementFromIndex(i)->getElementHandle());
                else
                    i++;
            }
            else
                i++;
        }
    }
    // Now we have to take care of the GROUP_EVERYTHING type:
    if (_creatorHandle == -2)
    { // only for old collections (i.e. those created via the GUI)
        bool removeAll = true;
        for (size_t i = 0; i < getElementCount(); i++)
        {
            if (getElementFromIndex(i)->getElementType() != sim_collectionelement_all)
                removeAll = false;
        }
        if (removeAll)
        {
            while (getElementCount() > 0)
                _removeCollectionElementFromHandle(getElementFromIndex(0)->getElementHandle());
        }
    }
    bool retVal = (getElementCount() != 0);
//    if (retVal)
//    {
    // Now we set-up the object list:
    std::vector<int> objs;
    for (size_t i = 0; i < getElementCount(); i++)
        getElementFromIndex(i)->addOrRemoveYourObjects(&objs);
    _updateCollectionObjects_(objs);
//    }
    return retVal;
}

void CCollection::removeCollectionElementFromHandle(int collectionElementHandle)
{
    _removeCollectionElementFromHandle(collectionElementHandle);
    actualizeCollection();
}

bool CCollection::announceScriptStateWillBeErased(int scriptHandle, bool simulationScript,
                                                  bool sceneSwitchPersistentScript)
{ // Return value true means that this collection needs to be erased
    return ((!sceneSwitchPersistentScript) && (_creatorHandle == scriptHandle));
}

bool CCollection::announceObjectWillBeErased(int objectHandle, bool copyBuffer)
{ // Return value true means that this collection is empty
    bool retVal = false;
    size_t initialSubGroupListSize = getElementCount();
    {
        size_t i = 0;
        while (i < getElementCount())
        {
            if ((getElementFromIndex(i)->getMainObject() ==
                 objectHandle)) //  GROUP_EVERYTHING is handled a little bit further down
            {
                _removeCollectionElementFromHandle(getElementFromIndex(i)->getElementHandle());
                i = 0;
            }
            else
                i++;
        }
    }

    // Now we have to take care of the GROUP_EVERYTHING type:
    bool removeAll = true;
    for (size_t i = 0; i < getElementCount(); i++)
    {
        if (getElementFromIndex(i)->getElementType() != sim_collectionelement_all)
            removeAll = false;
    }
    if (removeAll)
    {
        while (getElementCount() > 0)
            _removeCollectionElementFromHandle(getElementFromIndex(0)->getElementHandle());
    }

    if ((getElementCount() != initialSubGroupListSize) && copyBuffer)
        retVal = true; // (during copy/paste, only intact collections should be copied!)
    else
        retVal = (getElementCount() == 0);

    if (!retVal)
    { // We also remove that object from the object list:
        std::vector<int> objs(_collectionObjects);
        for (size_t i = 0; i < objs.size(); i++)
        {
            if (objs[i] == objectHandle)
            {
                objs.erase(objs.begin() + i);
                break;
            }
        }
        _updateCollectionObjects_(objs);
    }

    return retVal;
}

bool CCollection::setCollectionName(const char* newName, bool check)
{
    CCollection* it = nullptr;
    if (check)
        it = App::currentWorld->collections->getObjectFromHandle(_collectionHandle);
    std::string nn;
    if (it != this)
        nn = newName;
    else
    { // object is in world
        std::string nm(newName);
        tt::removeIllegalCharacters(nm, true);
        if (nm.size() > 0)
        {
            if (getCollectionName() != nm)
            {
                while (App::currentWorld->collections->getObjectFromName(nm.c_str()) != nullptr)
                    nm = tt::generateNewName_hashOrNoHash(nm.c_str(), !tt::isHashFree(nm.c_str()));
                nn = nm;
            }
        }
    }
    bool diff = false;
    if (nn.size() > 0)
    {
        diff = (_collectionName != nn);
        if (diff)
            _collectionName = nn;
    }

#ifdef SIM_WITH_GUI
    if (diff)
        GuiApp::setFullDialogRefreshFlag();
#endif
    return (diff);
}

void CCollection::performCollectionLoadingMapping(const std::map<int, int>* map, int opType)
{
    if (opType == 3)
        _collectionHandle = CWorld::getLoadingMapping(map, _collectionHandle); // model save
}

void CCollection::performObjectLoadingMapping(const std::map<int, int>* map, int opType)
{
    for (size_t i = 0; i < getElementCount(); i++)
        getElementFromIndex(i)->performObjectLoadingMapping(map);
    if (opType <= 1) // i.e. scene or model loading
        actualizeCollection();
}

CCollection* CCollection::copyYourself() const
{
    CCollection* newCollection = new CCollection(-2);
    newCollection->_collectionHandle = _collectionHandle; // important for copy operations connections
    newCollection->_collectionName = _collectionName;
    for (size_t i = 0; i < getElementCount(); i++)
        newCollection->_addCollectionElement(getElementFromIndex(i)->copyYourself());
    newCollection->_overridesObjectMainProperties = _overridesObjectMainProperties;
    return (newCollection);
}

void CCollection::emptyCollection()
{
    while (getElementCount() > 0)
        _removeCollectionElementFromHandle(getElementFromIndex(0)->getElementHandle());
    //_collectionObjects.clear();
    actualizeCollection();
}

int CCollection::getCreatorHandle() const
{
    return _creatorHandle;
}

std::string CCollection::getUniquePersistentIdString() const
{
    return _uniquePersistentIdString;
}

size_t CCollection::getSceneObjectCountInCollection() const
{
    return _collectionObjects.size();
}

int CCollection::getSceneObjectHandleFromIndex(size_t index) const
{
    int retVal = -1;
    if (index < _collectionObjects.size())
        retVal = _collectionObjects[index];
    return retVal;
}

int CCollection::getHandleProperty(const char* ppName, long long int& pState) const
{
    std::string _pName(ppName);
    int retVal = -1;

    if (_pName == propCollection_handle.name)
    {
        retVal = 1;
        pState = _collectionHandle;
    }

    return retVal;
}

int CCollection::getStringProperty(const char* ppName, std::string& pState) const
{
    std::string _pName(ppName);
    int retVal = -1;

    if (_pName == propCollection_objectType.name)
    {
        retVal = 1;
        pState = OBJECT_TYPE;
    }
    else if (_pName == propCollection_objectMetaInfo.name)
    {
        retVal = 1;
        pState = OBJECT_META_INFO;
    }

    return retVal;
}

int CCollection::getHandleArrayProperty(const char* ppName, std::vector<long long int>& pState) const
{
    std::string _pName(ppName);
    const char* pName = _pName.c_str();
    int retVal = -1;
    pState.clear();

    if (strcmp(pName, propCollection_objects.name) == 0)
    {
        for (size_t i = 0; i < _collectionObjects.size(); i++)
            pState.push_back(_collectionObjects[i]);
        retVal = 1;
    }

    return retVal;
}

int CCollection::getPropertyName(int& index, std::string& pName, std::string& appartenance, int excludeFlags)
{
    int retVal = -1;
    for (size_t i = 0; i < allProps_collection.size(); i++)
    {
        if ((pName.size() == 0) || utils::startsWith(allProps_collection[i].name, pName.c_str()))
        {
            if ((allProps_collection[i].flags & excludeFlags) == 0)
            {
                index--;
                if (index == -1)
                {
                    pName = allProps_collection[i].name;
                    retVal = 1;
                    break;
                }
            }
        }
    }
    return retVal;
}

int CCollection::getPropertyInfo(const char* ppName, int& info, std::string& infoTxt)
{
    const char* pName = ppName;
    int retVal = -1;
    for (size_t i = 0; i < allProps_collection.size(); i++)
    {
        if (strcmp(allProps_collection[i].name, pName) == 0)
        {
            retVal = allProps_collection[i].type;
            info = allProps_collection[i].flags;
            if (infoTxt == "j")
                infoTxt = allProps_collection[i].shortInfoTxt;
            else
            {
                auto w = QJsonDocument::fromJson(allProps_collection[i].shortInfoTxt).object();
                std::string descr = w["description"].toString().toStdString();
                std::string label = w["label"].toString().toStdString();
                if ( (infoTxt == "s") || (descr == "") )
                    infoTxt = label;
                else
                    infoTxt = descr;
            }
            break;
        }
    }
    return retVal;
}

void CCollection::serialize(CSer& ar)
{
    if (ar.isBinary())
    {
        if (ar.isStoring())
        { // Storing
            ar.storeDataName("Grn");
            ar << _collectionName;
            ar.flush();

            ar.storeDataName("Gix");
            ar << _collectionHandle;
            ar.flush();

            ar.storeDataName("Par");
            unsigned char nothing = 0;
            SIM_SET_CLEAR_BIT(nothing, 0, _overridesObjectMainProperties);
            ar << nothing;
            ar.flush();

            ar.storeDataName("Uis");
            ar << _uniquePersistentIdString;
            ar.flush();

            for (size_t i = 0; i < getElementCount(); i++)
            {
                ar.storeDataName("Asg");
                ar.setCountingMode();
                getElementFromIndex(i)->serialize(ar);
                if (ar.setWritingMode())
                    getElementFromIndex(i)->serialize(ar);
            }

            ar.storeDataName(SER_END_OF_OBJECT);
        }
        else
        { // Loading
            int byteQuantity;
            std::string theName = "";
            while (theName.compare(SER_END_OF_OBJECT) != 0)
            {
                theName = ar.readDataName();
                if (theName.compare(SER_END_OF_OBJECT) != 0)
                {
                    bool noHit = true;
                    if (theName.compare("Grn") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _collectionName;
                    }
                    if (theName.compare("Gix") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _collectionHandle;
                    }
                    if (theName.compare("Asg") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        CCollectionElement* it = new CCollectionElement(0, 0, false);
                        it->serialize(ar);
                        _addCollectionElement(it);
                    }
                    if (theName == "Par")
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        unsigned char nothing;
                        ar >> nothing;
                        _overridesObjectMainProperties = SIM_IS_BIT_SET(nothing, 0);
                    }
                    if (theName.compare("Uis") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _uniquePersistentIdString;
                    }
                    if (noHit)
                        ar.loadUnknownData();
                }
            }
        }
    }
    else
    {
        bool exhaustiveXml = ((ar.getFileType() != CSer::filetype_csim_xml_simplescene_file) &&
                              (ar.getFileType() != CSer::filetype_csim_xml_simplemodel_file));
        if (ar.isStoring())
        {
            if (exhaustiveXml)
                ar.xmlAddNode_int("handle", _collectionHandle);

            if (exhaustiveXml)
                ar.xmlAddNode_string("name", _collectionName.c_str());
            else
                ar.xmlAddNode_string("name", ("@collection@" + _collectionName).c_str());

            ar.xmlPushNewNode("switches");
            ar.xmlAddNode_bool("overrideProperties", _overridesObjectMainProperties);
            ar.xmlPopNode();

            if (exhaustiveXml)
            {
                std::string str(base64_encode((unsigned char*)_uniquePersistentIdString.c_str(),
                                              _uniquePersistentIdString.size()));
                ar.xmlAddNode_string("uniquePersistentIdString_base64Coded", str.c_str());
            }

            for (size_t i = 0; i < getElementCount(); i++)
            {
                ar.xmlAddNode_comment(" 'item' tag: at least one of such tag is required ", exhaustiveXml);
                ar.xmlPushNewNode("item");
                getElementFromIndex(i)->serialize(ar);
                ar.xmlPopNode();
            }
        }
        else
        {
            if (exhaustiveXml)
                ar.xmlGetNode_int("handle", _collectionHandle);

            if (ar.xmlGetNode_string("name", _collectionName, exhaustiveXml) && (!exhaustiveXml))
            {
                _collectionLoadName = _collectionName;
                if (_collectionName.find("@collection@") == 0)
                    _collectionName.assign(_collectionLoadName.begin() + strlen("@collection@"),
                                           _collectionLoadName.end());
                tt::removeIllegalCharacters(_collectionName, true);
            }

            if (ar.xmlPushChildNode("switches", exhaustiveXml))
            {
                ar.xmlGetNode_bool("overrideProperties", _overridesObjectMainProperties, exhaustiveXml);
                ar.xmlPopNode();
            }

            if (exhaustiveXml &&
                ar.xmlGetNode_string("uniquePersistentIdString_base64Coded", _uniquePersistentIdString))
                _uniquePersistentIdString = base64_decode(_uniquePersistentIdString);

            if (ar.xmlPushChildNode("item", exhaustiveXml))
            {
                while (true)
                {
                    CCollectionElement* it = new CCollectionElement(0, 0, false);
                    it->serialize(ar);
                    _addCollectionElement(it);
                    if (!ar.xmlPushSiblingNode("item", false))
                        break;
                }
                ar.xmlPopNode();
            }
        }
    }
}

void CCollection::_updateCollectionObjects_(const std::vector<int>& sceneObjectHandles)
{
    std::vector<int> cpy(_collectionObjects);
    _collectionObjects.assign(sceneObjectHandles.begin(), sceneObjectHandles.end());
    if (cpy != _collectionObjects)
    {
        if (App::worldContainer->getEventsEnabled())
        {
#if SIM_EVENT_PROTOCOL_VERSION >= 3
            {
                CCbor* ev = App::worldContainer->createEvent(EVENTTYPE_OBJECTCHANGED, _collectionHandle, _collectionHandle, nullptr, false);
                ev->appendKeyIntArray(propCollection_objects.name, _collectionObjects.data(), _collectionObjects.size());
                App::worldContainer->pushEvent();
            }
#endif
#if SIM_EVENT_PROTOCOL_VERSION <= 3
            // For backw. compatibility
            {
                CCbor* ev = App::worldContainer->createEvent("collectionChanged", -1, _collectionHandle, nullptr, false);
                ev->appendKeyIntArray(propCollection_objects.name, _collectionObjects.data(), _collectionObjects.size());
                App::worldContainer->pushEvent();
            }
#endif
        }

    }
}

std::string CCollection::getCollectionLoadName() const
{
    return (_collectionLoadName);
}

bool CCollection::getOverridesObjectMainProperties() const
{
    return (_overridesObjectMainProperties);
}

size_t CCollection::getElementCount() const
{
    return (_collectionElements.size());
}

CCollectionElement* CCollection::getElementFromIndex(size_t index) const
{
    CCollectionElement* retVal = nullptr;
    if (index < _collectionElements.size())
        retVal = _collectionElements[index];
    return (retVal);
}

CCollectionElement* CCollection::getElementFromHandle(int collectionElementHandle) const
{
    for (size_t i = 0; i < _collectionElements.size(); i++)
    {
        if (_collectionElements[i]->getElementHandle() == collectionElementHandle)
            return (_collectionElements[i]);
    }
    return (nullptr);
}

int CCollection::getCollectionHandle() const
{
    return (_collectionHandle);
}

std::string CCollection::getCollectionName() const
{
    return (_collectionName);
}

bool CCollection::setOverridesObjectMainProperties(bool o)
{
    bool diff = (_overridesObjectMainProperties != o);
    if (diff)
        _overridesObjectMainProperties = o;
    return (diff);
}

bool CCollection::setCollectionHandle(int newHandle)
{
    bool diff = (_collectionHandle != newHandle);
    _collectionHandle = newHandle;
    return (diff);
}

void CCollection::_addCollectionElement(CCollectionElement* collectionElement)
{
    _collectionElements.push_back(collectionElement);
}

void CCollection::_removeCollectionElementFromHandle(int collectionElementHandle)
{
    for (size_t i = 0; i < _collectionElements.size(); i++)
    {
        if (_collectionElements[i]->getElementHandle() == collectionElementHandle)
        {
            delete _collectionElements[i];
            _collectionElements.erase(_collectionElements.begin() + i);
            break;
        }
    }
}

void CCollection::pushCreationEvent() const
{
    if (App::worldContainer->getEventsEnabled())
    {
#if SIM_EVENT_PROTOCOL_VERSION >= 3
        {
            CCbor* ev = App::worldContainer->createEvent(EVENTTYPE_OBJECTADDED, _collectionHandle, _collectionHandle, nullptr, false);
            ev->appendKeyText(propCollection_objectType.name, OBJECT_TYPE.c_str());
            ev->appendKeyIntArray(propCollection_objects.name, _collectionObjects.data(), _collectionObjects.size());
            ev->appendKeyInt(propCollection_handle.name, _collectionHandle);
            App::worldContainer->pushEvent();
        }
#endif
#if SIM_EVENT_PROTOCOL_VERSION <= 3
        // For backw. compatibility
        {
            CCbor* ev = App::worldContainer->createEvent("collectionAdded", -1, _collectionHandle, nullptr, false);
            ev->appendKeyText(propCollection_objectType.name, OBJECT_TYPE.c_str());
            ev->appendKeyIntArray(propCollection_objects.name, _collectionObjects.data(), _collectionObjects.size());
            App::worldContainer->pushEvent();
        }
#endif
    }
}
