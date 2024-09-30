#include <simInternal.h>
#include <persistentDataContainer.h>
#include <app.h>
#include <vVarious.h>

CPersistentDataContainer::CPersistentDataContainer()
{
    _eventMutex.lock();
    _filename = "settings.dat";
    initializeWithDataFromFile();
}

CPersistentDataContainer::CPersistentDataContainer(const char *filename, const char *customFolder /*=nullptr*/)
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

void CPersistentDataContainer::clearData(const char *dataName, bool toFile)
{
    writeData(dataName, "", toFile, false);
}

void CPersistentDataContainer::writeData(const char *dataName, const std::string &value, bool toFile, bool allowEmptyString)
{
    _writeData(dataName, value, allowEmptyString);
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
}

void CPersistentDataContainer::_writeData(const char *dataName, const std::string &value, bool allowEmptyString)
{
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
        for (size_t i = 0; i < propertyTypes.size(); i++)
        {
            std::string tp = propertyTypes[i].second;
            int index = _getDataIndex((tp + nakedTag).c_str());
            if (index != -1)
            { // we have to remove this data:
                _dataNames.erase(_dataNames.begin() + index);
                _dataValues.erase(_dataValues.begin() + index);
            }
        }

        if ( (value.size() != 0) || allowEmptyString)
        { // we have to add this data:
            _dataNames.push_back(dataName);
            _dataValues.push_back(value);
        }
    }
}

bool CPersistentDataContainer::readData(const char *dataName, std::string &value)
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

bool CPersistentDataContainer::getPropertyName(int& index, std::string& pName)
{
    bool retVal = false;
    for (size_t i = 0; i < _dataNames.size(); i++)
    {
        index--;
        if (index == -1)
        {
            pName = _dataNames[i].c_str();
            size_t p = pName.find("&.");
            if (p != std::string::npos)
                pName.erase(0, p + 2);
            retVal = true;
            break;
        }
    }
    return retVal;
}

int CPersistentDataContainer::getAllDataNames(std::vector<std::string> &names)
{
    names.assign(_dataNames.begin(), _dataNames.end());
    return (int(names.size()));
}

int CPersistentDataContainer::_getDataIndex(const char *dataName)
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

void CPersistentDataContainer::_readFromFile(std::vector<std::string> &dataNames, std::vector<std::string> &dataValues)
{
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
}

void CPersistentDataContainer::_writeToFile(std::vector<std::string> &dataNames, std::vector<std::string> &dataValues)
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

void CPersistentDataContainer::_serialize(VArchive &ar, std::vector<std::string> &dataNames,
                                          std::vector<std::string> &dataValues)
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
