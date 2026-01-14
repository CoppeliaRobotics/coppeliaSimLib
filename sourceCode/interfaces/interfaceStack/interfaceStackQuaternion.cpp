#include <interfaceStackQuaternion.h>

CInterfaceStackQuaternion::CInterfaceStackQuaternion(const double* data, bool xyzwLayout)
{
    _objectType = sim_stackitem_quaternion;
    if (data != nullptr)
        _quaternion.setData(data, xyzwLayout);
}

CInterfaceStackQuaternion::~CInterfaceStackQuaternion()
{
}

const C4Vector* CInterfaceStackQuaternion::getValue() const
{
    return &_quaternion;
}

void CInterfaceStackQuaternion::setValue(const C4Vector* q)
{
    _quaternion = q[0];
}

CInterfaceStackObject* CInterfaceStackQuaternion::copyYourself() const
{
    double dat[4];
    _quaternion.getData(dat, true);
    CInterfaceStackQuaternion* retVal = new CInterfaceStackQuaternion(dat, true);
    return retVal;
}

void CInterfaceStackQuaternion::printContent(int spaces, std::string& buffer) const
{
    for (int i = 0; i < spaces; i++)
        buffer += " ";
    buffer += "QUATERNION: ";
    double dat[4];
    _quaternion.getData(dat, true);
    for (size_t i = 0; i < 4; i++)
    {
        buffer += std::to_string(dat[i]);
        if (i != 3)
            buffer += ", ";
    }
    buffer += "\n";
}

std::string CInterfaceStackQuaternion::getObjectData(std::string& /*auxInfos*/) const
{
    std::string retVal;
    double dat[4];
    _quaternion.getData(dat, true);
    for (size_t i = 0; i < 4; i++)
        retVal.append(reinterpret_cast<const char*>(&dat[i]), sizeof(double));
    return retVal;
}

void CInterfaceStackQuaternion::addCborObjectData(CCbor* cborObj) const
{
    double dat[4];
    _quaternion.getData(dat, true);
    cborObj->appendQuaternion(dat);
}

unsigned int CInterfaceStackQuaternion::createFromData(const char* data, unsigned char /*version*/, std::vector<CInterfaceStackObject*>& allCreatedObjects)
{
    allCreatedObjects.push_back(this);

    std::size_t pos = 0;
    double dat[4];
    std::memcpy(dat, data + pos, 4 * sizeof(double));
    _quaternion.setData(dat, true);
    return (unsigned int)(4 * sizeof(double));
}

bool CInterfaceStackQuaternion::checkCreateFromData(const char* data, unsigned int& w, unsigned int l, unsigned char version)
{
    bool retVal = false;
    if (l >= 4 * sizeof(double))
    {
        w = (unsigned int)(4 * sizeof(double));
        retVal = l >= w;
    }
    return retVal;
}
