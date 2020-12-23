#pragma once

#include "interfaceStackObject.h"
#include "interfaceStackTable.h"
#include <vector>
#include <string>
#include <map>

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
    void pushInt32OntoStack(int v);
    void pushInt64OntoStack(luaWrap_lua_Integer v);
    void pushStringOntoStack(const char* str,size_t l);
    void pushUCharArrayTableOntoStack(const unsigned char* arr,int l);
    void pushInt32ArrayTableOntoStack(const int* arr,int l);
    void pushInt64ArrayTableOntoStack(const luaWrap_lua_Integer* arr,int l);
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
    bool getStackStrictNumberValue(double& theValue) const;
    bool getStackStrictIntegerValue(luaWrap_lua_Integer& theValue) const;
    bool getStackDoubleValue(double& theValue) const;
    bool getStackFloatValue(float& theValue) const;
    bool getStackInt64Value(luaWrap_lua_Integer& theValue) const;
    bool getStackInt32Value(int& theValue) const;
    bool getStackStringValue(std::string& theValue) const;
    bool isStackValueNull() const;
    int getStackTableInfo(int infoType) const;
    bool getStackUCharArray(unsigned char* array,int count) const;
    bool getStackInt32Array(int* array,int count) const;
    bool getStackInt64Array(luaWrap_lua_Integer* array,int count) const;
    bool getStackFloatArray(float* array,int count) const;
    bool getStackDoubleArray(double* array,int count) const;
    bool unfoldStackTable();
    CInterfaceStackObject* getStackMapObject(const char* fieldName) const;
    bool getStackMapBoolValue(const char* fieldName,bool& val) const;
    bool getStackMapStrictNumberValue(const char* fieldName,double& val) const;
    bool getStackMapStrictIntegerValue(const char* fieldName,luaWrap_lua_Integer& val) const;
    bool getStackMapDoubleValue(const char* fieldName,double& val) const;
    bool getStackMapFloatValue(const char* fieldName,float& val) const;
    bool getStackMapLongIntValue(const char* fieldName,luaWrap_lua_Integer& val) const;
    bool getStackMapIntValue(const char* fieldName,int& val) const;
    bool getStackMapStringValue(const char* fieldName,std::string& val) const;
    bool getStackMapFloatArray(const char* fieldName,float* array,int count) const;
    std::string getBufferFromTable() const;

    void printContent(int cIndex,std::string& buffer) const;

protected:
    CInterfaceStackObject* _generateObjectFromLuaStack(luaWrap_lua_State* L,int index,std::map<void*,bool>& visitedTables);
    CInterfaceStackTable* _generateTableArrayFromLuaStack(luaWrap_lua_State* L,int index,std::map<void*,bool>& visitedTables);
    CInterfaceStackTable* _generateTableMapFromLuaStack(luaWrap_lua_State* L,int index,std::map<void*,bool>& visitedTables);
    int _countLuaStackTableEntries(luaWrap_lua_State* L,int index);

    void _pushOntoLuaStack(luaWrap_lua_State* L,CInterfaceStackObject* obj) const;

    int _interfaceStackId;
    std::vector<CInterfaceStackObject*> _stackObjects;
};
