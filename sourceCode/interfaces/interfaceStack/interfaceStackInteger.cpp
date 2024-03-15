#include <interfaceStackInteger.h>

CInterfaceStackInteger::CInterfaceStackInteger(long long int theValue)
{
    _objectType = sim_stackitem_integer;
    _value = theValue;
}

CInterfaceStackInteger::~CInterfaceStackInteger()
{
}

long long int CInterfaceStackInteger::getValue() const
{
    return (_value);
}

void CInterfaceStackInteger::setValue(long long int v)
{
    _value = v;
}

CInterfaceStackObject *CInterfaceStackInteger::copyYourself() const
{
    CInterfaceStackInteger *retVal = new CInterfaceStackInteger(_value);
    return (retVal);
}

void CInterfaceStackInteger::printContent(int spaces, std::string &buffer) const
{
    for (int i = 0; i < spaces; i++)
        buffer += " ";
    buffer += "INTEGER: ";
    buffer += std::to_string(_value);
    buffer += "\n";
}

std::string CInterfaceStackInteger::getObjectData(std::string & /*auxInfos*/) const
{
    std::string retVal;
    char *tmp = (char *)(&_value);
    for (size_t i = 0; i < sizeof(_value); i++)
        retVal.push_back(tmp[i]);
    return (retVal);
}

void CInterfaceStackInteger::addCborObjectData(CCbor *cborObj) const
{
    cborObj->appendInt(_value);
}

unsigned int CInterfaceStackInteger::createFromData(const char *data, unsigned char /*version*/, std::vector<CInterfaceStackObject*> &allCreatedObjects)
{
    allCreatedObjects.push_back(this);
    char *tmp = (char *)(&_value);
    for (size_t i = 0; i < sizeof(_value); i++)
        tmp[i] = data[i];
    return (sizeof(_value));
}

bool CInterfaceStackInteger::checkCreateFromData(const char *data, unsigned int &w, unsigned int l, unsigned char version)
{
    if (l < sizeof(long long int))
        return (false);
    w = sizeof(long long int);
    return (true);
}
