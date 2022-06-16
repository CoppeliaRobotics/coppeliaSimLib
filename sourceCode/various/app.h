#pragma once

#include "uiThread.h"
#include "simThread.h"
#include "folderSystem.h"
#include "userSettings.h"
#include "vMutex.h"
#include "worldContainer.h"
#include "gm.h"
#ifdef SIM_WITH_QT
    #include "simQApp.h"
    #include "simAndUiThreadSync.h"
#endif
#ifdef SIM_WITH_GUI
    #include "mainWindow.h"
#endif

class App
{
public:
    App(bool headless);
    virtual ~App();

    bool wasInitSuccessful();
    static void setBrowserEnabled(bool e);
    static bool getBrowserEnabled();
    static long long int getFreshUniqueId();

    static void beep(int frequ=5000,int duration=1000);
    static void setApplicationName(const char* name);
    static std::string getApplicationName();
    static void createWorldsContainer();
    static void deleteWorldsContainer();

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
    static void setAdditionalAddOnScript1(const char* script);
    static std::string getAdditionalAddOnScript1();
    static void setAdditionalAddOnScript2(const char* script);
    static std::string getAdditionalAddOnScript2();

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

    static std::string getConsoleLogFilter();
    static void setConsoleLogFilter(const char* filter);
    static bool logPluginMsg(const char* pluginName,int verbosityLevel,const char* logMsg);
    static void logMsg(int verbosityLevel,const char* msg);
    static void logMsg(int verbosityLevel,const char* msg,const char* subStr1,const char* subStr2=nullptr,const char* subStr3=nullptr);
    static void logMsg(int verbosityLevel,const char* msg,int int1,int int2=0,int int3=0);
    static void logScriptMsg(const char* scriptName,int verbosityLevel,const char* msg);
    static int getConsoleVerbosity(const char* pluginName=nullptr);
    static void setConsoleVerbosity(int v,const char* pluginName=nullptr);
    static int getStatusbarVerbosity(const char* pluginName=nullptr);
    static void setStatusbarVerbosity(int v,const char* pluginName=nullptr);
    static bool getConsoleOrStatusbarVerbosityTriggered(int verbosityLevel);
    static int getVerbosityLevelFromString(const char* verbosityStr);
    static bool getConsoleMsgToFile();
    static void setConsoleMsgToFile(bool f);
    static bool isCurrentThreadTheUiThread();
    static void clearStatusbar();
    static int getDlgVerbosity();
    static void setDlgVerbosity(int v);
    static void setStartupScriptString(const char* str);
    static void setExitCode(int c);
    static int getExitCode();
    static bool isOnline();
    static bool isQtAppBuilt();

    static void undoRedo_sceneChanged(const char* txt);
    static void undoRedo_sceneChangedGradual(const char* txt);
    static void undoRedo_sceneChangeStart(const char* txt);
    static void undoRedo_sceneChangeEnd();

    static float* getRGBPointerFromItem(int objType,int objID1,int objID2,int colComponent,std::string* auxDlgTitle);
    static CColorObject* getVisualParamPointerFromItem(int objType,int objID1,int objID2,std::string* auxDlgTitle,int* allowedParts);
    static CTextureProperty* getTexturePropertyPointerFromItem(int objType,int objID1,int objID2,std::string* auxDlgTitle,bool* is3D,bool* valid,CMesh** geom);

    static CFolderSystem* folders;
    static CUserSettings* userSettings;
    static CWorldContainer* worldContainer;
    static CWorld* currentWorld; // actually worldContainer->currentWorld
    static CUiThread* uiThread;
    static CSimThread* simThread;
    static CGm* gm;

    static int operationalUIParts;
    static int sc;

    static void _logMsgToStatusbar(const char* msg,bool html);

private:
    static void _logMsg(const char* originName,int verbosityLevel,const char* msg,const char* subStr1,const char* subStr2=nullptr,const char* subStr3=nullptr);
    static void _logMsg(const char* originName,int verbosityLevel,const char* msg,int int1,int int2=0,int int3=0);
    static void __logMsg(const char* originName,int verbosityLevel,const char* msg,int consoleVerbosity=-1,int statusbarVerbosity=-1);
    static bool _consoleLogFilter(const char* msg);
    static std::string _getHtmlEscapedString(const char* str);
    bool _initSuccessful;
    static bool _consoleMsgsToFile;
    static VFile* _consoleMsgsFile;
    static VArchive* _consoleMsgsArchive;

    static bool _browserEnabled;
    static bool _canInitSimThread;
    static long long int _nextUniqueId;

    static void _runInitializationCallback(void(*initCallBack)());
    static void _runDeinitializationCallback(void(*deinitCallBack)());
    static void _processGuiEventsUntilQuit();

//  static VTHREAD_ID_TYPE _guiThread;
    static bool _exitRequest;
    static bool _simulatorIsRunning;
    static std::string _applicationName;
    static std::vector<std::string> _applicationArguments;
    static std::map<std::string,std::string> _applicationNamedParams;
    static std::string _additionalAddOnScript1;
    static std::string _additionalAddOnScript2;
    static int _consoleVerbosity;
    static int _statusbarVerbosity;
    static int _dlgVerbosity;
    static int _exitCode;
    static bool _online;
    static std::string _consoleLogFilterStr;
    static std::string _startupScriptString;

    static volatile int _quitLevel;

#ifdef SIM_WITH_QT
public:
    static CSimQApp* qtApp;

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

class CFuncTrace
{
public:
    CFuncTrace(const char* functionName,int traceVerbosity)
    {
        if (App::getConsoleOrStatusbarVerbosityTriggered(traceVerbosity))
        {
            _txt=functionName;
            _verbosity=traceVerbosity;
            if (_verbosity==sim_verbosity_traceall)
                _txt+=" (C)";
            if (_verbosity==sim_verbosity_tracelua)
                _txt+=" (Lua API)";
            App::logMsg(_verbosity,(std::string("--> ")+_txt).c_str());
        }
    };
    virtual ~CFuncTrace()
    {
        if (_txt.size()>0)
            App::logMsg(_verbosity,(std::string("<-- ")+_txt).c_str());
    };

private:
    std::string _txt;
    int _verbosity;
};

