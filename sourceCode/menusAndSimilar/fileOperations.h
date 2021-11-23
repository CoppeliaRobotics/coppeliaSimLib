
#pragma once

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
    static bool loadScene(const char* pathAndFilename,bool displayMessages,bool setCurrentDir,std::vector<char>* loadBuffer=nullptr);
    static bool loadModel(const char* pathAndFilename,bool displayMessages,bool setCurrentDir,bool doUndoThingInHere,std::vector<char>* loadBuffer,bool onlyThumbnail,bool forceModelAsCopy);
    static bool saveScene(const char* pathAndFilename,bool displayMessages,bool setCurrentDir,bool changeSceneUniqueId,std::vector<char>* saveBuffer=nullptr);
    static bool saveModel(int modelBaseDummyID,const char* pathAndFilename,bool displayMessages,bool setCurrentDir,std::vector<char>* saveBuffer=nullptr);

    static int apiAddHeightfieldToScene(int xSize,float pointSpacing,const std::vector<std::vector<float>*>& readData,float shadingAngle,int options);
    static void createNewScene(bool displayMessages,bool forceForNewInstance);
    static void closeScene(bool displayMessages);
    static void addToRecentlyOpenedScenes(std::string filenameAndPath);

private:
    static void _removeFromRecentlyOpenedScenes(std::string filenameAndPath);
    static bool heightfieldImportRoutine(const char* pathName);
    static std::string _getStringOfVersionAndLicenseThatTheFileWasWrittenWith(unsigned short coppeliaSimVer,int licenseType,char revision);

#ifdef SIM_WITH_GUI
public:
    static void keyPress(int key);
    static void addMenu(VMenu* menu);

private:
    static bool _saveSceneWithDialogAndEverything();
    static bool _saveSceneAsWithDialogAndEverything(int filetype);
#endif
};
