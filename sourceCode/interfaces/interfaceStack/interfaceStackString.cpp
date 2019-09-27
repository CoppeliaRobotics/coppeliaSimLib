
#include "interfaceStackString.h"

CInterfaceStackString::CInterfaceStackString(const char* str,int l)
{
    _objectType=STACK_OBJECT_STRING;
    if (l==0)
    {
        if (str!=nullptr)
            _value.assign(str);
    }
    else
        _value.assign(str,str+l);
}

CInterfaceStackString::~CInterfaceStackString()
{
}

const char* CInterfaceStackString::getValue(int* l) const
{
    if (l!=nullptr)
        l[0]=(int)_value.size();
    return(_value.c_str());
}

void CInterfaceStackString::setValue(const char* str,int l)
{
    _value.assign(str,str+l);
}

CInterfaceStackObject* CInterfaceStackString::copyYourself() const
{
    CInterfaceStackString* retVal=new CInterfaceStackString(_value.c_str(),(int)_value.length());
    return(retVal);
}

void CInterfaceStackString::printContent(int spaces) const
{
    for (int i=0;i<spaces;i++)
        printf(" ");
    if (std::string(_value.c_str()).size()==_value.size())
        printf("STRING: %s\n",_value.c_str());
    else
        printf("STRING: <buffer data>\n");
}

std::string CInterfaceStackString::getObjectData() const
{
    std::string retVal;
    unsigned int l=(unsigned int)_value.size();
    char* tmp=(char*)(&l);
    for (size_t i=0;i<sizeof(l);i++)
        retVal.push_back(tmp[i]);
    for (size_t i=0;i<l;i++)
        retVal.push_back(_value[i]);
    return(retVal);
}

unsigned int CInterfaceStackString::createFromData(const char* data)
{
    unsigned int l;
    char* tmp=(char*)(&l);
    for (size_t i=0;i<sizeof(l);i++)
        tmp[i]=data[i];
    for (size_t i=0;i<l;i++)
        _value.push_back(data[sizeof(l)+i]);
    return(sizeof(l)+l);
}

bool CInterfaceStackString::checkCreateFromData(const char* data,unsigned int& w,unsigned int l)
{
    unsigned int m;
    if (l<sizeof(m))
        return(false);
    char* tmp=(char*)(&m);
    for (size_t i=0;i<sizeof(m);i++)
        tmp[i]=data[i];
    if (l<sizeof(m)+m)
        return(false);
    w=sizeof(m)+m;
    return(true);
}
