#pragma once

#include <customObject.h>
#include <ser.h>

class CustomObjectContainer
{
  public:
    CustomObjectContainer(int target);
    virtual ~CustomObjectContainer();

    void pushGenesisEvents() const;
    void serialize(CSer& ar);
    void saveToAppFolderIfNeeded();

    long long int getFreshHandle(bool forObject) const;
    CustomObject* getItem(long long int objectHandle) const;
    bool removeItem(long long int objectHandle);
    void announceScriptStateWillBeErased(int scriptHandle);
    void clear();

    long long int addClass(const char* objectTypeStr, const char* objectMetaInfo, int originScriptHandle);
    bool removeClass(const char* objectTypeStr);
    bool removeClass(long long int objectHandle);
    CustomObject* getClass(long long int objectHandle) const;
    CustomObject* getClass(const char* objectTypeStr) const;

    long long int addObject(const char* objectTypeStr, bool isVolatile, int originScriptHandle);
    CustomObject* getObject(long long int objectHandle) const;
    bool removeObject(long long int objectHandle);
    void getAllObjectHandles(std::vector<long long int>& objects) const;
    void getAllClassHandles(std::vector<long long int>& classes) const;

    int setBoolProperty(long long int target, const char* pName, bool pState);
    int getBoolProperty(long long int target, const char* pName, bool& pState) const;
    int setIntProperty(long long int target, const char* pName, int pState);
    int getIntProperty(long long int target, const char* pName, int& pState) const;
    int setLongProperty(long long int target, const char* pName, long long int pState);
    int getLongProperty(long long int target, const char* pName, long long int& pState) const;
    int setFloatProperty(long long int target, const char* pName, double pState);
    int getFloatProperty(long long int target, const char* pName, double& pState) const;
    int setHandleProperty(long long int target, const char* pName, long long int pState);
    int getHandleProperty(long long int target, const char* pName, long long int& pState) const;
    int setStringProperty(long long int target, const char* pName, const std::string& pState);
    int getStringProperty(long long int target, const char* pName, std::string& pState) const;
    int setBufferProperty(long long int target, const char* pName, const std::string& pState);
    int getBufferProperty(long long int target, const char* pName, std::string& pState) const;
    int setIntArray2Property(long long int target, const char* pName, const int* pState);
    int getIntArray2Property(long long int target, const char* pName, int* pState) const;
    int setVector3Property(long long int target, const char* pName, const C3Vector& pState);
    int getVector3Property(long long int target, const char* pName, C3Vector& pState) const;
    int setMatrixProperty(long long int target, const char* pName, const CMatrix& pState);
    int getMatrixProperty(long long int target, const char* pName, CMatrix& pState) const;
    int setQuaternionProperty(long long int target, const char* pName, const C4Vector& pState);
    int getQuaternionProperty(long long int target, const char* pName, C4Vector& pState) const;
    int setPoseProperty(long long int target, const char* pName, const C7Vector& pState);
    int getPoseProperty(long long int target, const char* pName, C7Vector& pState) const;
    int setColorProperty(long long int target, const char* pName, const float* pState);
    int getColorProperty(long long int target, const char* pName, float* pState) const;
    int setFloatArrayProperty(long long int target, const char* pName, const std::vector<double>& pState);
    int getFloatArrayProperty(long long int target, const char* pName, std::vector<double>& pState) const;
    int setIntArrayProperty(long long int target, const char* pName, const std::vector<int>& pState);
    int getIntArrayProperty(long long int target, const char* pName, std::vector<int>& pState) const;
    int setHandleArrayProperty(long long int target, const char* pName, const std::vector<long long int>& pState);
    int getHandleArrayProperty(long long int target, const char* pName, std::vector<long long int>& pState) const;
    int setStringArrayProperty(long long int target, const char* pName, const std::vector<std::string>& pState);
    int getStringArrayProperty(long long int target, const char* pName, std::vector<std::string>& pState) const;
    int setMethodProperty(long long int target, const char* pName, const void* pState);
    int getMethodProperty(long long int target, const char* pName, void*& pState) const;
    int setMethodProperty(long long int target, const char* pName, const std::string& pState);
    int getMethodProperty(long long int target, const char* pName, std::string& pState) const;

    int removeProperty(long long int target, const char* pName);
    int getPropertyName(long long int target, int& index, std::string& pName, std::string& appartenance, int excludeFlags) const;
    int getPropertyInfo(long long int target, const char* pName, int& info, std::string& infoTxt) const;
    int setPropertyInfo(long long int target, const char* pName, int info, const char* infoTxt);

  protected:
    void _loadFromAppFolder();
    void _notifyObjectListChanged() const;
    void _notifyClassListChanged() const;

    int _target;
    std::map<std::string, CustomObject*> _customClasses;
    std::map<long long int, CustomObject*> _customObjects;
};
