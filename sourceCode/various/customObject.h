#pragma once

#include <obj.h>

/*
// ----------------------------------------------------------------------------------------------
#define FUNCX(name, str, v1, v2, t1, t2) extern const SProperty name;
CUSTOMOBJECT_PROPERTIES
#undef FUNCX
extern const std::vector<SProperty> allProps_customObj;
// ----------------------------------------------------------------------------------------------
*/

class CustomObject: public Obj
{
  public:
    CustomObject(long long int handle, const char* objectTypeStr, const char* objectMetaInfo, int scriptHandle);
    virtual ~CustomObject();

    virtual int getLongProperty(const char* pName, long long int& pState) const;
    virtual int getStringProperty(const char* pName, std::string& pState) const;
    virtual int getPropertyName(int& index, std::string& pName, std::string& appartenance, int excludeFlags) const;
    virtual int getPropertyInfo(const char* pName, int& info, std::string& infoTxt) const;

    int getScriptHandle() const;

  private:
    int _scriptHandle;
};
