#pragma once

#include "interfaceStackObject.h"
#include "interfaceStackTable.h"
#include <vector>
#include <string>
#include <map>

class CInterfaceStack
{
public:
    CInterfaceStack(int a,int b,const char* c);
    virtual ~CInterfaceStack();

    void setId(int id);
    int getId() const;
    void clear();

    // C interface (creation):
    CInterfaceStack* copyYourself() const;
    void copyFrom(const CInterfaceStack* source);

    void pushObjectOntoStack(CInterfaceStackObject* obj);
    void pushNullOntoStack();
    void pushBoolOntoStack(bool v);
    void pushFloatOntoStack(float v);
    void pushDoubleOntoStack(double v);
    void pushInt32OntoStack(int v);
    void pushInt64OntoStack(long long int v);
    void pushStringOntoStack(const char* str,size_t l);
    void pushUCharArrayOntoStack(const unsigned char* arr,size_t l);
    void pushInt32ArrayOntoStack(const int* arr,size_t l);
    void pushInt64ArrayOntoStack(const long long int* arr,size_t l);
    void pushFloatArrayOntoStack(const float* arr,size_t l);
    void pushDoubleArrayOntoStack(const double* arr,size_t l);

    void insertKeyNullIntoStackTable(const char* key);
    void insertKeyBoolIntoStackTable(const char* key,bool value);
    void insertKeyFloatIntoStackTable(const char* key,float value);
    void insertKeyDoubleIntoStackTable(const char* key,double value);
    void insertKeyInt32IntoStackTable(const char* key,int value);
    void insertKeyInt64IntoStackTable(const char* key,long long int value);
    void insertKeyStringIntoStackTable(const char* key,const char* value,size_t l);
    void insertKeyInt32ArrayIntoStackTable(const char* key,const int* arr,size_t l);
    void insertKeyInt64ArrayIntoStackTable(const char* key,const long long int* arr,size_t l);
    void insertKeyFloatArrayIntoStackTable(const char* key,const float* arr,size_t l);
    void insertKeyDoubleArrayIntoStackTable(const char* key,const double* arr,size_t l);

    void pushTableOntoStack();
    bool insertDataIntoStackTable();
    bool pushTableFromBuffer(const char* data,unsigned int l);


    // C interface (read-out)
    int getStackSize() const;
    void popStackValue(int cnt);
    bool moveStackItemToTop(int cIndex);
    CInterfaceStackObject* getStackObjectFromIndex(size_t index) const;
    CInterfaceStackObject* detachStackObjectFromIndex(size_t index);
    bool getStackBoolValue(bool& theValue) const;
    bool getStackStrictDoubleValue(double& theValue) const;
    bool getStackStrictInt64Value(long long int& theValue) const;
    bool getStackDoubleValue(double& theValue) const;
    bool getStackFloatValue(float& theValue) const;
    bool getStackInt64Value(long long int& theValue) const;
    bool getStackInt32Value(int& theValue) const;
    bool getStackStringValue(std::string& theValue) const;
    bool isStackValueNull() const;
    int getStackTableInfo(int infoType) const;
    bool getStackUCharArray(unsigned char* array,int count) const;
    bool getStackInt32Array(int* array,int count) const;
    bool getStackInt64Array(long long int* array,int count) const;
    bool getStackFloatArray(float* array,int count) const;
    bool getStackDoubleArray(double* array,int count) const;
    bool unfoldStackTable();
    CInterfaceStackObject* getStackMapObject(const char* fieldName) const;
    bool getStackMapBoolValue(const char* fieldName,bool& val) const;
    bool getStackMapStrictDoubleValue(const char* fieldName,double& val) const;
    bool getStackMapStrictInt64Value(const char* fieldName,long long int& val) const;
    bool getStackMapDoubleValue(const char* fieldName,double& val) const;
    bool getStackMapFloatValue(const char* fieldName,float& val) const;
    bool getStackMapInt64Value(const char* fieldName,long long int& val) const;
    bool getStackMapInt32Value(const char* fieldName,int& val) const;
    bool getStackMapStringValue(const char* fieldName,std::string& val) const;
    bool getStackMapFloatArray(const char* fieldName,float* array,int count) const;
    std::string getBufferFromTable() const;
    std::string getCborEncodedBufferFromTable(int options) const;

    void printContent(int cIndex,std::string& buffer) const;

protected:
    int _interfaceStackId;
    std::vector<CInterfaceStackObject*> _stackObjects;
};
