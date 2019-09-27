
#pragma once

#include "interfaceStackObject.h"
#include "interfaceStackTable.h"
#include <vector>
#include <string>

class CInterfaceStack
{
public:
    CInterfaceStack();
    virtual ~CInterfaceStack();

    void setId(int id);
    int getId();
    void clear();

    // Lua interface (creation):
    void buildFromLuaStack(luaWrap_lua_State* L,int fromPos=1,int cnt=0);
    // Lua interface (read-out):
    void buildOntoLuaStack(luaWrap_lua_State* L,bool takeOnlyTop) const;
    // C interface (creation):
    CInterfaceStack* copyYourself() const;
    void pushNullOntoStack();
    void pushBoolOntoStack(bool v);
    void pushNumberOntoStack(double v);
    void pushStringOntoStack(const char* str,int l);
    void pushUCharArrayTableOntoStack(const unsigned char* arr,int l);
    void pushIntArrayTableOntoStack(const int* arr,int l);
    void pushFloatArrayTableOntoStack(const float* arr,int l);
    void pushDoubleArrayTableOntoStack(const double* arr,int l);
    void pushTableOntoStack();
    bool insertDataIntoStackTable();
    bool pushTableFromBuffer(const char* data,unsigned int l);


    // C interface (read-out)
    int getStackSize() const;
    void popStackValue(int cnt);
    bool moveStackItemToTop(int cIndex);
    bool getStackBoolValue(bool& theValue) const;
    bool getStackNumberValue(double& theValue) const;
    bool getStackStringValue(std::string& theValue) const;
    bool isStackValueNull() const;
    int getStackTableInfo(int infoType) const;
    bool getStackUCharArray(unsigned char* array,int count) const;
    bool getStackIntArray(int* array,int count) const;
    bool getStackFloatArray(float* array,int count) const;
    bool getStackDoubleArray(double* array,int count) const;
    bool unfoldStackTable();
    CInterfaceStackObject* getStackMapObject(const std::string& fieldName) const;
    bool getStackMapBoolValue(const std::string& fieldName,bool& val) const;
    bool getStackMapDoubleValue(const std::string& fieldName,double& val) const;
    bool getStackMapFloatValue(const std::string& fieldName,float& val) const;
    bool getStackMapIntValue(const std::string& fieldName,int& val) const;
    bool getStackMapStringValue(const std::string& fieldName,std::string& val) const;
    bool getStackMapFloatArray(const std::string& fieldName,float* array,int count) const;
    bool replaceStackMapBoolValue(const std::string& fieldName,bool val);
    bool replaceStackMapDoubleValue(const std::string& fieldName,double val);
    bool replaceStackMapFloatValue(const std::string& fieldName,float val);
    bool replaceStackMapIntValue(const std::string& fieldName,int val);
    bool replaceStackMapStringValue(const std::string& fieldName,std::string val);
    std::string getBufferFromTable() const;

    void printContent(int cIndex) const;

protected:
    CInterfaceStackObject* _generateObjectFromLuaStack(luaWrap_lua_State* L,int index,std::map<void*,bool>& visitedTables);
    CInterfaceStackTable* _generateTableArrayFromLuaStack(luaWrap_lua_State* L,int index,std::map<void*,bool>& visitedTables);
    CInterfaceStackTable* _generateTableMapFromLuaStack(luaWrap_lua_State* L,int index,std::map<void*,bool>& visitedTables);
    int _countLuaStackTableEntries(luaWrap_lua_State* L,int index);

    void _pushOntoLuaStack(luaWrap_lua_State* L,CInterfaceStackObject* obj) const;

    int _interfaceStackId;
    std::vector<CInterfaceStackObject*> _stackObjects;
};
