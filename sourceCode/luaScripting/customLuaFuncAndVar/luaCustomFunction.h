
#pragma once

#include "vrepMainHeader.h"
#include "luaWrapper.h"

class CLuaCustomFunction
{
public:
    // the old way:
    CLuaCustomFunction(const char* theFullFunctionName,const char* theCallTips,std::vector<int>& theInputArgTypes,void(*callBack)(struct SLuaCallBack* p));
    // the new way:
    CLuaCustomFunction(const char* theFullFunctionName,const char* theCallTips,void(*callBack)(struct SScriptCallBack* cb));

    virtual ~CLuaCustomFunction();
    
    void registerCustomLuaFunction(luaWrap_lua_State* L,luaWrap_lua_CFunction func) const;
    bool getUsesStackToExchangeData() const;
    std::string getFunctionName() const;
    bool isFunctionNameSame(const char* fullName) const;
    bool isPluginNameSame(const char* plugName) const;
    std::string getPluginName() const;
    std::string getCallTips() const;
    int getFunctionID() const;
    void setFunctionID(int newID);
    static std::string _getFunctionNameFromFull(const char* fullName);
    static std::string _getPluginNameFromFull(const char* fullName);
    bool hasCallback() const;
    bool hasCalltipsAndSyntaxHighlighing() const;
    bool hasAutocompletion() const;
    bool isDeprecated() const;

    void(*callBackFunction_new)(struct SScriptCallBack* p);
    void(*callBackFunction_old)(struct SLuaCallBack* p);
    std::vector<int> inputArgTypes;

protected:
    std::string pluginName;
    std::string functionName;
    std::string callTips;
    int functionID;
    bool useStackToExchangeData;
    bool _functionIsDefinedInScript;
};
