#pragma once

#include <string>

class CScriptCustomVariable
{
public:
    CScriptCustomVariable(const char* theFullVariableName,const char* theVariableValue,int theVariableStackValue);

    virtual ~CScriptCustomVariable();
    
    bool shouldBeDestroyed(const char* pluginName) const;
    bool isVariableNameSame(const char* fullName) const;
    bool isPluginNameSame(const char* plugName) const;
    bool isDeprecated() const;
    bool getHasAutoCompletion() const;
    std::string getVariableName() const;
    std::string getVariableValue() const;
    int getVariableStackId() const;

protected:
    std::string _getVariableNameFromFull(const char* fullName) const;
    std::string _getPluginNameFromFull(const char* fullName) const;

    std::string _pluginName;
    std::string _variableName;
    std::string _variableValue;
    int _variableStackId;
    bool _isDeprecated;
    bool _hasAutocompletion;
};
