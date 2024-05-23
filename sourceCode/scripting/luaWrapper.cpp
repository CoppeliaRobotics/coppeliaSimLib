#include <luaWrapper.h>
#include <simLib/simConst.h>
#include <app.h>

int luaWrapGet_LUA_MULTRET()
{
    return (LUA_MULTRET);
}

int luaWrapGet_LUA_MASKCOUNT()
{
    return (LUA_MASKCOUNT);
}

int luaWrapGet_LUA_MASKCALL()
{
    return (LUA_MASKCALL);
}

int luaWrapGet_LUA_MASKRET()
{
    return (LUA_MASKRET);
}

int luaWrapGet_LUA_HOOKCOUNT()
{
    return (LUA_HOOKCOUNT);
}

int luaWrapGet_LUA_HOOKCALL()
{
    return (LUA_HOOKCALL);
}

int luaWrapGet_LUA_HOOKRET()
{
    return (LUA_HOOKRET);
}

int luaWrapGet_LUA_REGISTRYINDEX()
{
    return (LUA_REGISTRYINDEX);
}

int luaWrapGet_LUA_RIDX_GLOBALS()
{
    return (LUA_RIDX_GLOBALS);
}

luaWrap_lua_State *luaWrap_luaL_newstate()
{
    return ((luaWrap_lua_State *)luaL_newstate());
}

void luaWrap_lua_close(luaWrap_lua_State *L)
{
    lua_close((lua_State *)L);
}

void luaWrap_luaL_openlibs(luaWrap_lua_State *L)
{
    luaL_openlibs((lua_State *)L);
}

void luaWrap_lua_sethook(luaWrap_lua_State *L, luaWrap_lua_Hook func, int mask, int cnt)
{
    lua_sethook((lua_State *)L, (lua_Hook)func, mask, cnt);
}

void luaWrap_lua_register(luaWrap_lua_State *L, const char *name, luaWrap_lua_CFunction func)
{
    lua_register((lua_State *)L, name, (lua_CFunction)func);
}

void luaWrap_lua_pushnumber(luaWrap_lua_State *L, luaWrap_lua_Number n)
{
    lua_pushnumber((lua_State *)L, n);
}

void luaWrap_lua_pushnil(luaWrap_lua_State *L)
{
    lua_pushnil((lua_State *)L);
}

void luaWrap_lua_pushboolean(luaWrap_lua_State *L, int b)
{
    lua_pushboolean((lua_State *)L, b);
}

void luaWrap_lua_pushinteger(luaWrap_lua_State *L, luaWrap_lua_Integer n)
{
    lua_pushinteger((lua_State *)L, n);
}

void luaWrap_lua_pushtext(luaWrap_lua_State *L, const char *str)
{
    lua_pushstring((lua_State *)L, str);
}

void luaWrap_lua_pushcclosure(luaWrap_lua_State *L, luaWrap_lua_CFunction func, int n)
{
    lua_pushcclosure((lua_State *)L, (lua_CFunction)func, n);
}

void luaWrap_lua_pushvalue(luaWrap_lua_State *L, int idx)
{
    lua_pushvalue((lua_State *)L, idx);
}

void luaWrap_lua_pushcfunction(luaWrap_lua_State *L, luaWrap_lua_CFunction func)
{
    lua_pushcfunction((lua_State *)L, (lua_CFunction)func);
}

void luaWrap_lua_createtable(luaWrap_lua_State *L, int narr, int nrec)
{
    lua_createtable((lua_State *)L, narr, nrec);
}

luaWrap_lua_Integer luaWrap_lua_tointeger(luaWrap_lua_State *L, int idx)
{
    if (lua_isinteger((lua_State *)L, idx))
        return (lua_tointeger((lua_State *)L, idx));
    return ((luaWrap_lua_Integer)lua_tonumber((lua_State *)L, idx));
}

luaWrap_lua_Number luaWrap_lua_tonumber(luaWrap_lua_State *L, int idx)
{
    return ((luaWrap_lua_Number)lua_tonumber((lua_State *)L, idx));
}

int luaWrap_lua_toboolean(luaWrap_lua_State *L, int idx)
{
    return (lua_toboolean((lua_State *)L, idx));
}

const void *luaWrap_lua_topointer(luaWrap_lua_State *L, int idx)
{
    return (lua_topointer((lua_State *)L, idx));
}

const char *luaWrap_lua_tostring(luaWrap_lua_State *L, int idx)
{ // buffer-tolerant
    size_t len;
    return luaWrap_lua_tobuffer(L, idx, &len);
}

int luaWrap_lua_isnumber(luaWrap_lua_State *L, int idx)
{
    return (lua_isnumber((lua_State *)L, idx));
}

int luaWrap_lua_isinteger(luaWrap_lua_State *L, int idx)
{
    return (lua_isinteger((lua_State *)L, idx));
}

int luaWrap_lua_isstring(luaWrap_lua_State *L, int idx)
{ // buffer-tolerant
    int retVal = 0;
    if (luaWrap_lua_isbuffer(L, idx))
        retVal = 1;
    else
        retVal = lua_isstring((lua_State *)L, idx);
    return retVal;
}

bool luaWrap_lua_isboolean(luaWrap_lua_State *L, int idx)
{
    return (lua_isboolean((lua_State *)L, idx));
}

bool luaWrap_lua_isnil(luaWrap_lua_State *L, int idx)
{
    return (lua_isnil((lua_State *)L, idx));
}

int luaWrap_lua_getmetatable(luaWrap_lua_State *L, int idx)
{
    return (lua_getmetatable((lua_State *)L, idx));
}

int luaWrap_lua_rawequal(luaWrap_lua_State *L, int idx1, int idx2)
{
    return (lua_rawequal((lua_State *)L, idx1, idx2));
}

bool luaWrap_lua_isfunction(luaWrap_lua_State *L, int idx)
{
    return (lua_isfunction((lua_State *)L, idx));
}

int luaWrap_lua_getstack(luaWrap_lua_State *L, int level, luaWrap_lua_Debug *deb)
{
    return (lua_getstack((lua_State *)L, level, (lua_Debug *)deb));
}

int luaWrap_lua_getinfo(luaWrap_lua_State *L, const char *what, luaWrap_lua_Debug *deb)
{
    return (lua_getinfo((lua_State *)L, what, (lua_Debug *)deb));
}

void luaWrap_lua_setglobal(luaWrap_lua_State *L, const char *name)
{
    lua_setglobal((lua_State *)L, name);
}

void luaWrap_lua_getfield(luaWrap_lua_State *L, int idx, const char *name)
{
    lua_getfield((lua_State *)L, idx, name);
}

void luaWrap_lua_setfield(luaWrap_lua_State *L, int idx, const char *name)
{
    lua_setfield((lua_State *)L, idx, name);
}

void luaWrap_lua_geti(luaWrap_lua_State *L, int idx, lua_Integer i)
{
    lua_geti((lua_State *)L, idx, i);
}

void luaWrap_lua_seti(luaWrap_lua_State *L, int idx, lua_Integer i)
{
    lua_seti((lua_State *)L, idx, i);
}

void luaWrap_lua_yield(luaWrap_lua_State *L, int nresults)
{
    lua_yield((lua_State *)L, nresults);
}

int luaWrap_luaL_dostring(luaWrap_lua_State *L, const char *str)
{
    return (luaL_dostring((lua_State *)L, str));
}

void luaWrap_lua_getglobal(luaWrap_lua_State *L, const char *str)
{
    lua_getglobal((lua_State *)L, str);
}

void luaWrap_lua_pop(luaWrap_lua_State *L, int n)
{
    lua_pop((lua_State *)L, n);
}

int luaWrap_lua_gettop(luaWrap_lua_State *L)
{
    return (lua_gettop((lua_State *)L));
}

void luaWrap_lua_settop(luaWrap_lua_State *L, int idx)
{
    lua_settop((lua_State *)L, idx);
}

size_t luaWrap_lua_rawlen(luaWrap_lua_State *L, int idx)
{
    return (lua_rawlen((lua_State *)L, idx));
}

void luaWrap_lua_rawgeti(luaWrap_lua_State *L, int idx, int n)
{
    lua_rawgeti((lua_State *)L, idx, n);
}

void luaWrap_lua_rawseti(luaWrap_lua_State *L, int idx, int n)
{
    lua_rawseti((lua_State *)L, idx, n);
}

void luaWrap_lua_newtable(luaWrap_lua_State *L)
{
    lua_newtable((lua_State *)L);
}

int luaWrap_luaL_loadbuffer(luaWrap_lua_State *L, const char *buff, size_t sz, const char *name)
{
    return (luaL_loadbuffer((lua_State *)L, buff, sz, name));
}

void luaWrap_lua_remove(luaWrap_lua_State *L, int idx)
{
    lua_remove((lua_State *)L, idx);
}

void luaWrap_lua_insert(luaWrap_lua_State *L, int idx)
{
    lua_insert((lua_State *)L, idx);
}

int luaWrap_lua_pcall(luaWrap_lua_State *L, int nargs, int nresult, int errfunc)
{
    return (lua_pcall((lua_State *)L, nargs, nresult, errfunc));
}

int luaWrap_lua_isuserdata(luaWrap_lua_State *L, int idx)
{
    return (lua_isuserdata((lua_State *)L, idx));
}

int luaWrap_lua_upvalueindex(int i)
{
    return (lua_upvalueindex(i));
}

int luaWrap_getCurrentCodeLine(luaWrap_lua_State *L)
{
    int r = -1;
    lua_Debug ar;
    if (lua_getstack((lua_State *)L, 1, &ar) == 1)
    {
        if (lua_getinfo((lua_State *)L, "l", &ar) == 1)
            r = ar.currentline;
    }
    return (r);
}

std::string luaWrap_getCurrentCodeSource(luaWrap_lua_State *L)
{
    std::string r = "?";
    lua_Debug ar;
    if (lua_getstack((lua_State *)L, 1, &ar) == 1)
    {
        if (lua_getinfo((lua_State *)L, "S", &ar) == 1)
            r = ar.source;
    }
    return (r);
}

void luaWrap_lua_settable(luaWrap_lua_State *L, int idx)
{
    lua_settable((lua_State *)L, idx);
}

int luaWrap_lua_next(luaWrap_lua_State *L, int idx)
{
    return (lua_next((lua_State *)L, idx));
}

int luaWrap_lua_stype(luaWrap_lua_State *L, int idx)
{
    int t = lua_type((lua_State *)L, idx);
    if (t == LUA_TNIL)
        return (sim_stackitem_null);
    if (t == LUA_TNUMBER)
    {
        int intT = lua_isinteger((lua_State *)L, idx);
        if (intT == 0)
            return (sim_stackitem_double);
        return (sim_stackitem_integer);
    }
    if (t == LUA_TBOOLEAN)
        return (sim_stackitem_bool);
    if (t == LUA_TSTRING)
        return (sim_stackitem_string);
    if (t == LUA_TTABLE)
        return (sim_stackitem_table);
    if (t == LUA_TFUNCTION)
        return (sim_stackitem_func);
    if (t == LUA_TUSERDATA)
        return (sim_stackitem_userdat);
    if (t == LUA_TTHREAD)
        return (sim_stackitem_thread);
    if (t == LUA_TLIGHTUSERDATA)
        return (sim_stackitem_lightuserdat);
    return (sim_stackitem_null);
}

int luaWrap_lua_error(luaWrap_lua_State *L)
{
    return (lua_error((lua_State *)L));
}

bool luaWrap_lua_isgeneraltable(luaWrap_lua_State *L, int idx)
{
    return (lua_istable((lua_State *)L, idx));
}

bool luaWrap_lua_isnonbuffertable(luaWrap_lua_State *L, int idx)
{
    bool retVal = false;
    if (lua_istable((lua_State *)L, idx))
        retVal = !luaWrap_lua_isbuffer(L, idx);
    return retVal;
}

bool luaWrap_lua_isbuffer(luaWrap_lua_State *L, int idx)
{
    bool retVal = false;
    if (lua_istable((lua_State *)L, idx))
    {
        lua_pushstring((lua_State *)L, "__buff__");
        if (idx < 0)
            idx--;
        lua_rawget((lua_State *)L, idx);
        retVal = (lua_isstring((lua_State *)L, -1) != 0);
        lua_pop((lua_State *)L, 1);
    }

/*  // following slower:
    // check for buffer via public isbuffer() interface Lua function
    int top = lua_gettop((lua_State *)L);
    lua_getglobal((lua_State *)L, "isbuffer");
    if (!lua_isfunction((lua_State *)L, -1))
    {
        lua_settop((lua_State *)L, top);
        return false;
    }
    lua_pushvalue((lua_State *)L, idx < 0 ? idx - 1 : idx);
    if (lua_pcall((lua_State *)L, 1, 1, 0) != LUA_OK)
    {
        lua_settop((lua_State *)L, top);
        return false;
    }
    retVal = lua_toboolean((lua_State *)L, -1);
    lua_pop((lua_State *)L, 1);
    lua_settop((lua_State *)L, top);
*/

    return retVal;
}

const char *luaWrap_lua_tobuffer(luaWrap_lua_State *L, int idx, size_t *len)
{
    if (idx < 0)
        idx = lua_gettop((lua_State *)L) + idx + 1;
    if (lua_isstring((lua_State *)L, idx))
        return (lua_tolstring((lua_State *)L, idx, len));
    if (luaWrap_lua_isbuffer(L, idx))
        return luaL_tolstring((lua_State *)L, idx, len); // buffer has a __tostring metamethod

    /* old:
    if (lua_istable((lua_State *)L, idx))
    {
        if (lua_getmetatable((lua_State *)L, idx))
        { // cannot lua_getfield here, since that triggers the __index metamethod
            lua_pushstring((lua_State *)L, "__buff__");
            lua_rawget((lua_State *)L, idx);
            if (lua_isstring((lua_State *)L, -1) != 0)
            { // we have a buffer, we replace it with its equivalent string
                lua_replace((lua_State *)L, idx);
                const char* buffer = lua_tolstring((lua_State *)L, idx, len);
                lua_pop((lua_State *)L, 1); // pop the metatable (lua_replace poped the string)
                return buffer;
            }
            lua_pop((lua_State *)L, 2); // pop the field and the metatable
        }
    }
    */

    len[0] = 0;
    return nullptr;
}

void luaWrap_lua_pushbuffer(luaWrap_lua_State *L, const char *str, size_t l)
{
    if (App::userSettings->useBuffers)
    {
        lua_getglobal((lua_State *)L, "tobuffer");
        lua_pushlstring((lua_State *)L, str, l);
        lua_pcall((lua_State *)L, 1, 1, 0);
    }
    else
        luaWrap_lua_pushbinarystring(L, str, l); // old, no difference between strings and buffers
}

void luaWrap_lua_pushbinarystring(luaWrap_lua_State *L, const char *str, size_t l)
{
    lua_pushlstring((lua_State *)L, str, l);
}
