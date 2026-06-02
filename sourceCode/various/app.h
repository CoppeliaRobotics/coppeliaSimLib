#pragma once

#include <folderSystem.h>
#include <userSettings.h>
#include <sceneContainer.h>
#include <sigHandler.h>
#include <simThread.h>
#include <gm.h>
#include <instance_id.h>
#include <QSystemSemaphore>
#include <obj.h>
#include <propertiesAndMethods.h>
#ifndef SIM_WITH_GUI
#include <simQApp.h>
#endif

// ----------------------------------------------------------------------------------------------
#define FUNCX(name, str, v1, v2, t1, t2) extern const SProperty name;
APP_PROPERTIES
#undef FUNCX
extern const std::vector<SProperty> allProps_app;
// ----------------------------------------------------------------------------------------------

struct SSysSemaphore
{
    QSystemSemaphore* semaphore;
    int cnt;
};

class App
{
  public:
    enum
    {
        appstage_none = 0,
        appstage_simInit1Done,
        appstage_guiInit1Done, // set by UI thread. An temp dummy QApplication is created to make sure Qt doesn't
                               // complain about wrong thread
        appstage_simInit2Done, // the SIM thread is ready, except for the simThread object, and sandbox/add-ons
                               // load/initializations
        appstage_guiInit2Done, // set by UI thread. The UI thread is ready
        appstage_simRunning,
        appstage_guiCleanupRequest,
        appstage_guiCleanupDone, // set by UI thread
        appstage_simCleanupDone,
    };

    App();
    virtual ~App();

    static int64_t getFreshUniqueId(int objectType);
    static void releaseUniqueId(int64_t uid, int objectType = -1);

    static UID getNewHandleFromOldHandle(int oldHandle);
    static int getOldHandleFromNewHandle(UID newHandle);

    static void beep(int frequ = 5000, int duration = 1000);

    static void init(const char* appDir, int options);
    static void cleanup();
    static void loop(void (*callback)(), bool stepIfRunning);

    static std::string getApplicationDir();
    static void postExitRequest();
    static bool getExitRequest();

    static void appendSimulationThreadCommand(int cmdId, int intP1 = -1, int intP2 = -1, double floatP1 = 0.0,
                                              double floatP2 = 0.0, const char* stringP1 = nullptr,
                                              const char* stringP2 = nullptr, double executionDelay = 0.0);
    static void appendSimulationThreadCommand(SSimulationThreadCommand cmd, double executionDelay = 0.0);

    static std::string getApplicationArgument(int index);
    static void setApplicationArgument(int index, std::string arg);

    static bool getAppNamedParam(const char* paramName, std::string& param, bool checkAlsoGroupType = false);
    static void setAppNamedParam(const char* paramName, const char* param, int paramLength = 0);
    static bool removeAppNamedParam(const char* paramName);

    static void setAdditionalAddOnScript1(const char* script);
    static std::string getAdditionalAddOnScript1();
    static void setAdditionalAddOnScript2(const char* script);
    static std::string getAdditionalAddOnScript2();
    static bool assemble(int parentHandle, int childHandle, bool justTest, bool msgs = false);
    static bool disassemble(int objectHandle, bool justTest, bool msgs = false);

    static std::string getConsoleLogFilter();
    static void setConsoleLogFilter(const char* filter);
    static bool logPluginMsg(const char* pluginName, int verbosityLevel, const char* logMsg);
    static void logMsg(int verbosityLevel, const char* msg);
    static void logMsg(int verbosityLevel, const char* msg, const char* subStr1, const char* subStr2 = nullptr,
                       const char* subStr3 = nullptr);
    static void logMsg(int verbosityLevel, const char* msg, int int1, int int2 = 0, int int3 = 0);
    static void logScriptMsg(const CDetachedScript* script, int verbosityLevel, const char* msg);
    static void setStringVerbosity(int what, const char* str);
    static int getConsoleVerbosity(const char* pluginName = nullptr);
    static void setConsoleVerbosity(int v, const char* pluginName = nullptr);
    static int getStatusbarVerbosity(const char* pluginName = nullptr);
    static void setStatusbarVerbosity(int v, const char* pluginName = nullptr);
    static bool getConsoleOrStatusbarVerbosityTriggered(int verbosityLevel);
    static int getVerbosityLevelFromString(const char* verbosityStr);
    static bool getConsoleMsgToFile();
    static void setConsoleMsgToFile(bool f);
    static std::string getConsoleMsgFile();
    static void setConsoleMsgFile(const char* f);
    static int getDlgVerbosity();
    static void setDlgVerbosity(int v);
    static void setStartupScriptString(const char* str);
    static void setExitCode(int c);
    static int getExitCode();
    static int getAppStage();
    static void setAppStage(int s);
    static void changeAppWideYieldingForbidLevel(int dx);
    static bool isAppWideYieldingForbidden();


    static void pushGenesisEvents();
    static void setPluginList(const std::vector<CPlugin*>* plugins);


    static int setBoolProperty_t(int64_t target, const char* pName, bool pState);
    static int getBoolProperty_t(int64_t target, const char* pName, bool& pState);
    static int setIntProperty_t(int64_t target, const char* pName, int pState);
    static int getIntProperty_t(int64_t target, const char* pName, int& pState);
    static int setLongProperty_t(int64_t target, const char* pName, int64_t pState);
    static int getLongProperty_t(int64_t target, const char* pName, int64_t& pState);
    static int setFloatProperty_t(int64_t target, const char* pName, double pState);
    static int getFloatProperty_t(int64_t target, const char* pName, double& pState);
    static int setHandleProperty_t(int64_t target, const char* pName, int64_t pState);
    static int getHandleProperty_t(int64_t target, const char* pName, int64_t& pState);
    static int setStringProperty_t(int64_t target, const char* pName, const std::string& pState);
    static int getStringProperty_t(int64_t target, const char* pName, std::string& pState);
    static int setTableProperty_t(int64_t target, const char* pName, const std::string& pState);
    static int getTableProperty_t(int64_t target, const char* pName, std::string& pState);
    static int setBufferProperty_t(int64_t target, const char* pName, const std::string& pState);
    static int getBufferProperty_t(int64_t target, const char* pName, std::string& pState);
    static int setIntArray2Property_t(int64_t target, const char* pName, const int* pState);
    static int getIntArray2Property_t(int64_t target, const char* pName, int* pState);
    static int setVector3Property_t(int64_t target, const char* pName, const C3Vector& pState);
    static int getVector3Property_t(int64_t target, const char* pName, C3Vector& pState);
    static int setMatrixProperty_t(int64_t target, const char* pName, const CMatrix& pState);
    static int getMatrixProperty_t(int64_t target, const char* pName, CMatrix& pState);
    static int setQuaternionProperty_t(int64_t target, const char* pName, const CQuaternion& pState);
    static int getQuaternionProperty_t(int64_t target, const char* pName, CQuaternion& pState);
    static int setPoseProperty_t(int64_t target, const char* pName, const CPose& pState);
    static int getPoseProperty_t(int64_t target, const char* pName, CPose& pState);
    static int setColorProperty_t(int64_t target, const char* pName, const float* pState);
    static int getColorProperty_t(int64_t target, const char* pName, float* pState);
    static int setFloatArrayProperty_t(int64_t target, const char* pName, const std::vector<double>& pState);
    static int getFloatArrayProperty_t(int64_t target, const char* pName, std::vector<double>& pState);
    static int setIntArrayProperty_t(int64_t target, const char* pName, const std::vector<int>& pState);
    static int getIntArrayProperty_t(int64_t target, const char* pName, std::vector<int>& pState);
    static int setHandleArrayProperty_t(int64_t target, const char* pName, const std::vector<int64_t>& pState); // ALL handle items have to be of the same type
    static int getHandleArrayProperty_t(int64_t target, const char* pName, std::vector<int64_t>& pState); // ALL handle items have to be of the same type
    static int setStringArrayProperty_t(int64_t target, const char* pName, const std::vector<std::string>& pState);
    static int getStringArrayProperty_t(int64_t target, const char* pName, std::vector<std::string>& pState);
    static int setMethodProperty_t(int64_t target, const char* pName, const void* pState);
    static int getMethodProperty_t(int64_t target, const char* pName, void*& pState);
    static int setMethodProperty_t(int64_t target, const char* pName, const std::string& pState);
    static int getMethodProperty_t(int64_t target, const char* pName, std::string& pState);
    static int removeProperty_t(int64_t target, const char* pName);
    static int getPropertyName_t(int64_t target, int& index, std::string& pName, std::string& appartenance, int excludeFlags);
    static int getPropertyInfo_t(int64_t target, const char* pName, int& info, std::string& infoTxt);
    static int setPropertyInfo_t(int64_t target, const char* pName, int info, const char* infoTxt);
    static bool isTargetValid_t(int64_t target);

    static void undoRedo_sceneChanged(const char* txt);
    static void undoRedo_sceneChangedGradual(const char* txt);

    static void setHierarchyEnabled(bool v);
    static bool getHierarchyEnabled();
    static void setOpenGlDisplayEnabled(bool e);
    static bool getOpenGlDisplayEnabled();
    static bool canSave();
    static int getHeadlessMode();
    static int getPlatform();
    static int getEventProtocolVersion();
    static void setEventProtocolVersion(int v);
    static void asyncResetScript(int scriptHandle);
    static bool appSemaphore(bool acquire, bool block = true);
    static bool systemSemaphore(const char* key, bool acquire);

    static CFolderSystem* folders;
    static CUserSettings* userSettings;
    static CSimThread* simThread;
    static CSceneContainer* scenes;
    static CScene* scene; // actually scenes->scene
    static CGm* gm;
    static std::vector<void*> callbacks;
    static InstancesList* instancesList;
    static qint64 pid;

#ifndef SIM_WITH_GUI
    static CSimQApp* qtApp;
#endif

  private:
    static void _logMsg(const char* originName, int verbosityLevel, const char* msg, const char* subStr1, const char* subStr2 = nullptr, const char* subStr3 = nullptr);
    static void _logMsg(const char* originName, int verbosityLevel, const char* msg, int int1, int int2 = 0, int int3 = 0);
    static void __logMsg(const char* originName, int verbosityLevel, const char* msg, int consoleVerbosity = -1, int statusbarVerbosity = -1);
    static bool _consoleLogFilter(const char* msg);
    static std::string _getHtmlEscapedString(const char* str);
    static bool _resolveTarget(int64_t& target);
    static bool _consoleMsgsToFile;
    static std::string _consoleMsgsFilename;
    static VFile* _consoleMsgsFile;
    static VArchive* _consoleMsgsArchive;
    static bool _hierarchyEnabled;
    static bool _openGlDisplayEnabled;
    static int _eventProtocolVersion;
    static int _appWideYieldingForbidLevel;

    static int64_t _nextUniqueId;
    static SignalHandler* _sigHandler;

    static int _qApp_argc;
    static char _qApp_arg0[];
    static char* _qApp_argv[1];
    static std::vector<std::string> _applicationArguments;
    static std::map<std::string, std::string> _applicationNamedParams;
    static std::string _additionalAddOnScript1;
    static std::string _additionalAddOnScript2;
    static int _consoleVerbosity;
    static int _statusbarVerbosity;
    static int _dlgVerbosity;
    static int _exitCode;
    static bool _exitRequest;
    static volatile int _appStage;
    static Obj* _obj;
    static std::string _consoleLogFilterStr;
    static std::string _startupScriptString;
    static std::map<std::string /*originName*/, std::map<int /*verbosityLevel*/, std::map<std::string /*msg*/, bool>>> _logOnceMessages;
    static std::string _applicationDir;
    static CPersistentDataContainer* _appStorage;
    static std::vector<int> _scriptsToReset;
    static std::map<std::string, SSysSemaphore> _systemSemaphores;
    static VMutex _appSemaphore;
    static std::vector<std::string> _pluginNames;

#ifdef USE_INT64_HANDLES
    static int64_t _nextHandle_object;
    static int64_t _nextHandle_collection;
    static int64_t _nextHandle_script;
    static int64_t _nextHandle_stack;
    static int64_t _nextHandle_texture;
    static int64_t _nextHandle_mesh;
#endif
};

class CFuncTrace
{
  public:
    CFuncTrace(const char* functionName, int traceVerbosity)
    {
        if (App::getConsoleOrStatusbarVerbosityTriggered(traceVerbosity))
        {
            _txt = functionName;
            _verbosity = traceVerbosity;
            if (_verbosity == sim_verbosity_traceall)
                _txt += " (C)";
            if (_verbosity == sim_verbosity_tracelua)
                _txt += " (Lua API)";
            App::logMsg(_verbosity, (std::string("--> ") + _txt).c_str());
        }
    };
    virtual ~CFuncTrace()
    {
        if (_txt.size() > 0)
            App::logMsg(_verbosity, (std::string("<-- ") + _txt).c_str());
    };

  private:
    std::string _txt;
    int _verbosity;
};
