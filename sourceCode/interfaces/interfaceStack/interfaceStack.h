#pragma once

#include <interfaceStackObject.h>
#include <interfaceStackTable.h>
#include <vector>
#include <string>
#include <simMath/mXnMatrix.h>
#include <simMath/4Vector.h>
#include <simMath/7Vector.h>
#include <map>
#include <obj.h>

// ----------------------------------------------------------------------------------------------
#define FUNCX(name, str, v1, v2, t1, t2) extern const SProperty name;
STACK_PROPERTIES
#undef FUNCX
extern const std::vector<SProperty> allProps_stack;
// ----------------------------------------------------------------------------------------------

class CInterfaceStack : public Obj
{
  public:
    CInterfaceStack(int a, int b, const char* c);
    virtual ~CInterfaceStack();

    void setId(int id);
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
    void pushShortHandleArrayOntoStack(const int* arr, size_t l, bool toFront = false);
    void pushFloatArrayOntoStack(const float* arr, size_t l, bool toFront = false);
    void pushDoubleArrayOntoStack(const double* arr, size_t l, bool toFront = false);
    void pushTextArrayOntoStack(const std::string* arr, size_t l, bool toFront = false);
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

    int setBoolProperty(const char* pName, bool pState);
    int getBoolProperty(const char* pName, bool& pState) const;
    int setIntProperty(const char* pName, int pState);
    int getIntProperty(const char* pName, int& pState) const;
    int setLongProperty(const char* pName, long long int pState);
    int getLongProperty(const char* pName, long long int& pState) const override;
    int setFloatProperty(const char* pName, double pState);
    int getFloatProperty(const char* pName, double& pState) const;
    int setHandleProperty(const char* pName, long long int pState);
    int getHandleProperty(const char* pName, long long int& pState) const;
    int setStringProperty(const char* pName, const char* pState);
    int getStringProperty(const char* pName, std::string& pState) const override;
    int setBufferProperty(const char* pName, const char* buffer, int bufferL);
    int getBufferProperty(const char* pName, std::string& pState) const;
    int setIntArray2Property(const char* pName, const int* pState);
    int getIntArray2Property(const char* pName, int* pState) const;
    int setVector2Property(const char* pName, const double* pState);
    int getVector2Property(const char* pName, double* pState) const;
    int setVector3Property(const char* pName, const C3Vector& pState);
    int getVector3Property(const char* pName, C3Vector& pState) const;
    int setMatrixProperty(const char* pName, const CMatrix& pState);
    int getMatrixProperty(const char* pName, CMatrix& pState) const;
    int setMatrix3x3Property(const char* pName, const CMatrix& pState);
    int getMatrix3x3Property(const char* pName, CMatrix& pState) const;
    int setMatrix4x4Property(const char* pName, const CMatrix& pState);
    int getMatrix4x4Property(const char* pName, CMatrix& pState) const;
    int setQuaternionProperty(const char* pName, const C4Vector& pState);
    int getQuaternionProperty(const char* pName, C4Vector& pState) const;
    int setPoseProperty(const char* pName, const C7Vector& pState);
    int getPoseProperty(const char* pName, C7Vector& pState) const;
    int setColorProperty(const char* pName, const float* pState);
    int getColorProperty(const char* pName, float* pState) const;
    int setFloatArrayProperty(const char* pName, const double* v, int vL);
    int getFloatArrayProperty(const char* pName, std::vector<double>& pState) const;
    int setIntArrayProperty(const char* pName, const int* v, int vL);
    int getIntArrayProperty(const char* pName, std::vector<int>& pState) const;
    int setHandleArrayProperty(const char* pName, const long long int* v, int vL); // ALL handle items have to be of the same type
    int getHandleArrayProperty(const char* pName, std::vector<long long int>& pState) const; // ALL handle items have to be of the same type
    int setStringArrayProperty(const char* pName, const std::vector<std::string>& pState);
    int getStringArrayProperty(const char* pName, std::vector<std::string>& pState) const;
    int removeProperty(const char* pName);
    int getPropertyName(int& index, std::string& pName, std::string& appartenance, int excludeFlags) const override;
    int getPropertyInfo(const char* pName, int& info, std::string& infoTxt) const override;

  protected:
    bool _getStackLocation_write(const char* ppName, int& ind, std::string& key);
    bool _getStackLocation_read(const char* ppName, int& ind, std::string& key, int& arrIndex) const;
    static int _getPropertyTypeForStackItem(const CInterfaceStackObject* obj, std::string& str, bool firstCall = true);

    std::vector<CInterfaceStackObject*> _stackObjects;
};
