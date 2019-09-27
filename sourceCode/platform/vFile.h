
#pragma once

#include "vrepMainHeader.h"
#include <exception>
#define VFILE_EXCEPTION_TYPE std::exception&
#ifndef SIM_WITHOUT_QT_AT_ALL
#include <QFile>
#include <QDir>
typedef QFile WFile;
#else
#include <fstream>
typedef std::fstream WFile;
#endif

class VFile  
{
public:
    VFile(const std::string& filename,unsigned short flags,bool dontThrow=false);
    VFile(const std::string& filename); // opens a Qt resource files
    virtual ~VFile();

    static void reportAndHandleFileExceptionError(VFILE_EXCEPTION_TYPE e);
    static bool createFolder(const std::string& pathAndName);
    static bool doesFileExist(const std::string& filenameAndPath);
    static bool doesFolderExist(const std::string& foldernameAndPath); // no final slash!
    static void eraseFile(const std::string& filenameAndPath);

    quint64 getLength();
    void close();
    WFile* getFile();
    bool flush();
    std::string getPathAndFilename();

    static unsigned short CREATE_WRITE;
    static unsigned short SHARE_EXCLUSIVE;
    static unsigned short READ;
    static unsigned short SHARE_DENY_NONE;

private:
    static bool _doesFileOrFolderExist(const std::string& filenameOrFoldernameAndPath,bool checkForFolder);

    std::string _pathAndFilename;
    WFile* _theFile;
#ifdef SIM_WITHOUT_QT_AT_ALL
    quint64 _fileLength;
#endif
};
