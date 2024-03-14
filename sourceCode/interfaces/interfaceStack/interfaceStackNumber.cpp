#include <interfaceStackNumber.h>

CInterfaceStackNumber::CInterfaceStackNumber(double theValue)
{
    _objectType = sim_stackitem_double;
    _value = theValue;
}

CInterfaceStackNumber::~CInterfaceStackNumber()
{
}

double CInterfaceStackNumber::getValue() const
{
    return (_value);
}

void CInterfaceStackNumber::setValue(long long int v)
{
    _value = (double)v;
}

void CInterfaceStackNumber::setValue(double v)
{
    _value = v;
}

CInterfaceStackObject *CInterfaceStackNumber::copyYourself() const
{
    CInterfaceStackNumber *retVal = new CInterfaceStackNumber(_value);
    return (retVal);
}

void CInterfaceStackNumber::printContent(int spaces, std::string &buffer) const
{
    for (int i = 0; i < spaces; i++)
        buffer += " ";
    buffer += "NUMBER: ";
    buffer += std::to_string(_value);
    buffer += "\n";
}

std::string CInterfaceStackNumber::getObjectData() const
{
    std::string retVal;
    char *tmp = (char *)(&_value);
    for (size_t i = 0; i < sizeof(_value); i++)
        retVal.push_back(tmp[i]);
    return (retVal);
}

void CInterfaceStackNumber::addCborObjectData(CCbor *cborObj) const
{
    cborObj->appendDouble(_value);
}

unsigned int CInterfaceStackNumber::createFromData(const char *data, unsigned char /*version*/)
{
    char *tmp = (char *)(&_value);
    for (size_t i = 0; i < sizeof(_value); i++)
        tmp[i] = data[i];
    return (sizeof(_value));
}

bool CInterfaceStackNumber::checkCreateFromData(const char *data, unsigned int &w, unsigned int l, unsigned char version)
{
    if (l < sizeof(double))
        return (false);
    w = sizeof(double);
    return (true);
}
