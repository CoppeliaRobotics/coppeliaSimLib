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

    int setBoolProperty_t(long long int target, const char* pName, bool pState);
    int getBoolProperty_t(long long int target, const char* pName, bool& pState) const;
    int setIntProperty_t(long long int target, const char* pName, int pState);
    int getIntProperty_t(long long int target, const char* pName, int& pState) const;
    int setLongProperty_t(long long int target, const char* pName, long long int pState);
    int getLongProperty_t(long long int target, const char* pName, long long int& pState) const;
    int setFloatProperty_t(long long int target, const char* pName, double pState);
    int getFloatProperty_t(long long int target, const char* pName, double& pState) const;
    int setHandleProperty_t(long long int target, const char* pName, long long int pState);
    int getHandleProperty_t(long long int target, const char* pName, long long int& pState) const;
    int setStringProperty_t(long long int target, const char* pName, const std::string& pState);
    int getStringProperty_t(long long int target, const char* pName, std::string& pState) const;
    int setBufferProperty_t(long long int target, const char* pName, const std::string& pState);
    int getBufferProperty_t(long long int target, const char* pName, std::string& pState) const;
    int setIntArray2Property_t(long long int target, const char* pName, const int* pState);
    int getIntArray2Property_t(long long int target, const char* pName, int* pState) const;
    int setVector3Property_t(long long int target, const char* pName, const C3Vector& pState);
    int getVector3Property_t(long long int target, const char* pName, C3Vector& pState) const;
    int setMatrixProperty_t(long long int target, const char* pName, const CMatrix& pState);
    int getMatrixProperty_t(long long int target, const char* pName, CMatrix& pState) const;
    int setQuaternionProperty_t(long long int target, const char* pName, const C4Vector& pState);
    int getQuaternionProperty_t(long long int target, const char* pName, C4Vector& pState) const;
    int setPoseProperty_t(long long int target, const char* pName, const C7Vector& pState);
    int getPoseProperty_t(long long int target, const char* pName, C7Vector& pState) const;
    int setColorProperty_t(long long int target, const char* pName, const float* pState);
    int getColorProperty_t(long long int target, const char* pName, float* pState) const;
    int setFloatArrayProperty_t(long long int target, const char* pName, const std::vector<double>& pState);
    int getFloatArrayProperty_t(long long int target, const char* pName, std::vector<double>& pState) const;
    int setIntArrayProperty_t(long long int target, const char* pName, const std::vector<int>& pState);
    int getIntArrayProperty_t(long long int target, const char* pName, std::vector<int>& pState) const;
    int setHandleArrayProperty_t(long long int target, const char* pName, const std::vector<long long int>& pState);
    int getHandleArrayProperty_t(long long int target, const char* pName, std::vector<long long int>& pState) const;
    int setStringArrayProperty_t(long long int target, const char* pName, const std::vector<std::string>& pState);
    int getStringArrayProperty_t(long long int target, const char* pName, std::vector<std::string>& pState) const;
    int removeProperty_t(long long int target, const char* pName);
    int getPropertyName_t(long long int target, int& index, std::string& pName, std::string& appartenance, int excludeFlags) const;
    int getPropertyInfo_t(long long int target, const char* pName, int& info, std::string& infoTxt) const;

  protected:
    std::vector<CInterfaceStack*> _allStacks;
};
