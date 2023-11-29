
#pragma once

#ifdef SIM_WITH_GUI
    #include <vMenubar.h>
#endif

struct SSimulationThreadCommand;

//FULLY STATIC CLASS
class CFileOperations  
{
public:
    static bool loadScene(const char* pathAndFilename,bool setCurrentDir,std::vector<char>* loadBuffer=nullptr,std::string* infoStr=nullptr,std::string* errorStr=nullptr);
    static bool loadModel(const char* pathAndFilename,bool setCurrentDir,bool doUndoThingInHere,std::vector<char>* loadBuffer,bool onlyThumbnail,bool forceModelAsCopy,std::string* infoStr=nullptr,std::string* errorStr=nullptr);
    static bool saveScene(const char* pathAndFilename,bool setCurrentDir,bool changeSceneUniqueId,std::vector<char>* saveBuffer=nullptr,std::string* infoStr=nullptr,std::string* errorStr=nullptr);
    static bool saveModel(int modelBaseDummyID,const char* pathAndFilename,bool setCurrentDir,std::vector<char>* saveBuffer=nullptr,std::string* infoStr=nullptr,std::string* errorStr=nullptr);

    static int createHeightfield(int xSize,double pointSpacing,const std::vector<std::vector<double>*>& readData,double shadingAngle,int options);
    static void createNewScene(bool keepCurrentScene);
    static void closeScene();

#ifdef SIM_WITH_GUI
public:
    static bool processCommand(int commandID);
    static bool processCommand(const SSimulationThreadCommand& cmd);
    static void keyPress(int key);
    static void addMenu(VMenu* menu);

private:
    static void _addToRecentlyOpenedScenes(std::string filenameAndPath);
    static void _removeFromRecentlyOpenedScenes(std::string filenameAndPath);
    static bool _heightfieldImportRoutine(const char* pathName);
    static bool _saveSceneWithDialogAndEverything();
    static bool _saveSceneAsWithDialogAndEverything(int filetype);
#endif
};
