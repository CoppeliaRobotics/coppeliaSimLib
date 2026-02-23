#include <simInternal.h>
#include <persistentDataContainer.h>
#include <app.h>
#include <vVarious.h>
#include <utils.h>

#define PERSISTENT_DATA_SEMAPHORE_TAG "__READING_PERSISTENT_DATA__"

CPersistentDataContainer::CPersistentDataContainer()
{
    _eventMutex.lock();
    _filename = "settings.dat";
    initializeWithDataFromFile();
}

CPersistentDataContainer::CPersistentDataContainer(const char* filename, const char* customFolder /*=nullptr*/)
{ // with filename == "" or filename == nullptr,  we we do not read/write to disk
    _eventMutex.lock();
    if (filename != nullptr)
        _filename = filename;
    if (customFolder != nullptr)
        _customFolder = customFolder; // otherwise user settings folder
    if (_filename.size() > 0)
        initializeWithDataFromFile();
}

CPersistentDataContainer::~CPersistentDataContainer()
{
    removeAllData();
    _eventMutex.unlock();
}

int CPersistentDataContainer::removeAllData()
{
    int retVal = int(_dataNames.size());
    _dataNames.clear();
    _dataValues.clear();
    return (retVal);
}

bool CPersistentDataContainer::clearData(const char* dataName, bool toFile)
{
    return writeData(dataName, "", toFile, false);
}

bool CPersistentDataContainer::writeData(const char* dataName, const std::string& value, bool toFile, bool allowEmptyString)
{
    App::systemSemaphore(PERSISTENT_DATA_SEMAPHORE_TAG, true);

    bool diff = _writeData(dataName, value, allowEmptyString);
    if ((_filename.size() > 0) && toFile)
    {
        std::vector<std::string> _dataNamesAux;
        std::vector<std::string> _dataValuesAux;
        _readFromFile(_dataNamesAux, _dataValuesAux);
        _dataNames.swap(_dataNamesAux);
        _dataValues.swap(_dataValuesAux);
        _writeData(dataName, value, allowEmptyString);
        _dataNames.swap(_dataNamesAux);
        _dataValues.swap(_dataValuesAux);
        _writeToFile(_dataNamesAux, _dataValuesAux);
    }

    App::systemSemaphore(PERSISTENT_DATA_SEMAPHORE_TAG, false);

    return diff;
}

bool CPersistentDataContainer::_writeData(const char* dataName, const std::string& value, bool allowEmptyString)
{
    bool diff = false;
    if (dataName != nullptr)
    {
        // since we now have typed data, make sure to first clear all same-name data of all types:
        std::string nakedTag(dataName);
        std::string currentTp;
        size_t p = nakedTag.find("&.");
        if (p != std::string::npos)
        {
            currentTp = std::string(dataName, dataName + p + 2);
            nakedTag.erase(0, p + 2);
        }
        std::string oldValue("§.23blabliblo#@123");
        bool removed = false;
        for (size_t i = 0; i < propertyTypes.size(); i++)
        {
            std::string tp = propertyTypes[i].second;
            int index = _getDataIndex((tp + nakedTag).c_str());
            if (index != -1)
            { // we have to remove this data:
                removed = true;
                oldValue = _dataValues[index];
                _dataNames.erase(_dataNames.begin() + index);
                _dataValues.erase(_dataValues.begin() + index);
            }
        }

        if ((value.size() != 0) || allowEmptyString)
        { // we have to add this data:
            _dataNames.push_back(dataName);
            _dataValues.push_back(value);
            diff = (oldValue != value);
        }
        else
            diff = removed;
    }
    return diff;
}

bool CPersistentDataContainer::readData(const char* dataName, std::string& value)
{
    if ((dataName == nullptr) || (strlen(dataName) == 0))
        return (false);
    int index = _getDataIndex(dataName);
    if (index == -1)
        return (false);
    value = _dataValues[index];
    return (true);
}

int CPersistentDataContainer::hasData(const char* dataName, bool checkAllTypes, int* dataSize /*= nullptr*/)
{ // returns its type, or -1 if not present
    int retVal = -1;

    if (checkAllTypes)
    {
        for (size_t j = 0; j < propertyTypes.size(); j++)
        {
            std::string tp = propertyTypes[j].second;
            tp += dataName;
            retVal = hasData(tp.c_str(), false, dataSize);
            if (retVal >= 0)
                break;
        }
    }
    else
    {
        std::string dummyVal;
        if (readData(dataName, dummyVal))
        {
            if (dataSize != nullptr)
                dataSize[0] = int(dummyVal.size());
            std::string tag(dataName);
            for (size_t j = 0; j < propertyTypes.size(); j++)
            {
                if (tag.find(propertyTypes[j].second) != std::string::npos)
                {
                    retVal = propertyTypes[j].first;
                    break;
                }
            }
        }
    }

    return retVal;
}

bool CPersistentDataContainer::getPropertyName(int& index, std::string& pName, int excludeFlagsMask) const
{
    bool retVal = false;
    for (size_t i = 0; i < _dataNames.size(); i++)
    {
        std::string nnmm(_dataNames[i]);
        if (utils::replaceSubstringEnd(nnmm, "&customData", ""))
        { // expose only the data created via property functions
            size_t p = nnmm.find("&.");
            if (p != std::string::npos)
                nnmm.erase(0, p + 2);
            if ((pName.size() == 0) || utils::startsWith((CUSTOMDATAPREFIX + nnmm).c_str(), pName.c_str()))
            {
                int flags = CUSTOMDATAFLAGS;
                if (_dataValues[i].size() > LARGE_PROPERTY_SIZE)
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
    }
    return retVal;
}

int CPersistentDataContainer::getAllDataNames(std::vector<std::string>& names)
{
    names.assign(_dataNames.begin(), _dataNames.end());
    return (int(names.size()));
}

int CPersistentDataContainer::_getDataIndex(const char* dataName)
{
    for (int i = 0; i < int(_dataNames.size()); i++)
    {
        if (_dataNames[i].compare(dataName) == 0)
            return (i);
    }
    return (-1);
}

void CPersistentDataContainer::initializeWithDataFromFile()
{
    _readFromFile(_dataNames, _dataValues);
}

void CPersistentDataContainer::_readFromFile(std::vector<std::string>& dataNames, std::vector<std::string>& dataValues)
{
    App::systemSemaphore(PERSISTENT_DATA_SEMAPHORE_TAG, true);

    dataNames.clear();
    dataValues.clear();

    if (_filename.size() > 0)
    {
        std::string filenameAndPath = CFolderSystem::getUserSettingsPath();
        if (_customFolder.size() > 0)
            filenameAndPath = _customFolder;
        filenameAndPath += "/";
        filenameAndPath += _filename;
        if (VFile::doesFileExist(filenameAndPath.c_str()))
        {
            try
            {
                VFile file(filenameAndPath.c_str(), VFile::READ | VFile::SHARE_DENY_NONE);
                VArchive archive(&file, VArchive::LOAD);
                _serialize(archive, dataNames, dataValues);
                archive.close();
                file.close();
            }
            catch (VFILE_EXCEPTION_TYPE)
            {
                // silent error since 3/2/2012: when the system folder dowesn't exist, we don't want an error!!
                // VFile::reportAndHandleFileExceptionError(e);
            }
        }
    }

    App::systemSemaphore(PERSISTENT_DATA_SEMAPHORE_TAG, false);
}

void CPersistentDataContainer::_writeToFile(std::vector<std::string>& dataNames, std::vector<std::string>& dataValues)
{
    std::string filenameAndPath = CFolderSystem::getUserSettingsPath();
    if (_customFolder.size() > 0)
        filenameAndPath = _customFolder;
    filenameAndPath += "/";
    filenameAndPath += _filename;
    try
    {
        VFile myFile(filenameAndPath.c_str(), VFile::CREATE_WRITE | VFile::SHARE_EXCLUSIVE);
        VArchive archive(&myFile, VArchive::STORE);
        _serialize(archive, dataNames, dataValues);
        archive.close();
        myFile.close();
    }
    catch (VFILE_EXCEPTION_TYPE)
    {
        // silent error since 3/2/2012: when the system folder doesn't exist, we don't want an error!!
        // VFile::reportAndHandleFileExceptionError(e);
    }
}

void CPersistentDataContainer::_serialize(VArchive& ar, std::vector<std::string>& dataNames,
                                          std::vector<std::string>& dataValues)
{
    if (ar.isStoring())
    { // Storing
        ar << int(dataNames.size());
        for (int i = 0; i < int(dataNames.size()); i++)
        {
            ar << (unsigned char)(1);

            ar << int(dataNames[i].size());
            for (int j = 0; j < int(dataNames[i].size()); j++)
                ar << (unsigned char)(dataNames[i][j]);

            ar << int(dataValues[i].size());
            for (int j = 0; j < int(dataValues[i].size()); j++)
                ar << (unsigned char)(dataValues[i][j]);
        }
        ar << (unsigned char)(0);
    }
    else
    { // Loading
        int count;
        ar >> count;
        if (count > 0)
        {
            unsigned char d = 1;
            while (d != 0)
            {
                ar >> d;
                if (d != 0)
                {
                    std::string dat;
                    std::string val;

                    int l;
                    unsigned char v;
                    ar >> l;
                    for (int i = 0; i < l; i++)
                    {
                        ar >> v;
                        dat += v;
                    }

                    ar >> l;
                    for (int i = 0; i < l; i++)
                    {
                        ar >> v;
                        val += v;
                    }

                    dataNames.push_back(dat);
                    dataValues.push_back(val);
                }
            }
        }
    }
}

void CPersistentDataContainer::appendEventData(const char* dataName, CCbor* ev, bool remove /*= false*/) const
{
    auto appendKeyItem = [&](const std::string& ttag, const std::string& dat) {
        std::string tg(ttag);
        size_t p = tg.find("&.");
        if (p != std::string::npos)
        {
            if (tg.find(proptypetag_bool) != std::string::npos)
            {
                tg.erase(0, p + 2);
                tg = CUSTOMDATAPREFIX + tg;
                ev->appendKeyBool(tg.c_str(), ((unsigned char*)dat.data())[0]);
            }
            else if (tg.find(proptypetag_int) != std::string::npos)
            {
                tg.erase(0, p + 2);
                tg = CUSTOMDATAPREFIX + tg;
                ev->appendKeyInt64(tg.c_str(), ((int*)dat.data())[0]);
            }
            else if (tg.find(proptypetag_float) != std::string::npos)
            {
                tg.erase(0, p + 2);
                tg = CUSTOMDATAPREFIX + tg;
                ev->appendKeyDouble(tg.c_str(), ((double*)dat.data())[0]);
            }
            else if (tg.find(proptypetag_string) != std::string::npos)
            {
                tg.erase(0, p + 2);
                tg = CUSTOMDATAPREFIX + tg;
                ev->appendKeyText(tg.c_str(), dat.c_str());
            }
            else if (tg.find(proptypetag_vector2) != std::string::npos)
            {
                tg.erase(0, p + 2);
                tg = CUSTOMDATAPREFIX + tg;
                ev->appendKeyDoubleArray(tg.c_str(), (double*)dat.data(), dat.size() / sizeof(double));
            }
            else if (tg.find(proptypetag_vector3) != std::string::npos)
            {
                tg.erase(0, p + 2);
                tg = CUSTOMDATAPREFIX + tg;
                ev->appendKeyDoubleArray(tg.c_str(), (double*)dat.data(), dat.size() / sizeof(double));
            }
            else if (tg.find(proptypetag_quaternion) != std::string::npos)
            {
                tg.erase(0, p + 2);
                tg = CUSTOMDATAPREFIX + tg;
                ev->appendKeyDoubleArray(tg.c_str(), (double*)dat.data(), dat.size() / sizeof(double));
            }
            else if (tg.find(proptypetag_pose) != std::string::npos)
            {
                tg.erase(0, p + 2);
                tg = CUSTOMDATAPREFIX + tg;
                ev->appendKeyDoubleArray(tg.c_str(), (double*)dat.data(), dat.size() / sizeof(double));
            }
            else if (tg.find(proptypetag_matrix3x3) != std::string::npos)
            {
                tg.erase(0, p + 2);
                tg = CUSTOMDATAPREFIX + tg;
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
                tg = CUSTOMDATAPREFIX + tg;
                ev->appendKeyDoubleArray(tg.c_str(), m, 16);
            }
            else if (tg.find(proptypetag_color) != std::string::npos)
            {
                tg.erase(0, p + 2);
                tg = CUSTOMDATAPREFIX + tg;
                ev->appendKeyFloatArray(tg.c_str(), (float*)dat.data(), dat.size() / sizeof(float));
            }
            else if (tg.find(proptypetag_floatarray) != std::string::npos)
            {
                tg.erase(0, p + 2);
                tg = CUSTOMDATAPREFIX + tg;
                ev->appendKeyDoubleArray(tg.c_str(), (double*)dat.data(), dat.size() / sizeof(double));
            }
            else if (tg.find(proptypetag_intarray) != std::string::npos)
            {
                tg.erase(0, p + 2);
                tg = CUSTOMDATAPREFIX + tg;
                ev->appendKeyInt32Array(tg.c_str(), (int*)dat.data(), dat.size() / sizeof(int));
            }
            else if (tg.find(proptypetag_intarray2) != std::string::npos)
            {
                tg.erase(0, p + 2);
                tg = CUSTOMDATAPREFIX + tg;
                ev->appendKeyInt32Array(tg.c_str(), (int*)dat.data(), dat.size() / sizeof(int));
            }
            else if (tg.find(proptypetag_long) != std::string::npos)
            {
                tg.erase(0, p + 2);
                tg = CUSTOMDATAPREFIX + tg;
                ev->appendKeyInt32Array(tg.c_str(), (int*)dat.data(), dat.size() / sizeof(long long int));
            }
            else
            {
                tg.erase(0, p + 2);
                tg = CUSTOMDATAPREFIX + tg;
                ev->appendKeyBuff(tg.c_str(), (unsigned char*)dat.data(), dat.size());
            }
        }
        else
        {
            tg = CUSTOMDATAPREFIX + tg;
            ev->appendKeyBuff(tg.c_str(), (unsigned char*)dat.data(), dat.size());
        }
    };

    if (remove)
    {
        std::string tg(dataName);
        if (utils::replaceSubstring(tg, "&customData", ""))
        { // user data set via property functions
            size_t p = tg.find("&.");
            if (p != std::string::npos)
                tg.erase(0, p + 2);
            tg = CUSTOMDATAPREFIX + tg;
            ev->appendKeyNull(tg.c_str());
        }
    }
    else
    {
        for (size_t i = 0; i < _dataNames.size(); i++)
        {
            if ((dataName == nullptr) || (_dataNames[i] == dataName))
            {
                std::string tg(_dataNames[i]);
                if (utils::replaceSubstring(tg, "&customData", ""))
                    appendKeyItem(tg, _dataValues[i]); // user data set via property functions
            }
        }
    }
}
