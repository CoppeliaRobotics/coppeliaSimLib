#include <interfaceStackObject.h>
#include <interfaceStackNull.h>
#include <interfaceStackNumber.h>
#include <interfaceStackInteger.h>
#include <interfaceStackBool.h>
#include <interfaceStackString.h>
#include <interfaceStackTable.h>

CInterfaceStackObject::CInterfaceStackObject()
{
}

CInterfaceStackObject::~CInterfaceStackObject()
{
}

int CInterfaceStackObject::getObjectType() const
{
    return (_objectType);
}

void CInterfaceStackObject::printContent(int spaces, std::string &buffer) const
{
}

CInterfaceStackObject *CInterfaceStackObject::copyYourself() const
{
    return (nullptr);
}

std::string CInterfaceStackObject::getObjectData() const
{
    return ("");
}

void CInterfaceStackObject::addCborObjectData(CCbor *cborObj) const
{
}

unsigned int CInterfaceStackObject::createFromData(const char * /*data*/, const unsigned char /*version*/)
{
    return (0);
}

CInterfaceStackObject *CInterfaceStackObject::createFromDataStatic(const char *data, unsigned int &retOffset,
                                                                   const unsigned char version)
{
    CInterfaceStackObject *obj = nullptr;
    char t = data[0];
    retOffset = 1;
    if (t == sim_stackitem_null)
        obj = new CInterfaceStackNull();
    if (t == sim_stackitem_double)
    {
        obj = new CInterfaceStackNumber(0.0);
        retOffset += obj->createFromData(data + retOffset, version);
        if (version == 0)
        { // when Lua didn't have an int type yet
            double v = ((CInterfaceStackNumber *)obj)->getValue();
            long long int w = (long long int)v;
            if (v == (double)w)
            {
                delete obj;
                obj = new CInterfaceStackInteger(w);
            }
        }
    }
    if (t == sim_stackitem_integer)
    {
        obj = new CInterfaceStackInteger(0);
        retOffset += obj->createFromData(data + retOffset, version);
    }
    if (t == sim_stackitem_bool)
    {
        obj = new CInterfaceStackBool(false);
        retOffset += obj->createFromData(data + retOffset, version);
    }
    if (t == sim_stackitem_string)
    {
        obj = new CInterfaceStackString(nullptr, 0);
        retOffset += obj->createFromData(data + retOffset, version);
    }
    if (t == sim_stackitem_table)
    {
        obj = new CInterfaceStackTable();
        retOffset += obj->createFromData(data + retOffset, version);
    }
    return (obj);
}
