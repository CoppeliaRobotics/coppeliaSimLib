#pragma once

#include "luaCustomFunction.h"
#include "luaCustomVariable.h"
#include <map>

class CLuaCustomFuncAndVarContainer
{
public:
    CLuaCustomFuncAndVarContainer();
    virtual ~CLuaCustomFuncAndVarContainer();

    void removeAllCustomFunctions();
    void removeAllCustomVariables();
    bool removeCustomFunction(const char* fullFunctionName);
    bool removeCustomVariable(const char* fullVariableName);
    bool insertCustomFunction(CLuaCustomFunction* function);
    bool insertCustomVariable(const char* fullVariableName,const char* variableValue,int stackHandle);

    void announcePluginWasKilled(const char* pluginName);
    bool doesCustomFunctionAlreadyExist(CLuaCustomFunction* function);
    CLuaCustomFunction* getCustomFunctionFromID(int functionID);
    void appendAllFunctionNames_spaceSeparated(std::string& v);
    void registerCustomLuaFunctions(luaWrap_lua_State* L,luaWrap_lua_CFunction func);

    void outputWarningWithFunctionNamesWithoutPlugin(bool o);

    void assignCustomVariables(luaWrap_lua_State* L,bool handleOnlyRequireAssignments);
    void appendAllVariableNames_spaceSeparated_keywordHighlight(std::string& v);
    void pushAllFunctionNamesThatStartSame_autoCompletionList(const std::string& txt,std::vector<std::string>& v,std::map<std::string,bool>& m);
    void pushAllVariableNamesThatStartSame_autoCompletionList(const std::string& txt,std::vector<std::string>& v,std::map<std::string,bool>& m);

    bool isVariableNamePresent(const char* name);
    int isFuncOrConstDeprecated(const char* name);


    std::vector<CLuaCustomFunction*> allCustomFunctions;
    std::vector<CLuaCustomVariable*> allCustomVariables;

    bool warningAboutFunctionNamesWithoutPlugin;
};
