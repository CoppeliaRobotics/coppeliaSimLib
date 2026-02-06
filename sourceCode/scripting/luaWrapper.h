#pragma once

#include <string>
#include <vector>

extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

struct luaWrap_lua_Debug
{
    int event;
    const char* name;
    const char* namewhat;
    const char* what;
    const char* source;
    int currentline;
    int linedefined;
    int lastlinedefined;
    unsigned char nups;
    unsigned char nparams;
    char isvararg;
    char istailcall;
    char* short_src;
    // and other fields..
};

typedef void luaWrap_lua_State;
typedef lua_Number luaWrap_lua_Number;
typedef lua_Integer luaWrap_lua_Integer;
typedef void (*luaWrap_lua_Hook)(void* L, void* ar);
typedef int (*luaWrap_lua_CFunction)(luaWrap_lua_State* L);

int luaWrapGet_LUA_MULTRET();
int luaWrapGet_LUA_MASKCOUNT();
int luaWrapGet_LUA_MASKCALL();
int luaWrapGet_LUA_MASKRET();
int luaWrapGet_LUA_HOOKCOUNT();
int luaWrapGet_LUA_HOOKCALL();
int luaWrapGet_LUA_HOOKRET();
int luaWrapGet_LUA_REGISTRYINDEX();
int luaWrapGet_LUA_RIDX_GLOBALS();

luaWrap_lua_State* luaWrap_luaL_newstate();
void luaWrap_lua_close(luaWrap_lua_State* L);
void luaWrap_luaL_openlibs(luaWrap_lua_State* L);
void luaWrap_lua_sethook(luaWrap_lua_State* L, luaWrap_lua_Hook func, int mask, int cnt);
void luaWrap_lua_register(luaWrap_lua_State* L, const char* name, luaWrap_lua_CFunction func);
void luaWrap_lua_pushnil(luaWrap_lua_State* L);
void luaWrap_lua_pushboolean(luaWrap_lua_State* L, int b);
void luaWrap_lua_pushinteger(luaWrap_lua_State* L, luaWrap_lua_Integer n);
void luaWrap_lua_pushnumber(luaWrap_lua_State* L, luaWrap_lua_Number n);
void luaWrap_lua_pushtext(luaWrap_lua_State* L, const char* str);
void luaWrap_lua_pushbuffer(luaWrap_lua_State* L, const char* str, size_t l);       // replaces luaWrap_lua_pushlstring !
bool luaWrap_lua_pushhandle(luaWrap_lua_State* L, int h);
void luaWrap_lua_pushbinarystring(luaWrap_lua_State* L, const char* str, size_t l); // is actually luaWrap_lua_pushlstring, for very special cases !
void luaWrap_lua_pushmatrix(luaWrap_lua_State* L, const double* matrix, size_t rows, size_t cols);
void luaWrap_lua_pushvector3(luaWrap_lua_State* L, const double* vector3);
void luaWrap_lua_pushquaternion(luaWrap_lua_State* L, const double* quaternion);
void luaWrap_lua_pushpose(luaWrap_lua_State* L, const double* pose);
void luaWrap_lua_pushcolor(luaWrap_lua_State* L, const float c[3]);
void luaWrap_lua_pushcclosure(luaWrap_lua_State* L, luaWrap_lua_CFunction func, int n);
void luaWrap_lua_pushvalue(luaWrap_lua_State* L, int idx);
void luaWrap_lua_pushcfunction(luaWrap_lua_State* L, luaWrap_lua_CFunction func);
void luaWrap_lua_createtable(luaWrap_lua_State* L, int narr, int nrec);
luaWrap_lua_Integer luaWrap_lua_tointeger(luaWrap_lua_State* L, int idx);
luaWrap_lua_Number luaWrap_lua_tonumber(luaWrap_lua_State* L, int idx);
int luaWrap_lua_toboolean(luaWrap_lua_State* L, int idx);
const void* luaWrap_lua_topointer(luaWrap_lua_State* L, int idx);
const char* luaWrap_lua_tostring(luaWrap_lua_State* L, int idx);
const char* luaWrap_lua_tobuffer(luaWrap_lua_State* L, int idx, size_t* len); // replaces luaWrap_lua_tolstring
int luaWrap_lua_tohandle(luaWrap_lua_State* L, int idx); // works with an int or a metatable with a 'handle' field
int luaWrap_lua_isnumber(luaWrap_lua_State* L, int idx);                      // see also luaWrap_lua_numbertype
int luaWrap_lua_isinteger(luaWrap_lua_State* L, int idx);
int luaWrap_lua_isstring(luaWrap_lua_State* L, int idx); // see also luaWrap_lua_stringtype
bool luaWrap_lua_isnil(luaWrap_lua_State* L, int idx);
bool luaWrap_lua_isboolean(luaWrap_lua_State* L, int idx);
bool luaWrap_lua_isgeneraltable(luaWrap_lua_State* L, int idx);   // a random table, including metatable or 'buffer' metatable
bool luaWrap_lua_isnonbuffertable(luaWrap_lua_State* L, int idx); // a table that is not a 'buffer' metatable
bool luaWrap_lua_ismetatable(luaWrap_lua_State* L, int idx);
bool luaWrap_lua_isbuffer(luaWrap_lua_State* L, int idx);         // a buffer (a 'buffer' metatable)
bool luaWrap_lua_ishandle(luaWrap_lua_State* L, int idx, int* handleVal = nullptr);         // a metatable with a 'handle' field
bool luaWrap_lua_ismatrix(luaWrap_lua_State* L, int idx, size_t* rows = nullptr, size_t* cols = nullptr, std::vector<double>* matrixData = nullptr);
bool luaWrap_lua_isvector3(luaWrap_lua_State* L, int idx, std::vector<double>* vectorData = nullptr, bool strict = false);
bool luaWrap_lua_isquaternion(luaWrap_lua_State* L, int idx, std::vector<double>* quaternionData = nullptr, bool strict = false);
bool luaWrap_lua_ispose(luaWrap_lua_State* L, int idx, std::vector<double>* poseData = nullptr, bool strict = false);
bool luaWrap_lua_iscolor(luaWrap_lua_State* L, int idx, float colorData[3] = nullptr, bool strict = false);
int luaWrap_lua_getmetatable(luaWrap_lua_State* L, int idx);
int luaWrap_lua_rawequal(luaWrap_lua_State* L, int idx1, int idx2);
bool luaWrap_lua_isfunction(luaWrap_lua_State* L, int idx);
int luaWrap_lua_getstack(luaWrap_lua_State* L, int level, luaWrap_lua_Debug* deb);
int luaWrap_lua_getinfo(luaWrap_lua_State* L, const char* what, luaWrap_lua_Debug* deb);
void luaWrap_lua_setglobal(luaWrap_lua_State* L, const char* name);
void luaWrap_lua_getfield(luaWrap_lua_State* L, int idx, const char* name);
void luaWrap_lua_setfield(luaWrap_lua_State* L, int idx, const char* name);
void luaWrap_lua_geti(luaWrap_lua_State* L, int idx, lua_Integer i);
void luaWrap_lua_seti(luaWrap_lua_State* L, int idx, lua_Integer i);
void luaWrap_lua_yield(luaWrap_lua_State* L, int nresults);
int luaWrap_luaL_dostring(luaWrap_lua_State* L, const char* str);
void luaWrap_lua_getglobal(luaWrap_lua_State* L, const char* str);
void luaWrap_lua_pop(luaWrap_lua_State* L, int n);
int luaWrap_lua_gettop(luaWrap_lua_State* L);
void luaWrap_lua_settop(luaWrap_lua_State* L, int idx);
int luaWrap_lua_absindex(luaWrap_lua_State* L, int idx);
size_t luaWrap_lua_rawlen(luaWrap_lua_State* L, int idx);
void luaWrap_lua_rawgeti(luaWrap_lua_State* L, int idx, int n);
void luaWrap_lua_rawseti(luaWrap_lua_State* L, int idx, int n);
void luaWrap_lua_newtable(luaWrap_lua_State* L);
int luaWrap_luaL_loadbuffer(luaWrap_lua_State* L, const char* buff, size_t sz, const char* name);
int luaWrap_luaL_ref (luaWrap_lua_State* L, int t);
void luaWrap_lua_call(luaWrap_lua_State* L, int nargs, int nresult);
int luaWrap_lua_pcall(luaWrap_lua_State* L, int nargs, int nresult, int errfunc);
void luaWrap_lua_remove(luaWrap_lua_State* L, int idx);
void luaWrap_lua_insert(luaWrap_lua_State* L, int idx);
int luaWrap_lua_isuserdata(luaWrap_lua_State* L, int idx);
int luaWrap_lua_upvalueindex(int i);
int luaWrap_getCurrentCodeLine(luaWrap_lua_State* L);
std::string luaWrap_getCurrentCodeSource(luaWrap_lua_State* L);
void luaWrap_lua_settable(luaWrap_lua_State* L, int idx);
int luaWrap_lua_next(luaWrap_lua_State* L, int idx);
bool luaWrap_lua_stringtype(luaWrap_lua_State* L, int idx);
bool luaWrap_lua_numbertype(luaWrap_lua_State* L, int idx);
int luaWrap_lua_stype(luaWrap_lua_State* L, int idx);
int luaWrap_lua_error(luaWrap_lua_State* L);
int luaWrap_luaL_callmeta(luaWrap_lua_State *L, int obj, const char *func);

bool getFloatsFromTable(luaWrap_lua_State* L, int tablePos, size_t floatCount, float* arrayField);
bool getDoublesFromTable(luaWrap_lua_State* L, int tablePos, size_t doubleCount, double* arrayField);
bool getIntsFromTable(luaWrap_lua_State* L, int tablePos, size_t intCount, int* arrayField);
bool getLongsFromTable(luaWrap_lua_State* L, int tablePos, size_t intCount, long long int* arrayField);
bool getUIntsFromTable(luaWrap_lua_State* L, int tablePos, size_t intCount, unsigned int* arrayField);
bool getUCharsFromTable(luaWrap_lua_State* L, int tablePos, size_t intCount, unsigned char* arrayField);
bool getCharBoolsFromTable(luaWrap_lua_State* L, int tablePos, size_t boolCount, char* arrayField);
bool getStringsFromTable(luaWrap_lua_State* L, int tablePos, size_t stringCount, std::vector<std::string>& array);
bool getHandlesFromTable(luaWrap_lua_State* L, int tablePos, size_t intCount, int* arrayField);
void pushFloatTableOntoStack(luaWrap_lua_State* L, size_t floatCount, const float* arrayField);
void pushDoubleTableOntoStack(luaWrap_lua_State* L, size_t doubleCount, const double* arrayField);
void pushIntTableOntoStack(luaWrap_lua_State* L, size_t intCount, const int* arrayField);
void pushLongTableOntoStack(luaWrap_lua_State* L, size_t intCount, const long long int* arrayField);
void pushULongTableOntoStack(luaWrap_lua_State* L, size_t intCount, const unsigned long long int* arrayField);
void pushUIntTableOntoStack(luaWrap_lua_State* L, size_t intCount, const unsigned int* arrayField);
void pushUCharTableOntoStack(luaWrap_lua_State* L, size_t intCount, const unsigned char* arrayField);
void pushStringTableOntoStack(luaWrap_lua_State* L, const std::vector<std::string>& stringTable);
void pushBufferTableOntoStack(luaWrap_lua_State* L, const std::vector<std::string>& stringTable);
bool pushHandleTableOntoStack(luaWrap_lua_State* L, size_t intCount, const int* arrayField);
