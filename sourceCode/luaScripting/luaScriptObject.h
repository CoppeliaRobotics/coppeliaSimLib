#pragma once

#include "luaWrapper.h"
#include "simTypes.h"
#include "userParameters.h"
#include "outsideCommandQueueForScript.h"
#include "vMutex.h"
#include "vThread.h"
#include "customData.h"
#include "interfaceStack.h"
#include <random>

#define DEFAULT_MAINSCRIPT_CODE "-- The main script is not supposed to be modified, except in special cases.\nrequire('defaultMainScript')"
#define DEFAULT_NONTHREADEDCHILDSCRIPT_NAME "dltcscpt.txt"
#define DEFAULT_THREADEDCHILDSCRIPT_NAME "dlttscpt.txt"
#define DEFAULT_THREADEDCHILDSCRIPTOLD_NAME "dlttscptbkcomp.txt"
#define DEFAULT_CUSTOMIZATIONSCRIPT_NAME "defaultCustomizationScript.txt"

#define SIM_SCRIPT_NAME_INDEX "sim_script_name_index" // keep this global, e.g. not _S.sim_script_name_index
#define SIM_SCRIPT_HANDLE "sim_script_handle" // keep this global, e.g. not _S.sim_script_handle

class CLuaScriptObject
{
    enum {
        scriptState_uninitialized=0,
        scriptState_initialized,
        scriptState_ended,
        scriptState_error=8
    };

public:
    CLuaScriptObject(int scriptTypeOrMinusOneForSerialization);
    virtual ~CLuaScriptObject();

    void initializeInitialValues(bool simulationAlreadyRunning);
    void simulationAboutToStart();
    void simulationAboutToEnd();
    void simulationEnded();

    int getScriptHandle() const;
    int getScriptUniqueID() const;
    void setScriptHandle(int newHandle);

    std::string getDescriptiveName() const;
    std::string getShortDescriptiveName() const;
    int getScriptNameIndexNumber() const;
    std::string getScriptPseudoName() const;
    void setAddOnName(const char* name);
    std::string getAddOnName() const;

    CLuaScriptObject* copyYourself();
    void serialize(CSer& ar);
    void performSceneObjectLoadingMapping(const std::vector<int>* map);
    bool announceSceneObjectWillBeErased(int objectHandle,bool copyBuffer);
    int flagScriptForRemoval();
    void setObjectHandleThatScriptIsAttachedTo(int newObjectHandle);
    int getObjectHandleThatScriptIsAttachedTo() const;
    int getObjectHandleThatScriptIsAttachedTo_child() const; // for child scripts
    int getObjectHandleThatScriptIsAttachedTo_customization() const; // for customization scripts

    void setScriptText(const char* scriptTxt);
    bool setScriptTextFromFile(const char* filename);
    const char* getScriptText();

    void resetCalledInThisSimulationStep();
    bool getCalledInThisSimulationStep() const;

    int callMainScript(int optionalCallType,const CInterfaceStack* inStack,CInterfaceStack* outStack,bool* functionPresent);
    int callChildScript(int callType,const CInterfaceStack* inStack,CInterfaceStack* outStack);
    int callCustomizationScript(int callType,const CInterfaceStack* inStack,CInterfaceStack* outStack);
    int callAddOn(int callType,const CInterfaceStack* inStack,CInterfaceStack* outStack);
    int callSandboxScript(int callType,const CInterfaceStack* inStack,CInterfaceStack* outStack);
    bool callSandboxScript_beforeMainScript();

    int callScriptFunction(const char* functionName,CInterfaceStack* stack);
    int setScriptVariable(const char* variableName,CInterfaceStack* stack);
    int executeScriptString(const char* scriptString,CInterfaceStack* stack);

    void terminateScriptExecutionExternally(bool generateErrorMsg);
    void handleDebug(const char* funcName,const char* funcType,bool inCall,bool sysCall);

    bool resetScript();
    bool _killLuaState();
    bool hasLuaState() const;
    bool isSimulationScript() const;
    bool isEmbeddedScript() const;
    bool isSceneSwitchPersistentScript() const;
    int getNumberOfPasses() const;
    void setNumberOfPasses(int p);
    std::string getLuaSearchPath() const;
    void setExecutionPriority(int order);
    int getExecutionPriority() const;
    void setDebugLevel(int l);
    int getDebugLevel() const;
    void setTreeTraversalDirection(int dir);
    int getTreeTraversalDirection() const;
    void flagForDestruction();
    bool getFlaggedForDestruction() const;
    void setAddOnScriptAutoRun();
    int getScriptType() const;
    void setScriptIsDisabled(bool isDisabled);
    bool getScriptIsDisabled() const;
    bool getScriptEnabledAndNoErrorRaised() const;

    void getPreviousEditionWindowPosAndSize(int posAndSize[4]) const;
    void setPreviousEditionWindowPosAndSize(const int posAndSize[4]);

    void setObjectCustomData(int header,const char* data,int dataLength);
    int getObjectCustomDataLength(int header) const;
    void getObjectCustomData(int header,char* data) const;
    bool getObjectCustomDataHeader(int index,int& header) const;

    // Same as above, but data is not serialized (but copied):
    void setObjectCustomData_tempData(int header,const char* data,int dataLength);
    int getObjectCustomDataLength_tempData(int header) const;
    void getObjectCustomData_tempData(int header,char* data) const;
    bool getObjectCustomDataHeader_tempData(int index,int& header) const;

    int getScriptExecutionTimeInMs() const;
    void resetScriptExecutionTime();
    void setRaiseErrors_backCompatibility(bool raise);
    bool getRaiseErrors_backCompatibility() const;

    double getRandomDouble();
    void setRandomSeed(unsigned int s);

    std::string getAndClearLastStackTraceback();
    void setLastError(const char* err);
    std::string getAndClearLastError();

    int getAddOnExecutionState() const;

    bool addCommandToOutsideCommandQueue(int commandID,int auxVal1,int auxVal2,int auxVal3,int auxVal4,const float aux2Vals[8],int aux2Count);
    int extractCommandFromOutsideCommandQueue(int auxVals[4],float aux2Vals[8],int& aux2Count);

    bool getContainsJointCallbackFunction() const;
    bool getContainsContactCallbackFunction() const;
    bool getContainsDynCallbackFunction() const;
    bool getContainsVisionCallbackFunction() const;
    bool getContainsTriggerCallbackFunction() const;
    bool getContainsUserConfigCallbackFunction() const;

    VTHREAD_ID_TYPE getThreadedScriptThreadId() const;

    void setAutomaticCascadingCallsDisabled_OLD(bool disabled);
    bool getAutomaticCascadingCallsDisabled_OLD() const;
    bool checkAndSetWarningAboutSimHandleChildScriptAlreadyIssued_oldCompatibility_7_8_2014();
    bool checkAndSetWarning_simRMLPosition_oldCompatibility_30_8_2014();
    bool checkAndSetWarning_simRMLVelocity_oldCompatibility_30_8_2014();
    bool checkAndSetWarning_simGetMpConfigForTipPose_oldCompatibility_21_1_2016();
    bool checkAndSetWarning_simFindIkPath_oldCompatibility_2_2_2016();

    std::string getFilenameForExternalScriptEditor() const;
    void fromFileToBuffer(); // when using an external editor
    void fromBufferToFile() const;

    bool shouldAutoYield();
    bool canManualYield() const;
    void setDelayForAutoYielding(int d);
    int changeAutoYieldingForbidLevel(int dx,bool absolute);
    int getAutoYieldingForbidLevel() const;
    int changeOverallYieldingForbidLevel(int dx,bool absolute);

    static bool canCallSystemCallback(int scriptType,bool threadedOld,int callType);
    static std::string getSystemCallbackString(int calltype,bool callTips);
    static std::string getSystemCallbackExString(int calltype);
    static std::vector<std::string> getAllSystemCallbackStrings(int scriptType,bool threaded,bool callTips);

    static int getScriptHandleFromLuaState(luaWrap_lua_State* L);
    static void setScriptNameIndexToLuaState(luaWrap_lua_State* L,int index);
    static int getScriptNameIndexFromLuaState(luaWrap_lua_State* L);

    CUserParameters* getScriptParametersObject_backCompatibility();
    void setCustScriptDisabledDSim_compatibilityMode_DEPRECATED(bool disabled);
    bool getCustScriptDisabledDSim_compatibilityMode_DEPRECATED() const;
    void setCustomizationScriptCleanupBeforeSave_DEPRECATED(bool doIt);
    bool getCustomizationScriptCleanupBeforeSave_DEPRECATED() const;
    int appendTableEntry_DEPRECATED(const char* arrayName,const char* keyName,const char* data,const int what[2]); // deprecated
    int callScriptFunction_DEPRECATED(const char* functionName, SLuaCallBack* pdata);
    int clearScriptVariable_DEPRECATED(const char* variableName); // deprecated
    void setThreadedExecution_oldThreads(bool threadedExec);
    bool getThreadedExecution_oldThreads() const;
    bool getThreadedExecutionIsUnderWay_oldThreads() const;
    void setExecuteJustOnce_oldThreads(bool justOnce);
    bool getExecuteJustOnce_oldThreads() const;
    bool launchThreadedChildScript_oldThreads();
    int resumeThreadedChildScriptIfLocationMatch_oldThreads(int resumeLocation);

protected:
    static void _luaHookFunc(luaWrap_lua_State* L,luaWrap_lua_Debug* ar);
    static std::string _getAdditionalLuaSearchPath();
    static void _setScriptHandleToLuaState(luaWrap_lua_State* L,int h);

    void _initLuaState();

    void _announceErrorWasRaisedAndDisableScript(const char* errMsg,bool runtimeError,bool debugRoutine=false);
    bool _luaLoadBuffer(luaWrap_lua_State* luaState,const char* buff,size_t sz,const char* name);
    int _luaPCall(luaWrap_lua_State* luaState,int nargs,int nresult,int errfunc,const char* funcName);

    int _callMainScript(int optionalCallType,const CInterfaceStack* inStack,CInterfaceStack* outStack,bool* functionPresent);
    int _callMainScriptNow(int callType,const CInterfaceStack* inStack,CInterfaceStack* outStack,bool* functionPresent);
    int _callChildScriptNow(int callType,const CInterfaceStack* inStack,CInterfaceStack* outStack);
    int _callAddOn(int callType,const CInterfaceStack* inStack,CInterfaceStack* outStack);
    int _callScriptFunction(int callType,const CInterfaceStack* inStack,CInterfaceStack* outStack);
    void _handleSimpleSysExCalls(int callType);

    bool _checkIfMixingOldAndNewCallMethods();

    void _insertScriptText(CLuaScriptObject* scriptObject,bool toFront,const char* txt);
    bool _replaceScriptText(CLuaScriptObject* scriptObject,const char* oldTxt,const char* newTxt);
    bool _replaceScriptText(CLuaScriptObject* scriptObject,const char* oldTxt1,const char* oldTxt2,const char* oldTxt3,const char* newTxt);
    bool _replaceScriptTextKeepMiddleUnchanged(CLuaScriptObject* scriptObject,const char* oldTxtStart,const char* oldTxtEnd,const char* newTxtStart,const char* newTxtEnd);
    bool _containsScriptText(CLuaScriptObject* scriptObject,const char* txt);
    std::string extractScriptText(CLuaScriptObject* scriptObject,const char* startLine,const char* endLine,bool discardEndLine);
    void _performNewApiAdjustments(CLuaScriptObject* scriptObject,bool forwardAdjustment);
    std::string _replaceOldApi(const char* txt,bool forwardAdjustment);
    int _countOccurences(const char* source,const char* word);
    void _splitApiText(const char* txt,size_t pos,std::string& beforePart,std::string& apiWord,std::string& afterPart);
    void _adjustScriptText1(CLuaScriptObject* scriptObject,bool doIt,bool doIt2);
    void _adjustScriptText2(CLuaScriptObject* scriptObject,bool doIt);
    void _adjustScriptText3(CLuaScriptObject* scriptObject,bool doIt);
    void _adjustScriptText4(CLuaScriptObject* scriptObject,bool doIt);
    void _adjustScriptText5(CLuaScriptObject* scriptObject,bool doIt);
    void _adjustScriptText6(CLuaScriptObject* scriptObject,bool doIt);
    void _adjustScriptText7(CLuaScriptObject* scriptObject,bool doIt);
    void _adjustScriptText8(CLuaScriptObject* scriptObject);
    void _adjustScriptText9(CLuaScriptObject* scriptObject);
    void _adjustScriptText10(CLuaScriptObject* scriptObject,bool doIt);
    void _adjustScriptText11(CLuaScriptObject* scriptObject,bool doIt);
    void _adjustScriptText12(CLuaScriptObject* scriptObject,bool doIt);
    void _adjustScriptText13(CLuaScriptObject* scriptObject,bool doIt);
    bool _convertThreadedScriptToCoroutine(CLuaScriptObject* scriptObject);

    int _scriptHandle;
    int _scriptUniqueId;
    int _scriptType;
    bool _scriptIsDisabled;
    int _scriptState;
    bool _mainScriptIsDefaultMainScript_old; // 16.11.2020
    int _executionPriority;
    int _debugLevel;
    bool _inDebug;
    bool _raiseErrors_backCompatibility;
    int _treeTraversalDirection;
    int _objectHandleAttachedTo;

    bool _calledInThisSimulationStep;

    int _timeForNextAutoYielding;
    int _delayForAutoYielding;
    int _forbidAutoYieldingLevel;
    int _forbidOverallYieldingLevel;

    std::string _scriptText;
    std::string _scriptTextExec; // the one getting executed!
    bool _externalScriptText;

    CUserParameters* _scriptParameters_backCompatibility;
    COutsideCommandQueueForScript* _outsideCommandQueue;
    CCustomData* _customObjectData;
    CCustomData* _customObjectData_tempData; // same as above, but is not serialized (but copied!)

    // Other variables that don't need serialization:
    luaWrap_lua_State* L;
    int _numberOfPasses;
    bool _inExecutionNow;
    int _loadBufferResult;

    bool _flaggedForDestruction;

    bool _custScriptDisabledDSim_compatibilityMode_DEPRECATED;
    bool _customizationScriptCleanupBeforeSave_DEPRECATED;
    int _timeOfPcallStart;
    std::string _lastStackTraceback;
    std::string _lastError;
    bool _compatibilityModeOrFirstTimeCall_sysCallbacks;
    bool _containsJointCallbackFunction;
    bool _containsContactCallbackFunction;
    bool _containsDynCallbackFunction;
    bool _containsVisionCallbackFunction;
    bool _containsTriggerCallbackFunction;
    bool _containsUserConfigCallbackFunction;
    void _printContext(const char* str,size_t p);

    std::string _addOnName;
    int _addOn_executionState;
    std::mt19937 _randGen;

    bool _initialValuesInitialized;
    int _previousEditionWindowPosAndSize[4];

    std::string _filenameForExternalScriptEditor;


    static int _nextIdForExternalScriptEditor;
    static int _scriptUniqueCounter;
    static std::map<std::string,std::string> _newApiMap;


    bool _warningAboutSimHandleChildScriptAlreadyIssued_oldCompatibility_7_8_2014;
    bool _warning_simRMLPosition_oldCompatibility_30_8_2014;
    bool _warning_simRMLVelocity_oldCompatibility_30_8_2014;
    bool _warning_simGetMpConfigForTipPose_oldCompatibility_21_1_2016;
    bool _warning_simFindIkPath_oldCompatibility_2_2_2016;
    bool _automaticCascadingCallsDisabled_OLD; // reset to false at simulation start!
    bool _threadedExecution_oldThreads;
    VTHREAD_ID_TYPE _threadedScript_associatedFiberOrThreadID_oldThreads;
    bool _threadedExecutionUnderWay_oldThreads;
    bool _executeJustOnce_oldThreads;
    void _launchThreadedChildScriptNow_oldThreads();

    static VMutex _globalMutex_oldThreads;
    static std::vector<CLuaScriptObject*> toBeCalledByThread_oldThreads;
    static VTHREAD_RETURN_TYPE _startAddressForThreadedScripts_oldThreads(VTHREAD_ARGUMENT_TYPE lpData);
};

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
