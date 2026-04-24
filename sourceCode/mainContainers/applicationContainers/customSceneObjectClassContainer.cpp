#include <customSceneObjectClassContainer.h>
#include <app.h>

CustomSceneObjectClassContainer::CustomSceneObjectClassContainer()
{
}

CustomSceneObjectClassContainer::~CustomSceneObjectClassContainer()
{
    clear();
}

int CustomSceneObjectClassContainer::makeObject(int classHandle)
{
    CSceneObject* classObj = getClass(classHandle);
    CSceneObject* obj = classObj->copyYourself();
    int retVal = App::scene->sceneObjects->addObjectToScene(obj, false, true);
    obj->getCustomizationPart()->setIgnoreSetterGetter(true);
    obj->getCustomizationPart()->setObjectCanAddRemoveProperty(true);
    obj->setPropertyInfo("name", sim_propertyinfo_removable, ""); // first make it removable
    obj->removeProperty("name");
    obj->setPropertyInfo("customClass", sim_propertyinfo_removable, ""); // first make it removable
    obj->removeProperty("customClass");
    obj->setHandleProperty("class", classHandle);
    obj->setPropertyInfo("class", sim_propertyinfo_notwritable | sim_propertyinfo_constant | sim_propertyinfo_modelhashexclude, R"("{"handleType":"class"})");
    std::string objType;
    classObj->getStringProperty("name", objType);
    obj->setObjectTypeStr(objType.c_str());
    obj->getCustomizationPart()->setIgnoreSetterGetter(false);
    obj->getCustomizationPart()->setObjectCanAddRemoveProperty(false);
    return retVal;
}

int CustomSceneObjectClassContainer::makeClass(CSceneObject* obj, const char* className)
{
    CSceneObject* copy = obj->copyYourself();
    copy->setObjectTypeStr("class");
    copy->enableCustomizationPart();
    int h = sim_object_sceneobjectclassstart;
    while (getClass(h) != nullptr)
        h++;
    copy->setObjectHandle(h);
    _customClasses.insert({className, copy});
    copy->setStringProperty("name", className);
    copy->setPropertyInfo("name", sim_propertyinfo_notwritable | sim_propertyinfo_constant | sim_propertyinfo_modelhashexclude, "");
    copy->setBoolProperty("customClass", true);
    copy->setPropertyInfo("customClass", sim_propertyinfo_notwritable | sim_propertyinfo_constant | sim_propertyinfo_modelhashexclude, "");
    _notifyClassListChanged();
    return h;
}

bool CustomSceneObjectClassContainer::removeClass(const char* className)
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

bool CustomSceneObjectClassContainer::removeClass(int classHandle)
{
    bool retVal = false;
    for (auto it = _customClasses.begin(); it != _customClasses.end(); ++it)
    {
        if (it->second->getObjectHandle() == classHandle)
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

void CustomSceneObjectClassContainer::clear()
{
    for (auto it = _customClasses.begin(); it != _customClasses.end(); ++it)
        delete it->second;
    _customClasses.clear();
}

CSceneObject* CustomSceneObjectClassContainer::getClass(int classHandle) const
{
    CSceneObject* retVal = nullptr;
    for (auto it = _customClasses.begin(); it != _customClasses.end(); ++it)
    {
        if (it->second->getObjectHandle() == classHandle)
        {
            retVal = it->second;
            break;
        }
    }
    return retVal;
}

CSceneObject* CustomSceneObjectClassContainer::getClass(const char* className) const
{
    CSceneObject* retVal = nullptr;
    auto it = _customClasses.find(className);
    if (it != _customClasses.end())
        retVal = it->second;
    return retVal;
}

void CustomSceneObjectClassContainer::getAllClassHandles(std::vector<long long int>& classes) const
{
    classes.clear();
    for (auto it = _customClasses.begin(); it != _customClasses.end(); ++it)
        classes.push_back(it->second->getObjectHandle());
}

void CustomSceneObjectClassContainer::_notifyClassListChanged() const
{
    if ((App::scenes != nullptr) && App::scenes->getEventsEnabled())
    {
        std::vector<long long int> customClassList;
        getAllClassHandles(customClassList);
        CCbor* ev = App::scenes->createObjectChangedEvent(sim_handle_app, nullptr, true);
        ev->appendKeyHandleArray(propApp_customSceneObjectClasses.name, customClassList.data(), customClassList.size());
        App::scenes->pushEvent();
    }
}

int CustomSceneObjectClassContainer::setBoolProperty(long long int target, const char* ppName, bool pState)
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->getCustomizationPart()->setBoolProperty(ppName, pState, true);
    return retVal;
}

int CustomSceneObjectClassContainer::getBoolProperty(long long int target, const char* ppName, bool& pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->getCustomizationPart()->getBoolProperty(ppName, pState, true);
    return retVal;
}

int CustomSceneObjectClassContainer::setIntProperty(long long int target, const char* ppName, int pState)
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->getCustomizationPart()->setIntProperty(ppName, pState, true);
    return retVal;
}

int CustomSceneObjectClassContainer::getIntProperty(long long int target, const char* ppName, int& pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->getCustomizationPart()->getIntProperty(ppName, pState, true);
    return retVal;
}

int CustomSceneObjectClassContainer::setLongProperty(long long int target, const char* ppName, long long int pState)
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->getCustomizationPart()->setLongProperty(ppName, pState, true);
    return retVal;
}

int CustomSceneObjectClassContainer::getLongProperty(long long int target, const char* ppName, long long int& pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->getCustomizationPart()->getLongProperty(ppName, pState, true);
    return retVal;
}

int CustomSceneObjectClassContainer::setHandleProperty(long long int target, const char* ppName, long long int pState)
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->getCustomizationPart()->setHandleProperty(ppName, pState, true);
    return retVal;
}

int CustomSceneObjectClassContainer::getHandleProperty(long long int target, const char* ppName, long long int& pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->getCustomizationPart()->getHandleProperty(ppName, pState, true);
    return retVal;
}

int CustomSceneObjectClassContainer::setFloatProperty(long long int target, const char* ppName, double pState)
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->getCustomizationPart()->setFloatProperty(ppName, pState, true);
    return retVal;
}

int CustomSceneObjectClassContainer::getFloatProperty(long long int target, const char* ppName, double& pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->getCustomizationPart()->getFloatProperty(ppName, pState, true);
    return retVal;
}

int CustomSceneObjectClassContainer::setStringProperty(long long int target, const char* ppName, const std::string& pState)
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->getCustomizationPart()->setStringProperty(ppName, pState, true);
    return retVal;
}

int CustomSceneObjectClassContainer::getStringProperty(long long int target, const char* ppName, std::string& pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->getCustomizationPart()->getStringProperty(ppName, pState, true);
    return retVal;
}

int CustomSceneObjectClassContainer::setBufferProperty(long long int target, const char* ppName, const std::string& pState)
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->getCustomizationPart()->setBufferProperty(ppName, pState, true);
    return retVal;
}

int CustomSceneObjectClassContainer::getBufferProperty(long long int target, const char* ppName, std::string& pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->getCustomizationPart()->getBufferProperty(ppName, pState, true);
    return retVal;
}

int CustomSceneObjectClassContainer::setIntArray2Property(long long int target, const char* ppName, const int* pState)
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->getCustomizationPart()->setIntArray2Property(ppName, pState, true);
    return retVal;
}

int CustomSceneObjectClassContainer::getIntArray2Property(long long int target, const char* ppName, int* pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->getCustomizationPart()->getIntArray2Property(ppName, pState, true);
    return retVal;
}

int CustomSceneObjectClassContainer::setVector3Property(long long int target, const char* ppName, const C3Vector& pState)
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->getCustomizationPart()->setVector3Property(ppName, pState, true);
    return retVal;
}

int CustomSceneObjectClassContainer::getVector3Property(long long int target, const char* ppName, C3Vector& pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->getCustomizationPart()->getVector3Property(ppName, pState, true);
    return retVal;
}

int CustomSceneObjectClassContainer::setMatrixProperty(long long int target, const char* ppName, const CMatrix& pState)
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->getCustomizationPart()->setMatrixProperty(ppName, pState, true);
    return retVal;
}

int CustomSceneObjectClassContainer::getMatrixProperty(long long int target, const char* ppName, CMatrix& pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->getCustomizationPart()->getMatrixProperty(ppName, pState, true);
    return retVal;
}

int CustomSceneObjectClassContainer::setQuaternionProperty(long long int target, const char* ppName, const C4Vector& pState)
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->getCustomizationPart()->setQuaternionProperty(ppName, pState, true);
    return retVal;
}

int CustomSceneObjectClassContainer::getQuaternionProperty(long long int target, const char* ppName, C4Vector& pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->getCustomizationPart()->getQuaternionProperty(ppName, pState, true);
    return retVal;
}

int CustomSceneObjectClassContainer::setPoseProperty(long long int target, const char* ppName, const C7Vector& pState)
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->getCustomizationPart()->setPoseProperty(ppName, pState, true);
    return retVal;
}

int CustomSceneObjectClassContainer::getPoseProperty(long long int target, const char* ppName, C7Vector& pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->getCustomizationPart()->getPoseProperty(ppName, pState, true);
    return retVal;
}

int CustomSceneObjectClassContainer::setColorProperty(long long int target, const char* ppName, const float* pState)
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->getCustomizationPart()->setColorProperty(ppName, pState, true);
    return retVal;
}

int CustomSceneObjectClassContainer::getColorProperty(long long int target, const char* ppName, float* pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->getCustomizationPart()->getColorProperty(ppName, pState, true);
    return retVal;
}

int CustomSceneObjectClassContainer::setFloatArrayProperty(long long int target, const char* ppName, const std::vector<double>& pState)
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->getCustomizationPart()->setFloatArrayProperty(ppName, pState, true);
    return retVal;
}

int CustomSceneObjectClassContainer::getFloatArrayProperty(long long int target, const char* ppName, std::vector<double>& pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->getCustomizationPart()->getFloatArrayProperty(ppName, pState, true);
    return retVal;
}

int CustomSceneObjectClassContainer::setIntArrayProperty(long long int target, const char* ppName, const std::vector<int>& pState)
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->getCustomizationPart()->setIntArrayProperty(ppName, pState, true);
    return retVal;
}

int CustomSceneObjectClassContainer::getIntArrayProperty(long long int target, const char* ppName, std::vector<int>& pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->getCustomizationPart()->getIntArrayProperty(ppName, pState, true);
    return retVal;
}

int CustomSceneObjectClassContainer::setHandleArrayProperty(long long int target, const char* ppName, const std::vector<long long int>& pState)
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->getCustomizationPart()->setHandleArrayProperty(ppName, pState, true);
    return retVal;
}

int CustomSceneObjectClassContainer::getHandleArrayProperty(long long int target, const char* ppName, std::vector<long long int>& pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->getCustomizationPart()->getHandleArrayProperty(ppName, pState, true);
    return retVal;
}

int CustomSceneObjectClassContainer::setStringArrayProperty(long long int target, const char* ppName, const std::vector<std::string>& pState)
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->getCustomizationPart()->setStringArrayProperty(ppName, pState, true);
    return retVal;
}

int CustomSceneObjectClassContainer::getStringArrayProperty(long long int target, const char* ppName, std::vector<std::string>& pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->getCustomizationPart()->getStringArrayProperty(ppName, pState, true);
    return retVal;
}

int CustomSceneObjectClassContainer::setMethodProperty(long long int target, const char* ppName, const void* pState)
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->getCustomizationPart()->setMethodProperty(ppName, pState, true);
    return retVal;
}

int CustomSceneObjectClassContainer::getMethodProperty(long long int target, const char* ppName, void*& pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->getCustomizationPart()->getMethodProperty(ppName, pState, true);
    return retVal;
}

int CustomSceneObjectClassContainer::setMethodProperty(long long int target, const char* ppName, const std::string& pState)
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->getCustomizationPart()->setMethodProperty(ppName, pState, true);
    return retVal;
}

int CustomSceneObjectClassContainer::getMethodProperty(long long int target, const char* ppName, std::string& pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->getCustomizationPart()->getMethodProperty(ppName, pState, true);
    return retVal;
}

int CustomSceneObjectClassContainer::removeProperty(long long int target, const char* ppName)
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->getCustomizationPart()->removeProperty(ppName, true);
    return retVal;
}

int CustomSceneObjectClassContainer::getPropertyName(long long int target, int& index, std::string& pName, std::string& appartenance, int excludeFlags) const
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->getCustomizationPart()->getPropertyName(index, pName, appartenance, excludeFlags, true);
    return retVal;
}

int CustomSceneObjectClassContainer::getPropertyInfo(long long int target, const char* ppName, int& info, std::string& infoTxt) const
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->getCustomizationPart()->getPropertyInfo(ppName, info, infoTxt, true);
    return retVal;
}

int CustomSceneObjectClassContainer::setPropertyInfo(long long int target, const char* ppName, int info, const char* infoTxt)
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->getCustomizationPart()->setPropertyInfo(ppName, info, infoTxt, true);
    return retVal;
}
