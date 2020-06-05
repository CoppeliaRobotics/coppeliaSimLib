#include "simInternal.h"
#include "folderSystem.h"
#include "global.h"
#include "vVarious.h"
#include "app.h"
#include "simFlavor.h"

CFolderSystem::CFolderSystem()
{
    static std::string currentDir(VVarious::getModulePath());
    _executablePath=currentDir;
    _extScriptEditorTempPath=currentDir;
    _remoteApiTempPath=currentDir;
    _systemPath=_executablePath+"/"+SIM_SYSTEM_DIRECTORY_NAME;
#ifdef MAC_SIM
    // We are inside of the package!!!
    /*
    _scenesPath=_executablePath+"/../../../"+CSimFlavor::getStringVal(13);// if scenes can't be found, it will use the last used directory somehow!
    _modelsPath=_executablePath+"/../../../"+CSimFlavor::getStringVal(14);
    _cadFilesPath=_executablePath+"/../../../"+SIM_CADFILES_DIRECTORY_NAME;
    _texturesPath=_executablePath+"/../../../";
    _videosPath=_executablePath+"/../../../";
    _otherFilesPath=_executablePath+"/../../../";
    */
    _scenesPath=_executablePath+"/../Resources/"+CSimFlavor::getStringVal(13);// if scenes can't be found, it will use the last used directory somehow!
    _modelsPath=_executablePath+"/../Resources/"+CSimFlavor::getStringVal(14);
    _cadFilesPath=_executablePath+"/../Resources/"+SIM_CADFILES_DIRECTORY_NAME;
    _texturesPath=_executablePath+"/../Resources/";
    _videosPath=_executablePath+"/../Resources/";
    _otherFilesPath=_executablePath+"/../Resources/";
#else
    _scenesPath=_executablePath+"/"+CSimFlavor::getStringVal(13);// if scenes can't be found, it will use the last used directory somehow!
    _modelsPath=_executablePath+"/"+CSimFlavor::getStringVal(14);
    _cadFilesPath=_executablePath+"/"+SIM_CADFILES_DIRECTORY_NAME;
    _texturesPath=_executablePath;
    _videosPath=_executablePath;
    _otherFilesPath=_executablePath;
#endif

    if (App::userSettings->defaultDirectoryForScenes.length()!=0)
        _scenesPath=App::userSettings->defaultDirectoryForScenes;
    if (App::userSettings->defaultDirectoryForModels.length()!=0)
        _modelsPath=App::userSettings->defaultDirectoryForModels;
    if (App::userSettings->defaultDirectoryForCadFiles.length()!=0)
        _cadFilesPath=App::userSettings->defaultDirectoryForCadFiles;
    if (App::userSettings->defaultDirectoryForMiscFiles.length()!=0)
        _otherFilesPath=App::userSettings->defaultDirectoryForMiscFiles;
    if (App::userSettings->defaultDirectoryForExternalScriptEditor.length()!=0)
        _extScriptEditorTempPath=App::userSettings->defaultDirectoryForExternalScriptEditor;
    if (App::userSettings->defaultDirectoryForRemoteApiFiles.length()!=0)
        _remoteApiTempPath=App::userSettings->defaultDirectoryForRemoteApiFiles;

    VVarious::removePathFinalSlashOrBackslash(_scenesPath);
    VVarious::removePathFinalSlashOrBackslash(_modelsPath);
    VVarious::removePathFinalSlashOrBackslash(_cadFilesPath);
    VVarious::removePathFinalSlashOrBackslash(_otherFilesPath);
    VVarious::removePathFinalSlashOrBackslash(_extScriptEditorTempPath);
    VVarious::removePathFinalSlashOrBackslash(_remoteApiTempPath);
}

CFolderSystem::~CFolderSystem()
{

}

std::string CFolderSystem::getPathFromFull(const char* full)
{ // no final slash is returned
    return(VVarious::splitPath_path(full));
}

std::string CFolderSystem::getNameFromFull(const char* full)
{
    return(VVarious::splitPath_fileBase(full));
}

std::string CFolderSystem::getExecutablePath() const
{
    return(_executablePath);
}

std::string CFolderSystem::getSystemPath() const
{
    return(_systemPath);
}

std::string CFolderSystem::getScenesPath() const
{
    return(_scenesPath);
}

void CFolderSystem::setScenesPath(const char* path)
{
    _scenesPath=path;
    VVarious::removePathFinalSlashOrBackslash(_scenesPath);
}

std::string CFolderSystem::getModelsPath() const
{
    return(_modelsPath);
}

void CFolderSystem::setModelsPath(const char* path)
{
    _modelsPath=path;
    VVarious::removePathFinalSlashOrBackslash(_modelsPath);
}

std::string CFolderSystem::getCadFilesPath() const
{
    return(_cadFilesPath);
}

void CFolderSystem::setCadFilesPath(const char* path)
{
    _cadFilesPath=path;
    VVarious::removePathFinalSlashOrBackslash(_cadFilesPath);
}

std::string CFolderSystem::getTexturesPath() const
{
    return(_texturesPath);
}

void CFolderSystem::setTexturesPath(const char* path)
{
    _texturesPath=path;
    VVarious::removePathFinalSlashOrBackslash(_texturesPath);
}

std::string CFolderSystem::getVideosPath() const
{
    return(_videosPath);
}

void CFolderSystem::setVideosPath(const char* path)
{
    _videosPath=path;
    VVarious::removePathFinalSlashOrBackslash(_videosPath);
}

std::string CFolderSystem::getOtherFilesPath() const
{
    return(_otherFilesPath);
}

void CFolderSystem::setOtherFilesPath(const char* path)
{
    _otherFilesPath=path;
    VVarious::removePathFinalSlashOrBackslash(_otherFilesPath);
}

std::string CFolderSystem::getExtScriptEditorTempPath() const
{
    return(_extScriptEditorTempPath);
}

void CFolderSystem::setExtScriptEditorTempPath(const char* path)
{
    _extScriptEditorTempPath=path;
    VVarious::removePathFinalSlashOrBackslash(_extScriptEditorTempPath);
}

std::string CFolderSystem::getRemoteApiTempPath() const
{
    return(_remoteApiTempPath);
}

void CFolderSystem::setRemoteApiTempPath(const char* path)
{
    _remoteApiTempPath=path;
    VVarious::removePathFinalSlashOrBackslash(_remoteApiTempPath);
}

