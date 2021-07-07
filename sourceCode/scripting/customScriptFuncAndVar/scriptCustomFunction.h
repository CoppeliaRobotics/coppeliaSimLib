#pragma once

#include <vector>
#include <string>

class CScriptCustomFunction
{
public:
    CScriptCustomFunction(const char* theFullFunctionName,const char* theCallTips,void(*callBack)(struct SScriptCallBack* cb));

    virtual ~CScriptCustomFunction();
    
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
    std::vector<int> inputArgTypes;

    // Old:
    CScriptCustomFunction(const char* theFullFunctionName,const char* theCallTips,std::vector<int>& theInputArgTypes,void(*callBack)(struct SLuaCallBack* p));
    void(*callBackFunction_old)(struct SLuaCallBack* p);

protected:
    std::string pluginName;
    std::string functionName;
    std::string callTips;
    int functionID;
    bool useStackToExchangeData;
    bool _functionIsDefinedInScript;
};
