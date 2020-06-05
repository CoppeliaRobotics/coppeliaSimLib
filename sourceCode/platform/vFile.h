#pragma once

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
    VFile(const char* filename,unsigned short flags,bool dontThrow=false);
    VFile(const char* filename); // opens a Qt resource files
    virtual ~VFile();

    static void reportAndHandleFileExceptionError(VFILE_EXCEPTION_TYPE e);
    static bool createFolder(const char* pathAndName);
    static bool doesFileExist(const char* filenameAndPath);
    static bool doesFolderExist(const char* foldernameAndPath); // no final slash!
    static void eraseFile(const char* filenameAndPath);
    static int eraseFilesWithPrefix(const char* pathWithoutTerminalSlash,const char* prefix);

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
    static bool _doesFileOrFolderExist(const char* filenameOrFoldernameAndPath,bool checkForFolder);

    std::string _pathAndFilename;
    WFile* _theFile;
#ifdef SIM_WITHOUT_QT_AT_ALL
    quint64 _fileLength;
#endif
};
