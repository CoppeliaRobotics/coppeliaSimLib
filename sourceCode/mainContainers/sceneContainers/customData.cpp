#include <customData.h>
#include <app.h>
#include <base64.h>
#include <tt.h>
#include <utils.h>

CCustomData::CCustomData()
{
    _eventPrefix = CUSTOMDATAPREFIX;
}

CCustomData::~CCustomData()
{
}

void CCustomData::setItemsAreVolatile()
{
    _eventPrefix = SIGNALPREFIX;
}

std::string CCustomData::getTypeless(const char* tag)
{
    std::string d(tag);
    if ((d.size() > 0) && (d[0] == '&'))
    {
        for (size_t i = 0; i < propertyTypes.size(); i++)
        {
            std::string tp = propertyTypes[i].second;
            if ((tp.size() != 0) && (d.find(tp) == 0))
            {
                d.erase(0, tp.size());
                break;
            }
        }
    }
    return d;
}

bool CCustomData::setData(const char* tag, const char* data, size_t dataLen, bool allowEmptyData /*= true*/)
{ // non-property functions (e.g. sim.setCustomBufferData) do not allow empty data
    bool diff = false;
    if ((tag == nullptr) || (std::strlen(tag) == 0))
    {
        for (size_t i = 0; i < _data.size(); i++)
            _dataEvents[_data[i].tag] = false;
        diff = (_data.size() > 0);
        _data.clear();
    }
    else
    {
        if (allowEmptyData || (dataLen != 0))
        { // since we now have typed data, make sure that same-name data of other types is cleared beforehand:
            std::string nakedTag(tag);
            std::string currentTp;
            size_t p = nakedTag.find("&.");
            if (p != std::string::npos)
            {
                currentTp = std::string(tag, tag + p + 2);
                nakedTag.erase(0, p + 2);
            }
            for (size_t i = 0; i < propertyTypes.size(); i++)
            {
                std::string tp = propertyTypes[i].second;
                if (tp != currentTp)
                    diff = clearData((tp + nakedTag).c_str()) || diff;
            }
        }

        int f = -1;
        for (size_t i = 0; i < _data.size(); i++)
        {
            if (_data[i].tag.compare(tag) == 0)
            {
                f = int(i);
                break;
            }
        }
        if ((!allowEmptyData) && (dataLen == 0))
        { // clear (old, with non-property functions)
            if (f != -1)
            {
                _data.erase(_data.begin() + f);
                diff = true;
                _dataEvents[tag] = false;
            }
        }
        else
        { // change/add
            bool ddiff = false;
            if (f == -1)
            {
                ddiff = true;
                SCustomData dat;
                _data.push_back(dat);
                f = int(_data.size() - 1);
            }
            else
            {
                if (dataLen == _data[size_t(f)].data.size())
                {
                    for (size_t i = 0; i < dataLen; i++)
                    {
                        if (_data[size_t(f)].data[i] != data[i])
                        {
                            ddiff = true;
                            break;
                        }
                    }
                }
                else
                    ddiff = true;
            }
            _data[size_t(f)].tag = tag;
            _data[size_t(f)].data.assign(data, dataLen);
            if (ddiff)
            {
                diff = true;
                _dataEvents[tag] = true;
            }
        }
    }
    return (diff);
}

bool CCustomData::clearData(const char* tag)
{
    bool diff = false;
    for (size_t i = 0; i < _data.size(); i++)
    {
        if (_data[i].tag.compare(tag) == 0)
        {
            _data.erase(_data.begin() + i);
            _dataEvents[tag] = false;
            diff = true;
            break;
        }
    }
    return (diff);
}

int CCustomData::hasData(const char* tag, bool checkAllTypes, int* dataSize /*= nullptr*/) const
{ // returns its type, or -1 if not present
    int retVal = -1;

    if (checkAllTypes)
    {
        for (size_t j = 0; j < propertyTypes.size(); j++)
        {
            std::string tp = propertyTypes[j].second;
            tp += tag;
            retVal = hasData(tp.c_str(), false, dataSize);
            if (retVal >= 0)
                break;
        }
    }
    else
    {
        for (size_t i = 0; i < _data.size(); i++)
        {
            if (_data[i].tag.compare(tag) == 0)
            {
                for (size_t j = 0; j < propertyTypes.size(); j++)
                {
                    if (_data[i].tag.find(propertyTypes[j].second) != std::string::npos)
                    {
                        retVal = propertyTypes[j].first;
                        break;
                    }
                }
                if (dataSize != nullptr)
                    dataSize[0] = int(_data[i].data.size());
                break;
            }
        }
    }

    return retVal;
}

std::string CCustomData::getData(const char* tag) const
{
    std::string retVal;
    for (size_t i = 0; i < _data.size(); i++)
    {
        if (_data[i].tag.compare(tag) == 0)
        {
            retVal = _data[i].data;
            break;
        }
    }
    return (retVal);
}

bool CCustomData::getPropertyName(int& index, std::string& pName, int excludeFlagsMask) const
{
    bool retVal = false;
    for (size_t i = 0; i < _data.size(); i++)
    {
        std::string nnmm(_data[i].tag);
        size_t p = nnmm.find("&.");
        if (p != std::string::npos)
            nnmm.erase(0, p + 2);
        if ((pName.size() == 0) || utils::startsWith((_eventPrefix + nnmm).c_str(), pName.c_str()))
        {
            int flags;
            if (_eventPrefix == SIGNALPREFIX)
                flags = SIGNALFLAGS;
            else
                flags = CUSTOMDATAFLAGS;
            if (_data[i].data.size() > LARGE_PROPERTY_SIZE)
                flags |= sim_propertyinfo_largedata;
            if ((flags & excludeFlagsMask) == 0)
            {
                index--;
                if (index == -1)
                {
                    pName = nnmm;
                    retVal = true;
                    break;
                }
            }
        }
    }
    return retVal;
}

size_t CCustomData::getDataCount() const
{
    return (_data.size());
}

std::string CCustomData::getAllTags(size_t* cnt) const
{
    if (cnt != nullptr)
        cnt[0] = _data.size();
    std::string retVal;
    for (size_t i = 0; i < _data.size(); i++)
    {
        retVal.insert(retVal.end(), _data[i].tag.begin(), _data[i].tag.end());
        retVal += '\0';
    }
    return (retVal);
}

void CCustomData::copyYourselfInto(CCustomData& theCopy) const
{
    theCopy._data.clear();
    for (size_t i = 0; i < _data.size(); i++)
        theCopy._data.push_back(_data[i]);
    theCopy._dataEvents = _dataEvents;
}

void CCustomData::serializeData(CSer& ar, const char* objectName)
{
    if (ar.isBinary())
    {
        if (ar.isStoring())
        { // Storing
            for (size_t i = 0; i < _data.size(); i++)
            {
                ar.storeDataName("Cdt");
                ar << _data[i].tag;
                ar << int(_data[i].data.size());
                for (size_t j = 0; j < _data[i].data.size(); j++)
                    ar << _data[i].data[j];
                ar.flush();
            }
            ar.storeDataName(SER_END_OF_OBJECT);
        }
        else
        { // Loading
            int byteQuantity;
            std::string theName = "";
            while (theName.compare(SER_END_OF_OBJECT) != 0)
            {
                theName = ar.readDataName();
                if (theName.compare(SER_END_OF_OBJECT) != 0)
                {
                    bool noHit = true;
                    if (theName == "Cdt")
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        SCustomData dat;
                        ar >> dat.tag;
                        int l;
                        ar >> l;
                        dat.data.resize(size_t(l));
                        for (size_t i = 0; i < size_t(l); i++)
                            ar >> dat.data[i];
                        _data.push_back(dat);
                    }
                    if (noHit)
                        ar.loadUnknownData();
                }
            }
        }
    }
    else
    {
        if (ar.isStoring())
        {
            size_t totSize = 0;
            for (size_t i = 0; i < _data.size(); i++)
                totSize += _data[i].data.size();
            if (ar.xmlSaveDataInline(totSize))
            {
                for (size_t i = 0; i < _data.size(); i++)
                {
                    ar.xmlPushNewNode("data");
                    ar.xmlAddNode_string("tag", _data[i].tag.c_str());
                    std::string str(base64_encode((unsigned char*)_data[i].data.c_str(), _data[i].data.size()));
                    ar.xmlAddNode_string("data_base64Coded", str.c_str());
                    ar.xmlPopNode();
                }
            }
            else
            {
                CSer* serObj = nullptr;
                if (objectName != nullptr)
                    serObj = ar.xmlAddNode_binFile("file", (std::string("objectCustomData_") + objectName).c_str());
                else
                    serObj = ar.xmlAddNode_binFile("file", "sceneCustomData");
                serObj[0] << int(_data.size());
                for (size_t i = 0; i < _data.size(); i++)
                {
                    serObj[0] << _data[i].tag;
                    serObj[0] << int(_data[i].data.size());
                    for (size_t j = 0; j < _data[i].data.size(); j++)
                        serObj[0] << _data[i].data[j];
                }
                serObj->flush();
                serObj->writeClose();
                delete serObj;
            }
        }
        else
        {
            CSer* serObj = ar.xmlGetNode_binFile("file", false);
            if (serObj == nullptr)
            {
                if (ar.xmlPushChildNode("data", false))
                {
                    while (true)
                    {
                        SCustomData dat;
                        ar.xmlGetNode_string("tag", dat.tag);
                        std::string data;
                        ar.xmlGetNode_string("data_base64Coded", data);
                        data = base64_decode(data);
                        dat.data = data;
                        _data.push_back(dat);
                        if (!ar.xmlPushSiblingNode("data", false))
                            break;
                    }
                    ar.xmlPopNode();
                }
            }
            else
            {
                int s;
                serObj[0] >> s;
                for (size_t i = 0; i < size_t(s); i++)
                {
                    SCustomData dat;
                    serObj[0] >> dat.tag;
                    int l;
                    serObj[0] >> l;
                    ar >> l;
                    dat.data.resize(size_t(l));
                    for (size_t j = 0; j < size_t(l); j++)
                        ar >> dat.data[j];
                    _data.push_back(dat);
                }
                serObj->readClose();
                delete serObj;
            }
        }
    }
}

void CCustomData::appendEventData(const char* tag, CCbor* ev, bool remove /*= false*/) const
{
    auto appendKeyItem = [&](const std::string& ttag, const std::string& dat) {
        std::string tg(ttag);
        size_t p = tg.find("&.");
        if (p != std::string::npos)
        {
            if (tg.find(proptypetag_bool) != std::string::npos)
            {
                tg.erase(0, p + 2);
                tg = _eventPrefix + tg;
                ev->appendKeyBool(tg.c_str(), ((unsigned char*)dat.data())[0]);
            }
            else if (tg.find(proptypetag_int) != std::string::npos)
            {
                tg.erase(0, p + 2);
                tg = _eventPrefix + tg;
                ev->appendKeyInt(tg.c_str(), ((int*)dat.data())[0]);
            }
            else if (tg.find(proptypetag_float) != std::string::npos)
            {
                tg.erase(0, p + 2);
                tg = _eventPrefix + tg;
                ev->appendKeyDouble(tg.c_str(), ((double*)dat.data())[0]);
            }
            else if (tg.find(proptypetag_string) != std::string::npos)
            {
                tg.erase(0, p + 2);
                tg = _eventPrefix + tg;
                ev->appendKeyText(tg.c_str(), dat.c_str());
            }
            else if (tg.find(proptypetag_vector2) != std::string::npos)
            {
                tg.erase(0, p + 2);
                tg = _eventPrefix + tg;
                ev->appendKeyDoubleArray(tg.c_str(), (double*)dat.data(), dat.size() / sizeof(double));
            }
            else if (tg.find(proptypetag_vector3) != std::string::npos)
            {
                tg.erase(0, p + 2);
                tg = _eventPrefix + tg;
                ev->appendKeyDoubleArray(tg.c_str(), (double*)dat.data(), dat.size() / sizeof(double));
            }
            else if (tg.find(proptypetag_quaternion) != std::string::npos)
            {
                tg.erase(0, p + 2);
                tg = _eventPrefix + tg;
                ev->appendKeyDoubleArray(tg.c_str(), (double*)dat.data(), dat.size() / sizeof(double));
            }
            else if (tg.find(proptypetag_pose) != std::string::npos)
            {
                tg.erase(0, p + 2);
                tg = _eventPrefix + tg;
                ev->appendKeyDoubleArray(tg.c_str(), (double*)dat.data(), dat.size() / sizeof(double));
            }
            else if (tg.find(proptypetag_matrix3x3) != std::string::npos)
            {
                tg.erase(0, p + 2);
                tg = _eventPrefix + tg;
                ev->appendKeyDoubleArray(tg.c_str(), (double*)dat.data(), dat.size() / sizeof(double));
            }
            else if (tg.find(proptypetag_matrix4x4) != std::string::npos)
            {
                double m[16];
                for (size_t j = 0; j < 12; j++)
                    m[j] = ((double*)dat.data())[j];
                m[12] = 0.0;
                m[13] = 0.0;
                m[14] = 0.0;
                m[15] = 1.0;
                tg.erase(0, p + 2);
                tg = _eventPrefix + tg;
                ev->appendKeyDoubleArray(tg.c_str(), m, 16);
            }
            else if (tg.find(proptypetag_color) != std::string::npos)
            {
                tg.erase(0, p + 2);
                tg = _eventPrefix + tg;
                ev->appendKeyFloatArray(tg.c_str(), (float*)dat.data(), dat.size() / sizeof(float));
            }
            else if (tg.find(proptypetag_floatarray) != std::string::npos)
            {
                tg.erase(0, p + 2);
                tg = _eventPrefix + tg;
                ev->appendKeyDoubleArray(tg.c_str(), (double*)dat.data(), dat.size() / sizeof(double));
            }
            else if (tg.find(proptypetag_intarray) != std::string::npos)
            {
                tg.erase(0, p + 2);
                tg = _eventPrefix + tg;
                ev->appendKeyIntArray(tg.c_str(), (int*)dat.data(), dat.size() / sizeof(int));
            }
            else if (tg.find(proptypetag_intarray2) != std::string::npos)
            {
                tg.erase(0, p + 2);
                tg = _eventPrefix + tg;
                ev->appendKeyIntArray(tg.c_str(), (int*)dat.data(), dat.size() / sizeof(int));
            }
            else if (tg.find(proptypetag_long) != std::string::npos)
            {
                tg.erase(0, p + 2);
                tg = _eventPrefix + tg;
                ev->appendKeyIntArray(tg.c_str(), (int*)dat.data(), dat.size() / sizeof(long long int));
            }
            else
            {
                tg.erase(0, p + 2);
                tg = _eventPrefix + tg;
                ev->appendKeyBuff(tg.c_str(), (unsigned char*)dat.data(), dat.size());
            }
        }
        else
        {
            tg = _eventPrefix + tg;
            ev->appendKeyBuff(tg.c_str(), (unsigned char*)dat.data(), dat.size());
        }
    };

    if (remove)
    {
        std::string tg(tag);
        size_t p = tg.find("&.");
        if (p != std::string::npos)
            tg.erase(0, p + 2);
        tg = _eventPrefix + tg;
        ev->appendKeyNull(tg.c_str());
    }
    else
    {
        for (size_t i = 0; i < _data.size(); i++)
        {
            if ((tag == nullptr) || (_data[i].tag == tag))
                appendKeyItem(_data[i].tag, _data[i].data);
        }
    }
}

void CCustomData::getDataEvents(std::map<std::string, bool>& dataEvents)
{
    for (const auto& it : _dataEvents)
        dataEvents[getTypeless(it.first.c_str())] = it.second;
}

void CCustomData::clearDataEvents()
{
    _dataEvents.clear();
}

std::string CCustomData::getLegacySignalFromIndex(const char* legacyTag, int index)
{
    std::string retVal;
    for (size_t i = 0; i < _data.size(); i++)
    {
        if (_data[i].tag.find(legacyTag) != std::string::npos)
        {
            index--;
            if (index == -1)
            {
                retVal = _data[i].tag;
                break;
            }
        }
    }
    return retVal;
}
