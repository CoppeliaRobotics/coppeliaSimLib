#pragma once

#include <string>
#include <interfaceStack.h>
#include <app.h>
#include <detachedScript.h>

extern std::string callMethod(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
bool checkInputArguments(const char* method, const CInterfaceStack* inStack, std::string* errStr, std::vector<int> inargs);

bool hasNonNullArg(const CInterfaceStack* inStack, int index);
bool fetchBool(const CInterfaceStack* inStack, int index, bool defaultValue = false);
long long int fetchLong(const CInterfaceStack* inStack, int index, long long int defaultValue = -1);
int fetchInt(const CInterfaceStack* inStack, int index, int defaultValue = -1);
long long int fetchHandle(const CInterfaceStack* inStack, int index, long long int defaultValue = -1);
double fetchDouble(const CInterfaceStack* inStack, int index, double defaultValue = 0.0);
std::string fetchText(const CInterfaceStack* inStack, int index, const char* txt = "");
std::string fetchBuffer(const CInterfaceStack* inStack, int index);
void fetchBuffer(const CInterfaceStack* inStack, int index, std::vector<char>& buff);
void fetchColor(const CInterfaceStack* inStack, int index, float outArr[3], std::initializer_list<float> arr = {});
void fetchColor(const CInterfaceStack* inStack, int index, float outArr[3], const float defaultArr[3]);
C4Vector fetchQuaternion(const CInterfaceStack* inStack, int index, std::initializer_list<double> wxyz = {});
C4Vector fetchQuaternion(const CInterfaceStack* inStack, int index, const double wxyz[4]);
C7Vector fetchPose(const CInterfaceStack* inStack, int index, std::initializer_list<double> xyzqwqxqyqz = {});
C7Vector fetchPose(const CInterfaceStack* inStack, int index, const double xyzqwqxqyqz[7]);
C3Vector fetchVector3(const CInterfaceStack* inStack, int index, std::initializer_list<double> xyz = {});
C3Vector fetchVector3(const CInterfaceStack* inStack, int index, const double xyz[3]);
CMatrix fetchMatrix(const CInterfaceStack* inStack, int index);
void fetchIntArray(const CInterfaceStack* inStack, int index, std::vector<int>& outArr, std::initializer_list<int> arr = {});
void fetchIntArray(const CInterfaceStack* inStack, int index, std::vector<int>& outArr, std::vector<int>& arr);
void fetchLongArray(const CInterfaceStack* inStack, int index, std::vector<long long int>& outArr);
void fetchHandleArray(const CInterfaceStack* inStack, int index, std::vector<int>& outArr);
void fetchHandleArray(const CInterfaceStack* inStack, int index, std::vector<long long int>& outArr, std::initializer_list<long long int> arr = {});
void fetchHandleArray(const CInterfaceStack* inStack, int index, std::vector<long long int>& outArr, std::vector<long long int>& arr);
void fetchDoubleArray(const CInterfaceStack* inStack, int index, std::vector<double>& outArr, std::initializer_list<double> arr = {});
void fetchDoubleArray(const CInterfaceStack* inStack, int index, std::vector<double>& outArr, std::vector<double>& arr);
void fetchVector(const CInterfaceStack* inStack, int index, std::vector<double>& outArr, std::initializer_list<double> arr = {});
void fetchTextArray(const CInterfaceStack* inStack, int index, std::vector<std::string>& outArr, std::initializer_list<std::string> arr = {});
void fetchTextArray(const CInterfaceStack* inStack, int index, std::vector<std::string>& outArr, std::vector<std::string>& arr);

void fetchArrayAsConsecutiveNumbers(const CInterfaceStack* inStack, int index, std::vector<float>& outArr);
void fetchArrayAsConsecutiveNumbers(const CInterfaceStack* inStack, int index, std::vector<double>& outArr);

void pushNull(CInterfaceStack* outStack);
void pushBool(CInterfaceStack* outStack, bool v);
void pushLong(CInterfaceStack* outStack, long long int v);
void pushInt(CInterfaceStack* outStack, int v);
void pushHandle(CInterfaceStack* outStack, long long int v);
void pushDouble(CInterfaceStack* outStack, double v);
void pushText(CInterfaceStack* outStack, const char* v);
void pushBuffer(CInterfaceStack* outStack, const char* buff, size_t length);
void pushColor(CInterfaceStack* outStack, float v[3]);
void pushQuaternion(CInterfaceStack* outStack, const C4Vector& v);
void pushPose(CInterfaceStack* outStack, const C7Vector& v);
void pushVector3(CInterfaceStack* outStack, const C3Vector& v);
void pushVector(CInterfaceStack* outStack, const double* v, size_t length);
void pushMatrix(CInterfaceStack* outStack, const CMatrix& v);
void pushIntArray(CInterfaceStack* outStack, const int* v, size_t length);
void pushLongArray(CInterfaceStack* outStack, const long long int* v, size_t length);
void pushHandleArray(CInterfaceStack* outStack, const long long int* v, size_t length);
void pushShortHandleArray(CInterfaceStack* outStack, const int* v, size_t length);
void pushFloatArray(CInterfaceStack* outStack, const float* v, size_t length);
void pushDoubleArray(CInterfaceStack* outStack, const double* v, size_t length);
void pushTextArray(CInterfaceStack* outStack, const std::string* v, size_t length);
void pushObject(CInterfaceStack* outStack, CInterfaceStackObject* obj);

CSceneObject* getSceneObject(int identifier, const char* method, std::string* errMsg = nullptr, size_t argPos = -1);
CSceneObject* getSpecificSceneObjectType(int identifier, const char* method, int type, std::string* errMsg = nullptr, size_t argPos = -1);
CCollection* getCollection(int identifier, const char* method, std::string* errMsg = nullptr, size_t argPos = -1);
CDrawingObject* getDrawingObject(int identifier, const char* method, std::string* errMsg = nullptr, size_t argPos = -1);
CDetachedScript* getDetachedScript(int identifier, const char* method, std::string* errMsg = nullptr, size_t argPos = -1);
bool doesEntityExist(int identifier, const char* method, std::string* errMsg = nullptr, size_t argPos = -1);
std::string getInvalidArgString(size_t argPos);

extern std::string _method_test(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);

extern std::string _method_getPosition(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_setPosition(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_getQuaternion(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_setQuaternion(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_getPose(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_setPose(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_setParent(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_handleSandboxScript(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_handleAddOnScripts(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_handleCustomizationScripts(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_handleSimulationScripts(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_loadModel(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_loadModelFromBuffer(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_loadModelThumbnail(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_loadModelThumbnailFromBuffer(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_saveModel(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_saveModelToBuffer(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_loadScene(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_loadSceneFromBuffer(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_save(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_saveToBuffer(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_removeModel(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_duplicateObjects(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_addItem(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_removeItem(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_checkCollision(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_checkDistance(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_handleSensor(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_resetSensor(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_checkSensor(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_getObjects(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_addItems(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_clearItems(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_removeItems(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_callFunction(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_executeString(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_getApiInfo(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_getApiFunc(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_getStackTraceback(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_init(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_scale(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_scaleTree(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_startSimulation(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_pauseSimulation(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_stopSimulation(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_getName(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_dynamicReset(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_loadImage(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_loadImageFromBuffer(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_saveImage(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_saveImageToBuffer(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_transformImage(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_getImage(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_setImage(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_getDepth(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_relocateFrame(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_alignBoundingBox(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_logInfo(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_logWarn(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_logError(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_quit(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_systemLock(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_setStepping(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_getStepping(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_getObject(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_announceChange(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_getObjectFromUid(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_getInertia(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_setInertia(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_computeInertia(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_addForce(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_addTorque(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_ungroup(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_divide(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_packTable(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_unpackTable(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_pack(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_unpack(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_packDoubleTable(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_packFloatTable(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_packInt64Table(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_packInt32Table(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_packUInt32Table(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_packInt16Table(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_packUInt16Table(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_packInt8Table(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_packUInt8Table(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_unpackDoubleTable(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_unpackFloatTable(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_unpackInt64Table(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_unpackInt32Table(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_unpackUInt32Table(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_unpackInt16Table(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_unpackUInt16Table(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_unpackInt8Table(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_unpackUInt8Table(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_groupShapes(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_mergeShapes(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_getBoolProperty(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_getBufferProperty(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_getColorProperty(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_getFloatArrayProperty(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_getFloatProperty(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_getStringArrayProperty(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_getHandleArrayProperty(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_getHandleProperty(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_getIntArray2Property(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_getIntArrayProperty(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_getIntProperty(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_getLongProperty(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_getPoseProperty(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_getQuaternionProperty(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_getStringProperty(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_getVector3Property(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_getTableProperty(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_setBoolProperty(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_setBufferProperty(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_setColorProperty(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_setFloatArrayProperty(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_setFloatProperty(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_setStringArrayProperty(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_setHandleArrayProperty(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_setHandleProperty(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_setIntArray2Property(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_setIntArrayProperty(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_setIntProperty(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_setLongProperty(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_setPoseProperty(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_setQuaternionProperty(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_setStringProperty(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_setVector3Property(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_getMatrixProperty(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_setMatrixProperty(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_getMethodProperty(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_setMethodProperty(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_setTableProperty(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_removeProperty(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_getPropertyName(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_getPropertyInfo(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_setPropertyInfo(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_isValid(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_addCurve(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_addSignal(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_reset(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_setSignalPoint(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_removeTrace(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_snapshotTrace(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_step(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_makeClass(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_makeObject(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);

extern std::string _method_remove(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_removeObjects(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_createCamera(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_createLight(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_createGraph(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_createCustomSceneObject(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_createCustomObjectClass(int targetObj, const char* method, CDetachedScript* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);

/*


#define OCTREE_META_METHODS R"(
     "checkPointOccupancy": "sim-2.checkOctreePointOccupancy",
     "insertObject": "sim-2.insertObjectIntoOctree",
     "insertVoxels": "sim-2.insertVoxelsIntoOctree",
     "removeVoxels": "sim-2.removeVoxelsFromOctree",
     "subtractObject": "sim-2.subtractObjectFromOctree")"

#define POINTCLOUD_META_METHODS R"(
        "insertObject": "sim-2.insertObjectIntoPointCloud",
        "insertPoints": "sim-2.insertPointsIntoPointCloud",
        "intersectPoints": "sim-2.intersectPointsWithPointCloud",
        "removePoints": "sim-2.removePointsFromPointCloud",
        "subtractObject": "sim-2.subtractObjectFromPointCloud")"


*/
