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
    CSceneObjectCustomizationPart(CSceneObject* sceneObj);
    virtual ~CSceneObjectCustomizationPart();

    void serialize(CSer& ar);
    CSceneObjectCustomizationPart* copyYourself(CSceneObject* appartenance) const;

    int setBoolProperty(const char* pName, bool pState, bool viaClass = false);
    int getBoolProperty(const char* pName, bool& pState, bool viaClass = false) const;
    int setIntProperty(const char* pName, int pState, bool viaClass = false);
    int getIntProperty(const char* pName, int& pState, bool viaClass = false) const;
    int setLongProperty(const char* pName, long long int pState, bool viaClass = false);
    int getLongProperty(const char* pName, long long int& pState, bool viaClass = false) const;
    int setFloatProperty(const char* pName, double pState, bool viaClass = false);
    int getFloatProperty(const char* pName, double& pState, bool viaClass = false) const;
    int setHandleProperty(const char* pName, long long int pState, bool viaClass = false);
    int getHandleProperty(const char* pName, long long int& pState, bool viaClass = false) const;
    int setStringProperty(const char* pName, const std::string& pState, bool viaClass = false);
    int getStringProperty(const char* pName, std::string& pState, bool viaClass = false) const;
    int setTableProperty(const char* pName, const std::string& pState, bool viaClass = false);
    int getTableProperty(const char* pName, std::string& pState, bool viaClass = false) const;
    int setBufferProperty(const char* pName, const std::string& pState, bool viaClass = false);
    int getBufferProperty(const char* pName, std::string& pState, bool viaClass = false) const;
    int setIntArray2Property(const char* pName, const int* pState, bool viaClass = false);
    int getIntArray2Property(const char* pName, int* pState, bool viaClass = false) const;
    int setVector3Property(const char* pName, const C3Vector& pState, bool viaClass = false);
    int getVector3Property(const char* pName, C3Vector& pState, bool viaClass = false) const;
    int setMatrixProperty(const char* pName, const CMatrix& pState, bool viaClass = false);
    int getMatrixProperty(const char* pName, CMatrix& pState, bool viaClass = false) const;
    int setQuaternionProperty(const char* pName, const C4Vector& pState, bool viaClass = false);
    int getQuaternionProperty(const char* pName, C4Vector& pState, bool viaClass = false) const;
    int setPoseProperty(const char* pName, const C7Vector& pState, bool viaClass = false);
    int getPoseProperty(const char* pName, C7Vector& pState, bool viaClass = false) const;
    int setColorProperty(const char* pName, const float* pState, bool viaClass = false);
    int getColorProperty(const char* pName, float* pState, bool viaClass = false) const;
    int setFloatArrayProperty(const char* pName, const std::vector<double>& pState, bool viaClass = false);
    int getFloatArrayProperty(const char* pName, std::vector<double>& pState, bool viaClass = false) const;
    int setIntArrayProperty(const char* pName, const std::vector<int>& pState, bool viaClass = false);
    int getIntArrayProperty(const char* pName, std::vector<int>& pState, bool viaClass = false) const;
    int setHandleArrayProperty(const char* pName, const std::vector<long long int>& pState, bool viaClass = false);
    int getHandleArrayProperty(const char* pName, std::vector<long long int>& pState, bool viaClass = false) const;
    int setStringArrayProperty(const char* pName, const std::vector<std::string>& pState, bool viaClass = false);
    int getStringArrayProperty(const char* pName, std::vector<std::string>& pState, bool viaClass = false) const;
    int setMethodProperty(const char* pName, const void* pState, bool viaClass = false);
    int getMethodProperty(const char* pName, void*& pState, bool viaClass = false) const;
    int setMethodProperty(const char* pName, const std::string& pState, bool viaClass = false);
    int getMethodProperty(const char* pName, std::string& pState, bool viaClass = false) const;
    int removeProperty(const char* pName, bool viaClass = false);
    int getPropertyName(int& index, std::string& pName, std::string& appartenance, int excludeFlags, bool viaClass = false) const;
    int getPropertyInfo(const char* pName, int& info, std::string& infoTxt, bool viaClass = false) const;
    int setPropertyInfo(const char* pName, int info, const char* infoTxt, bool viaClass = false);

    bool isClass() const;
    bool getResetChanged();
    void setIgnoreSetterGetter(bool f);
    void setObjectCanAddRemoveProperty(bool f);

  protected:
    template <typename T, typename PushF, typename GetF>
    void _callPropertySetterGetter(const char* pName, const char* suffix, T& pState, PushF pushFunc, GetF getFunc) const;
    void _triggerEvent(const char* pName, CCbor* evv = nullptr);

    CSceneObject* _sceneObject;
    bool _changed;
    bool _ignoreSetterGetter;
    bool _objectCanAddRemoveProperty;
    CCustomProperties _customProperties;
};
