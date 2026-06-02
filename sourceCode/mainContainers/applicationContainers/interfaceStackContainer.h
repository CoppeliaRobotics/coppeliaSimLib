#pragma once

#include <interfaceStack.h>

class CInterfaceStackContainer
{
  public:
    CInterfaceStackContainer();
    virtual ~CInterfaceStackContainer();

    CInterfaceStack* createStack();
    CInterfaceStack* createStackCopy(const CInterfaceStack* original);
    bool destroyStack(int id);
    bool destroyStack(CInterfaceStack* stack);
    CInterfaceStack* getStack(int id) const;

    int setBoolProperty_t(int64_t target, const char* pName, bool pState);
    int getBoolProperty_t(int64_t target, const char* pName, bool& pState) const;
    int setIntProperty_t(int64_t target, const char* pName, int pState);
    int getIntProperty_t(int64_t target, const char* pName, int& pState) const;
    int setLongProperty_t(int64_t target, const char* pName, int64_t pState);
    int getLongProperty_t(int64_t target, const char* pName, int64_t& pState) const;
    int setFloatProperty_t(int64_t target, const char* pName, double pState);
    int getFloatProperty_t(int64_t target, const char* pName, double& pState) const;
    int setHandleProperty_t(int64_t target, const char* pName, int64_t pState);
    int getHandleProperty_t(int64_t target, const char* pName, int64_t& pState) const;
    int setStringProperty_t(int64_t target, const char* pName, const std::string& pState);
    int getStringProperty_t(int64_t target, const char* pName, std::string& pState) const;
    int setBufferProperty_t(int64_t target, const char* pName, const std::string& pState);
    int getBufferProperty_t(int64_t target, const char* pName, std::string& pState) const;
    int setIntArray2Property_t(int64_t target, const char* pName, const int* pState);
    int getIntArray2Property_t(int64_t target, const char* pName, int* pState) const;
    int setVector3Property_t(int64_t target, const char* pName, const C3Vector& pState);
    int getVector3Property_t(int64_t target, const char* pName, C3Vector& pState) const;
    int setMatrixProperty_t(int64_t target, const char* pName, const CMatrix& pState);
    int getMatrixProperty_t(int64_t target, const char* pName, CMatrix& pState) const;
    int setQuaternionProperty_t(int64_t target, const char* pName, const CQuaternion& pState);
    int getQuaternionProperty_t(int64_t target, const char* pName, CQuaternion& pState) const;
    int setPoseProperty_t(int64_t target, const char* pName, const CPose& pState);
    int getPoseProperty_t(int64_t target, const char* pName, CPose& pState) const;
    int setColorProperty_t(int64_t target, const char* pName, const float* pState);
    int getColorProperty_t(int64_t target, const char* pName, float* pState) const;
    int setFloatArrayProperty_t(int64_t target, const char* pName, const std::vector<double>& pState);
    int getFloatArrayProperty_t(int64_t target, const char* pName, std::vector<double>& pState) const;
    int setIntArrayProperty_t(int64_t target, const char* pName, const std::vector<int>& pState);
    int getIntArrayProperty_t(int64_t target, const char* pName, std::vector<int>& pState) const;
    int setHandleArrayProperty_t(int64_t target, const char* pName, const std::vector<int64_t>& pState);
    int getHandleArrayProperty_t(int64_t target, const char* pName, std::vector<int64_t>& pState) const;
    int setStringArrayProperty_t(int64_t target, const char* pName, const std::vector<std::string>& pState);
    int getStringArrayProperty_t(int64_t target, const char* pName, std::vector<std::string>& pState) const;
    int removeProperty_t(int64_t target, const char* pName);
    int getPropertyName_t(int64_t target, int& index, std::string& pName, std::string& appartenance, int excludeFlags) const;
    int getPropertyInfo_t(int64_t target, const char* pName, int& info, std::string& infoTxt) const;

  protected:
    std::vector<CInterfaceStack*> _allStacks;
};
