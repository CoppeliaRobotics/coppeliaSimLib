#include <customObjectContainer.h>
#include <app.h>

CustomObjectContainer::CustomObjectContainer(int storageLocation)
{
    _storageLocation = storageLocation;
}

CustomObjectContainer::~CustomObjectContainer()
{
    clear();
}

void CustomObjectContainer::pushGenesisEvents() const
{
    for (auto it = _customObjects.begin(); it != _customObjects.end(); )
        it->second->pushObjectCreationEvent();
}

long long int CustomObjectContainer::getFreshHandle() const
{
    long long int retVal;
    if (_storageLocation == sim_handle_app)
        retVal = sim_object_customappstart;
    if (_storageLocation == sim_handle_scene)
        retVal = sim_object_customscenestart;
    while ((getObject(retVal) != nullptr) || (getClass(retVal) != nullptr))
        retVal++;
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

long long int CustomObjectContainer::addClass(const char* objectTypeStr, const char* objectMetaInfo, int originScriptHandle)
{
    long long int retVal = -1;
    if (getClass(objectTypeStr) == nullptr)
    {
        retVal = getFreshHandle();
        CustomObject* obj = new CustomObject(retVal, objectTypeStr, objectMetaInfo, originScriptHandle, _storageLocation);
        _customClasses.insert({objectTypeStr, obj});
        obj->setIntProperty("storageLocation", _storageLocation);
        obj->setPropertyInfo("storageLocation", sim_propertyinfo_notwritable | sim_propertyinfo_constant | sim_propertyinfo_modelhashexclude, "");
    }
    return retVal;
}

bool CustomObjectContainer::removeClass(const char* objectTypeStr)
{
    bool retVal = false;
    auto it = _customClasses.find(objectTypeStr);
    if (it != _customClasses.end())
    {
        delete it->second;
        _customClasses.erase(it);
        retVal = true;
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
            break;
        }
    }
    return retVal;
}

CustomObject* CustomObjectContainer::getClass(const char* objectTypeStr) const
{
    CustomObject* retVal = nullptr;
    auto it = _customClasses.find(objectTypeStr);
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

long long int CustomObjectContainer::addObject(const char* objectTypeStr, bool isVolatile, int originScriptHandle)
{
    long long int retVal = -1;
    CustomObject* classObj = getClass(objectTypeStr);
    if (classObj != nullptr)
    {
        retVal = getFreshHandle();
        CustomObject* obj = classObj->createObject(retVal, originScriptHandle);
        obj->setVolatile(isVolatile);
        _customObjects.insert({retVal, obj});
        obj->pushObjectCreationEvent();
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
    }
    return retVal;
}

void CustomObjectContainer::announceScriptStateWillBeErased(int scriptHandle)
{
    for (auto it = _customObjects.begin(); it != _customObjects.end(); )
    {
        CustomObject* obj = it->second;
        if (obj->getScriptHandle() == scriptHandle)
        {
            delete obj;
            it = _customObjects.erase(it);  // erase returns next valid iterator
        }
        else
            ++it;
    }
    for (auto it = _customClasses.begin(); it != _customClasses.end(); )
    {
        CustomObject* obj = it->second;
        if (obj->getScriptHandle() == scriptHandle)
        {
            delete obj;
            it = _customClasses.erase(it);  // erase returns next valid iterator
        }
        else
            ++it;
    }
}

void CustomObjectContainer::clear()
{
    for (auto it = _customClasses.begin(); it != _customClasses.end(); )
        delete it->second;
    _customClasses.clear();
    for (auto it = _customObjects.begin(); it != _customObjects.end(); )
        delete it->second;
    _customObjects.clear();
}

void CustomObjectContainer::_storeClasses() const
{
    /*
    CSer serObj("", filetype_csim_bin_generic_file);

    filetype_unspecified_file
    serObj = new CSer(_pathAndFilename.c_str(), CSer::getFileTypeFromName(_pathAndFilename.c_str()));
    retVal = serObj->writeOpenBinary(App::userSettings->compressFiles);


    for (size_t i = 0; i < _customClasses.size(); i++)
    {
        CustomObject* cl = _customClasses[i];
        if (cl->getObjectHandle() == -1)
        { // Only classes that have finished their definition process

        }
    }

    App::scene->saveScene(serObj[0], !autoSaveMechanism);
    serObj->writeClose();
}
delete serObj;
    */
}

int CustomObjectContainer::setBoolProperty(long long int target, const char* ppName, bool pState)
{
    int retVal = sim_propertyret_unknowntarget;
    CustomObject* obj = getItem(target);
    if (obj != nullptr)
    {
        if (strcmp(ppName, "_configDone_") == 0)
        {
            CustomObject* classObj = getClass(target);
            retVal = sim_propertyret_ok;
            classObj->setLongProperty("handle", -1);
            _storeClasses();
        }
        else
            retVal = obj->setBoolProperty(ppName, pState);
    }
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

int CustomObjectContainer::setStringProperty(long long int target, const char* ppName, const char* pState)
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

int CustomObjectContainer::setBufferProperty(long long int target, const char* ppName, const char* buffer, int bufferL)
{
    int retVal = sim_propertyret_unknowntarget;
    CustomObject* obj = getItem(target);
    if (obj != nullptr)
        retVal = obj->setBufferProperty(ppName, buffer, bufferL);
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

int CustomObjectContainer::setVector2Property(long long int target, const char* ppName, const double* pState)
{
    int retVal = sim_propertyret_unknowntarget;
    CustomObject* obj = getItem(target);
    if (obj != nullptr)
        retVal = obj->setVector2Property(ppName, pState);
    return retVal;
}

int CustomObjectContainer::getVector2Property(long long int target, const char* ppName, double* pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CustomObject* obj = getItem(target);
    if (obj != nullptr)
        retVal = obj->getVector2Property(ppName, pState);
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

int CustomObjectContainer::setFloatArrayProperty(long long int target, const char* ppName, const double* v, int vL)
{
    int retVal = sim_propertyret_unknowntarget;
    CustomObject* obj = getItem(target);
    if (obj != nullptr)
        retVal = obj->setFloatArrayProperty(ppName, v, vL);
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

int CustomObjectContainer::setIntArrayProperty(long long int target, const char* ppName, const int* v, int vL)
{
    int retVal = sim_propertyret_unknowntarget;
    CustomObject* obj = getItem(target);
    if (obj != nullptr)
        retVal = obj->setIntArrayProperty(ppName, v, vL);
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

int CustomObjectContainer::setHandleArrayProperty(long long int target, const char* ppName, const long long int* v, int vL)
{
    int retVal = sim_propertyret_unknowntarget;
    CustomObject* obj = getItem(target);
    if (obj != nullptr)
        retVal = obj->setHandleArrayProperty(ppName, v, vL);
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
