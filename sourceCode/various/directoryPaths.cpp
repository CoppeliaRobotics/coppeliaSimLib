#include "simInternal.h"
#include "directoryPaths.h"
#include "global.h"
#include "vVarious.h"
#include "app.h"
#include "simFlavor.h"

CDirectoryPaths::CDirectoryPaths()
{
    static std::string currentDir(VVarious::getModulePath());
    executableDirectory=currentDir;
    extScriptEditorTempFileDirectory=currentDir;
    remoteApiFileTransferDirectory=currentDir;
    systemDirectory=executableDirectory+"/"+SIM_SYSTEM_DIRECTORY_NAME;
#ifdef MAC_SIM
    // We are inside of the package!!!
    /*
    sceneDirectory=executableDirectory+"/../../../"+CSimFlavor::getStringVal(13);// if scenes can't be found, it will use the last used directory somehow!
    modelDirectory=executableDirectory+"/../../../"+CSimFlavor::getStringVal(14);
    cadFormatDirectory=executableDirectory+"/../../../"+SIM_CADFILES_DIRECTORY_NAME;
    textureDirectory=executableDirectory+"/../../../";
    videoDirectory=executableDirectory+"/../../../";
    otherFilesDirectory=executableDirectory+"/../../../";
    */
    sceneDirectory=executableDirectory+"/../../Resources/"+CSimFlavor::getStringVal(13);// if scenes can't be found, it will use the last used directory somehow!
    modelDirectory=executableDirectory+"/../../Resources/"+CSimFlavor::getStringVal(14);
    cadFormatDirectory=executableDirectory+"/../../Resources/"+SIM_CADFILES_DIRECTORY_NAME;
    textureDirectory=executableDirectory+"/../../Resources/";
    videoDirectory=executableDirectory+"/../../Resources/";
    otherFilesDirectory=executableDirectory+"/../../Resources/";
#else
    sceneDirectory=executableDirectory+"/"+CSimFlavor::getStringVal(13);// if scenes can't be found, it will use the last used directory somehow!
    modelDirectory=executableDirectory+"/"+CSimFlavor::getStringVal(14);
    cadFormatDirectory=executableDirectory+"/"+SIM_CADFILES_DIRECTORY_NAME;
    textureDirectory=executableDirectory;
    videoDirectory=executableDirectory;
    otherFilesDirectory=executableDirectory;
#endif

    if (App::userSettings->defaultDirectoryForScenes.length()!=0)
        sceneDirectory=App::userSettings->defaultDirectoryForScenes;
    if (App::userSettings->defaultDirectoryForModels.length()!=0)
        modelDirectory=App::userSettings->defaultDirectoryForModels;
    if (App::userSettings->defaultDirectoryForCadFiles.length()!=0)
        cadFormatDirectory=App::userSettings->defaultDirectoryForCadFiles;
    if (App::userSettings->defaultDirectoryForMiscFiles.length()!=0)
        otherFilesDirectory=App::userSettings->defaultDirectoryForMiscFiles;
    if (App::userSettings->defaultDirectoryForExternalScriptEditor.length()!=0)
        extScriptEditorTempFileDirectory=App::userSettings->defaultDirectoryForExternalScriptEditor;
    if (App::userSettings->defaultDirectoryForRemoteApiFiles.length()!=0)
        remoteApiFileTransferDirectory=App::userSettings->defaultDirectoryForRemoteApiFiles;

    VVarious::removePathFinalSlashOrBackslash(sceneDirectory);
    VVarious::removePathFinalSlashOrBackslash(modelDirectory);
    VVarious::removePathFinalSlashOrBackslash(cadFormatDirectory);
    VVarious::removePathFinalSlashOrBackslash(otherFilesDirectory);
    VVarious::removePathFinalSlashOrBackslash(extScriptEditorTempFileDirectory);
    VVarious::removePathFinalSlashOrBackslash(remoteApiFileTransferDirectory);
}

CDirectoryPaths::~CDirectoryPaths()
{

}

std::string CDirectoryPaths::getPathFromFull(const std::string& full)
{ // no final slash is returned
    return(VVarious::splitPath_path(full));
}

std::string CDirectoryPaths::getNameFromFull(const std::string& full)
{
    return(VVarious::splitPath_fileBase(full));
}
