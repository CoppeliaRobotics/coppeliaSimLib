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
    int retVal = -1;
    CSceneObject* classObj = getClass(classHandle);
    QJsonDocument doc = QJsonDocument::fromJson(classObj->getObjectMetaInfo().c_str());
    if ((!doc.isNull()) && doc.isObject())
    {
        QJsonObject jsonObj = doc.object();
        jsonObj["class"] = false;
        QJsonDocument newDoc(jsonObj);
        std::string newObjectMetaInfo = QString::fromUtf8(newDoc.toJson(QJsonDocument::Compact)).toStdString();
        CSceneObject* obj = classObj->copyYourself();
        obj->setObjectMetaInfo(newObjectMetaInfo.c_str());
        retVal = App::scene->sceneObjects->addObjectToScene(obj, false, true);
/*
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
    */
    }
    return retVal;
}

int CustomSceneObjectClassContainer::makeClass(CSceneObject* obj, const char* typeString)
{
    int retVal = -1;
    QJsonDocument doc = QJsonDocument::fromJson(obj->getObjectMetaInfo().c_str());
    if ((!doc.isNull()) && doc.isObject() && (getClass(typeString) == nullptr))
    {
        QJsonObject jsonObj = doc.object();
        jsonObj["class"] = true;
        QJsonDocument newDoc(jsonObj);
        std::string newObjectMetaInfo = QString::fromUtf8(newDoc.toJson(QJsonDocument::Compact)).toStdString();
        CSceneObject* copy = obj->copyYourself();
        copy->setObjectMetaInfo(newObjectMetaInfo.c_str());
        copy->setObjectTypeStr(typeString);
        copy->enableCustomizationPart();
        retVal = sim_object_sceneobjectclassstart;
        while (getClass(retVal) != nullptr)
            retVal++;
        copy->setObjectHandle(retVal);
        _customClasses.insert({typeString, copy});
        /*
        copy->setStringProperty("name", className);
        copy->setPropertyInfo("name", sim_propertyinfo_notwritable | sim_propertyinfo_constant | sim_propertyinfo_modelhashexclude, "");
        copy->setBoolProperty("customClass", true);
        copy->setPropertyInfo("customClass", sim_propertyinfo_notwritable | sim_propertyinfo_constant | sim_propertyinfo_modelhashexclude, "");
        */
        _notifyClassListChanged();
    }
    return retVal;
}

bool CustomSceneObjectClassContainer::removeClass(const char* typeString)
{
    bool retVal = false;
    auto it = _customClasses.find(typeString);
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

CSceneObject* CustomSceneObjectClassContainer::getClass(const char* typeString) const
{
    CSceneObject* retVal = nullptr;
    auto it = _customClasses.find(typeString);
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

int CustomSceneObjectClassContainer::setBoolProperty_t(long long int target, const char* ppName, bool pState)
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->setBoolProperty(ppName, pState);
    return retVal;
}

int CustomSceneObjectClassContainer::getBoolProperty_t(long long int target, const char* ppName, bool& pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->getBoolProperty(ppName, pState);
    return retVal;
}

int CustomSceneObjectClassContainer::setIntProperty_t(long long int target, const char* ppName, int pState)
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->setIntProperty(ppName, pState);
    return retVal;
}

int CustomSceneObjectClassContainer::getIntProperty_t(long long int target, const char* ppName, int& pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->getIntProperty(ppName, pState);
    return retVal;
}

int CustomSceneObjectClassContainer::setLongProperty_t(long long int target, const char* ppName, long long int pState)
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->setLongProperty(ppName, pState);
    return retVal;
}

int CustomSceneObjectClassContainer::getLongProperty_t(long long int target, const char* ppName, long long int& pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->getLongProperty(ppName, pState);
    return retVal;
}

int CustomSceneObjectClassContainer::setHandleProperty_t(long long int target, const char* ppName, long long int pState)
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->setHandleProperty(ppName, pState);
    return retVal;
}

int CustomSceneObjectClassContainer::getHandleProperty_t(long long int target, const char* ppName, long long int& pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->getHandleProperty(ppName, pState);
    return retVal;
}

int CustomSceneObjectClassContainer::setFloatProperty_t(long long int target, const char* ppName, double pState)
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->setFloatProperty(ppName, pState);
    return retVal;
}

int CustomSceneObjectClassContainer::getFloatProperty_t(long long int target, const char* ppName, double& pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->getFloatProperty(ppName, pState);
    return retVal;
}

int CustomSceneObjectClassContainer::setStringProperty_t(long long int target, const char* ppName, const std::string& pState)
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->setStringProperty(ppName, pState);
    return retVal;
}

int CustomSceneObjectClassContainer::getStringProperty_t(long long int target, const char* ppName, std::string& pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->getStringProperty(ppName, pState);
    return retVal;
}

int CustomSceneObjectClassContainer::setTableProperty_t(long long int target, const char* ppName, const std::string& pState)
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->setTableProperty(ppName, pState);
    return retVal;
}

int CustomSceneObjectClassContainer::getTableProperty_t(long long int target, const char* ppName, std::string& pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->getTableProperty(ppName, pState);
    return retVal;
}

int CustomSceneObjectClassContainer::setBufferProperty_t(long long int target, const char* ppName, const std::string& pState)
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->setBufferProperty(ppName, pState);
    return retVal;
}

int CustomSceneObjectClassContainer::getBufferProperty_t(long long int target, const char* ppName, std::string& pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->getBufferProperty(ppName, pState);
    return retVal;
}

int CustomSceneObjectClassContainer::setIntArray2Property_t(long long int target, const char* ppName, const int* pState)
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->setIntArray2Property(ppName, pState);
    return retVal;
}

int CustomSceneObjectClassContainer::getIntArray2Property_t(long long int target, const char* ppName, int* pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->getIntArray2Property(ppName, pState);
    return retVal;
}

int CustomSceneObjectClassContainer::setVector3Property_t(long long int target, const char* ppName, const C3Vector& pState)
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->setVector3Property(ppName, pState);
    return retVal;
}

int CustomSceneObjectClassContainer::getVector3Property_t(long long int target, const char* ppName, C3Vector& pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->getVector3Property(ppName, pState);
    return retVal;
}

int CustomSceneObjectClassContainer::setMatrixProperty_t(long long int target, const char* ppName, const CMatrix& pState)
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->setMatrixProperty(ppName, pState);
    return retVal;
}

int CustomSceneObjectClassContainer::getMatrixProperty_t(long long int target, const char* ppName, CMatrix& pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->getMatrixProperty(ppName, pState);
    return retVal;
}

int CustomSceneObjectClassContainer::setQuaternionProperty_t(long long int target, const char* ppName, const C4Vector& pState)
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->setQuaternionProperty(ppName, pState);
    return retVal;
}

int CustomSceneObjectClassContainer::getQuaternionProperty_t(long long int target, const char* ppName, C4Vector& pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->getQuaternionProperty(ppName, pState);
    return retVal;
}

int CustomSceneObjectClassContainer::setPoseProperty_t(long long int target, const char* ppName, const C7Vector& pState)
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->setPoseProperty(ppName, pState);
    return retVal;
}

int CustomSceneObjectClassContainer::getPoseProperty_t(long long int target, const char* ppName, C7Vector& pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->getPoseProperty(ppName, pState);
    return retVal;
}

int CustomSceneObjectClassContainer::setColorProperty_t(long long int target, const char* ppName, const float* pState)
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->setColorProperty(ppName, pState);
    return retVal;
}

int CustomSceneObjectClassContainer::getColorProperty_t(long long int target, const char* ppName, float* pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->getColorProperty(ppName, pState);
    return retVal;
}

int CustomSceneObjectClassContainer::setFloatArrayProperty_t(long long int target, const char* ppName, const std::vector<double>& pState)
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->setFloatArrayProperty(ppName, pState);
    return retVal;
}

int CustomSceneObjectClassContainer::getFloatArrayProperty_t(long long int target, const char* ppName, std::vector<double>& pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->getFloatArrayProperty(ppName, pState);
    return retVal;
}

int CustomSceneObjectClassContainer::setIntArrayProperty_t(long long int target, const char* ppName, const std::vector<int>& pState)
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->setIntArrayProperty(ppName, pState);
    return retVal;
}

int CustomSceneObjectClassContainer::getIntArrayProperty_t(long long int target, const char* ppName, std::vector<int>& pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->getIntArrayProperty(ppName, pState);
    return retVal;
}

int CustomSceneObjectClassContainer::setHandleArrayProperty_t(long long int target, const char* ppName, const std::vector<long long int>& pState)
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->setHandleArrayProperty(ppName, pState);
    return retVal;
}

int CustomSceneObjectClassContainer::getHandleArrayProperty_t(long long int target, const char* ppName, std::vector<long long int>& pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->getHandleArrayProperty(ppName, pState);
    return retVal;
}

int CustomSceneObjectClassContainer::setStringArrayProperty_t(long long int target, const char* ppName, const std::vector<std::string>& pState)
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->setStringArrayProperty(ppName, pState);
    return retVal;
}

int CustomSceneObjectClassContainer::getStringArrayProperty_t(long long int target, const char* ppName, std::vector<std::string>& pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->getStringArrayProperty(ppName, pState);
    return retVal;
}

int CustomSceneObjectClassContainer::setMethodProperty_t(long long int target, const char* ppName, const void* pState)
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->setMethodProperty(ppName, pState);
    return retVal;
}

int CustomSceneObjectClassContainer::getMethodProperty_t(long long int target, const char* ppName, void*& pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->getMethodProperty(ppName, pState);
    return retVal;
}

int CustomSceneObjectClassContainer::setMethodProperty_t(long long int target, const char* ppName, const std::string& pState)
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->setMethodProperty(ppName, pState);
    return retVal;
}

int CustomSceneObjectClassContainer::getMethodProperty_t(long long int target, const char* ppName, std::string& pState) const
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->getMethodProperty(ppName, pState);
    return retVal;
}

int CustomSceneObjectClassContainer::removeProperty_t(long long int target, const char* ppName)
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->removeProperty(ppName);
    return retVal;
}

int CustomSceneObjectClassContainer::getPropertyName_t(long long int target, int& index, std::string& pName, std::string& appartenance, int excludeFlags) const
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->getPropertyName(index, pName, appartenance, excludeFlags);
    return retVal;
}

int CustomSceneObjectClassContainer::getPropertyInfo_t(long long int target, const char* ppName, int& info, std::string& infoTxt) const
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->getPropertyInfo(ppName, info, infoTxt);
    return retVal;
}

int CustomSceneObjectClassContainer::setPropertyInfo_t(long long int target, const char* ppName, int info, const char* infoTxt)
{
    int retVal = sim_propertyret_unknowntarget;
    CSceneObject* obj = getClass(target);
    if (obj != nullptr)
        retVal = obj->setPropertyInfo(ppName, info, infoTxt);
    return retVal;
}
