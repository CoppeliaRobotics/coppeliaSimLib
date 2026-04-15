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
    _isClass = true;
}

CustomObject::~CustomObject()
{
    if (!_isClass)
    {
        App::scenes->createEvent(EVENTTYPE_OBJECTREMOVED, _objectHandle, _objectHandle, nullptr, false);
        App::scenes->pushEvent();
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
    if (!_isClass)
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
                    if (getBoolProperty(pName, v) > 0)
                        ev->appendKeyBool(pName, v);
                }
                else if (t == sim_propertytype_int)
                {
                    int v;
                    if (getIntProperty(pName, v) > 0)
                        ev->appendKeyInt64(pName, v);
                }
                else if (t == sim_propertytype_long)
                {
                    long long int v;
                    if (getLongProperty(pName, v) > 0)
                        ev->appendKeyInt64(pName, v);
                }
                else if (t == sim_propertytype_float)
                {
                    double v;
                    if (getFloatProperty(pName, v) > 0)
                        ev->appendKeyDouble(pName, v);
                }
                else if (t == sim_propertytype_handle)
                {
                    long long int v;
                    if (getHandleProperty(pName, v) > 0)
                        ev->appendKeyHandle(pName, v);
                }
                else if (t == sim_propertytype_string)
                {
                    std::string v;
                    if (getStringProperty(pName, v) > 0)
                        ev->appendKeyText(pName, v.c_str());
                }
                else if (t == sim_propertytype_buffer)
                {
                    std::string v;
                    if (getBufferProperty(pName, v) > 0)
                        ev->appendKeyBuff(pName, (unsigned char*)v.data(), v.size());
                }
                else if (t == sim_propertytype_intarray2)
                {
                    int v[2];
                    if (getIntArray2Property(pName, v) > 0)
                        ev->appendKeyInt32Array(pName, v, 2);
                }
                else if (t == sim_propertytype_vector2)
                {
                    double v[2];
                    if (getVector2Property(pName, v) > 0)
                        ev->appendKeyDoubleArray(pName, v, 2);
                }
                else if (t == sim_propertytype_vector3)
                {
                    C3Vector v;
                    if (getVector3Property(pName, v) > 0)
                        ev->appendKeyVector3(pName, v);
                }
                else if (t == sim_propertytype_quaternion)
                {
                    C4Vector v;
                    if (getQuaternionProperty(pName, v) > 0)
                        ev->appendKeyQuaternion(pName, v);
                }
                else if (t == sim_propertytype_pose)
                {
                    C7Vector v;
                    if (getPoseProperty(pName, v) > 0)
                        ev->appendKeyPose(pName, v);
                }
                else if (t == sim_propertytype_matrix)
                {
                    CMatrix v;
                    if (getMatrixProperty(pName, v) > 0)
                        ev->appendKeyMatrix(pName, v.data.data(), v.rows, v.cols);
                }
                else if (t == sim_propertytype_color)
                {
                    float v[3];
                    if (getColorProperty(pName, v) > 0)
                        ev->appendKeyFloatArray(pName, v, 3);
                }
                else if (t == sim_propertytype_floatarray)
                {
                    std::vector<double> v;
                    if (getFloatArrayProperty(pName, v) > 0)
                        ev->appendKeyDoubleArray(pName, v.data(), v.size());
                }
                else if (t == sim_propertytype_intarray)
                {
                    std::vector<int> v;
                    if (getIntArrayProperty(pName, v) > 0)
                        ev->appendKeyInt32Array(pName, v.data(), v.size());
                }
                else if (t == sim_propertytype_handlearray)
                {
                    std::vector<long long int> v;
                    if (getHandleArrayProperty(pName, v) > 0)
                        ev->appendKeyHandleArray(pName, v.data(), v.size());
                }
                else if (t == sim_propertytype_stringarray)
                {
                    std::vector<std::string> v;
                    if (getStringArrayProperty(pName, v) > 0)
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

int CustomObject::setBoolProperty(const char* pName, bool pState)
{
    int retVal = Obj::setBoolProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        bool changed = false;
        retVal = _customProperties.setBoolProperty(pName, pState, changed);
        if (changed)
            _triggerEvent(pName);
    }
    return retVal;
}

int CustomObject::getBoolProperty(const char* pName, bool& pState) const
{
    int retVal = Obj::getBoolProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
        retVal = _customProperties.getBoolProperty(pName, pState);
    return retVal;
}

int CustomObject::setIntProperty(const char* pName, int pState)
{
    int retVal = Obj::setIntProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        bool changed = false;
        retVal = _customProperties.setIntProperty(pName, pState, changed);
        if (changed)
            _triggerEvent(pName);
    }
    return retVal;
}

int CustomObject::getIntProperty(const char* pName, int& pState) const
{
    int retVal = Obj::getIntProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
        retVal = _customProperties.getIntProperty(pName, pState);
    return retVal;
}

int CustomObject::setLongProperty(const char* pName, long long int pState)
{
    int retVal = Obj::setLongProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        bool changed = false;
        retVal = _customProperties.setLongProperty(pName, pState, changed);
        if (changed)
            _triggerEvent(pName);
    }
    return retVal;
}

int CustomObject::getLongProperty(const char* pName, long long int& pState) const
{
    int retVal = Obj::getLongProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
        retVal = _customProperties.getLongProperty(pName, pState);
    return retVal;
}

int CustomObject::setFloatProperty(const char* pName, double pState)
{
    int retVal = Obj::setFloatProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        bool changed = false;
        retVal = _customProperties.setFloatProperty(pName, pState, changed);
        if (changed)
            _triggerEvent(pName);
    }
    return retVal;
}

int CustomObject::getFloatProperty(const char* pName, double& pState) const
{
    int retVal = Obj::getFloatProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
        retVal = _customProperties.getFloatProperty(pName, pState);
    return retVal;
}

int CustomObject::setHandleProperty(const char* pName, long long int pState)
{
    int retVal = Obj::setHandleProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        bool changed = false;
        retVal = _customProperties.setHandleProperty(pName, pState, changed);
        if (changed)
            _triggerEvent(pName);
    }
    return retVal;
}

int CustomObject::getHandleProperty(const char* pName, long long int& pState) const
{
    int retVal = Obj::getHandleProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
        retVal = _customProperties.getHandleProperty(pName, pState);
    return retVal;
}

int CustomObject::setStringProperty(const char* pName, const char* pState)
{
    int retVal = Obj::setStringProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        bool changed = false;
        retVal = _customProperties.setStringProperty(pName, pState, changed);
        if (changed)
            _triggerEvent(pName);
    }
    return retVal;
}

int CustomObject::getStringProperty(const char* pName, std::string& pState) const
{
    int retVal = Obj::getStringProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
        retVal = _customProperties.getStringProperty(pName, pState);
    return retVal;
}

int CustomObject::setBufferProperty(const char* pName, const char* buffer, int bufferL)
{
    int retVal = Obj::setBufferProperty(pName, buffer, bufferL);
    if (retVal == sim_propertyret_unknownproperty)
    {
        bool changed = false;
        retVal = _customProperties.setBufferProperty(pName, buffer, bufferL, changed);
        if (changed)
            _triggerEvent(pName);
    }
    return retVal;
}

int CustomObject::getBufferProperty(const char* pName, std::string& pState) const
{
    int retVal = Obj::getBufferProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
        retVal = _customProperties.getBufferProperty(pName, pState);
    return retVal;
}

int CustomObject::setIntArray2Property(const char* pName, const int* pState)
{
    int retVal = Obj::setIntArray2Property(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        bool changed = false;
        retVal = _customProperties.setIntArray2Property(pName, pState, changed);
        if (changed)
            _triggerEvent(pName);
    }
    return retVal;
}

int CustomObject::getIntArray2Property(const char* pName, int* pState) const
{
    int retVal = Obj::getIntArray2Property(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
        retVal = _customProperties.getIntArray2Property(pName, pState);
    return retVal;
}

int CustomObject::setVector2Property(const char* pName, const double* pState)
{
    int retVal = Obj::setVector2Property(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        bool changed = false;
        retVal = _customProperties.setVector2Property(pName, pState, changed);
        if (changed)
            _triggerEvent(pName);
    }
    return retVal;
}

int CustomObject::getVector2Property(const char* pName, double* pState) const
{
    int retVal = Obj::getVector2Property(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
        retVal = _customProperties.getVector2Property(pName, pState);
    return retVal;
}

int CustomObject::setVector3Property(const char* pName, const C3Vector& pState)
{
    int retVal = Obj::setVector3Property(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        bool changed = false;
        retVal = _customProperties.setVector3Property(pName, pState, changed);
        if (changed)
            _triggerEvent(pName);
    }
    return retVal;
}

int CustomObject::getVector3Property(const char* pName, C3Vector& pState) const
{
    int retVal = Obj::getVector3Property(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
        retVal = _customProperties.getVector3Property(pName, pState);
    return retVal;
}

int CustomObject::setMatrixProperty(const char* pName, const CMatrix& pState)
{
    int retVal = Obj::setMatrixProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        bool changed = false;
        retVal = _customProperties.setMatrixProperty(pName, pState, changed);
        if (changed)
            _triggerEvent(pName);
    }
    return retVal;
}

int CustomObject::getMatrixProperty(const char* pName, CMatrix& pState) const
{
    int retVal = Obj::getMatrixProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
        retVal = _customProperties.getMatrixProperty(pName, pState);
    return retVal;
}

int CustomObject::setQuaternionProperty(const char* pName, const C4Vector& pState)
{
    int retVal = Obj::setQuaternionProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        bool changed = false;
        retVal = _customProperties.setQuaternionProperty(pName, pState, changed);
        if (changed)
            _triggerEvent(pName);
    }
    return retVal;
}

int CustomObject::getQuaternionProperty(const char* pName, C4Vector& pState) const
{
    int retVal = Obj::getQuaternionProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
        retVal = _customProperties.getQuaternionProperty(pName, pState);
    return retVal;
}

int CustomObject::setPoseProperty(const char* pName, const C7Vector& pState)
{
    int retVal = Obj::setPoseProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        bool changed = false;
        retVal = _customProperties.setPoseProperty(pName, pState, changed);
        if (changed)
            _triggerEvent(pName);
    }
    return retVal;
}

int CustomObject::getPoseProperty(const char* pName, C7Vector& pState) const
{
    int retVal = Obj::getPoseProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
        retVal = _customProperties.getPoseProperty(pName, pState);
    return retVal;
}

int CustomObject::setColorProperty(const char* pName, const float* pState)
{
    int retVal = Obj::setColorProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        bool changed = false;
        retVal = _customProperties.setColorProperty(pName, pState, changed);
        if (changed)
            _triggerEvent(pName);
    }
    return retVal;
}

int CustomObject::getColorProperty(const char* pName, float* pState) const
{
    int retVal = Obj::getColorProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
        retVal = _customProperties.getColorProperty(pName, pState);
    return retVal;
}

int CustomObject::setFloatArrayProperty(const char* pName, const double* v, int vL)
{
    int retVal = Obj::setFloatArrayProperty(pName, v, vL);
    if (retVal == sim_propertyret_unknownproperty)
    {
        bool changed = false;
        retVal = _customProperties.setFloatArrayProperty(pName, v, vL, changed);
        if (changed)
            _triggerEvent(pName);
    }
    return retVal;
}

int CustomObject::getFloatArrayProperty(const char* pName, std::vector<double>& pState) const
{
    pState.clear();
    int retVal = Obj::getFloatArrayProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
        retVal = _customProperties.getFloatArrayProperty(pName, pState);
    return retVal;
}

int CustomObject::setIntArrayProperty(const char* pName, const int* v, int vL)
{
    if (v == nullptr)
        vL = 0;
    int retVal = Obj::setIntArrayProperty(pName, v, vL);
    if (retVal == sim_propertyret_unknownproperty)
    {
        bool changed = false;
        retVal = _customProperties.setIntArrayProperty(pName, v, vL, changed);
        if (changed)
            _triggerEvent(pName);
    }
    return retVal;
}

int CustomObject::getIntArrayProperty(const char* pName, std::vector<int>& pState) const
{
    pState.clear();
    int retVal = Obj::getIntArrayProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
        retVal = _customProperties.getIntArrayProperty(pName, pState);
    return retVal;
}

int CustomObject::setHandleArrayProperty(const char* pName, const long long int* v, int vL)
{
    if (v == nullptr)
        vL = 0;
    int retVal = Obj::setHandleArrayProperty(pName, v, vL);
    if (retVal == sim_propertyret_unknownproperty)
    {
        bool changed = false;
        retVal = _customProperties.setHandleArrayProperty(pName, v, vL, changed);
        if (changed)
            _triggerEvent(pName);
    }
    return retVal;
}

int CustomObject::getHandleArrayProperty(const char* pName, std::vector<long long int>& pState) const
{
    pState.clear();
    int retVal = Obj::getHandleArrayProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
        retVal = _customProperties.getHandleArrayProperty(pName, pState);
    return retVal;
}

int CustomObject::setStringArrayProperty(const char* pName, const std::vector<std::string>& pState)
{
    int retVal = Obj::setStringArrayProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
    {
        bool changed = false;
        retVal = _customProperties.setStringArrayProperty(pName, pState, changed);
        if (changed)
            _triggerEvent(pName);
    }
    return retVal;
}

int CustomObject::getStringArrayProperty(const char* pName, std::vector<std::string>& pState) const
{
    pState.clear();
    int retVal = Obj::getStringArrayProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
        retVal = _customProperties.getStringArrayProperty(pName, pState);
    return retVal;
}

int CustomObject::setMethodProperty(const char* pName, const void* pState)
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

int CustomObject::getMethodProperty(const char* pName, void*& pState) const
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

int CustomObject::getPropertyName(int& index, std::string& pName, std::string& appartenance, int excludeFlags) const
{
    int retVal = Obj::getPropertyName(index, pName, appartenance, excludeFlags);
    if (retVal == sim_propertyret_unknownproperty)
    {
        retVal = _customProperties.getPropertyName(index, pName, appartenance, excludeFlags);
        if (retVal == sim_propertyret_ok)
            appartenance = getObjectTypeStr();
    }
    return retVal;
}

int CustomObject::getPropertyInfo(const char* pName, int& info, std::string& infoTxt) const
{
    int retVal = Obj::getPropertyInfo(pName, info, infoTxt);
    if (retVal == sim_propertyret_unknownproperty)
        retVal = _customProperties.getPropertyInfo(pName, info, infoTxt);
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
