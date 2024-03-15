#include <interfaceStackString.h>

CInterfaceStackString::CInterfaceStackString(const char *str)
{
    _isBuffer = false;
    _isText = true;
    _objectType = sim_stackitem_string;
    _cborCoded = false;
    if (str != nullptr)
        _value.assign(str);
}

CInterfaceStackString::CInterfaceStackString(const char *str, size_t strLength, bool isBuffer)
{
    _isText = false;
    _isBuffer = isBuffer;
    _objectType = sim_stackitem_string;
    _cborCoded = false;
    if (str != nullptr)
        _value.assign(str, str + strLength);
}

CInterfaceStackString::~CInterfaceStackString()
{
}

void CInterfaceStackString::setCborCoded(bool coded)
{
    _cborCoded = coded;
}

void CInterfaceStackString::setAuxData(unsigned char opt)
{
    _isBuffer = (opt & 1);
    _isText = (opt & 2);
}

const char *CInterfaceStackString::getValue(size_t *l) const
{
    if (l != nullptr)
        l[0] = _value.size();
    return (_value.c_str());
}

bool CInterfaceStackString::isBuffer() const
{
    return _isBuffer;
}

bool CInterfaceStackString::isText() const
{
    return _isText;
}

CInterfaceStackObject *CInterfaceStackString::copyYourself() const
{
    CInterfaceStackString *retVal = new CInterfaceStackString(_value.c_str(), _value.size(), _isBuffer);
    retVal->_isText = _isText;
    retVal->_cborCoded = _cborCoded;
    return (retVal);
}

void CInterfaceStackString::printContent(int spaces, std::string &buffer) const
{
    for (int i = 0; i < spaces; i++)
        buffer += " ";
    if (_isText)
    {
        buffer += "TEXT: " + _value;
        buffer += "\n";
    }
    else
    {
        if (_isBuffer)
            buffer += "BUFFER: <buffer data>\n";
        else
        { // a random string can also contain actual text
            if (CCbor::isText(_value.c_str(), _value.size()))
            {
                buffer += "BINARY STRING <text data>: " + _value;
                buffer += "\n";
            }
            else
                buffer += "BINARY STRING: <buffer data>\n";
        }
    }
}

std::string CInterfaceStackString::getObjectData(std::string &auxInfos) const
{
    std::string retVal;
    unsigned char bb = 0;
    if (_isBuffer)
        bb |= 1;
    if (_isText)
        bb |= 2;
    auxInfos.push_back((char)bb);
    unsigned int l = (unsigned int)_value.size();
    char *tmp = (char *)(&l);
    for (size_t i = 0; i < sizeof(l); i++)
        retVal.push_back(tmp[i]);
    for (size_t i = 0; i < l; i++)
        retVal.push_back(_value[i]);
    return (retVal);
}

void CInterfaceStackString::addCborObjectData(CCbor *cborObj) const
{
    if (_cborCoded)
        cborObj->appendRaw((const unsigned char *)_value.c_str(), _value.size());
    else
        cborObj->appendLuaString(_value, _isBuffer, _isText);
}

unsigned int CInterfaceStackString::createFromData(const char *data, unsigned char /*version*/, std::vector<CInterfaceStackObject*> &allCreatedObjects)
{
    allCreatedObjects.push_back(this);
    size_t p = 0;
    _isBuffer = false;
    _isText = false;
    unsigned int l;
    char *tmp = (char *)(&l);
    for (size_t i = 0; i < sizeof(l); i++)
        tmp[i] = data[p + i];
    for (size_t i = 0; i < l; i++)
        _value.push_back(p + data[sizeof(l) + i]);
    return (sizeof(l) + l);
}

bool CInterfaceStackString::checkCreateFromData(const char *data, unsigned int &w, unsigned int l, unsigned char version)
{
    unsigned int m;
    if (l < sizeof(m))
        return (false);
    char *tmp = (char *)(&m);
    for (size_t i = 0; i < sizeof(m); i++)
        tmp[i] = data[i];
    if (l < sizeof(m) + m)
        return (false);
    w = sizeof(m) + m;
    return (true);
}
