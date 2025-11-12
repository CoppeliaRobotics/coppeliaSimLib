#pragma once

#include <simLib/simTypes.h>
#include <outsideCommandQueueForScript.h>
#include <interfaceStack.h>
#include <set>
#include <unordered_set>
#include <plugin.h>
#include <random>

#define DEFAULT_MAINSCRIPT_CODE "-- The main script is not supposed to be modified, except in special cases.\nrequire('defaultMainScript')"
#define DEFAULT_NONTHREADEDCHILDSCRIPT "simulationScript"
#define DEFAULT_THREADEDCHILDSCRIPT "simulationScript-threaded"
#define DEFAULT_NONTHREADEDCUSTOMIZATIONSCRIPT "customizationScript"
#define DEFAULT_THREADEDCUSTOMIZATIONSCRIPT "customizationScript-threaded"
#define SIM_SCRIPT_HANDLE "sim_script_handle"         // keep this global, e.g. not _S.sim_script_handle
#define SIM_PLUGIN_NAMESPACES "sim_plugin_namespaces" // keep this global, e.g. not _S.sim_plugin_handles

// Old:
#include <userParameters.h>
#include <customData_old.h>
#define SIM_SCRIPT_NAME_INDEX_OLD "sim_script_name_index" // keep this global, e.g. not _S.sim_script_name_index

#define SCRIPT_META_METHODS R"("callFunction": "sim-2.callScriptFunction",
        "executeScriptString": "sim-2.executeScriptString",
        "getApiFunc": "sim-2.getApiFunc",
        "getApiInfo": "sim-2.getApiInfo",
        "getStackTraceback": "sim-2.getStackTraceback",
        "init": "sim-2.initScript")"

// ----------------------------------------------------------------------------------------------
// flags: bit0: not writable, bit1: not readable, bit2: removable
#define DEFINE_PROPERTIES                                                                                                                                                   \
    FUNCX(propScriptObj_objectType, "objectType", sim_propertytype_string, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, "Object type", "")             \
    FUNCX(propScriptObj_objectMetaInfo, "objectMetaInfo", sim_propertytype_string, sim_propertyinfo_silent | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, "Object meta information", "")             \
    FUNCX(propScriptObj_scriptDisabled, "scriptDisabled", sim_propertytype_bool, 0, "Enabled", "Script is enabled")                                                         \
    FUNCX(propScriptObj_restartOnError, "restartOnError", sim_propertytype_bool, 0, "Restart", "Restart on error")                                                          \
    FUNCX(propScriptObj_execPriority, "execPriority", sim_propertytype_int, 0, "Execution priority", "")                                                                    \
    FUNCX(propScriptObj_scriptType, "scriptType", sim_propertytype_int, sim_propertyinfo_notwritable, "Type", "Script type")                                                \
    FUNCX(propScriptObj_executionDepth, "executionDepth", sim_propertytype_int, sim_propertyinfo_silent | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, "Execution depth", "")    \
    FUNCX(propScriptObj_scriptState, "scriptState", sim_propertytype_int, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, "State", "Script state")        \
    FUNCX(propScriptObj_language, "language", sim_propertytype_string, sim_propertyinfo_notwritable, "Language", "")                                                        \
    FUNCX(propScriptObj_code, "code", sim_propertytype_string, 0, "Code", "Script content")                                                                                 \
    FUNCX(propScriptObj_scriptName, "scriptName", sim_propertytype_string, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, "Name", "Script name")         \
    FUNCX(propScriptObj_addOnPath, "addOnPath", sim_propertytype_string, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, "Add-on path", "Path of add-on") \
    FUNCX(propScriptObj_addOnMenuPath, "addOnMenuPath", sim_propertytype_string, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, "Add-on menu path", "Menu path of add-on")

#define FUNCX(name, str, v1, v2, t1, t2) const SProperty name = {str, v1, v2, t1, t2};
DEFINE_PROPERTIES
#undef FUNCX
#define FUNCX(name, str, v1, v2, t1, t2) name,
const std::vector<SProperty> allProps_scriptObject = {DEFINE_PROPERTIES};
#undef FUNCX
#undef DEFINE_PROPERTIES
// ----------------------------------------------------------------------------------------------

class CSceneObject;

class CScriptObject
{
    friend class CScript;

  public:
    enum
    {
        scriptState_unloaded = 0,
        scriptState_uninitialized,
        scriptState_initialized,
        scriptState_ended,
        scriptState_error = 8,
        scriptState_suspended = 16 // only add-ons
    };

    CScriptObject(int scriptType);
    virtual ~CScriptObject();

    static void destroy(CScriptObject* obj, bool registeredObject, bool announceScriptDestruction = true);

    void addSpecializedObjectEventData(CCbor* ev);
    int setHandle();
    void initializeInitialValues(bool simulationAlreadyRunning);
    void simulationAboutToStart();
    void simulationAboutToEnd();
    void simulationEnded();

    int getScriptHandle() const;
    long long int getScriptUid() const;
    size_t getSimpleHash() const;

    std::string getDescriptiveName() const;
    std::string getShortDescriptiveName() const;
    void setDisplayAddOnName(const char* name);
    std::string getScriptName() const;

    CScriptObject* copyYourself();

    void serialize(CSer& ar);
    void performScriptLoadingMapping(const std::map<int, int>* map, int opType);
    void performSceneObjectLoadingMapping(const std::map<int, int>* map);
    bool announceSceneObjectWillBeErased(const CSceneObject* object, bool copyBuffer);
    int flagScriptForRemoval();
    void setObjectHandleThatScriptIsAttachedTo(int newObjectHandle);
    int getObjectHandleThatScriptIsAttachedTo(int scriptTypeToConsider) const;

    void setScriptText(const char* scriptTxt, bool toFileIfApplicable = true);
    bool setScriptTextFromFile(const char* filename);
    const char* getScriptText();

    void resetCalledInThisSimulationStep();
    bool getCalledInThisSimulationStep() const;

    int systemCallMainScript(int optionalCallType, const CInterfaceStack* inStack, CInterfaceStack* outStack);
    int systemCallScript(int callType, const CInterfaceStack* inStack, CInterfaceStack* outStack,
                         bool addOnManuallyStarted = false);
    int callCustomScriptFunction(const char* functionName, CInterfaceStack* inOutStack);
    bool shouldTemporarilySuspendMainScript();

    int executeScriptString(const char* scriptString, CInterfaceStack* outStack);

    int setBoolProperty(const char* pName, bool pState);
    int getBoolProperty(const char* pName, bool& pState) const;
    int setIntProperty(const char* pName, int pState);
    int getIntProperty(const char* pName, int& pState) const;
    int setLongProperty(const char* pName, long long int pState);
    int getLongProperty(const char* pName, long long int& pState) const;
    int setStringProperty(const char* pName, const char* pState);
    int getStringProperty(const char* pName, std::string& pState) const;
    int getPropertyName(int& index, std::string& pName, std::string* appartenance) const;
    static int getPropertyName_static(int& index, std::string& pName, std::string* appartenance);
    int getPropertyInfo(const char* pName, int& info, std::string& infoTxt, bool detachedScript) const;
    static int getPropertyInfo_static(const char* pName, int& info, std::string& infoTxt, bool detachedScript);

    void terminateScriptExecutionExternally(bool generateErrorMsg);

    void setIsSceneObjectScript(bool s);
    bool resetScript();
    void initScript();
    bool hasInterpreterState() const;
    bool getIsUpToDate();
    bool isSimulationOrMainScript() const;
    bool isSimulatonCustomizationOrMainScript() const;
    bool isSceneSwitchPersistentScript() const;
    int getNumberOfPasses() const;
    void setNumberOfPasses(int p);
    void flagForDestruction();
    bool getFlaggedForDestruction() const;
    int getScriptType() const;
    void setParentIsProxy(bool isDisabled);
    bool getParentIsProxy() const;
    void setAutoRestartOnError(bool restart);
    void setScriptIsDisabled(bool isDisabled);
    bool getScriptIsDisabled() const;
    bool getScriptHasError() const;
    bool getScriptDisabledAndNoErrorRaised() const;
    void getPreviousEditionWindowPosAndSize(int posAndSize[4]) const;
    void setPreviousEditionWindowPosAndSize(const int posAndSize[4]);

    int getScriptExecutionTimeInMs() const;
    void resetScriptExecutionTime();

    double getRandomDouble();
    void setRandomSeed(unsigned int s);

    std::string getAndClearLastStackTraceback();

    int getScriptState() const;
    void setScriptState(int state);

    void setScriptExecPriority(int priority);
    int getScriptExecPriority() const;

    bool addCommandToOutsideCommandQueue(int commandID, int auxVal1, int auxVal2, int auxVal3, int auxVal4,
                                         const double aux2Vals[8], int aux2Count);
    int extractCommandFromOutsideCommandQueue(int auxVals[4], double aux2Vals[8], int& aux2Count);

    void setEventFilters(const std::map<long long int, std::set<std::string>>& filters);
    bool prepareFilteredEventsBuffer(const std::vector<unsigned char>& input, const std::vector<SEventInf>& inf, std::vector<unsigned char>& output) const;

    bool hasSystemFunction(int callType, bool returnTrueIfNotInitialized = true) const;
    bool hasSystemFunctionOrHook(int callType) const;
    void setTemporarilySuspended(bool s);

    std::string getFilenameForExternalScriptEditor();
    void fromFileToBuffer(); // when using an external editor
    void fromBufferToFile() const;

    bool shouldAutoYield();
    bool canManualYield() const;
    int getDelayForAutoYielding() const;
    void setDelayForAutoYielding(int d);
    int changeAutoYieldingForbidLevel(int dx, bool absolute);
    int getAutoYieldingForbidLevel() const;
    int changeOverallYieldingForbidLevel(int dx, bool absolute);
    std::string getLang() const;
    void setLang(const char* lang);
    void setExecutionDepth(int d);
    int getExecutionDepth() const;
    bool isNotInCopyBuffer() const;

    void loadPluginFuncsAndVars(CPlugin* plug);

    void registerPluginFunctions();
    bool registerPluginVariables(bool onlyRequireStatements);

    bool wasModulePreviouslyUsed(const char* moduleName) const;
    void addUsedModule(const char* moduleName);
    void addModulesDetectedInCode();

    int getAddOnUiMenuHandle() const;
    void setAddOnPath(const char* p);
    std::string getAddOnPath() const;

    void printInterpreterStack() const;

    bool hasFunctionHook(const char* sysFunc) const;
    int getFuncAndHookCnt(int sysCall, size_t what) const;
    void setFuncAndHookCnt(int sysCall, size_t what, int cnt);
    int registerFunctionHook(const char* sysFunc, const char* userFunc, bool before);
    bool replaceScriptText(const char* oldTxt, const char* newTxt);

    static void getMatchingFunctions(const char* txt, std::set<std::string>& v, const CScriptObject* requestOrigin);
    static void getMatchingConstants(const char* txt, std::set<std::string>& v, const CScriptObject* requestOrigin);
    static std::string getFunctionCalltip(const char* txt, const CScriptObject* requestOrigin);
    static bool canCallSystemCallback(int scriptType, bool threadedOld, int callType);
    static bool isSystemCallbackInReverseOrder(int callType);
    static bool isSystemCallbackInterruptible(int callType);
    static int getSystemCallbackFromString(const char* cb);
    static std::string getSystemCallbackString(int calltype, int what);
    static std::vector<int> getAllSystemCallbacks(int scriptType, bool threadedOld);
    static std::vector<std::string> getAllSystemCallbackStrings(int scriptType, int what);

    static void setInExternalCall(int scriptHandle);
    static int getInExternalCall();

    void signalSet(const char* sigName, long long int target = sim_handle_scene);
    void signalRemoved(const char* sigName);

    // Lua specific:
    // -----------------------------
    void registerNewFunctions_lua();
    static void buildFromInterpreterStack_lua(void* LL, CInterfaceStack* stack, int fromPos, int cnt);
    static void buildOntoInterpreterStack_lua(void* LL, const CInterfaceStack* stack, bool takeOnlyTop);
    static int getScriptHandleFromInterpreterState_lua(void* LL);
    static std::string getSearchPath_lua();
    static std::string getSearchCPath_lua();
    // -----------------------------

    // Python specific:
    // -----------------------------
    static std::string getSearchPath_python();
    // -----------------------------

    // Old:
    // *****************************************
    std::string getScriptPseudoName_old() const;
    int setScriptVariable_old(const char* variableName, CInterfaceStack* stack);
    void setObjectCustomData_old(int header, const char* data, int dataLength);
    int getObjectCustomDataLength_old(int header) const;
    void getObjectCustomData_old(int header, char* data) const;
    void setAutomaticCascadingCallsDisabled_old(bool disabled);
    bool getAutomaticCascadingCallsDisabled_old() const;
    CUserParameters* getScriptParametersObject_backCompatibility();
    void setCustScriptDisabledDSim_compatibilityMode_DEPRECATED(bool disabled);
    bool getCustScriptDisabledDSim_compatibilityMode_DEPRECATED() const;
    void setCustomizationScriptCleanupBeforeSave_DEPRECATED(bool doIt);
    bool getCustomizationScriptCleanupBeforeSave_DEPRECATED() const;
    int appendTableEntry_DEPRECATED(const char* arrayName, const char* keyName, const char* data, const int what[2]); // deprecated
    int callScriptFunction_DEPRECATED(const char* functionName, SLuaCallBack* pdata);
    int clearScriptVariable_DEPRECATED(const char* variableName); // deprecated
    void setExecutionPriority_old(int order);
    int getExecutionPriority_old() const;
    static void setScriptNameIndexToInterpreterState_lua_old(void* LL, int index);
    static int getScriptNameIndexFromInterpreterState_lua_old(void* LL);
    // *****************************************

  protected:
    std::string _getScriptTypeN() const;
    std::string _removeLangTagInCode();
    bool _initInterpreterState(std::string* errorMsg);
    bool _killInterpreterState();
    void _announceErrorWasRaisedAndPossiblyPauseSimulation(const char* errMsg, bool runtimeError);
    bool _loadCode();
    int ___loadCode(const char* code, const char* functionsToFind, std::vector<bool>& functionsFound,
                    std::string* errorMsg);
    int _callSystemScriptFunction(int callType, const CInterfaceStack* inStack, CInterfaceStack* outStack);
    int _callScriptFunction(int sysCallType, const char* functionName, const CInterfaceStack* inStack,
                            CInterfaceStack* outStack, std::string* errorMsg);
    int _callScriptFunc(const char* functionName, const CInterfaceStack* inStack, CInterfaceStack* outStack,
                        std::string* errorMsg);
    bool _execScriptString(const char* scriptString, CInterfaceStack* outStack);
    void _handleInfoCallback();

    int _scriptHandle;        // is unique since 25.11.2022. Unique across scenes for old script, but not for new script objects (with new script objects, scriptHandle is same as scene object)
    int _sceneObjectHandle;   // is same as _scriptHandle with the new scene object scripts. With old associated scripts, is handle of scene object this script is associated with. -1 with add-ons and sandbox
    long long int _scriptUid; // unique across all scenes
    int _scriptType;
    bool _tempSuspended;
    bool _sceneObjectScript;
    bool _parentIsProxy;
    bool _scriptIsDisabled;
    std::string _lang;
    bool _autoRestartOnError;
    int _scriptState;
    int _executionDepth;
    int _autoStartAddOn;
    int _addOnUiMenuHandle;
    int _addOnExecPriority; // only for add-ons. Not saved
    std::map<long long int, std::set<std::string>> _eventFilters;

    bool _calledInThisSimulationStep;

    int _timeForNextAutoYielding;
    int _delayForAutoYielding;
    int _forbidAutoYieldingLevel;
    int _forbidOverallYieldingLevel;

    std::string _scriptText;
    std::string _scriptTextExec; // the one getting executed!
    bool _externalScriptText;

    COutsideCommandQueueForScript* _outsideCommandQueue;
    std::unordered_set<std::string> _previouslyUsedModules; // needed for the code editor syntax and calltips

    void* _interpreterState; // !! _interpreterState is not the same for a script when in normal or inside a coroutine !!

    int _numberOfPasses;

    bool _flaggedForDestruction;

    int _timeOfScriptExecutionStart;
    std::string _lastStackTraceback;

    std::vector<bool> _containedSystemCallbacks;
    int _sysFuncAndHookCnt_event[3]; // function, hook before, hook after
    int _sysFuncAndHookCnt_dyn[3];
    int _sysFuncAndHookCnt_contact[3];
    int _sysFuncAndHookCnt_joint[3];
    void _printContext(const char* str, size_t p);

    std::string _addOnPath;     // "D:/coppeliaRobotics/coppeliaSim/addOns/Subdivide large triangles.lua"
    std::string _addOnMenuName; // "Subdivide large triangles"
    std::string _addOnMenuPath; // "Geometry / Mesh >> Subdivide large triangles..."

    std::mt19937 _randGen;

    bool _scriptObjectInitialValuesInitialized;
    int _previousEditionWindowPosAndSize[4];

    std::string _filenameForExternalScriptEditor;
    std::vector<std::string> _functionHooks_before;
    std::vector<std::string> _functionHooks_after;
    int _initFunctionHookCount;

    static int _nextScriptHandle; // for main, sandbox, add-ons and old scripts
    static std::vector<int> _externalScriptCalls;
    static std::map<std::string, std::pair<int, int>> _signalNameToScriptHandle; // a script that created a signal will automatically remove it at state destruction

    // Lua specific:
    // -----------------------------
    int _execSimpleString_safe_lua(void* LL, const char* string);
    int _loadBufferResult_lua;
    bool _loadBuffer_lua(const char* buff, size_t sz, const char* name);
    void _registerNewVariables_lua();
    static CInterfaceStackObject* _generateObjectFromInterpreterStack_lua(void* LL, int index,
                                                                          std::map<void*, bool>& visitedTables);
    static CInterfaceStackTable* _generateTableArrayFromInterpreterStack_lua(void* LL, int index,
                                                                             std::map<void*, bool>& visitedTables);
    static CInterfaceStackTable* _generateTableMapFromInterpreterStack_lua(void* LL, int index,
                                                                           std::map<void*, bool>& visitedTables);
    static int _countInterpreterStackTableEntries_lua(void* LL, int index);
    static void _pushOntoInterpreterStack_lua(void* LL, CInterfaceStackObject* obj);
    static void _hookFunction_lua(void* LL, void* arr);
    static void _setScriptHandleToInterpreterState_lua(void* LL, int h);
    // -----------------------------

    // Old:
    // *****************************************
    void _performNewApiAdjustments_old(CScriptObject* scriptObject, bool forwardAdjustment);
    std::string _replaceOldApi(const char* txt, bool forwardAdjustment);
    int _getScriptNameIndexNumber_old() const;
    bool _convertThreadedScriptToCoroutine_old(CScriptObject* scriptObject, bool execJustOnce);
    void _adjustScriptText1_old(CScriptObject* scriptObject, bool doIt, bool doIt2);
    void _adjustScriptText2_old(CScriptObject* scriptObject, bool doIt);
    void _adjustScriptText3_old(CScriptObject* scriptObject, bool doIt);
    void _adjustScriptText4_old(CScriptObject* scriptObject, bool doIt);
    void _adjustScriptText5_old(CScriptObject* scriptObject, bool doIt);
    void _adjustScriptText6_old(CScriptObject* scriptObject, bool doIt);
    void _adjustScriptText7_old(CScriptObject* scriptObject, bool doIt);
    void _adjustScriptText10_old(CScriptObject* scriptObject, bool doIt);
    void _adjustScriptText11_old(CScriptObject* scriptObject, bool doIt);
    void _adjustScriptText12_old(CScriptObject* scriptObject, bool doIt);
    void _adjustScriptText13_old(CScriptObject* scriptObject, bool doIt);
    void _adjustScriptText14_old(CScriptObject* scriptObject, bool doIt);
    void _adjustScriptText15_old(CScriptObject* scriptObject, bool doIt);
    void _adjustScriptText16_old(CScriptObject* scriptObject, bool doIt);
    void _adjustScriptText17_old(CScriptObject* scriptObject, bool doIt);
    void _detectDeprecated_old(CScriptObject* scriptObject);
    void _insertScriptText_old(CScriptObject* scriptObject, bool toFront, const char* txt);
    bool _replaceScriptText_old(CScriptObject* scriptObject, const char* oldTxt, const char* newTxt);
    bool _replaceScriptText_old(CScriptObject* scriptObject, const char* oldTxt1, const char* oldTxt2, const char* oldTxt3, const char* newTxt);
    bool _replaceScriptTextKeepMiddleUnchanged_old(CScriptObject* scriptObject, const char* oldTxtStart, const char* oldTxtEnd, const char* newTxtStart, const char* newTxtEnd);
    bool _containsScriptText_old(CScriptObject* scriptObject, const char* txt);
    void _splitApiText_old(const char* txt, size_t pos, std::string& beforePart, std::string& apiWord, std::string& afterPart);
    bool _custScriptDisabledDSim_compatibilityMode_DEPRECATED;
    bool _customizationScriptCleanupBeforeSave_DEPRECATED;
    bool _mainScriptIsDefaultMainScript_old; // 16.11.2020
    CUserParameters* _scriptParameters_backCompatibility;
    CCustomData_old* _customObjectData_old;
    bool _automaticCascadingCallsDisabled_old; // reset to false at simulation start!
    int _executionPriority_old;
    static std::map<std::string, std::string> _newApiMap_old;
};

// Old:
// *****************************************
struct SNewApiMapping
{
    std::string oldApi;
    std::string newApi;
};
const extern SNewApiMapping _simApiMapping[];
const extern SNewApiMapping _simBubbleApiMapping[];
const extern SNewApiMapping _simK3ApiMapping[];
const extern SNewApiMapping _simMTBApiMapping[];
const extern SNewApiMapping _simOpenMeshApiMapping[];
const extern SNewApiMapping _simSkeletonApiMapping[];
const extern SNewApiMapping _simQHullApiMapping[];
const extern SNewApiMapping _simRemoteApiApiMapping[];
const extern SNewApiMapping _simRRS1ApiMapping[];
const extern SNewApiMapping _simVisionApiMapping[];
const extern SNewApiMapping _simCamApiMapping[];
const extern SNewApiMapping _simJoyApiMapping[];
const extern SNewApiMapping _simWiiApiMapping[];
const extern SNewApiMapping _simURDFApiMapping[];
const extern SNewApiMapping _simBWFApiMapping[];
const extern SNewApiMapping _simUIApiMapping[];
const extern SNewApiMapping _simROSApiMapping[];
const extern SNewApiMapping _simICPApiMapping[];
const extern SNewApiMapping _simOMPLApiMapping[];
const extern SNewApiMapping _simSDFApiMapping[];
const extern SNewApiMapping _simSurfRecApiMapping[];
const extern SNewApiMapping _simxApiMapping[];
// *****************************************
