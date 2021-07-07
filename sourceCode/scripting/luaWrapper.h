#pragma once

#include <string>

extern "C" {
    #include "lua.h"
    #include "lauxlib.h"
    #include "lualib.h"
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
typedef void (*luaWrap_lua_Hook) (void* L,void* ar);
typedef int (*luaWrap_lua_CFunction) (luaWrap_lua_State* L);

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
void luaWrap_lua_sethook(luaWrap_lua_State* L,luaWrap_lua_Hook func,int mask,int cnt);
void luaWrap_lua_register(luaWrap_lua_State* L,const char* name,luaWrap_lua_CFunction func);
void luaWrap_lua_pushnil(luaWrap_lua_State* L);
void luaWrap_lua_pushboolean(luaWrap_lua_State* L,int b);
void luaWrap_lua_pushinteger(luaWrap_lua_State* L,luaWrap_lua_Integer n);
void luaWrap_lua_pushnumber(luaWrap_lua_State* L,luaWrap_lua_Number n);
void luaWrap_lua_pushstring(luaWrap_lua_State* L,const char* str);
void luaWrap_lua_pushlstring(luaWrap_lua_State* L,const char* str,size_t l);
void luaWrap_lua_pushcclosure(luaWrap_lua_State* L,luaWrap_lua_CFunction func,int n);
void luaWrap_lua_pushvalue(luaWrap_lua_State* L,int idx);
void luaWrap_lua_pushcfunction(luaWrap_lua_State* L,luaWrap_lua_CFunction func);
void luaWrap_lua_createtable(luaWrap_lua_State* L,int narr, int nrec);
luaWrap_lua_Integer luaWrap_lua_tointeger(luaWrap_lua_State* L,int idx);
luaWrap_lua_Number luaWrap_lua_tonumber(luaWrap_lua_State* L,int idx);
int luaWrap_lua_toboolean(luaWrap_lua_State* L,int idx);
const void* luaWrap_lua_topointer(luaWrap_lua_State* L,int idx);
const char* luaWrap_lua_tostring(luaWrap_lua_State* L,int idx);
const char* luaWrap_lua_tolstring(luaWrap_lua_State* L,int idx,size_t* len);
int luaWrap_lua_isnumber(luaWrap_lua_State* L,int idx);
int luaWrap_lua_isinteger(luaWrap_lua_State* L,int idx);
int luaWrap_lua_isstring(luaWrap_lua_State* L,int idx);
bool luaWrap_lua_isnil(luaWrap_lua_State* L,int idx);
bool luaWrap_lua_isboolean(luaWrap_lua_State* L,int idx);
bool luaWrap_lua_istable(luaWrap_lua_State* L,int idx);
bool luaWrap_lua_isfunction(luaWrap_lua_State* L,int idx);
int luaWrap_lua_getstack(luaWrap_lua_State* L,int level,luaWrap_lua_Debug* deb);
int luaWrap_lua_getinfo(luaWrap_lua_State* L,const char* what,luaWrap_lua_Debug* deb);
void luaWrap_lua_setglobal(luaWrap_lua_State* L,const char* name);
void luaWrap_lua_getfield(luaWrap_lua_State* L,int idx,const char* name);
void luaWrap_lua_setfield(luaWrap_lua_State* L,int idx,const char* name);
void luaWrap_lua_yield(luaWrap_lua_State* L,int nresults);
int luaWrap_luaL_dostring(luaWrap_lua_State* L,const char* str);
void luaWrap_lua_getglobal(luaWrap_lua_State* L,const char* str);
void luaWrap_lua_pop(luaWrap_lua_State* L,int n);
int luaWrap_lua_gettop(luaWrap_lua_State* L);
void luaWrap_lua_settop(luaWrap_lua_State* L,int idx);
size_t luaWrap_lua_rawlen(luaWrap_lua_State* L,int idx);
void luaWrap_lua_rawgeti(luaWrap_lua_State* L,int idx,int n);
void luaWrap_lua_rawseti(luaWrap_lua_State* L,int idx,int n);
void luaWrap_lua_newtable(luaWrap_lua_State* L);
int luaWrap_luaL_loadbuffer(luaWrap_lua_State* L,const char* buff,size_t sz,const char* name);
int luaWrap_lua_pcall(luaWrap_lua_State* L,int nargs,int nresult,int errfunc);
void luaWrap_lua_remove(luaWrap_lua_State* L,int idx);
void luaWrap_lua_insert(luaWrap_lua_State* L,int idx);
int luaWrap_lua_isuserdata(luaWrap_lua_State* L,int idx);
int luaWrap_lua_upvalueindex(int i);
int luaWrap_getCurrentCodeLine(luaWrap_lua_State* L);
std::string luaWrap_getCurrentCodeSource(luaWrap_lua_State* L);
void luaWrap_lua_settable(luaWrap_lua_State* L,int idx);
int luaWrap_lua_next(luaWrap_lua_State* L,int idx);
int luaWrap_lua_stype(luaWrap_lua_State* L,int idx);
int luaWrap_lua_error(luaWrap_lua_State* L);

