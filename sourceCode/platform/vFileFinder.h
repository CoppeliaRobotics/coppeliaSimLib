#pragma once

#include <vector>
#include <string>

struct SFileOrFolder
{
    std::string name;
    std::string path;
    bool isFile;
    unsigned long long int lastWriteTime;
};

class VFileFinder  
{
public:
    VFileFinder();
    virtual ~VFileFinder();

    int searchFilesWithExtension(const char* pathWithoutTerminalSlash,const char* extension);
    int searchFolders(const char* pathWithoutTerminalSlash);
    int searchFilesOrFolders(const char* pathWithoutTerminalSlash);
    static int countFiles(const char* pathWithoutTerminalSlash);
    static int countFolders(const char* pathWithoutTerminalSlash);
    static int countFilesWithPrefix(const char* pathWithoutTerminalSlash,const char* prefix);

    SFileOrFolder* getFoundItem(int index);

private:
    int _searchFilesOrFolders(const char* pathWithoutTerminalSlash,const char* extension,int mode); // mode=0 --> file, mode=1 --> folder, mode=2 --> file and folder

    std::vector<SFileOrFolder> _searchResult;
};
