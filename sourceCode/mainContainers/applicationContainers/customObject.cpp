#include <customObject.h>
#include <utils.h>

CustomObject::CustomObject(long long int handle, const char* objectTypeStr, const char* objectMetaInfo, int scriptHandle)
{
    _objectHandle = handle;
    _objectTypeStr = objectTypeStr;
    _objectMetaInfo = objectMetaInfo;
    _scriptHandle = scriptHandle;
}

CustomObject::~CustomObject()
{
}

int CustomObject::getScriptHandle() const
{
    return _scriptHandle;
}

int CustomObject::setBoolProperty(const char* pName, bool pState)
{
    int retVal = Obj::setBoolProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
        retVal = _customProperties.setBoolProperty(pName, pState);
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
        retVal = _customProperties.setIntProperty(pName, pState);
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
        retVal = _customProperties.setLongProperty(pName, pState);
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
        retVal = _customProperties.setFloatProperty(pName, pState);
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
        retVal = _customProperties.setHandleProperty(pName, pState);
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
        retVal = _customProperties.setStringProperty(pName, pState);
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
        retVal = _customProperties.setBufferProperty(pName, buffer, bufferL);
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
        retVal = _customProperties.setIntArray2Property(pName, pState);
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
        retVal = _customProperties.setVector2Property(pName, pState);
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
        retVal = _customProperties.setVector3Property(pName, pState);
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
        retVal = _customProperties.setMatrixProperty(pName, pState);
    return retVal;
}

int CustomObject::getMatrixProperty(const char* pName, CMatrix& pState) const
{
    int retVal = Obj::getMatrixProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
        retVal = _customProperties.getMatrixProperty(pName, pState);
    return retVal;
}

int CustomObject::setMatrix3x3Property(const char* pName, const CMatrix& pState)
{
    int retVal = Obj::setMatrix3x3Property(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
        retVal = _customProperties.setMatrix3x3Property(pName, pState);
    return retVal;
}

int CustomObject::getMatrix3x3Property(const char* pName, CMatrix& pState) const
{
    int retVal = Obj::getMatrix3x3Property(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
        retVal = _customProperties.getMatrix3x3Property(pName, pState);
    return retVal;
}

int CustomObject::setMatrix4x4Property(const char* pName, const CMatrix& pState)
{
    int retVal = Obj::setMatrix4x4Property(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
        retVal = _customProperties.setMatrix4x4Property(pName, pState);
    return retVal;
}

int CustomObject::getMatrix4x4Property(const char* pName, CMatrix& pState) const
{
    int retVal = Obj::getMatrix4x4Property(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
        retVal = _customProperties.getMatrix4x4Property(pName, pState);
    return retVal;
}

int CustomObject::setQuaternionProperty(const char* pName, const C4Vector& pState)
{
    int retVal = Obj::setQuaternionProperty(pName, pState);
    if (retVal == sim_propertyret_unknownproperty)
        retVal = _customProperties.setQuaternionProperty(pName, pState);
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
        retVal = _customProperties.setPoseProperty(pName, pState);
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
        retVal = _customProperties.setColorProperty(pName, pState);
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
        retVal = _customProperties.setFloatArrayProperty(pName, v, vL);
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
        retVal = _customProperties.setIntArrayProperty(pName, v, vL);
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
        retVal = _customProperties.setHandleArrayProperty(pName, v, vL);
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
        retVal = _customProperties.setStringArrayProperty(pName, pState);
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

int CustomObject::removeProperty(const char* pName)
{
    int retVal = Obj::removeProperty(pName);
    if (retVal == sim_propertyret_unknownproperty)
        retVal = _customProperties.removeProperty(pName);
    return retVal;
}

int CustomObject::getPropertyName(int& index, std::string& pName, std::string& appartenance, int excludeFlags) const
{
    int retVal = Obj::getPropertyName(index, pName, appartenance, excludeFlags);
    if (retVal == sim_propertyret_unknownproperty)
        retVal = _customProperties.getPropertyName(index, pName, appartenance, excludeFlags);
    return retVal;
}

int CustomObject::getPropertyInfo(const char* pName, int& info, std::string& infoTxt) const
{
    int retVal = Obj::getPropertyInfo(pName, info, infoTxt);
    if (retVal == sim_propertyret_unknownproperty)
        retVal = _customProperties.getPropertyInfo(pName, info, infoTxt);
    return retVal;
}
