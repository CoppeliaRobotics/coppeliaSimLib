#pragma once

#include <string>
#include <interfaceStack.h>

extern std::string callMethod(int target, const char* methodName, const CInterfaceStack* inStack, CInterfaceStack* outStack);
bool checkInputArguments(int target, const char* method, const CInterfaceStack* inStack, std::string* errStr, std::vector<int> inargs);

bool fetchBoolArg(const CInterfaceStack* inStack, int index, bool defaultValue = false);
long long int fetchLongArg(const CInterfaceStack* inStack, int index, long long int defaultValue = -1);
int fetchIntArg(const CInterfaceStack* inStack, int index, int defaultValue = -1);
int fetchHandleArg(const CInterfaceStack* inStack, int index, int defaultValue = -1);
double fetchDoubleArg(const CInterfaceStack* inStack, int index, double defaultValue = 0.0);
std::string fetchTextArg(const CInterfaceStack* inStack, int index, const char* txt = "");
std::string fetchBufferArg(const CInterfaceStack* inStack, int index);
void fetchColorArg(const CInterfaceStack* inStack, int index, float outArr[3], std::initializer_list<float> arr = {});
void fetchColorArg(const CInterfaceStack* inStack, int index, float outArr[3], const float defaultArr[3]);
C4Vector fetchQuaternionArg(const CInterfaceStack* inStack, int index, std::initializer_list<double> wxyz = {});
C4Vector fetchQuaternionArg(const CInterfaceStack* inStack, int index, const double wxyz[4]);
C7Vector fetchPoseArg(const CInterfaceStack* inStack, int index, std::initializer_list<double> xyzqwqxqyqz = {});
C7Vector fetchPoseArg(const CInterfaceStack* inStack, int index, const double xyzqwqxqyqz[7]);
C3Vector fetchVector3Arg(const CInterfaceStack* inStack, int index, std::initializer_list<double> xyz = {});
C3Vector fetchVector3Arg(const CInterfaceStack* inStack, int index, const double xyz[3]);
CMatrix fetchMatrixArg(const CInterfaceStack* inStack, int index);
void fetchIntArrayArg(const CInterfaceStack* inStack, int index, std::vector<int>& outArr, std::initializer_list<int> arr = {});
void fetchIntArrayArg(const CInterfaceStack* inStack, int index, std::vector<int>& outArr, std::vector<int>& arr);
void fetchHandleArrayArg(const CInterfaceStack* inStack, int index, std::vector<int>& outArr, std::initializer_list<int> arr = {});
void fetchHandleArrayArg(const CInterfaceStack* inStack, int index, std::vector<int>& outArr, std::vector<int>& arr);
void fetchDoubleArrayArg(const CInterfaceStack* inStack, int index, std::vector<double>& outArr, std::initializer_list<double> arr = {});
void fetchDoubleArrayArg(const CInterfaceStack* inStack, int index, std::vector<double>& outArr, std::vector<double>& arr);
void fetchVectorArg(const CInterfaceStack* inStack, int index, std::vector<double>& outArr, std::initializer_list<double> arr = {});
void fetchTextArrayArg(const CInterfaceStack* inStack, int index, std::vector<std::string>& outArr, std::initializer_list<std::string> arr = {});
void fetchTextArrayArg(const CInterfaceStack* inStack, int index, std::vector<std::string>& outArr, std::vector<std::string>& arr);

extern std::string _method_getPosition(int target, const char* method, const CInterfaceStack* inStack, CInterfaceStack* outStack);
