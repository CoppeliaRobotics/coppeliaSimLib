
#include "interfaceStackBool.h"

CInterfaceStackBool::CInterfaceStackBool(bool theValue)
{
    _objectType=STACK_OBJECT_BOOL;
    _value=theValue;
}

CInterfaceStackBool::~CInterfaceStackBool()
{
}

bool CInterfaceStackBool::getValue() const
{
    return(_value);
}

CInterfaceStackObject* CInterfaceStackBool::copyYourself() const
{
    CInterfaceStackBool* retVal=new CInterfaceStackBool(_value);
    return(retVal);
}

void CInterfaceStackBool::printContent(int spaces,std::string& buffer) const
{
    for (int i=0;i<spaces;i++)
        buffer+=" ";
    if (_value)
        buffer+="BOOL: true\n";
    else
        buffer+="BOOL: false\n";
}

std::string CInterfaceStackBool::getObjectData() const
{
    std::string retVal("a");
    if (_value)
        retVal[0]=1;
    else
        retVal[0]=0;
    return(retVal);
}

void CInterfaceStackBool::getCborObjectData(CCbor* cborObj) const
{
    cborObj->appendBool(_value);
}

unsigned int CInterfaceStackBool::createFromData(const char* data)
{
    _value=(data[0]!=0);
    return(1);
}

bool CInterfaceStackBool::checkCreateFromData(const char* data,unsigned int& w,unsigned int l)
{
    if (l<1)
        return(false);
    w=1;
    return(true);
}
