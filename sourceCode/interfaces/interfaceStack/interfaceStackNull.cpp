
#include <interfaceStackNull.h>

CInterfaceStackNull::CInterfaceStackNull()
{
    _objectType = sim_stackitem_null;
}

CInterfaceStackNull::~CInterfaceStackNull()
{
}

CInterfaceStackObject* CInterfaceStackNull::copyYourself() const
{
    CInterfaceStackNull* retVal = new CInterfaceStackNull();
    return (retVal);
}

void CInterfaceStackNull::printContent(int spaces, std::string& buffer) const
{
    for (int i = 0; i < spaces; i++)
        buffer += " ";
    buffer += "nullptr\n";
}

std::string CInterfaceStackNull::getObjectData(std::string& /*auxInfos*/) const
{
    return ("");
}

void CInterfaceStackNull::addCborObjectData(CCbor* cborObj) const
{
    cborObj->appendNull();
}

unsigned int CInterfaceStackNull::createFromData(const char* /*data*/, unsigned char /*version*/, std::vector<CInterfaceStackObject*>& allCreatedObjects)
{
    allCreatedObjects.push_back(this);
    return (0);
}

bool CInterfaceStackNull::checkCreateFromData(const char* data, unsigned int& w, unsigned int l, unsigned char version)
{
    w = 0;
    return (true);
}
