#pragma once

#include <ser.h>
#include <simMath/3Vector.h>
#include <simMath/7Vector.h>
#include <simMath/mXnMatrix.h>
#include <customProperties.h>

// ----------------------------------------------------------------------------------------------
#define FUNCX(name, str, v1, v2, t1, t2) extern const SProperty name;
CUSTOMSCENEOBJECTCLASS_PROPERTIES
#undef FUNCX
extern const std::vector<SProperty> allProps_customSceneObjectClass;
// ----------------------------------------------------------------------------------------------

class CSceneObject;

class CSceneObjectCustomizationPart
{
  public:
    CSceneObjectCustomizationPart(CSceneObject* sceneObj, const char* objectTypeStr);
    virtual ~CSceneObjectCustomizationPart();

    void serialize(CSer& ar);
    CSceneObjectCustomizationPart* copyYourself(CSceneObject* appartenance) const;

    int setBoolProperty(const char* pName, bool pState);
    int getBoolProperty(const char* pName, bool& pState) const;
    int setIntProperty(const char* pName, int pState);
    int getIntProperty(const char* pName, int& pState) const;
    int setLongProperty(const char* pName, long long int pState);
    int getLongProperty(const char* pName, long long int& pState) const;
    int setFloatProperty(const char* pName, double pState);
    int getFloatProperty(const char* pName, double& pState) const;
    int setHandleProperty(const char* pName, long long int pState);
    int getHandleProperty(const char* pName, long long int& pState) const;
    int setStringProperty(const char* pName, const std::string& pState);
    int getStringProperty(const char* pName, std::string& pState) const;
    int setTableProperty(const char* pName, const std::string& pState);
    int getTableProperty(const char* pName, std::string& pState) const;
    int setBufferProperty(const char* pName, const std::string& pState);
    int getBufferProperty(const char* pName, std::string& pState) const;
    int setIntArray2Property(const char* pName, const int* pState);
    int getIntArray2Property(const char* pName, int* pState) const;
    int setVector3Property(const char* pName, const C3Vector& pState);
    int getVector3Property(const char* pName, C3Vector& pState) const;
    int setMatrixProperty(const char* pName, const CMatrix& pState);
    int getMatrixProperty(const char* pName, CMatrix& pState) const;
    int setQuaternionProperty(const char* pName, const C4Vector& pState);
    int getQuaternionProperty(const char* pName, C4Vector& pState) const;
    int setPoseProperty(const char* pName, const C7Vector& pState);
    int getPoseProperty(const char* pName, C7Vector& pState) const;
    int setColorProperty(const char* pName, const float* pState);
    int getColorProperty(const char* pName, float* pState) const;
    int setFloatArrayProperty(const char* pName, const std::vector<double>& pState);
    int getFloatArrayProperty(const char* pName, std::vector<double>& pState) const;
    int setIntArrayProperty(const char* pName, const std::vector<int>& pState);
    int getIntArrayProperty(const char* pName, std::vector<int>& pState) const;
    int setHandleArrayProperty(const char* pName, const std::vector<long long int>& pState);
    int getHandleArrayProperty(const char* pName, std::vector<long long int>& pState) const;
    int setStringArrayProperty(const char* pName, const std::vector<std::string>& pState);
    int getStringArrayProperty(const char* pName, std::vector<std::string>& pState) const;
    int setMethodProperty(const char* pName, const void* pState);
    int getMethodProperty(const char* pName, void*& pState) const;
    int setMethodProperty(const char* pName, const std::string& pState);
    int getMethodProperty(const char* pName, std::string& pState) const;
    int removeProperty(const char* pName);
    int getPropertyName(int& index, std::string& pName, std::string& appartenance, int excludeFlags) const;
    int getPropertyInfo(const char* pName, int& info, std::string& infoTxt) const;
    int setPropertyInfo(const char* pName, int info, const char* infoTxt);

    bool isClass() const;
    bool getResetChanged();
    void setIgnoreSetterGetter(bool f);
    void setObjectCanAddRemoveProperty(bool f);

  protected:
    template <typename T, typename PushF, typename GetF>
    void _callPropertySetterGetter(const char* pName, const char* suffix, T& pState, PushF pushFunc, GetF getFunc) const;
    void _triggerEvent(const char* pName, CCbor* evv = nullptr);

    CSceneObject* _sceneObject;
    std::string _objectTypeStr;
    bool _changed;
    bool _ignoreSetterGetter;
    bool _objectCanAddRemoveProperty;
    CCustomProperties _customProperties;
};
