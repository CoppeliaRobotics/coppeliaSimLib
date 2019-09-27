#pragma once

#include "vrepMainHeader.h"

struct SFileOrFolder
{
    std::string name;
    std::string path;
    bool isFile;
    suint64 lastWriteTime;
};

class VFileFinder  
{
public:
    VFileFinder();
    virtual ~VFileFinder();

    int searchFilesWithExtension(const std::string& pathWithoutTerminalSlash,const std::string& extension);
    int searchFolders(const std::string& pathWithoutTerminalSlash);
    int searchFilesOrFolders(const std::string& pathWithoutTerminalSlash);

    SFileOrFolder* getFoundItem(int index);

private:
    int _searchFilesOrFolders(const std::string& pathWithoutTerminalSlash,std::string extension,int mode); // mode=0 --> file, mode=1 --> folder, mode=2 --> file and folder

    std::vector<SFileOrFolder> _searchResult;
};
