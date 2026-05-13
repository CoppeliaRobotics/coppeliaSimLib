#pragma once

#include <simLib/simTypes.h>
#include <outsideCommandQueueForScript.h>
#include <interfaceStack.h>
#include <set>
#include <unordered_set>
#include <plugin.h>
#include <random>
#include <propertiesAndMethods.h>
#include <obj.h>

#define PROXY_FUNC_NAME_STR "__proxyFuncName__"
#define DEFAULT_MAINSCRIPT_CODE "-- The main script is not supposed to be modified, except in special cases.\nrequire('defaultMainScript')"
#define DEFAULT_NONTHREADEDCHILDSCRIPT "simulationScript"
#define DEFAULT_THREADEDCHILDSCRIPT "simulationScript-threaded"
#define DEFAULT_NONTHREADEDCUSTOMIZATIONSCRIPT "customizationScript"
#define DEFAULT_THREADEDCUSTOMIZATIONSCRIPT "customizationScript-threaded"
#define SIM_SCRIPT_HANDLE "sim_script_handle"                   // keep this global
#define SIM_DETACHEDSCRIPT_HANDLE "sim_detachedscript_handle"   // keep this global
#define SIM_PLUGIN_NAMESPACES "sim_plugin_namespaces"           // keep this global

// Old:
#include <userParameters.h>
#include <customData_old.h>
#define SIM_SCRIPT_NAME_INDEX_OLD "sim_script_name_index" // keep this global, e.g. not _S.sim_script_name_index

// ----------------------------------------------------------------------------------------------
#define FUNCX(name, str, v1, v2, t1, t2) extern const SProperty name;
DETACHEDSCRIPT_PROPERTIES
#undef FUNCX
extern const std::vector<SProperty> allProps_detachedScript;
// ----------------------------------------------------------------------------------------------

class CSceneObject;

class CDetachedScript : public Obj
{
    friend class CScript;

  public:
    CDetachedScript(int scriptType);
    virtual ~CDetachedScript();

    static void destroy(CDetachedScript* obj, bool registeredObject, bool announceScriptDestruction = true);

    void pushObjectCreationEvent();
    void pushObjectRemoveEvent();
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

    CDetachedScript* copyYourself();

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
    int systemCallScript(int callType, const CInterfaceStack* inStack, CInterfaceStack* outStack, bool addOnManuallyStarted = false);
    int callCustomScriptFunction(const char* functionName, CInterfaceStack* inStack = nullptr, CInterfaceStack* outStack = nullptr, std::string* errorMsg = nullptr);
    bool shouldTemporarilySuspendMainScript();

    int executeScriptString(const char* scriptString, CInterfaceStack* outStack);

    int setBoolProperty(const char* pName, bool pState) override;
    int getBoolProperty(const char* pName, bool& pState) const override;
    int setIntProperty(const char* pName, int pState) override;
    int getIntProperty(const char* pName, int& pState) const override;
    int setLongProperty(const char* pName, long long int pState)override;
    int getLongProperty(const char* pName, long long int& pState) const override;
    int getHandleProperty(const char* pName, long long int& pState) const override;
    int setStringProperty(const char* pName, const std::string& pState)override;
    int getStringProperty(const char* pName, std::string& pState) const override;
    int getStringArrayProperty(const char* pName, std::vector<std::string>& pState) const override;
    int getPropertyName(int& index, std::string& pName, std::string& appartenance, int excludeFlags) const override;
    int getPropertyInfo(const char* pName, int& info, std::string& infoTxt) const override;

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
    bool getAutoRestartOnError() const;
    void setScriptIsDisabled(bool isDisabled);
    bool getScriptIsDisabled() const;
    bool getScriptHasError() const;
    bool getScriptDisabledAndNoErrorRaised() const;
    void getPreviousEditionWindowPosAndSize(int posAndSize[4]) const;
    void setPreviousEditionWindowPosAndSize(const int posAndSize[4]);
    int getSimVersion() const;

    int getScriptExecutionTimeInMs() const;
    void resetScriptExecutionTime();

    double getRandomDouble();
    void setRandomSeed(unsigned int s);

    std::string getAndClearLastStackTraceback();

    int getScriptState() const;
    void setScriptState(int state);

    void setScriptExecPriority(int priority);
    int getScriptExecPriority() const;

    bool addCommandToOutsideCommandQueue(int commandID, int auxVal1, int auxVal2, int auxVal3, int auxVal4, const double aux2Vals[8], int aux2Count);
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
    std::string getAddOnMenuPath() const;

    void printInterpreterStack() const;

    bool hasFunctionHook(const char* sysFunc) const;
    int getFuncAndHookCnt(int sysCall, size_t what) const;
    void setFuncAndHookCnt(int sysCall, size_t what, int cnt);
    int registerFunctionHook(const char* sysFunc, const char* userFunc, bool before);
    void removeFunctionHook(const char* sysFunc, const char* userFunc, bool before);
    bool replaceScriptText(const char* oldTxt, const char* newTxt);

    static void getMatchingFunctions(const char* txt, std::set<std::string>& v, const CDetachedScript* requestOrigin);
    static void getMatchingConstants(const char* txt, std::set<std::string>& v, const CDetachedScript* requestOrigin);
    static std::string getFunctionCalltip(const char* txt, const CDetachedScript* requestOrigin);
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
    static size_t buildOntoInterpreterStack_lua(void* LL, const CInterfaceStack* stack, bool takeOnlyTop, bool interlaceWithTypeInfo = false);
    static int getScriptObjectOrDetachedScriptHandleFromInterpreterState_lua(void* LL);
    static int getDetachedScriptHandleFromInterpreterState_lua(void* LL);
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
    std::string _removeLangTagInCode();
    bool _initInterpreterState(std::string* errorMsg);
    bool _killInterpreterState();
    void _announceErrorWasRaisedAndPossiblyPauseSimulation(const char* errMsg, bool runtimeError);
    bool _loadCode();
    int ___loadCode(const char* code, const char* functionsToFind, std::vector<bool>& functionsFound, std::string* errorMsg);
    int _callSystemScriptFunction(int callType, const CInterfaceStack* inStack, CInterfaceStack* outStack);
    int _callScriptFunction(int sysCallType, const char* functionName, const CInterfaceStack* inStack, CInterfaceStack* outStack, std::string* errorMsg);
    int _callScriptFunc(const char* functionName, const CInterfaceStack* inStack, CInterfaceStack* outStack, std::string* errorMsg);
    bool _execScriptString(const char* scriptString, CInterfaceStack* outStack);
    void _handleInfoCallback();
    void _setScriptHandleToInterpreterState_lua(void* LL);

    int _scriptHandle;        // is unique since 25.11.2022. Unique across scenes for old script, but not for new script objects (with new script objects, scriptHandle is same as scene object)
                              // See Obj::_objectHandle too (which is the detached script handle)
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

    bool _detachedScriptInitialValuesInitialized;
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
    static CInterfaceStackObject* _getObjectFromInterpreterStack_lua(void* LL, int index, std::map<void*, bool>& visitedTables);
    static CInterfaceStackTable* _getTableFromInterpreterStack_lua(void* LL, int index, std::map<void*, bool>& visitedTables);

    static int _getInterpreterStackArraySize_lua(void* LL, int index);
    static int _countInterpreterStackTableEntries_lua(void* LL, int index);
    static void _pushOntoInterpreterStack_lua(void* LL, CInterfaceStackObject* obj, bool pushOnlySimpleTypes = false);
    static void _hookFunction_lua(void* LL, void* arr);
    // -----------------------------

    // Old:
    // *****************************************
    void _performNewApiAdjustments_old(CDetachedScript* detachedScript, bool forwardAdjustment);
    std::string _replaceOldApi(const char* txt, bool forwardAdjustment);
    int _getScriptNameIndexNumber_old() const;
    bool _convertThreadedScriptToCoroutine_old(CDetachedScript* detachedScript, bool execJustOnce);
    void _adjustScriptText1_old(CDetachedScript* detachedScript, bool doIt, bool doIt2);
    void _adjustScriptText2_old(CDetachedScript* detachedScript, bool doIt);
    void _adjustScriptText3_old(CDetachedScript* detachedScript, bool doIt);
    void _adjustScriptText4_old(CDetachedScript* detachedScript, bool doIt);
    void _adjustScriptText5_old(CDetachedScript* detachedScript, bool doIt);
    void _adjustScriptText6_old(CDetachedScript* detachedScript, bool doIt);
    void _adjustScriptText7_old(CDetachedScript* detachedScript, bool doIt);
    void _adjustScriptText10_old(CDetachedScript* detachedScript, bool doIt);
    void _adjustScriptText11_old(CDetachedScript* detachedScript, bool doIt);
    void _adjustScriptText12_old(CDetachedScript* detachedScript, bool doIt);
    void _adjustScriptText13_old(CDetachedScript* detachedScript, bool doIt);
    void _adjustScriptText14_old(CDetachedScript* detachedScript, bool doIt);
    void _adjustScriptText15_old(CDetachedScript* detachedScript, bool doIt);
    void _adjustScriptText16_old(CDetachedScript* detachedScript, bool doIt);
    void _adjustScriptText17_old(CDetachedScript* detachedScript, bool doIt);
    void _detectDeprecated_old(CDetachedScript* detachedScript);
    void _insertScriptText_old(CDetachedScript* detachedScript, bool toFront, const char* txt);
    bool _replaceScriptText_old(CDetachedScript* detachedScript, const char* oldTxt, const char* newTxt);
    bool _replaceScriptText_old(CDetachedScript* detachedScript, const char* oldTxt1, const char* oldTxt2, const char* oldTxt3, const char* newTxt);
    bool _replaceScriptTextKeepMiddleUnchanged_old(CDetachedScript* detachedScript, const char* oldTxtStart, const char* oldTxtEnd, const char* newTxtStart, const char* newTxtEnd);
    bool _containsScriptText_old(CDetachedScript* detachedScript, const char* txt);
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
