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
    void getCborObjectData(CCbor* cborObj) const;
    unsigned int createFromData(const char* data);
    static bool checkCreateFromData(const char* data,unsigned int& w,unsigned int l);

    bool isTableArray() const;
    size_t getArraySize() const;
    size_t getMapEntryCount() const;
    bool isCircularRef() const;
    void setCircularRef();

    CInterfaceStackObject* getArrayItemAtIndex(size_t ind) const;
    CInterfaceStackObject* getMapItemAtIndex(size_t ind,std::string& stringKey,double& numberKey,long long int& integerKey,bool& boolKey,int& keyType) const;
    void getAllObjectsAndClearTable(std::vector<CInterfaceStackObject*>& allObjs);

    void setUCharArray(const unsigned char* array,size_t l);
    void setInt32Array(const int* array,size_t l);
    void setInt64Array(const long long int* array,size_t l);
    void setFloatArray(const float* array,size_t l);
    void setDoubleArray(const double* array,size_t l);

    void appendArrayObject(CInterfaceStackObject* obj);
    void appendMapObject(CInterfaceStackObject* obj,const char* key,size_t l);
    void appendMapObject(CInterfaceStackObject* obj,long long int key);
    void appendMapObject(CInterfaceStackObject* obj,double key);
    void appendMapObject(CInterfaceStackObject* obj,bool key);
    void appendArrayOrMapObject(CInterfaceStackObject* obj,CInterfaceStackObject* key);

    bool getUCharArray(unsigned char* array,int count) const;
    bool getInt32Array(int* array,int count) const;
    bool getInt64Array(long long int* array,int count) const;
    bool getFloatArray(float* array,int count) const;
    bool getDoubleArray(double* array,int count) const;
    CInterfaceStackObject* getMapObject(const char* fieldName) const;

    bool removeFromKey(const CInterfaceStackObject* keyToRemove);

    int getTableInfo(int infoType) const;

protected:
    bool _areAllValueThis(int what,bool integerAndDoubleTolerant) const;

    std::vector<CInterfaceStackObject*> _tableObjects;
    bool _isTableArray;
    bool _isCircularRef;
};
