#include <interfaceStackHandle.h>

CInterfaceStackHandle::CInterfaceStackHandle(long long int theValue)
{
    _objectType = sim_stackitem_handle;
    _value = theValue;
}

CInterfaceStackHandle::~CInterfaceStackHandle()
{
}

long long int CInterfaceStackHandle::getValue() const
{
    return _value;
}

void CInterfaceStackHandle::setValue(long long int v)
{
    _value = v;
}

CInterfaceStackObject* CInterfaceStackHandle::copyYourself() const
{
    CInterfaceStackHandle* retVal = new CInterfaceStackHandle(_value);
    return retVal;
}

void CInterfaceStackHandle::printContent(int spaces, std::string& buffer) const
{
    for (int i = 0; i < spaces; i++)
        buffer += " ";
    buffer += "HANDLE: ";
    buffer += std::to_string(_value);
    buffer += "\n";
}

std::string CInterfaceStackHandle::getObjectData(std::string& /*auxInfos*/) const
{
    std::string retVal;
    char* tmp = (char*)(&_value);
    for (size_t i = 0; i < sizeof(_value); i++)
        retVal.push_back(tmp[i]);
    return retVal;
}

void CInterfaceStackHandle::addCborObjectData(CCbor* cborObj) const
{
    cborObj->appendInt(_value);
}

unsigned int CInterfaceStackHandle::createFromData(const char* data, unsigned char /*version*/, std::vector<CInterfaceStackObject*>& allCreatedObjects)
{
    allCreatedObjects.push_back(this);
    char* tmp = (char*)(&_value);
    for (size_t i = 0; i < sizeof(_value); i++)
        tmp[i] = data[i];
    return sizeof(_value);
}

bool CInterfaceStackHandle::checkCreateFromData(const char* data, unsigned int& w, unsigned int l, unsigned char version)
{
    if (l < sizeof(long long int))
        return false;
    w = sizeof(long long int);
    return true;
}
