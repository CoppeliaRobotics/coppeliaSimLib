#include "interfaceStackInteger.h"

CInterfaceStackInteger::CInterfaceStackInteger(luaWrap_lua_Integer theValue)
{
    _objectType=STACK_OBJECT_INTEGER;
    _value=theValue;
}

CInterfaceStackInteger::~CInterfaceStackInteger()
{
}

luaWrap_lua_Integer CInterfaceStackInteger::getValue() const
{
    return(_value);
}

void CInterfaceStackInteger::setValue(luaWrap_lua_Integer theValue)
{
    _value=theValue;
}

CInterfaceStackObject* CInterfaceStackInteger::copyYourself() const
{
    CInterfaceStackInteger* retVal=new CInterfaceStackInteger(_value);
    return(retVal);
}

void CInterfaceStackInteger::printContent(int spaces,std::string& buffer) const
{
    for (int i=0;i<spaces;i++)
        buffer+=" ";
    buffer+="INTEGER: ";
    buffer+=std::to_string(_value);
    buffer+="\n";
}

std::string CInterfaceStackInteger::getObjectData() const
{
    std::string retVal;
    char* tmp=(char*)(&_value);
    for (size_t i=0;i<sizeof(_value);i++)
        retVal.push_back(tmp[i]);
    return(retVal);
}

unsigned int CInterfaceStackInteger::createFromData(const char* data)
{
    char* tmp=(char*)(&_value);
    for (size_t i=0;i<sizeof(_value);i++)
        tmp[i]=data[i];
    return(sizeof(_value));
}

bool CInterfaceStackInteger::checkCreateFromData(const char* data,unsigned int& w,unsigned int l)
{
    if (l<sizeof(luaWrap_lua_Integer))
        return(false);
    w=sizeof(luaWrap_lua_Integer);
    return(true);
}
