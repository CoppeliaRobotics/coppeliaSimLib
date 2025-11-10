#pragma once

#include <folderSystem.h>
#include <userSettings.h>
#include <worldContainer.h>
#include <sigHandler.h>
#include <simThread.h>
#include <gm.h>
#include <instance_id.h>
#include <QSystemSemaphore>
#ifndef SIM_WITH_GUI
#include <simQApp.h>
#endif

#define CUSTOMDATAPREFIX "customData."
#define SIGNALPREFIX "signal."
#define NAMEDPARAMPREFIX "namedParam."
#define REFSPREFIX "refs."
#define ORIGREFSPREFIX "origRefs."
#define COLORPREFIX "color."
#define COLORPREFIX_CAP "Color."

#define APP_META_METHODS R"("getBoolProperty": "sim-2.getBoolProperty",
        "getBufferProperty": "sim-2.getBufferProperty",
        "getColorProperty": "sim-2.getColorProperty",
        "getExplicitHandling": "sim-2.getExplicitHandling",
        "getFloatArrayProperty": "sim-2.getFloatArrayProperty",
        "getFloatProperty": "sim-2.getFloatProperty",
        "getHandleArrayProperty": "sim-2.getHandleArrayProperty",
        "getHandleProperty": "sim-2.getHandleProperty",
        "getIntArray2Property": "sim-2.getIntArray2Property",
        "getIntArrayProperty": "sim-2.getIntArrayProperty",
        "getIntProperty": "sim-2.getIntProperty",
        "getLongProperty": "sim-2.getLongProperty",
        "getPoseProperty": "sim-2.getPoseProperty",
        "getProperties": "sim-2.getProperties",
        "getPropertiesInfos": "sim-2.getPropertiesInfos",
        "getProperty": "sim-2.getProperty",
        "getPropertyInfo": "sim-2.getPropertyInfo",
        "getPropertyName": "sim-2.getPropertyName",
        "getPropertyTypeString": "sim-2.getPropertyTypeString",
        "getQuaternionProperty": "sim-2.getQuaternionProperty",
        "getStringProperty": "sim-2.getStringProperty",
        "getTableProperty": "sim-2.getTableProperty",
        "getVector2Property": "sim-2.getVector2Property",
        "getVector3Property": "sim-2.getVector3Property",
        "removeProperty": "sim-2.removeProperty",
        "setBoolProperty": "sim-2.setBoolProperty",
        "setBufferProperty": "sim-2.setBufferProperty",
        "setColorProperty": "sim-2.setColorProperty",
        "setFloatArrayProperty": "sim-2.setFloatArrayProperty",
        "setFloatProperty": "sim-2.setFloatProperty",
        "setHandleArrayProperty": "sim-2.setHandleArrayProperty",
        "setHandleProperty": "sim-2.setHandleProperty",
        "setIntArray2Property": "sim-2.setIntArray2Property",
        "setIntArrayProperty": "sim-2.setIntArrayProperty",
        "setIntProperty": "sim-2.setIntProperty",
        "setLongProperty": "sim-2.setLongProperty",
        "setPoseProperty": "sim-2.setPoseProperty",
        "setProperties": "sim-2.setProperties",
        "setProperty": "sim-2.setProperty",
        "setQuaternionProperty": "sim-2.setQuaternionProperty",
        "setStringProperty": "sim-2.setStringProperty",
        "setTableProperty": "sim-2.setTableProperty",
        "setVector2Property": "sim-2.setVector2Property",
        "setVector3Property": "sim-2.setVector3Property")"

// ----------------------------------------------------------------------------------------------
// flags: bit0: not writable, bit1: not readable, bit2: removable
#define DEFINE_PROPERTIES                                                                                                                                                                                                                     \
    FUNCX(propApp_objectType, "objectType", sim_propertytype_string, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, "Object type", "")                                                                                        \
    FUNCX(propApp_objectMetaInfo, "objectMetaInfo", sim_propertytype_string, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, "Object meta information", "")                                                                                        \
    FUNCX(propApp_sessionId, "sessionId", sim_propertytype_string, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, "Session ID", "")                                                                                        \
    FUNCX(propApp_protocolVersion, "protocolVersion", sim_propertytype_int, sim_propertyinfo_notwritable, "Protocol", "Protocol version")                                                                                                     \
    FUNCX(propApp_productVersion, "productVersion", sim_propertytype_string, sim_propertyinfo_notwritable, "Product string", "Product version (string)")                                                                                      \
    FUNCX(propApp_productVersionNb, "productVersionNb", sim_propertytype_int, sim_propertyinfo_notwritable, "Product", "Product version (number)")                                                                                            \
    FUNCX(propApp_platform, "platform", sim_propertytype_int, sim_propertyinfo_notwritable, "Platform", "Platform (0: Windows, 1: macOS, 2: Linux)")                                                                                                                                 \
    FUNCX(propApp_flavor, "flavor", sim_propertytype_int, sim_propertyinfo_notwritable, "Flavor", "Flavor (0: lite, 1: edu, 2: pro)")                                                                                                                                         \
    FUNCX(propApp_qtVersion, "qtVersion", sim_propertytype_int, sim_propertyinfo_notwritable, "Qt", "Qt version")                                                                                                                             \
    FUNCX(propApp_processId, "processId", sim_propertytype_int, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, "Process", "Process ID")                                                                                    \
    FUNCX(propApp_processCnt, "processCnt", sim_propertytype_int, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, "Processes", "Overall processes")                                                                         \
    FUNCX(propApp_consoleVerbosity, "consoleVerbosity", sim_propertytype_int, 0, "Console verbosity", "")                                                                                                                                     \
    FUNCX(propApp_statusbarVerbosity, "statusbarVerbosity", sim_propertytype_int, 0, "Statusbar verbosity", "")                                                                                                                               \
    FUNCX(propApp_dialogVerbosity, "dialogVerbosity", sim_propertytype_int, 0, "Dialog verbosity", "")                                                                                                                                        \
    FUNCX(propApp_consoleVerbosityStr, "consoleVerbosityStr", sim_propertytype_string, sim_propertyinfo_notreadable | sim_propertyinfo_modelhashexclude, "Console verbosity string", "Console verbosity string, only for client app")         \
    FUNCX(propApp_statusbarVerbosityStr, "statusbarVerbosityStr", sim_propertytype_string, sim_propertyinfo_notreadable | sim_propertyinfo_modelhashexclude, "Statusbar verbosity string", "Statusbar verbosity string, only for client app") \
    FUNCX(propApp_dialogVerbosityStr, "dialogVerbosityStr", sim_propertytype_string, sim_propertyinfo_notreadable | sim_propertyinfo_modelhashexclude, "Dialog verbosity string", "Dialog verbosity string, only for client app")             \
    FUNCX(propApp_auxAddOn1, "auxAddOn1", sim_propertytype_string, sim_propertyinfo_modelhashexclude, "Aux. add-on 1", "Auxiliary add-on 1")             \
    FUNCX(propApp_auxAddOn2, "auxAddOn2", sim_propertytype_string, sim_propertyinfo_modelhashexclude, "Aux. add-on 2", "Auxiliary add-on 2")             \
    FUNCX(propApp_startupCode, "startupCode", sim_propertytype_string, sim_propertyinfo_modelhashexclude, "Start-up code", "")             \
    FUNCX(propApp_defaultTranslationStepSize, "defaultTranslationStepSize", sim_propertytype_float, 0, "Translation step size", "Default translation step size")                                                                              \
    FUNCX(propApp_defaultRotationStepSize, "defaultRotationStepSize", sim_propertytype_float, 0, "Rotation step size", "Default rotation step size")                                                                                          \
    FUNCX(propApp_hierarchyEnabled, "hierarchyEnabled", sim_propertytype_bool, sim_propertyinfo_modelhashexclude, "Hierarchy enabled", "")                                                                                                    \
    FUNCX(propApp_browserEnabled, "browserEnabled", sim_propertytype_bool, sim_propertyinfo_modelhashexclude, "Browser enabled", "")                                                                                                          \
    FUNCX(propApp_displayEnabled, "displayEnabled", sim_propertytype_bool, sim_propertyinfo_modelhashexclude, "Display enabled", "")                                                                                                          \
    FUNCX(propApp_appDir, "appPath", sim_propertytype_string, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, "Application path", "")                                                                                       \
    FUNCX(propApp_machineId, "machineId", sim_propertytype_string, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, "Machine ID", "")                                                                                        \
    FUNCX(propApp_legacyMachineId, "legacyMachineId", sim_propertytype_string, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, "Legacy machine ID", "")                                                                     \
    FUNCX(propApp_tempDir, "tempPath", sim_propertytype_string, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, "Temporary path", "")                                                                                       \
    FUNCX(propApp_sceneTempDir, "sceneTempPath", sim_propertytype_string, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, "Scene temporary path", "")                                                                       \
    FUNCX(propApp_settingsDir, "settingsPath", sim_propertytype_string, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, "Settings path", "")                                                                                \
    FUNCX(propApp_luaDir, "luaPath", sim_propertytype_string, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, "Lua path", "")                                                                                               \
    FUNCX(propApp_pythonDir, "pythonPath", sim_propertytype_string, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, "Python path", "")                                                                                      \
    FUNCX(propApp_mujocoDir, "mujocoPath", sim_propertytype_string, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, "MuJoCo path", "")                                                                                      \
    FUNCX(propApp_systemDir, "systemPath", sim_propertytype_string, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, "System path", "")                                                                                      \
    FUNCX(propApp_resourceDir, "resourcePath", sim_propertytype_string, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, "Resource path", "")                                                                                \
    FUNCX(propApp_addOnDir, "addOnPath", sim_propertytype_string, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, "Add-on path", "")                                                                                        \
    FUNCX(propApp_sceneDir, "scenePath", sim_propertytype_string, sim_propertyinfo_modelhashexclude, "Scene path", "")                                                                                                                        \
    FUNCX(propApp_modelDir, "modelPath", sim_propertytype_string, sim_propertyinfo_modelhashexclude, "Model path", "")                                                                                                                        \
    FUNCX(propApp_importExportDir, "importExportPath", sim_propertytype_string, sim_propertyinfo_modelhashexclude, "Import/export path", "")                                                                                                  \
    FUNCX(propApp_defaultPython, "defaultPython", sim_propertytype_string, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, "Default Python", "Default Python interpreter")                                                  \
    FUNCX(propApp_sandboxLang, "sandboxLang", sim_propertytype_string, sim_propertyinfo_notwritable, "Sandbox language", "Default sandbox language")                                                                                          \
    FUNCX(propApp_headlessMode, "headlessMode", sim_propertytype_int, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, "Headless mode", "Headless mode (0: not headless, 1: GUI suppressed, 2: headless library)")           \
    FUNCX(propApp_canSave, "canSave", sim_propertytype_bool, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, "Can save", "Whether save operation is allowed in given state")                                                \
    FUNCX(propApp_idleFps, "idleFps", sim_propertytype_int, sim_propertyinfo_modelhashexclude, "Idle FPS", "Desired maximum rendering frames per second, when simulation is not running")                                                     \
    FUNCX(propApp_appArg1, "appArg1", sim_propertytype_string, sim_propertyinfo_modelhashexclude, "App arg. 1", "")                                                                                                                           \
    FUNCX(propApp_appArg2, "appArg2", sim_propertytype_string, sim_propertyinfo_modelhashexclude, "App arg. 2", "")                                                                                                                           \
    FUNCX(propApp_appArg3, "appArg3", sim_propertytype_string, sim_propertyinfo_modelhashexclude, "App arg. 3", "")                                                                                                                           \
    FUNCX(propApp_appArg4, "appArg4", sim_propertytype_string, sim_propertyinfo_modelhashexclude, "App arg. 4", "")                                                                                                                           \
    FUNCX(propApp_appArg5, "appArg5", sim_propertytype_string, sim_propertyinfo_modelhashexclude, "App arg. 5", "")                                                                                                                           \
    FUNCX(propApp_appArg6, "appArg6", sim_propertytype_string, sim_propertyinfo_modelhashexclude, "App arg. 6", "")                                                                                                                           \
    FUNCX(propApp_appArg7, "appArg7", sim_propertytype_string, sim_propertyinfo_modelhashexclude, "App arg. 7", "")                                                                                                                           \
    FUNCX(propApp_appArg8, "appArg8", sim_propertytype_string, sim_propertyinfo_modelhashexclude, "App arg. 8", "")                                                                                                                           \
    FUNCX(propApp_appArg9, "appArg9", sim_propertytype_string, sim_propertyinfo_modelhashexclude, "App arg. 9", "")                                                                                                                           \
    FUNCX(propApp_randomQuaternion, "randomQuaternion", sim_propertytype_quaternion, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, "Random quaternion", "")                                                               \
    FUNCX(propApp_randomFloat, "randomFloat", sim_propertytype_float, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, "Random number", "")                                                                                  \
    FUNCX(propApp_randomString, "randomString", sim_propertytype_string, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, "Random string", "")                                                                               \
    FUNCX(propApp_notifyDeprecated, "notifyDeprecated", sim_propertytype_int, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, "Notify deprecated", "Notify deprecated API (0: off, 1: light, 2: full)")                     \
    FUNCX(propApp_execUnsafe, "execUnsafe", sim_propertytype_bool, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, "Execute unsafe", "")                                                                                    \
    FUNCX(propApp_execUnsafeExt, "execUnsafeExt", sim_propertytype_bool, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, "Execute unsafe extended", "Execute unsafe for code triggered externally")                         \
    FUNCX(propApp_dongleSerial, "dongleSerial", sim_propertytype_string, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, "", "")                                                                                            \
    FUNCX(propApp_machineSerialND, "machineSerialND", sim_propertytype_string, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, "", "")                                                                                      \
    FUNCX(propApp_machineSerial, "machineSerial", sim_propertytype_string, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, "", "")                                                                                          \
    FUNCX(propApp_dongleID, "dongleID", sim_propertytype_string, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, "", "")                                                                                                    \
    FUNCX(propApp_machineIDX, "machineIDX", sim_propertytype_string, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, "", "")                                                                                                \
    FUNCX(propApp_machineID0, "machineID0", sim_propertytype_string, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, "", "")                                                                                                \
    FUNCX(propApp_machineID1, "machineID1", sim_propertytype_string, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, "", "")                                                                                                \
    FUNCX(propApp_machineID2, "machineID2", sim_propertytype_string, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, "", "")                                                                                                \
    FUNCX(propApp_machineID3, "machineID3", sim_propertytype_string, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, "", "")                                                                                                \
    FUNCX(propApp_pid, "pid", sim_propertytype_long, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, "PID", "")

#define FUNCX(name, str, v1, v2, t1, t2) const SProperty name = {str, v1, v2, t1, t2};
DEFINE_PROPERTIES
#undef FUNCX
#define FUNCX(name, str, v1, v2, t1, t2) name,
const std::vector<SProperty> allProps_app = {DEFINE_PROPERTIES};
#undef FUNCX
#undef DEFINE_PROPERTIES
// ----------------------------------------------------------------------------------------------

#define proptypetag_bool "&bool&."
#define proptypetag_int "&int&."
#define proptypetag_float "&dbl&."
#define proptypetag_string "&str&."
#define proptypetag_buffer ""
#define proptypetag_intarray2 "&ivect2&."
#define proptypetag_long "&lng&."
#define proptypetag_vector2 "&vect2&."
#define proptypetag_vector3 "&vect3&."
#define proptypetag_quaternion "&quat&."
#define proptypetag_pose "&pose&."
#define proptypetag_matrix3x3 "&mtrx33&."
#define proptypetag_matrix4x4 "&mtrx44&."
#define proptypetag_color "&col&."
#define proptypetag_floatarray "&vect&."
#define proptypetag_intarray "&ivect&."
#define proptypetag_table "&tbl&."
#define proptypetag_matrix "&mtrxXX&."
#define proptypetag_array "&arr&."
#define proptypetag_map "&map&."
#define proptypetag_null "&nul&."
#define proptypetag_handle "&han&."
#define proptypetag_handlearray "&hanvect&."

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
    static int setHandleArrayProperty(long long int target, const char* pName, const long long int* v, int vL);
    static int getHandleArrayProperty(long long int target, const char* pName, std::vector<long long int>& pState);
    static int removeProperty(long long int target, const char* pName);
    static int getPropertyName(long long int target, int& index, std::string& pName, std::string& appartenance, bool staticParsing);
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
