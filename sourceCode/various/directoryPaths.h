
#pragma once

#include "vrepMainHeader.h"
#include "mainCont.h"

class CDirectoryPaths 
{
public:
    CDirectoryPaths();
    virtual ~CDirectoryPaths();

    static std::string getPathFromFull(const std::string& full); // no final backslash or slash is returned
    static std::string getNameFromFull(const std::string& full);

    std::string executableDirectory;
    std::string systemDirectory;
    std::string sceneDirectory;
    std::string modelDirectory;
    std::string uiDirectory;
    std::string cadFormatDirectory;
    std::string textureDirectory;
    std::string videoDirectory;
    std::string otherFilesDirectory;
    std::string extScriptEditorTempFileDirectory;
    std::string remoteApiFileTransferDirectory;
};
