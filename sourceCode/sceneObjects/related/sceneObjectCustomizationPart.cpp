#include <sceneObjectCustomizationPart.h>
#include <utils.h>
#include <sceneObject.h>
#include <app.h>

CSceneObjectCustomizationPart::CSceneObjectCustomizationPart(CSceneObject* sceneObj)
{
    _sceneObject = sceneObj;
    _changed = false;
    _ignoreSetterGetter = false;
    _objectCanAddRemoveProperty = false;
}

CSceneObjectCustomizationPart::~CSceneObjectCustomizationPart()
{
}

bool CSceneObjectCustomizationPart::getResetChanged()
{
    bool retVal = _changed;
    _changed = false;
    return retVal;
}

bool CSceneObjectCustomizationPart::isClass() const
{

    return (_sceneObject->getObjectHandle() >= sim_object_sceneobjectclassstart);
}

void CSceneObjectCustomizationPart::setIgnoreSetterGetter(bool f)
{
    _ignoreSetterGetter = f;
}

void CSceneObjectCustomizationPart::setObjectCanAddRemoveProperty(bool f)
{
    _objectCanAddRemoveProperty = f;
}

void CSceneObjectCustomizationPart::_triggerEvent(const char* pName, CCbor* evv /*= nullptr*/)
{
    _changed = true;
    _ignoreSetterGetter = true;
    if ((_sceneObject != nullptr) && (App::scenes != nullptr) && App::scenes->getEventsEnabled())
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
                    ev = App::scenes->createEvent(EVENTTYPE_OBJECTCHANGED, _sceneObject->getObjectHandle(), _sceneObject->getObjectHandle(), nullptr, false);
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
                ev = App::scenes->createEvent(EVENTTYPE_OBJECTCHANGED, _sceneObject->getObjectHandle(), _sceneObject->getObjectHandle(), nullptr, false);
            ev->appendKeyNull(pName);
            if (evv == nullptr)
                App::scenes->pushEvent();
        }
    }
    _ignoreSetterGetter = false;
}

CSceneObjectCustomizationPart* CSceneObjectCustomizationPart::copyYourself(CSceneObject* appartenance) const
{
    CSceneObjectCustomizationPart* retVal = new CSceneObjectCustomizationPart(appartenance);
    retVal->_customProperties = _customProperties;
    return retVal;
}

void CSceneObjectCustomizationPart::serialize(CSer& ar)
{
    if (ar.isBinary())
    {
        if (ar.isStoring())
        { // Storing
            std::vector<std::string> names;
            std::vector<std::string> data;
            _customProperties.getAllPropertyData(names, data);

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
void CSceneObjectCustomizationPart::_callPropertySetterGetter(const char* pName, const char* suffix, T& pState, PushF pushFunc, GetF getFunc) const
{
    int info;
    std::string infoTxt;
    if (sim_propertytype_method == getPropertyInfo((std::string(pName) + suffix).c_str(), info, infoTxt))
    {
        CInterfaceStack* inStack = App::scenes->interfaceStackContainer->createStack();
        CInterfaceStack* outStack = App::scenes->interfaceStackContainer->createStack();
        inStack->pushTextOntoStack(pName);
        pushFunc(inStack, pState);
        int ret = simCallMethod_internal(_sceneObject->getObjectHandle(), (std::string(pName) + suffix).c_str(), inStack->getObjectHandle(), outStack->getObjectHandle(), -1);
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
            err = std::string("error in property setter/getter for object ") + std::to_string(_sceneObject->getObjectHandle()) + ": " + err;
            App::logScriptMsg(nullptr, sim_verbosity_scripterrors, err.c_str());
        }
    }
}


int CSceneObjectCustomizationPart::setBoolProperty(const char* pName, bool pState, bool viaClass /*= false*/)
{
    int retVal = sim_propertyret_unknownproperty;
    if (viaClass)
        retVal = _sceneObject->Obj::setBoolProperty(pName, pState);
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

int CSceneObjectCustomizationPart::getBoolProperty(const char* pName, bool& pState, bool viaClass /*= false*/) const
{
    int retVal = sim_propertyret_unknownproperty;
    if (viaClass)
        retVal = _sceneObject->Obj::getBoolProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        retVal = _customProperties.getBoolProperty(pName, pState);
        if ((retVal == sim_propertyret_ok) && (!_ignoreSetterGetter) && (!isClass()))
            _callPropertySetterGetter(pName, GET_SUFFIX, pState, [](CInterfaceStack* s, const bool& v) { s->pushBoolOntoStack(v); }, [](CInterfaceStack* s, bool& v) { return s->getStackBoolValue(v); });
    }
    return retVal;
}

int CSceneObjectCustomizationPart::setIntProperty(const char* pName, int pState, bool viaClass /*= false*/)
{
    int retVal = sim_propertyret_unknownproperty;
    if (viaClass)
        retVal = _sceneObject->Obj::setIntProperty(pName, pState);
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

int CSceneObjectCustomizationPart::getIntProperty(const char* pName, int& pState, bool viaClass /*= false*/) const
{
    int retVal = sim_propertyret_unknownproperty;
    if (viaClass)
        retVal = _sceneObject->Obj::getIntProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        retVal = _customProperties.getIntProperty(pName, pState);
        if ((retVal == sim_propertyret_ok) && (!_ignoreSetterGetter) && (!isClass()))
            _callPropertySetterGetter(pName, GET_SUFFIX, pState, [](CInterfaceStack* s, const int& v) { s->pushInt32OntoStack(v); }, [](CInterfaceStack* s, int& v) { return s->getStackInt32Value(v); });
    }
    return retVal;
}

int CSceneObjectCustomizationPart::setLongProperty(const char* pName, long long int pState, bool viaClass /*= false*/)
{
    int retVal = sim_propertyret_unknownproperty;
    if (viaClass)
        retVal = _sceneObject->Obj::setLongProperty(pName, pState);
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

int CSceneObjectCustomizationPart::getLongProperty(const char* pName, long long int& pState, bool viaClass /*= false*/) const
{
    int retVal = sim_propertyret_unknownproperty;
    if (viaClass)
        retVal = _sceneObject->Obj::getLongProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        retVal = _customProperties.getLongProperty(pName, pState);
        if ((retVal == sim_propertyret_ok) && (!_ignoreSetterGetter) && (!isClass()))
            _callPropertySetterGetter(pName, GET_SUFFIX, pState, [](CInterfaceStack* s, const long long& v) { s->pushInt64OntoStack(v); }, [](CInterfaceStack* s, long long int& v) { return s->getStackInt64Value(v); });
    }
    return retVal;
}

int CSceneObjectCustomizationPart::setFloatProperty(const char* pName, double pState, bool viaClass /*= false*/)
{
    int retVal = sim_propertyret_unknownproperty;
    if (viaClass)
        retVal = _sceneObject->Obj::setFloatProperty(pName, pState);
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

int CSceneObjectCustomizationPart::getFloatProperty(const char* pName, double& pState, bool viaClass /*= false*/) const
{
    int retVal = sim_propertyret_unknownproperty;
    if (viaClass)
        retVal = _sceneObject->Obj::getFloatProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        retVal = _customProperties.getFloatProperty(pName, pState);
        if ((retVal == sim_propertyret_ok) && (!_ignoreSetterGetter) && (!isClass()))
            _callPropertySetterGetter(pName, GET_SUFFIX, pState, [](CInterfaceStack* s, const double& v) { s->pushDoubleOntoStack(v); }, [](CInterfaceStack* s, double& v) { return s->getStackDoubleValue(v); });
    }
    return retVal;
}

int CSceneObjectCustomizationPart::setHandleProperty(const char* pName, long long int pState, bool viaClass /*= false*/)
{
    int retVal = sim_propertyret_unknownproperty;
    if (viaClass)
        retVal = _sceneObject->Obj::setHandleProperty(pName, pState);
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

int CSceneObjectCustomizationPart::getHandleProperty(const char* pName, long long int& pState, bool viaClass /*= false*/) const
{
    int retVal = sim_propertyret_unknownproperty;
    if (viaClass)
        retVal = _sceneObject->Obj::getHandleProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        retVal = _customProperties.getHandleProperty(pName, pState);
        if ((retVal == sim_propertyret_ok) && (!_ignoreSetterGetter) && (!isClass()))
            _callPropertySetterGetter(pName, GET_SUFFIX, pState, [](CInterfaceStack* s, const long long int& v) { s->pushHandleOntoStack(v); }, [](CInterfaceStack* s, long long int& v) { return s->getStackHandleValue(v); });
    }
    return retVal;
}

int CSceneObjectCustomizationPart::setStringProperty(const char* pName, const std::string& pState, bool viaClass /*= false*/)
{
    int retVal = sim_propertyret_unknownproperty;
    if (viaClass)
        retVal = _sceneObject->Obj::setStringProperty(pName, pState);
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

int CSceneObjectCustomizationPart::getStringProperty(const char* pName, std::string& pState, bool viaClass /*= false*/) const
{
    int retVal = sim_propertyret_unknownproperty;
    if (viaClass)
        retVal = _sceneObject->Obj::getStringProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        retVal = _customProperties.getStringProperty(pName, pState);
        if ((retVal == sim_propertyret_ok) && (!_ignoreSetterGetter) && (!isClass()))
            _callPropertySetterGetter(pName, GET_SUFFIX, pState, [](CInterfaceStack* s, const std::string& v) { s->pushTextOntoStack(v.c_str()); }, [](CInterfaceStack* s, std::string& v) { return s->getStackStringValue(v); });
    }
    return retVal;
}

int CSceneObjectCustomizationPart::setTableProperty(const char* pName, const std::string& pState, bool viaClass /*= false*/)
{
    int retVal = sim_propertyret_unknownproperty;
    if (viaClass)
        retVal = _sceneObject->Obj::setTableProperty(pName, pState);
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

int CSceneObjectCustomizationPart::getTableProperty(const char* pName, std::string& pState, bool viaClass /*= false*/) const
{
    int retVal = sim_propertyret_unknownproperty;
    if (viaClass)
        retVal = _sceneObject->Obj::getTableProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        retVal = _customProperties.getTableProperty(pName, pState);
        if ((retVal == sim_propertyret_ok) && (!_ignoreSetterGetter) && (!isClass()))
            _callPropertySetterGetter(pName, GET_SUFFIX, pState, [](CInterfaceStack* s, const std::string& v) { s->pushBufferOntoStack(v.data(), v.size()); }, [](CInterfaceStack* s, std::string& v) { return s->getStackStringValue(v); });
    }
    return retVal;
}

int CSceneObjectCustomizationPart::setBufferProperty(const char* pName, const std::string& pState, bool viaClass /*= false*/)
{
    int retVal = sim_propertyret_unknownproperty;
    if (viaClass)
        retVal = _sceneObject->Obj::setBufferProperty(pName, pState);
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

int CSceneObjectCustomizationPart::getBufferProperty(const char* pName, std::string& pState, bool viaClass /*= false*/) const
{
    int retVal = sim_propertyret_unknownproperty;
    if (viaClass)
        retVal = _sceneObject->Obj::getBufferProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        retVal = _customProperties.getBufferProperty(pName, pState);
        if ((retVal == sim_propertyret_ok) && (!_ignoreSetterGetter) && (!isClass()))
            _callPropertySetterGetter(pName, GET_SUFFIX, pState, [](CInterfaceStack* s, const std::string& v) { s->pushBufferOntoStack(v.data(), v.size()); }, [](CInterfaceStack* s, std::string& v) { return s->getStackStringValue(v); });
    }
    return retVal;
}

int CSceneObjectCustomizationPart::setIntArray2Property(const char* pName, const int* pState, bool viaClass /*= false*/)
{
    int retVal = sim_propertyret_unknownproperty;
    if (viaClass)
        retVal = _sceneObject->Obj::setIntArray2Property(pName, pState);
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

int CSceneObjectCustomizationPart::getIntArray2Property(const char* pName, int* pState, bool viaClass /*= false*/) const
{
    int retVal = sim_propertyret_unknownproperty;
    if (viaClass)
        retVal = _sceneObject->Obj::getIntArray2Property(pName, pState);
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

int CSceneObjectCustomizationPart::setVector3Property(const char* pName, const C3Vector& pState, bool viaClass /*= false*/)
{
    int retVal = sim_propertyret_unknownproperty;
    if (viaClass)
        retVal = _sceneObject->Obj::setVector3Property(pName, pState);
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

int CSceneObjectCustomizationPart::getVector3Property(const char* pName, C3Vector& pState, bool viaClass /*= false*/) const
{
    int retVal = sim_propertyret_unknownproperty;
    if (viaClass)
        retVal = _sceneObject->Obj::getVector3Property(pName, pState);
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

int CSceneObjectCustomizationPart::setMatrixProperty(const char* pName, const CMatrix& pState, bool viaClass /*= false*/)
{
    int retVal = sim_propertyret_unknownproperty;
    if (viaClass)
        retVal = _sceneObject->Obj::setMatrixProperty(pName, pState);
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

int CSceneObjectCustomizationPart::getMatrixProperty(const char* pName, CMatrix& pState, bool viaClass /*= false*/) const
{
    int retVal = sim_propertyret_unknownproperty;
    if (viaClass)
        retVal = _sceneObject->Obj::getMatrixProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        retVal = _customProperties.getMatrixProperty(pName, pState);
        if ((retVal == sim_propertyret_ok) && (!_ignoreSetterGetter) && (!isClass()))
            _callPropertySetterGetter(pName, GET_SUFFIX, pState, [](CInterfaceStack* s, const CMatrix& v) { s->pushMatrixOntoStack(v); }, [](CInterfaceStack* s, CMatrix& v) { return s->getStackMatrix(v); });
    }
    return retVal;
}

int CSceneObjectCustomizationPart::setQuaternionProperty(const char* pName, const C4Vector& pState, bool viaClass /*= false*/)
{
    int retVal = sim_propertyret_unknownproperty;
    if (viaClass)
        retVal = _sceneObject->Obj::setQuaternionProperty(pName, pState);
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

int CSceneObjectCustomizationPart::getQuaternionProperty(const char* pName, C4Vector& pState, bool viaClass /*= false*/) const
{
    int retVal = sim_propertyret_unknownproperty;
    if (viaClass)
        retVal = _sceneObject->Obj::getQuaternionProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        retVal = _customProperties.getQuaternionProperty(pName, pState);
        if ((retVal == sim_propertyret_ok) && (!_ignoreSetterGetter) && (!isClass()))
            _callPropertySetterGetter(pName, GET_SUFFIX, pState, [](CInterfaceStack* s, const C4Vector& v) { s->pushQuaternionOntoStack(v); }, [](CInterfaceStack* s, C4Vector& v) { return s->getStackQuaternion(v); });
    }
    return retVal;
}

int CSceneObjectCustomizationPart::setPoseProperty(const char* pName, const C7Vector& pState, bool viaClass /*= false*/)
{
    int retVal = sim_propertyret_unknownproperty;
    if (viaClass)
        retVal = _sceneObject->Obj::setPoseProperty(pName, pState);
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

int CSceneObjectCustomizationPart::getPoseProperty(const char* pName, C7Vector& pState, bool viaClass /*= false*/) const
{
    int retVal = sim_propertyret_unknownproperty;
    if (viaClass)
        retVal = _sceneObject->Obj::getPoseProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        retVal = _customProperties.getPoseProperty(pName, pState);
        if ((retVal == sim_propertyret_ok) && (!_ignoreSetterGetter) && (!isClass()))
            _callPropertySetterGetter(pName, GET_SUFFIX, pState, [](CInterfaceStack* s, const C7Vector& v) { s->pushPoseOntoStack(v); }, [](CInterfaceStack* s, C7Vector& v) { return s->getStackPose(v); });
    }
    return retVal;
}

int CSceneObjectCustomizationPart::setColorProperty(const char* pName, const float* pState, bool viaClass /*= false*/)
{
    int retVal = sim_propertyret_unknownproperty;
    if (viaClass)
        retVal = _sceneObject->Obj::setColorProperty(pName, pState);
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

int CSceneObjectCustomizationPart::getColorProperty(const char* pName, float* pState, bool viaClass /*= false*/) const
{
    int retVal = sim_propertyret_unknownproperty;
    if (viaClass)
        retVal = _sceneObject->Obj::getColorProperty(pName, pState);
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

int CSceneObjectCustomizationPart::setFloatArrayProperty(const char* pName, const std::vector<double>& pState, bool viaClass /*= false*/)
{
    int retVal = sim_propertyret_unknownproperty;
    if (viaClass)
        retVal = _sceneObject->Obj::setFloatArrayProperty(pName, pState);
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

int CSceneObjectCustomizationPart::getFloatArrayProperty(const char* pName, std::vector<double>& pState, bool viaClass /*= false*/) const
{
    pState.clear();
    int retVal = sim_propertyret_unknownproperty;
    if (viaClass)
        retVal = _sceneObject->Obj::getFloatArrayProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        retVal = _customProperties.getFloatArrayProperty(pName, pState);
        if ((retVal == sim_propertyret_ok) && (!_ignoreSetterGetter) && (!isClass()))
            _callPropertySetterGetter(pName, GET_SUFFIX, pState, [](CInterfaceStack* s, const std::vector<double>& w) { s->pushDoubleArrayOntoStack(w.data(), w.size()); }, [](CInterfaceStack* s, std::vector<double>& w) { return s->getStackDoubleArray(w.data(), w.size()); });
    }
    return retVal;
}

int CSceneObjectCustomizationPart::setIntArrayProperty(const char* pName, const std::vector<int>& pState, bool viaClass /*= false*/)
{
    int retVal = sim_propertyret_unknownproperty;
    if (viaClass)
        retVal = _sceneObject->Obj::setIntArrayProperty(pName, pState);
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

int CSceneObjectCustomizationPart::getIntArrayProperty(const char* pName, std::vector<int>& pState, bool viaClass /*= false*/) const
{
    pState.clear();
    int retVal = sim_propertyret_unknownproperty;
    if (viaClass)
        retVal = _sceneObject->Obj::getIntArrayProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        retVal = _customProperties.getIntArrayProperty(pName, pState);
        if ((retVal == sim_propertyret_ok) && (!_ignoreSetterGetter) && (!isClass()))
            _callPropertySetterGetter(pName, GET_SUFFIX, pState, [](CInterfaceStack* s, const std::vector<int>& w) { s->pushInt32ArrayOntoStack(w.data(), w.size()); }, [](CInterfaceStack* s, std::vector<int>& w) { return s->getStackInt32Array(w.data(), w.size()); });
    }
    return retVal;
}

int CSceneObjectCustomizationPart::setHandleArrayProperty(const char* pName, const std::vector<long long int>& pState, bool viaClass /*= false*/)
{
    int retVal = sim_propertyret_unknownproperty;
    if (viaClass)
        retVal = _sceneObject->Obj::setHandleArrayProperty(pName, pState);
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

int CSceneObjectCustomizationPart::getHandleArrayProperty(const char* pName, std::vector<long long int>& pState, bool viaClass /*= false*/) const
{
    pState.clear();
    int retVal = sim_propertyret_unknownproperty;
    if (viaClass)
        retVal = _sceneObject->Obj::getHandleArrayProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        retVal = _customProperties.getHandleArrayProperty(pName, pState);
        if ((retVal == sim_propertyret_ok) && (!_ignoreSetterGetter) && (!isClass()))
            _callPropertySetterGetter(pName, GET_SUFFIX, pState, [](CInterfaceStack* s, const std::vector<long long int>& w) { s->pushInt64ArrayOntoStack(w.data(), w.size()); }, [](CInterfaceStack* s, std::vector<long long int>& w) { return s->getStackInt64Array(w.data(), w.size()); });
    }
    return retVal;
}

int CSceneObjectCustomizationPart::setStringArrayProperty(const char* pName, const std::vector<std::string>& pState, bool viaClass /*= false*/)
{
    int retVal = sim_propertyret_unknownproperty;
    if (viaClass)
        retVal = _sceneObject->Obj::setStringArrayProperty(pName, pState);
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

int CSceneObjectCustomizationPart::getStringArrayProperty(const char* pName, std::vector<std::string>& pState, bool viaClass /*= false*/) const
{
    pState.clear();
    int retVal = sim_propertyret_unknownproperty;
    if (viaClass)
        retVal = _sceneObject->Obj::getStringArrayProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        retVal = _customProperties.getStringArrayProperty(pName, pState);
        if ((retVal == sim_propertyret_ok) && (!_ignoreSetterGetter) && (!isClass()))
            _callPropertySetterGetter(pName, GET_SUFFIX, pState, [](CInterfaceStack* s, const std::vector<std::string>& w) { s->pushTextArrayOntoStack(w.data(), w.size()); }, [](CInterfaceStack* s, std::vector<std::string>& w) { w.clear(); return s->getStackTextArray(w); });
    }
    return retVal;
}

int CSceneObjectCustomizationPart::setMethodProperty(const char* pName, const void* pState, bool viaClass /*= false*/)
{
    std::string ppN(pName);
    ppN += "@cfunc";
    int retVal = sim_propertyret_unknownproperty;
    if (isClass())
    {
        bool changed = false;
        retVal = _customProperties.setMethodProperty(ppN.c_str(), pState, changed);
    }
    return retVal;
}

int CSceneObjectCustomizationPart::getMethodProperty(const char* pName, void*& pState, bool viaClass /*= false*/) const
{
    std::string ppN(pName);
    ppN += "@cfunc";
    int retVal = _customProperties.getMethodProperty(ppN.c_str(), pState);
    if ((!isClass()) && (retVal == sim_propertyret_unknownproperty))
    {
        CSceneObject* cl = App::scenes->customSceneObjectClasses->getClass(_sceneObject->getObjectTypeStr().c_str());
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

int CSceneObjectCustomizationPart::setMethodProperty(const char* pName, const std::string& pState, bool viaClass /*= false*/)
{
    int retVal = sim_propertyret_unknownproperty;
    if (isClass())
    {
        bool changed = false;
        retVal = _customProperties.setMethodProperty(pName, pState, changed);
        //if (changed)
        //    _triggerEvent(pName);
    }
    return retVal;
}

int CSceneObjectCustomizationPart::getMethodProperty(const char* pName, std::string& pState, bool viaClass /*= false*/) const
{
    int retVal = _customProperties.getMethodProperty(pName, pState);
    if ((!isClass()) && (retVal == sim_propertyret_unknownproperty))
    {
        CSceneObject* cl = App::scenes->customSceneObjectClasses->getClass(_sceneObject->getObjectTypeStr().c_str());
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

int CSceneObjectCustomizationPart::removeProperty(const char* pName, bool viaClass /*= false*/)
{
    int retVal = sim_propertyret_unknownproperty;
    if (viaClass)
        retVal = _sceneObject->Obj::removeProperty(pName);
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

int CSceneObjectCustomizationPart::getPropertyName(int& index, std::string& pName, std::string& appartenance, int excludeFlags, bool viaClass /*= false*/) const
{
    int retVal = sim_propertyret_unknownproperty;
    if (viaClass)
        retVal = _sceneObject->Obj::getPropertyName(index, pName, appartenance, excludeFlags);
    if (retVal == sim_propertyret_unknownproperty)
    {
        retVal = _customProperties.getPropertyName(index, pName, appartenance, excludeFlags);
        if ((!isClass()) && (retVal == sim_propertyret_unknownproperty))
        {
            CSceneObject* cl = App::scenes->customSceneObjectClasses->getClass(_sceneObject->getObjectTypeStr().c_str());
            if (cl != nullptr)
                retVal = cl->getPropertyName(index, pName, appartenance, excludeFlags | sim_propertyinfo_retmethodsonly);
        }
        if (retVal == sim_propertyret_ok)
        {
            if (isClass())
            {
                if ((pName == "customClass") || (pName == "name") || (pName == "target"))
                    appartenance = "TODO_CLASS_NAME";
                else
                {
                    std::string theName;
                    getStringProperty("name", theName);
                    appartenance = theName;
                }
            }
            else
                appartenance = _sceneObject->getObjectTypeStr();
        }
    }
    if ((retVal == sim_propertyret_unknownproperty) && isClass())
    {
        const std::vector<SProperty>* prop = &allProps_customSceneObjectClass;
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
                            appartenance = "TODO_CUSTOMSCENEOBJECTCLASS_APPARTENANCE";
                        else
                            appartenance = "TODO_CUSTOMSCENEOBJECT_APPARTENANCE";
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

int CSceneObjectCustomizationPart::getPropertyInfo(const char* pName, int& info, std::string& infoTxt, bool viaClass /*= false*/) const
{
    int retVal = sim_propertyret_unknownproperty;
    if (viaClass)
        retVal = _sceneObject->Obj::getPropertyInfo(pName, info, infoTxt);
    if (retVal == sim_propertyret_unknownproperty)
    {
        retVal = _customProperties.getPropertyInfo(pName, info, infoTxt);
        if ((!isClass()) && (retVal == sim_propertyret_unknownproperty))
        {
            CSceneObject* cl = App::scenes->customSceneObjectClasses->getClass(_sceneObject->getObjectTypeStr().c_str());
            if (cl != nullptr)
                retVal = cl->getPropertyInfo(pName, info, infoTxt);
        }
    }
    if ((retVal == sim_propertyret_unknownproperty) && isClass())
    {
        const std::vector<SProperty>* prop = &allProps_customObjectClass;
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

int CSceneObjectCustomizationPart::setPropertyInfo(const char* pName, int info, const char* infoTxt, bool viaClass /*= false*/)
{
    int retVal = sim_propertyret_unknownproperty;
    if (viaClass)
        retVal = _sceneObject->Obj::setPropertyInfo(pName, info, infoTxt);
    if (retVal == sim_propertyret_unknownproperty)
    {
        retVal = sim_propertyret_unknownproperty;
        if ((isClass() || _objectCanAddRemoveProperty))
            retVal = _customProperties.setPropertyInfo(pName, info, infoTxt);
        else
            retVal = sim_propertyret_unavailable;
    }
    return retVal;
}
