
#pragma once

#include "interfaceStackObject.h"
#include <vector>

class CInterfaceStackTable : public CInterfaceStackObject
{
public:
    CInterfaceStackTable();
    virtual ~CInterfaceStackTable();

    CInterfaceStackObject* copyYourself() const;
    void printContent(int spaces) const;
    std::string getObjectData() const;
    unsigned int createFromData(const char* data);
    static bool checkCreateFromData(const char* data,unsigned int& w,unsigned int l);

    bool isTableArray() const;
    int getArraySize() const;
    int getMapEntryCount() const;
    bool isCircularRef() const;
    void setCircularRef();

    CInterfaceStackObject* getArrayItemAtIndex(int ind) const;
    CInterfaceStackObject* getMapItemAtIndex(int ind,std::string& stringKey,double& numberKey,bool& boolKey,int& keyType) const;
    void getAllObjectsAndClearTable(std::vector<CInterfaceStackObject*>& allObjs);

    void setUCharArray(const unsigned char* array,int l);
    void setIntArray(const int* array,int l);
    void setFloatArray(const float* array,int l);
    void setDoubleArray(const double* array,int l);

    void appendArrayObject(CInterfaceStackObject* obj);
    void appendMapObject(CInterfaceStackObject* obj,const char* key);
    void appendMapObject(CInterfaceStackObject* obj,double key);
    void appendMapObject(CInterfaceStackObject* obj,bool key);
    void appendArrayOrMapObject(CInterfaceStackObject* obj,CInterfaceStackObject* key);

    bool getUCharArray(unsigned char* array,int count) const;
    bool getIntArray(int* array,int count) const;
    bool getFloatArray(float* array,int count) const;
    bool getDoubleArray(double* array,int count) const;
    CInterfaceStackObject* getMapObject(const std::string& fieldName) const;

    bool removeFromKey(const CInterfaceStackObject* keyToRemove);

    int getTableInfo(int infoType) const;

protected:
    bool _areAllValueThis(int what) const;

    std::vector<CInterfaceStackObject*> _tableObjects;
    bool _isTableArray;
    bool _isCircularRef;
};
