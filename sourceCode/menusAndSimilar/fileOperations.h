
#pragma once

#include "vrepMainHeader.h"
#ifdef SIM_WITH_GUI
    #include "vMenubar.h"
#endif

struct SSimulationThreadCommand;

//FULLY STATIC CLASS
class CFileOperations  
{
public:
    static bool processCommand(int commandID);
    static bool processCommand(const SSimulationThreadCommand& cmd);
    static bool loadScene(const char* pathAndFilename,bool displayMessages,bool displayDialogs,bool setCurrentDir);
    static bool loadModel(const char* pathAndFilename,bool displayMessages,bool displayDialogs,bool setCurrentDir,std::string* acknowledgmentPointerInReturn,bool doUndoThingInHere,std::vector<char>* loadBuffer,bool onlyThumbnail,bool forceModelAsCopy);
    static bool saveScene(const char* pathAndFilename,bool displayMessages,bool displayDialogs,bool setCurrentDir,bool changeSceneUniqueId);
    static bool saveModel(int modelBaseDummyID,const char* pathAndFilename,bool displayMessages,bool displayDialogs,bool setCurrentDir,std::vector<char>* saveBuffer=nullptr);

    static int apiAddHeightfieldToScene(int xSize,float pointSpacing,const std::vector<std::vector<float>*>& readData,float shadingAngle,int options);
    static void createNewScene(bool displayMessages,bool forceForNewInstance);
    static void closeScene(bool displayMessages,bool displayDialogs);
    static void addToRecentlyOpenedScenes(std::string filenameAndPath);
    static bool apiExportIkContent(const char* pathAndName,bool displayDialogs);

private:
    static void _removeFromRecentlyOpenedScenes(std::string filenameAndPath);
    static bool _pathImportRoutine(const std::string& pathName,bool displayDialogs);
    static bool _pathExportPoints(const std::string& pathName,int pathID,bool bezierPoints,bool displayDialogs);
    static bool heightfieldImportRoutine(const std::string& pathName);
    static std::string _getStringOfVersionAndLicenseThatTheFileWasWrittenWith(unsigned short vrepVer,int licenseType,char revision);

#ifdef SIM_WITH_GUI
public:
    static void keyPress(int key);
    static void addMenu(VMenu* menu);

private:
    static bool _saveSceneWithDialogAndEverything();
    static bool _saveSceneAsWithDialogAndEverything(int filetype);
#endif
};
