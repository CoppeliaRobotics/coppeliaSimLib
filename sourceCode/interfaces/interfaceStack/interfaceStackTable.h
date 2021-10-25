#pragma once

#include "interfaceStackObject.h"
#include <vector>

class CInterfaceStackTable : public CInterfaceStackObject
{
public:
    CInterfaceStackTable();
    virtual ~CInterfaceStackTable();

    CInterfaceStackObject* copyYourself() const;
    void printContent(int spaces,std::string& buffer) const;
    std::string getObjectData() const;
    void addCborObjectData(CCbor* cborObj) const;
    unsigned int createFromData(const char* data);
    static bool checkCreateFromData(const char* data,unsigned int& w,unsigned int l);

    bool isEmpty() const;
    bool isTableArray() const;
    size_t getArraySize() const;
    size_t getMapEntryCount() const;
    bool isCircularRef() const;
    void setCircularRef();

    CInterfaceStackObject* getArrayItemAtIndex(size_t ind) const;
    CInterfaceStackObject* getMapItemAtIndex(size_t ind,std::string& stringKey,double& numberKey,long long int& integerKey,bool& boolKey,int& keyType) const;
    void removeArrayItemAtIndex(size_t ind);
    void getAllObjectsAndClearTable(std::vector<CInterfaceStackObject*>& allObjs);

    void setUCharArray(const unsigned char* array,size_t l);
    void setInt32Array(const int* array,size_t l);
    void setInt64Array(const long long int* array,size_t l);
    void setFloatArray(const float* array,size_t l);
    void setDoubleArray(const double* array,size_t l);

    void appendArrayObject(CInterfaceStackObject* obj);
    void insertArrayObject(CInterfaceStackObject* obj,size_t pos);
    void appendMapObject(const char* key,size_t keyL,CInterfaceStackObject* obj);
    void appendMapObject(long long int key,CInterfaceStackObject* obj);
    void appendMapObject(double key,CInterfaceStackObject* obj);
    void appendMapObject(bool key,CInterfaceStackObject* obj);
    void appendArrayOrMapObject(CInterfaceStackObject* key,CInterfaceStackObject* obj);

    void appendMapObject_stringNull(const char* key);
    void appendMapObject_stringBool(const char* key,bool value);
    void appendMapObject_stringFloat(const char* key,float value);
    void appendMapObject_stringDouble(const char* key,double value);
    void appendMapObject_stringInt32(const char* key,int value);
    void appendMapObject_stringInt64(const char* key,long long int value);
    void appendMapObject_stringString(const char* key,const char* value,size_t l,bool cborCoded=false);
    void appendMapObject_stringInt32Array(const char* key,const int* arr,size_t l);
    void appendMapObject_stringInt64Array(const char* key,const long long int* arr,size_t l);
    void appendMapObject_stringFloatArray(const char* key,const float* arr,size_t l);
    void appendMapObject_stringDoubleArray(const char* key,const double* arr,size_t l);
    void appendMapObject_stringObject(const char* key,CInterfaceStackObject* obj);

    bool getUCharArray(unsigned char* array,int count) const;
    bool getInt32Array(int* array,int count) const;
    bool getInt64Array(long long int* array,int count) const;
    bool getFloatArray(float* array,int count) const;
    bool getDoubleArray(double* array,int count) const;
    CInterfaceStackObject* getMapObject(const char* fieldName) const;

    bool removeFromKey(const char* keyToRemove);
    bool removeFromKey(const CInterfaceStackObject* keyToRemove);

    int getTableInfo(int infoType) const;

protected:
    bool _areAllValueThis(int what,bool integerAndDoubleTolerant) const;

    std::vector<CInterfaceStackObject*> _tableObjects;
    bool _isTableArray;
    bool _isCircularRef;
};
