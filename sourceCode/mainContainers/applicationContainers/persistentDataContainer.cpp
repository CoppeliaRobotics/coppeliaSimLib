
#include "vrepMainHeader.h"
#include "v_rep_internal.h"
#include "persistentDataContainer.h"
#include "app.h"
#include "vVarious.h"

CPersistentDataContainer::CPersistentDataContainer()
{
    _filename="persistentData.dat";
    initializeWithDataFromFile();
}

CPersistentDataContainer::CPersistentDataContainer(const std::string& filename)
{
    _filename=filename;
    initializeWithDataFromFile();
}

CPersistentDataContainer::~CPersistentDataContainer()
{
    removeAllData();
}

int CPersistentDataContainer::removeAllData()
{
    int retVal=int(_dataNames.size());
    _dataNames.clear();
    _dataValues.clear();
    return(retVal);
}

void CPersistentDataContainer::writeData(const char* dataName,const std::string& value,bool toFile)
{
    _writeData(dataName,value);
    if (toFile)
    {
        std::vector<std::string> _dataNamesAux;
        std::vector<std::string> _dataValuesAux;
        _readFromFile(_dataNamesAux,_dataValuesAux);
        _dataNames.swap(_dataNamesAux);
        _dataValues.swap(_dataValuesAux);
        _writeData(dataName,value);
        _dataNames.swap(_dataNamesAux);
        _dataValues.swap(_dataValuesAux);
        _writeToFile(_dataNamesAux,_dataValuesAux);
    }
}

void CPersistentDataContainer::_writeData(const char* dataName,const std::string& value)
{
    if (dataName!=nullptr)
    {
        int index=_getDataIndex(dataName);
        if (index==-1)
        {
            if (strlen(dataName)!=0)
            { // we have to add this data:
                _dataNames.push_back(dataName);
                _dataValues.push_back(value);
            }
        }
        else
        { 
            if (value.length()!=0)
                _dataValues[index]=value; // we have to update this data:
            else
            { // we have to remove this data:
                _dataNames.erase(_dataNames.begin()+index);
                _dataValues.erase(_dataValues.begin()+index);
            }
        }
    }
}

bool CPersistentDataContainer::readData(const char* dataName,std::string& value)
{
    if ((dataName==nullptr)||(strlen(dataName)==0))
        return(false);
    int index=_getDataIndex(dataName);
    if (index==-1)
        return(false);
    value=_dataValues[index];
    return(true);
}

int CPersistentDataContainer::getAllDataNames(std::vector<std::string>& names)
{
    names.assign(_dataNames.begin(),_dataNames.end());
    return(int(names.size()));
}

int CPersistentDataContainer::_getDataIndex(const char* dataName)
{
    for (int i=0;i<int(_dataNames.size());i++)
    {
        if (_dataNames[i].compare(dataName)==0)
            return(i);
    }
    return(-1);
}

void CPersistentDataContainer::initializeWithDataFromFile()
{
    _readFromFile(_dataNames,_dataValues);
}


void CPersistentDataContainer::_readFromFile(std::vector<std::string>& dataNames,std::vector<std::string>& dataValues)
{
    dataNames.clear();
    dataValues.clear();
    std::string filenameAndPath(VVarious::getModulePath()+VREP_SLASH+V_REP_SYSTEM_DIRECTORY_NAME+VREP_SLASH+_filename.c_str());
    if (VFile::doesFileExist(filenameAndPath))
    {
        try
        {
            VFile file(filenameAndPath,VFile::READ|VFile::SHARE_DENY_NONE);
            VArchive archive(&file,VArchive::LOAD);
            _serialize(archive,dataNames,dataValues);
            archive.close();
            file.close();
        }
        catch(VFILE_EXCEPTION_TYPE e)
        {
            // silent error since 3/2/2012: when the system folder dowesn't exist, we don't want an error!!    VFile::reportAndHandleFileExceptionError(e);
        }
    }
}

void CPersistentDataContainer::_writeToFile(std::vector<std::string>& dataNames,std::vector<std::string>& dataValues)
{
    std::string filenameAndPath(VVarious::getModulePath()+VREP_SLASH+V_REP_SYSTEM_DIRECTORY_NAME+VREP_SLASH+_filename.c_str());
    try
    {
        VFile myFile(filenameAndPath,VFile::CREATE_WRITE|VFile::SHARE_EXCLUSIVE);
        VArchive archive(&myFile,VArchive::STORE);
        _serialize(archive,dataNames,dataValues);
        archive.close();
        myFile.close();
    }
    catch(VFILE_EXCEPTION_TYPE e)
    {
        // silent error since 3/2/2012: when the system folder dowesn't exist, we don't want an error!!    VFile::reportAndHandleFileExceptionError(e);
    }
}

void CPersistentDataContainer::_serialize(VArchive& ar,std::vector<std::string>& dataNames,std::vector<std::string>& dataValues)
{
    if (ar.isStoring())
    { // Storing
        ar << int(dataNames.size());
        for (int i=0;i<int(dataNames.size());i++)
        {
            ar << (unsigned char) (1);

            ar << int(dataNames[i].size());
            for (int j=0;j<int(dataNames[i].size());j++)
                ar << (unsigned char)(dataNames[i][j]);

            ar << int(dataValues[i].size());
            for (int j=0;j<int(dataValues[i].size());j++)
                ar << (unsigned char)(dataValues[i][j]);
        }
        ar << (unsigned char) (0);
    }
    else
    {       // Loading
        int count;
        ar >> count;
        if (count>0)
        {
            unsigned char d=1;
            while (d!=0)
            {
                ar >> d;
                if (d!=0)
                {
                    std::string dat;
                    std::string val;

                    int l;
                    unsigned char v;
                    ar >> l;
                    for (int i=0;i<l;i++)
                    {
                        ar >> v;
                        dat+=v;
                    }

                    ar >> l;
                    for (int i=0;i<l;i++)
                    {
                        ar >> v;
                        val+=v;
                    }

                    dataNames.push_back(dat);
                    dataValues.push_back(val);
                }
            }
        }
    }
}

