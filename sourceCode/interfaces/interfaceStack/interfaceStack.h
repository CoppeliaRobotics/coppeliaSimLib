#pragma once

#include <interfaceStackObject.h>
#include <interfaceStackTable.h>
#include <vector>
#include <string>
#include <simMath/mXnMatrix.h>
#include <simMath/4Vector.h>
#include <simMath/7Vector.h>
#include <map>

class CInterfaceStack
{
  public:
    CInterfaceStack(int a, int b, const char* c);
    virtual ~CInterfaceStack();

    void setId(int id);
    int getId() const;
    void clear();

    // C interface (creation):
    CInterfaceStack* copyYourself() const;
    void copyFrom(const CInterfaceStack* source);

    void pushObjectOntoStack(CInterfaceStackObject* obj, bool toFront = false);
    void pushNullOntoStack(bool toFront = false);
    void pushBoolOntoStack(bool v, bool toFront = false);
    void pushFloatOntoStack(float v, bool toFront = false);
    void pushDoubleOntoStack(double v, bool toFront = false);
    void pushInt32OntoStack(int v, bool toFront = false);
    void pushInt64OntoStack(long long int v, bool toFront = false);
    void pushHandleOntoStack(long long int v, bool toFront = false);
    void pushBufferOntoStack(const char* buff, size_t buffLength, bool toFront = false);
    void pushBinaryStringOntoStack(const char* buff, size_t buffLength, bool toFront = false);
    void pushTextOntoStack(const char* str, bool toFront = false);
    void pushUCharArrayOntoStack(const unsigned char* arr, size_t l, bool toFront = false);
    void pushInt32ArrayOntoStack(const int* arr, size_t l, bool toFront = false);
    void pushInt64ArrayOntoStack(const long long int* arr, size_t l, bool toFront = false);
    void pushHandleArrayOntoStack(const long long int* arr, size_t l, bool toFront = false);
    void pushFloatArrayOntoStack(const float* arr, size_t l, bool toFront = false);
    void pushDoubleArrayOntoStack(const double* arr, size_t l, bool toFront = false);
    void pushMatrixOntoStack(const double* matrix, size_t rows, size_t cols, bool toFront = false);
    void pushQuaternionOntoStack(const double* q, bool toFront = false, bool xyzwLayout = false);
    void pushPoseOntoStack(const double* q, bool toFront = false, bool xyzqxqyqzqwLayout = false);
    void pushColorOntoStack(const float c[3], bool toFront = false);

    void insertKeyNullIntoStackTable(const char* key);
    void insertKeyBoolIntoStackTable(const char* key, bool value);
    void insertKeyFloatIntoStackTable(const char* key, float value);
    void insertKeyDoubleIntoStackTable(const char* key, double value);
    void insertKeyInt32IntoStackTable(const char* key, int value);
    void insertKeyInt64IntoStackTable(const char* key, long long int value);
    void insertKeyHandleIntoStackTable(const char* key, long long int value);
    void insertKeyTextIntoStackTable(const char* key, const char* value);
    void insertKeyBinaryStringIntoStackTable(const char* key, const char* value, size_t l);
    void insertKeyBufferIntoStackTable(const char* key, const char* value, size_t l);
    void insertKeyInt32ArrayIntoStackTable(const char* key, const int* arr, size_t l);
    void insertKeyInt64ArrayIntoStackTable(const char* key, const long long int* arr, size_t l);
    void insertKeyHandleArrayIntoStackTable(const char* key, const long long int* arr, size_t l);
    void insertKeyFloatArrayIntoStackTable(const char* key, const float* arr, size_t l);
    void insertKeyDoubleArrayIntoStackTable(const char* key, const double* arr, size_t l);
    void insertKeyMatrixIntoStackTable(const char* key, const double* matrix, size_t rows, size_t cols);
    void insertKeyQuaternionIntoStackTable(const char* key, const double* q, bool xyzwLayout = false);
    void insertKeyPoseIntoStackTable(const char* key, const double* p, bool xyzqxqyqzqwLayout = false);
    void insertKeyColorIntoStackTable(const char* key, const float c[3]);

    bool insertItem(int pos, CInterfaceStackObject* item);

    void pushTableOntoStack();
    bool insertDataIntoStackTable();
    bool pushTableFromBuffer(const char* data, unsigned int l);

    // C interface (read-out)
    int getStackSize() const;
    void popStackValue(int cnt);
    bool moveStackItemToTop(int cIndex);
    int getStackItemType(int cIndex);
    int getStackStringType(int cIndex);
    CInterfaceStackObject* getStackObjectFromIndex(size_t index) const;
    bool replaceStackObjectFromIndex(size_t index, CInterfaceStackObject* obj);
    CInterfaceStackObject* detachStackObjectFromIndex(size_t index);
    bool getStackBoolValue(bool& theValue) const;
    bool getStackStrictDoubleValue(double& theValue) const;
    bool getStackStrictInt64Value(long long int& theValue) const;
    bool getStackDoubleValue(double& theValue) const;
    bool getStackFloatValue(float& theValue) const;
    bool getStackInt64Value(long long int& theValue) const;
    bool getStackHandleValue(long long int& theValue) const;
    bool getStackInt32Value(int& theValue) const;
    bool getStackStringValue(std::string& theValue) const;
    bool isStackValueNull() const;
    int getStackTableInfo(int infoType) const;
    bool getStackUCharArray(unsigned char* array, int count) const;
    bool getStackInt32Array(int* array, int count) const;
    bool getStackInt64Array(long long int* array, int count) const;
    bool getStackHandleArray(long long int* array, int count) const;
    bool getStackFloatArray(float* array, int count) const;
    bool getStackDoubleArray(double* array, int count) const;
    bool getStackColor(float array[3]) const;
    const CMatrix* getStackMatrix() const;
    const C4Vector* getStackQuaternion() const;
    const C7Vector* getStackPose() const;
    bool unfoldStackTable();
    CInterfaceStackObject* getStackMapObject(const char* fieldName) const;
    CInterfaceStackObject* getStackIntMapObject(long long int key) const;
    void getStackMapKeys(std::vector<std::string>* stringKeys, std::vector<long long int>* intKeys) const;
    bool getStackMapBoolValue(const char* fieldName, bool& val) const;
    bool getStackMapStrictDoubleValue(const char* fieldName, double& val) const;
    bool getStackMapStrictInt64Value(const char* fieldName, long long int& val) const;
    bool getStackMapDoubleValue(const char* fieldName, double& val) const;
    bool getStackMapFloatValue(const char* fieldName, float& val) const;
    bool getStackMapInt64Value(const char* fieldName, long long int& val) const;
    bool getStackMapInt32Value(const char* fieldName, int& val) const;
    bool getStackMapHandleValue(const char* fieldName, long long int& val) const;
    bool getStackMapStringValue(const char* fieldName, std::string& val) const;
    bool getStackMapFloatArray(const char* fieldName, float* array, int count) const;
    bool getStackMapDoubleArray(const char* fieldName, double* array, int count) const;
    bool getStackMapColor(const char* fieldName, float array[3]) const;
    const CMatrix* getStackMapMatrix(const char* fieldName) const;
    const C4Vector* getStackMapQuaternion(const char* fieldName) const;
    const C7Vector* getStackMapPose(const char* fieldName) const;
    std::string getBufferFromTable() const;
    std::string getCborEncodedBuffer(int options) const;

    void printContent(int cIndex, std::string& buffer) const;

  protected:
    int _interfaceStackId;
    std::vector<CInterfaceStackObject*> _stackObjects;
};
