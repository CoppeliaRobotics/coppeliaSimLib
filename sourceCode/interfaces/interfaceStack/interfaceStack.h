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
    void pushInt64OntoStack(int64_t v, bool toFront = false);
    void pushHandleOntoStack(int64_t v, bool toFront = false);
    void pushBufferOntoStack(const char* buff, size_t buffLength, bool toFront = false);
    void pushBinaryStringOntoStack(const char* buff, size_t buffLength, bool toFront = false);
    void pushTextOntoStack(const char* str, bool toFront = false);
    void pushUCharArrayOntoStack(const unsigned char* arr, size_t l, bool toFront = false);
    void pushInt32ArrayOntoStack(const int* arr, size_t l, bool toFront = false);
    void pushInt64ArrayOntoStack(const int64_t* arr, size_t l, bool toFront = false);
    void pushHandleArrayOntoStack(const int64_t* arr, size_t l, bool toFront = false);
    void pushShortHandleArrayOntoStack(const int* arr, size_t l, bool toFront = false);
    void pushFloatArrayOntoStack(const float* arr, size_t l, bool toFront = false);
    void pushDoubleArrayOntoStack(const double* arr, size_t l, bool toFront = false);
    void pushTextArrayOntoStack(const std::string* arr, size_t l, bool toFront = false);
    void pushVector3OntoStack(const double* vector, bool toFront = false);
    void pushVector3OntoStack(const C3Vector& vector, bool toFront = false);
    void pushVectorOntoStack(const double* vector, size_t l, bool toFront = false);
    void pushMatrixOntoStack(const float* matrix, size_t rows, size_t cols, bool toFront = false);
    void pushMatrixOntoStack(const double* matrix, size_t rows, size_t cols, bool toFront = false);
    void pushMatrixOntoStack(const CMatrix& matrix, bool toFront = false);
    void pushQuaternionOntoStack(const CQuaternion& quaternion, bool toFront = false);
    void pushQuaternionOntoStack(const double* q, bool toFront = false, bool xyzwLayout = false);
    void pushPoseOntoStack(const CPose& pose, bool toFront = false);
    void pushPoseOntoStack(const double* q, bool toFront = false, bool xyzqxqyqzqwLayout = false);
    void pushColorOntoStack(const float c[4], bool toFront = false);

    void insertKeyNullIntoStackTable(const char* key);
    void insertKeyBoolIntoStackTable(const char* key, bool value);
    void insertKeyFloatIntoStackTable(const char* key, float value);
    void insertKeyDoubleIntoStackTable(const char* key, double value);
    void insertKeyInt32IntoStackTable(const char* key, int value);
    void insertKeyInt64IntoStackTable(const char* key, int64_t value);
    void insertKeyHandleIntoStackTable(const char* key, int64_t value);
    void insertKeyTextIntoStackTable(const char* key, const char* value);
    void insertKeyBinaryStringIntoStackTable(const char* key, const char* value, size_t l);
    void insertKeyBufferIntoStackTable(const char* key, const char* value, size_t l);
    void insertKeyInt32ArrayIntoStackTable(const char* key, const int* arr, size_t l);
    void insertKeyInt64ArrayIntoStackTable(const char* key, const int64_t* arr, size_t l);
    void insertKeyHandleArrayIntoStackTable(const char* key, const int64_t* arr, size_t l);
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
    bool getStackStrictInt64Value(int64_t& theValue) const;
    bool getStackDoubleValue(double& theValue) const;
    bool getStackFloatValue(float& theValue) const;
    bool getStackInt64Value(int64_t& theValue) const;
    bool getStackHandleValue(int64_t& theValue) const;
    bool getStackInt32Value(int& theValue) const;
    bool getStackStringValue(std::string& theValue) const;
    bool isStackValueNull() const;
    int getStackTableInfo(int infoType) const;
    bool getStackUCharArray(unsigned char* array, int count) const;
    bool getStackInt32Array(int* array, int count) const;
    bool getStackInt64Array(int64_t* array, int count) const;
    bool getStackHandleArray(int64_t* array, int count) const;
    bool getStackFloatArray(float* array, int count) const;
    bool getStackDoubleArray(double* array, int count) const;
    bool getStackTextArray(std::vector<std::string>& array) const;
    bool getStackColor(float array[4]) const;
    bool getStackMatrix(CMatrix& m) const;
    bool getStackQuaternion(CQuaternion& q) const;
    bool getStackPose(CPose& p) const;
    const CMatrix* getStackMatrix() const;
    const CQuaternion* getStackQuaternion() const;
    const CPose* getStackPose() const;
    bool unfoldStackTable();
    CInterfaceStackObject* getStackMapObject(const char* fieldName) const;
    CInterfaceStackObject* getStackIntMapObject(int64_t key) const;
    void getStackMapKeys(std::vector<std::string>* stringKeys, std::vector<int64_t>* intKeys) const;
    bool getStackMapBoolValue(const char* fieldName, bool& val) const;
    bool getStackMapStrictDoubleValue(const char* fieldName, double& val) const;
    bool getStackMapStrictInt64Value(const char* fieldName, int64_t& val) const;
    bool getStackMapDoubleValue(const char* fieldName, double& val) const;
    bool getStackMapFloatValue(const char* fieldName, float& val) const;
    bool getStackMapInt64Value(const char* fieldName, int64_t& val) const;
    bool getStackMapInt32Value(const char* fieldName, int& val) const;
    bool getStackMapHandleValue(const char* fieldName, int64_t& val) const;
    bool getStackMapStringValue(const char* fieldName, std::string& val) const;
    bool getStackMapFloatArray(const char* fieldName, float* array, int count) const;
    bool getStackMapDoubleArray(const char* fieldName, double* array, int count) const;
    bool getStackMapColor(const char* fieldName, float array[4]) const;
    const CMatrix* getStackMapMatrix(const char* fieldName) const;
    const CQuaternion* getStackMapQuaternion(const char* fieldName) const;
    const CPose* getStackMapPose(const char* fieldName) const;
    std::string getBufferFromTable() const;
    std::string getCborEncodedBuffer(int index, int options) const;

    void fetchContent(int cIndex, std::string& buffer) const;

    int setBoolProperty(const char* pName, bool pState) override;
    int getBoolProperty(const char* pName, bool& pState) const override;
    int setIntProperty(const char* pName, int pState) override;
    int getIntProperty(const char* pName, int& pState) const override;
    int setLongProperty(const char* pName, int64_t pState) override;
    int getLongProperty(const char* pName, int64_t& pState) const override;
    int setFloatProperty(const char* pName, double pState) override;
    int getFloatProperty(const char* pName, double& pState) const override;
    int setHandleProperty(const char* pName, int64_t pState) override;
    int getHandleProperty(const char* pName, int64_t& pState) const override;
    int setStringProperty(const char* pName, const std::string& pState) override;
    int getStringProperty(const char* pName, std::string& pState) const override;
    int setBufferProperty(const char* pName, const std::string& pState) override;
    int getBufferProperty(const char* pName, std::string& pState) const override;
    int setIntArray2Property(const char* pName, const int* pState) override;
    int getIntArray2Property(const char* pName, int* pState) const override;
    int setVector3Property(const char* pName, const C3Vector& pState) override;
    int getVector3Property(const char* pName, C3Vector& pState) const override;
    int setMatrixProperty(const char* pName, const CMatrix& pState) override;
    int getMatrixProperty(const char* pName, CMatrix& pState) const override;
    int setQuaternionProperty(const char* pName, const CQuaternion& pState) override;
    int getQuaternionProperty(const char* pName, CQuaternion& pState) const override;
    int setPoseProperty(const char* pName, const CPose& pState) override;
    int getPoseProperty(const char* pName, CPose& pState) const override;
    int setColorProperty(const char* pName, const float* pState) override;
    int getColorProperty(const char* pName, float* pState) const override;
    int setFloatArrayProperty(const char* pName, const std::vector<double>& pState) override;
    int getFloatArrayProperty(const char* pName, std::vector<double>& pState) const override;
    int setIntArrayProperty(const char* pName, const std::vector<int>& pState) override;
    int getIntArrayProperty(const char* pName, std::vector<int>& pState) const override;
    int setHandleArrayProperty(const char* pName, const std::vector<int64_t>& pState) override; // ALL handle items have to be of the same type
    int getHandleArrayProperty(const char* pName, std::vector<int64_t>& pState) const override; // ALL handle items have to be of the same type
    int setStringArrayProperty(const char* pName, const std::vector<std::string>& pState) override;
    int getStringArrayProperty(const char* pName, std::vector<std::string>& pState) const override;
    int removeProperty(const char* pName) override;
    int getPropertyName(int& index, std::string& pName, std::string& appartenance, int excludeFlags) const override;
    int getPropertyInfo(const char* pName, int& info, std::string& infoTxt) const override;

  protected:
    bool _getStackLocation_write(const char* ppName, int& ind, std::string& key);
    bool _getStackLocation_read(const char* ppName, int& ind, std::string& key, int& arrIndex) const;
    static int _getPropertyTypeForStackItem(const CInterfaceStackObject* obj, std::string& str, bool firstCall = true);

    std::vector<CInterfaceStackObject*> _stackObjects;
};
