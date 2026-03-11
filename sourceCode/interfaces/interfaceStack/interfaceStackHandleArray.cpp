#include <interfaceStackHandleArray.h>

CInterfaceStackHandleArray::CInterfaceStackHandleArray(const long long int* vals, size_t cnt)
{
    _objectType = sim_stackitem_handlearray;
    setValue(vals, cnt);
}

CInterfaceStackHandleArray::~CInterfaceStackHandleArray()
{
}

const long long int* CInterfaceStackHandleArray::getValue(size_t* cnt) const
{
    if (cnt != nullptr)
        cnt[0] = _handles.size();
    return _handles.data();
}

void CInterfaceStackHandleArray::setValue(const long long int* vals, size_t cnt)
{
    _handles.assign(vals, vals + cnt);
}

void CInterfaceStackHandleArray::setValue(const int* vals, size_t cnt)
{
    _handles.resize(cnt);
    for (size_t i = 0; i < cnt; i++)
        _handles[i] = vals[i];
}

CInterfaceStackObject* CInterfaceStackHandleArray::copyYourself() const
{
    CInterfaceStackHandleArray* retVal = new CInterfaceStackHandleArray(_handles.data(), _handles.size());
    return retVal;
}

void CInterfaceStackHandleArray::printContent(int spaces, std::string& buffer) const
{
    for (int i = 0; i < spaces; i++)
        buffer += " ";
    buffer += "HANDLEARRAY: ";
    for (size_t i = 0; i < _handles.size(); i++)
    {
        buffer += std::to_string(_handles[i]);
        if (i != _handles.size() - 1)
            buffer += ", ";
    }
    buffer += "\n";
}

std::string CInterfaceStackHandleArray::getObjectData(std::string& /*auxInfos*/) const
{
    std::string retVal;
    size_t c = _handles.size();
    retVal.append(reinterpret_cast<const char*>(&c), sizeof(c));
    for (size_t i = 0; i < c; i++)
        retVal.append(reinterpret_cast<const char*>(_handles.data() + i), sizeof(long long int));
    return retVal;
}

void CInterfaceStackHandleArray::addCborObjectData(CCbor* cborObj) const
{
    cborObj->appendHandleArray(_handles.data(), _handles.size());
}

unsigned int CInterfaceStackHandleArray::createFromData(const char* data, unsigned char /*version*/, std::vector<CInterfaceStackObject*>& allCreatedObjects)
{
    std::size_t pos = 0;
    size_t s;
    std::memcpy(&s, data + pos, sizeof(s));
    pos += sizeof(s);
    _handles.resize(s);
    std::memcpy(_handles.data(), data + pos, s * sizeof(long long int));
    return (unsigned int)(sizeof(s) + s * sizeof(long long int));
}

bool CInterfaceStackHandleArray::checkCreateFromData(const char* data, unsigned int& w, unsigned int l, unsigned char version)
{
    bool retVal = false;
    if (l >= sizeof(size_t) + sizeof(long long int))
    {
        size_t s;
        std::memcpy(&s, data + 0, sizeof(s));
        w = (unsigned int)(sizeof(s) + s * sizeof(long long int));
        retVal = l >= w;
    }
    return retVal;
}
