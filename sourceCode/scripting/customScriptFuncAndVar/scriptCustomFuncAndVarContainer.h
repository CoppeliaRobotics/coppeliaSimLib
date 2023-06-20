#pragma once

#include <scriptCustomFunction.h>
#include <scriptCustomVariable.h>
#include <set>

class CScriptCustomFuncAndVarContainer
{
public:
    CScriptCustomFuncAndVarContainer();
    virtual ~CScriptCustomFuncAndVarContainer();

    void removeAllCustomFunctions();
    void removeAllCustomVariables();
    bool removeCustomFunction(const char* fullFunctionName);
    bool removeCustomVariable(const char* fullVariableName);
    bool insertCustomFunction(CScriptCustomFunction* function);
    bool insertCustomVariable(const char* fullVariableName,const char* variableValue,int stackHandle);

    void announcePluginWasKilled(const char* pluginName);
    bool doesCustomFunctionAlreadyExist(CScriptCustomFunction* function);
    CScriptCustomFunction* getCustomFunctionFromID(int functionID);
    void appendAllFunctionNames_spaceSeparated(std::string& v);

    size_t getCustomFunctionCount() const;
    CScriptCustomFunction* getCustomFunctionFromIndex(size_t index);
    size_t getCustomVariableCount() const;
    CScriptCustomVariable* getCustomVariableFromIndex(size_t index);

    void appendAllVariableNames_spaceSeparated_keywordHighlight(std::string& v);
    void insertAllFunctionNamesThatStartSame(const char* txt,std::set<std::string>& v);
    void insertAllVariableNamesThatStartSame(const char* txt,std::set<std::string>& v);

protected:
    std::vector<CScriptCustomFunction*> _allCustomFunctions;
    std::vector<CScriptCustomVariable*> _allCustomVariables;
};
