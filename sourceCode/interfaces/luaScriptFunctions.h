#pragma once

#include <simLib/simTypes.h>
#include <luaWrapper.h>
#include <vector>

class CScriptCustomFunction;

enum
{
    lua_arg_empty = 0,
    lua_arg_nil,
    lua_arg_number,
    lua_arg_integer,
    lua_arg_bool,
    lua_arg_string,
    lua_arg_buffer = lua_arg_string,
    lua_arg_function,
    lua_arg_userdata,
    lua_arg_table,
    lua_arg_optional = 64
};

struct SLuaCommands
{
    std::string name;
    luaWrap_lua_CFunction func;
};

struct SLuaVariables
{
    std::string name;
    int val;
};

void _registerTableFunction(luaWrap_lua_State* L, char const* const tableName, char const* const functionName, luaWrap_lua_CFunction functionCallback);

void getFloatsFromTable(luaWrap_lua_State* L, int tablePos, size_t floatCount, float* arrayField);
void getDoublesFromTable(luaWrap_lua_State* L, int tablePos, size_t doubleCount, double* arrayField);
bool getIntsFromTable(luaWrap_lua_State* L, int tablePos, size_t intCount, int* arrayField);
bool getLongsFromTable(luaWrap_lua_State* L, int tablePos, size_t intCount, long long int* arrayField);
bool getUIntsFromTable(luaWrap_lua_State* L, int tablePos, size_t intCount, unsigned int* arrayField);
bool getUCharsFromTable(luaWrap_lua_State* L, int tablePos, size_t intCount, unsigned char* arrayField);
void getCharBoolsFromTable(luaWrap_lua_State* L, int tablePos, size_t boolCount, char* arrayField);
void getStringsFromTable(luaWrap_lua_State* L, int tablePos, size_t stringCount, std::vector<std::string>& array);
void pushFloatTableOntoStack(luaWrap_lua_State* L, size_t floatCount, const float* arrayField);
void pushDoubleTableOntoStack(luaWrap_lua_State* L, size_t doubleCount, const double* arrayField);
void pushIntTableOntoStack(luaWrap_lua_State* L, size_t intCount, const int* arrayField);
void pushLongTableOntoStack(luaWrap_lua_State* L, size_t intCount, const long long int* arrayField);
void pushUIntTableOntoStack(luaWrap_lua_State* L, size_t intCount, const unsigned int* arrayField);
void pushUCharTableOntoStack(luaWrap_lua_State* L, size_t intCount, const unsigned char* arrayField);
void pushStringTableOntoStack(luaWrap_lua_State* L, const std::vector<std::string>& stringTable);
void pushBufferTableOntoStack(luaWrap_lua_State* L, const std::vector<std::string>& stringTable);

int luaToInt(luaWrap_lua_State* L, int pos);
double luaToDouble(luaWrap_lua_State* L, int pos);
bool luaToBool(luaWrap_lua_State* L, int pos);

std::string _LUA_START(luaWrap_lua_State* L, const char* funcName, int& argOffset);
void _reportWarningsIfNeeded(luaWrap_lua_State* L, const char* functionName, const char* warningString);
void _raiseErrorIfNeeded(luaWrap_lua_State* L, const char* functionName, const char* errorString, bool cSideErrorReporting);
bool doesEntityExist(std::string* errStr, int identifier);
int fetchBoolArg(luaWrap_lua_State* L, int index, bool defaultValue = false);
long long int fetchLongArg(luaWrap_lua_State* L, int index, long long int defaultValue = -1);
int fetchIntArg(luaWrap_lua_State* L, int index, int defaultValue = -1);
double fetchDoubleArg(luaWrap_lua_State* L, int index, double defaultValue = 0.0);
std::string fetchTextArg(luaWrap_lua_State* L, int index, const char* txt = "");
std::string fetchBufferArg(luaWrap_lua_State* L, int index);
void fetchIntArrayArg(luaWrap_lua_State* L, int index, std::vector<int>& outArr, std::initializer_list<int> arr = {});
void fetchIntArrayArg(luaWrap_lua_State* L, int index, std::vector<int>& outArr, std::vector<int>& arr);
void fetchFloatArrayArg(luaWrap_lua_State* L, int index, std::vector<float>& outArr, std::initializer_list<float> arr = {});
void fetchFloatArrayArg(luaWrap_lua_State* L, int index, std::vector<float>& outArr, std::vector<float>& arr);
void fetchDoubleArrayArg(luaWrap_lua_State* L, int index, std::vector<double>& outArr, std::initializer_list<double> arr = {});
void fetchDoubleArrayArg(luaWrap_lua_State* L, int index, std::vector<double>& outArr, std::vector<double>& arr);
void fetchTextArrayArg(luaWrap_lua_State* L, int index, std::vector<std::string>& outArr, std::initializer_list<std::string> arr = {});
void fetchTextArrayArg(luaWrap_lua_State* L, int index, std::vector<std::string>& outArr, std::vector<std::string>& arr);
bool isArgNilOrMissing(luaWrap_lua_State* L, int index);
bool checkInputArguments(luaWrap_lua_State* L, std::string* errStr, int argOffset,int type1 = lua_arg_empty,
                         int type1Cnt_zeroIfNotTable = -2, int type2 = lua_arg_empty, int type2Cnt_zeroIfNotTable = -2,
                         int type3 = lua_arg_empty, int type3Cnt_zeroIfNotTable = -2, int type4 = lua_arg_empty,
                         int type4Cnt_zeroIfNotTable = -2, int type5 = lua_arg_empty, int type5Cnt_zeroIfNotTable = -2,
                         int type6 = lua_arg_empty, int type6Cnt_zeroIfNotTable = -2, int type7 = lua_arg_empty,
                         int type7Cnt_zeroIfNotTable = -2, int type8 = lua_arg_empty, int type8Cnt_zeroIfNotTable = -2,
                         int type9 = lua_arg_empty, int type9Cnt_zeroIfNotTable = -2, int type10 = lua_arg_empty,
                         int type10Cnt_zeroIfNotTable = -2, int type11 = lua_arg_empty,
                         int type11Cnt_zeroIfNotTable = -2);
int checkOneGeneralInputArgument(luaWrap_lua_State* L, int index, int type, int cnt_orZeroIfNotTable, bool optional, bool nilInsteadOfTypeAndCountAllowed, std::string* errStr, int argOffset);
bool checkOneInputArgument(luaWrap_lua_State* L, int index, int type, std::string* errStr, int argOffset);

int _genericFunctionHandler(luaWrap_lua_State* L, void (*callback)(struct SScriptCallBack* cb), std::string& raiseErrorWithMsg, CScriptCustomFunction* func = nullptr);

const extern SLuaCommands simLuaCommands[];
const extern SLuaVariables simLuaVariables[];

extern int _ccallback(luaWrap_lua_State* L, size_t index);
extern int _ccallback0(luaWrap_lua_State* L);
extern int _ccallback1(luaWrap_lua_State* L);
extern int _ccallback2(luaWrap_lua_State* L);
extern int _ccallback3(luaWrap_lua_State* L);
extern int _loadPlugin(luaWrap_lua_State* L);
extern int _unloadPlugin(luaWrap_lua_State* L);
extern int _registerCodeEditorInfos(luaWrap_lua_State* L);
extern int _auxFunc(luaWrap_lua_State* L);

extern int _setAutoYield(luaWrap_lua_State* L);
extern int _getAutoYield(luaWrap_lua_State* L);
extern int _getYieldAllowed(luaWrap_lua_State* L);
extern int _setYieldAllowed(luaWrap_lua_State* L);
extern int _registerScriptFuncHook(luaWrap_lua_State* L);

extern int _addLog(luaWrap_lua_State* L);
extern int _quitSimulator(luaWrap_lua_State* L);

extern int _simHandleExtCalls(luaWrap_lua_State* L);
extern int _simGetLastInfo(luaWrap_lua_State* L);
extern int _simIsHandle(luaWrap_lua_State* L);
extern int _simHandleSimulationScripts(luaWrap_lua_State* L);
extern int _simHandleEmbeddedScripts(luaWrap_lua_State* L);
extern int _simGenericFunctionHandler(luaWrap_lua_State* L);
extern int _simHandleDynamics(luaWrap_lua_State* L);
extern int _simHandleProximitySensor(luaWrap_lua_State* L);
extern int _simResetProximitySensor(luaWrap_lua_State* L);
extern int _simCheckProximitySensor(luaWrap_lua_State* L);
extern int _simGetObject(luaWrap_lua_State* L);
extern int _simGetObjectUid(luaWrap_lua_State* L);
extern int _simGetObjectFromUid(luaWrap_lua_State* L);
extern int _simGetScript(luaWrap_lua_State* L);
extern int _simGetObjectPosition(luaWrap_lua_State* L);
extern int _simSetObjectPosition(luaWrap_lua_State* L);
extern int _simGetJointPosition(luaWrap_lua_State* L);
extern int _simSetJointPosition(luaWrap_lua_State* L);
extern int _simSetJointTargetPosition(luaWrap_lua_State* L);
extern int _simGetJointTargetPosition(luaWrap_lua_State* L);
extern int _simSetJointTargetVelocity(luaWrap_lua_State* L);
extern int _simGetJointTargetVelocity(luaWrap_lua_State* L);
extern int _simGetObjectAlias(luaWrap_lua_State* L);
extern int _simSetObjectAlias(luaWrap_lua_State* L);
extern int _simRemoveObjects(luaWrap_lua_State* L);
extern int _simRemoveModel(luaWrap_lua_State* L);
extern int _simGetSimulationTime(luaWrap_lua_State* L);
extern int _simGetSimulationState(luaWrap_lua_State* L);
extern int _simGetSystemTime(luaWrap_lua_State* L);
extern int _simCheckCollision(luaWrap_lua_State* L);
extern int _simCheckDistance(luaWrap_lua_State* L);
extern int _simGetSimulationTimeStep(luaWrap_lua_State* L);
extern int _simGetSimulatorMessage(luaWrap_lua_State* L);
extern int _simResetGraph(luaWrap_lua_State* L);
extern int _simHandleGraph(luaWrap_lua_State* L);
extern int _simAddGraphStream(luaWrap_lua_State* L);
extern int _simDestroyGraphCurve(luaWrap_lua_State* L);
extern int _simSetGraphStreamTransformation(luaWrap_lua_State* L);
extern int _simDuplicateGraphCurveToStatic(luaWrap_lua_State* L);
extern int _simAddGraphCurve(luaWrap_lua_State* L);
extern int _simSetGraphStreamValue(luaWrap_lua_State* L);
extern int _simRefreshDialogs(luaWrap_lua_State* L);
extern int _simStopSimulation(luaWrap_lua_State* L);
extern int _simPauseSimulation(luaWrap_lua_State* L);
extern int _simStartSimulation(luaWrap_lua_State* L);
extern int _simGetObjectPose(luaWrap_lua_State* L);
extern int _simSetObjectPose(luaWrap_lua_State* L);
extern int _simGetObjectChildPose(luaWrap_lua_State* L);
extern int _simSetObjectChildPose(luaWrap_lua_State* L);
extern int _simSetObjectParent(luaWrap_lua_State* L);
extern int _simGetObjectType(luaWrap_lua_State* L);
extern int _simGetJointType(luaWrap_lua_State* L);
extern int _simGetJointInterval(luaWrap_lua_State* L);
extern int _simSetJointInterval(luaWrap_lua_State* L);
extern int _simLoadScene(luaWrap_lua_State* L);
extern int _simCloseScene(luaWrap_lua_State* L);
extern int _simSaveScene(luaWrap_lua_State* L);
extern int _simLoadModel(luaWrap_lua_State* L);
extern int _simSaveModel(luaWrap_lua_State* L);
extern int _simGetObjectSel(luaWrap_lua_State* L);
extern int _simSetObjectSel(luaWrap_lua_State* L);
extern int _simSetNavigationMode(luaWrap_lua_State* L);
extern int _simGetNavigationMode(luaWrap_lua_State* L);
extern int _simSetPage(luaWrap_lua_State* L);
extern int _simGetPage(luaWrap_lua_State* L);
extern int _simCopyPasteObjects(luaWrap_lua_State* L);
extern int _simScaleObjects(luaWrap_lua_State* L);
extern int _simSetAutoYieldDelay(luaWrap_lua_State* L);
extern int _simGetAutoYieldDelay(luaWrap_lua_State* L);
extern int _simSaveImage(luaWrap_lua_State* L);
extern int _simLoadImage(luaWrap_lua_State* L);
extern int _simGetScaledImage(luaWrap_lua_State* L);
extern int _simTransformImage(luaWrap_lua_State* L);
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
extern int _simGetJointVelocity(luaWrap_lua_State* L);
extern int _simAddForceAndTorque(luaWrap_lua_State* L);
extern int _simAddForce(luaWrap_lua_State* L);
extern int _simSetExplicitHandling(luaWrap_lua_State* L);
extern int _simGetExplicitHandling(luaWrap_lua_State* L);
extern int _simGetLinkDummy(luaWrap_lua_State* L);
extern int _simSetLinkDummy(luaWrap_lua_State* L);
extern int _simCreateDrawingObject(luaWrap_lua_State* L);
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
extern int _simGetShapeVertex(luaWrap_lua_State* L);
extern int _simGetShapeTriangle(luaWrap_lua_State* L);
extern int _simSetObjectColor(luaWrap_lua_State* L);
extern int _simGetObjectColor(luaWrap_lua_State* L);
extern int _simSetShapeColor(luaWrap_lua_State* L);
extern int _simGetShapeColor(luaWrap_lua_State* L);
extern int _simResetDynamicObject(luaWrap_lua_State* L);
extern int _simSetJointMode(luaWrap_lua_State* L);
extern int _simGetJointMode(luaWrap_lua_State* L);
extern int _simGetContactInfo(luaWrap_lua_State* L);
extern int _simAuxiliaryConsoleOpen(luaWrap_lua_State* L);
extern int _simAuxiliaryConsoleClose(luaWrap_lua_State* L);
extern int _simAuxiliaryConsolePrint(luaWrap_lua_State* L);
extern int _simAuxiliaryConsoleShow(luaWrap_lua_State* L);
extern int _simImportShape(luaWrap_lua_State* L);
extern int _simImportMesh(luaWrap_lua_State* L);
extern int _simExportMesh(luaWrap_lua_State* L);
extern int _simCreateShape(luaWrap_lua_State* L);
extern int _simGetShapeMesh(luaWrap_lua_State* L);
extern int _simGetShapeBB(luaWrap_lua_State* L);
extern int _simCreatePrimitiveShape(luaWrap_lua_State* L);
extern int _simCreateHeightfieldShape(luaWrap_lua_State* L);
extern int _simCreateJoint(luaWrap_lua_State* L);
extern int _simCreateDummy(luaWrap_lua_State* L);
extern int _simCreateScript(luaWrap_lua_State* L);
extern int _simCreateProximitySensor(luaWrap_lua_State* L);
extern int _simCreateForceSensor(luaWrap_lua_State* L);
extern int _simCreateVisionSensor(luaWrap_lua_State* L);
extern int _simFloatingViewAdd(luaWrap_lua_State* L);
extern int _simFloatingViewRemove(luaWrap_lua_State* L);
extern int _simAdjustView(luaWrap_lua_State* L);
extern int _simCameraFitToView(luaWrap_lua_State* L);
extern int _simAnnounceSceneContentChange(luaWrap_lua_State* L);
extern int _simLaunchExecutable(luaWrap_lua_State* L);
extern int _simGetJointForce(luaWrap_lua_State* L);
extern int _simGetJointTargetForce(luaWrap_lua_State* L);
extern int _simSetJointTargetForce(luaWrap_lua_State* L);
extern int _simHandleVisionSensor(luaWrap_lua_State* L);
extern int _simResetVisionSensor(luaWrap_lua_State* L);
extern int _simGetVisionSensorImg(luaWrap_lua_State* L);
extern int _simSetVisionSensorImg(luaWrap_lua_State* L);
extern int _simGetVisionSensorDepth(luaWrap_lua_State* L);
extern int _simCheckVisionSensor(luaWrap_lua_State* L);
extern int _simRuckigPos(luaWrap_lua_State* L);
extern int _simRuckigVel(luaWrap_lua_State* L);
extern int _simRuckigStep(luaWrap_lua_State* L);
extern int _simRuckigRemove(luaWrap_lua_State* L);
extern int _simGetObjectQuaternion(luaWrap_lua_State* L);
extern int _simSetObjectQuaternion(luaWrap_lua_State* L);
extern int _simGroupShapes(luaWrap_lua_State* L);
extern int _simUngroupShape(luaWrap_lua_State* L);
extern int _simFindIkPath(luaWrap_lua_State* L);
extern int _simSetShapeMaterial(luaWrap_lua_State* L);
extern int _simGetTextureId(luaWrap_lua_State* L);
extern int _simReadTexture(luaWrap_lua_State* L);
extern int _simWriteTexture(luaWrap_lua_State* L);
extern int _simCreateTexture(luaWrap_lua_State* L);
extern int _simGetShapeGeomInfo(luaWrap_lua_State* L);
extern int _simScaleObject(luaWrap_lua_State* L);
extern int _simSetShapeTexture(luaWrap_lua_State* L);
extern int _simGetShapeTextureId(luaWrap_lua_State* L);
extern int _simCreateCollectionEx(luaWrap_lua_State* L);
extern int _simDestroyCollection(luaWrap_lua_State* L);
extern int _simAddToCollection(luaWrap_lua_State* L);
extern int _simHandleAddOnScripts(luaWrap_lua_State* L);
extern int _simHandleSandboxScript(luaWrap_lua_State* L);
extern int _simAlignShapeBB(luaWrap_lua_State* L);
extern int _simRelocateShapeFrame(luaWrap_lua_State* L);
extern int _simGetQuaternionFromMatrix(luaWrap_lua_State* L);
extern int _simCallScriptFunction(luaWrap_lua_State* L);
extern int _simGetExtensionString(luaWrap_lua_State* L);
extern int _simComputeMassAndInertia(luaWrap_lua_State* L);
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
extern int _simHandleSimulationStart(luaWrap_lua_State* L);
extern int _simHandleSensingStart(luaWrap_lua_State* L);
extern int _simAuxFunc(luaWrap_lua_State* L);
extern int _simSetReferencedHandles(luaWrap_lua_State* L);
extern int _simGetReferencedHandles(luaWrap_lua_State* L);
extern int _simGetReferencedHandlesTags(luaWrap_lua_State* L);
extern int _simGetGraphCurve(luaWrap_lua_State* L);
extern int _simGetGraphInfo(luaWrap_lua_State* L);
extern int _simGetShapeViz(luaWrap_lua_State* L);
extern int _simExecuteScriptString(luaWrap_lua_State* L);
extern int _simGetApiFunc(luaWrap_lua_State* L);
extern int _simGetApiInfo(luaWrap_lua_State* L);
extern int _simGetPluginInfo(luaWrap_lua_State* L);
extern int _simSetPluginInfo(luaWrap_lua_State* L);
extern int _simTest(luaWrap_lua_State* L);
extern int _simTextEditorOpen(luaWrap_lua_State* L);
extern int _simTextEditorClose(luaWrap_lua_State* L);
extern int _simTextEditorShow(luaWrap_lua_State* L);
extern int _simTextEditorGetInfo(luaWrap_lua_State* L);
extern int _simSetJointDependency(luaWrap_lua_State* L);
extern int _simGetJointDependency(luaWrap_lua_State* L);
extern int _simGetStackTraceback(luaWrap_lua_State* L);
extern int _simGetShapeMass(luaWrap_lua_State* L);
extern int _simSetShapeMass(luaWrap_lua_State* L);
extern int _simGetShapeInertia(luaWrap_lua_State* L);
extern int _simSetShapeInertia(luaWrap_lua_State* L);
extern int _simIsDynamicallyEnabled(luaWrap_lua_State* L);
extern int _simGenerateShapeFromPath(luaWrap_lua_State* L);
extern int _simGetClosestPosOnPath(luaWrap_lua_State* L);
extern int _simInitScript(luaWrap_lua_State* L);
extern int _simModuleEntry(luaWrap_lua_State* L);
extern int _simPushUserEvent(luaWrap_lua_State* L);
extern int _simGetGenesisEvents(luaWrap_lua_State* L);
extern int _simBroadcastMsg(luaWrap_lua_State* L);
extern int _simHandleJointMotion(luaWrap_lua_State* L);
extern int _simGetVisionSensorRes(luaWrap_lua_State* L);
extern int _simGetObjectHierarchyOrder(luaWrap_lua_State* L);
extern int _simSetObjectHierarchyOrder(luaWrap_lua_State* L);
extern int _sim_qhull(luaWrap_lua_State* L);
extern int _simSystemSemaphore(luaWrap_lua_State* L);
extern int _simSetBoolProperty(luaWrap_lua_State* L);
extern int _simGetBoolProperty(luaWrap_lua_State* L);
extern int _simSetIntProperty(luaWrap_lua_State* L);
extern int _simGetIntProperty(luaWrap_lua_State* L);
extern int _simSetLongProperty(luaWrap_lua_State* L);
extern int _simGetLongProperty(luaWrap_lua_State* L);
extern int _simSetHandleProperty(luaWrap_lua_State* L);
extern int _simGetHandleProperty(luaWrap_lua_State* L);
extern int _simSetFloatProperty(luaWrap_lua_State* L);
extern int _simGetFloatProperty(luaWrap_lua_State* L);
extern int _simSetStringProperty(luaWrap_lua_State* L);
extern int _simGetStringProperty(luaWrap_lua_State* L);
extern int _simSetTableProperty(luaWrap_lua_State* L);
extern int _simGetTableProperty(luaWrap_lua_State* L);
extern int _simSetBufferProperty(luaWrap_lua_State* L);
extern int _simGetBufferProperty(luaWrap_lua_State* L);
extern int _simSetIntArray2Property(luaWrap_lua_State* L);
extern int _simGetIntArray2Property(luaWrap_lua_State* L);
extern int _simSetVector2Property(luaWrap_lua_State* L);
extern int _simGetVector2Property(luaWrap_lua_State* L);
extern int _simSetVector3Property(luaWrap_lua_State* L);
extern int _simGetVector3Property(luaWrap_lua_State* L);
extern int _simSetQuaternionProperty(luaWrap_lua_State* L);
extern int _simGetQuaternionProperty(luaWrap_lua_State* L);
extern int _simSetPoseProperty(luaWrap_lua_State* L);
extern int _simGetPoseProperty(luaWrap_lua_State* L);
extern int _simSetColorProperty(luaWrap_lua_State* L);
extern int _simGetColorProperty(luaWrap_lua_State* L);
extern int _simSetFloatArrayProperty(luaWrap_lua_State* L);
extern int _simGetFloatArrayProperty(luaWrap_lua_State* L);
extern int _simSetIntArrayProperty(luaWrap_lua_State* L);
extern int _simGetIntArrayProperty(luaWrap_lua_State* L);
extern int _simSetHandleArrayProperty(luaWrap_lua_State* L);
extern int _simGetHandleArrayProperty(luaWrap_lua_State* L);
extern int _simSetStringArrayProperty(luaWrap_lua_State* L);
extern int _simGetStringArrayProperty(luaWrap_lua_State* L);
extern int _simRemoveProperty(luaWrap_lua_State* L);
extern int _simGetPropertyName(luaWrap_lua_State* L);
extern int _simGetPropertyInfo(luaWrap_lua_State* L);
extern int _simSetEventFilters(luaWrap_lua_State* L);

#include <luaScriptFunctions-old.h>
