#pragma once

#include "interfaceStackObject.h"

class CInterfaceStackInteger : public CInterfaceStackObject
{
public:
    CInterfaceStackInteger(luaWrap_lua_Integer theValue);
    virtual ~CInterfaceStackInteger();

    CInterfaceStackObject* copyYourself() const;
    void printContent(int spaces,std::string& buffer) const;
    std::string getObjectData() const;
    unsigned int createFromData(const char* data);
    static bool checkCreateFromData(const char* data,unsigned int& w,unsigned int l);

    luaWrap_lua_Integer getValue() const;
    void setValue(luaWrap_lua_Integer theValue);

protected:
    luaWrap_lua_Integer _value;
};
