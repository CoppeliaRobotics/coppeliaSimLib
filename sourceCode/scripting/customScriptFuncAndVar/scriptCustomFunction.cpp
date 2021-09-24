#include "scriptCustomFunction.h"

CScriptCustomFunction::CScriptCustomFunction(const char* theFullFunctionName,const char* theCallTips,void(*callBack)(struct SScriptCallBack* cb))
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

CScriptCustomFunction::~CScriptCustomFunction()
{
}

bool CScriptCustomFunction::hasCallback() const
{
    return( (callBackFunction_old!=nullptr)||(callBackFunction_new!=nullptr) );
}

bool CScriptCustomFunction::hasCalltipsAndSyntaxHighlighing() const
{
    return(callTips.length()>0);
}

bool CScriptCustomFunction::hasAutocompletion() const
{
//    return( (callTips.length()>0)&&(hasCallback()||_functionIsDefinedInScript) );
    return(callTips.length()>0);
}

bool CScriptCustomFunction::isDeprecated() const
{
//    return( (callTips.length()==0)||( (!hasCallback())&&(!_functionIsDefinedInScript) ) );
    return(callTips.length()==0);
}

bool CScriptCustomFunction::getUsesStackToExchangeData() const
{
    return(useStackToExchangeData);
}

bool CScriptCustomFunction::isFunctionNameSame(const char* fullName) const
{
    std::string funcName(_getFunctionNameFromFull(fullName));
    return(functionName.compare(funcName)==0);
}

bool CScriptCustomFunction::isPluginNameSame(const char* plugName) const
{
    return(pluginName.compare(plugName)==0);
}

std::string CScriptCustomFunction::getFunctionName() const
{
    return(functionName);
}

std::string CScriptCustomFunction::getPluginName() const
{
    return(pluginName);
}

std::string CScriptCustomFunction::getCallTips() const
{
    return(callTips);
}

int CScriptCustomFunction::getFunctionID() const
{
    return(functionID);
}

void CScriptCustomFunction::setFunctionID(int newID)
{
    functionID=newID;
}

std::string CScriptCustomFunction::_getFunctionNameFromFull(const char* fullName)
{
    std::string f(fullName);
    size_t p=f.find('@');
    if (p==std::string::npos)
        return(f);
    std::string subF(f.begin(),f.begin()+p);
    return(subF);
}

std::string CScriptCustomFunction::_getPluginNameFromFull(const char* fullName)
{
    std::string f(fullName);
    size_t p=f.find('@');
    if (p==std::string::npos)
        return("");
    std::string subF(f.begin()+p+1,f.end());
    return(subF);
}

CScriptCustomFunction::CScriptCustomFunction(const char* theFullFunctionName,const char* theCallTips,std::vector<int>& theInputArgTypes,void(*callBack)(struct SLuaCallBack* p))
{ // old
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

