#include <customProperties.h>
#include <fstream>
#include <cstring>
#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <utils.h>

CCustomProperties::CCustomProperties()
{
}

CCustomProperties::~CCustomProperties()
{
}

void CCustomProperties::clear()
{
    _properties.clear();
}

size_t CCustomProperties::getPropertyCount() const
{
    return _properties.size();
}

void CCustomProperties::getAllPropertyData(std::vector<std::string>& names, std::vector<std::string>& data) const
{
    for (auto it = _properties.begin(); it != _properties.end(); ++it )
    {
        names.push_back(it->first);
        data.push_back(it->second);
    }
}

void CCustomProperties::setAllPropertyData(const std::vector<std::string>& names, const std::vector<std::string>& data)
{
    clear();
    for (size_t i = 0; i < names.size(); i++)
        _properties.insert({names[i], data[i]});
}

void CCustomProperties::copyFromExceptMethods(const CCustomProperties* source)
{
    _properties = source->_properties;
    std::string pName, appart;
    int index = 0;
    int ind = index;
    std::vector<std::string> toRemove;
    while (getPropertyName(ind, pName, appart, 0) == sim_propertyret_ok)
    {
        int info;
        std::string infoTxt;
        if (getPropertyInfo(pName.c_str(), info, infoTxt) == sim_propertytype_method)
            toRemove.push_back(pName);
        ind = ++index;
        pName.clear();
    }
    for (size_t i = 0; i < toRemove.size(); i++)
    {
        setPropertyInfo(toRemove[i].c_str(), sim_propertyinfo_removable, "");
        removeProperty(toRemove[i].c_str());
    }
}

void CCustomProperties::_writeInt32(std::string& buf, size_t offset, int32_t val)
{
    unsigned char b[4];
    b[0] = (unsigned char)(val & 0xFF);
    b[1] = (unsigned char)((val >> 8) & 0xFF);
    b[2] = (unsigned char)((val >> 16) & 0xFF);
    b[3] = (unsigned char)((val >> 24) & 0xFF);
    buf[offset + 0] = b[0];
    buf[offset + 1] = b[1];
    buf[offset + 2] = b[2];
    buf[offset + 3] = b[3];
}

int32_t CCustomProperties::_readInt32(const std::string& buf, size_t offset)
{
    unsigned char b0 = (unsigned char)buf[offset + 0];
    unsigned char b1 = (unsigned char)buf[offset + 1];
    unsigned char b2 = (unsigned char)buf[offset + 2];
    unsigned char b3 = (unsigned char)buf[offset + 3];
    return (int32_t)(b0 | (b1 << 8) | (b2 << 16) | (b3 << 24));
}

std::string CCustomProperties::_packHeader(int propType, int propInfo, const std::string& infoTxt)
{
    // Header: type(4) + info(4) + infoTxtLen(4) + infoTxt(N)
    size_t headerSize = 12 + infoTxt.size();
    std::string header(headerSize, '\0');
    _writeInt32(header, 0, (int32_t)propType);
    _writeInt32(header, 4, (int32_t)propInfo);
    _writeInt32(header, 8, (int32_t)infoTxt.size());
    if (!infoTxt.empty())
        std::memcpy(&header[12], infoTxt.data(), infoTxt.size());
    return header;
}

bool CCustomProperties::_unpackHeader(const std::string& buffer, int& propType, int& propInfo, std::string& infoTxt, size_t& dataOffset)
{
    if (buffer.size() < 12)
        return false;
    propType = _readInt32(buffer, 0);
    propInfo = _readInt32(buffer, 4);
    int32_t infoTxtLen = _readInt32(buffer, 8);
    if (infoTxtLen < 0 || buffer.size() < (size_t)(12 + infoTxtLen))
        return false;
    infoTxt.assign(buffer.data() + 12, (size_t)infoTxtLen);
    dataOffset = 12 + (size_t)infoTxtLen;
    return true;
}

std::string CCustomProperties::_packProperty(int propType, int propInfo, const std::string& infoTxt, const char* data, size_t dataLen)
{
    std::string header = _packHeader(propType, propInfo, infoTxt);
    std::string result;
    result.reserve(header.size() + dataLen);
    result = header;
    if (data != nullptr && dataLen > 0)
        result.append(data, dataLen);
    return result;
}

bool CCustomProperties::hasTypedProperty(const char* pName, int propType) const
{
    auto it = _properties.find(pName);
    if (it == _properties.end())
        return false;
    return (_readInt32(it->second, 0) == propType);
}

bool CCustomProperties::_findProperty(const char* pName, int& propType, int& propInfo, std::string& infoTxt, const char*& dataPtr, size_t& dataLen) const
{
    auto it = _properties.find(pName);
    if (it == _properties.end())
        return false;

    size_t dataOffset;
    if (!_unpackHeader(it->second, propType, propInfo, infoTxt, dataOffset))
        return false;

    dataPtr = it->second.data() + dataOffset;
    dataLen = it->second.size() - dataOffset;
    return true;
}

bool CCustomProperties::_hasProperty(const char* pName) const
{
    return _properties.find(pName) != _properties.end();
}

void CCustomProperties::_setPropertyRaw(const char* pName, int propType, int propInfo, const std::string& infoTxt, const char* data, size_t dataLen)
{
    _properties[pName] = _packProperty(propType, propInfo, infoTxt, data, dataLen);
}

bool CCustomProperties::_updatePropertyData(const char* pName, const char* data, size_t dataLen)
{
    auto it = _properties.find(pName);
    if (it == _properties.end())
        return false;

    int propType, propInfo;
    std::string infoTxt;
    size_t dataOffset;
    if (!_unpackHeader(it->second, propType, propInfo, infoTxt, dataOffset))
        return false;

    // Check if the data is actually different
    size_t oldDataLen = it->second.size() - dataOffset;
    const char* oldData = it->second.data() + dataOffset;
    size_t newDataLen = (data != nullptr) ? dataLen : 0;

    if (oldDataLen == newDataLen && (newDataLen == 0 || memcmp(oldData, data, newDataLen) == 0))
        return false;

    // Rebuild: keep header, replace data
    std::string newBuf;
    newBuf.reserve(dataOffset + newDataLen);
    newBuf.assign(it->second.data(), dataOffset);
    if (newDataLen > 0)
        newBuf.append(data, newDataLen);
    it->second = std::move(newBuf);
    return true;
}

int CCustomProperties::setBoolProperty(const char* pName, bool pState, bool& valueChange)
{
    int propType, propInfo;
    std::string infoTxt;
    const char* dataPtr;
    size_t dataLen;
    valueChange = false;

    bool alreadyPresent = _findProperty(pName, propType, propInfo, infoTxt, dataPtr, dataLen);
    if (alreadyPresent)
    {
        if (propInfo & sim_propertyinfo_notwritable)
            return sim_propertyret_unavailable;
        if (propType != sim_propertytype_bool)
            return sim_propertyret_unavailable;
        char val = pState ? 1 : 0;
        valueChange = _updatePropertyData(pName, &val, 1);
    }
    else
    {
        char val = pState ? 1 : 0;
        _setPropertyRaw(pName, sim_propertytype_bool, sim_propertyinfo_removable, "", &val, 1);
        valueChange = true;
    }
    return sim_propertyret_ok;
}

int CCustomProperties::getBoolProperty(const char* pName, bool& pState) const
{
    int propType, propInfo;
    std::string infoTxt;
    const char* dataPtr;
    size_t dataLen;

    if (!_findProperty(pName, propType, propInfo, infoTxt, dataPtr, dataLen))
        return sim_propertyret_unknownproperty;

    if (propInfo & sim_propertyinfo_notreadable)
        return sim_propertyret_unavailable;
    if (propType != sim_propertytype_bool)
        return sim_propertyret_unavailable;
    if (dataLen >= 1)
        pState = (dataPtr[0] != 0);
    return sim_propertyret_ok;
}

int CCustomProperties::setIntProperty(const char* pName, int pState, bool& valueChange)
{
    int propType, propInfo;
    std::string infoTxt;
    const char* dataPtr;
    size_t dataLen;
    valueChange = false;

    bool alreadyPresent = _findProperty(pName, propType, propInfo, infoTxt, dataPtr, dataLen);
    if (alreadyPresent)
    {
        if (propInfo & sim_propertyinfo_notwritable)
            return sim_propertyret_unavailable;
        if (propType != sim_propertytype_int)
            return sim_propertyret_unavailable;
        valueChange = _updatePropertyData(pName, (const char*)&pState, sizeof(int));
    }
    else
    {
        _setPropertyRaw(pName, sim_propertytype_int, sim_propertyinfo_removable, "", (const char*)&pState, sizeof(int));
        valueChange = true;
    }
    return sim_propertyret_ok;
}

int CCustomProperties::getIntProperty(const char* pName, int& pState) const
{
    int propType, propInfo;
    std::string infoTxt;
    const char* dataPtr;
    size_t dataLen;

    if (!_findProperty(pName, propType, propInfo, infoTxt, dataPtr, dataLen))
        return sim_propertyret_unknownproperty;

    if (propInfo & sim_propertyinfo_notreadable)
        return sim_propertyret_unavailable;
    if (propType != sim_propertytype_int)
        return sim_propertyret_unavailable;
    if (dataLen >= sizeof(int))
        std::memcpy(&pState, dataPtr, sizeof(int));
    return sim_propertyret_ok;
}

int CCustomProperties::setLongProperty(const char* pName, long long int pState, bool& valueChange)
{
    int propType, propInfo;
    std::string infoTxt;
    const char* dataPtr;
    size_t dataLen;
    valueChange = false;

    bool alreadyPresent = _findProperty(pName, propType, propInfo, infoTxt, dataPtr, dataLen);
    if (alreadyPresent)
    {
        if (propInfo & sim_propertyinfo_notwritable)
            return sim_propertyret_unavailable;
        if (propType != sim_propertytype_long)
            return sim_propertyret_unavailable;
        valueChange = _updatePropertyData(pName, (const char*)&pState, sizeof(long long int));
    }
    else
    {
        _setPropertyRaw(pName, sim_propertytype_long, sim_propertyinfo_removable, "", (const char*)&pState, sizeof(long long int));
        valueChange = true;
    }
    return sim_propertyret_ok;
}

int CCustomProperties::getLongProperty(const char* pName, long long int& pState) const
{
    int propType, propInfo;
    std::string infoTxt;
    const char* dataPtr;
    size_t dataLen;

    if (!_findProperty(pName, propType, propInfo, infoTxt, dataPtr, dataLen))
        return sim_propertyret_unknownproperty;

    if (propInfo & sim_propertyinfo_notreadable)
        return sim_propertyret_unavailable;
    if (propType != sim_propertytype_long)
        return sim_propertyret_unavailable;
    if (dataLen >= sizeof(long long int))
        std::memcpy(&pState, dataPtr, sizeof(long long int));
    return sim_propertyret_ok;
}

int CCustomProperties::setFloatProperty(const char* pName, double pState, bool& valueChange)
{
    int propType, propInfo;
    std::string infoTxt;
    const char* dataPtr;
    size_t dataLen;
    valueChange = false;

    bool alreadyPresent = _findProperty(pName, propType, propInfo, infoTxt, dataPtr, dataLen);
    if (alreadyPresent)
    {
        if (propInfo & sim_propertyinfo_notwritable)
            return sim_propertyret_unavailable;
        if (propType != sim_propertytype_float)
            return sim_propertyret_unavailable;
        valueChange = _updatePropertyData(pName, (const char*)&pState, sizeof(double));
    }
    else
    {
        _setPropertyRaw(pName, sim_propertytype_float, sim_propertyinfo_removable, "", (const char*)&pState, sizeof(double));
        valueChange = true;
    }
    return sim_propertyret_ok;
}

int CCustomProperties::getFloatProperty(const char* pName, double& pState) const
{
    int propType, propInfo;
    std::string infoTxt;
    const char* dataPtr;
    size_t dataLen;

    if (!_findProperty(pName, propType, propInfo, infoTxt, dataPtr, dataLen))
        return sim_propertyret_unknownproperty;

    if (propInfo & sim_propertyinfo_notreadable)
        return sim_propertyret_unavailable;
    if (propType != sim_propertytype_float)
        return sim_propertyret_unavailable;
    if (dataLen >= sizeof(double))
        std::memcpy(&pState, dataPtr, sizeof(double));
    return sim_propertyret_ok;
}

int CCustomProperties::setHandleProperty(const char* pName, long long int pState, bool& valueChange)
{
    int propType, propInfo;
    std::string infoTxt;
    const char* dataPtr;
    size_t dataLen;
    valueChange = false;

    bool alreadyPresent = _findProperty(pName, propType, propInfo, infoTxt, dataPtr, dataLen);
    if (alreadyPresent)
    {
        if (propInfo & sim_propertyinfo_notwritable)
            return sim_propertyret_unavailable;
        if (propType != sim_propertytype_handle)
            return sim_propertyret_unavailable;
        valueChange = _updatePropertyData(pName, (const char*)&pState, sizeof(long long int));
    }
    else
    {
        _setPropertyRaw(pName, sim_propertytype_handle, sim_propertyinfo_removable, "", (const char*)&pState, sizeof(long long int));
        valueChange = true;
    }
    return sim_propertyret_ok;
}

int CCustomProperties::getHandleProperty(const char* pName, long long int& pState) const
{
    int propType, propInfo;
    std::string infoTxt;
    const char* dataPtr;
    size_t dataLen;

    if (!_findProperty(pName, propType, propInfo, infoTxt, dataPtr, dataLen))
        return sim_propertyret_unknownproperty;

    if (propInfo & sim_propertyinfo_notreadable)
        return sim_propertyret_unavailable;
    if (propType != sim_propertytype_handle)
        return sim_propertyret_unavailable;
    if (dataLen >= sizeof(long long int))
        std::memcpy(&pState, dataPtr, sizeof(long long int));
    return sim_propertyret_ok;
}

int CCustomProperties::setStringProperty(const char* pName, const std::string& pState, bool& valueChange)
{
    size_t sLen = pState.size();

    int propType, propInfo;
    std::string infoTxt;
    const char* dataPtr;
    size_t dataLen;
    valueChange = false;

    bool alreadyPresent = _findProperty(pName, propType, propInfo, infoTxt, dataPtr, dataLen);
    if (alreadyPresent)
    {
        if (propInfo & sim_propertyinfo_notwritable)
            return sim_propertyret_unavailable;
        if (propType != sim_propertytype_string)
            return sim_propertyret_unavailable;
        // Store as: length(4 bytes) + string data
        std::string packed(4 + sLen, '\0');
        int32_t len32 = (int32_t)sLen;
        std::memcpy(&packed[0], &len32, 4);
        if (sLen > 0)
            std::memcpy(&packed[4], pState.c_str(), sLen);
        valueChange = _updatePropertyData(pName, packed.data(), packed.size());
    }
    else
    {
        std::string packed(4 + sLen, '\0');
        int32_t len32 = (int32_t)sLen;
        std::memcpy(&packed[0], &len32, 4);
        if (sLen > 0)
            std::memcpy(&packed[4], pState.c_str(), sLen);
        _setPropertyRaw(pName, sim_propertytype_string, sim_propertyinfo_removable, "", packed.data(), packed.size());
        valueChange = true;
    }
    return sim_propertyret_ok;
}

int CCustomProperties::getStringProperty(const char* pName, std::string& pState) const
{
    pState.clear();
    int propType, propInfo;
    std::string infoTxt;
    const char* dataPtr;
    size_t dataLen;

    if (!_findProperty(pName, propType, propInfo, infoTxt, dataPtr, dataLen))
        return sim_propertyret_unknownproperty;

    if (propInfo & sim_propertyinfo_notreadable)
        return sim_propertyret_unavailable;
    if (propType != sim_propertytype_string)
        return sim_propertyret_unavailable;
    if (dataLen >= 4)
    {
        int32_t sLen;
        std::memcpy(&sLen, dataPtr, 4);
        if (sLen > 0 && dataLen >= (size_t)(4 + sLen))
            pState.assign(dataPtr + 4, (size_t)sLen);
    }
    return sim_propertyret_ok;
}

int CCustomProperties::setTableProperty(const char* pName, const std::string& pState, bool& valueChange)
{
    int propType, propInfo;
    std::string infoTxt;
    const char* dataPtr;
    size_t dataLen;
    valueChange = false;

    // Store as: length(4 bytes) + buffer data
    std::string packed(4 + pState.size(), '\0');
    int32_t len32 = (int32_t)pState.size();
    std::memcpy(&packed[0], &len32, 4);
    if (pState.size() > 0)
        std::memcpy(&packed[4], pState.data(), pState.size());

    bool alreadyPresent = _findProperty(pName, propType, propInfo, infoTxt, dataPtr, dataLen);
    if (alreadyPresent)
    {
        if (propInfo & sim_propertyinfo_notwritable)
            return sim_propertyret_unavailable;
        if (propType != sim_propertytype_table)
            return sim_propertyret_unavailable;
        valueChange = _updatePropertyData(pName, packed.data(), packed.size());
    }
    else
    {
        _setPropertyRaw(pName, sim_propertytype_table, sim_propertyinfo_removable, "", packed.data(), packed.size());
        valueChange = true;
    }
    return sim_propertyret_ok;
}

int CCustomProperties::getTableProperty(const char* pName, std::string& pState) const
{
    pState.clear();
    int propType, propInfo;
    std::string infoTxt;
    const char* dataPtr;
    size_t dataLen;

    if (!_findProperty(pName, propType, propInfo, infoTxt, dataPtr, dataLen))
        return sim_propertyret_unknownproperty;

    if (propInfo & sim_propertyinfo_notreadable)
        return sim_propertyret_unavailable;
    if (propType != sim_propertytype_table)
        return sim_propertyret_unavailable;
    if (dataLen >= 4)
    {
        int32_t bLen;
        std::memcpy(&bLen, dataPtr, 4);
        if (bLen > 0 && dataLen >= (size_t)(4 + bLen))
            pState.assign(dataPtr + 4, (size_t)bLen);
    }
    return sim_propertyret_ok;
}

int CCustomProperties::setBufferProperty(const char* pName, const std::string& pState, bool& valueChange)
{
    int propType, propInfo;
    std::string infoTxt;
    const char* dataPtr;
    size_t dataLen;
    valueChange = false;

           // Store as: length(4 bytes) + buffer data
    std::string packed(4 + pState.size(), '\0');
    int32_t len32 = (int32_t)pState.size();
    std::memcpy(&packed[0], &len32, 4);
    if (pState.size() > 0)
        std::memcpy(&packed[4], pState.data(), pState.size());

    bool alreadyPresent = _findProperty(pName, propType, propInfo, infoTxt, dataPtr, dataLen);
    if (alreadyPresent)
    {
        if (propInfo & sim_propertyinfo_notwritable)
            return sim_propertyret_unavailable;
        if (propType != sim_propertytype_buffer)
            return sim_propertyret_unavailable;
        valueChange = _updatePropertyData(pName, packed.data(), packed.size());
    }
    else
    {
        _setPropertyRaw(pName, sim_propertytype_buffer, sim_propertyinfo_removable, "", packed.data(), packed.size());
        valueChange = true;
    }
    return sim_propertyret_ok;
}

int CCustomProperties::getBufferProperty(const char* pName, std::string& pState) const
{
    pState.clear();
    int propType, propInfo;
    std::string infoTxt;
    const char* dataPtr;
    size_t dataLen;

    if (!_findProperty(pName, propType, propInfo, infoTxt, dataPtr, dataLen))
        return sim_propertyret_unknownproperty;

    if (propInfo & sim_propertyinfo_notreadable)
        return sim_propertyret_unavailable;
    if (propType != sim_propertytype_buffer)
        return sim_propertyret_unavailable;
    if (dataLen >= 4)
    {
        int32_t bLen;
        std::memcpy(&bLen, dataPtr, 4);
        if (bLen > 0 && dataLen >= (size_t)(4 + bLen))
            pState.assign(dataPtr + 4, (size_t)bLen);
    }
    return sim_propertyret_ok;
}

int CCustomProperties::setIntArray2Property(const char* pName, const int* pState, bool& valueChange)
{
    int propType, propInfo;
    std::string infoTxt;
    const char* dataPtr;
    size_t dataLen;
    valueChange = false;

    bool alreadyPresent = _findProperty(pName, propType, propInfo, infoTxt, dataPtr, dataLen);
    if (alreadyPresent)
    {
        if (propInfo & sim_propertyinfo_notwritable)
            return sim_propertyret_unavailable;
        if (propType != sim_propertytype_intarray2)
            return sim_propertyret_unavailable;
        valueChange = _updatePropertyData(pName, (const char*)pState, 2 * sizeof(int));
    }
    else
    {
        _setPropertyRaw(pName, sim_propertytype_intarray2, sim_propertyinfo_removable, "", (const char*)pState, 2 * sizeof(int));
        valueChange = true;
    }
    return sim_propertyret_ok;
}

int CCustomProperties::getIntArray2Property(const char* pName, int* pState) const
{
    int propType, propInfo;
    std::string infoTxt;
    const char* dataPtr;
    size_t dataLen;

    if (!_findProperty(pName, propType, propInfo, infoTxt, dataPtr, dataLen))
        return sim_propertyret_unknownproperty;

    if (propInfo & sim_propertyinfo_notreadable)
        return sim_propertyret_unavailable;
    if (propType != sim_propertytype_intarray2)
        return sim_propertyret_unavailable;
    if (dataLen >= 2 * sizeof(int))
        std::memcpy(pState, dataPtr, 2 * sizeof(int));
    return sim_propertyret_ok;
}

int CCustomProperties::setVector3Property(const char* pName, const C3Vector& pState, bool& valueChange)
{
    int propType, propInfo;
    std::string infoTxt;
    const char* dataPtr;
    size_t dataLen;
    valueChange = false;

    bool alreadyPresent = _findProperty(pName, propType, propInfo, infoTxt, dataPtr, dataLen);
    if (alreadyPresent)
    {
        if (propInfo & sim_propertyinfo_notwritable)
            return sim_propertyret_unavailable;
        if (propType != sim_propertytype_vector3)
            return sim_propertyret_unavailable;
        valueChange = _updatePropertyData(pName, (const char*)pState.data, 3 * sizeof(double));
    }
    else
    {
        _setPropertyRaw(pName, sim_propertytype_vector3, sim_propertyinfo_removable, "", (const char*)pState.data, 3 * sizeof(double));
        valueChange = true;
    }
    return sim_propertyret_ok;
}

int CCustomProperties::getVector3Property(const char* pName, C3Vector& pState) const
{
    int propType, propInfo;
    std::string infoTxt;
    const char* dataPtr;
    size_t dataLen;

    if (!_findProperty(pName, propType, propInfo, infoTxt, dataPtr, dataLen))
        return sim_propertyret_unknownproperty;

    if (propInfo & sim_propertyinfo_notreadable)
        return sim_propertyret_unavailable;
    if (propType != sim_propertytype_vector3)
        return sim_propertyret_unavailable;
    if (dataLen >= 3 * sizeof(double))
        std::memcpy(pState.data, dataPtr, 3 * sizeof(double));
    return sim_propertyret_ok;
}

int CCustomProperties::setMatrixProperty(const char* pName, const CMatrix& pState, bool& valueChange)
{
    // Store: rows(4) + cols(4) + data(rows*cols*sizeof(double))
    int32_t rows = (int32_t)pState.rows;
    int32_t cols = (int32_t)pState.cols;
    size_t dataSize = 8 + (size_t)(rows * cols) * sizeof(double);
    std::string packed(dataSize, '\0');
    std::memcpy(&packed[0], &rows, 4);
    std::memcpy(&packed[4], &cols, 4);
    if (rows * cols > 0)
        std::memcpy(&packed[8], pState.data.data(), (size_t)(rows * cols) * sizeof(double));

    int propType, propInfo;
    std::string infoTxt;
    const char* dp;
    size_t dl;
    valueChange = false;

    bool alreadyPresent = _findProperty(pName, propType, propInfo, infoTxt, dp, dl);
    if (alreadyPresent)
    {
        if (propInfo & sim_propertyinfo_notwritable)
            return sim_propertyret_unavailable;
        if (propType != sim_propertytype_matrix)
            return sim_propertyret_unavailable;
        valueChange = _updatePropertyData(pName, packed.data(), packed.size());
    }
    else
    {
        _setPropertyRaw(pName, sim_propertytype_matrix, sim_propertyinfo_removable, "", packed.data(), packed.size());
        valueChange = true;
    }
    return sim_propertyret_ok;
}

int CCustomProperties::getMatrixProperty(const char* pName, CMatrix& pState) const
{
    int propType, propInfo;
    std::string infoTxt;
    const char* dataPtr;
    size_t dataLen;

    if (!_findProperty(pName, propType, propInfo, infoTxt, dataPtr, dataLen))
        return sim_propertyret_unknownproperty;

    if (propInfo & sim_propertyinfo_notreadable)
        return sim_propertyret_unavailable;
    if (propType != sim_propertytype_matrix)
        return sim_propertyret_unavailable;
    if (dataLen >= 8)
    {
        int32_t rows, cols;
        std::memcpy(&rows, dataPtr, 4);
        std::memcpy(&cols, dataPtr + 4, 4);
        if (dataLen >= 8 + (size_t)(rows * cols) * sizeof(double))
        {
            pState.resize(rows, cols, 0.0);
            std::memcpy(pState.data.data(), dataPtr + 8, (size_t)(rows * cols) * sizeof(double));
        }
    }
    return sim_propertyret_ok;
}

int CCustomProperties::setQuaternionProperty(const char* pName, const C4Vector& pState, bool& valueChange)
{
    int propType, propInfo;
    std::string infoTxt;
    const char* dataPtr;
    size_t dataLen;
    valueChange = false;

    double dat[4];
    pState.getData(dat, true);
    bool alreadyPresent = _findProperty(pName, propType, propInfo, infoTxt, dataPtr, dataLen);
    if (alreadyPresent)
    {
        if (propInfo & sim_propertyinfo_notwritable)
            return sim_propertyret_unavailable;
        if (propType != sim_propertytype_quaternion)
            return sim_propertyret_unavailable;
        valueChange = _updatePropertyData(pName, (const char*)dat, 4 * sizeof(double));
    }
    else
    {
        _setPropertyRaw(pName, sim_propertytype_quaternion, sim_propertyinfo_removable, "", (const char*)dat, 4 * sizeof(double));
        valueChange = true;
    }
    return sim_propertyret_ok;
}

int CCustomProperties::getQuaternionProperty(const char* pName, C4Vector& pState) const
{
    int propType, propInfo;
    std::string infoTxt;
    const char* dataPtr;
    size_t dataLen;

    if (!_findProperty(pName, propType, propInfo, infoTxt, dataPtr, dataLen))
        return sim_propertyret_unknownproperty;

    if (propInfo & sim_propertyinfo_notreadable)
        return sim_propertyret_unavailable;
    if (propType != sim_propertytype_quaternion)
        return sim_propertyret_unavailable;
    if (dataLen >= 4 * sizeof(double))
        pState.setData((double*)dataPtr, true);
    return sim_propertyret_ok;
}

int CCustomProperties::setPoseProperty(const char* pName, const C7Vector& pState, bool& valueChange)
{
    int propType, propInfo;
    std::string infoTxt;
    const char* dataPtr;
    size_t dataLen;
    valueChange = false;

    bool alreadyPresent = _findProperty(pName, propType, propInfo, infoTxt, dataPtr, dataLen);
    double dat[7];
    pState.getData(dat, true);
    if (alreadyPresent)
    {
        if (propInfo & sim_propertyinfo_notwritable)
            return sim_propertyret_unavailable;
        if (propType != sim_propertytype_pose)
            return sim_propertyret_unavailable;
        valueChange = _updatePropertyData(pName, (const char*)dat, 7 * sizeof(double));
    }
    else
    {
        _setPropertyRaw(pName, sim_propertytype_pose, sim_propertyinfo_removable, "", (const char*)dat, 7 * sizeof(double));
        valueChange = true;
    }
    return sim_propertyret_ok;
}

int CCustomProperties::getPoseProperty(const char* pName, C7Vector& pState) const
{
    int propType, propInfo;
    std::string infoTxt;
    const char* dataPtr;
    size_t dataLen;

    if (!_findProperty(pName, propType, propInfo, infoTxt, dataPtr, dataLen))
        return sim_propertyret_unknownproperty;

    if (propInfo & sim_propertyinfo_notreadable)
        return sim_propertyret_unavailable;
    if (propType != sim_propertytype_pose)
        return sim_propertyret_unavailable;
    if (dataLen >= 7 * sizeof(double))
    {
        double dat[7];
        std::memcpy(dat, dataPtr, 7 * sizeof(double));
        pState.setData(dat, true);
    }
    return sim_propertyret_ok;
}

int CCustomProperties::setColorProperty(const char* pName, const float* pState, bool& valueChange)
{
    int propType, propInfo;
    std::string infoTxt;
    const char* dataPtr;
    size_t dataLen;
    valueChange = false;

    bool alreadyPresent = _findProperty(pName, propType, propInfo, infoTxt, dataPtr, dataLen);
    if (alreadyPresent)
    {
        if (propInfo & sim_propertyinfo_notwritable)
            return sim_propertyret_unavailable;
        if (propType != sim_propertytype_color)
            return sim_propertyret_unavailable;
        valueChange = _updatePropertyData(pName, (const char*)pState, 3 * sizeof(float));
    }
    else
    {
        _setPropertyRaw(pName, sim_propertytype_color, sim_propertyinfo_removable, "", (const char*)pState, 3 * sizeof(float));
        valueChange = true;
    }
    return sim_propertyret_ok;
}

int CCustomProperties::getColorProperty(const char* pName, float* pState) const
{
    int propType, propInfo;
    std::string infoTxt;
    const char* dataPtr;
    size_t dataLen;

    if (!_findProperty(pName, propType, propInfo, infoTxt, dataPtr, dataLen))
        return sim_propertyret_unknownproperty;

    if (propInfo & sim_propertyinfo_notreadable)
        return sim_propertyret_unavailable;
    if (propType != sim_propertytype_color)
        return sim_propertyret_unavailable;
    if (dataLen >= 3 * sizeof(float))
        std::memcpy(pState, dataPtr, 3 * sizeof(float));
    return sim_propertyret_ok;
}

int CCustomProperties::setFloatArrayProperty(const char* pName, const std::vector<double>& pState, bool& valueChange)
{
    std::string packed(4 + (size_t)pState.size() * sizeof(double), '\0');
    int32_t count = (int32_t)pState.size();
    std::memcpy(&packed[0], &count, 4);
    if (pState.size() > 0)
        std::memcpy(&packed[4], pState.data(), (size_t)pState.size() * sizeof(double));

    int propType, propInfo;
    std::string infoTxt;
    const char* dataPtr;
    size_t dataLen;
    valueChange = false;

    bool alreadyPresent = _findProperty(pName, propType, propInfo, infoTxt, dataPtr, dataLen);
    if (alreadyPresent)
    {
        if (propInfo & sim_propertyinfo_notwritable)
            return sim_propertyret_unavailable;
        if (propType != sim_propertytype_floatarray)
            return sim_propertyret_unavailable;
        valueChange = _updatePropertyData(pName, packed.data(), packed.size());
    }
    else
    {
        _setPropertyRaw(pName, sim_propertytype_floatarray, sim_propertyinfo_removable, "", packed.data(), packed.size());
        valueChange = true;
    }
    return sim_propertyret_ok;
}

int CCustomProperties::getFloatArrayProperty(const char* pName, std::vector<double>& pState) const
{
    pState.clear();
    int propType, propInfo;
    std::string infoTxt;
    const char* dataPtr;
    size_t dataLen;

    if (!_findProperty(pName, propType, propInfo, infoTxt, dataPtr, dataLen))
        return sim_propertyret_unknownproperty;

    if (propInfo & sim_propertyinfo_notreadable)
        return sim_propertyret_unavailable;
    if (propType != sim_propertytype_floatarray)
        return sim_propertyret_unavailable;
    if (dataLen >= 4)
    {
        int32_t count;
        std::memcpy(&count, dataPtr, 4);
        if (count > 0 && dataLen >= 4 + (size_t)count * sizeof(double))
        {
            pState.resize((size_t)count);
            std::memcpy(pState.data(), dataPtr + 4, (size_t)count * sizeof(double));
        }
    }
    return sim_propertyret_ok;
}

int CCustomProperties::setIntArrayProperty(const char* pName, const std::vector<int>& pState, bool& valueChange)
{
    std::string packed(4 + (size_t)pState.size() * sizeof(int), '\0');
    int32_t count = (int32_t)pState.size();
    std::memcpy(&packed[0], &count, 4);
    if (pState.size() > 0)
        std::memcpy(&packed[4], pState.data(), (size_t)pState.size() * sizeof(int));

    int propType, propInfo;
    std::string infoTxt;
    const char* dataPtr;
    size_t dataLen;
    valueChange = false;

    bool alreadyPresent = _findProperty(pName, propType, propInfo, infoTxt, dataPtr, dataLen);
    if (alreadyPresent)
    {
        if (propInfo & sim_propertyinfo_notwritable)
            return sim_propertyret_unavailable;
        if (propType != sim_propertytype_intarray)
            return sim_propertyret_unavailable;
        valueChange = _updatePropertyData(pName, packed.data(), packed.size());
    }
    else
    {
        _setPropertyRaw(pName, sim_propertytype_intarray, sim_propertyinfo_removable, "", packed.data(), packed.size());
        valueChange = true;

    }
    return sim_propertyret_ok;
}

int CCustomProperties::getIntArrayProperty(const char* pName, std::vector<int>& pState) const
{
    pState.clear();
    int propType, propInfo;
    std::string infoTxt;
    const char* dataPtr;
    size_t dataLen;

    if (!_findProperty(pName, propType, propInfo, infoTxt, dataPtr, dataLen))
        return sim_propertyret_unknownproperty;

    if (propInfo & sim_propertyinfo_notreadable)
        return sim_propertyret_unavailable;
    if (propType != sim_propertytype_intarray)
        return sim_propertyret_unavailable;
    if (dataLen >= 4)
    {
        int32_t count;
        std::memcpy(&count, dataPtr, 4);
        if (count > 0 && dataLen >= 4 + (size_t)count * sizeof(int))
        {
            pState.resize((size_t)count);
            std::memcpy(pState.data(), dataPtr + 4, (size_t)count * sizeof(int));
        }
    }
    return sim_propertyret_ok;
}

int CCustomProperties::setHandleArrayProperty(const char* pName, const std::vector<long long int>& pState, bool& valueChange)
{
    std::string packed(4 + (size_t)pState.size() * sizeof(long long int), '\0');
    int32_t count = (int32_t)pState.size();
    std::memcpy(&packed[0], &count, 4);
    if (pState.size() > 0)
        std::memcpy(&packed[4], pState.data(), pState.size() * sizeof(long long int));

    int propType, propInfo;
    std::string infoTxt;
    const char* dataPtr;
    size_t dataLen;
    valueChange = false;

    bool alreadyPresent = _findProperty(pName, propType, propInfo, infoTxt, dataPtr, dataLen);
    if (alreadyPresent)
    {
        if (propInfo & sim_propertyinfo_notwritable)
            return sim_propertyret_unavailable;
        if (propType != sim_propertytype_handlearray)
            return sim_propertyret_unavailable;
        valueChange = _updatePropertyData(pName, packed.data(), packed.size());
    }
    else
    {
        _setPropertyRaw(pName, sim_propertytype_handlearray, sim_propertyinfo_removable, "", packed.data(), packed.size());
        valueChange = true;
    }
    return sim_propertyret_ok;
}

int CCustomProperties::getHandleArrayProperty(const char* pName, std::vector<long long int>& pState) const
{
    pState.clear();
    int propType, propInfo;
    std::string infoTxt;
    const char* dataPtr;
    size_t dataLen;

    if (!_findProperty(pName, propType, propInfo, infoTxt, dataPtr, dataLen))
        return sim_propertyret_unknownproperty;

    if (propInfo & sim_propertyinfo_notreadable)
        return sim_propertyret_unavailable;
    if (propType != sim_propertytype_handlearray)
        return sim_propertyret_unavailable;
    if (dataLen >= 4)
    {
        int32_t count;
        std::memcpy(&count, dataPtr, 4);
        if (count > 0 && dataLen >= 4 + (size_t)count * sizeof(long long int))
        {
            pState.resize((size_t)count);
            std::memcpy(pState.data(), dataPtr + 4, (size_t)count * sizeof(long long int));
        }
    }
    return sim_propertyret_ok;
}

int CCustomProperties::setStringArrayProperty(const char* pName, const std::vector<std::string>& pState, bool& valueChange)
{
    // Pack: count(4) + for each string: len(4) + data
    size_t totalSize = 4;
    for (auto& s : pState)
        totalSize += 4 + s.size();

    std::string packed(totalSize, '\0');
    size_t offset = 0;
    int32_t count = (int32_t)pState.size();
    std::memcpy(&packed[offset], &count, 4);
    offset += 4;
    for (auto& s : pState)
    {
        int32_t sLen = (int32_t)s.size();
        std::memcpy(&packed[offset], &sLen, 4);
        offset += 4;
        if (sLen > 0)
        {
            std::memcpy(&packed[offset], s.data(), (size_t)sLen);
            offset += (size_t)sLen;
        }
    }

    int propType, propInfo;
    std::string infoTxt;
    const char* dataPtr;
    size_t dataLen;
    valueChange = false;

    bool alreadyPresent = _findProperty(pName, propType, propInfo, infoTxt, dataPtr, dataLen);
    if (alreadyPresent)
    {
        if (propInfo & sim_propertyinfo_notwritable)
            return sim_propertyret_unavailable;
        if (propType != sim_propertytype_stringarray)
            return sim_propertyret_unavailable;
        valueChange = _updatePropertyData(pName, packed.data(), packed.size());
    }
    else
    {
        _setPropertyRaw(pName, sim_propertytype_stringarray, sim_propertyinfo_removable, "", packed.data(), packed.size());
        valueChange = true;
    }
    return sim_propertyret_ok;
}

int CCustomProperties::getStringArrayProperty(const char* pName, std::vector<std::string>& pState) const
{
    pState.clear();
    int propType, propInfo;
    std::string infoTxt;
    const char* dataPtr;
    size_t dataLen;

    if (!_findProperty(pName, propType, propInfo, infoTxt, dataPtr, dataLen))
        return sim_propertyret_unknownproperty;

    if (propInfo & sim_propertyinfo_notreadable)
        return sim_propertyret_unavailable;
    if (propType != sim_propertytype_stringarray)
        return sim_propertyret_unavailable;
    if (dataLen >= 4)
    {
        int32_t count;
        std::memcpy(&count, dataPtr, 4);
        size_t offset = 4;
        for (int32_t i = 0; i < count && offset + 4 <= dataLen; i++)
        {
            int32_t sLen;
            std::memcpy(&sLen, dataPtr + offset, 4);
            offset += 4;
            if (sLen >= 0 && offset + (size_t)sLen <= dataLen)
            {
                pState.emplace_back(dataPtr + offset, (size_t)sLen);
                offset += (size_t)sLen;
            }
            else
                break;
        }
    }
    return sim_propertyret_ok;
}

int CCustomProperties::setMethodProperty(const char* pName, const void* pState, bool& valueChange)
{
    std::string packed;
    packed.append((const char*)&pState, sizeof(void*));

    int propType, propInfo;
    std::string infoTxt;
    const char* dataPtr;
    size_t dataLen;
    valueChange = false;

    bool alreadyPresent = _findProperty(pName, propType, propInfo, infoTxt, dataPtr, dataLen);
    if (alreadyPresent)
    {
        if (propInfo & sim_propertyinfo_notwritable)
            return sim_propertyret_unavailable;
        if (propType != sim_propertytype_method)
            return sim_propertyret_unavailable;
        valueChange = _updatePropertyData(pName, packed.data(), packed.size());
    }
    else
    {
        _setPropertyRaw(pName, sim_propertytype_method, sim_propertyinfo_removable, "", packed.data(), packed.size());
        valueChange = true;
    }
    return sim_propertyret_ok;
}

int CCustomProperties::getMethodProperty(const char* pName, void*& pState) const
{
    pState = nullptr;
    int propType, propInfo;
    std::string infoTxt;
    const char* dataPtr;
    size_t dataLen;

    if (!_findProperty(pName, propType, propInfo, infoTxt, dataPtr, dataLen))
        return sim_propertyret_unknownproperty;

    if (propInfo & sim_propertyinfo_notreadable)
        return sim_propertyret_unavailable;
    if (propType != sim_propertytype_method)
        return sim_propertyret_unavailable;
    if (dataLen >= sizeof(void*))
    {
        memcpy(&pState, dataPtr, sizeof(void*));
    }
    return sim_propertyret_ok;
}

int CCustomProperties::setMethodProperty(const char* pName, const std::string& pState, bool& valueChange)
{
    std::string packed;
    if (pState.size() > 0)
        packed.append(pState.data(), pState.size());

    int propType, propInfo;
    std::string infoTxt;
    const char* dataPtr;
    size_t dataLen;
    valueChange = false;

    bool alreadyPresent = _findProperty(pName, propType, propInfo, infoTxt, dataPtr, dataLen);
    if (alreadyPresent)
    {
        if (propInfo & sim_propertyinfo_notwritable)
            return sim_propertyret_unavailable;
        if (propType != sim_propertytype_method)
            return sim_propertyret_unavailable;
        valueChange = _updatePropertyData(pName, packed.data(), packed.size());
    }
    else
    {
        _setPropertyRaw(pName, sim_propertytype_method, sim_propertyinfo_removable, "", packed.data(), packed.size());
        valueChange = true;
    }
    return sim_propertyret_ok;
}

int CCustomProperties::getMethodProperty(const char* pName, std::string& pState) const
{
    pState.clear();
    int propType, propInfo;
    std::string infoTxt;
    const char* dataPtr;
    size_t dataLen;

    if (!_findProperty(pName, propType, propInfo, infoTxt, dataPtr, dataLen))
        return sim_propertyret_unknownproperty;

    if (propInfo & sim_propertyinfo_notreadable)
        return sim_propertyret_unavailable;
    if (propType != sim_propertytype_method)
        return sim_propertyret_unavailable;
    if (dataLen > 0)
    {
        pState.assign(dataPtr, dataLen);
    }
    return sim_propertyret_ok;
}


int CCustomProperties::removeProperty(const char* pName)
{
    auto it = _properties.find(pName);
    if (it == _properties.end())
        return sim_propertyret_unknownproperty;

    int propType, propInfo;
    std::string infoTxt;
    size_t dataOffset;
    if (!_unpackHeader(it->second, propType, propInfo, infoTxt, dataOffset))
        return sim_propertyret_unknownproperty;

    if (!(propInfo & sim_propertyinfo_removable))
        return sim_propertyret_unavailable;

    _properties.erase(it);
    return sim_propertyret_ok;
}

int CCustomProperties::getPropertyName(int& index, std::string& pName, std::string& appartenance, int excludeFlags) const
{
    for (auto it = _properties.begin(); it != _properties.end(); ++it)
    {
        const std::string& propName = it->first;

        int propType, propInfo;
        std::string infoTxt;
        size_t dataOffset;
        if (!_unpackHeader(it->second, propType, propInfo, infoTxt, dataOffset))
            continue;

        if ((pName.size() == 0) || utils::startsWith(propName.c_str(), pName.c_str()))
        {
            if ((propInfo & excludeFlags) == 0)
            {
                if (((excludeFlags & sim_propertyinfo_retmethodsonly) == 0) || ((propType == sim_propertytype_method) && (!boost::algorithm::ends_with(propName, SET_SUFFIX)) && (!boost::algorithm::ends_with(propName, GET_SUFFIX))))
                {
                    index--;
                    if (index == -1)
                    {
                        pName = propName;
                        return sim_propertyret_ok;
                    }
                }
            }
        }
    }
    return sim_propertyret_unknownproperty;
}

int CCustomProperties::getPropertyInfo(const char* pName, int& info, std::string& infoTxt) const
{
    int propType, propInfo;
    std::string propInfoTxt;
    const char* dataPtr;
    size_t dataLen;

    if (!_findProperty(pName, propType, propInfo, propInfoTxt, dataPtr, dataLen))
        return sim_propertyret_unknownproperty;

    info = propInfo;
    infoTxt = propInfoTxt;
    return propType;
}

int CCustomProperties::setPropertyInfo(const char* pName, int info, const char* infoTxt)
{
    int propType, propInfo;
    std::string propInfoTxt;
    const char* dataPtr;
    size_t dataLen;

    if (!_findProperty(pName, propType, propInfo, propInfoTxt, dataPtr, dataLen))
        return sim_propertyret_unknownproperty;

    std::string newBuf;

    // type (unchanged)
    int32_t t = propType;
    newBuf.append((const char*)&t, sizeof(int32_t));

    // flags
    int32_t newFlags = info;
    newBuf.append((const char*)&newFlags, sizeof(int32_t));

    // info text
    std::string newInfoTxt;
    if (infoTxt != nullptr)
        newInfoTxt = infoTxt;
    int32_t txtLen = (int32_t)newInfoTxt.size();
    newBuf.append((const char*)&txtLen, sizeof(int32_t));
    if (txtLen > 0)
        newBuf.append(newInfoTxt.data(), txtLen);

    // property data (unchanged)
    if (dataLen > 0)
        newBuf.append(dataPtr, dataLen);

    _properties[pName] = newBuf;
    return sim_propertyret_ok;
}

std::string CCustomProperties::serialize() const
{
    // Format:
    // propertyCount (int32)
    // For each property:
    //   nameLen (int32) + name bytes
    //   valueLen (int32) + value bytes (the entire packed buffer)

    std::string result;
    int32_t count = (int32_t)_properties.size();
    result.append((const char*)&count, 4);

    for (auto& kv : _properties)
    {
        int32_t nameLen = (int32_t)kv.first.size();
        result.append((const char*)&nameLen, 4);
        result.append(kv.first);

        int32_t valLen = (int32_t)kv.second.size();
        result.append((const char*)&valLen, 4);
        result.append(kv.second);
    }
    return result;
}

bool CCustomProperties::deserialize(const std::string& data)
{
    _properties.clear();

    if (data.size() < 4)
        return false;

    size_t offset = 0;
    int32_t count;
    std::memcpy(&count, data.data() + offset, 4);
    offset += 4;

    for (int32_t i = 0; i < count; i++)
    {
        if (offset + 4 > data.size())
            return false;
        int32_t nameLen;
        std::memcpy(&nameLen, data.data() + offset, 4);
        offset += 4;

        if (nameLen < 0 || offset + (size_t)nameLen > data.size())
            return false;
        std::string name(data.data() + offset, (size_t)nameLen);
        offset += (size_t)nameLen;

        if (offset + 4 > data.size())
            return false;
        int32_t valLen;
        std::memcpy(&valLen, data.data() + offset, 4);
        offset += 4;

        if (valLen < 0 || offset + (size_t)valLen > data.size())
            return false;
        std::string value(data.data() + offset, (size_t)valLen);
        offset += (size_t)valLen;

        _properties[name] = std::move(value);
    }
    return true;
}

bool CCustomProperties::saveToFile(const char* filename) const
{
    std::string data = serialize();
    std::ofstream ofs(filename, std::ios::binary);
    if (!ofs.is_open())
        return false;
    ofs.write(data.data(), (std::streamsize)data.size());
    return ofs.good();
}

bool CCustomProperties::loadFromFile(const char* filename)
{
    std::ifstream ifs(filename, std::ios::binary | std::ios::ate);
    if (!ifs.is_open())
        return false;

    std::streamsize size = ifs.tellg();
    ifs.seekg(0, std::ios::beg);

    std::string data((size_t)size, '\0');
    if (!ifs.read(&data[0], size))
        return false;

    return deserialize(data);
}
