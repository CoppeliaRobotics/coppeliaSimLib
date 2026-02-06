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

luaWrap_lua_State* luaWrap_luaL_newstate()
{
    return ((luaWrap_lua_State*)luaL_newstate());
}

void luaWrap_lua_close(luaWrap_lua_State* L)
{
    lua_close((lua_State*)L);
}

void luaWrap_luaL_openlibs(luaWrap_lua_State* L)
{
    luaL_openlibs((lua_State*)L);
}

void luaWrap_lua_sethook(luaWrap_lua_State* L, luaWrap_lua_Hook func, int mask, int cnt)
{
    lua_sethook((lua_State*)L, (lua_Hook)func, mask, cnt);
}

void luaWrap_lua_register(luaWrap_lua_State* L, const char* name, luaWrap_lua_CFunction func)
{
    lua_register((lua_State*)L, name, (lua_CFunction)func);
}

void luaWrap_lua_pushnumber(luaWrap_lua_State* L, luaWrap_lua_Number n)
{
    lua_pushnumber((lua_State*)L, n);
}

void luaWrap_lua_pushnil(luaWrap_lua_State* L)
{
    lua_pushnil((lua_State*)L);
}

void luaWrap_lua_pushboolean(luaWrap_lua_State* L, int b)
{
    lua_pushboolean((lua_State*)L, b);
}

void luaWrap_lua_pushinteger(luaWrap_lua_State* L, luaWrap_lua_Integer n)
{
    lua_pushinteger((lua_State*)L, n);
}

void luaWrap_lua_pushtext(luaWrap_lua_State* L, const char* str)
{
    lua_pushstring((lua_State*)L, str);
}

void luaWrap_lua_pushcclosure(luaWrap_lua_State* L, luaWrap_lua_CFunction func, int n)
{
    lua_pushcclosure((lua_State*)L, (lua_CFunction)func, n);
}

void luaWrap_lua_pushvalue(luaWrap_lua_State* L, int idx)
{
    lua_pushvalue((lua_State*)L, idx);
}

void luaWrap_lua_pushcfunction(luaWrap_lua_State* L, luaWrap_lua_CFunction func)
{
    lua_pushcfunction((lua_State*)L, (lua_CFunction)func);
}

void luaWrap_lua_createtable(luaWrap_lua_State* L, int narr, int nrec)
{
    lua_createtable((lua_State*)L, narr, nrec);
}

luaWrap_lua_Integer luaWrap_lua_tointeger(luaWrap_lua_State* L, int idx)
{
    if (lua_isinteger((lua_State*)L, idx))
        return (lua_tointeger((lua_State*)L, idx));
    return ((luaWrap_lua_Integer)lua_tonumber((lua_State*)L, idx));
}

luaWrap_lua_Number luaWrap_lua_tonumber(luaWrap_lua_State* L, int idx)
{
    return ((luaWrap_lua_Number)lua_tonumber((lua_State*)L, idx));
}

int luaWrap_lua_toboolean(luaWrap_lua_State* L, int idx)
{
    return (lua_toboolean((lua_State*)L, idx));
}

const void* luaWrap_lua_topointer(luaWrap_lua_State* L, int idx)
{
    return (lua_topointer((lua_State*)L, idx));
}

const char* luaWrap_lua_tostring(luaWrap_lua_State* L, int idx)
{ // buffer-tolerant
    size_t len;
    return luaWrap_lua_tobuffer(L, idx, &len);
}

int luaWrap_lua_isnumber(luaWrap_lua_State* L, int idx)
{ // see also luaWrap_lua_numbertype
    return (lua_isnumber((lua_State*)L, idx));
}

int luaWrap_lua_isinteger(luaWrap_lua_State* L, int idx)
{
    return (lua_isinteger((lua_State*)L, idx));
}

int luaWrap_lua_isstring(luaWrap_lua_State* L, int idx)
{ // buffer-tolerant. See also luaWrap_lua_stringtype
    int retVal = 0;
    if (luaWrap_lua_isbuffer(L, idx))
        retVal = 1;
    else
        retVal = lua_isstring((lua_State*)L, idx);
    return retVal;
}

bool luaWrap_lua_isboolean(luaWrap_lua_State* L, int idx)
{
    return (lua_isboolean((lua_State*)L, idx));
}

bool luaWrap_lua_isnil(luaWrap_lua_State* L, int idx)
{
    return (lua_isnil((lua_State*)L, idx));
}

int luaWrap_lua_getmetatable(luaWrap_lua_State* L, int idx)
{
    return (lua_getmetatable((lua_State*)L, idx));
}

int luaWrap_lua_rawequal(luaWrap_lua_State* L, int idx1, int idx2)
{
    return (lua_rawequal((lua_State*)L, idx1, idx2));
}

bool luaWrap_lua_isfunction(luaWrap_lua_State* L, int idx)
{
    return (lua_isfunction((lua_State*)L, idx));
}

int luaWrap_lua_getstack(luaWrap_lua_State* L, int level, luaWrap_lua_Debug* deb)
{
    return (lua_getstack((lua_State*)L, level, (lua_Debug*)deb));
}

int luaWrap_lua_getinfo(luaWrap_lua_State* L, const char* what, luaWrap_lua_Debug* deb)
{
    return (lua_getinfo((lua_State*)L, what, (lua_Debug*)deb));
}

void luaWrap_lua_setglobal(luaWrap_lua_State* L, const char* name)
{
    lua_setglobal((lua_State*)L, name);
}

void luaWrap_lua_getfield(luaWrap_lua_State* L, int idx, const char* name)
{
    lua_getfield((lua_State*)L, idx, name);
}

void luaWrap_lua_setfield(luaWrap_lua_State* L, int idx, const char* name)
{
    lua_setfield((lua_State*)L, idx, name);
}

void luaWrap_lua_geti(luaWrap_lua_State* L, int idx, lua_Integer i)
{
    lua_geti((lua_State*)L, idx, i);
}

void luaWrap_lua_seti(luaWrap_lua_State* L, int idx, lua_Integer i)
{
    lua_seti((lua_State*)L, idx, i);
}

void luaWrap_lua_yield(luaWrap_lua_State* L, int nresults)
{
    lua_yield((lua_State*)L, nresults);
}

int luaWrap_luaL_dostring(luaWrap_lua_State* L, const char* str)
{
    return (luaL_dostring((lua_State*)L, str));
}

void luaWrap_lua_getglobal(luaWrap_lua_State* L, const char* str)
{
    lua_getglobal((lua_State*)L, str);
}

void luaWrap_lua_pop(luaWrap_lua_State* L, int n)
{
    lua_pop((lua_State*)L, n);
}

int luaWrap_lua_gettop(luaWrap_lua_State* L)
{
    return (lua_gettop((lua_State*)L));
}

void luaWrap_lua_settop(luaWrap_lua_State* L, int idx)
{
    lua_settop((lua_State*)L, idx);
}

int luaWrap_lua_absindex(luaWrap_lua_State* L, int idx)
{
    return lua_absindex((lua_State*)L, idx);
}

size_t luaWrap_lua_rawlen(luaWrap_lua_State* L, int idx)
{
    return (lua_rawlen((lua_State*)L, idx));
}

void luaWrap_lua_rawgeti(luaWrap_lua_State* L, int idx, int n)
{
    lua_rawgeti((lua_State*)L, idx, n);
}

void luaWrap_lua_rawseti(luaWrap_lua_State* L, int idx, int n)
{
    lua_rawseti((lua_State*)L, idx, n);
}

void luaWrap_lua_newtable(luaWrap_lua_State* L)
{
    lua_newtable((lua_State*)L);
}

int luaWrap_luaL_loadbuffer(luaWrap_lua_State* L, const char* buff, size_t sz, const char* name)
{
    return (luaL_loadbuffer((lua_State*)L, buff, sz, name));
}

int luaWrap_luaL_callmeta(luaWrap_lua_State *L, int obj, const char *func)
{
    return luaL_callmeta((lua_State*)L, obj, func);
}

void luaWrap_lua_remove(luaWrap_lua_State* L, int idx)
{
    lua_remove((lua_State*)L, idx);
}

void luaWrap_lua_insert(luaWrap_lua_State* L, int idx)
{
    lua_insert((lua_State*)L, idx);
}

int luaWrap_luaL_ref(luaWrap_lua_State* L, int t)
{
    return luaL_ref((lua_State*)L, t);
}

void luaWrap_lua_call(luaWrap_lua_State* L, int nargs, int nresult)
{
    lua_call((lua_State*)L, nargs, nresult);
}

int luaWrap_lua_pcall(luaWrap_lua_State* L, int nargs, int nresult, int errfunc)
{
    return (lua_pcall((lua_State*)L, nargs, nresult, errfunc));
}

int luaWrap_lua_isuserdata(luaWrap_lua_State* L, int idx)
{
    return (lua_isuserdata((lua_State*)L, idx));
}

int luaWrap_lua_upvalueindex(int i)
{
    return (lua_upvalueindex(i));
}

int luaWrap_getCurrentCodeLine(luaWrap_lua_State* L)
{
    int r = -1;
    lua_Debug ar;
    if (lua_getstack((lua_State*)L, 1, &ar) == 1)
    {
        if (lua_getinfo((lua_State*)L, "l", &ar) == 1)
            r = ar.currentline;
    }
    return (r);
}

std::string luaWrap_getCurrentCodeSource(luaWrap_lua_State* L)
{
    std::string r = "?";
    lua_Debug ar;
    if (lua_getstack((lua_State*)L, 1, &ar) == 1)
    {
        if (lua_getinfo((lua_State*)L, "S", &ar) == 1)
            r = ar.source;
    }
    return (r);
}

void luaWrap_lua_settable(luaWrap_lua_State* L, int idx)
{
    lua_settable((lua_State*)L, idx);
}

int luaWrap_lua_next(luaWrap_lua_State* L, int idx)
{
    return (lua_next((lua_State*)L, idx));
}

bool luaWrap_lua_stringtype(luaWrap_lua_State* L, int idx)
{
    return (lua_type((lua_State*)L, idx) == LUA_TSTRING);
}

bool luaWrap_lua_numbertype(luaWrap_lua_State* L, int idx)
{
    return (lua_type((lua_State*)L, idx) == LUA_TNUMBER);
}

int luaWrap_lua_stype(luaWrap_lua_State* L, int idx)
{
    int retVal = sim_stackitem_null;
    int t = lua_type((lua_State*)L, idx);
    if (t == LUA_TNIL)
        retVal = sim_stackitem_null;
    else if (t == LUA_TNUMBER)
    {
        if (lua_isinteger((lua_State*)L, idx) == 0)
            retVal = sim_stackitem_double;
        else
            retVal = sim_stackitem_integer;
    }
    else if (t == LUA_TBOOLEAN)
        retVal = sim_stackitem_bool;
    else if (t == LUA_TSTRING)
        retVal = sim_stackitem_string;
    else if (t == LUA_TTABLE)
        retVal = sim_stackitem_table;
    else if (t == LUA_TFUNCTION)
        retVal = sim_stackitem_func;
    else if (t == LUA_TUSERDATA)
        retVal = sim_stackitem_userdat;
    else if (t == LUA_TTHREAD)
        retVal = sim_stackitem_thread;
    else if (t == LUA_TLIGHTUSERDATA)
        retVal = sim_stackitem_lightuserdat;
    return retVal;
}

int luaWrap_lua_error(luaWrap_lua_State* L)
{
    return (lua_error((lua_State*)L));
}

bool luaWrap_lua_isgeneraltable(luaWrap_lua_State* L, int idx)
{
    return (lua_istable((lua_State*)L, idx));
}

bool luaWrap_lua_isnonbuffertable(luaWrap_lua_State* L, int idx)
{
    bool retVal = false;
    if (lua_istable((lua_State*)L, idx))
        retVal = !luaWrap_lua_isbuffer(L, idx);
    return retVal;
}

bool luaWrap_lua_ismetatable(luaWrap_lua_State* L, int idx)
{
    bool retVal = false;
    if (lua_istable((lua_State*)L, idx))
    {
        retVal = (lua_getmetatable((lua_State*)L, idx) != 0);
        if (retVal)
            lua_pop((lua_State*)L, 1);
    }
    return retVal;
}

bool luaWrap_lua_isbuffer(luaWrap_lua_State* L, int idx)
{
    bool retVal = false;

    if (lua_getmetatable((lua_State*)L, idx)) // Get the metatable of the object at 'idx'
    {
        lua_getglobal((lua_State*)L, "__buffmetatable__"); // Get the global metatable
        retVal = (lua_rawequal((lua_State*)L, -1, -2));    // Compare the two metatables
        lua_pop((lua_State*)L, 2);                         // Remove both metatables from the stack
    }

    /* old, less reliable way
    if (lua_istable((lua_State *)L, idx))
    {
        lua_pushstring((lua_State *)L, "__buff__");
        if (idx < 0)
            idx--;
        lua_rawget((lua_State *)L, idx);
        retVal = (lua_isstring((lua_State *)L, -1) != 0);
        lua_pop((lua_State *)L, 1);
    }
    */

    return retVal;
}

bool luaWrap_lua_ishandle(luaWrap_lua_State* L, int idx, int* handleVal /*= nullptr*/)
{
    bool retVal = false;
    idx = lua_absindex((lua_State*)L, idx);
    if (lua_isinteger((lua_State*)L, idx))
    {
        if (handleVal != nullptr)
            handleVal[0] = lua_tointeger((lua_State*)L, idx);
        retVal = true;
    }
    else if (lua_getmetatable((lua_State*)L, idx)) // Check if there is a metatable
    {
        lua_pop((lua_State*)L, 1); // Remove the metatable
        lua_getfield((lua_State*)L, idx, "handle");
        if (lua_isinteger((lua_State*)L, -1))
        {
            if (handleVal != nullptr)
                handleVal[0] = lua_tointeger((lua_State*)L, -1);
            retVal = true;
        }
        lua_pop((lua_State*)L, 1); // Remove the handle value
    }
    return retVal;
}

bool luaWrap_lua_ismatrix(luaWrap_lua_State* L, int idx, size_t* rows /*= nullptr*/, size_t* cols /*= nullptr*/, std::vector<double>* matrixData /*= nullptr*/)
{
    bool retVal = false;
    int abs_idx = lua_absindex((lua_State*)L, idx);
    if (luaL_callmeta((lua_State*)L, abs_idx, "__ismatrix") == 1)
    {
        retVal = lua_toboolean((lua_State*)L, -1);
        lua_pop((lua_State*)L,1);
        if (retVal)
        {
            if (rows != nullptr)
            {
                rows[0] = 0;
                if (luaL_callmeta((lua_State*)L, abs_idx, "rows") == 1)
                {
                    rows[0] = (size_t)lua_tointeger((lua_State*)L, -1);
                    lua_pop((lua_State*)L,1);
                }
            }
            if (cols != nullptr)
            {
                cols[0] = 0;
                if (luaL_callmeta((lua_State*)L, abs_idx, "cols") == 1)
                {
                    cols[0] = (size_t)lua_tointeger((lua_State*)L, -1);
                    lua_pop((lua_State*)L,1);
                }
            }
            if (matrixData != nullptr)
            {
                if (luaL_callmeta((lua_State*)L, abs_idx, "data") == 1)
                {
                    size_t n = lua_rawlen((lua_State*)L, -1);
                    matrixData->clear();
                    for (size_t i = 1; i <= n; i++)
                    {
                        lua_rawgeti((lua_State*)L, -1, (lua_Integer)i);
                        matrixData->push_back(lua_tonumber((lua_State*)L, -1));
                        lua_pop((lua_State*)L, 1);
                    }
                }
                lua_pop((lua_State*)L, 1);
            }
        }
    }
    return retVal;
}

bool luaWrap_lua_isvector3(luaWrap_lua_State* L, int idx, std::vector<double>* vectorData /*= nullptr*/, bool strict /*= false*/)
{
    bool retVal = false;
    int abs_idx = lua_absindex((lua_State*)L, idx);
    size_t rows, cols;
    std::vector<double> dat;
    if (luaWrap_lua_ismatrix(L, abs_idx, &rows, &cols, &dat))
    {
        if ((rows == 3) && (cols == 1))
        {
            if (vectorData != nullptr)
                dat.swap(vectorData[0]);
            retVal = true;
        }
    }
    if ((!retVal) && (!strict))
    {
        if (luaWrap_lua_isnonbuffertable(L, abs_idx))
        {
            int s = int(lua_rawlen((lua_State*)L, abs_idx));
            if (s == 3)
            {
                dat.resize(s);
                if (getDoublesFromTable(L, abs_idx, s, dat.data()))
                {
                    if (vectorData != nullptr)
                        dat.swap(vectorData[0]);
                    retVal = true;
                }
            }
        }
    }
    return retVal;
}

bool luaWrap_lua_isquaternion(luaWrap_lua_State* L, int idx, std::vector<double>* quaternionData /*= nullptr*/, bool strict /*= false*/)
{
    bool retVal = false;
    int abs_idx = lua_absindex((lua_State*)L, idx);
    if (luaL_callmeta((lua_State*)L, abs_idx, "__isquaternion") == 1)
    {
        retVal = lua_toboolean((lua_State*)L, -1);
        lua_pop((lua_State*)L,1);
        if (retVal)
        {
            if (quaternionData != nullptr)
            {
                if (luaL_callmeta((lua_State*)L, abs_idx, "data") == 1)
                {
                    size_t n = lua_rawlen((lua_State*)L, -1);
                    quaternionData->clear();
                    for (size_t i = 1; i <= n; i++)
                    {
                        lua_rawgeti((lua_State*)L, -1, (lua_Integer)i);
                        quaternionData->push_back(lua_tonumber((lua_State*)L, -1));
                        lua_pop((lua_State*)L, 1);
                    }
                }
                lua_pop((lua_State*)L, 1);
            }
        }
    }
    if ((!retVal) && (!strict))
    {
        if (luaWrap_lua_isnonbuffertable(L, abs_idx))
        {
            int s = int(lua_rawlen((lua_State*)L, abs_idx));
            if (s == 4)
            {
                std::vector<double> dat;
                dat.resize(s);
                if (getDoublesFromTable(L, abs_idx, s, dat.data()))
                {
                    if (quaternionData != nullptr)
                        dat.swap(quaternionData[0]);
                    retVal = true;
                }
            }
        }
    }
    return retVal;
}

bool luaWrap_lua_ispose(luaWrap_lua_State* L, int idx, std::vector<double>* poseData /*= nullptr*/, bool strict /*= false*/)
{
    bool retVal = false;
    int abs_idx = lua_absindex((lua_State*)L, idx);
    if (luaL_callmeta((lua_State*)L, abs_idx, "__ispose") == 1)
    {
        retVal = lua_toboolean((lua_State*)L, -1);
        lua_pop((lua_State*)L,1);
        if (retVal)
        {
            if (poseData != nullptr)
            {
                if (luaL_callmeta((lua_State*)L, abs_idx, "data") == 1)
                {
                    size_t n = lua_rawlen((lua_State*)L, -1);
                    poseData->clear();
                    for (size_t i = 1; i <= n; i++)
                    {
                        lua_rawgeti((lua_State*)L, -1, (lua_Integer)i);
                        poseData->push_back(lua_tonumber((lua_State*)L, -1));
                        lua_pop((lua_State*)L, 1);
                    }
                }
                lua_pop((lua_State*)L, 1);
            }
        }
    }
    if ((!retVal) && (!strict))
    {
        if (luaWrap_lua_isnonbuffertable(L, abs_idx))
        {
            int s = int(lua_rawlen((lua_State*)L, abs_idx));
            if (s == 7)
            {
                std::vector<double> dat;
                dat.resize(s);
                if (getDoublesFromTable(L, abs_idx, s, dat.data()))
                {
                    if (poseData != nullptr)
                        dat.swap(poseData[0]);
                    retVal = true;
                }
            }
        }
    }
    return retVal;
}

bool luaWrap_lua_iscolor(luaWrap_lua_State* L, int idx, float colorData[3] /*= nullptr*/, bool strict /*= false*/)
{
    bool retVal = false;
    int abs_idx = lua_absindex((lua_State*)L, idx);
    if (luaL_callmeta((lua_State*)L, abs_idx, "__iscolor") == 1)
    {
        retVal = lua_toboolean((lua_State*)L, -1);
        lua_pop((lua_State*)L,1);
        if (retVal)
        {
            if (colorData != nullptr)
            {
                if (luaL_callmeta((lua_State*)L, abs_idx, "data") == 1)
                {
                    for (size_t i = 1; i <= 3; i++)
                    {
                        lua_rawgeti((lua_State*)L, -1, (lua_Integer)i);
                        colorData[i - 1] = float(lua_tonumber((lua_State*)L, -1));
                        lua_pop((lua_State*)L, 1);
                    }
                }
                lua_pop((lua_State*)L, 1);
            }
        }
    }
    if ((!retVal) && (!strict))
    {
        if (luaWrap_lua_isnonbuffertable(L, abs_idx))
        {
            int s = int(lua_rawlen((lua_State*)L, abs_idx));
            if (s == 3)
            {
                float c[3];
                if (getFloatsFromTable(L, abs_idx, s, c))
                {
                    if (colorData != nullptr)
                    {
                        for (size_t i = 0; i < 3; i++)
                            colorData[i] = c[i];
                    }
                    retVal = true;
                }
            }
        }
    }
    return retVal;
}

int luaWrap_lua_tohandle(luaWrap_lua_State* L, int idx)
{
    int retVal = -1;
    idx = lua_absindex((lua_State*)L, idx);
    luaWrap_lua_ishandle(L, idx, &retVal);
    return retVal;
}

const char* luaWrap_lua_tobuffer(luaWrap_lua_State* L, int idx, size_t* len)
{
    const char* retVal = nullptr;
    len[0] = 0;
    idx = lua_absindex((lua_State*)L, idx);

    if (lua_isstring((lua_State*)L, idx))
        retVal = lua_tolstring((lua_State*)L, idx, len);

    if (luaWrap_lua_isbuffer(L, idx))
    {
        retVal = luaL_tolstring((lua_State*)L, idx, len); // buffer has a __tostring metamethod
        lua_pop((lua_State*)L, 1);                        // the metamethod adds a value to the stack
    }

    /* old, less reliable way
    if (lua_istable((lua_State *)L, idx))
    {
        if (lua_getmetatable((lua_State *)L, idx))
        { // cannot lua_getfield here, since that triggers the __index metamethod
            lua_pushstring((lua_State *)L, "__buff__");
            lua_rawget((lua_State *)L, idx);
            if (lua_isstring((lua_State *)L, -1) != 0)
            { // we have a buffer, we replace it with its equivalent string
                lua_replace((lua_State *)L, idx);
                retVal = lua_tolstring((lua_State *)L, idx, len);
                lua_pop((lua_State *)L, 1); // pop the metatable (lua_replace poped the string)
            }
            else
                lua_pop((lua_State *)L, 2); // pop the field and the metatable
        }
    }
    */

    return retVal;
}

void luaWrap_lua_pushbuffer(luaWrap_lua_State* L, const char* str, size_t l)
{
    if (App::userSettings->useBuffers)
    {
        lua_getglobal((lua_State*)L, "tobuffer");
        lua_pushlstring((lua_State*)L, str, l);
        lua_pcall((lua_State*)L, 1, 1, 0);
    }
    else
        luaWrap_lua_pushbinarystring(L, str, l); // old, no difference between strings and buffers
}

bool luaWrap_lua_pushhandle(luaWrap_lua_State* L, int h)
{
    bool retVal = false;
    if ((h == sim_handle_scene) || (h == sim_handle_app) || (h >= 0))
    {
        lua_getglobal((lua_State*)L, "sim");
        if (lua_istable((lua_State*)L, -1))
        {
            lua_getfield((lua_State*)L, -1, "Object");
            lua_pushinteger((lua_State*)L, h);
            if (lua_pcall((lua_State*)L, 1, 1, 0) == LUA_OK)
            {
                lua_remove((lua_State*)L, -2);
                retVal = true;
            }
            else
                lua_pop((lua_State*)L, 2);
        }
        else
            lua_pop((lua_State*)L, 1);
    }
    if (!retVal)
        lua_pushnil((lua_State*)L);
    return retVal;
}

void luaWrap_lua_pushmatrix(luaWrap_lua_State* L, const double* matrix, size_t rows, size_t cols)
{
    lua_getglobal((lua_State*)L, "_Matrix__");
    if (lua_isnil((lua_State*)L, -1))
    {
        lua_pop((lua_State*)L, 1); // pop nil
        lua_getglobal((lua_State*)L, "require");
        lua_pushstring((lua_State*)L, "simEigen");
        if (lua_pcall((lua_State*)L, 1, 1, 0) == LUA_OK)
        {
            lua_getfield((lua_State*)L, -1, "Matrix");
            lua_pushvalue((lua_State*)L, -1);
            lua_setglobal((lua_State*)L, "_Matrix__");
            lua_pushinteger((lua_State*)L, rows);
            lua_pushinteger((lua_State*)L, cols);
            lua_createtable((lua_State*)L, int(rows * cols), 0);
            for (size_t i = 0; i < rows * cols; i++)
            {
                lua_pushnumber((lua_State*)L, matrix[i]);
                lua_rawseti((lua_State*)L, -2, i + 1);
            }
            lua_pcall((lua_State*)L, 3, 1, 0);
            lua_remove((lua_State*)L, -2); // simEigen module
        }
        else
        {
            lua_pop((lua_State*)L, 1);
            App::logMsg(sim_verbosity_errors, "failed to require simEigen in luaWrap_lua_pushmatrix.");
            lua_pushnil((lua_State*)L);
        }
    }
    else
    {
        lua_pushinteger((lua_State*)L, rows);
        lua_pushinteger((lua_State*)L, cols);
        lua_createtable((lua_State*)L, int(rows * cols), 0);
        for (size_t i = 0; i < rows * cols; i++)
        {
            lua_pushnumber((lua_State*)L, matrix[i]);
            lua_rawseti((lua_State*)L, -2, i + 1);
        }
        lua_pcall((lua_State*)L, 3, 1, 0);
    }
}

void luaWrap_lua_pushvector3(luaWrap_lua_State* L, const double* dat)
{
    luaWrap_lua_pushmatrix(L, dat, 3, 1);
}

void luaWrap_lua_pushquaternion(luaWrap_lua_State* L, const double* dat)
{
    lua_getglobal((lua_State*)L, "_Quaternion__");
    if (lua_isnil((lua_State*)L, -1))
    {
        lua_pop((lua_State*)L, 1); // pop nil
        lua_getglobal((lua_State*)L, "require");
        lua_pushstring((lua_State*)L, "simEigen");
        if (lua_pcall((lua_State*)L, 1, 1, 0) == LUA_OK)
        {
            lua_getfield((lua_State*)L, -1, "Quaternion");
            lua_pushvalue((lua_State*)L, -1);
            lua_setglobal((lua_State*)L, "_Quaternion__");
            lua_createtable((lua_State*)L, 4, 0);
            for (size_t i = 0; i < 4; i++)
            {
                lua_pushnumber((lua_State*)L, dat[i]);
                lua_rawseti((lua_State*)L, -2, i + 1);
            }
            lua_pcall((lua_State*)L, 1, 1, 0);
            lua_remove((lua_State*)L, -2); // simEigen module
        }
        else
        {
            lua_pop((lua_State*)L, 1);
            App::logMsg(sim_verbosity_errors, "failed to require simEigen in luaWrap_lua_pushquaternion.");
            lua_pushnil((lua_State*)L);
        }
    }
    else
    {
        lua_createtable((lua_State*)L, 4, 0);
        for (size_t i = 0; i < 4; i++)
        {
            lua_pushnumber((lua_State*)L, dat[i]);
            lua_rawseti((lua_State*)L, -2, i + 1);
        }
        lua_pcall((lua_State*)L, 1, 1, 0);
    }
}

void luaWrap_lua_pushpose(luaWrap_lua_State* L, const double* dat)
{
    lua_getglobal((lua_State*)L, "_Pose__");
    if (lua_isnil((lua_State*)L, -1))
    {
        lua_pop((lua_State*)L, 1); // pop nil
        lua_getglobal((lua_State*)L, "require");
        lua_pushstring((lua_State*)L, "simEigen");
        if (lua_pcall((lua_State*)L, 1, 1, 0) == LUA_OK)
        {
            lua_getfield((lua_State*)L, -1, "Pose");
            lua_pushvalue((lua_State*)L, -1);
            lua_setglobal((lua_State*)L, "_Pose__");
            lua_createtable((lua_State*)L, 7, 0);
            for (size_t i = 0; i < 7; i++)
            {
                lua_pushnumber((lua_State*)L, dat[i]);
                lua_rawseti((lua_State*)L, -2, i + 1);
            }
            lua_pcall((lua_State*)L, 1, 1, 0);
            lua_remove((lua_State*)L, -2); // simEigen module
        }
        else
        {
            lua_pop((lua_State*)L, 1);
            App::logMsg(sim_verbosity_errors, "failed to require simEigen in luaWrap_lua_pushpose.");
            lua_pushnil((lua_State*)L);
        }
    }
    else
    {
        lua_createtable((lua_State*)L, 7, 0);
        for (size_t i = 0; i < 7; i++)
        {
            lua_pushnumber((lua_State*)L, dat[i]);
            lua_rawseti((lua_State*)L, -2, i + 1);
        }
        lua_pcall((lua_State*)L, 1, 1, 0);
    }
}

void luaWrap_lua_pushcolor(luaWrap_lua_State* L, const float c[3])
{
    lua_getglobal((lua_State*)L, "_Color__");
    if (lua_isnil((lua_State*)L, -1))
    {
        lua_pop((lua_State*)L, 1); // pop nil
        lua_getglobal((lua_State*)L, "require");
        lua_pushstring((lua_State*)L, "Color");
        if (lua_pcall((lua_State*)L, 1, 1, 0) == LUA_OK)
        {
            lua_pushvalue((lua_State*)L, -1);
            lua_setglobal((lua_State*)L, "_Color__");
            lua_createtable((lua_State*)L, 3, 0);
            for (size_t i = 0; i < 3; i++)
            {
                lua_pushnumber((lua_State*)L, c[i]);
                lua_rawseti((lua_State*)L, -2, i + 1);
            }
            lua_pcall((lua_State*)L, 1, 1, 0);
        }
        else
        {
            lua_pop((lua_State*)L, 1);
            App::logMsg(sim_verbosity_errors, "failed to require Color in luaWrap_lua_pushcolor.");
            lua_pushnil((lua_State*)L);
        }
    }
    else
    {
        lua_createtable((lua_State*)L, 3, 0);
        for (size_t i = 0; i < 3; i++)
        {
            lua_pushnumber((lua_State*)L, c[i]);
            lua_rawseti((lua_State*)L, -2, i + 1);
        }
        lua_pcall((lua_State*)L, 1, 1, 0);
    }
}

void luaWrap_lua_pushbinarystring(luaWrap_lua_State* L, const char* str, size_t l)
{
    lua_pushlstring((lua_State*)L, str, l);
}

bool getFloatsFromTable(luaWrap_lua_State* L, int tablePos, size_t floatCount, float* arrayField)
{
    bool retVal = true;
    for (size_t i = 0; i < floatCount; i++)
    {
        lua_rawgeti((lua_State*)L, tablePos, int(i + 1));
        if (!lua_isnumber((lua_State*)L, -1))
            retVal = false;        // Not a number
        arrayField[i] = (float)lua_tonumber((lua_State*)L, -1);
        lua_pop((lua_State*)L, 1); // we pop one element from the stack;
    }
    return retVal;
}

bool getDoublesFromTable(luaWrap_lua_State* L, int tablePos, size_t doubleCount, double* arrayField)
{
    bool retVal = true;
    for (size_t i = 0; i < doubleCount; i++)
    {
        lua_rawgeti((lua_State*)L, tablePos, int(i + 1));
        if (!lua_isnumber((lua_State*)L, -1))
            retVal = false;        // Not a number
        arrayField[i] = lua_tonumber((lua_State*)L, -1);
        lua_pop((lua_State*)L, 1); // we pop one element from the stack;
    }
    return retVal;
}

bool getIntsFromTable(luaWrap_lua_State* L, int tablePos, size_t intCount, int* arrayField)
{
    bool retVal = true;
    for (size_t i = 0; i < intCount; i++)
    {
        lua_rawgeti((lua_State*)L, tablePos, int(i + 1));
        if (!lua_isinteger((lua_State*)L, -1))
            retVal = false;        // Not an integer
        arrayField[i] = int(lua_tointeger((lua_State*)L, -1));
        lua_pop((lua_State*)L, 1); // we pop one element from the stack;
    }
    return retVal;
}

bool getLongsFromTable(luaWrap_lua_State* L, int tablePos, size_t intCount, long long int* arrayField)
{
    bool retVal = true;
    for (size_t i = 0; i < intCount; i++)
    {
        lua_rawgeti((lua_State*)L, tablePos, int(i + 1));
        if (!lua_isinteger((lua_State*)L, -1))
            retVal = false;        // Not an integer
        arrayField[i] = lua_tointeger((lua_State*)L, -1);
        lua_pop((lua_State*)L, 1); // we pop one element from the stack;
    }
    return retVal;
}

bool getUIntsFromTable(luaWrap_lua_State* L, int tablePos, size_t intCount, unsigned int* arrayField)
{
    bool retVal = true;
    for (size_t i = 0; i < intCount; i++)
    {
        lua_rawgeti((lua_State*)L, tablePos, int(i + 1));
        if (!lua_isinteger((lua_State*)L, -1))
            retVal = false;        // Not an integer
        arrayField[i] = (unsigned int)lua_tointeger((lua_State*)L, -1);
        lua_pop((lua_State*)L, 1); // we pop one element from the stack;
    }
    return retVal;
}

bool getUCharsFromTable(luaWrap_lua_State* L, int tablePos, size_t intCount, unsigned char* arrayField)
{
    bool retVal = true;
    for (size_t i = 0; i < intCount; i++)
    {
        lua_rawgeti((lua_State*)L, tablePos, int(i + 1));
        if (!lua_isinteger((lua_State*)L, -1))
            retVal = false;        // Not an integer
        arrayField[i] = (unsigned char)lua_tointeger((lua_State*)L, -1);
        lua_pop((lua_State*)L, 1); // we pop one element from the stack;
    }
    return retVal;
}

bool getCharBoolsFromTable(luaWrap_lua_State* L, int tablePos, size_t boolCount, char* arrayField)
{
    bool retVal = true;
    for (size_t i = 0; i < boolCount; i++)
    {
        lua_rawgeti((lua_State*)L, tablePos, int(i + 1));
        if (!lua_isboolean((lua_State*)L, -1))
            retVal = false;        // Not a boolean!!
        arrayField[i] = (char)lua_toboolean((lua_State*)L, -1);
        lua_pop((lua_State*)L, 1); // we pop one element from the stack;
    }
    return retVal;
}

bool getStringsFromTable(luaWrap_lua_State* L, int tablePos, size_t stringCount, std::vector<std::string>& array)
{
    bool retVal = true;
    for (size_t i = 0; i < stringCount; i++)
    {
        lua_rawgeti((lua_State*)L, tablePos, int(i + 1));
        if (!luaWrap_lua_isstring((lua_State*)L, -1))
            retVal = false;        // Not a string!!
        array.push_back(lua_tostring((lua_State*)L, -1));
        lua_pop((lua_State*)L, 1); // we pop one element from the stack;
    }
    return retVal;
}

bool getHandlesFromTable(luaWrap_lua_State* L, int tablePos, size_t intCount, int* arrayField)
{
    bool retVal = true;
    for (size_t i = 0; i < intCount; i++)
    {
        lua_rawgeti((lua_State*)L, tablePos, int(i + 1));
        if (!luaWrap_lua_ishandle(L, -1))
            retVal = false;        // Not a handle!!
        arrayField[i] = luaWrap_lua_tohandle(L, -1);
        lua_pop((lua_State*)L, 1); // we pop one element from the stack;
    }
    return retVal;
}

void pushFloatTableOntoStack(luaWrap_lua_State* L, size_t floatCount, const float* arrayField)
{
    lua_createtable((lua_State*)L, int(floatCount), 0);
    int newTablePos = lua_gettop((lua_State*)L);
    for (size_t i = 0; i < floatCount; i++)
    {
        lua_pushnumber((lua_State*)L, arrayField[i]);
        lua_rawseti((lua_State*)L, newTablePos, int(i + 1));
    }
}

void pushDoubleTableOntoStack(luaWrap_lua_State* L, size_t doubleCount, const double* arrayField)
{
    lua_createtable((lua_State*)L, int(doubleCount), 0);
    int newTablePos = lua_gettop((lua_State*)L);
    for (size_t i = 0; i < doubleCount; i++)
    {
        lua_pushnumber((lua_State*)L, arrayField[i]);
        lua_rawseti((lua_State*)L, newTablePos, int(i + 1));
    }
}

void pushIntTableOntoStack(luaWrap_lua_State* L, size_t intCount, const int* arrayField)
{
    lua_createtable((lua_State*)L, int(intCount), 0);
    int newTablePos = lua_gettop((lua_State*)L);
    for (size_t i = 0; i < intCount; i++)
    {
        lua_pushinteger((lua_State*)L, arrayField[i]);
        lua_rawseti((lua_State*)L, newTablePos, int(i + 1));
    }
}

void pushLongTableOntoStack(luaWrap_lua_State* L, size_t intCount, const long long int* arrayField)
{
    lua_createtable((lua_State*)L, int(intCount), 0);
    int newTablePos = lua_gettop((lua_State*)L);
    for (size_t i = 0; i < intCount; i++)
    {
        lua_pushinteger((lua_State*)L, arrayField[i]);
        lua_rawseti((lua_State*)L, newTablePos, int(i + 1));
    }
}

void pushULongTableOntoStack(luaWrap_lua_State* L, size_t intCount, const unsigned long long int* arrayField)
{
    lua_createtable((lua_State*)L, int(intCount), 0);
    int newTablePos = lua_gettop((lua_State*)L);
    for (size_t i = 0; i < intCount; i++)
    {
        lua_pushinteger((lua_State*)L, arrayField[i]);
        lua_rawseti((lua_State*)L, newTablePos, int(i + 1));
    }
}

void pushUIntTableOntoStack(luaWrap_lua_State* L, size_t intCount, const unsigned int* arrayField)
{
    lua_createtable((lua_State*)L, int(intCount), 0);
    int newTablePos = lua_gettop((lua_State*)L);
    for (size_t i = 0; i < intCount; i++)
    {
        lua_pushinteger((lua_State*)L, arrayField[i]);
        lua_rawseti((lua_State*)L, newTablePos, int(i + 1));
    }
}

void pushUCharTableOntoStack(luaWrap_lua_State* L, size_t intCount, const unsigned char* arrayField)
{
    lua_createtable((lua_State*)L, int(intCount), 0);
    int newTablePos = lua_gettop((lua_State*)L);
    for (size_t i = 0; i < intCount; i++)
    {
        lua_pushinteger((lua_State*)L, arrayField[i]);
        lua_rawseti((lua_State*)L, newTablePos, int(i + 1));
    }
}

void pushStringTableOntoStack(luaWrap_lua_State* L, const std::vector<std::string>& stringTable)
{
    lua_createtable((lua_State*)L, int(stringTable.size()), 0);
    int newTablePos = lua_gettop((lua_State*)L);
    for (size_t i = 0; i < stringTable.size(); i++)
    {
        luaWrap_lua_pushtext((lua_State*)L, stringTable[i].c_str());
        lua_rawseti((lua_State*)L, newTablePos, (int)i + 1);
    }
}

void pushBufferTableOntoStack(luaWrap_lua_State* L, const std::vector<std::string>& stringTable)
{
    lua_createtable((lua_State*)L, int(stringTable.size()), 0);
    int newTablePos = lua_gettop((lua_State*)L);
    for (size_t i = 0; i < stringTable.size(); i++)
    {
        luaWrap_lua_pushbuffer((lua_State*)L, stringTable[i].c_str(), stringTable[i].size());
        lua_rawseti((lua_State*)L, newTablePos, (int)i + 1);
    }
}

bool pushHandleTableOntoStack(luaWrap_lua_State* L, size_t intCount, const int* arrayField)
{
    bool retVal = true;
    lua_createtable((lua_State*)L, int(intCount), 0);
    int newTablePos = lua_gettop((lua_State*)L);
    for (size_t i = 0; i < intCount; i++)
    {
        if (!luaWrap_lua_pushhandle((lua_State*)L, arrayField[i]))
            retVal = false;
        lua_rawseti((lua_State*)L, newTablePos, int(i + 1));
    }
    return retVal;
}

