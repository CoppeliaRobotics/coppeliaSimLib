
#pragma once

#include "vArchive.h"

class CPersistentDataContainer 
{
public:
    CPersistentDataContainer();
    CPersistentDataContainer(const std::string& filename);
    virtual ~CPersistentDataContainer();

    int removeAllData();

    void initializeWithDataFromFile();

    void writeData(const char* dataName,const std::string& value,bool toFile);
    bool readData(const char* dataName,std::string& value);
    int getAllDataNames(std::vector<std::string>& names);

protected:
    void _writeData(const char* dataName,const std::string& value);
    int _getDataIndex(const char* dataName);

    void _readFromFile(std::vector<std::string>& dataNames,std::vector<std::string>& dataValues);
    void _writeToFile(std::vector<std::string>& dataNames,std::vector<std::string>& dataValues);
    void _serialize(VArchive& ar,std::vector<std::string>& dataNames,std::vector<std::string>& dataValues);

    std::string _filename;
    std::vector<std::string> _dataNames;
    std::vector<std::string> _dataValues;
};
