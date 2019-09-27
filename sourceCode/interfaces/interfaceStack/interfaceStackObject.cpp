
#include "interfaceStackObject.h"
#include "interfaceStackNull.h"
#include "interfaceStackNumber.h"
#include "interfaceStackBool.h"
#include "interfaceStackString.h"
#include "interfaceStackTable.h"

CInterfaceStackObject::CInterfaceStackObject()
{
}

CInterfaceStackObject::~CInterfaceStackObject()
{
}

int CInterfaceStackObject::getObjectType() const
{
    return(_objectType);
}

void CInterfaceStackObject::printContent(int spaces) const
{
}

CInterfaceStackObject* CInterfaceStackObject::copyYourself() const
{
    return(nullptr);
}

std::string CInterfaceStackObject::getObjectData() const
{
    return("");
}

unsigned int CInterfaceStackObject::createFromData(const char* data)
{
    return(0);
}

CInterfaceStackObject* CInterfaceStackObject::createFromDataStatic(const char* data,unsigned int& retOffset)
{
    CInterfaceStackObject* obj=nullptr;
    char t=data[0];
    retOffset=1;
    if (t==STACK_OBJECT_NULL)
        obj=new CInterfaceStackNull();
    if (t==STACK_OBJECT_NUMBER)
    {
        obj=new CInterfaceStackNumber(0.0);
        retOffset+=obj->createFromData(data+retOffset);
    }
    if (t==STACK_OBJECT_BOOL)
    {
        obj=new CInterfaceStackBool(false);
        retOffset+=obj->createFromData(data+retOffset);
    }
    if (t==STACK_OBJECT_STRING)
    {
        obj=new CInterfaceStackString(nullptr,0);
        retOffset+=obj->createFromData(data+retOffset);
    }
    if (t==STACK_OBJECT_TABLE)
    {
        obj=new CInterfaceStackTable();
        retOffset+=obj->createFromData(data+retOffset);
    }
    return(obj);
}

