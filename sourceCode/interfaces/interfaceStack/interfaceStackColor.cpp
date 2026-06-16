#include <interfaceStackColor.h>

CInterfaceStackColor::CInterfaceStackColor(const float c[4])
{
    _objectType = sim_stackitem_color;
    if (c != nullptr)
        setValue(c);
}

CInterfaceStackColor::~CInterfaceStackColor()
{
}

const float* CInterfaceStackColor::getValue() const
{
    return _color;
}

void CInterfaceStackColor::setValue(const float c[4])
{
    for (size_t i = 0; i < 4; i++)
        _color[i] = c[i];
}

CInterfaceStackObject* CInterfaceStackColor::copyYourself() const
{
    CInterfaceStackColor* retVal = new CInterfaceStackColor(_color);
    return retVal;
}

void CInterfaceStackColor::fetchContent(int spaces, std::string& buffer) const
{
    for (int i = 0; i < spaces; i++)
        buffer += " ";
    buffer += "COLOR: ";
    for (size_t i = 0; i < 4; i++)
    {
        buffer += std::to_string(_color[i]);
        if (i != 2)
            buffer += ", ";
    }
    buffer += "\n";
}

std::string CInterfaceStackColor::getObjectData(std::string& /*auxInfos*/) const
{
    std::string retVal;
    for (size_t i = 0; i < 4; i++)
        retVal.append(reinterpret_cast<const char*>(&_color[i]), sizeof(float));
    return retVal;
}

void CInterfaceStackColor::addCborObjectData(CCbor* cborObj) const
{
    cborObj->appendColor(_color);
}

unsigned int CInterfaceStackColor::createFromData(const char* data, unsigned char /*version*/, std::vector<CInterfaceStackObject*>& allCreatedObjects)
{
    allCreatedObjects.push_back(this);

    std::memcpy(_color, data, 4 * sizeof(float));
    return (unsigned int)(4 * sizeof(float));
}

bool CInterfaceStackColor::checkCreateFromData(const char* data, unsigned int& w, unsigned int l, unsigned char version)
{
    bool retVal = false;
    if (l >= 4 * sizeof(float))
    {
        w = (unsigned int)(4 * sizeof(float));
        retVal = l >= w;
    }
    return retVal;
}
