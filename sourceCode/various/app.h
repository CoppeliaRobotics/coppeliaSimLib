#pragma once

#include <folderSystem.h>
#include <userSettings.h>
#include <worldContainer.h>
#include <sigHandler.h>
#include <simThread.h>
#include <gm.h>
#include <instance_id.h>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSystemSemaphore>
#include <propertiesAndMethods.h>
#ifndef SIM_WITH_GUI
#include <simQApp.h>
#endif

// ----------------------------------------------------------------------------------------------
#define FUNCX(name, str, v1, v2, t1, t2) const SProperty name = {str, v1, v2, t1, t2};
APP_PROPERTIES
#undef FUNCX
#define FUNCX(name, str, v1, v2, t1, t2) name,
const std::vector<SProperty> allProps_app = {APP_PROPERTIES};
#undef FUNCX
// ----------------------------------------------------------------------------------------------

static std::vector<std::pair<int, std::string>> propertyTypes = {
    {sim_propertytype_bool, proptypetag_bool},
    {sim_propertytype_int, proptypetag_int},
    {sim_propertytype_float, proptypetag_float},
    {sim_propertytype_string, proptypetag_string},
    {sim_propertytype_intarray2, proptypetag_intarray2},
    {sim_propertytype_long, proptypetag_long},
    {sim_propertytype_vector2, proptypetag_vector2},
    {sim_propertytype_vector3, proptypetag_vector3},
    {sim_propertytype_quaternion, proptypetag_quaternion},
    {sim_propertytype_pose, proptypetag_pose},
    {sim_propertytype_matrix3x3, proptypetag_matrix3x3},
    {sim_propertytype_matrix4x4, proptypetag_matrix4x4},
    {sim_propertytype_color, proptypetag_color},
    {sim_propertytype_floatarray, proptypetag_floatarray},
    {sim_propertytype_intarray, proptypetag_intarray},
    {sim_propertytype_table, proptypetag_table},
    {sim_propertytype_matrix, proptypetag_matrix},
    {sim_propertytype_null, proptypetag_null},
    {sim_propertytype_array, proptypetag_array},
    {sim_propertytype_map, proptypetag_map},
    {sim_propertytype_handle, proptypetag_handle},
    {sim_propertytype_handlearray, proptypetag_handlearray},
    {sim_propertytype_stringarray, proptypetag_stringarray},

    {sim_propertytype_buffer, proptypetag_buffer}, // keep always at the end
};

static std::map<int, std::string> propertyStrings = {
    {sim_propertytype_bool, proptypetag_bool},
    {sim_propertytype_int, proptypetag_int},
    {sim_propertytype_float, proptypetag_float},
    {sim_propertytype_string, proptypetag_string},
    {sim_propertytype_intarray2, proptypetag_intarray2},
    {sim_propertytype_long, proptypetag_long},
    {sim_propertytype_vector2, proptypetag_vector2},
    {sim_propertytype_vector3, proptypetag_vector3},
    {sim_propertytype_quaternion, proptypetag_quaternion},
    {sim_propertytype_pose, proptypetag_pose},
    {sim_propertytype_matrix3x3, proptypetag_matrix3x3},
    {sim_propertytype_matrix4x4, proptypetag_matrix4x4},
    {sim_propertytype_color, proptypetag_color},
    {sim_propertytype_floatarray, proptypetag_floatarray},
    {sim_propertytype_intarray, proptypetag_intarray},
    {sim_propertytype_table, proptypetag_table},
    {sim_propertytype_matrix, proptypetag_matrix},
    {sim_propertytype_null, proptypetag_null},
    {sim_propertytype_array, proptypetag_array},
    {sim_propertytype_map, proptypetag_map},
    {sim_propertytype_handle, proptypetag_handle},
    {sim_propertytype_handlearray, proptypetag_handlearray},
    {sim_propertytype_stringarray, proptypetag_stringarray},

    {sim_propertytype_buffer, proptypetag_buffer},
};

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

    static long long int getFreshUniqueId(int objectType);
    static void releaseUniqueId(long long int uid, int objectType = -1);

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

    static bool getAppNamedParam(const char* paramName, std::string& param);
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
    static void logScriptMsg(const CScriptObject* script, int verbosityLevel, const char* msg);
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

    static void pushGenesisEvents();
    static void setPluginList(const std::vector<CPlugin*>* plugins);


    static int setBoolProperty(long long int target, const char* pName, bool pState);
    static int getBoolProperty(long long int target, const char* pName, bool& pState);
    static int setIntProperty(long long int target, const char* pName, int pState);
    static int getIntProperty(long long int target, const char* pName, int& pState);
    static int setLongProperty(long long int target, const char* pName, long long int pState);
    static int getLongProperty(long long int target, const char* pName, long long int& pState);
    static int setFloatProperty(long long int target, const char* pName, double pState);
    static int getFloatProperty(long long int target, const char* pName, double& pState);
    static int setHandleProperty(long long int target, const char* pName, long long int pState);
    static int getHandleProperty(long long int target, const char* pName, long long int& pState);
    static int setStringProperty(long long int target, const char* pName, const char* pState);
    static int getStringProperty(long long int target, const char* pName, std::string& pState);
    static int setBufferProperty(long long int target, const char* pName, const char* buffer, int bufferL);
    static int getBufferProperty(long long int target, const char* pName, std::string& pState);
    static int setIntArray2Property(long long int target, const char* pName, const int* pState);
    static int getIntArray2Property(long long int target, const char* pName, int* pState);
    static int setVector2Property(long long int target, const char* pName, const double* pState);
    static int getVector2Property(long long int target, const char* pName, double* pState);
    static int setVector3Property(long long int target, const char* pName, const C3Vector& pState);
    static int getVector3Property(long long int target, const char* pName, C3Vector& pState);
    static int setQuaternionProperty(long long int target, const char* pName, const C4Vector& pState);
    static int getQuaternionProperty(long long int target, const char* pName, C4Vector& pState);
    static int setPoseProperty(long long int target, const char* pName, const C7Vector& pState);
    static int getPoseProperty(long long int target, const char* pName, C7Vector& pState);
    static int setColorProperty(long long int target, const char* pName, const float* pState);
    static int getColorProperty(long long int target, const char* pName, float* pState);
    static int setFloatArrayProperty(long long int target, const char* pName, const double* v, int vL);
    static int getFloatArrayProperty(long long int target, const char* pName, std::vector<double>& pState);
    static int setIntArrayProperty(long long int target, const char* pName, const int* v, int vL);
    static int getIntArrayProperty(long long int target, const char* pName, std::vector<int>& pState);
    static int setHandleArrayProperty(long long int target, const char* pName, const long long int* v, int vL); // ALL handle items have to be of the same type
    static int getHandleArrayProperty(long long int target, const char* pName, std::vector<long long int>& pState); // ALL handle items have to be of the same type
    static int setStringArrayProperty(long long int target, const char* pName, const std::vector<std::string>& pState);
    static int getStringArrayProperty(long long int target, const char* pName, std::vector<std::string>& pState);
    static int removeProperty(long long int target, const char* pName);
    static int getPropertyName(long long int target, int& index, std::string& pName, std::string& appartenance, bool staticParsing, int excludeFlags);
    static int getPropertyInfo(long long int target, const char* pName, int& info, std::string& infoTxt, bool staticParsing);

    static void undoRedo_sceneChanged(const char* txt);
    static void undoRedo_sceneChangedGradual(const char* txt);

    static void setHierarchyEnabled(bool v);
    static bool getHierarchyEnabled();
    static void setOpenGlDisplayEnabled(bool e);
    static bool getOpenGlDisplayEnabled();
    static bool canSave();
    static int getHeadlessMode();
    static int getPlatform();
    static void asyncResetScript(int scriptHandle);
    static bool appSemaphore(bool acquire, bool block = true);
    static bool systemSemaphore(const char* key, bool acquire);

    static CFolderSystem* folders;
    static CUserSettings* userSettings;
    static CSimThread* simThread;
    static CWorldContainer* worldContainer;
    static CWorld* currentWorld; // actually worldContainer->currentWorld
    static CGm* gm;
    static std::vector<void*> callbacks;
    static InstancesList* instancesList;
    static qint64 pid;

#ifndef SIM_WITH_GUI
    static CSimQApp* qtApp;
#endif

  private:
    static void _logMsg(const char* originName, int verbosityLevel, const char* msg, const char* subStr1,
                        const char* subStr2 = nullptr, const char* subStr3 = nullptr);
    static void _logMsg(const char* originName, int verbosityLevel, const char* msg, int int1, int int2 = 0,
                        int int3 = 0);
    static void __logMsg(const char* originName, int verbosityLevel, const char* msg, int consoleVerbosity = -1,
                         int statusbarVerbosity = -1);
    static bool _consoleLogFilter(const char* msg);
    static std::string _getHtmlEscapedString(const char* str);
    static bool _consoleMsgsToFile;
    static std::string _consoleMsgsFilename;
    static VFile* _consoleMsgsFile;
    static VArchive* _consoleMsgsArchive;
    static bool _hierarchyEnabled;
    static bool _openGlDisplayEnabled;

    static long long int _nextUniqueId;
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
    static std::string _consoleLogFilterStr;
    static std::string _startupScriptString;
    static std::map<std::string /*originName*/, std::map<int /*verbosityLevel*/, std::map<std::string /*msg*/, bool>>> _logOnceMessages;
    static std::string _applicationDir;
    static CPersistentDataContainer* _appStorage;
    static std::vector<int> _scriptsToReset;
    static std::map<std::string, SSysSemaphore> _systemSemaphores;
    static VMutex _appSemaphore;
    static std::vector<std::string> _pluginNames;

#ifdef USE_LONG_LONG_HANDLES
    static long long int _nextHandle_object;
    static long long int _nextHandle_collection;
    static long long int _nextHandle_script;
    static long long int _nextHandle_stack;
    static long long int _nextHandle_texture;
    static long long int _nextHandle_mesh;
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
