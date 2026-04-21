#include <customObject.h>
#include <utils.h>
#include <app.h>

CustomObject::CustomObject(long long int handle, const char* objectTypeStr, const char* objectMetaInfo, int originScriptHandle, int target)
{
    _objectHandle = handle;
    _objectTypeStr = objectTypeStr;
    _objectMetaInfo = objectMetaInfo;
    _scriptHandle = originScriptHandle;
    _target = target;
    _volatile = true;
    _isClass = (strlen(objectMetaInfo) > 0);
}

CustomObject::~CustomObject()
{
    if (!_isClass)
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
    CustomObject* object = new CustomObject(handle, _objectTypeStr.c_str(), _objectMetaInfo.c_str(), originScriptHandle, _target);
    object->_customProperties.copyFromExceptMethods(&_customProperties);
    object->_isClass = false;
    return object;
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

void CustomObject::_triggerEvent(const char* pName, CCbor* evv /*= nullptr*/) const
{
    if ((!_isClass) && (App::scenes != nullptr) && App::scenes->getEventsEnabled())
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
                    if (getBoolProperty(pName, v, true) == sim_propertyret_ok)
                        ev->appendKeyBool(pName, v);
                }
                else if (t == sim_propertytype_int)
                {
                    int v;
                    if (getIntProperty(pName, v, true) == sim_propertyret_ok)
                        ev->appendKeyInt64(pName, v);
                }
                else if (t == sim_propertytype_long)
                {
                    long long int v;
                    if (getLongProperty(pName, v, true) == sim_propertyret_ok)
                        ev->appendKeyInt64(pName, v);
                }
                else if (t == sim_propertytype_float)
                {
                    double v;
                    if (getFloatProperty(pName, v, true) == sim_propertyret_ok)
                        ev->appendKeyDouble(pName, v);
                }
                else if (t == sim_propertytype_handle)
                {
                    long long int v;
                    if (getHandleProperty(pName, v, true) == sim_propertyret_ok)
                        ev->appendKeyHandle(pName, v);
                }
                else if (t == sim_propertytype_string)
                {
                    std::string v;
                    if (getStringProperty(pName, v, true) == sim_propertyret_ok)
                        ev->appendKeyText(pName, v.c_str());
                }
                else if (t == sim_propertytype_buffer)
                {
                    std::string v;
                    if (getBufferProperty(pName, v, true) == sim_propertyret_ok)
                        ev->appendKeyBuff(pName, (unsigned char*)v.data(), v.size());
                }
                else if (t == sim_propertytype_intarray2)
                {
                    int v[2];
                    if (getIntArray2Property(pName, v, true) == sim_propertyret_ok)
                        ev->appendKeyInt32Array(pName, v, 2);
                }
                else if (t == sim_propertytype_vector3)
                {
                    C3Vector v;
                    if (getVector3Property(pName, v, true) == sim_propertyret_ok)
                        ev->appendKeyVector3(pName, v);
                }
                else if (t == sim_propertytype_quaternion)
                {
                    C4Vector v;
                    if (getQuaternionProperty(pName, v, true) == sim_propertyret_ok)
                        ev->appendKeyQuaternion(pName, v);
                }
                else if (t == sim_propertytype_pose)
                {
                    C7Vector v;
                    if (getPoseProperty(pName, v, true) == sim_propertyret_ok)
                        ev->appendKeyPose(pName, v);
                }
                else if (t == sim_propertytype_matrix)
                {
                    CMatrix v;
                    if (getMatrixProperty(pName, v, true) == sim_propertyret_ok)
                        ev->appendKeyMatrix(pName, v.data.data(), v.rows, v.cols);
                }
                else if (t == sim_propertytype_color)
                {
                    float v[3];
                    if (getColorProperty(pName, v, true) == sim_propertyret_ok)
                        ev->appendKeyFloatArray(pName, v, 3);
                }
                else if (t == sim_propertytype_floatarray)
                {
                    std::vector<double> v;
                    if (getFloatArrayProperty(pName, v, true) == sim_propertyret_ok)
                        ev->appendKeyDoubleArray(pName, v.data(), v.size());
                }
                else if (t == sim_propertytype_intarray)
                {
                    std::vector<int> v;
                    if (getIntArrayProperty(pName, v, true) == sim_propertyret_ok)
                        ev->appendKeyInt32Array(pName, v.data(), v.size());
                }
                else if (t == sim_propertytype_handlearray)
                {
                    std::vector<long long int> v;
                    if (getHandleArrayProperty(pName, v, true) == sim_propertyret_ok)
                        ev->appendKeyHandleArray(pName, v.data(), v.size());
                }
                else if (t == sim_propertytype_stringarray)
                {
                    std::vector<std::string> v;
                    if (getStringArrayProperty(pName, v, true) == sim_propertyret_ok)
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
}

void CustomObject::pushObjectCreationEvent() const
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
            ar << _objectTypeStr;
            ar << int(names.size());
            for (size_t i = 0; i < names.size(); i++)
            {
                ar << names[i];
                ar << data[i];
            }
            ar.flush();

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
                    if (theName.compare("obj") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _objectTypeStr;
                        int cnt;
                        ar >> cnt;
                        std::vector<std::string> names;
                        std::vector<std::string> data;
                        std::string s;
                        for (int i = 0; i < cnt; i++)
                        {
                            ar >> s;
                            names.push_back(s);
                            ar >> s;
                            data.push_back(s);
                        }
                        _customProperties.setAllPropertyData(names, data);
                    }
                    if (noHit)
                        ar.loadUnknownData();
                }
            }
        }
    }
    else
    {
        /*
        if (ar.isStoring())
        {
            size_t totSize = 0;
            for (size_t i = 0; i < _data.size(); i++)
                totSize += _data[i].data.size();
            if (ar.xmlSaveDataInline(totSize))
            {
                for (size_t i = 0; i < _data.size(); i++)
                {
                    ar.xmlPushNewNode("data");
                    ar.xmlAddNode_string("tag", _data[i].tag.c_str());
                    std::string str(base64_encode((unsigned char*)_data[i].data.c_str(), _data[i].data.size()));
                    ar.xmlAddNode_string("data_base64Coded", str.c_str());
                    ar.xmlPopNode();
                }
            }
            else
            {
                CSer* serObj = nullptr;
                if (objectName != nullptr)
                    serObj = ar.xmlAddNode_binFile("file", (std::string("objectCustomData_") + objectName).c_str());
                else
                    serObj = ar.xmlAddNode_binFile("file", "sceneCustomData");
                serObj[0] << int(_data.size());
                for (size_t i = 0; i < _data.size(); i++)
                {
                    serObj[0] << _data[i].tag;
                    serObj[0] << int(_data[i].data.size());
                    for (size_t j = 0; j < _data[i].data.size(); j++)
                        serObj[0] << _data[i].data[j];
                }
                serObj->flush();
                serObj->writeClose();
                delete serObj;
            }
        }
        else
        {
            CSer* serObj = ar.xmlGetNode_binFile("file", false);
            if (serObj == nullptr)
            {
                if (ar.xmlPushChildNode("data", false))
                {
                    while (true)
                    {
                        SCustomData dat;
                        ar.xmlGetNode_string("tag", dat.tag);
                        std::string data;
                        ar.xmlGetNode_string("data_base64Coded", data);
                        data = base64_decode(data);
                        dat.data = data;
                        _data.push_back(dat);
                        if (!ar.xmlPushSiblingNode("data", false))
                            break;
                    }
                    ar.xmlPopNode();
                }
            }
            else
            {
                int s;
                serObj[0] >> s;
                for (size_t i = 0; i < size_t(s); i++)
                {
                    SCustomData dat;
                    serObj[0] >> dat.tag;
                    int l;
                    serObj[0] >> l;
                    ar >> l;
                    dat.data.resize(size_t(l));
                    for (size_t j = 0; j < size_t(l); j++)
                        ar >> dat.data[j];
                    _data.push_back(dat);
                }
                serObj->readClose();
                delete serObj;
            }
        }
*/
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
            err = std::string("error in property setter for object ") + std::to_string(_objectHandle) + ": " + err;
            App::logScriptMsg(nullptr, sim_verbosity_scripterrors, err.c_str());
        }
    }
}


int CustomObject::setBoolProperty(const char* pName, bool pState)
{
    int retVal = Obj::setBoolProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        _callPropertySetterGetter(pName, SET_SUFFIX, pState, [](CInterfaceStack* s, const bool& v) { s->pushBoolOntoStack(v); }, [](CInterfaceStack* s, bool& v) { return s->getStackBoolValue(v); });
        bool changed = false;
        retVal = _customProperties.setBoolProperty(pName, pState, changed);
        if (changed)
            _triggerEvent(pName);
    }
    return retVal;
}

int CustomObject::getBoolProperty(const char* pName, bool& pState, bool direct /*= false*/) const
{
    int retVal = Obj::getBoolProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        retVal = _customProperties.getBoolProperty(pName, pState);
        if ((retVal == sim_propertyret_ok) && (!direct))
            _callPropertySetterGetter(pName, GET_SUFFIX, pState, [](CInterfaceStack* s, const bool& v) { s->pushBoolOntoStack(v); }, [](CInterfaceStack* s, bool& v) { return s->getStackBoolValue(v); });
    }
    return retVal;
}

int CustomObject::setIntProperty(const char* pName, int pState)
{
    int retVal = Obj::setIntProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        _callPropertySetterGetter(pName, SET_SUFFIX, pState, [](CInterfaceStack* s, const int& v) { s->pushInt32OntoStack(v); }, [](CInterfaceStack* s, int& v) { return s->getStackInt32Value(v); });
        bool changed = false;
        retVal = _customProperties.setIntProperty(pName, pState, changed);
        if (changed)
            _triggerEvent(pName);
    }
    return retVal;
}

int CustomObject::getIntProperty(const char* pName, int& pState, bool direct /*= false*/) const
{
    int retVal = Obj::getIntProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        retVal = _customProperties.getIntProperty(pName, pState);
        if ((retVal == sim_propertyret_ok) && (!direct))
            _callPropertySetterGetter(pName, GET_SUFFIX, pState, [](CInterfaceStack* s, const int& v) { s->pushInt32OntoStack(v); }, [](CInterfaceStack* s, int& v) { return s->getStackInt32Value(v); });
    }
    return retVal;
}

int CustomObject::setLongProperty(const char* pName, long long int pState)
{
    int retVal = Obj::setLongProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        _callPropertySetterGetter(pName, SET_SUFFIX, pState, [](CInterfaceStack* s, const long long int& v) { s->pushInt64OntoStack(v); }, [](CInterfaceStack* s, long long int& v) { return s->getStackInt64Value(v); });
        bool changed = false;
        retVal = _customProperties.setLongProperty(pName, pState, changed);
        if (changed)
            _triggerEvent(pName);
    }
    return retVal;
}

int CustomObject::getLongProperty(const char* pName, long long int& pState, bool direct /*= false*/) const
{
    int retVal = Obj::getLongProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        retVal = _customProperties.getLongProperty(pName, pState);
        if ((retVal == sim_propertyret_ok) && (!direct))
            _callPropertySetterGetter(pName, GET_SUFFIX, pState, [](CInterfaceStack* s, const long long& v) { s->pushInt64OntoStack(v); }, [](CInterfaceStack* s, long long int& v) { return s->getStackInt64Value(v); });
    }
    return retVal;
}

int CustomObject::setFloatProperty(const char* pName, double pState)
{
    int retVal = Obj::setFloatProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        _callPropertySetterGetter(pName, SET_SUFFIX, pState, [](CInterfaceStack* s, const double& v) { s->pushDoubleOntoStack(v); }, [](CInterfaceStack* s, double& v) { return s->getStackDoubleValue(v); });
        bool changed = false;
        retVal = _customProperties.setFloatProperty(pName, pState, changed);
        if (changed)
            _triggerEvent(pName);
    }
    return retVal;
}

int CustomObject::getFloatProperty(const char* pName, double& pState, bool direct /*= false*/) const
{
    int retVal = Obj::getFloatProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        retVal = _customProperties.getFloatProperty(pName, pState);
        if ((retVal == sim_propertyret_ok) && (!direct))
            _callPropertySetterGetter(pName, GET_SUFFIX, pState, [](CInterfaceStack* s, const double& v) { s->pushDoubleOntoStack(v); }, [](CInterfaceStack* s, double& v) { return s->getStackDoubleValue(v); });
    }
    return retVal;
}

int CustomObject::setHandleProperty(const char* pName, long long int pState)
{
    int retVal = Obj::setHandleProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        _callPropertySetterGetter(pName, SET_SUFFIX, pState, [](CInterfaceStack* s, const long long int& v) { s->pushHandleOntoStack(v); }, [](CInterfaceStack* s, long long int& v) { return s->getStackHandleValue(v); });
        bool changed = false;
        retVal = _customProperties.setHandleProperty(pName, pState, changed);
        if (changed)
            _triggerEvent(pName);
    }
    return retVal;
}

int CustomObject::getHandleProperty(const char* pName, long long int& pState, bool direct /*= false*/) const
{
    int retVal = Obj::getHandleProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        retVal = _customProperties.getHandleProperty(pName, pState);
        if ((retVal == sim_propertyret_ok) && (!direct))
            _callPropertySetterGetter(pName, GET_SUFFIX, pState, [](CInterfaceStack* s, const long long int& v) { s->pushHandleOntoStack(v); }, [](CInterfaceStack* s, long long int& v) { return s->getStackHandleValue(v); });
    }
    return retVal;
}

int CustomObject::setStringProperty(const char* pName, const std::string& pState)
{
    int retVal = Obj::setStringProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        std::string pp(pState);
        _callPropertySetterGetter(pName, SET_SUFFIX, pp, [](CInterfaceStack* s, const std::string& v) { s->pushTextOntoStack(v.c_str()); }, [](CInterfaceStack* s, std::string& v) { return s->getStackStringValue(v); });
        bool changed = false;
        retVal = _customProperties.setStringProperty(pName, pp, changed);
        if (changed)
            _triggerEvent(pName);
    }
    return retVal;
}

int CustomObject::getStringProperty(const char* pName, std::string& pState, bool direct /*= false*/) const
{
    int retVal = Obj::getStringProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        retVal = _customProperties.getStringProperty(pName, pState);
        if ((retVal == sim_propertyret_ok) && (!direct))
            _callPropertySetterGetter(pName, GET_SUFFIX, pState, [](CInterfaceStack* s, const std::string& v) { s->pushTextOntoStack(v.c_str()); }, [](CInterfaceStack* s, std::string& v) { return s->getStackStringValue(v); });
    }
    return retVal;
}

int CustomObject::setBufferProperty(const char* pName, const std::string& pState)
{
    int retVal = Obj::setBufferProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        std::string pp(pState);
        _callPropertySetterGetter(pName, SET_SUFFIX, pp, [](CInterfaceStack* s, const std::string& v) { s->pushBufferOntoStack(v.data(), v.size()); }, [](CInterfaceStack* s, std::string& v) { return s->getStackStringValue(v); });
        bool changed = false;
        retVal = _customProperties.setBufferProperty(pName, pp, changed);
        if (changed)
            _triggerEvent(pName);
    }
    return retVal;
}

int CustomObject::getBufferProperty(const char* pName, std::string& pState, bool direct /*= false*/) const
{
    int retVal = Obj::getBufferProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        retVal = _customProperties.getBufferProperty(pName, pState);
        if ((retVal == sim_propertyret_ok) && (!direct))
            _callPropertySetterGetter(pName, GET_SUFFIX, pState, [](CInterfaceStack* s, const std::string& v) { s->pushBufferOntoStack(v.data(), v.size()); }, [](CInterfaceStack* s, std::string& v) { return s->getStackStringValue(v); });
    }
    return retVal;
}

int CustomObject::setIntArray2Property(const char* pName, const int* pState)
{
    int retVal = Obj::setIntArray2Property(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        int pp[2] = {pState[0], pState[1]};
        _callPropertySetterGetter(pName, SET_SUFFIX, pp, [](CInterfaceStack* s, const int (&v)[2]) { s->pushInt32ArrayOntoStack(v, 2); }, [](CInterfaceStack* s, int (&v)[2]) { return s->getStackInt32Array(v, 2); });
        bool changed = false;
        retVal = _customProperties.setIntArray2Property(pName, pp, changed);
        if (changed)
            _triggerEvent(pName);
    }
    return retVal;
}

int CustomObject::getIntArray2Property(const char* pName, int* pState, bool direct /*= false*/) const
{
    int retVal = Obj::getIntArray2Property(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        retVal = _customProperties.getIntArray2Property(pName, pState);
        if ((retVal == sim_propertyret_ok) && (!direct))
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
        CMatrix m(3, 1);
        m.data.assign(pState.data, pState.data + 3);
        _callPropertySetterGetter(pName, SET_SUFFIX, m, [](CInterfaceStack* s, const CMatrix& v) { s->pushMatrixOntoStack(v); }, [](CInterfaceStack* s, CMatrix& v) { return s->getStackMatrix(v); });
        C3Vector p(m.data.data());
        bool changed = false;
        retVal = _customProperties.setVector3Property(pName, p, changed);
        if (changed)
            _triggerEvent(pName);
    }
    return retVal;
}

int CustomObject::getVector3Property(const char* pName, C3Vector& pState, bool direct /*= false*/) const
{
    int retVal = Obj::getVector3Property(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        retVal = _customProperties.getVector3Property(pName, pState);
        if ((retVal == sim_propertyret_ok) && (!direct))
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
        CMatrix pp(pState);
        _callPropertySetterGetter(pName, SET_SUFFIX, pp, [](CInterfaceStack* s, const CMatrix& v) { s->pushMatrixOntoStack(v); }, [](CInterfaceStack* s, CMatrix& v) { return s->getStackMatrix(v); });
        bool changed = false;
        retVal = _customProperties.setMatrixProperty(pName, pp, changed);
        if (changed)
            _triggerEvent(pName);
    }
    return retVal;
}

int CustomObject::getMatrixProperty(const char* pName, CMatrix& pState, bool direct /*= false*/) const
{
    int retVal = Obj::getMatrixProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        retVal = _customProperties.getMatrixProperty(pName, pState);
        if ((retVal == sim_propertyret_ok) && (!direct))
            _callPropertySetterGetter(pName, GET_SUFFIX, pState, [](CInterfaceStack* s, const CMatrix& v) { s->pushMatrixOntoStack(v); }, [](CInterfaceStack* s, CMatrix& v) { return s->getStackMatrix(v); });
    }
    return retVal;
}

int CustomObject::setQuaternionProperty(const char* pName, const C4Vector& pState)
{
    int retVal = Obj::setQuaternionProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        C4Vector pp(pState);
        _callPropertySetterGetter(pName, SET_SUFFIX, pp, [](CInterfaceStack* s, const C4Vector& v) { s->pushQuaternionOntoStack(v); }, [](CInterfaceStack* s, C4Vector& v) { return s->getStackQuaternion(v); });
        bool changed = false;
        retVal = _customProperties.setQuaternionProperty(pName, pp, changed);
        if (changed)
            _triggerEvent(pName);
    }
    return retVal;
}

int CustomObject::getQuaternionProperty(const char* pName, C4Vector& pState, bool direct /*= false*/) const
{
    int retVal = Obj::getQuaternionProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        retVal = _customProperties.getQuaternionProperty(pName, pState);
        if ((retVal == sim_propertyret_ok) && (!direct))
            _callPropertySetterGetter(pName, GET_SUFFIX, pState, [](CInterfaceStack* s, const C4Vector& v) { s->pushQuaternionOntoStack(v); }, [](CInterfaceStack* s, C4Vector& v) { return s->getStackQuaternion(v); });
    }
    return retVal;
}

int CustomObject::setPoseProperty(const char* pName, const C7Vector& pState)
{
    int retVal = Obj::setPoseProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        C7Vector pp(pState);
        _callPropertySetterGetter(pName, SET_SUFFIX, pp, [](CInterfaceStack* s, const C7Vector& v) { s->pushPoseOntoStack(v); }, [](CInterfaceStack* s, C7Vector& v) { return s->getStackPose(v); });
        bool changed = false;
        retVal = _customProperties.setPoseProperty(pName, pp, changed);
        if (changed)
            _triggerEvent(pName);
    }
    return retVal;
}

int CustomObject::getPoseProperty(const char* pName, C7Vector& pState, bool direct /*= false*/) const
{
    int retVal = Obj::getPoseProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        retVal = _customProperties.getPoseProperty(pName, pState);
        if ((retVal == sim_propertyret_ok) && (!direct))
            _callPropertySetterGetter(pName, GET_SUFFIX, pState, [](CInterfaceStack* s, const C7Vector& v) { s->pushPoseOntoStack(v); }, [](CInterfaceStack* s, C7Vector& v) { return s->getStackPose(v); });
    }
    return retVal;
}

int CustomObject::setColorProperty(const char* pName, const float* pState)
{
    int retVal = Obj::setColorProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        float pp[3] = {pState[0], pState[1], pState[2]};
        _callPropertySetterGetter(pName, SET_SUFFIX, pp, [](CInterfaceStack* s, const float (&v)[3]) { s->pushColorOntoStack(v); }, [](CInterfaceStack* s, float (&v)[3]) { return s->getStackColor(v); });
        bool changed = false;
        retVal = _customProperties.setColorProperty(pName, pp, changed);
        if (changed)
            _triggerEvent(pName);
    }
    return retVal;
}

int CustomObject::getColorProperty(const char* pName, float* pState, bool direct /*= false*/) const
{
    int retVal = Obj::getColorProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        retVal = _customProperties.getColorProperty(pName, pState);
        if ((retVal == sim_propertyret_ok) && (!direct))
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
        std::vector<double> pp(pState);
        _callPropertySetterGetter(pName, SET_SUFFIX, pp, [](CInterfaceStack* s, const std::vector<double>& w) { s->pushDoubleArrayOntoStack(w.data(), w.size()); }, [](CInterfaceStack* s, std::vector<double>& w) { return s->getStackDoubleArray(w.data(), w.size()); });
        bool changed = false;
        retVal = _customProperties.setFloatArrayProperty(pName, pp, changed);
        if (changed)
            _triggerEvent(pName);
    }
    return retVal;
}

int CustomObject::getFloatArrayProperty(const char* pName, std::vector<double>& pState, bool direct /*= false*/) const
{
    pState.clear();
    int retVal = Obj::getFloatArrayProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        retVal = _customProperties.getFloatArrayProperty(pName, pState);
        if ((retVal == sim_propertyret_ok) && (!direct))
            _callPropertySetterGetter(pName, GET_SUFFIX, pState, [](CInterfaceStack* s, const std::vector<double>& w) { s->pushDoubleArrayOntoStack(w.data(), w.size()); }, [](CInterfaceStack* s, std::vector<double>& w) { return s->getStackDoubleArray(w.data(), w.size()); });
    }
    return retVal;
}

int CustomObject::setIntArrayProperty(const char* pName, const std::vector<int>& pState)
{
    int retVal = Obj::setIntArrayProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        std::vector<int> pp(pState);
        _callPropertySetterGetter(pName, SET_SUFFIX, pp, [](CInterfaceStack* s, const std::vector<int>& w) { s->pushInt32ArrayOntoStack(w.data(), w.size()); }, [](CInterfaceStack* s, std::vector<int>& w) { return s->getStackInt32Array(w.data(), w.size()); });
        bool changed = false;
        retVal = _customProperties.setIntArrayProperty(pName, pp, changed);
        if (changed)
            _triggerEvent(pName);
    }
    return retVal;
}

int CustomObject::getIntArrayProperty(const char* pName, std::vector<int>& pState, bool direct /*= false*/) const
{
    pState.clear();
    int retVal = Obj::getIntArrayProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        retVal = _customProperties.getIntArrayProperty(pName, pState);
        if ((retVal == sim_propertyret_ok) && (!direct))
            _callPropertySetterGetter(pName, GET_SUFFIX, pState, [](CInterfaceStack* s, const std::vector<int>& w) { s->pushInt32ArrayOntoStack(w.data(), w.size()); }, [](CInterfaceStack* s, std::vector<int>& w) { return s->getStackInt32Array(w.data(), w.size()); });
    }
    return retVal;
}

int CustomObject::setHandleArrayProperty(const char* pName, const std::vector<long long int>& pState)
{
    int retVal = Obj::setHandleArrayProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        std::vector<long long int> pp(pState);
        _callPropertySetterGetter(pName, SET_SUFFIX, pp, [](CInterfaceStack* s, const std::vector<long long int>& w) { s->pushInt64ArrayOntoStack(w.data(), w.size()); }, [](CInterfaceStack* s, std::vector<long long int>& w) { return s->getStackInt64Array(w.data(), w.size()); });
        bool changed = false;
        retVal = _customProperties.setHandleArrayProperty(pName, pp, changed);
        if (changed)
            _triggerEvent(pName);
    }
    return retVal;
}

int CustomObject::getHandleArrayProperty(const char* pName, std::vector<long long int>& pState, bool direct /*= false*/) const
{
    pState.clear();
    int retVal = Obj::getHandleArrayProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        retVal = _customProperties.getHandleArrayProperty(pName, pState);
        if ((retVal == sim_propertyret_ok) && (!direct))
            _callPropertySetterGetter(pName, GET_SUFFIX, pState, [](CInterfaceStack* s, const std::vector<long long int>& w) { s->pushInt64ArrayOntoStack(w.data(), w.size()); }, [](CInterfaceStack* s, std::vector<long long int>& w) { return s->getStackInt64Array(w.data(), w.size()); });
    }
    return retVal;
}

int CustomObject::setStringArrayProperty(const char* pName, const std::vector<std::string>& pState)
{
    int retVal = Obj::setStringArrayProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        std::vector<std::string> pp(pState);
        _callPropertySetterGetter(pName, SET_SUFFIX, pp, [](CInterfaceStack* s, const std::vector<std::string>& w) { s->pushTextArrayOntoStack(w.data(), w.size()); }, [](CInterfaceStack* s, std::vector<std::string>& w) { w.clear(); return s->getStackTextArray(w); });
        bool changed = false;
        retVal = _customProperties.setStringArrayProperty(pName, pp, changed);
        if (changed)
            _triggerEvent(pName);
    }
    return retVal;
}

int CustomObject::getStringArrayProperty(const char* pName, std::vector<std::string>& pState, bool direct /*= false*/) const
{
    pState.clear();
    int retVal = Obj::getStringArrayProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        retVal = _customProperties.getStringArrayProperty(pName, pState);
        if ((retVal == sim_propertyret_ok) && (!direct))
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
        bool changed = false;
        retVal = _customProperties.setMethodProperty(ppN.c_str(), pState, changed);
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
    if ((!_isClass) && (retVal == sim_propertyret_unknownproperty))
    {
        CustomObject* cl = nullptr;
        if (_target == sim_handle_app)
            cl = App::scenes->customObjects->getClass(getObjectTypeStr().c_str());
        else if (_target == sim_handle_scene)
            cl = App::scene->customObjects->getClass(getObjectTypeStr().c_str());
        if (cl != nullptr)
            retVal = cl->getMethodProperty(ppN.c_str(), pState);
    }
    return retVal;
}

int CustomObject::setMethodProperty(const char* pName, const std::string& pState)
{
    int retVal = sim_propertyret_unknownproperty;
    if (retVal == sim_propertyret_unknownproperty)
    {
        bool changed = false;
        retVal = _customProperties.setMethodProperty(pName, pState, changed);
        //if (changed)
        //    _triggerEvent(pName);
    }
    return retVal;
}

int CustomObject::getMethodProperty(const char* pName, std::string& pState) const
{
    int retVal = sim_propertyret_unknownproperty;
    if (retVal == sim_propertyret_unknownproperty)
        retVal = _customProperties.getMethodProperty(pName, pState);
    if ((!_isClass) && (retVal == sim_propertyret_unknownproperty))
    {
        CustomObject* cl = nullptr;
        if (_target == sim_handle_app)
            cl = App::scenes->customObjects->getClass(getObjectTypeStr().c_str());
        else if (_target == sim_handle_scene)
            cl = App::scene->customObjects->getClass(getObjectTypeStr().c_str());
        if (cl != nullptr)
            retVal = cl->getMethodProperty(pName, pState);
    }
    return retVal;
}

int CustomObject::removeProperty(const char* pName)
{
    int retVal = Obj::removeProperty(pName);
    if (retVal == sim_propertyret_unknownproperty)
    {
        retVal = _customProperties.removeProperty(pName);
        if (retVal == sim_propertyret_ok)
            _triggerEvent(pName);
    }
    return retVal;
}

int CustomObject::getPropertyName(int& index, std::string& pName, std::string& appartenance, int excludeFlags, bool methodsOnly /*= false*/) const
{
    int retVal = sim_propertyret_unknownproperty;
    if (!methodsOnly)
        retVal = Obj::getPropertyName(index, pName, appartenance, excludeFlags);
    if (retVal == sim_propertyret_unknownproperty)
    {
        retVal = _customProperties.getPropertyName(index, pName, appartenance, excludeFlags, methodsOnly);
        if ((!_isClass) && (retVal == sim_propertyret_unknownproperty))
        {
            CustomObject* cl = nullptr;
            if (_target == sim_handle_app)
                cl = App::scenes->customObjects->getClass(getObjectTypeStr().c_str());
            else if (_target == sim_handle_scene)
                cl = App::scene->customObjects->getClass(getObjectTypeStr().c_str());
            if (cl != nullptr)
                retVal = cl->getPropertyName(index, pName, appartenance, excludeFlags, true);
        }
        if (retVal == sim_propertyret_ok)
            appartenance = getObjectTypeStr();
    }
    return retVal;
}

int CustomObject::getPropertyInfo(const char* pName, int& info, std::string& infoTxt) const
{
    int retVal = Obj::getPropertyInfo(pName, info, infoTxt);
    if (retVal == sim_propertyret_unknownproperty)
    {
        retVal = _customProperties.getPropertyInfo(pName, info, infoTxt);
        if ((!_isClass) && (retVal == sim_propertyret_unknownproperty))
        {
            CustomObject* cl = nullptr;
            if (_target == sim_handle_app)
                cl = App::scenes->customObjects->getClass(getObjectTypeStr().c_str());
            else if (_target == sim_handle_scene)
                cl = App::scene->customObjects->getClass(getObjectTypeStr().c_str());
            if (cl != nullptr)
                retVal = cl->getPropertyInfo(pName, info, infoTxt);
        }
    }
    return retVal;
}

int CustomObject::setPropertyInfo(const char* pName, int info, const char* infoTxt)
{
    int dInfo;
    std::string dInfoTxt;
    int retVal = Obj::getPropertyInfo(pName, dInfo, dInfoTxt);
    if (retVal == sim_propertyret_unknownproperty)
        retVal = _customProperties.setPropertyInfo(pName, info, infoTxt);
    else
        retVal = sim_propertyret_unavailable;
    return retVal;
}
