#include "interfaceStackInteger.h"

CInterfaceStackInteger::CInterfaceStackInteger(long long int theValue)
{
    _objectType=STACK_OBJECT_INTEGER;
    _value=theValue;
}

CInterfaceStackInteger::~CInterfaceStackInteger()
{
}

long long int CInterfaceStackInteger::getValue() const
{
    return(_value);
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
#ifdef LUA_STACK_COMPATIBILITY_MODE
    double v=(double)_value;
    char* tmp=(char*)(&v);
    for (size_t i=0;i<sizeof(v);i++)
        retVal.push_back(tmp[i]);
#else
    char* tmp=(char*)(&_value);
    for (size_t i=0;i<sizeof(_value);i++)
        retVal.push_back(tmp[i]);
#endif
    return(retVal);
}

void CInterfaceStackInteger::addCborObjectData(CCbor* cborObj) const
{
    cborObj->appendInt(_value);
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
    if (l<sizeof(long long int))
        return(false);
    w=sizeof(long long int);
    return(true);
}
