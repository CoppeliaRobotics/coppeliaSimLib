#pragma once

#include <exception>
#define VFILE_EXCEPTION_TYPE std::exception&
#include <QFile>
#include <QDir>
typedef QFile WFile;

class VFile
{
  public:
    VFile(const char* filename, uint16_t flags, bool dontThrow = false);
    VFile(const char* filename); // opens a Qt resource files
    virtual ~VFile();

    static void reportAndHandleFileExceptionError(VFILE_EXCEPTION_TYPE e);
    static bool createFolder(const char* pathAndName);
    static bool doesFileExist(const char* filenameAndPath);
    static bool doesFolderExist(const char* foldernameAndPath); // no final slash!
    static void eraseFile(const char* filenameAndPath);
    static int eraseFilesWithPrefix(const char* pathWithoutTerminalSlash, const char* prefix);

    quint64 getLength();
    void close();
    WFile* getFile();
    bool flush();
    std::string getPathAndFilename();

    static uint16_t CREATE_WRITE;
    static uint16_t SHARE_EXCLUSIVE;
    static uint16_t READ;
    static uint16_t SHARE_DENY_NONE;

  private:
    static bool _doesFileOrFolderExist(const char* filenameOrFoldernameAndPath, bool checkForFolder);

    std::string _pathAndFilename;
    WFile* _theFile;
};
