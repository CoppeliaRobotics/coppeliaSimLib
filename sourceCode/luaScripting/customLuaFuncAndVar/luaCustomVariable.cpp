
#include "vrepMainHeader.h"
#include "luaCustomVariable.h"
#include "app.h"

CLuaCustomVariable::CLuaCustomVariable(const char* theFullVariableName,const char* theVariableValue,int theVariableStackValue)
{
    _variableName=_getVariableNameFromFull(theFullVariableName);
    _pluginName=_getPluginNameFromFull(theFullVariableName);
    _hasAutocompletion=true;
    _isDeprecated=(theVariableStackValue==-1);

    if (theVariableValue!=nullptr)
    { // we register a simple variable
        _variableValue=theVariableValue;
        _variableStackValue=0; // i.e. not using the stack
        _hasAutocompletion=(theVariableStackValue!=-1);
    }
    else
    {
        _variableValue="";
        _variableStackValue=theVariableStackValue;
    }
}

CLuaCustomVariable::~CLuaCustomVariable()
{
    if (_variableStackValue!=0)
        App::ct->interfaceStackContainer->destroyStack(_variableStackValue);
}

bool CLuaCustomVariable::getHasAutoCompletion() const
{
    return(_hasAutocompletion);
}

void CLuaCustomVariable::pushVariableOntoLuaStack(luaWrap_lua_State* L,bool handleOnlyRequireAssignments) const
{
    if (_variableStackValue==0)
    { // simple variable
        bool doIt=false;
        if ( handleOnlyRequireAssignments&&(_variableValue.find("require")==0) )
            doIt=true;
        if ( (!handleOnlyRequireAssignments)&&(_variableValue.find("require")!=0) )
            doIt=true;
        if (doIt)
        {
            std::string tmp(_variableName);
            tmp+="="+_variableValue;
            luaWrap_luaL_dostring(L,tmp.c_str());
        }
    }
    else
    { // stack variable
        if (_variableStackValue!=0)
        {
            if (!handleOnlyRequireAssignments)
            {
                CInterfaceStack* stack=App::ct->interfaceStackContainer->getStack(_variableStackValue);
                stack->buildOntoLuaStack(L,true);
                luaWrap_lua_setglobal(L,_variableName.c_str());
            }
        }
    }
}

bool CLuaCustomVariable::isVariableNameSame(const char* fullName) const
{
    std::string varName(_getVariableNameFromFull(fullName));
    return(_variableName.compare(varName)==0);
}

bool CLuaCustomVariable::isPluginNameSame(const char* plugName) const
{
    return(_pluginName.compare(plugName)==0);
}

bool CLuaCustomVariable::isDeprecated() const
{
    return(_isDeprecated);
}

bool CLuaCustomVariable::shouldBeDestroyed(const char* pluginName) const
{
    return(_pluginName.compare(pluginName)==0);
}

std::string CLuaCustomVariable::getVariableName() const
{
    return(_variableName);
}

std::string CLuaCustomVariable::_getVariableNameFromFull(const char* fullName) const
{
    std::string f(fullName);
    size_t p=f.find('@');
    if (p==std::string::npos)
        return(f);
    std::string subF(f.begin(),f.begin()+p);
    return(subF);
}

std::string CLuaCustomVariable::_getPluginNameFromFull(const char* fullName) const
{
    std::string f(fullName);
    size_t p=f.find('@');
    if (p==std::string::npos)
        return("");
    std::string subF(f.begin()+p+1,f.end());
    return(subF);
}
