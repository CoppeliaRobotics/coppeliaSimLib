#include <customObject.h>
#include <utils.h>
#include <app.h>

static std::string OBJECT_META_INFO = R"(
{
    "superclass": "object",
    "namespaces": {
    }
}
)";

CustomObject::CustomObject(long long int handle, const char* objectTypeStr, const char* objectMetaInfo, int originScriptHandle, int target)
{
    _objectHandle = handle;
    _objectTypeStr = objectTypeStr;
    _objectMetaInfo = objectMetaInfo;
    _scriptHandle = originScriptHandle;
    _target = target;
    _volatile = true;
    _changed = false;
    _ignoreSetterGetter = false;
    _objectCanAddRemoveProperty = false;
}

CustomObject::~CustomObject()
{
    if (!isClass())
    {
        if ((App::scenes != nullptr) && App::scenes->getEventsEnabled())
        {
            App::scenes->createEvent(EVENTTYPE_OBJECTREMOVED, _objectHandle, _objectHandle, nullptr, false);
            App::scenes->pushEvent();
        }
    }
}

CustomObject* CustomObject::createObject(long long int handle, int originScriptHandle) const
{
    CustomObject* retVal = nullptr;
    QJsonDocument doc = QJsonDocument::fromJson(_objectMetaInfo.c_str());
    if ((!doc.isNull()) && doc.isObject())
    {
        QJsonObject jsonObj = doc.object();
        jsonObj["class"] = false;
        QJsonDocument newDoc(jsonObj);
        std::string newObjectMetaInfo = QString::fromUtf8(newDoc.toJson(QJsonDocument::Compact)).toStdString();
        retVal = new CustomObject(handle, _objectTypeStr.c_str(), newObjectMetaInfo.c_str(), originScriptHandle, _target);
        retVal->_customProperties.copyFromExceptMethods(&_customProperties);
    }
    return retVal;
}

int CustomObject::getScriptHandle() const
{
    return _scriptHandle;
}

bool CustomObject::getVolatile() const
{
    return _volatile;
}

void CustomObject::setVolatile(bool v)
{
    _volatile = v;
}

bool CustomObject::getResetChanged()
{
    bool retVal = _changed;
    _changed = false;
    return retVal;
}

void CustomObject::setIgnoreSetterGetter(bool f)
{
    _ignoreSetterGetter = f;
}

void CustomObject::setObjectCanAddRemoveProperty(bool f)
{
    _objectCanAddRemoveProperty = f;
}

void CustomObject::_triggerEvent(const char* pName, CCbor* evv /*= nullptr*/)
{
    _changed = true;
    _ignoreSetterGetter = true;
    if ((!isClass()) && (App::scenes != nullptr) && App::scenes->getEventsEnabled())
    {
        int flags;
        std::string infoTxt;
        int t = getPropertyInfo(pName, flags, infoTxt);
        if (t >= sim_propertytype_start)
        {
            if ((flags & (sim_propertyinfo_silent)) == 0)
            {
                CCbor* ev = evv;
                if (evv == nullptr)
                    ev = App::scenes->createEvent(EVENTTYPE_OBJECTCHANGED, _objectHandle, _objectHandle, nullptr, false);
                if (t == sim_propertytype_bool)
                {
                    bool v;
                    if (getBoolProperty(pName, v) == sim_propertyret_ok)
                        ev->appendKeyBool(pName, v);
                }
                else if (t == sim_propertytype_int)
                {
                    int v;
                    if (getIntProperty(pName, v) == sim_propertyret_ok)
                        ev->appendKeyInt64(pName, v);
                }
                else if (t == sim_propertytype_long)
                {
                    long long int v;
                    if (getLongProperty(pName, v) == sim_propertyret_ok)
                        ev->appendKeyInt64(pName, v);
                }
                else if (t == sim_propertytype_float)
                {
                    double v;
                    if (getFloatProperty(pName, v) == sim_propertyret_ok)
                        ev->appendKeyDouble(pName, v);
                }
                else if (t == sim_propertytype_handle)
                {
                    long long int v;
                    if (getHandleProperty(pName, v) == sim_propertyret_ok)
                        ev->appendKeyHandle(pName, v);
                }
                else if (t == sim_propertytype_string)
                {
                    std::string v;
                    if (getStringProperty(pName, v) == sim_propertyret_ok)
                        ev->appendKeyText(pName, v.c_str());
                }
                else if (t == sim_propertytype_table)
                {
                    std::string v;
                    if (getTableProperty(pName, v) == sim_propertyret_ok)
                        ev->appendKeyBuff(pName, (unsigned char*)v.data(), v.size());
                }
                else if (t == sim_propertytype_buffer)
                {
                    std::string v;
                    if (getBufferProperty(pName, v) == sim_propertyret_ok)
                        ev->appendKeyBuff(pName, (unsigned char*)v.data(), v.size());
                }
                else if (t == sim_propertytype_intarray2)
                {
                    int v[2];
                    if (getIntArray2Property(pName, v) == sim_propertyret_ok)
                        ev->appendKeyInt32Array(pName, v, 2);
                }
                else if (t == sim_propertytype_vector3)
                {
                    C3Vector v;
                    if (getVector3Property(pName, v) == sim_propertyret_ok)
                        ev->appendKeyVector3(pName, v);
                }
                else if (t == sim_propertytype_quaternion)
                {
                    C4Vector v;
                    if (getQuaternionProperty(pName, v) == sim_propertyret_ok)
                        ev->appendKeyQuaternion(pName, v);
                }
                else if (t == sim_propertytype_pose)
                {
                    C7Vector v;
                    if (getPoseProperty(pName, v) == sim_propertyret_ok)
                        ev->appendKeyPose(pName, v);
                }
                else if (t == sim_propertytype_matrix)
                {
                    CMatrix v;
                    if (getMatrixProperty(pName, v) == sim_propertyret_ok)
                        ev->appendKeyMatrix(pName, v.data.data(), v.rows, v.cols);
                }
                else if (t == sim_propertytype_color)
                {
                    float v[3];
                    if (getColorProperty(pName, v) == sim_propertyret_ok)
                        ev->appendKeyFloatArray(pName, v, 3);
                }
                else if (t == sim_propertytype_floatarray)
                {
                    std::vector<double> v;
                    if (getFloatArrayProperty(pName, v) == sim_propertyret_ok)
                        ev->appendKeyDoubleArray(pName, v.data(), v.size());
                }
                else if (t == sim_propertytype_intarray)
                {
                    std::vector<int> v;
                    if (getIntArrayProperty(pName, v) == sim_propertyret_ok)
                        ev->appendKeyInt32Array(pName, v.data(), v.size());
                }
                else if (t == sim_propertytype_handlearray)
                {
                    std::vector<long long int> v;
                    if (getHandleArrayProperty(pName, v) == sim_propertyret_ok)
                        ev->appendKeyHandleArray(pName, v.data(), v.size());
                }
                else if (t == sim_propertytype_stringarray)
                {
                    std::vector<std::string> v;
                    if (getStringArrayProperty(pName, v) == sim_propertyret_ok)
                        ev->appendKeyTextArray(pName, v);
                }
                if (evv == nullptr)
                    App::scenes->pushEvent();
            }
        }
        else
        { // property removed
            CCbor* ev = evv;
            if (evv == nullptr)
                ev = App::scenes->createEvent(EVENTTYPE_OBJECTCHANGED, _objectHandle, _objectHandle, nullptr, false);
            ev->appendKeyNull(pName);
            if (evv == nullptr)
                App::scenes->pushEvent();
        }
    }
    _ignoreSetterGetter = false;
}

void CustomObject::pushObjectCreationEvent()
{
    if ((App::scenes != nullptr) && App::scenes->getEventsEnabled())
    {
        CCbor* ev = App::scenes->createEvent(EVENTTYPE_OBJECTADDED, _objectHandle, _objectHandle, nullptr, false);
        ev->appendKeyText(propObject_objectType.name, getObjectTypeStr().c_str());
        int indexI = 0;
        int index = indexI;
        std::string pName, appartenance;
        while (sim_propertyret_ok == getPropertyName(index, pName, appartenance, 0))
        {
           if (appartenance == getObjectTypeStr())
                _triggerEvent(pName.c_str(), ev);
            index = ++indexI;
            pName.clear();
        }
        App::scenes->pushEvent();
    }
}

void CustomObject::serialize(CSer& ar)
{
    if (ar.isBinary())
    {
        if (ar.isStoring())
        { // Storing
            std::vector<std::string> names;
            std::vector<std::string> data;
            _customProperties.getAllPropertyData(names, data);

            ar.storeDataName("obj");
            ar << _objectHandle;
            ar << _objectTypeStr;
            ar << _objectMetaInfo;
            ar << int(names.size());
            ar.flush();

            for (size_t i = 0; i < names.size(); i++)
            {
                ar.storeDataName("pro");
                ar << names[i];
                ar << data[i];
                ar.flush();
            }

            ar.storeDataName(SER_END_OF_OBJECT);
        }
        else
        { // Loading
            int byteQuantity;
            std::string theName = "";
            std::vector<std::string> names;
            std::vector<std::string> data;
            while (theName.compare(SER_END_OF_OBJECT) != 0)
            {
                theName = ar.readDataName();
                if (theName.compare(SER_END_OF_OBJECT) != 0)
                {
                    bool noHit = true;
                    if (theName.compare("obj") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _objectHandle;
                        ar >> _objectTypeStr;
                        ar >> _objectMetaInfo;
                        int cnt;
                        ar >> cnt;
                    }

                    if (theName.compare("pro") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        std::string s;
                        ar >> s;
                        names.push_back(s);
                        ar >> s;
                        data.push_back(s);
                    }
                    if (noHit)
                        ar.loadUnknownData();
                }
            }
            _customProperties.setAllPropertyData(names, data);
        }
    }
    else
    {
        if (ar.isStoring())
        {
            std::vector<std::string> names;
            std::vector<std::string> data;
            _customProperties.getAllPropertyData(names, data);

            ar.xmlAddNode_longlong("handle", _objectHandle);
            ar.xmlAddNode_string("type", _objectTypeStr.c_str());
            ar.xmlAddNode_string("metaInfo", _objectMetaInfo.c_str());

            for (size_t i = 0; i < names.size(); i++)
            {
                ar.xmlPushNewNode("property");
                ar.xmlAddNode_string("name", names[i].c_str());
                ar.xmlAddNode_string("data", utils::encode64(data[i]).c_str());
                ar.xmlPopNode();
            }
        }
        else
        {
            ar.xmlGetNode_longlong("handle", _objectHandle);
            ar.xmlGetNode_string("type", _objectTypeStr);
            ar.xmlGetNode_string("metaInfo", _objectMetaInfo);

            if (ar.xmlPushChildNode("property", false))
            {
                std::vector<std::string> names;
                std::vector<std::string> data;
                while (true)
                {
                    std::string d;
                    ar.xmlGetNode_string("name", d);
                    names.push_back(d);
                    ar.xmlGetNode_string("data", d);
                    data.push_back(utils::decode64(d));
                    if (!ar.xmlPushSiblingNode("property", false))
                        break;
                }
                ar.xmlPopNode();
                _customProperties.setAllPropertyData(names, data);
            }
        }
    }
}

template <typename T, typename PushF, typename GetF>
void CustomObject::_callPropertySetterGetter(const char* pName, const char* suffix, T& pState, PushF pushFunc, GetF getFunc) const
{
    int info;
    std::string infoTxt;
    if (sim_propertytype_method == getPropertyInfo((std::string(pName) + suffix).c_str(), info, infoTxt))
    {
        CInterfaceStack* inStack = App::scenes->interfaceStackContainer->createStack();
        CInterfaceStack* outStack = App::scenes->interfaceStackContainer->createStack();
        inStack->pushTextOntoStack(pName);
        pushFunc(inStack, pState);
        int ret = simCallMethod_internal(_objectHandle, (std::string(pName) + suffix).c_str(), inStack->getObjectHandle(), outStack->getObjectHandle(), -1);
        std::string err;
        if (ret == 1)
        {
            if (!getFunc(outStack, pState))
                err = "invalid return value.";
        }
        else
        {
            char* d = simGetLastError_internal();
            err = d;
            delete[] d;
        }
        App::scenes->interfaceStackContainer->destroyStack(outStack);
        App::scenes->interfaceStackContainer->destroyStack(inStack);
        if (!err.empty())
        {
            err = std::string("error in property setter/getter for object ") + std::to_string(_objectHandle) + ": " + err;
            App::logScriptMsg(nullptr, sim_verbosity_scripterrors, err.c_str());
        }
    }
}


int CustomObject::setBoolProperty(const char* pName, bool pState)
{
    int retVal = Obj::setBoolProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        if (isClass() || _objectCanAddRemoveProperty || _customProperties.hasTypedProperty(pName, sim_propertytype_bool))
        { // property already exists (with correct type), or we want to set it to a class
            if ((!_ignoreSetterGetter) && (!isClass()))
                _callPropertySetterGetter(pName, SET_SUFFIX, pState, [](CInterfaceStack* s, const bool& v) { s->pushBoolOntoStack(v); }, [](CInterfaceStack* s, bool& v) { return s->getStackBoolValue(v); });
            bool changed = false;
            retVal = _customProperties.setBoolProperty(pName, pState, changed);
            if (changed)
                _triggerEvent(pName);
        }
    }
    return retVal;
}

int CustomObject::getBoolProperty(const char* pName, bool& pState) const
{
    int retVal = Obj::getBoolProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        retVal = _customProperties.getBoolProperty(pName, pState);
        if ((retVal == sim_propertyret_ok) && (!_ignoreSetterGetter) && (!isClass()))
            _callPropertySetterGetter(pName, GET_SUFFIX, pState, [](CInterfaceStack* s, const bool& v) { s->pushBoolOntoStack(v); }, [](CInterfaceStack* s, bool& v) { return s->getStackBoolValue(v); });
    }
    return retVal;
}

int CustomObject::setIntProperty(const char* pName, int pState)
{
    int retVal = Obj::setIntProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        if (isClass() || _objectCanAddRemoveProperty || _customProperties.hasTypedProperty(pName, sim_propertytype_int))
        { // property already exists (with correct type), or we want to set it to a class
            _callPropertySetterGetter(pName, SET_SUFFIX, pState, [](CInterfaceStack* s, const int& v) { s->pushInt32OntoStack(v); }, [](CInterfaceStack* s, int& v) { return s->getStackInt32Value(v); });
            bool changed = false;
            retVal = _customProperties.setIntProperty(pName, pState, changed);
            if (changed)
                _triggerEvent(pName);
        }
    }
    return retVal;
}

int CustomObject::getIntProperty(const char* pName, int& pState) const
{
    int retVal = Obj::getIntProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        retVal = _customProperties.getIntProperty(pName, pState);
        if ((retVal == sim_propertyret_ok) && (!_ignoreSetterGetter) && (!isClass()))
            _callPropertySetterGetter(pName, GET_SUFFIX, pState, [](CInterfaceStack* s, const int& v) { s->pushInt32OntoStack(v); }, [](CInterfaceStack* s, int& v) { return s->getStackInt32Value(v); });
    }
    return retVal;
}

int CustomObject::setLongProperty(const char* pName, long long int pState)
{
    int retVal = Obj::setLongProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        if (isClass() || _objectCanAddRemoveProperty || _customProperties.hasTypedProperty(pName, sim_propertytype_long))
        { // property already exists (with correct type), or we want to set it to a class
            if ((!_ignoreSetterGetter) && (!isClass()))
                _callPropertySetterGetter(pName, SET_SUFFIX, pState, [](CInterfaceStack* s, const long long int& v) { s->pushInt64OntoStack(v); }, [](CInterfaceStack* s, long long int& v) { return s->getStackInt64Value(v); });
            bool changed = false;
            retVal = _customProperties.setLongProperty(pName, pState, changed);
            if (changed)
                _triggerEvent(pName);
        }
    }
    return retVal;
}

int CustomObject::getLongProperty(const char* pName, long long int& pState) const
{
    int retVal = Obj::getLongProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        retVal = _customProperties.getLongProperty(pName, pState);
        if ((retVal == sim_propertyret_ok) && (!_ignoreSetterGetter) && (!isClass()))
            _callPropertySetterGetter(pName, GET_SUFFIX, pState, [](CInterfaceStack* s, const long long& v) { s->pushInt64OntoStack(v); }, [](CInterfaceStack* s, long long int& v) { return s->getStackInt64Value(v); });
    }
    return retVal;
}

int CustomObject::setFloatProperty(const char* pName, double pState)
{
    int retVal = Obj::setFloatProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        if (isClass() || _objectCanAddRemoveProperty || _customProperties.hasTypedProperty(pName, sim_propertytype_float))
        { // property already exists (with correct type), or we want to set it to a class
            if ((!_ignoreSetterGetter) && (!isClass()))
                _callPropertySetterGetter(pName, SET_SUFFIX, pState, [](CInterfaceStack* s, const double& v) { s->pushDoubleOntoStack(v); }, [](CInterfaceStack* s, double& v) { return s->getStackDoubleValue(v); });
            bool changed = false;
            retVal = _customProperties.setFloatProperty(pName, pState, changed);
            if (changed)
                _triggerEvent(pName);
        }
    }
    return retVal;
}

int CustomObject::getFloatProperty(const char* pName, double& pState) const
{
    int retVal = Obj::getFloatProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        retVal = _customProperties.getFloatProperty(pName, pState);
        if ((retVal == sim_propertyret_ok) && (!_ignoreSetterGetter) && (!isClass()))
            _callPropertySetterGetter(pName, GET_SUFFIX, pState, [](CInterfaceStack* s, const double& v) { s->pushDoubleOntoStack(v); }, [](CInterfaceStack* s, double& v) { return s->getStackDoubleValue(v); });
    }
    return retVal;
}

int CustomObject::setHandleProperty(const char* pName, long long int pState)
{
    int retVal = Obj::setHandleProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        if (isClass() || _objectCanAddRemoveProperty || _customProperties.hasTypedProperty(pName, sim_propertytype_handle))
        { // property already exists (with correct type), or we want to set it to a class
            if ((!_ignoreSetterGetter) && (!isClass()))
                _callPropertySetterGetter(pName, SET_SUFFIX, pState, [](CInterfaceStack* s, const long long int& v) { s->pushHandleOntoStack(v); }, [](CInterfaceStack* s, long long int& v) { return s->getStackHandleValue(v); });
            bool changed = false;
            retVal = _customProperties.setHandleProperty(pName, pState, changed);
            if (changed)
                _triggerEvent(pName);
        }
    }
    return retVal;
}

int CustomObject::getHandleProperty(const char* pName, long long int& pState) const
{
    int retVal = Obj::getHandleProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        retVal = _customProperties.getHandleProperty(pName, pState);
        if ((retVal == sim_propertyret_ok) && (!_ignoreSetterGetter) && (!isClass()))
            _callPropertySetterGetter(pName, GET_SUFFIX, pState, [](CInterfaceStack* s, const long long int& v) { s->pushHandleOntoStack(v); }, [](CInterfaceStack* s, long long int& v) { return s->getStackHandleValue(v); });
    }
    return retVal;
}

int CustomObject::setStringProperty(const char* pName, const std::string& pState)
{
    int retVal = Obj::setStringProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        if (isClass() || _objectCanAddRemoveProperty || _customProperties.hasTypedProperty(pName, sim_propertytype_string))
        { // property already exists (with correct type), or we want to set it to a class
            std::string pp(pState);
            if ((!_ignoreSetterGetter) && (!isClass()))
                _callPropertySetterGetter(pName, SET_SUFFIX, pp, [](CInterfaceStack* s, const std::string& v) { s->pushTextOntoStack(v.c_str()); }, [](CInterfaceStack* s, std::string& v) { return s->getStackStringValue(v); });
            bool changed = false;
            retVal = _customProperties.setStringProperty(pName, pp, changed);
            if (changed)
                _triggerEvent(pName);
        }
    }
    return retVal;
}

int CustomObject::getStringProperty(const char* pName, std::string& pState) const
{
    int retVal = Obj::getStringProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        retVal = _customProperties.getStringProperty(pName, pState);
        if ((retVal == sim_propertyret_ok) && (!_ignoreSetterGetter) && (!isClass()))
            _callPropertySetterGetter(pName, GET_SUFFIX, pState, [](CInterfaceStack* s, const std::string& v) { s->pushTextOntoStack(v.c_str()); }, [](CInterfaceStack* s, std::string& v) { return s->getStackStringValue(v); });
    }
    return retVal;
}

int CustomObject::setTableProperty(const char* pName, const std::string& pState)
{
    int retVal = Obj::setStringProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        if (isClass() || _objectCanAddRemoveProperty || _customProperties.hasTypedProperty(pName, sim_propertytype_table))
        { // property already exists (with correct type), or we want to set it to a class
            std::string pp(pState);
            if ((!_ignoreSetterGetter) && (!isClass()))
                _callPropertySetterGetter(pName, SET_SUFFIX, pp, [](CInterfaceStack* s, const std::string& v) { s->pushBufferOntoStack(v.data(), v.size()); }, [](CInterfaceStack* s, std::string& v) { return s->getStackStringValue(v); });
            bool changed = false;
            retVal = _customProperties.setTableProperty(pName, pp, changed);
            if (changed)
                _triggerEvent(pName);
        }
    }
    return retVal;
}

int CustomObject::getTableProperty(const char* pName, std::string& pState) const
{
    int retVal = Obj::getStringProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        retVal = _customProperties.getTableProperty(pName, pState);
        if ((retVal == sim_propertyret_ok) && (!_ignoreSetterGetter) && (!isClass()))
            _callPropertySetterGetter(pName, GET_SUFFIX, pState, [](CInterfaceStack* s, const std::string& v) { s->pushBufferOntoStack(v.data(), v.size()); }, [](CInterfaceStack* s, std::string& v) { return s->getStackStringValue(v); });
    }
    return retVal;
}

int CustomObject::setBufferProperty(const char* pName, const std::string& pState)
{
    int retVal = Obj::setBufferProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        if (isClass() || _objectCanAddRemoveProperty || _customProperties.hasTypedProperty(pName, sim_propertytype_buffer))
        { // property already exists (with correct type), or we want to set it to a class
            std::string pp(pState);
            if ((!_ignoreSetterGetter) && (!isClass()))
                _callPropertySetterGetter(pName, SET_SUFFIX, pp, [](CInterfaceStack* s, const std::string& v) { s->pushBufferOntoStack(v.data(), v.size()); }, [](CInterfaceStack* s, std::string& v) { return s->getStackStringValue(v); });
            bool changed = false;
            retVal = _customProperties.setBufferProperty(pName, pp, changed);
            if (changed)
                _triggerEvent(pName);
        }
    }
    return retVal;
}

int CustomObject::getBufferProperty(const char* pName, std::string& pState) const
{
    int retVal = Obj::getBufferProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        retVal = _customProperties.getBufferProperty(pName, pState);
        if ((retVal == sim_propertyret_ok) && (!_ignoreSetterGetter) && (!isClass()))
            _callPropertySetterGetter(pName, GET_SUFFIX, pState, [](CInterfaceStack* s, const std::string& v) { s->pushBufferOntoStack(v.data(), v.size()); }, [](CInterfaceStack* s, std::string& v) { return s->getStackStringValue(v); });
    }
    return retVal;
}

int CustomObject::setIntArray2Property(const char* pName, const int* pState)
{
    int retVal = Obj::setIntArray2Property(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        if (isClass() || _objectCanAddRemoveProperty || _customProperties.hasTypedProperty(pName, sim_propertytype_intarray2))
        { // property already exists (with correct type), or we want to set it to a class
            int pp[2] = {pState[0], pState[1]};
            if ((!_ignoreSetterGetter) && (!isClass()))
                _callPropertySetterGetter(pName, SET_SUFFIX, pp, [](CInterfaceStack* s, const int (&v)[2]) { s->pushInt32ArrayOntoStack(v, 2); }, [](CInterfaceStack* s, int (&v)[2]) { return s->getStackInt32Array(v, 2); });
            bool changed = false;
            retVal = _customProperties.setIntArray2Property(pName, pp, changed);
            if (changed)
                _triggerEvent(pName);
        }
    }
    return retVal;
}

int CustomObject::getIntArray2Property(const char* pName, int* pState) const
{
    int retVal = Obj::getIntArray2Property(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        retVal = _customProperties.getIntArray2Property(pName, pState);
        if ((retVal == sim_propertyret_ok) && (!_ignoreSetterGetter) && (!isClass()))
        {
            int pp[2] = {pState[0], pState[1]};
            _callPropertySetterGetter(pName, GET_SUFFIX, pp, [](CInterfaceStack* s, const int (&v)[2]) { s->pushInt32ArrayOntoStack(v, 2); }, [](CInterfaceStack* s, int (&v)[2]) { return s->getStackInt32Array(v, 2); });
            pState[0] = pp[0];
            pState[1] = pp[1];
        }
    }
    return retVal;
}

int CustomObject::setVector3Property(const char* pName, const C3Vector& pState)
{
    int retVal = Obj::setVector3Property(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        if (isClass() || _objectCanAddRemoveProperty || _customProperties.hasTypedProperty(pName, sim_propertytype_vector3))
        { // property already exists (with correct type), or we want to set it to a class
            CMatrix m(3, 1);
            m.data.assign(pState.data, pState.data + 3);
            if ((!_ignoreSetterGetter) && (!isClass()))
                _callPropertySetterGetter(pName, SET_SUFFIX, m, [](CInterfaceStack* s, const CMatrix& v) { s->pushMatrixOntoStack(v); }, [](CInterfaceStack* s, CMatrix& v) { return s->getStackMatrix(v); });
            C3Vector p(m.data.data());
            bool changed = false;
            retVal = _customProperties.setVector3Property(pName, p, changed);
            if (changed)
                _triggerEvent(pName);
        }
    }
    return retVal;
}

int CustomObject::getVector3Property(const char* pName, C3Vector& pState) const
{
    int retVal = Obj::getVector3Property(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        retVal = _customProperties.getVector3Property(pName, pState);
        if ((retVal == sim_propertyret_ok) && (!_ignoreSetterGetter) && (!isClass()))
        {
            CMatrix m(3, 1);
            m.data.assign(pState.data, pState.data + 3);
            _callPropertySetterGetter(pName, GET_SUFFIX, m, [](CInterfaceStack* s, const CMatrix& v) { s->pushMatrixOntoStack(v); }, [](CInterfaceStack* s, CMatrix& v) { return s->getStackMatrix(v); });
            pState.setData(m.data.data());
        }
    }
    return retVal;
}

int CustomObject::setMatrixProperty(const char* pName, const CMatrix& pState)
{
    int retVal = Obj::setMatrixProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        if (isClass() || _objectCanAddRemoveProperty || _customProperties.hasTypedProperty(pName, sim_propertytype_matrix))
        { // property already exists (with correct type), or we want to set it to a class
            CMatrix pp(pState);
            if ((!_ignoreSetterGetter) && (!isClass()))
                _callPropertySetterGetter(pName, SET_SUFFIX, pp, [](CInterfaceStack* s, const CMatrix& v) { s->pushMatrixOntoStack(v); }, [](CInterfaceStack* s, CMatrix& v) { return s->getStackMatrix(v); });
            bool changed = false;
            retVal = _customProperties.setMatrixProperty(pName, pp, changed);
            if (changed)
                _triggerEvent(pName);
        }
    }
    return retVal;
}

int CustomObject::getMatrixProperty(const char* pName, CMatrix& pState) const
{
    int retVal = Obj::getMatrixProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        retVal = _customProperties.getMatrixProperty(pName, pState);
        if ((retVal == sim_propertyret_ok) && (!_ignoreSetterGetter) && (!isClass()))
            _callPropertySetterGetter(pName, GET_SUFFIX, pState, [](CInterfaceStack* s, const CMatrix& v) { s->pushMatrixOntoStack(v); }, [](CInterfaceStack* s, CMatrix& v) { return s->getStackMatrix(v); });
    }
    return retVal;
}

int CustomObject::setQuaternionProperty(const char* pName, const C4Vector& pState)
{
    int retVal = Obj::setQuaternionProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        if (isClass() || _objectCanAddRemoveProperty || _customProperties.hasTypedProperty(pName, sim_propertytype_quaternion))
        { // property already exists (with correct type), or we want to set it to a class
            C4Vector pp(pState);
            if ((!_ignoreSetterGetter) && (!isClass()))
                _callPropertySetterGetter(pName, SET_SUFFIX, pp, [](CInterfaceStack* s, const C4Vector& v) { s->pushQuaternionOntoStack(v); }, [](CInterfaceStack* s, C4Vector& v) { return s->getStackQuaternion(v); });
            bool changed = false;
            retVal = _customProperties.setQuaternionProperty(pName, pp, changed);
            if (changed)
                _triggerEvent(pName);
        }
    }
    return retVal;
}

int CustomObject::getQuaternionProperty(const char* pName, C4Vector& pState) const
{
    int retVal = Obj::getQuaternionProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        retVal = _customProperties.getQuaternionProperty(pName, pState);
        if ((retVal == sim_propertyret_ok) && (!_ignoreSetterGetter) && (!isClass()))
            _callPropertySetterGetter(pName, GET_SUFFIX, pState, [](CInterfaceStack* s, const C4Vector& v) { s->pushQuaternionOntoStack(v); }, [](CInterfaceStack* s, C4Vector& v) { return s->getStackQuaternion(v); });
    }
    return retVal;
}

int CustomObject::setPoseProperty(const char* pName, const C7Vector& pState)
{
    int retVal = Obj::setPoseProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        if (isClass() || _objectCanAddRemoveProperty || _customProperties.hasTypedProperty(pName, sim_propertytype_pose))
        { // property already exists (with correct type), or we want to set it to a class
            C7Vector pp(pState);
            if ((!_ignoreSetterGetter) && (!isClass()))
                _callPropertySetterGetter(pName, SET_SUFFIX, pp, [](CInterfaceStack* s, const C7Vector& v) { s->pushPoseOntoStack(v); }, [](CInterfaceStack* s, C7Vector& v) { return s->getStackPose(v); });
            bool changed = false;
            retVal = _customProperties.setPoseProperty(pName, pp, changed);
            if (changed)
                _triggerEvent(pName);
        }
    }
    return retVal;
}

int CustomObject::getPoseProperty(const char* pName, C7Vector& pState) const
{
    int retVal = Obj::getPoseProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        retVal = _customProperties.getPoseProperty(pName, pState);
        if ((retVal == sim_propertyret_ok) && (!_ignoreSetterGetter) && (!isClass()))
            _callPropertySetterGetter(pName, GET_SUFFIX, pState, [](CInterfaceStack* s, const C7Vector& v) { s->pushPoseOntoStack(v); }, [](CInterfaceStack* s, C7Vector& v) { return s->getStackPose(v); });
    }
    return retVal;
}

int CustomObject::setColorProperty(const char* pName, const float* pState)
{
    int retVal = Obj::setColorProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        if (isClass() || _objectCanAddRemoveProperty || _customProperties.hasTypedProperty(pName, sim_propertytype_color))
        { // property already exists (with correct type), or we want to set it to a class
            float pp[3] = {pState[0], pState[1], pState[2]};
            if ((!_ignoreSetterGetter) && (!isClass()))
                _callPropertySetterGetter(pName, SET_SUFFIX, pp, [](CInterfaceStack* s, const float (&v)[3]) { s->pushColorOntoStack(v); }, [](CInterfaceStack* s, float (&v)[3]) { return s->getStackColor(v); });
            bool changed = false;
            retVal = _customProperties.setColorProperty(pName, pp, changed);
            if (changed)
                _triggerEvent(pName);
        }
    }
    return retVal;
}

int CustomObject::getColorProperty(const char* pName, float* pState) const
{
    int retVal = Obj::getColorProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        retVal = _customProperties.getColorProperty(pName, pState);
        if ((retVal == sim_propertyret_ok) && (!_ignoreSetterGetter) && (!isClass()))
        {
            float pp[3] = {pState[0], pState[1], pState[2]};
            _callPropertySetterGetter(pName, GET_SUFFIX, pp, [](CInterfaceStack* s, const float (&v)[3]) { s->pushColorOntoStack(v); }, [](CInterfaceStack* s, float (&v)[3]) { return s->getStackColor(v); });
            pState[0] = pp[0];
            pState[1] = pp[1];
            pState[2] = pp[2];
        }
    }
    return retVal;
}

int CustomObject::setFloatArrayProperty(const char* pName, const std::vector<double>& pState)
{
    int retVal = Obj::setFloatArrayProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        if (isClass() || _objectCanAddRemoveProperty || _customProperties.hasTypedProperty(pName, sim_propertytype_floatarray))
        { // property already exists (with correct type), or we want to set it to a class
            std::vector<double> pp(pState);
            if ((!_ignoreSetterGetter) && (!isClass()))
                _callPropertySetterGetter(pName, SET_SUFFIX, pp, [](CInterfaceStack* s, const std::vector<double>& w) { s->pushDoubleArrayOntoStack(w.data(), w.size()); }, [](CInterfaceStack* s, std::vector<double>& w) { return s->getStackDoubleArray(w.data(), w.size()); });
            bool changed = false;
            retVal = _customProperties.setFloatArrayProperty(pName, pp, changed);
            if (changed)
                _triggerEvent(pName);
        }
    }
    return retVal;
}

int CustomObject::getFloatArrayProperty(const char* pName, std::vector<double>& pState) const
{
    pState.clear();
    int retVal = Obj::getFloatArrayProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        retVal = _customProperties.getFloatArrayProperty(pName, pState);
        if ((retVal == sim_propertyret_ok) && (!_ignoreSetterGetter) && (!isClass()))
            _callPropertySetterGetter(pName, GET_SUFFIX, pState, [](CInterfaceStack* s, const std::vector<double>& w) { s->pushDoubleArrayOntoStack(w.data(), w.size()); }, [](CInterfaceStack* s, std::vector<double>& w) { return s->getStackDoubleArray(w.data(), w.size()); });
    }
    return retVal;
}

int CustomObject::setIntArrayProperty(const char* pName, const std::vector<int>& pState)
{
    int retVal = Obj::setIntArrayProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        if (isClass() || _objectCanAddRemoveProperty || _customProperties.hasTypedProperty(pName, sim_propertytype_intarray))
        { // property already exists (with correct type), or we want to set it to a class
            std::vector<int> pp(pState);
            if ((!_ignoreSetterGetter) && (!isClass()))
                _callPropertySetterGetter(pName, SET_SUFFIX, pp, [](CInterfaceStack* s, const std::vector<int>& w) { s->pushInt32ArrayOntoStack(w.data(), w.size()); }, [](CInterfaceStack* s, std::vector<int>& w) { return s->getStackInt32Array(w.data(), w.size()); });
            bool changed = false;
            retVal = _customProperties.setIntArrayProperty(pName, pp, changed);
            if (changed)
                _triggerEvent(pName);
        }
    }
    return retVal;
}

int CustomObject::getIntArrayProperty(const char* pName, std::vector<int>& pState) const
{
    pState.clear();
    int retVal = Obj::getIntArrayProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        retVal = _customProperties.getIntArrayProperty(pName, pState);
        if ((retVal == sim_propertyret_ok) && (!_ignoreSetterGetter) && (!isClass()))
            _callPropertySetterGetter(pName, GET_SUFFIX, pState, [](CInterfaceStack* s, const std::vector<int>& w) { s->pushInt32ArrayOntoStack(w.data(), w.size()); }, [](CInterfaceStack* s, std::vector<int>& w) { return s->getStackInt32Array(w.data(), w.size()); });
    }
    return retVal;
}

int CustomObject::setHandleArrayProperty(const char* pName, const std::vector<long long int>& pState)
{
    int retVal = Obj::setHandleArrayProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        if (isClass() || _objectCanAddRemoveProperty || _customProperties.hasTypedProperty(pName, sim_propertytype_handlearray))
        { // property already exists (with correct type), or we want to set it to a class
            std::vector<long long int> pp(pState);
            if ((!_ignoreSetterGetter) && (!isClass()))
                _callPropertySetterGetter(pName, SET_SUFFIX, pp, [](CInterfaceStack* s, const std::vector<long long int>& w) { s->pushInt64ArrayOntoStack(w.data(), w.size()); }, [](CInterfaceStack* s, std::vector<long long int>& w) { return s->getStackInt64Array(w.data(), w.size()); });
            bool changed = false;
            retVal = _customProperties.setHandleArrayProperty(pName, pp, changed);
            if (changed)
                _triggerEvent(pName);
        }
    }
    return retVal;
}

int CustomObject::getHandleArrayProperty(const char* pName, std::vector<long long int>& pState) const
{
    pState.clear();
    int retVal = Obj::getHandleArrayProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        retVal = _customProperties.getHandleArrayProperty(pName, pState);
        if ((retVal == sim_propertyret_ok) && (!_ignoreSetterGetter) && (!isClass()))
            _callPropertySetterGetter(pName, GET_SUFFIX, pState, [](CInterfaceStack* s, const std::vector<long long int>& w) { s->pushInt64ArrayOntoStack(w.data(), w.size()); }, [](CInterfaceStack* s, std::vector<long long int>& w) { return s->getStackInt64Array(w.data(), w.size()); });
    }
    return retVal;
}

int CustomObject::setStringArrayProperty(const char* pName, const std::vector<std::string>& pState)
{
    int retVal = Obj::setStringArrayProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        if (isClass() || _objectCanAddRemoveProperty || _customProperties.hasTypedProperty(pName, sim_propertytype_stringarray))
        { // property already exists (with correct type), or we want to set it to a class
            std::vector<std::string> pp(pState);
            if ((!_ignoreSetterGetter) && (!isClass()))
                _callPropertySetterGetter(pName, SET_SUFFIX, pp, [](CInterfaceStack* s, const std::vector<std::string>& w) { s->pushTextArrayOntoStack(w.data(), w.size()); }, [](CInterfaceStack* s, std::vector<std::string>& w) { w.clear(); return s->getStackTextArray(w); });
            bool changed = false;
            retVal = _customProperties.setStringArrayProperty(pName, pp, changed);
            if (changed)
                _triggerEvent(pName);
        }
    }
    return retVal;
}

int CustomObject::getStringArrayProperty(const char* pName, std::vector<std::string>& pState) const
{
    pState.clear();
    int retVal = Obj::getStringArrayProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        retVal = _customProperties.getStringArrayProperty(pName, pState);
        if ((retVal == sim_propertyret_ok) && (!_ignoreSetterGetter) && (!isClass()))
            _callPropertySetterGetter(pName, GET_SUFFIX, pState, [](CInterfaceStack* s, const std::vector<std::string>& w) { s->pushTextArrayOntoStack(w.data(), w.size()); }, [](CInterfaceStack* s, std::vector<std::string>& w) { w.clear(); return s->getStackTextArray(w); });
    }
    return retVal;
}

int CustomObject::setMethodProperty(const char* pName, const void* pState)
{
    std::string ppN(pName);
    ppN += "@cfunc";
    int retVal = sim_propertyret_unknownproperty;
    if (retVal == sim_propertyret_unknownproperty)
    {
        if (isClass())
        {
            bool changed = false;
            retVal = _customProperties.setMethodProperty(ppN.c_str(), pState, changed);
        }
    }
    return retVal;
}

int CustomObject::getMethodProperty(const char* pName, void*& pState) const
{
    std::string ppN(pName);
    ppN += "@cfunc";
    int retVal = sim_propertyret_unknownproperty;
    if (retVal == sim_propertyret_unknownproperty)
        retVal = _customProperties.getMethodProperty(ppN.c_str(), pState);
    if ((!isClass()) && (retVal == sim_propertyret_unknownproperty))
    {
        CustomObject* cl = App::scenes->customObjects->getClass(getObjectTypeStr().c_str());
        if (cl != nullptr)
            retVal = cl->getMethodProperty(ppN.c_str(), pState);
// if related class not found, then methods won't be listed anyways
//        else
//        {
//            std::string err = std::string("error in getMethodProperty for object ") + std::to_string(_objectHandle) + ": associated class was not found.";
//            App::logScriptMsg(nullptr, sim_verbosity_scripterrors, err.c_str());
//        }
    }
    return retVal;
}

int CustomObject::setMethodProperty(const char* pName, const std::string& pState)
{
    int retVal = sim_propertyret_unknownproperty;
    if (retVal == sim_propertyret_unknownproperty)
    {
        if (isClass())
        {
            bool changed = false;
            retVal = _customProperties.setMethodProperty(pName, pState, changed);
            //if (changed)
            //    _triggerEvent(pName);
        }
    }
    return retVal;
}

int CustomObject::getMethodProperty(const char* pName, std::string& pState) const
{
    int retVal = sim_propertyret_unknownproperty;
    if (retVal == sim_propertyret_unknownproperty)
        retVal = _customProperties.getMethodProperty(pName, pState);
    if ((!isClass()) && (retVal == sim_propertyret_unknownproperty))
    {
        CustomObject* cl = App::scenes->customObjects->getClass(getObjectTypeStr().c_str());
        if (cl != nullptr)
            retVal = cl->getMethodProperty(pName, pState);
// if related class not found, then methods won't be listed anyways
//        else
//        {
//            std::string err = std::string("error in getMethodProperty for object ") + std::to_string(_objectHandle) + ": associated class was not found.";
//            App::logScriptMsg(nullptr, sim_verbosity_scripterrors, err.c_str());
//        }
    }
    return retVal;
}

int CustomObject::removeProperty(const char* pName)
{
    int retVal = Obj::removeProperty(pName);
    if (retVal == sim_propertyret_unknownproperty)
    {
        if (isClass() || _objectCanAddRemoveProperty)
        {
            retVal = _customProperties.removeProperty(pName);
            if (retVal == sim_propertyret_ok)
                _triggerEvent(pName);
        }
    }
    return retVal;
}

int CustomObject::getPropertyName(int& index, std::string& pName, std::string& appartenance, int excludeFlags) const
{
    int retVal = sim_propertyret_unknownproperty;
    if ((excludeFlags & sim_propertyinfo_retmethodsonly) == 0)
        retVal = Obj::getPropertyName(index, pName, appartenance, excludeFlags);
    if (retVal == sim_propertyret_unknownproperty)
    {
        retVal = _customProperties.getPropertyName(index, pName, appartenance, excludeFlags);
        if ((!isClass()) && (retVal == sim_propertyret_unknownproperty))
        {
            CustomObject* cl = App::scenes->customObjects->getClass(getObjectTypeStr().c_str());
            if (cl != nullptr)
                retVal = cl->getPropertyName(index, pName, appartenance, excludeFlags | sim_propertyinfo_retmethodsonly);
        }
        if (retVal == sim_propertyret_ok)
        {
            if (isClass())
            {
                if ((pName == "customClass") || (pName == "name") || (pName == "target"))
                    appartenance = getObjectTypeStr();
                else
                {
                    std::string theName;
                    getStringProperty("name", theName);
                    appartenance = theName;
                }
            }
            else
                appartenance = getObjectTypeStr();
        }
    }
    if (retVal == sim_propertyret_unknownproperty)
    {
        const std::vector<SProperty>* prop;
        if (isClass())
            prop = &allProps_customObjectClass;
        else
            prop = &allProps_customObject;
        for (size_t i = 0; i < prop->size(); i++)
        {
            if ((pName.size() == 0) || utils::startsWith(prop->at(i).name, pName.c_str()))
            {
                if ((prop->at(i).flags & excludeFlags) == 0)
                {
                    index--;
                    if (index == -1)
                    {
                        if (isClass())
                            appartenance = "TODO_CUSTOMOBJECTCLASS_APPARTENANCE";
                        else
                            appartenance = "TODO_CUSTOMOBJECT_APPARTENANCE";
                        pName = prop->at(i).name;
                        retVal = sim_propertyret_ok;
                        break;
                    }
                }
            }
        }
    }
    return retVal;
}

int CustomObject::getPropertyInfo(const char* pName, int& info, std::string& infoTxt) const
{
    int retVal = Obj::getPropertyInfo(pName, info, infoTxt);
    if (retVal == sim_propertyret_unknownproperty)
    {
        retVal = _customProperties.getPropertyInfo(pName, info, infoTxt);
        if ((!isClass()) && (retVal == sim_propertyret_unknownproperty))
        {
            CustomObject* cl = App::scenes->customObjects->getClass(getObjectTypeStr().c_str());
            if (cl != nullptr)
                retVal = cl->getPropertyInfo(pName, info, infoTxt);
        }
    }
    if (retVal == sim_propertyret_unknownproperty)
    {
        const std::vector<SProperty>* prop;
        if (isClass())
            prop = &allProps_customObjectClass;
        else
            prop = &allProps_customObject;
        for (size_t i = 0; i < prop->size(); i++)
        {
            if (strcmp(prop->at(i).name, pName) == 0)
            {
                retVal = prop->at(i).type;
                info = prop->at(i).flags;
                if (infoTxt == "j")
                    infoTxt = prop->at(i).shortInfoTxt;
                else
                {
                    auto w = QJsonDocument::fromJson(prop->at(i).shortInfoTxt.c_str()).object();
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
    }
    return retVal;
}

int CustomObject::setPropertyInfo(const char* pName, int info, const char* infoTxt)
{
    int dInfo;
    std::string dInfoTxt;
    int retVal = Obj::getPropertyInfo(pName, dInfo, dInfoTxt);
    if ((retVal == sim_propertyret_unknownproperty) && (isClass() || _objectCanAddRemoveProperty))
        retVal = _customProperties.setPropertyInfo(pName, info, infoTxt);
    else
        retVal = sim_propertyret_unavailable;
    return retVal;
}
