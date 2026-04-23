#pragma once

#include <obj.h>
#include <ser.h>
#include <interfaceStack.h>
#include <customProperties.h>

class CustomObject: public Obj
{
  public:
    CustomObject(long long int handle,const char* objectTypeStr, const char* objectMetaInfo, int originScriptHandle, int target); // class definition
    virtual ~CustomObject();

    CustomObject* createObject(long long int handle, int originScriptHandle) const;

    void pushObjectCreationEvent();
    void serialize(CSer& ar);

    int setBoolProperty(const char* pName, bool pState) override;
    int getBoolProperty(const char* pName, bool& pState) const override;
    int setIntProperty(const char* pName, int pState) override;
    int getIntProperty(const char* pName, int& pState) const override;
    int setLongProperty(const char* pName, long long int pState) override;
    int getLongProperty(const char* pName, long long int& pState) const override;
    int setFloatProperty(const char* pName, double pState) override;
    int getFloatProperty(const char* pName, double& pState) const override;
    int setHandleProperty(const char* pName, long long int pState) override;
    int getHandleProperty(const char* pName, long long int& pState) const override;
    int setStringProperty(const char* pName, const std::string& pState) override;
    int getStringProperty(const char* pName, std::string& pState) const override;
    int setBufferProperty(const char* pName, const std::string& pState) override;
    int getBufferProperty(const char* pName, std::string& pState) const override;
    int setIntArray2Property(const char* pName, const int* pState) override;
    int getIntArray2Property(const char* pName, int* pState) const override;
    int setVector3Property(const char* pName, const C3Vector& pState) override;
    int getVector3Property(const char* pName, C3Vector& pState) const override;
    int setMatrixProperty(const char* pName, const CMatrix& pState) override;
    int getMatrixProperty(const char* pName, CMatrix& pState) const override;
    int setQuaternionProperty(const char* pName, const C4Vector& pState) override;
    int getQuaternionProperty(const char* pName, C4Vector& pState) const override;
    int setPoseProperty(const char* pName, const C7Vector& pState) override;
    int getPoseProperty(const char* pName, C7Vector& pState) const override;
    int setColorProperty(const char* pName, const float* pState) override;
    int getColorProperty(const char* pName, float* pState) const override;
    int setFloatArrayProperty(const char* pName, const std::vector<double>& pState) override;
    int getFloatArrayProperty(const char* pName, std::vector<double>& pState) const override;
    int setIntArrayProperty(const char* pName, const std::vector<int>& pState) override;
    int getIntArrayProperty(const char* pName, std::vector<int>& pState) const override;
    int setHandleArrayProperty(const char* pName, const std::vector<long long int>& pState) override;
    int getHandleArrayProperty(const char* pName, std::vector<long long int>& pState) const override;
    int setStringArrayProperty(const char* pName, const std::vector<std::string>& pState) override;
    int getStringArrayProperty(const char* pName, std::vector<std::string>& pState) const override;
    int setMethodProperty(const char* pName, const void* pState) override;
    int getMethodProperty(const char* pName, void*& pState) const override;
    int setMethodProperty(const char* pName, const std::string& pState) override;
    int getMethodProperty(const char* pName, std::string& pState) const override;
    int removeProperty(const char* pName) override;
    int getPropertyName(int& index, std::string& pName, std::string& appartenance, int excludeFlags) const override;
    int getPropertyInfo(const char* pName, int& info, std::string& infoTxt) const override;
    int setPropertyInfo(const char* pName, int info, const char* infoTxt) override;

    CCustomProperties& getCustomProperties() { return _customProperties; }
    const CCustomProperties& getCustomProperties() const { return _customProperties; }

    int getScriptHandle() const;
    bool getVolatile() const;
    void setVolatile(bool v);
    bool getResetChanged();
    void setIgnoreSetterGetter(bool f);
    void setObjectCanAddRemoveProperty(bool f);


  protected:
    template <typename T, typename PushF, typename GetF>
    void _callPropertySetterGetter(const char* pName, const char* suffix, T& pState, PushF pushFunc, GetF getFunc) const;
    void _triggerEvent(const char* pName, CCbor* evv = nullptr);

    int _scriptHandle;
    int _target;
    bool _volatile;
    bool _changed;
    bool _ignoreSetterGetter;
    bool _objectCanAddRemoveProperty;
    CCustomProperties _customProperties;
};
