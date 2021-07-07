#include "scriptCustomVariable.h"
#include "app.h"

CScriptCustomVariable::CScriptCustomVariable(const char* theFullVariableName,const char* theVariableValue,int theVariableStackValue)
{
    _variableName=_getVariableNameFromFull(theFullVariableName);
    _pluginName=_getPluginNameFromFull(theFullVariableName);
    _hasAutocompletion=true;
    _isDeprecated=(theVariableStackValue==-1);

    if (theVariableValue!=nullptr)
    { // we register a simple variable
        _variableValue=theVariableValue;
        _variableStackId=0; // i.e. not using the stack
        _hasAutocompletion=(theVariableStackValue!=-1);
    }
    else
    {
        _variableValue="";
        _variableStackId=theVariableStackValue;
    }
}

CScriptCustomVariable::~CScriptCustomVariable()
{
    if (_variableStackId!=0)
        App::worldContainer->interfaceStackContainer->destroyStack(_variableStackId);
}

bool CScriptCustomVariable::getHasAutoCompletion() const
{
    return(_hasAutocompletion);
}

bool CScriptCustomVariable::isVariableNameSame(const char* fullName) const
{
    std::string varName(_getVariableNameFromFull(fullName));
    return(_variableName.compare(varName)==0);
}

bool CScriptCustomVariable::isPluginNameSame(const char* plugName) const
{
    return(_pluginName.compare(plugName)==0);
}

bool CScriptCustomVariable::isDeprecated() const
{
    return(_isDeprecated);
}

bool CScriptCustomVariable::shouldBeDestroyed(const char* pluginName) const
{
    return(_pluginName.compare(pluginName)==0);
}

std::string CScriptCustomVariable::getVariableName() const
{
    return(_variableName);
}

std::string CScriptCustomVariable::getVariableValue() const
{
    return(_variableValue);
}

int CScriptCustomVariable::getVariableStackId() const
{
    return(_variableStackId);
}

std::string CScriptCustomVariable::_getVariableNameFromFull(const char* fullName) const
{
    std::string f(fullName);
    size_t p=f.find('@');
    if (p==std::string::npos)
        return(f);
    std::string subF(f.begin(),f.begin()+p);
    return(subF);
}

std::string CScriptCustomVariable::_getPluginNameFromFull(const char* fullName) const
{
    std::string f(fullName);
    size_t p=f.find('@');
    if (p==std::string::npos)
        return("");
    std::string subF(f.begin()+p+1,f.end());
    return(subF);
}
