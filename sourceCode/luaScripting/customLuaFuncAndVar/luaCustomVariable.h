#pragma once

#include <string>
#include "luaWrapper.h"

class CLuaCustomVariable
{
public:
    CLuaCustomVariable(const char* theFullVariableName,const char* theVariableValue,int theVariableStackValue);

    virtual ~CLuaCustomVariable();
    
    void pushVariableOntoLuaStack(luaWrap_lua_State* L,bool handleOnlyRequireAssignments) const;
    bool shouldBeDestroyed(const char* pluginName) const;
    bool isVariableNameSame(const char* fullName) const;
    bool isPluginNameSame(const char* plugName) const;
    bool isDeprecated() const;
    bool getHasAutoCompletion() const;
    std::string getVariableName() const;

protected:
    std::string _getVariableNameFromFull(const char* fullName) const;
    std::string _getPluginNameFromFull(const char* fullName) const;

    std::string _pluginName;
    std::string _variableName;
    std::string _variableValue;
    int _variableStackValue;
    bool _isDeprecated;
    bool _hasAutocompletion;
};
