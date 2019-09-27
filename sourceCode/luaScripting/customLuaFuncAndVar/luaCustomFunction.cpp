
#include "vrepMainHeader.h"
#include "luaCustomFunction.h"

CLuaCustomFunction::CLuaCustomFunction(const char* theFullFunctionName,const char* theCallTips,std::vector<int>& theInputArgTypes,void(*callBack)(struct SLuaCallBack* p))
{ // the old way, called through simRegisterCustomLuaFunction
    useStackToExchangeData=false;
    _functionIsDefinedInScript=false;
    if (theFullFunctionName!=nullptr)
    {
        functionName=_getFunctionNameFromFull(theFullFunctionName);
        pluginName=_getPluginNameFromFull(theFullFunctionName);
    }
    if (theCallTips!=nullptr)
        callTips=theCallTips;
    for (size_t i=0;i<theInputArgTypes.size();i++)
        inputArgTypes.push_back(theInputArgTypes[i]);
    callBackFunction_new=nullptr;
    callBackFunction_old=callBack;
}

CLuaCustomFunction::CLuaCustomFunction(const char* theFullFunctionName,const char* theCallTips,void(*callBack)(struct SScriptCallBack* cb))
{ // the new way, called through simRegisterScriptCallbackFunction
    useStackToExchangeData=true;
    _functionIsDefinedInScript=false;
    if (theFullFunctionName!=nullptr)
    {
        functionName=_getFunctionNameFromFull(theFullFunctionName);
        pluginName=_getPluginNameFromFull(theFullFunctionName);
    }
    if (theCallTips!=nullptr)
    {
        std::string ct(theCallTips);
        if (ct.find("####")==0)
        {
            ct.erase(0,4);
            _functionIsDefinedInScript=true;
        }
        callTips=ct;
    }
    callBackFunction_old=nullptr;
    callBackFunction_new=callBack;
}

CLuaCustomFunction::~CLuaCustomFunction()
{
}

bool CLuaCustomFunction::hasCallback() const
{
    return( (callBackFunction_old!=nullptr)||(callBackFunction_new!=nullptr) );
}

bool CLuaCustomFunction::hasCalltipsAndSyntaxHighlighing() const
{
    return(callTips.length()>0);
}

bool CLuaCustomFunction::hasAutocompletion() const
{
    return( (callTips.length()>0)&&(hasCallback()||_functionIsDefinedInScript) );
}

bool CLuaCustomFunction::isDeprecated() const
{
    return( (callTips.length()==0)||( (!hasCallback())&&(!_functionIsDefinedInScript) ) );
}

void CLuaCustomFunction::registerCustomLuaFunction(luaWrap_lua_State* L,luaWrap_lua_CFunction func) const
{
    if (hasCallback())
    {
        size_t p=functionName.find(".");
        if (p!=std::string::npos)
        { // this is the new notation, e.g. simUI.create()
            std::string prefix(functionName.begin(),functionName.begin()+p);

            luaWrap_lua_pushnumber(L,functionID+1);
            luaWrap_lua_pushcclosure(L,func,1);
            luaWrap_lua_setfield(L,luaWrapGet_LUA_GLOBALSINDEX(),"__iuafkjsdgoi158zLK");

            std::string tmp("if not ");
            tmp+=prefix;
            tmp+=" then ";
            tmp+=prefix;
            tmp+="={} end ";
            tmp+=functionName;
            tmp+="=__iuafkjsdgoi158zLK __iuafkjsdgoi158zLK=nil";
            luaWrap_luaL_dostring(L,tmp.c_str());
        }
        else
        {
            luaWrap_lua_pushnumber(L,functionID+1);
            luaWrap_lua_pushcclosure(L,func,1);
            luaWrap_lua_setfield(L,luaWrapGet_LUA_GLOBALSINDEX(),functionName.c_str());
        }
    }
}

bool CLuaCustomFunction::getUsesStackToExchangeData() const
{
    return(useStackToExchangeData);
}

bool CLuaCustomFunction::isFunctionNameSame(const char* fullName) const
{
    std::string funcName(_getFunctionNameFromFull(fullName));
    return(functionName.compare(funcName)==0);
}

bool CLuaCustomFunction::isPluginNameSame(const char* plugName) const
{
    return(pluginName.compare(plugName)==0);
}

std::string CLuaCustomFunction::getFunctionName() const
{
    return(functionName);
}

std::string CLuaCustomFunction::getPluginName() const
{
    return(pluginName);
}

std::string CLuaCustomFunction::getCallTips() const
{
    return(callTips);
}

int CLuaCustomFunction::getFunctionID() const
{
    return(functionID);
}

void CLuaCustomFunction::setFunctionID(int newID)
{
    functionID=newID;
}

std::string CLuaCustomFunction::_getFunctionNameFromFull(const char* fullName)
{
    std::string f(fullName);
    size_t p=f.find('@');
    if (p==std::string::npos)
        return(f);
    std::string subF(f.begin(),f.begin()+p);
    return(subF);
}

std::string CLuaCustomFunction::_getPluginNameFromFull(const char* fullName)
{
    std::string f(fullName);
    size_t p=f.find('@');
    if (p==std::string::npos)
        return("");
    std::string subF(f.begin()+p+1,f.end());
    return(subF);
}
