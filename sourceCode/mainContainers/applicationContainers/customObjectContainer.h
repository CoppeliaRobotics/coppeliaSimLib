#pragma once

#include <customObject.h>
#include <ser.h>

// Class:
//         objectType = "class"
//         customClass = true
//         name = "Console"
//
// Object:
//         objectType = "Console"
//         class = <handle of corresponding class>

class CustomObjectContainer
{
  public:
    CustomObjectContainer(int target);
    virtual ~CustomObjectContainer();
    void init();

    void pushGenesisEvents() const;
    void serialize(CSer& ar);
    void saveToAppFolderIfNeeded();

    long long int getFreshHandle(bool forObject) const;
    CustomObject* getItem(long long int objectHandle) const;
    bool removeItem(long long int objectHandle);
    void announceScriptStateWillBeErased(int scriptHandle);
    void clear();

    long long int makeClass(const char* typeString, const char* objectMetaInfo);
    bool removeClass(const char* typeString);
    bool removeClass(long long int objectHandle);
    CustomObject* getClass(long long int objectHandle) const;
    CustomObject* getClass(const char* typeString) const;

    long long int makeObject(const CustomObject* classObject, bool isVolatile, int originScriptHandle);
    CustomObject* getObject(long long int objectHandle) const;
    bool removeObject(long long int objectHandle);
    void getAllObjectHandles(std::vector<long long int>& objects) const;
    void getAllClassHandles(std::vector<long long int>& classes) const;

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
    int setTableProperty_t(long long int target, const char* pName, const std::string& pState);
    int getTableProperty_t(long long int target, const char* pName, std::string& pState) const;
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
    int setMethodProperty_t(long long int target, const char* pName, const void* pState);
    int getMethodProperty_t(long long int target, const char* pName, void*& pState) const;
    int setMethodProperty_t(long long int target, const char* pName, const std::string& pState);
    int getMethodProperty_t(long long int target, const char* pName, std::string& pState) const;

    int removeProperty_t(long long int target, const char* pName);
    int getPropertyName_t(long long int target, int& index, std::string& pName, std::string& appartenance, int excludeFlags) const;
    int getPropertyInfo_t(long long int target, const char* pName, int& info, std::string& infoTxt) const;
    int setPropertyInfo_t(long long int target, const char* pName, int info, const char* infoTxt);

  protected:
    void _loadFromAppFolder();
    void _notifyObjectListChanged() const;
    void _notifyClassListChanged() const;

    int _target;
    std::map<std::string, CustomObject*> _customClasses; // className - CustomObject ) e.g. "Console" - object
    std::map<long long int, CustomObject*> _customObjects;
};
