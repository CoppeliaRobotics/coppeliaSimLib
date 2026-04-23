#include <customObjectContainer.h>
#include <app.h>

CustomObjectContainer::CustomObjectContainer(int target)
{
    _target = target;
}

CustomObjectContainer::~CustomObjectContainer()
{
    clear();
}

void CustomObjectContainer::init()
{
    if (_target == sim_handle_app)
        _loadFromAppFolder();
}

void CustomObjectContainer::pushGenesisEvents() const
{
    for (auto it = _customObjects.begin(); it != _customObjects.end(); ++it)
        it->second->pushObjectCreationEvent();
}

void CustomObjectContainer::serialize(CSer& ar)
{
    if (ar.isBinary())
    {
        if (ar.isStoring())
        { // Storing
            for (auto it = _customObjects.begin(); it != _customObjects.end(); ++it)
            {
                CustomObject* obj = it->second;
                if (!obj->getVolatile())
                {
                    ar.storeDataName("Obj");
                    ar.setCountingMode();
                    it->second->serialize(ar);
                    if (ar.setWritingMode())
                        it->second->serialize(ar);
                }
            }
            ar.storeDataName(SER_END_OF_OBJECT);
        }
        else
        { // Loading
            int byteQuantity;
            std::string theName = "";
            _customObjects.clear();
            while (theName.compare(SER_END_OF_OBJECT) != 0)
            {
                theName = ar.readDataName();
                if (theName.compare(SER_END_OF_OBJECT) != 0)
                {
                    bool noHit = true;
                    if (theName.compare("Obj") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        CustomObject* obj = new CustomObject(-1, "", "", -1, _target);
                        obj->serialize(ar);
                        obj->setVolatile(false);
                        _customObjects.insert({obj->getObjectHandle(), obj});
                    }
                    if (noHit)
                        ar.loadUnknownData();
                }
            }
        }
    }
    else
    {
        if (ar.isStoring())
        {
            for (auto it = _customObjects.begin(); it != _customObjects.end(); ++it)
            {
                CustomObject* obj = it->second;
                if (!obj->getVolatile())
                {
                    ar.xmlPushNewNode("customObject");
                    obj->serialize(ar);
                    ar.xmlPopNode();
                }
            }
        }
        else
        {
            _customObjects.clear();
            if (ar.xmlPushChildNode("customObject", false))
            {
                while (true)
                {
                    CustomObject* obj = new CustomObject(-1, "", "", -1, _target);
                    obj->serialize(ar);
                    obj->setVolatile(false);
                    _customObjects.insert({obj->getObjectHandle(), obj});
                    if (!ar.xmlPushSiblingNode("customObject", false))
                        break;
                }
                ar.xmlPopNode();
            }
        }
    }
    if (!ar.isStoring())
    { // associate all objects with their classes:
        for (auto it = _customObjects.begin(); it != _customObjects.end(); ++it)
        {
            CustomObject* obj = it->second;
            CustomObject* classObj = App::scenes->customObjects->getClass(obj->getObjectTypeStr().c_str());
            int classHandle = -1;
            if (classObj != nullptr)
                classHandle = classObj->getObjectHandle();
            else
            { // too early, doesn't print yet
                std::string err = std::string("custom object ") + std::to_string(it->first) + " could not find its associated class.";
                App::logScriptMsg(nullptr, sim_verbosity_scriptwarnings, err.c_str());
            }
            obj->setIgnoreSetterGetter(true);
            obj->setObjectCanAddRemoveProperty(true);
            obj->setPropertyInfo("class", 0, ""); // make it writable temporarily
            obj->setHandleProperty("class", classHandle);
            obj->setPropertyInfo("class", sim_propertyinfo_notwritable | sim_propertyinfo_constant | sim_propertyinfo_modelhashexclude, R"("{"handleType":"class"})");
            obj->setIgnoreSetterGetter(false);
            obj->setObjectCanAddRemoveProperty(false);
        }
    }
}

void CustomObjectContainer::saveToAppFolderIfNeeded()
{
    bool doIt = false;
    for (auto it = _customObjects.begin(); it != _customObjects.end(); ++it)
    {
        CustomObject* obj = it->second;
        if (obj->getResetChanged() && (!obj->getVolatile()))
            doIt = true;
    }
    if (doIt)
    {
        CSer ar((App::folders->getUserSettingsPath() + "/customObjects.ttg").c_str(), CSer::filetype_csim_bin_generic_file);
        ar.writeOpenBinary(false);
        serialize(ar);
        ar.writeClose();
    }
}

void CustomObjectContainer::_loadFromAppFolder()
{
    std::string file(App::folders->getUserSettingsPath() + "/customObjects.ttg");
    if (VFile::doesFileExist(file.c_str()))
    {
        try
        {
            CSer ar(file.c_str(), CSer::filetype_csim_bin_generic_file);
            ar.readOpenBinary(CSer::filetype_csim_bin_generic_file, false);
            serialize(ar);
            ar.readClose();
        }
        catch (VFILE_EXCEPTION_TYPE e)
        {
            VFile::reportAndHandleFileExceptionError(e);
        }
    }
}

long long int CustomObjectContainer::getFreshHandle(bool forObject) const
{
    long long int retVal = sim_object_customappstart;
    if (forObject)
    {
        if (_target == sim_handle_scene)
            retVal = sim_object_customscenestart;
        while (getObject(retVal) != nullptr)
            retVal++;
    }
    else
    {
        retVal = sim_object_customclassstart;
        while (getClass(retVal) != nullptr)
            retVal++;
    }
    return retVal;
}

CustomObject* CustomObjectContainer::getItem(long long int objectHandle) const
{
    CustomObject* retVal = getObject(objectHandle);
    if (retVal == nullptr)
        retVal = getClass(objectHandle);
    return retVal;
}

bool CustomObjectContainer::removeItem(long long int objectHandle)
{
    bool retVal = removeClass(objectHandle);
    if (!retVal)
        retVal = removeObject(objectHandle);
    return retVal;
}

long long int CustomObjectContainer::addClass(const char* className, const char* objectMetaInfo, int originScriptHandle)
{
    long long int retVal = -1;
    if (getClass(className) == nullptr)
    {
        retVal = getFreshHandle(false);
        CustomObject* obj = new CustomObject(retVal, "class", objectMetaInfo, originScriptHandle, _target);
        _customClasses.insert({className, obj});
        obj->setIntProperty("target", _target);
        obj->setPropertyInfo("target", sim_propertyinfo_notwritable | sim_propertyinfo_constant | sim_propertyinfo_modelhashexclude, "");
        obj->setStringProperty("name", className);
        obj->setPropertyInfo("name", sim_propertyinfo_notwritable | sim_propertyinfo_constant | sim_propertyinfo_modelhashexclude, "");
        obj->setBoolProperty("customClass", true);
        obj->setPropertyInfo("customClass", sim_propertyinfo_notwritable | sim_propertyinfo_constant | sim_propertyinfo_modelhashexclude, "");
        //obj->setMethodProperty("remove", nullptr);
        //obj->setPropertyInfo("remove", sim_propertyinfo_notreadable | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, "");
        _notifyClassListChanged();
    }
    return retVal;
}

bool CustomObjectContainer::removeClass(const char* className)
{
    bool retVal = false;
    auto it = _customClasses.find(className);
    if (it != _customClasses.end())
    {
        delete it->second;
        _customClasses.erase(it);
        retVal = true;
        _notifyClassListChanged();
    }
    return retVal;
}

bool CustomObjectContainer::removeClass(long long int objectHandle)
{
    bool retVal = false;
    for (auto it = _customClasses.begin(); it != _customClasses.end(); ++it)
    {
        if (it->second->getObjectHandle() == objectHandle)
        {
            delete it->second;
            _customClasses.erase(it);
            retVal = true;
            _notifyClassListChanged();
            break;
        }
    }
    return retVal;
}

CustomObject* CustomObjectContainer::getClass(const char* className) const
{
    CustomObject* retVal = nullptr;
    auto it = _customClasses.find(className);
    if (it != _customClasses.end())
        retVal = it->second;
    return retVal;
}

CustomObject* CustomObjectContainer::getClass(long long int objectHandle) const
{
    CustomObject* retVal = nullptr;
    for (auto it = _customClasses.begin(); it != _customClasses.end(); ++it)
    {
        if (it->second->getObjectHandle() == objectHandle)
        {
            retVal = it->second;
            break;
        }
    }
    return retVal;
}

long long int CustomObjectContainer::addObject(const char* className, bool isVolatile, int originScriptHandle)
{
    long long int retVal = -1;
    CustomObject* classObj = App::scenes->customObjects->getClass(className);
    if (classObj != nullptr)
    {
        retVal = getFreshHandle(true);
        CustomObject* obj = classObj->createObject(retVal, originScriptHandle);
        obj->setVolatile(isVolatile);
        _customObjects.insert({retVal, obj});
        obj->setIgnoreSetterGetter(true);
        obj->setObjectCanAddRemoveProperty(true);
        obj->setPropertyInfo("name", sim_propertyinfo_removable, ""); // first make it removable
        obj->removeProperty("name");
        obj->setPropertyInfo("customClass", sim_propertyinfo_removable, ""); // first make it removable
        obj->removeProperty("customClass");
        obj->setHandleProperty("class", classObj->getObjectHandle());
        obj->setPropertyInfo("class", sim_propertyinfo_notwritable | sim_propertyinfo_constant | sim_propertyinfo_modelhashexclude, R"("{"handleType":"class"})");
        obj->setIgnoreSetterGetter(false);
        obj->setObjectCanAddRemoveProperty(false);
        obj->pushObjectCreationEvent();
        _notifyObjectListChanged();
    }
    return retVal;
}

CustomObject* CustomObjectContainer::getObject(long long int objectHandle) const
{
    CustomObject* retVal = nullptr;
    auto it = _customObjects.find(objectHandle);
    if (it != _customObjects.end())
        retVal = it->second;
    return retVal;
}

bool CustomObjectContainer::removeObject(long long int objectHandle)
{
    bool retVal = false;
    auto it = _customObjects.find(objectHandle);
    if (it != _customObjects.end())
    {
        delete it->second;
        _customObjects.erase(it);
        retVal = true;
        _notifyObjectListChanged();
    }
    return retVal;
}

void CustomObjectContainer::getAllObjectHandles(std::vector<long long int>& objects) const
{
    objects.clear();
    for (auto it = _customObjects.begin(); it != _customObjects.end(); ++it)
        objects.push_back(it->first);
}

void CustomObjectContainer::getAllClassHandles(std::vector<long long int>& classes) const
{
    classes.clear();
    for (auto it = _customClasses.begin(); it != _customClasses.end(); ++it)
        classes.push_back(it->second->getObjectHandle());
}

void CustomObjectContainer::_notifyObjectListChanged() const
{
    if ((App::scenes != nullptr) && App::scenes->getEventsEnabled())
    {
        std::vector<long long int> customObjectList;
        getAllObjectHandles(customObjectList);
        CCbor* ev = App::scenes->createObjectChangedEvent(_target, nullptr, true);
        if (_target == sim_handle_app)
            ev->appendKeyHandleArray(propApp_customObjects.name, customObjectList.data(), customObjectList.size());
        if (_target == sim_handle_scene)
            ev->appendKeyHandleArray(propScene_customObjects.name, customObjectList.data(), customObjectList.size());
        App::scenes->pushEvent();
    }
}

void CustomObjectContainer::_notifyClassListChanged() const
{
    if ((App::scenes != nullptr) && App::scenes->getEventsEnabled() && (_target == sim_handle_app))
    {
        std::vector<long long int> customClassList;
        getAllClassHandles(customClassList);
        CCbor* ev = App::scenes->createObjectChangedEvent(_target, nullptr, true);
        ev->appendKeyHandleArray(propApp_customClasses.name, customClassList.data(), customClassList.size());
        App::scenes->pushEvent();
    }
}

void CustomObjectContainer::announceScriptStateWillBeErased(int scriptHandle)
{
    bool notify = false;
    for (auto it = _customObjects.begin(); it != _customObjects.end(); )
    {
        CustomObject* obj = it->second;
        if (obj->getScriptHandle() == scriptHandle)
        {
            notify = true;
            delete obj;
            it = _customObjects.erase(it);  // erase returns next valid iterator
        }
        else
            ++it;
    }
    if (notify)
        _notifyObjectListChanged();
    notify = false;
    for (auto it = _customClasses.begin(); it != _customClasses.end(); )
    {
        CustomObject* obj = it->second;
        if (obj->getScriptHandle() == scriptHandle)
        {
            notify = true;
            delete obj;
            it = _customClasses.erase(it);  // erase returns next valid iterator
        }
        else
            ++it;
    }
    if (notify)
        _notifyClassListChanged();
}

void CustomObjectContainer::clear()
{
    bool notify = (_customClasses.size() > 0);
    for (auto it = _customClasses.begin(); it != _customClasses.end(); ++it )
        delete it->second;
    _customClasses.clear();
    if (notify)
        _notifyClassListChanged();
    notify = (_customObjects.size() > 0);
    for (auto it = _customObjects.begin(); it != _customObjects.end(); ++it )
        delete it->second;
    _customObjects.clear();
    if (notify)
        _notifyObjectListChanged();
}

int CustomObjectContainer::setBoolProperty(long long int target, const char* ppName, bool pState)
{
    int retVal = sim_propertyret_unknowntarget;
    CustomObject* obj = getItem(target);
    if (obj != nullptr)
        retVal = obj->setBoolProperty(ppName, pState);
    return retVal;
}

int CustomObjectContainer::getBoolProperty(long long int target, const char* ppName, bool& pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CustomObject* obj = getItem(target);
    if (obj != nullptr)
        retVal = obj->getBoolProperty(ppName, pState);
    return retVal;
}

int CustomObjectContainer::setIntProperty(long long int target, const char* ppName, int pState)
{
    int retVal = sim_propertyret_unknowntarget;
    CustomObject* obj = getItem(target);
    if (obj != nullptr)
        retVal = obj->setIntProperty(ppName, pState);
    return retVal;
}

int CustomObjectContainer::getIntProperty(long long int target, const char* ppName, int& pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CustomObject* obj = getItem(target);
    if (obj != nullptr)
        retVal = obj->getIntProperty(ppName, pState);
    return retVal;
}

int CustomObjectContainer::setLongProperty(long long int target, const char* ppName, long long int pState)
{
    int retVal = sim_propertyret_unknowntarget;
    CustomObject* obj = getItem(target);
    if (obj != nullptr)
        retVal = obj->setLongProperty(ppName, pState);
    return retVal;
}

int CustomObjectContainer::getLongProperty(long long int target, const char* ppName, long long int& pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CustomObject* obj = getItem(target);
    if (obj != nullptr)
        retVal = obj->getLongProperty(ppName, pState);
    return retVal;
}

int CustomObjectContainer::setHandleProperty(long long int target, const char* ppName, long long int pState)
{
    int retVal = sim_propertyret_unknowntarget;
    CustomObject* obj = getItem(target);
    if (obj != nullptr)
        retVal = obj->setHandleProperty(ppName, pState);
    return retVal;
}

int CustomObjectContainer::getHandleProperty(long long int target, const char* ppName, long long int& pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CustomObject* obj = getItem(target);
    if (obj != nullptr)
        retVal = obj->getHandleProperty(ppName, pState);
    return retVal;
}

int CustomObjectContainer::setFloatProperty(long long int target, const char* ppName, double pState)
{
    int retVal = sim_propertyret_unknowntarget;
    CustomObject* obj = getItem(target);
    if (obj != nullptr)
        retVal = obj->setFloatProperty(ppName, pState);
    return retVal;
}

int CustomObjectContainer::getFloatProperty(long long int target, const char* ppName, double& pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CustomObject* obj = getItem(target);
    if (obj != nullptr)
        retVal = obj->getFloatProperty(ppName, pState);
    return retVal;
}

int CustomObjectContainer::setStringProperty(long long int target, const char* ppName, const std::string& pState)
{
    int retVal = sim_propertyret_unknowntarget;
    CustomObject* obj = getItem(target);
    if (obj != nullptr)
        retVal = obj->setStringProperty(ppName, pState);
    return retVal;
}

int CustomObjectContainer::getStringProperty(long long int target, const char* ppName, std::string& pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CustomObject* obj = getItem(target);
    if (obj != nullptr)
        retVal = obj->getStringProperty(ppName, pState);
    return retVal;
}

int CustomObjectContainer::setBufferProperty(long long int target, const char* ppName, const std::string& pState)
{
    int retVal = sim_propertyret_unknowntarget;
    CustomObject* obj = getItem(target);
    if (obj != nullptr)
        retVal = obj->setBufferProperty(ppName, pState);
    return retVal;
}

int CustomObjectContainer::getBufferProperty(long long int target, const char* ppName, std::string& pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CustomObject* obj = getItem(target);
    if (obj != nullptr)
        retVal = obj->getBufferProperty(ppName, pState);
    return retVal;
}

int CustomObjectContainer::setIntArray2Property(long long int target, const char* ppName, const int* pState)
{
    int retVal = sim_propertyret_unknowntarget;
    CustomObject* obj = getItem(target);
    if (obj != nullptr)
        retVal = obj->setIntArray2Property(ppName, pState);
    return retVal;
}

int CustomObjectContainer::getIntArray2Property(long long int target, const char* ppName, int* pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CustomObject* obj = getItem(target);
    if (obj != nullptr)
        retVal = obj->getIntArray2Property(ppName, pState);
    return retVal;
}

int CustomObjectContainer::setVector3Property(long long int target, const char* ppName, const C3Vector& pState)
{
    int retVal = sim_propertyret_unknowntarget;
    CustomObject* obj = getItem(target);
    if (obj != nullptr)
        retVal = obj->setVector3Property(ppName, pState);
    return retVal;
}

int CustomObjectContainer::getVector3Property(long long int target, const char* ppName, C3Vector& pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CustomObject* obj = getItem(target);
    if (obj != nullptr)
        retVal = obj->getVector3Property(ppName, pState);
    return retVal;
}

int CustomObjectContainer::setMatrixProperty(long long int target, const char* ppName, const CMatrix& pState)
{
    int retVal = sim_propertyret_unknowntarget;
    CustomObject* obj = getItem(target);
    if (obj != nullptr)
        retVal = obj->setMatrixProperty(ppName, pState);
    return retVal;
}

int CustomObjectContainer::getMatrixProperty(long long int target, const char* ppName, CMatrix& pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CustomObject* obj = getItem(target);
    if (obj != nullptr)
        retVal = obj->getMatrixProperty(ppName, pState);
    return retVal;
}

int CustomObjectContainer::setQuaternionProperty(long long int target, const char* ppName, const C4Vector& pState)
{
    int retVal = sim_propertyret_unknowntarget;
    CustomObject* obj = getItem(target);
    if (obj != nullptr)
        retVal = obj->setQuaternionProperty(ppName, pState);
    return retVal;
}

int CustomObjectContainer::getQuaternionProperty(long long int target, const char* ppName, C4Vector& pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CustomObject* obj = getItem(target);
    if (obj != nullptr)
        retVal = obj->getQuaternionProperty(ppName, pState);
    return retVal;
}

int CustomObjectContainer::setPoseProperty(long long int target, const char* ppName, const C7Vector& pState)
{
    int retVal = sim_propertyret_unknowntarget;
    CustomObject* obj = getItem(target);
    if (obj != nullptr)
        retVal = obj->setPoseProperty(ppName, pState);
    return retVal;
}

int CustomObjectContainer::getPoseProperty(long long int target, const char* ppName, C7Vector& pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CustomObject* obj = getItem(target);
    if (obj != nullptr)
        retVal = obj->getPoseProperty(ppName, pState);
    return retVal;
}

int CustomObjectContainer::setColorProperty(long long int target, const char* ppName, const float* pState)
{
    int retVal = sim_propertyret_unknowntarget;
    CustomObject* obj = getItem(target);
    if (obj != nullptr)
        retVal = obj->setColorProperty(ppName, pState);
    return retVal;
}

int CustomObjectContainer::getColorProperty(long long int target, const char* ppName, float* pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CustomObject* obj = getItem(target);
    if (obj != nullptr)
        retVal = obj->getColorProperty(ppName, pState);
    return retVal;
}

int CustomObjectContainer::setFloatArrayProperty(long long int target, const char* ppName, const std::vector<double>& pState)
{
    int retVal = sim_propertyret_unknowntarget;
    CustomObject* obj = getItem(target);
    if (obj != nullptr)
        retVal = obj->setFloatArrayProperty(ppName, pState);
    return retVal;
}

int CustomObjectContainer::getFloatArrayProperty(long long int target, const char* ppName, std::vector<double>& pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CustomObject* obj = getItem(target);
    if (obj != nullptr)
        retVal = obj->getFloatArrayProperty(ppName, pState);
    return retVal;
}

int CustomObjectContainer::setIntArrayProperty(long long int target, const char* ppName, const std::vector<int>& pState)
{
    int retVal = sim_propertyret_unknowntarget;
    CustomObject* obj = getItem(target);
    if (obj != nullptr)
        retVal = obj->setIntArrayProperty(ppName, pState);
    return retVal;
}

int CustomObjectContainer::getIntArrayProperty(long long int target, const char* ppName, std::vector<int>& pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CustomObject* obj = getItem(target);
    if (obj != nullptr)
        retVal = obj->getIntArrayProperty(ppName, pState);
    return retVal;
}

int CustomObjectContainer::setHandleArrayProperty(long long int target, const char* ppName, const std::vector<long long int>& pState)
{
    int retVal = sim_propertyret_unknowntarget;
    CustomObject* obj = getItem(target);
    if (obj != nullptr)
        retVal = obj->setHandleArrayProperty(ppName, pState);
    return retVal;
}

int CustomObjectContainer::getHandleArrayProperty(long long int target, const char* ppName, std::vector<long long int>& pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CustomObject* obj = getItem(target);
    if (obj != nullptr)
        retVal = obj->getHandleArrayProperty(ppName, pState);
    return retVal;
}

int CustomObjectContainer::setStringArrayProperty(long long int target, const char* ppName, const std::vector<std::string>& pState)
{
    int retVal = sim_propertyret_unknowntarget;
    CustomObject* obj = getItem(target);
    if (obj != nullptr)
        retVal = obj->setStringArrayProperty(ppName, pState);
    return retVal;
}

int CustomObjectContainer::getStringArrayProperty(long long int target, const char* ppName, std::vector<std::string>& pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CustomObject* obj = getItem(target);
    if (obj != nullptr)
        retVal = obj->getStringArrayProperty(ppName, pState);
    return retVal;
}

int CustomObjectContainer::setMethodProperty(long long int target, const char* ppName, const void* pState)
{
    int retVal = sim_propertyret_unknowntarget;
    CustomObject* obj = getItem(target);
    if (obj != nullptr)
        retVal = obj->setMethodProperty(ppName, pState);
    return retVal;
}

int CustomObjectContainer::getMethodProperty(long long int target, const char* ppName, void*& pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CustomObject* obj = getItem(target);
    if (obj != nullptr)
        retVal = obj->getMethodProperty(ppName, pState);
    return retVal;
}

int CustomObjectContainer::setMethodProperty(long long int target, const char* ppName, const std::string& pState)
{
    int retVal = sim_propertyret_unknowntarget;
    CustomObject* obj = getItem(target);
    if (obj != nullptr)
        retVal = obj->setMethodProperty(ppName, pState);
    return retVal;
}

int CustomObjectContainer::getMethodProperty(long long int target, const char* ppName, std::string& pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CustomObject* obj = getItem(target);
    if (obj != nullptr)
        retVal = obj->getMethodProperty(ppName, pState);
    return retVal;
}

int CustomObjectContainer::removeProperty(long long int target, const char* ppName)
{
    int retVal = sim_propertyret_unknowntarget;
    CustomObject* obj = getItem(target);
    if (obj != nullptr)
        retVal = obj->removeProperty(ppName);
    return retVal;
}

int CustomObjectContainer::getPropertyName(long long int target, int& index, std::string& pName, std::string& appartenance, int excludeFlags) const
{
    int retVal = sim_propertyret_unknowntarget;
    CustomObject* obj = getItem(target);
    if (obj != nullptr)
        retVal = obj->getPropertyName(index, pName, appartenance, excludeFlags);
    return retVal;
}

int CustomObjectContainer::getPropertyInfo(long long int target, const char* ppName, int& info, std::string& infoTxt) const
{
    int retVal = sim_propertyret_unknowntarget;
    CustomObject* obj = getItem(target);
    if (obj != nullptr)
        retVal = obj->getPropertyInfo(ppName, info, infoTxt);
    return retVal;
}

int CustomObjectContainer::setPropertyInfo(long long int target, const char* ppName, int info, const char* infoTxt)
{
    int retVal = sim_propertyret_unknowntarget;
    CustomObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->setPropertyInfo(ppName, info, infoTxt);
    else
    {
        obj = getObject(target);
        if (obj != nullptr)
            retVal = sim_propertyret_unavailable;
    }
    return retVal;
}
