#include <interfaceStackPose.h>

CInterfaceStackPose::CInterfaceStackPose(const double* data, bool xyzqxqyqzqwLayout)
{
    _objectType = sim_stackitem_pose;
    if (data != nullptr)
        _pose.setData(data, xyzqxqyqzqwLayout);
}

CInterfaceStackPose::~CInterfaceStackPose()
{
}

const C7Vector* CInterfaceStackPose::getValue() const
{
    return &_pose;
}

void CInterfaceStackPose::setValue(const C7Vector* p)
{
    _pose = p[0];
}

CInterfaceStackObject* CInterfaceStackPose::copyYourself() const
{
    double dat[7];
    _pose.getData(dat, true);
    CInterfaceStackPose* retVal = new CInterfaceStackPose(dat, true);
    return retVal;
}

void CInterfaceStackPose::printContent(int spaces, std::string& buffer) const
{
    for (int i = 0; i < spaces; i++)
        buffer += " ";
    buffer += "POSE: ";
    double dat[7];
    _pose.getData(dat, true);
    for (size_t i = 0; i < 7; i++)
    {
        buffer += std::to_string(dat[i]);
        if (i != 6)
            buffer += ", ";
    }
    buffer += "\n";
}

std::string CInterfaceStackPose::getObjectData(std::string& /*auxInfos*/) const
{
    std::string retVal;
    double dat[7];
    _pose.getData(dat, true);
    for (size_t i = 0; i < 7; i++)
        retVal.append(reinterpret_cast<const char*>(&dat[i]), sizeof(double));
    return retVal;
}

void CInterfaceStackPose::addCborObjectData(CCbor* cborObj) const
{
    double dat[7];
    _pose.getData(dat, true);
    cborObj->appendPose(dat);
}

unsigned int CInterfaceStackPose::createFromData(const char* data, unsigned char /*version*/, std::vector<CInterfaceStackObject*>& allCreatedObjects)
{
    allCreatedObjects.push_back(this);

    std::size_t pos = 0;
    double dat[7];
    std::memcpy(dat, data + pos, 7 * sizeof(double));
    _pose.setData(dat, true);
    return (unsigned int)(7 * sizeof(double));
}

bool CInterfaceStackPose::checkCreateFromData(const char* data, unsigned int& w, unsigned int l, unsigned char version)
{
    bool retVal = false;
    if (l >= 7 * sizeof(double))
    {
        w = (unsigned int)(7 * sizeof(double));
        retVal = l >= w;
    }
    return retVal;
}
