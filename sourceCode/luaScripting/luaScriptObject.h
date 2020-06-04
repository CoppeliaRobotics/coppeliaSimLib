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

#define DEFAULT_MAINSCRIPT_NAME "dltmscpt.txt"
#define DEFAULT_NONTHREADEDCHILDSCRIPT_NAME "dltcscpt.txt"
#define DEFAULT_THREADEDCHILDSCRIPT_NAME "dlttscpt.txt"
#define DEFAULT_CUSTOMIZATIONSCRIPT_NAME "defaultCustomizationScript.txt"

class CLuaScriptObject
{
public:
    CLuaScriptObject(int scriptTypeOrMinusOneForSerialization);
    virtual ~CLuaScriptObject();

    void initializeInitialValues(bool simulationIsRunning);
    void simulationAboutToStart();
    void simulationAboutToEnd();
    void simulationEnded();

    int getScriptID() const;
    int getScriptUniqueID() const;
    void setScriptID(int newID);
    bool isSceneScript() const;

    std::string getDescriptiveName() const;
    std::string getShortDescriptiveName() const;
    std::string getScriptSuffixNumberString() const;
    std::string getScriptPseudoName() const;
    void setAddOnName(const char* name);
    std::string getAddOnName() const;

    CLuaScriptObject* copyYourself();
    void serialize(CSer& ar);
    void performSceneObjectLoadingMapping(const std::vector<int>* map);
    bool announceSceneObjectWillBeErased(int objectID,bool copyBuffer);
    int flagScriptForRemoval();
    void setObjectIDThatScriptIsAttachedTo(int newObjectID);
    int getObjectIDThatScriptIsAttachedTo() const;
    int getObjectIDThatScriptIsAttachedTo_child() const; // for child scripts
    int getObjectIDThatScriptIsAttachedTo_customization() const; // for customization scripts


    void setScriptText(const char* scriptTxt);
    bool setScriptTextFromFile(const char* filename);
    const char* getScriptText();

    void setCalledInThisSimulationStep(bool c);
    bool getCalledInThisSimulationStep() const;

    int runMainScript(int optionalCallType,const CInterfaceStack* inStack,CInterfaceStack* outStack,bool* functionPresent);
    bool launchThreadedChildScript();
    int resumeThreadedChildScriptIfLocationMatch(int resumeLocation);
    int runNonThreadedChildScript(int callType,const CInterfaceStack* inStack,CInterfaceStack* outStack);
    int runCustomizationScript(int callType,const CInterfaceStack* inStack,CInterfaceStack* outStack);
    int runAddOn(int callType,const CInterfaceStack* inStack,CInterfaceStack* outStack);
    int runSandboxScript(int callType,const CInterfaceStack* inStack,CInterfaceStack* outStack);
    bool runSandboxScript_beforeMainScript();


    void handleDebug(const char* funcName,const char* funcType,bool inCall,bool sysCall);

    int callScriptFunction(const char* functionName, SLuaCallBack* pdata);
    int callScriptFunctionEx(const char* functionName,CInterfaceStack* stack);
    int setScriptVariable(const char* variableName,CInterfaceStack* stack);
    int clearScriptVariable(const char* variableName); // deprecated
    int executeScriptString(const char* scriptString,CInterfaceStack* stack);
    int appendTableEntry(const char* arrayName,const char* keyName,const char* data,const int what[2]); // deprecated

    void prefixWithLuaLocationName(std::string& message);
    void decomposeLuaMessage(const char* message,std::string& locationName,std::string& nakedMessage);

    bool killLuaState();
    bool hasLuaState() const;
    bool isSimulationScript() const;
    int getNumberOfPasses() const;
    void setNumberOfPasses(int p);
    int setUserData(char* data);
    char* getUserData(int id) const;
    std::string getLuaSearchPath() const;
    void releaseUserData(int id);
    void clearAllUserData();
    void setThreadedExecution(bool threadedExec);
    bool getThreadedExecution() const;
    bool getThreadedExecutionIsUnderWay() const;
    void setExecutionOrder(int order);
    int getExecutionOrder() const;
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
    void setDisableCustomizationScriptWithError(bool d);
    bool getDisableCustomizationScriptWithError() const;
    void setExecuteJustOnce(bool justOnce);
    bool getExecuteJustOnce() const;

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

    CUserParameters* getScriptParametersObject_backCompatibility();

    void setCustomizedMainScript(bool customized);
    bool isDefaultMainScript() const;

    std::string errorWithCustomizationScript();
    void setCustomizationScriptIsTemporarilyDisabled(bool disabled);
    bool getCustomizationScriptIsTemporarilyDisabled() const;
    void setCustScriptDisabledDSim_compatibilityMode(bool disabled);
    bool getCustScriptDisabledDSim_compatibilityMode() const;
    void setCustomizationScriptCleanupBeforeSave(bool doIt);
    bool getCustomizationScriptCleanupBeforeSave() const;

    bool hasCustomizationScripAnyChanceToGetExecuted(bool whenSimulationRuns,bool forCleanUpSection) const;

    int getScriptExecutionTimeInMs() const;

    double getRandomDouble();
    void setRandomSeed(unsigned int s);

    std::string getAndClearLastStackTraceback();
    void setLastError(const char* err);
    std::string getAndClearLastError();

    int getAddOnExecutionState() const;

    bool addCommandToOutsideCommandQueue(int commandID,int auxVal1,int auxVal2,int auxVal3,int auxVal4,const float aux2Vals[8],int aux2Count);
    int extractCommandFromOutsideCommandQueue(int auxVals[4],float aux2Vals[8],int& aux2Count);

    static bool emergencyStopButtonPressed;

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

    static bool canCallSystemCallback(int scriptType,bool threaded,int callType);
    static std::string getSystemCallbackString(int calltype,bool callTips);
    static std::string getSystemCallbackExString(int calltype);
    static std::vector<std::string> getAllSystemCallbackStrings(int scriptType,bool threaded,bool callTips);

protected:
    void _displayScriptError(const char* errMsg,bool debugRoutine=false);
    bool _luaLoadBuffer(luaWrap_lua_State* luaState,const char* buff,size_t sz,const char* name);
    int _luaPCall(luaWrap_lua_State* luaState,int nargs,int nresult,int errfunc,const char* funcName);

    int _runMainScript(int optionalCallType,const CInterfaceStack* inStack,CInterfaceStack* outStack,bool* functionPresent);
    int _runMainScriptNow(int callType,const CInterfaceStack* inStack,CInterfaceStack* outStack,bool* functionPresent);
    int _runNonThreadedChildScriptNow(int callType,const CInterfaceStack* inStack,CInterfaceStack* outStack);
    void _launchThreadedChildScriptNow();
    int _runCustomizationScript(int callType,const CInterfaceStack* inStack,CInterfaceStack* outStack);
    int _runAddOn(int callType,const CInterfaceStack* inStack,CInterfaceStack* outStack);
    int _runScriptOrCallScriptFunction(int callType,const CInterfaceStack* inStack,CInterfaceStack* outStack,std::string* errorMsg);
    void _handleSimpleSysExCalls(int callType);

    bool _prepareLuaStateAndCallScriptInitSectionIfNeeded();
    bool _checkIfMixingOldAndNewCallMethods();

    void _insertScriptText(CLuaScriptObject* scriptObject,bool toFront,const char* txt);
    bool _replaceScriptText(CLuaScriptObject* scriptObject,const char* oldTxt,const char* newTxt);
    bool _replaceScriptText(CLuaScriptObject* scriptObject,const char* oldTxt1,const char* oldTxt2,const char* oldTxt3,const char* newTxt);
    bool _replaceScriptTextKeepMiddleUnchanged(CLuaScriptObject* scriptObject,const char* oldTxtStart,const char* oldTxtEnd,const char* newTxtStart,const char* newTxtEnd);
    bool _containsScriptText(CLuaScriptObject* scriptObject,const char* txt);
    std::string extractScriptText(CLuaScriptObject* scriptObject,const char* startLine,const char* endLine,bool discardEndLine);
    void _performNewApiAdjustments(CLuaScriptObject* scriptObject,bool forwardAdjustment);
    std::string _replaceOldApi(const std::string& txt,bool forwardAdjustment);
    int _countOccurences(const std::string& source,const char* word);
    void _splitApiText(const std::string& txt,size_t pos,std::string& beforePart,std::string& apiWord,std::string& afterPart);
    void _adjustScriptText1(CLuaScriptObject* scriptObject,bool doIt,bool doIt2);
    void _adjustScriptText2(CLuaScriptObject* scriptObject,bool doIt);
    void _adjustScriptText3(CLuaScriptObject* scriptObject,bool doIt);
    void _adjustScriptText4(CLuaScriptObject* scriptObject,bool doIt);
    void _adjustScriptText5(CLuaScriptObject* scriptObject,bool doIt);
    void _adjustScriptText6(CLuaScriptObject* scriptObject,bool doIt);
    void _adjustScriptText7(CLuaScriptObject* scriptObject,bool doIt);
    void _adjustScriptText8(CLuaScriptObject* scriptObject,int adjust);
    void _adjustScriptText9(CLuaScriptObject* scriptObject);
    void _adjustScriptText10(CLuaScriptObject* scriptObject,bool doIt);
    void _adjustScriptText11(CLuaScriptObject* scriptObject,bool doIt);

    // Variables that need to be copied and serialized:
    int scriptID;
    int _scriptUniqueId;
    int _scriptType; // sim_scriptproperty_mainscript, etc.
    bool _threadedExecution;
    bool _scriptIsDisabled;
    bool _executeJustOnce;
    bool _mainScriptIsDefaultMainScript;
    bool _disableCustomizationScriptWithError;
    int _executionOrder;
    int _debugLevel;
    bool _inDebug;
    int _treeTraversalDirection;
    int _objectIDAttachedTo;
    bool _calledInThisSimulationStep;

    std::string _scriptText;
    std::string _scriptTextExec; // the one getting executed!
    bool _externalScriptText;

    CUserParameters* _scriptParameters_backCompatibility;
    COutsideCommandQueueForScript* _outsideCommandQueue;
    CCustomData* _customObjectData;
    CCustomData* _customObjectData_tempData; // same as above, but is not serialized (but copied!)

    // Other variables that don't need serialization:
    luaWrap_lua_State* L;
    VTHREAD_ID_TYPE _threadedScript_associatedFiberOrThreadID;
    int _numberOfPasses;
    bool _threadedExecutionUnderWay;
    bool _inExecutionNow;
    int _loadBufferResult;

    bool _flaggedForDestruction;

    bool _customizationScriptIsTemporarilyDisabled;
    bool _custScriptDisabledDSim_compatibilityMode;
    bool _customizationScriptCleanupBeforeSave;
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

    int _messageReportingOverride;

    VMutex _localMutex;
    std::string _addOnName;
    int _addOn_executionState;
    std::mt19937 _randGen;


    std::vector<char*> _userData;
    std::vector<int> _userDataIds;

    bool _initialValuesInitialized;

    int _previousEditionWindowPosAndSize[4];

    bool _warningAboutSimHandleChildScriptAlreadyIssued_oldCompatibility_7_8_2014;
    bool _warning_simRMLPosition_oldCompatibility_30_8_2014;
    bool _warning_simRMLVelocity_oldCompatibility_30_8_2014;
    bool _warning_simGetMpConfigForTipPose_oldCompatibility_21_1_2016;
    bool _warning_simFindIkPath_oldCompatibility_2_2_2016;
    bool _automaticCascadingCallsDisabled_OLD; // reset to false at simulation start!

    std::string _filenameForExternalScriptEditor;

    static int _nextIdForExternalScriptEditor;
    static int _scriptUniqueCounter;
    static VMutex _globalMutex;
    static std::vector<CLuaScriptObject*> toBeCalledByThread;
    static VTHREAD_RETURN_TYPE _startAddressForThreadedScripts(VTHREAD_ARGUMENT_TYPE lpData);
    static std::map<std::string,std::string> _newApiMap;
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
