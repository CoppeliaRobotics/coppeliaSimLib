#pragma once

#include <obj.h>
#include <customProperties.h>

class CustomObject: public Obj
{
  public:
    CustomObject(long long int handle,const char* objectTypeStr, const char* objectMetaInfo, int originScriptHandle); // class definition
    virtual ~CustomObject();

    CustomObject* createObject(long long int handle, int originScriptHandle) const;

    void pushObjectCreationEvent() const;

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
    int setStringProperty(const char* pName, const char* pState) override;
    int getStringProperty(const char* pName, std::string& pState) const override;
    int setBufferProperty(const char* pName, const char* buffer, int bufferL) override;
    int getBufferProperty(const char* pName, std::string& pState) const override;
    int setIntArray2Property(const char* pName, const int* pState) override;
    int getIntArray2Property(const char* pName, int* pState) const override;
    int setVector2Property(const char* pName, const double* pState) override;
    int getVector2Property(const char* pName, double* pState) const override;
    int setVector3Property(const char* pName, const C3Vector& pState) override;
    int getVector3Property(const char* pName, C3Vector& pState) const override;
    int setMatrixProperty(const char* pName, const CMatrix& pState) override;
    int getMatrixProperty(const char* pName, CMatrix& pState) const override;
    int setMatrix3x3Property(const char* pName, const CMatrix& pState) override;
    int getMatrix3x3Property(const char* pName, CMatrix& pState) const override;
    int setMatrix4x4Property(const char* pName, const CMatrix& pState) override;
    int getMatrix4x4Property(const char* pName, CMatrix& pState) const override;
    int setQuaternionProperty(const char* pName, const C4Vector& pState) override;
    int getQuaternionProperty(const char* pName, C4Vector& pState) const override;
    int setPoseProperty(const char* pName, const C7Vector& pState) override;
    int getPoseProperty(const char* pName, C7Vector& pState) const override;
    int setColorProperty(const char* pName, const float* pState) override;
    int getColorProperty(const char* pName, float* pState) const override;
    int setFloatArrayProperty(const char* pName, const double* v, int vL) override;
    int getFloatArrayProperty(const char* pName, std::vector<double>& pState) const override;
    int setIntArrayProperty(const char* pName, const int* v, int vL) override;
    int getIntArrayProperty(const char* pName, std::vector<int>& pState) const override;
    int setHandleArrayProperty(const char* pName, const long long int* v, int vL) override;
    int getHandleArrayProperty(const char* pName, std::vector<long long int>& pState) const override;
    int setStringArrayProperty(const char* pName, const std::vector<std::string>& pState) override;
    int getStringArrayProperty(const char* pName, std::vector<std::string>& pState) const override;
    int removeProperty(const char* pName) override;
    int getPropertyName(int& index, std::string& pName, std::string& appartenance, int excludeFlags) const override;
    int getPropertyInfo(const char* pName, int& info, std::string& infoTxt) const override;
    int setPropertyInfo(const char* pName, int info, const char* infoTxt);

    CCustomProperties& getCustomProperties() { return _customProperties; }
    const CCustomProperties& getCustomProperties() const { return _customProperties; }

    int getScriptHandle() const;

  protected:
    void _triggerEvent(const char* pName, CCbor* evv = nullptr) const;

    bool _isClass;
    int _scriptHandle;
    CCustomProperties _customProperties;
};
