#pragma once

#include "vrepMainHeader.h"
#include "luaWrapper.h"
#include "vMutex.h"
#ifndef WIN_VREP
#include <pthread.h>
#endif

class CLuaScriptObject;
class CInterfaceStack;
class CLuaCustomFunction;

enum {lua_arg_empty,lua_arg_nil,lua_arg_number,lua_arg_bool,lua_arg_string,lua_arg_function,lua_arg_userdata,lua_arg_table};

struct SLuaCommands
{
    std::string name;
    luaWrap_lua_CFunction func;
    std::string callTip;
    bool autoComplete;
};

struct SLuaVariables
{
    std::string name;
    int val;
    bool autoComplete;
};


luaWrap_lua_State* initializeNewLuaState(const char* scriptSuffixNumberString,int debugLevel);
std::string getAdditionalLuaSearchPath();
void registerTableFunction(luaWrap_lua_State* L,char const* const tableName,char const* const functionName,luaWrap_lua_CFunction functionCallback);
void registerNewLuaFunctions(luaWrap_lua_State* L);
void prepareNewLuaVariables_onlyRequire(luaWrap_lua_State* L);
void prepareNewLuaVariables_noRequire(luaWrap_lua_State* L);
void setNewLuaVariable(luaWrap_lua_State* L,const char* name,int identifier);

void pushCorrectTypeOntoLuaStack(luaWrap_lua_State* L,const std::string& txt);
int getCorrectType(const std::string& txt);

bool readCustomFunctionDataFromStack(luaWrap_lua_State* L,int ind,int dataType,
                                     std::vector<char>& inBoolVector,
                                     std::vector<int>& inIntVector,
                                     std::vector<float>& inFloatVector,
                                     std::vector<double>& inDoubleVector,
                                     std::vector<std::string>& inStringVector,
                                     std::vector<std::string>& inCharVector,
                                    std::vector<int>& inInfoVector);
void writeCustomFunctionDataOntoStack(luaWrap_lua_State* L,int dataType,int dataSize,
                                      unsigned char* boolData,int& boolDataPos,
                                      int* intData,int& intDataPos,
                                      float* floatData,int& floatDataPos,
                                      double* doubleData,int& doubleDataPos,
                                      char* stringData,int& stringDataPos,
                                      char* charData,int& charDataPos);



void getFloatsFromTable(luaWrap_lua_State* L,int tablePos,int floatCount,float* arrayField);
void getDoublesFromTable(luaWrap_lua_State* L,int tablePos,int doubleCount,double* arrayField);
bool getIntsFromTable(luaWrap_lua_State* L,int tablePos,int intCount,int* arrayField);
bool getUIntsFromTable(luaWrap_lua_State* L,int tablePos,int intCount,unsigned int* arrayField);
bool getUCharsFromTable(luaWrap_lua_State* L,int tablePos,int intCount,unsigned char* arrayField);
void getBoolsFromTable(luaWrap_lua_State* L,int tablePos,int boolCount,char* arrayField);
void pushFloatTableOntoStack(luaWrap_lua_State* L,int floatCount,const float* arrayField);
void pushDoubleTableOntoStack(luaWrap_lua_State* L,int doubleCount,const double* arrayField);
void pushIntTableOntoStack(luaWrap_lua_State* L,int intCount,const int* arrayField);
void pushUIntTableOntoStack(luaWrap_lua_State* L,int intCount,const unsigned int* arrayField);
void pushUCharTableOntoStack(luaWrap_lua_State* L,int intCount,const unsigned char* arrayField);
void pushStringTableOntoStack(luaWrap_lua_State* L,const std::vector<std::string>& stringTable);
void pushLStringTableOntoStack(luaWrap_lua_State* L,const std::vector<std::string>& stringTable);
void insertFloatsIntoTableAlreadyOnStack(luaWrap_lua_State* L,int tablePos,int floatCount,const float* arrayField);


int luaToInt(luaWrap_lua_State* L,int pos);
float luaToFloat(luaWrap_lua_State* L,int pos);
double luaToDouble(luaWrap_lua_State* L,int pos);
bool luaToBool(luaWrap_lua_State* L,int pos);

bool isDashFree(const std::string& functionName,const std::string& name);
bool suffixAdjustStringIfNeeded(const std::string& functionName,bool outputError,luaWrap_lua_State* L,std::string& name);
int getCurrentScriptID(luaWrap_lua_State* L);

void getScriptTree_mainOr(luaWrap_lua_State* L,bool selfIncluded,std::vector<int>& scriptHandles);
void getScriptChain(luaWrap_lua_State* L,bool selfIncluded,bool mainIncluded,std::vector<int>& scriptHandles);

void luaApiCallWarning(const char* functionName,const char* message);
bool _hasErrors(std::string& funcErrorString);

void memorizeLocation(luaWrap_lua_State* L);
int getLocationIndex(VTHREAD_ID_TYPE threadID);
void forgetLocation();
std::string getLocationString();


// Input argument checking:
bool checkInputArguments(luaWrap_lua_State* L,std::string* errStr,
                         int type1=lua_arg_empty,int type1Cnt_zeroIfNotTable=-2,
                         int type2=lua_arg_empty,int type2Cnt_zeroIfNotTable=-2,
                         int type3=lua_arg_empty,int type3Cnt_zeroIfNotTable=-2,
                         int type4=lua_arg_empty,int type4Cnt_zeroIfNotTable=-2,
                         int type5=lua_arg_empty,int type5Cnt_zeroIfNotTable=-2,
                         int type6=lua_arg_empty,int type6Cnt_zeroIfNotTable=-2,
                         int type7=lua_arg_empty,int type7Cnt_zeroIfNotTable=-2,
                         int type8=lua_arg_empty,int type8Cnt_zeroIfNotTable=-2,
                         int type9=lua_arg_empty,int type9Cnt_zeroIfNotTable=-2,
                         int type10=lua_arg_empty,int type10Cnt_zeroIfNotTable=-2,
                         int type11=lua_arg_empty,int type11Cnt_zeroIfNotTable=-2);
int checkOneGeneralInputArgument(luaWrap_lua_State* L,int index,
                           int type,int cnt_orZeroIfNotTable,bool optional,bool nilInsteadOfTypeAndCountAllowed,std::string* errStr);
bool checkOneInputArgument(luaWrap_lua_State* L,int index,int type,std::string* errStr);


void luaHookFunction(luaWrap_lua_State* L,luaWrap_lua_Debug* ar);
void moduleCommonPart(luaWrap_lua_State* L,int action,std::string* errorString);


int handleChildScriptsRoutine_OLD(int callType,CLuaScriptObject* it,CInterfaceStack& inputArguments);
int launchThreadedChildScriptsRoutine_OLD(CLuaScriptObject* it);

void appendAllVrepFunctionNames_spaceSeparated(std::string& keywords,int scriptType,bool scriptIsThreaded);
void appendAllVrepVariableNames_spaceSeparated(std::string& keywords);
void pushAllVrepFunctionNamesThatStartSame_autoCompletionList(const std::string& txt,std::vector<std::string>& v,std::map<std::string,bool>& m,int scriptType,bool scriptIsThreaded);
void pushAllVrepVariableNamesThatStartSame_autoCompletionList(const std::string& txt,std::vector<std::string>& v,std::map<std::string,bool>& m);
std::string getVrepFunctionCalltip(const char* txt,int scriptType,bool scriptIsThreaded,bool forceDoNotSupportOldApi);
int isFuncOrConstDeprecated(const char* txt);


int _genericFunctionHandler_new(luaWrap_lua_State* L,CLuaCustomFunction* func,std::string& raiseErrorWithMsg);
int _genericFunctionHandler_old(luaWrap_lua_State* L,CLuaCustomFunction* func);


const extern SLuaCommands simLuaCommands[];
const extern SLuaCommands simLuaCommandsOldApi[];

const extern SLuaVariables simLuaVariables[];
const extern SLuaVariables simLuaVariablesOldApi[];

extern int _simHandleChildScripts(luaWrap_lua_State* L);
extern int _simLaunchThreadedChildScripts(luaWrap_lua_State* L);
extern int _simHandleSensingChildScripts(luaWrap_lua_State* L);
extern int _simGetScriptName(luaWrap_lua_State* L);
extern int _simGetObjectAssociatedWithScript(luaWrap_lua_State* L);
extern int _simGetScriptAssociatedWithObject(luaWrap_lua_State* L);
extern int _simGetCustomizationScriptAssociatedWithObject(luaWrap_lua_State* L);
extern int _simGenericFunctionHandler(luaWrap_lua_State* L);
extern int _simGetScriptExecutionCount(luaWrap_lua_State* L);
extern int _simIsScriptExecutionThreaded(luaWrap_lua_State* L);
extern int _simIsScriptRunningInThread(luaWrap_lua_State* L);
extern int _simOpenModule(luaWrap_lua_State* L);
extern int _simCloseModule(luaWrap_lua_State* L);
extern int _simHandleModule(luaWrap_lua_State* L);
extern int _simBoolOr16(luaWrap_lua_State* L);
extern int _simBoolAnd16(luaWrap_lua_State* L);
extern int _simBoolXor16(luaWrap_lua_State* L);
extern int _simBoolOr32(luaWrap_lua_State* L);
extern int _simBoolAnd32(luaWrap_lua_State* L);
extern int _simBoolXor32(luaWrap_lua_State* L);
extern int _simHandleDynamics(luaWrap_lua_State* L);
extern int _simHandleIkGroup(luaWrap_lua_State* L);
extern int _simCheckIkGroup(luaWrap_lua_State* L);
extern int _simHandleCollision(luaWrap_lua_State* L);
extern int _simReadCollision(luaWrap_lua_State* L);
extern int _simHandleDistance(luaWrap_lua_State* L);
extern int _simReadDistance(luaWrap_lua_State* L);
extern int _simHandleProximitySensor(luaWrap_lua_State* L);
extern int _simReadProximitySensor(luaWrap_lua_State* L);
extern int _simHandleMill(luaWrap_lua_State* L);
extern int _simResetCollision(luaWrap_lua_State* L);
extern int _simResetDistance(luaWrap_lua_State* L);
extern int _simResetProximitySensor(luaWrap_lua_State* L);
extern int _simResetMill(luaWrap_lua_State* L);
extern int _simCheckProximitySensor(luaWrap_lua_State* L);
extern int _simCheckProximitySensorEx(luaWrap_lua_State* L);
extern int _simCheckProximitySensorEx2(luaWrap_lua_State* L);
extern int _simGetObjectHandle(luaWrap_lua_State* L);
extern int _simGetScriptHandle(luaWrap_lua_State* L);
extern int _simGetCollectionHandle(luaWrap_lua_State* L);
extern int _simRemoveCollection(luaWrap_lua_State* L);
extern int _simEmptyCollection(luaWrap_lua_State* L);
extern int _simGetObjectPosition(luaWrap_lua_State* L);
extern int _simGetObjectOrientation(luaWrap_lua_State* L);
extern int _simSetObjectPosition(luaWrap_lua_State* L);
extern int _simSetObjectOrientation(luaWrap_lua_State* L);
extern int _simGetJointPosition(luaWrap_lua_State* L);
extern int _simSetJointPosition(luaWrap_lua_State* L);
extern int _simSetJointTargetPosition(luaWrap_lua_State* L);
extern int _simGetJointTargetPosition(luaWrap_lua_State* L);
extern int _simSetJointForce(luaWrap_lua_State* L);
extern int _simGetPathPosition(luaWrap_lua_State* L);
extern int _simSetPathPosition(luaWrap_lua_State* L);
extern int _simGetPathLength(luaWrap_lua_State* L);
extern int _simSetJointTargetVelocity(luaWrap_lua_State* L);
extern int _simGetJointTargetVelocity(luaWrap_lua_State* L);
extern int _simSetPathTargetNominalVelocity(luaWrap_lua_State* L);
extern int _simGetObjectName(luaWrap_lua_State* L);
extern int _simGetCollectionName(luaWrap_lua_State* L);
extern int _simSetObjectName(luaWrap_lua_State* L);
extern int _simSetCollectionName(luaWrap_lua_State* L);
extern int _simRemoveObject(luaWrap_lua_State* L);
extern int _simRemoveModel(luaWrap_lua_State* L);
extern int _simGetSimulationTime(luaWrap_lua_State* L);
extern int _simGetSimulationState(luaWrap_lua_State* L);
extern int _simGetSystemTime(luaWrap_lua_State* L);
extern int _simGetSystemTimeInMs(luaWrap_lua_State* L);
extern int _simCheckCollision(luaWrap_lua_State* L);
extern int _simCheckCollisionEx(luaWrap_lua_State* L);
extern int _simCheckDistance(luaWrap_lua_State* L);
extern int _simGetObjectConfiguration(luaWrap_lua_State* L);
extern int _simSetObjectConfiguration(luaWrap_lua_State* L);
extern int _simGetConfigurationTree(luaWrap_lua_State* L);
extern int _simSetConfigurationTree(luaWrap_lua_State* L);
extern int _simHandleMechanism(luaWrap_lua_State* L);
extern int _simGetSimulationTimeStep(luaWrap_lua_State* L);
extern int _simGetSimulatorMessage(luaWrap_lua_State* L);
extern int _simAddScript(luaWrap_lua_State* L);
extern int _simAssociateScriptWithObject(luaWrap_lua_State* L);
extern int _simSetScriptText(luaWrap_lua_State* L);
extern int _simResetTracing(luaWrap_lua_State* L);
extern int _simHandleTracing(luaWrap_lua_State* L);
extern int _simResetGraph(luaWrap_lua_State* L);
extern int _simHandleGraph(luaWrap_lua_State* L);
extern int _simAddStatusbarMessage(luaWrap_lua_State* L);
extern int _simGetLastError(luaWrap_lua_State* L);
extern int _simGetObjects(luaWrap_lua_State* L);
extern int _simRefreshDialogs(luaWrap_lua_State* L);
extern int _simGetModuleName(luaWrap_lua_State* L);
extern int _simGetIkGroupHandle(luaWrap_lua_State* L);
extern int _simGetCollisionHandle(luaWrap_lua_State* L);
extern int _simRemoveScript(luaWrap_lua_State* L);
extern int _simGetDistanceHandle(luaWrap_lua_State* L);
extern int _simGetScriptSimulationParameter(luaWrap_lua_State* L);
extern int _simSetScriptSimulationParameter(luaWrap_lua_State* L);
extern int _simStopSimulation(luaWrap_lua_State* L);
extern int _simPauseSimulation(luaWrap_lua_State* L);
extern int _simStartSimulation(luaWrap_lua_State* L);
extern int _simGetObjectMatrix(luaWrap_lua_State* L);
extern int _simSetObjectMatrix(luaWrap_lua_State* L);
extern int _simGetJointMatrix(luaWrap_lua_State* L);
extern int _simSetSphericalJointMatrix(luaWrap_lua_State* L);
extern int _simBuildIdentityMatrix(luaWrap_lua_State* L);
extern int _simCopyMatrix(luaWrap_lua_State* L);
extern int _simBuildMatrix(luaWrap_lua_State* L);
extern int _simGetEulerAnglesFromMatrix(luaWrap_lua_State* L);
extern int _simInvertMatrix(luaWrap_lua_State* L);
extern int _simMultiplyMatrices(luaWrap_lua_State* L);
extern int _simInterpolateMatrices(luaWrap_lua_State* L);
extern int _simMultiplyVector(luaWrap_lua_State* L);
extern int _simGetObjectParent(luaWrap_lua_State* L);
extern int _simSetObjectParent(luaWrap_lua_State* L);
extern int _simGetObjectChild(luaWrap_lua_State* L);
extern int _simGetObjectType(luaWrap_lua_State* L);
extern int _simGetJointType(luaWrap_lua_State* L);
extern int _simSetBoolParameter(luaWrap_lua_State* L);
extern int _simGetBoolParameter(luaWrap_lua_State* L);
extern int _simSetInt32Parameter(luaWrap_lua_State* L);
extern int _simGetInt32Parameter(luaWrap_lua_State* L);
extern int _simSetFloatParameter(luaWrap_lua_State* L);
extern int _simGetFloatParameter(luaWrap_lua_State* L);
extern int _simSetStringParameter(luaWrap_lua_State* L);
extern int _simGetStringParameter(luaWrap_lua_State* L);
extern int _simSetArrayParameter(luaWrap_lua_State* L);
extern int _simGetArrayParameter(luaWrap_lua_State* L);
extern int _simGetJointInterval(luaWrap_lua_State* L);
extern int _simSetJointInterval(luaWrap_lua_State* L);
extern int _simLoadScene(luaWrap_lua_State* L);
extern int _simCloseScene(luaWrap_lua_State* L);
extern int _simSaveScene(luaWrap_lua_State* L);
extern int _simLoadModel(luaWrap_lua_State* L);
extern int _simSaveModel(luaWrap_lua_State* L);
extern int _simIsObjectInSelection(luaWrap_lua_State* L);
extern int _simAddObjectToSelection(luaWrap_lua_State* L);
extern int _simRemoveObjectFromSelection(luaWrap_lua_State* L);
extern int _simGetObjectSelectionSize(luaWrap_lua_State* L);
extern int _simGetObjectLastSelection(luaWrap_lua_State* L);
extern int _simGetObjectSelection(luaWrap_lua_State* L);
extern int _simGetRealTimeSimulation(luaWrap_lua_State* L);
extern int _simLockInterface(luaWrap_lua_State* L);
extern int _simGetMechanismHandle(luaWrap_lua_State* L);
extern int _simGetPathPlanningHandle(luaWrap_lua_State* L);
extern int _simSearchPath(luaWrap_lua_State* L);
extern int _simInitializePathSearch(luaWrap_lua_State* L);
extern int _simPerformPathSearchStep(luaWrap_lua_State* L);
extern int _simSetNavigationMode(luaWrap_lua_State* L);
extern int _simGetNavigationMode(luaWrap_lua_State* L);
extern int _simSetPage(luaWrap_lua_State* L);
extern int _simGetPage(luaWrap_lua_State* L);
extern int _simReleaseScriptRawBuffer(luaWrap_lua_State* L);
extern int _simCopyPasteSelectedObjects(luaWrap_lua_State* L);
extern int _simCopyPasteObjects(luaWrap_lua_State* L);
extern int _simDeleteSelectedObjects(luaWrap_lua_State* L);
extern int _simScaleSelectedObjects(luaWrap_lua_State* L);
extern int _simScaleObjects(luaWrap_lua_State* L);
extern int _simGetObjectUniqueIdentifier(luaWrap_lua_State* L);
extern int _simGetNameSuffix(luaWrap_lua_State* L);
extern int _simSetNameSuffix(luaWrap_lua_State* L);
extern int _simSetThreadAutomaticSwitch(luaWrap_lua_State* L);
extern int _simGetThreadAutomaticSwitch(luaWrap_lua_State* L);
extern int _simSetThreadSwitchTiming(luaWrap_lua_State* L);
extern int _simSetThreadResumeLocation(luaWrap_lua_State* L);
extern int _simResumeThreads(luaWrap_lua_State* L);
extern int _simSwitchThread(luaWrap_lua_State* L);
extern int _simCreateIkGroup(luaWrap_lua_State* L);
extern int _simRemoveIkGroup(luaWrap_lua_State* L);
extern int _simCreateIkElement(luaWrap_lua_State* L);
extern int _simCreateMotionPlanning(luaWrap_lua_State* L);
extern int _simRemoveMotionPlanning(luaWrap_lua_State* L);
extern int _simCreateCollection(luaWrap_lua_State* L);
extern int _simAddObjectToCollection(luaWrap_lua_State* L);
extern int _simSaveImage(luaWrap_lua_State* L);
extern int _simLoadImage(luaWrap_lua_State* L);
extern int _simGetScaledImage(luaWrap_lua_State* L);
extern int _simTransformImage(luaWrap_lua_State* L);
extern int _simGetQHull(luaWrap_lua_State* L);
extern int _simGetDecimatedMesh(luaWrap_lua_State* L);
extern int _simExportIk(luaWrap_lua_State* L);
extern int _simComputeJacobian(luaWrap_lua_State* L);
extern int _simSendData(luaWrap_lua_State* L);
extern int _simReceiveData(luaWrap_lua_State* L);
extern int _simPackTable(luaWrap_lua_State* L);
extern int _simUnpackTable(luaWrap_lua_State* L);
extern int _simPackInt32Table(luaWrap_lua_State* L);
extern int _simPackUInt32Table(luaWrap_lua_State* L);
extern int _simPackFloatTable(luaWrap_lua_State* L);
extern int _simPackDoubleTable(luaWrap_lua_State* L);
extern int _simPackUInt8Table(luaWrap_lua_State* L);
extern int _simPackUInt16Table(luaWrap_lua_State* L);
extern int _simUnpackInt32Table(luaWrap_lua_State* L);
extern int _simUnpackUInt32Table(luaWrap_lua_State* L);
extern int _simUnpackFloatTable(luaWrap_lua_State* L);
extern int _simUnpackDoubleTable(luaWrap_lua_State* L);
extern int _simUnpackUInt8Table(luaWrap_lua_State* L);
extern int _simUnpackUInt16Table(luaWrap_lua_State* L);
extern int _simTransformBuffer(luaWrap_lua_State* L);
extern int _simCombineRgbImages(luaWrap_lua_State* L);
extern int _simGetVelocity(luaWrap_lua_State* L);
extern int _simGetObjectVelocity(luaWrap_lua_State* L);
extern int _simAddForceAndTorque(luaWrap_lua_State* L);
extern int _simAddForce(luaWrap_lua_State* L);
extern int _simSetExplicitHandling(luaWrap_lua_State* L);
extern int _simGetExplicitHandling(luaWrap_lua_State* L);
extern int _simGetLinkDummy(luaWrap_lua_State* L);
extern int _simSetLinkDummy(luaWrap_lua_State* L);
extern int _simSetGraphUserData(luaWrap_lua_State* L);
extern int _simAddDrawingObject(luaWrap_lua_State* L);
extern int _simRemoveDrawingObject(luaWrap_lua_State* L);
extern int _simAddDrawingObjectItem(luaWrap_lua_State* L);
extern int _simAddParticleObject(luaWrap_lua_State* L);
extern int _simRemoveParticleObject(luaWrap_lua_State* L);
extern int _simAddParticleObjectItem(luaWrap_lua_State* L);
extern int _simSerialOpen(luaWrap_lua_State* L);
extern int _simSerialClose(luaWrap_lua_State* L);
extern int _simSerialSend(luaWrap_lua_State* L);
extern int _simSerialRead(luaWrap_lua_State* L);
extern int _simSerialCheck(luaWrap_lua_State* L);
extern int _simSerialPortOpen(luaWrap_lua_State* L);
extern int _simSerialPortClose(luaWrap_lua_State* L);
extern int _simSerialPortSend(luaWrap_lua_State* L);
extern int _simSerialPortRead(luaWrap_lua_State* L);
extern int _simGetObjectSizeFactor(luaWrap_lua_State* L);
extern int _simResetMilling(luaWrap_lua_State* L);
extern int _simApplyMilling(luaWrap_lua_State* L);
extern int _simSetIntegerSignal(luaWrap_lua_State* L);
extern int _simGetIntegerSignal(luaWrap_lua_State* L);
extern int _simClearIntegerSignal(luaWrap_lua_State* L);
extern int _simSetFloatSignal(luaWrap_lua_State* L);
extern int _simGetFloatSignal(luaWrap_lua_State* L);
extern int _simClearFloatSignal(luaWrap_lua_State* L);
extern int _simSetDoubleSignal(luaWrap_lua_State* L);
extern int _simGetDoubleSignal(luaWrap_lua_State* L);
extern int _simClearDoubleSignal(luaWrap_lua_State* L);
extern int _simSetStringSignal(luaWrap_lua_State* L);
extern int _simGetStringSignal(luaWrap_lua_State* L);
extern int _simClearStringSignal(luaWrap_lua_State* L);
extern int _simGetSignalName(luaWrap_lua_State* L);
extern int _simWaitForSignal(luaWrap_lua_State* L);
extern int _simPersistentDataWrite(luaWrap_lua_State* L);
extern int _simPersistentDataRead(luaWrap_lua_State* L);
extern int _simSetObjectProperty(luaWrap_lua_State* L);
extern int _simGetObjectProperty(luaWrap_lua_State* L);
extern int _simSetObjectSpecialProperty(luaWrap_lua_State* L);
extern int _simGetObjectSpecialProperty(luaWrap_lua_State* L);
extern int _simSetModelProperty(luaWrap_lua_State* L);
extern int _simGetModelProperty(luaWrap_lua_State* L);
extern int _simMoveToPosition(luaWrap_lua_State* L);
extern int _simMoveToObject(luaWrap_lua_State* L);
extern int _simFollowPath(luaWrap_lua_State* L);
extern int _simMoveToJointPositions(luaWrap_lua_State* L);
extern int _simWait(luaWrap_lua_State* L);
extern int _simDelegateChildScriptExecution(luaWrap_lua_State* L);
extern int _simGetDataOnPath(luaWrap_lua_State* L);
extern int _simGetPositionOnPath(luaWrap_lua_State* L);
extern int _simGetOrientationOnPath(luaWrap_lua_State* L);
extern int _simGetClosestPositionOnPath(luaWrap_lua_State* L);
extern int _simReadForceSensor(luaWrap_lua_State* L);
extern int _simBreakForceSensor(luaWrap_lua_State* L);
extern int _simGetShapeVertex(luaWrap_lua_State* L);
extern int _simGetShapeTriangle(luaWrap_lua_State* L);
extern int _simGetLightParameters(luaWrap_lua_State* L);
extern int _simSetLightParameters(luaWrap_lua_State* L);
extern int _simSetShapeColor(luaWrap_lua_State* L);
extern int _simGetShapeColor(luaWrap_lua_State* L);
extern int _simResetDynamicObject(luaWrap_lua_State* L);
extern int _simSetJointMode(luaWrap_lua_State* L);
extern int _simGetJointMode(luaWrap_lua_State* L);
extern int _simGetContactInfo(luaWrap_lua_State* L);
extern int _simSetThreadIsFree(luaWrap_lua_State* L);
extern int _simTubeOpen(luaWrap_lua_State* L);
extern int _simTubeClose(luaWrap_lua_State* L);
extern int _simTubeWrite(luaWrap_lua_State* L);
extern int _simTubeRead(luaWrap_lua_State* L);
extern int _simTubeStatus(luaWrap_lua_State* L);
extern int _simAuxiliaryConsoleOpen(luaWrap_lua_State* L);
extern int _simAuxiliaryConsoleClose(luaWrap_lua_State* L);
extern int _simAuxiliaryConsolePrint(luaWrap_lua_State* L);
extern int _simAuxiliaryConsoleShow(luaWrap_lua_State* L);
extern int _simImportShape(luaWrap_lua_State* L);
extern int _simImportMesh(luaWrap_lua_State* L);
extern int _simExportMesh(luaWrap_lua_State* L);
extern int _simCreateMeshShape(luaWrap_lua_State* L);
extern int _simGetShapeMesh(luaWrap_lua_State* L);
extern int _simCreatePureShape(luaWrap_lua_State* L);
extern int _simCreateHeightfieldShape(luaWrap_lua_State* L);
extern int _simAddBanner(luaWrap_lua_State* L);
extern int _simRemoveBanner(luaWrap_lua_State* L);
extern int _simCreateJoint(luaWrap_lua_State* L);
extern int _simCreateDummy(luaWrap_lua_State* L);
extern int _simCreateProximitySensor(luaWrap_lua_State* L);
extern int _simCreatePath(luaWrap_lua_State* L);
extern int _simInsertPathCtrlPoints(luaWrap_lua_State* L);
extern int _simCutPathCtrlPoints(luaWrap_lua_State* L);
extern int _simGetIkGroupMatrix(luaWrap_lua_State* L);
extern int _simCreateForceSensor(luaWrap_lua_State* L);
extern int _simCreateVisionSensor(luaWrap_lua_State* L);
extern int _simFloatingViewAdd(luaWrap_lua_State* L);
extern int _simFloatingViewRemove(luaWrap_lua_State* L);
extern int _simAdjustView(luaWrap_lua_State* L);
extern int _simCameraFitToView(luaWrap_lua_State* L);
extern int _simAnnounceSceneContentChange(luaWrap_lua_State* L);
extern int _simGetObjectInt32Parameter(luaWrap_lua_State* L);
extern int _simSetObjectInt32Parameter(luaWrap_lua_State* L);
extern int _simGetObjectFloatParameter(luaWrap_lua_State* L);
extern int _simSetObjectFloatParameter(luaWrap_lua_State* L);
extern int _simGetObjectStringParameter(luaWrap_lua_State* L);
extern int _simSetObjectStringParameter(luaWrap_lua_State* L);
extern int _simGetRotationAxis(luaWrap_lua_State* L);
extern int _simRotateAroundAxis(luaWrap_lua_State* L);
extern int _simLaunchExecutable(luaWrap_lua_State* L);
extern int _simGetJointForce(luaWrap_lua_State* L);
extern int _simJointGetForce(luaWrap_lua_State* L);
extern int _simSetIkGroupProperties(luaWrap_lua_State* L);
extern int _simSetIkElementProperties(luaWrap_lua_State* L);
extern int _simIsHandleValid(luaWrap_lua_State* L);
extern int _simHandleVisionSensor(luaWrap_lua_State* L);
extern int _simReadVisionSensor(luaWrap_lua_State* L);
extern int _simResetVisionSensor(luaWrap_lua_State* L);
extern int _simGetVisionSensorResolution(luaWrap_lua_State* L);
extern int _simGetVisionSensorImage(luaWrap_lua_State* L);
extern int _simGetVisionSensorCharImage(luaWrap_lua_State* L);
extern int _simSetVisionSensorImage(luaWrap_lua_State* L);
extern int _simSetVisionSensorCharImage(luaWrap_lua_State* L);
extern int _simGetVisionSensorDepthBuffer(luaWrap_lua_State* L);
extern int _simCheckVisionSensor(luaWrap_lua_State* L);
extern int _simCheckVisionSensorEx(luaWrap_lua_State* L);
extern int _simRMLPos(luaWrap_lua_State* L);
extern int _simRMLVel(luaWrap_lua_State* L);
extern int _simRMLStep(luaWrap_lua_State* L);
extern int _simRMLRemove(luaWrap_lua_State* L);
extern int _simRMLMoveToPosition(luaWrap_lua_State* L);
extern int _simRMLMoveToJointPositions(luaWrap_lua_State* L);
extern int _simGetObjectQuaternion(luaWrap_lua_State* L);
extern int _simSetObjectQuaternion(luaWrap_lua_State* L);
extern int _simSetShapeMassAndInertia(luaWrap_lua_State* L);
extern int _simGetShapeMassAndInertia(luaWrap_lua_State* L);
extern int _simGroupShapes(luaWrap_lua_State* L);
extern int _simUngroupShape(luaWrap_lua_State* L);
extern int _simConvexDecompose(luaWrap_lua_State* L);
extern int _simGetMotionPlanningHandle(luaWrap_lua_State* L);
extern int _simFindMpPath(luaWrap_lua_State* L);
extern int _simSimplifyMpPath(luaWrap_lua_State* L);
extern int _simFindIkPath(luaWrap_lua_State* L);
extern int _simGetMpConfigTransition(luaWrap_lua_State* L);
extern int _simAddGhost(luaWrap_lua_State* L);
extern int _simModifyGhost(luaWrap_lua_State* L);
extern int _simQuitSimulator(luaWrap_lua_State* L);
extern int _simGetThreadId(luaWrap_lua_State* L);
extern int _simSetShapeMaterial(luaWrap_lua_State* L);
extern int _simGetTextureId(luaWrap_lua_State* L);
extern int _simReadTexture(luaWrap_lua_State* L);
extern int _simWriteTexture(luaWrap_lua_State* L);
extern int _simCreateTexture(luaWrap_lua_State* L);
extern int _simWriteCustomDataBlock(luaWrap_lua_State* L);
extern int _simReadCustomDataBlock(luaWrap_lua_State* L);
extern int _simReadCustomDataBlockTags(luaWrap_lua_State* L);
extern int _simAddPointCloud(luaWrap_lua_State* L);
extern int _simModifyPointCloud(luaWrap_lua_State* L);
extern int _simGetShapeGeomInfo(luaWrap_lua_State* L);
extern int _simGetObjectsInTree(luaWrap_lua_State* L);
extern int _simSetObjectSizeValues(luaWrap_lua_State* L);
extern int _simGetObjectSizeValues(luaWrap_lua_State* L);
extern int _simScaleObject(luaWrap_lua_State* L);
extern int _simSetShapeTexture(luaWrap_lua_State* L);
extern int _simGetShapeTextureId(luaWrap_lua_State* L);
extern int _simGetCollectionObjects(luaWrap_lua_State* L);
extern int _simHandleCustomizationScripts(luaWrap_lua_State* L);
extern int _simHandleAddOnScripts(luaWrap_lua_State* L);
extern int _simHandleSandboxScript(luaWrap_lua_State* L);
extern int _simSetScriptAttribute(luaWrap_lua_State* L);
extern int _simGetScriptAttribute(luaWrap_lua_State* L);
extern int _simReorientShapeBoundingBox(luaWrap_lua_State* L);
extern int _simBuildMatrixQ(luaWrap_lua_State* L);
extern int _simGetQuaternionFromMatrix(luaWrap_lua_State* L);
extern int _simFileDialog(luaWrap_lua_State* L);
extern int _simMsgBox(luaWrap_lua_State* L);
extern int _simLoadModule(luaWrap_lua_State* L);
extern int _simUnloadModule(luaWrap_lua_State* L);
extern int _simCallScriptFunction(luaWrap_lua_State* L);
extern int _simGetConfigForTipPose(luaWrap_lua_State* L);
extern int _simGenerateIkPath(luaWrap_lua_State* L);
extern int _simGetExtensionString(luaWrap_lua_State* L);
extern int _simComputeMassAndInertia(luaWrap_lua_State* L);
extern int _simSetScriptVariable(luaWrap_lua_State* L);
extern int _simGetEngineFloatParameter(luaWrap_lua_State* L);
extern int _simGetEngineInt32Parameter(luaWrap_lua_State* L);
extern int _simGetEngineBoolParameter(luaWrap_lua_State* L);
extern int _simSetEngineFloatParameter(luaWrap_lua_State* L);
extern int _simSetEngineInt32Parameter(luaWrap_lua_State* L);
extern int _simSetEngineBoolParameter(luaWrap_lua_State* L);
extern int _simCreateOctree(luaWrap_lua_State* L);
extern int _simCreatePointCloud(luaWrap_lua_State* L);
extern int _simSetPointCloudOptions(luaWrap_lua_State* L);
extern int _simGetPointCloudOptions(luaWrap_lua_State* L);
extern int _simInsertVoxelsIntoOctree(luaWrap_lua_State* L);
extern int _simRemoveVoxelsFromOctree(luaWrap_lua_State* L);
extern int _simInsertPointsIntoPointCloud(luaWrap_lua_State* L);
extern int _simRemovePointsFromPointCloud(luaWrap_lua_State* L);
extern int _simIntersectPointsWithPointCloud(luaWrap_lua_State* L);
extern int _simGetOctreeVoxels(luaWrap_lua_State* L);
extern int _simGetPointCloudPoints(luaWrap_lua_State* L);
extern int _simInsertObjectIntoOctree(luaWrap_lua_State* L);
extern int _simSubtractObjectFromOctree(luaWrap_lua_State* L);
extern int _simInsertObjectIntoPointCloud(luaWrap_lua_State* L);
extern int _simSubtractObjectFromPointCloud(luaWrap_lua_State* L);
extern int _simCheckOctreePointOccupancy(luaWrap_lua_State* L);
extern int _simSetVisionSensorFilter(luaWrap_lua_State* L);
extern int _simGetVisionSensorFilter(luaWrap_lua_State* L);
extern int _simHandleSimulationStart(luaWrap_lua_State* L);
extern int _simHandleSensingStart(luaWrap_lua_State* L);
extern int _simAuxFunc(luaWrap_lua_State* L);
extern int _simSetReferencedHandles(luaWrap_lua_State* L);
extern int _simGetReferencedHandles(luaWrap_lua_State* L);
extern int _simGetGraphCurve(luaWrap_lua_State* L);
extern int _simGetGraphInfo(luaWrap_lua_State* L);
extern int _simGetShapeViz(luaWrap_lua_State* L);
extern int _simExecuteScriptString(luaWrap_lua_State* L);
extern int _simGetApiFunc(luaWrap_lua_State* L);
extern int _simGetApiInfo(luaWrap_lua_State* L);
extern int _simGetModuleInfo(luaWrap_lua_State* L);
extern int _simRegisterScriptFunction(luaWrap_lua_State* L);
extern int _simRegisterScriptVariable(luaWrap_lua_State* L);
extern int _simIsDeprecated(luaWrap_lua_State* L);
extern int _simGetPersistentDataTags(luaWrap_lua_State* L);
extern int _simGetRandom(luaWrap_lua_State* L);
extern int _simTest(luaWrap_lua_State* L);
extern int _simTextEditorOpen(luaWrap_lua_State* L);
extern int _simTextEditorClose(luaWrap_lua_State* L);
extern int _simTextEditorShow(luaWrap_lua_State* L);
extern int _simTextEditorGetInfo(luaWrap_lua_State* L);
extern int _simSetJointDependency(luaWrap_lua_State* L);
extern int _simGetStackTraceback(luaWrap_lua_State* L);
extern int _simSetStringNamedParam(luaWrap_lua_State* L);
extern int _simGetStringNamedParam(luaWrap_lua_State* L);

// DEPRECATED
extern int _simOpenTextEditor(luaWrap_lua_State* L);
extern int _simCloseTextEditor(luaWrap_lua_State* L);
extern int _simGetMaterialId(luaWrap_lua_State* L);
extern int _simGetShapeMaterial(luaWrap_lua_State* L);
extern int _simHandleVarious(luaWrap_lua_State* L);
extern int _simGetInstanceIndex(luaWrap_lua_State* L);
extern int _simGetVisibleInstanceIndex(luaWrap_lua_State* L);
extern int _simRMLPosition(luaWrap_lua_State* L);
extern int _simRMLVelocity(luaWrap_lua_State* L);
extern int _simResetPath(luaWrap_lua_State* L);
extern int _simHandlePath(luaWrap_lua_State* L);
extern int _simResetJoint(luaWrap_lua_State* L);
extern int _simHandleJoint(luaWrap_lua_State* L);
extern int _simGetMpConfigForTipPose(luaWrap_lua_State* L);
extern int _simEnableWorkThreads(luaWrap_lua_State* L);
extern int _simWaitForWorkThreads(luaWrap_lua_State* L);
extern int _simGetInvertedMatrix(luaWrap_lua_State* L);
extern int _simGetSystemTimeInMilliseconds(luaWrap_lua_State* L);
extern int _simAddSceneCustomData(luaWrap_lua_State* L);
extern int _simGetSceneCustomData(luaWrap_lua_State* L);
extern int _simAddObjectCustomData(luaWrap_lua_State* L);
extern int _simGetObjectCustomData(luaWrap_lua_State* L);
extern int _simSetUIPosition(luaWrap_lua_State* L);
extern int _simGetUIPosition(luaWrap_lua_State* L);
extern int _simGetUIHandle(luaWrap_lua_State* L);
extern int _simGetUIProperty(luaWrap_lua_State* L);
extern int _simSetUIProperty(luaWrap_lua_State* L);
extern int _simGetUIEventButton(luaWrap_lua_State* L);
extern int _simGetUIButtonProperty(luaWrap_lua_State* L);
extern int _simSetUIButtonProperty(luaWrap_lua_State* L);
extern int _simGetUIButtonSize(luaWrap_lua_State* L);
extern int _simSetUIButtonLabel(luaWrap_lua_State* L);
extern int _simGetUIButtonLabel(luaWrap_lua_State* L);
extern int _simSetUISlider(luaWrap_lua_State* L);
extern int _simGetUISlider(luaWrap_lua_State* L);
extern int _simCreateUIButtonArray(luaWrap_lua_State* L);
extern int _simSetUIButtonArrayColor(luaWrap_lua_State* L);
extern int _simDeleteUIButtonArray(luaWrap_lua_State* L);
extern int _simCreateUI(luaWrap_lua_State* L);
extern int _simCreateUIButton(luaWrap_lua_State* L);
extern int _simLoadUI(luaWrap_lua_State* L);
extern int _simSaveUI(luaWrap_lua_State* L);
extern int _simRemoveUI(luaWrap_lua_State* L);
extern int _simSetUIButtonColor(luaWrap_lua_State* L);
extern int _simHandleChildScript(luaWrap_lua_State* L);
extern int _simHandleChildScripts_legacy(luaWrap_lua_State* L);
extern int _simHandleChildScripts2_legacy(luaWrap_lua_State* L,std::string &functionName, std::string& errorString);
extern int _simLaunchThreadedChildScripts_legacy(luaWrap_lua_State* L);
extern int _simResumeThreads_legacy(luaWrap_lua_State* L);
