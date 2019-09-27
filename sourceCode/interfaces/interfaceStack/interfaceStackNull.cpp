
#include "interfaceStackNull.h"

CInterfaceStackNull::CInterfaceStackNull()
{
    _objectType=STACK_OBJECT_NULL;
}

CInterfaceStackNull::~CInterfaceStackNull()
{
}

CInterfaceStackObject* CInterfaceStackNull::copyYourself() const
{
    CInterfaceStackNull* retVal=new CInterfaceStackNull();
    return(retVal);
}

void CInterfaceStackNull::printContent(int spaces) const
{
    for (int i=0;i<spaces;i++)
        printf(" ");
    printf("nullptr\n");
}

std::string CInterfaceStackNull::getObjectData() const
{
    return("");
}

unsigned int CInterfaceStackNull::createFromData(const char* data)
{
    return(0);
}

bool CInterfaceStackNull::checkCreateFromData(const char* data,unsigned int& w,unsigned int l)
{
    w=0;
    return(true);
}
