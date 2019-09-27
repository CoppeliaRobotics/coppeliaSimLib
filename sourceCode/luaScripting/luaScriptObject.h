#pragma once

#include "luaWrapper.h"
#include "v_repTypes.h"
#include "luaScriptParameters.h"
#include "outsideCommandQueueForScript.h"
#include "vMutex.h"
#include "vThread.h"
#include "customData.h"
#include "interfaceStack.h"
#include "luaScriptObjectBase.h"
#include <random>

#define DEFAULT_MAINSCRIPT_NAME "dltmscpt.txt"
#define DEFAULT_NONTHREADEDCHILDSCRIPT_NAME "dltcscpt.txt"
#define DEFAULT_THREADEDCHILDSCRIPT_NAME "dlttscpt.txt"
#define DEFAULT_CUSTOMIZATIONSCRIPT_NAME "defaultCustomizationScript.txt"

class CLuaScriptObject: public CLuaScriptObjectBase
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
    void perform3DObjectLoadingMapping(std::vector<int>* map);
    bool announce3DObjectWillBeErased(int objectID,bool copyBuffer);
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

    void _displayScriptError(const char* errMsg,int errorType);

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

    CLuaScriptParameters* getScriptParametersObject();

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

    int getErrorReportMode() const;
    void setErrorReportMode(int e);

    double getRandomDouble();
    void setRandomSeed(unsigned int s);

    std::string getLastErrorString() const;
    void setLastErrorString(const char* txt);
    std::string getLastStackTraceback();

    int getAddOnExecutionState() const;

    bool addCommandToOutsideCommandQueue(int commandID,int auxVal1,int auxVal2,int auxVal3,int auxVal4,const float aux2Vals[8],int aux2Count);
    int extractCommandFromOutsideCommandQueue(int auxVals[4],float aux2Vals[8],int& aux2Count);

    static bool emergencyStopButtonPressed;

    void setInsideCustomLuaFunction(bool inside);
    bool getInsideCustomLuaFunction() const;

    bool getContainsJointCallbackFunction() const;
    bool getContainsContactCallbackFunction() const;
    bool getContainsDynCallbackFunction() const;

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

    CLuaScriptParameters* scriptParameters;
    COutsideCommandQueueForScript* _outsideCommandQueue;
    CCustomData* _customObjectData;
    CCustomData* _customObjectData_tempData; // same as above, but is not serialized (but copied!)

    // Other variables that don't need serialization:
    luaWrap_lua_State* L;
    VTHREAD_ID_TYPE _threadedScript_associatedFiberOrThreadID;
    int _numberOfPasses;
    bool _threadedExecutionUnderWay;
    int _insideCustomLuaFunction;
    bool _inExecutionNow;
    int _loadBufferResult;

    bool _flaggedForDestruction;

    bool _customizationScriptIsTemporarilyDisabled;
    bool _custScriptDisabledDSim_compatibilityMode;
    bool _customizationScriptCleanupBeforeSave;
    int _timeOfPcallStart;
    int _errorReportMode;
    std::string _lastErrorString;
    std::string _lastStackTraceback;
    bool _compatibilityModeOrFirstTimeCall_sysCallbacks;
    bool _containsJointCallbackFunction;
    bool _containsContactCallbackFunction;
    bool _containsDynCallbackFunction;

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
};
