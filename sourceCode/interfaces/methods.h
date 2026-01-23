#pragma once

#include <string>
#include <interfaceStack.h>
#include <app.h>
#include <scriptObject.h>

extern std::string callMethod(int targetObj, const char* method, CScriptObject* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
bool checkInputArguments(const char* method, const CInterfaceStack* inStack, std::string* errStr, std::vector<int> inargs);

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
void fetchHandleArray(const CInterfaceStack* inStack, int index, std::vector<long long int>& outArr, std::initializer_list<long long int> arr = {});
void fetchHandleArray(const CInterfaceStack* inStack, int index, std::vector<long long int>& outArr, std::vector<long long int>& arr);
void fetchDoubleArray(const CInterfaceStack* inStack, int index, std::vector<double>& outArr, std::initializer_list<double> arr = {});
void fetchDoubleArray(const CInterfaceStack* inStack, int index, std::vector<double>& outArr, std::vector<double>& arr);
void fetchVector(const CInterfaceStack* inStack, int index, std::vector<double>& outArr, std::initializer_list<double> arr = {});
void fetchTextArray(const CInterfaceStack* inStack, int index, std::vector<std::string>& outArr, std::initializer_list<std::string> arr = {});
void fetchTextArray(const CInterfaceStack* inStack, int index, std::vector<std::string>& outArr, std::vector<std::string>& arr);

void pushNull(CInterfaceStack* outStack);
void pushBool(CInterfaceStack* outStack, bool v);
void pushLong(CInterfaceStack* outStack, long long int v);
void pushInt(CInterfaceStack* outStack, int v);
void pushHandle(CInterfaceStack* outStack, int v);
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
void pushHandleArray(CInterfaceStack* outStack, const long long int* v, size_t length);
void pushShortHandleArray(CInterfaceStack* outStack, const int* v, size_t length);
void pushDoubleArray(CInterfaceStack* outStack, const double* v, size_t length);
void pushTextArray(CInterfaceStack* outStack, const std::string* v, size_t length);

CSceneObject* getSceneObject(int identifier, std::string* errMsg = nullptr, size_t argPos = -1);
CSceneObject* getSpecificSceneObjectType(int identifier, int type, std::string* errMsg = nullptr, size_t argPos = -1);
CCollection* getCollection(int identifier, std::string* errMsg = nullptr, size_t argPos = -1);
CDrawingObject* getDrawingObject(int identifier, std::string* errMsg = nullptr, size_t argPos = -1);
CScriptObject* getDetachedScript(int identifier, std::string* errMsg = nullptr, size_t argPos = -1);
bool doesEntityExist(int identifier, std::string* errMsg = nullptr, size_t argPos = -1);
std::string getInvalidArgString(size_t argPos);

extern std::string _method_test(int targetObj, const char* method, CScriptObject* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);

extern std::string _method_getPosition(int targetObj, const char* method, CScriptObject* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_setPosition(int targetObj, const char* method, CScriptObject* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_getQuaternion(int targetObj, const char* method, CScriptObject* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_setQuaternion(int targetObj, const char* method, CScriptObject* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_getPose(int targetObj, const char* method, CScriptObject* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_setPose(int targetObj, const char* method, CScriptObject* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_setParent(int targetObj, const char* method, CScriptObject* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_handleSandboxScript(int targetObj, const char* method, CScriptObject* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_handleAddOnScripts(int targetObj, const char* method, CScriptObject* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_handleCustomizationScripts(int targetObj, const char* method, CScriptObject* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_handleSimulationScripts(int targetObj, const char* method, CScriptObject* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_loadModel(int targetObj, const char* method, CScriptObject* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_loadModelFromBuffer(int targetObj, const char* method, CScriptObject* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_loadModelThumbnail(int targetObj, const char* method, CScriptObject* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_loadModelThumbnailFromBuffer(int targetObj, const char* method, CScriptObject* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_saveModel(int targetObj, const char* method, CScriptObject* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_saveModelToBuffer(int targetObj, const char* method, CScriptObject* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_loadScene(int targetObj, const char* method, CScriptObject* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_loadSceneFromBuffer(int targetObj, const char* method, CScriptObject* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_saveScene(int targetObj, const char* method, CScriptObject* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_saveSceneToBuffer(int targetObj, const char* method, CScriptObject* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_removeModel(int targetObj, const char* method, CScriptObject* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_remove(int targetObj, const char* method, CScriptObject* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_removeObjects(int targetObj, const char* method, CScriptObject* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_duplicateObjects(int targetObj, const char* method, CScriptObject* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_addItem(int targetObj, const char* method, CScriptObject* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_removeItem(int targetObj, const char* method, CScriptObject* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_checkCollision(int targetObj, const char* method, CScriptObject* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_checkDistance(int targetObj, const char* method, CScriptObject* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_handleSensor(int targetObj, const char* method, CScriptObject* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_resetSensor(int targetObj, const char* method, CScriptObject* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_checkSensor(int targetObj, const char* method, CScriptObject* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
extern std::string _method_getObjects(int targetObj, const char* method, CScriptObject* currentScript, const CInterfaceStack* inStack, CInterfaceStack* outStack);
