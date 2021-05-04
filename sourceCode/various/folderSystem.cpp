#include "simInternal.h"
#include "folderSystem.h"
#include "vFile.h"
#include "global.h"
#include "vVarious.h"
#include "app.h"
#include "simFlavor.h"
#include <filesystem>
#ifdef SIM_WITH_QT
    #include <QStandardPaths>
#endif

CFolderSystem::CFolderSystem()
{
    _executablePath=VVarious::getModulePath();
    VVarious::removePathFinalSlashOrBackslash(_executablePath);
#ifdef MAC_SIM
    _resourcesPath=_executablePath+"/../Resources";
#else
    _resourcesPath=_executablePath;
#endif
    _systemPath=_executablePath+"/"+SIM_SYSTEM_DIRECTORY_NAME;
    _scenesPath=_resourcesPath+"/"+CSimFlavor::getStringVal(13);// if scenes can't be found, it will use the last used directory somehow!
    _modelsPath=_resourcesPath+"/"+CSimFlavor::getStringVal(14);
    _cadFilesPath=_resourcesPath+"/"+SIM_CADFILES_DIRECTORY_NAME;
    _texturesPath=_resourcesPath;
    _videosPath=_resourcesPath;
    _otherFilesPath=_resourcesPath;

    if (App::userSettings->defaultDirectoryForScenes.length()!=0)
        setScenesPath(App::userSettings->defaultDirectoryForScenes.c_str());
    if (App::userSettings->defaultDirectoryForModels.length()!=0)
        setModelsPath(App::userSettings->defaultDirectoryForModels.c_str());
    if (App::userSettings->defaultDirectoryForCadFiles.length()!=0)
        setCadFilesPath(App::userSettings->defaultDirectoryForCadFiles.c_str());
    if (App::userSettings->defaultDirectoryForMiscFiles.length()!=0)
        setOtherFilesPath(App::userSettings->defaultDirectoryForMiscFiles.c_str());

#ifdef SIM_WITH_QT
    _tempDir=new QTemporaryDir();
    _tempDir->setAutoRemove(true);
    if (_tempDir->isValid())
        _tempDataPath=_tempDir->path().toStdString().c_str();
    QString s(QStandardPaths::locate(QStandardPaths::AppDataLocation,"CoppeliaSim",QStandardPaths::LocateDirectory));
    if (s.length()==0)
    {
        QString wl(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
        QDir(wl).mkdir("CoppeliaSim");
    }
    _appDataPath=QStandardPaths::writableLocation(QStandardPaths::AppDataLocation).toStdString()+"/CoppeliaSim";
#endif
    if (_tempDataPath.size()==0)
    {
        _tempDataPath=_executablePath+"/tmp";
        if (!VFile::doesFolderExist(_tempDataPath.c_str()))
            VFile::createFolder(_tempDataPath.c_str());
    }
    if (_appDataPath.size()==0)
    {
        _appDataPath=_executablePath+"/app";
        if (!VFile::doesFolderExist(_appDataPath.c_str()))
            VFile::createFolder(_appDataPath.c_str());
    }
}

CFolderSystem::~CFolderSystem()
{
#ifdef SIM_WITH_QT
    if (_tempDir->isValid())
        delete _tempDir;
        _tempDataPath.clear();
#endif
    if (_tempDataPath.size()>0)
    {
        try
        {
            std::filesystem::remove_all(_tempDataPath.c_str());
        }
        catch(std::filesystem::filesystem_error const &e)
        {
        }
    }
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

std::string CFolderSystem::getResourcesPath() const
{
    return(_resourcesPath);
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

std::string CFolderSystem::getAppDataPath() const
{
    return(_appDataPath);
}

std::string CFolderSystem::getTempDataPath() const
{
    return(_tempDataPath);
}

std::string CFolderSystem::getSceneTempDataPath() const
{
    std::string folder=_tempDataPath+"/sceneData"+std::to_string(App::currentWorld->environment->getSceneUniqueID());
    if (!VFile::doesFolderExist(folder.c_str()))
        VFile::createFolder(folder.c_str());
    return(folder);
}
