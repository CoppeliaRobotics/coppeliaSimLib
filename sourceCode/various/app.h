#pragma once

#include "uiThread.h"
#include "simThread.h"
#include "directoryPaths.h"
#include "userSettings.h"
#include "vMutex.h"
#include "mainContainer.h"
#include "appBase.h"
#ifndef SIM_WITHOUT_QT_AT_ALL
    #include "vrepQApp.h"
    #include "simAndUiThreadSync.h"
#endif
#ifdef SIM_WITH_GUI
    #include "mainWindow.h"
#endif

class App : public AppBase
{
public:
    App(bool headless);
    virtual ~App();

    bool wasInitSuccessful();
    static void setBrowserEnabled(bool e);
    static bool getBrowserEnabled();

    static void beep(int frequ=5000,int duration=1000);
    static void setApplicationName(const char* name);
    static std::string getApplicationName();
    static void createMainContainer();
    static void deleteMainContainer();

    static void run(void(*initCallBack)(),void(*loopCallBack)(),void(*deinitCallBack)(),bool launchSimThread);
    static void postExitRequest();
    static bool getExitRequest();
    static bool isSimulatorRunning();

    static void simulationThreadInit();
    static void simulationThreadDestroy();
    static void simulationThreadLoop();
    static bool canInitSimThread();

    static void setQuitLevel(int l);
    static int getQuitLevel();

    static std::string getApplicationArgument(int index);
    static void setApplicationArgument(int index,std::string arg);
    static std::string getApplicationNamedParam(const char* paramName);
    static int setApplicationNamedParam(const char* paramName,const char* param,int paramLength);

    static bool executeUiThreadCommand(SUIThreadCommand* cmdIn,SUIThreadCommand* cmdOut);
    static void appendSimulationThreadCommand(int cmdId,int intP1=-1,int intP2=-1,float floatP1=0.0,float floatP2=0.0,const char* stringP1=nullptr,const char* stringP2=nullptr,int executionDelay=0);
    static void appendSimulationThreadCommand(SSimulationThreadCommand cmd,int executionDelay=0);

    static int getEditModeType(); // helper
    static void setRebuildHierarchyFlag(); // helper
    static void setResetHierarchyViewFlag(); // helper
    static void setRefreshHierarchyViewFlag(); // helper
    static void setLightDialogRefreshFlag(); // helper
    static void setFullDialogRefreshFlag(); // helper
    static void setDialogRefreshDontPublishFlag(); // helper
    static void setToolbarRefreshFlag(); // helper
    static int getMouseMode(); // helper
    static void setMouseMode(int mm); // helper
    static void setDefaultMouseMode(); // helper
    static bool isFullScreen(); // helper
    static void setFullScreen(bool f); // helper

    static void addStatusbarMessage(const std::string& txt,bool scriptErrorMsg=false);
    static void clearStatusbar();

    static float* getRGBPointerFromItem(int objType,int objID1,int objID2,int colComponent,std::string* auxDlgTitle);
    static CVisualParam* getVisualParamPointerFromItem(int objType,int objID1,int objID2,std::string* auxDlgTitle,int* allowedParts);
    static CTextureProperty* getTexturePropertyPointerFromItem(int objType,int objID1,int objID2,std::string* auxDlgTitle,bool* is3D,bool* valid,CGeometric** geom);

    static CDirectoryPaths* directories;
    static CUserSettings* userSettings;
    static CMainContainer* ct;
    static CUiThread* uiThread;
    static CSimThread* simThread;

    static int operationalUIParts;
    static int sc;

private:
    bool _initSuccessful;
    static bool _browserEnabled;
    static bool _canInitSimThread;

    static void _runInitializationCallback(void(*initCallBack)());
    static void _runDeinitializationCallback(void(*deinitCallBack)());
    static void _processGuiEventsUntilQuit();

//  static VTHREAD_ID_TYPE _guiThread;
    static bool _exitRequest;
    static bool _simulatorIsRunning;
    static std::string _applicationName;
    static std::vector<std::string> _applicationArguments;
    static std::map<std::string,std::string> _applicationNamedParams;

    static volatile int _quitLevel;

#ifndef SIM_WITHOUT_QT_AT_ALL
public:
    static CVrepQApp* qtApp;

private:
    static int _qApp_argc;
    static char _qApp_arg0[];
    static char* _qApp_argv[1];
#endif

#ifdef SIM_WITH_GUI
public:
    static void showSplashScreen();
    static void setIcon();
    static CMainWindow* mainWindow;
    static void createMainWindow();
    static void deleteMainWindow();
    static void setShowConsole(bool s);
#endif
};
