
#include "vrepMainHeader.h"
#include "app.h"
#include "luaCustomFuncAndVarContainer.h"

CLuaCustomFuncAndVarContainer::CLuaCustomFuncAndVarContainer()
{
    warningAboutFunctionNamesWithoutPlugin=false;
}

CLuaCustomFuncAndVarContainer::~CLuaCustomFuncAndVarContainer()
{
    removeAllCustomFunctions();
    removeAllCustomVariables();
}

void CLuaCustomFuncAndVarContainer::outputWarningWithFunctionNamesWithoutPlugin(bool o)
{
    warningAboutFunctionNamesWithoutPlugin=o;
}

void CLuaCustomFuncAndVarContainer::removeAllCustomFunctions()
{
    for (size_t i=0;i<allCustomFunctions.size();i++)
        delete allCustomFunctions[i];
    allCustomFunctions.clear();
}

bool CLuaCustomFuncAndVarContainer::removeCustomFunction(const char* fullFunctionName)
{
    for (size_t i=0;i<allCustomFunctions.size();i++)
    {
        if (allCustomFunctions[i]->isFunctionNameSame(fullFunctionName))
        { // we have to remove this one
            delete allCustomFunctions[i];
            allCustomFunctions.erase(allCustomFunctions.begin()+i);
            return(true);
        }
    }
    return(false);
}

void CLuaCustomFuncAndVarContainer::announcePluginWasKilled(const char* pluginName)
{
    for (size_t i=0;i<allCustomFunctions.size();i++)
    {
        if (allCustomFunctions[i]->isPluginNameSame(pluginName))
        { // we have to remove this one
            delete allCustomFunctions[i];
            allCustomFunctions.erase(allCustomFunctions.begin()+i);
            i--; // reprocess this position
        }
    }

    for (size_t i=0;i<allCustomVariables.size();i++)
    {
        if (allCustomVariables[i]->isPluginNameSame(pluginName))
        { // we have to remove this one
            delete allCustomVariables[i];
            allCustomVariables.erase(allCustomVariables.begin()+i);
            i--; // reprocess this position
        }
    }
}

bool CLuaCustomFuncAndVarContainer::insertCustomFunction(CLuaCustomFunction* function)
{
    if (doesCustomFunctionAlreadyExist(function))
        return(false);
    if (function->getFunctionName().length()==0)
        return(false);

    if ((function->getPluginName()=="")&&warningAboutFunctionNamesWithoutPlugin)
    {
        printf("Warning: Detected a custom function name that is not formatted as\n");
        printf("         funcName@pluginName: %s\n",function->getFunctionName().c_str());
        printf("         Unloading this plugin dynamically might lead to a crash.\n");
    }

    int newID=0;
    while (getCustomFunctionFromID(newID)!=nullptr)
        newID++;
    function->setFunctionID(newID);
    allCustomFunctions.push_back(function);
    return(true);
}

bool CLuaCustomFuncAndVarContainer::doesCustomFunctionAlreadyExist(CLuaCustomFunction* function)
{
    for (int i=0;i<int(allCustomFunctions.size());i++)
    {
        if (function->getFunctionName()==allCustomFunctions[i]->getFunctionName())
            return(true);
    }
    return(false);
}

CLuaCustomFunction* CLuaCustomFuncAndVarContainer::getCustomFunctionFromID(int functionID)
{
    for (int i=0;i<int(allCustomFunctions.size());i++)
    {
        if (functionID==allCustomFunctions[i]->getFunctionID())
            return(allCustomFunctions[i]);
    }
    return(nullptr);
}

void CLuaCustomFuncAndVarContainer::appendAllFunctionNames_spaceSeparated(std::string& v)
{
    for (size_t i=0;i<allCustomFunctions.size();i++)
    {
        if (allCustomFunctions[i]->hasCalltipsAndSyntaxHighlighing())
        {
            v+=allCustomFunctions[i]->getFunctionName();
            v+=" ";
        }
    }
}

void CLuaCustomFuncAndVarContainer::registerCustomLuaFunctions(luaWrap_lua_State* L,luaWrap_lua_CFunction func)
{
    for (size_t i=0;i<allCustomFunctions.size();i++)
    {
        if (allCustomFunctions[i]->hasCallback())
            allCustomFunctions[i]->registerCustomLuaFunction(L,func);
    }
}

void CLuaCustomFuncAndVarContainer::removeAllCustomVariables()
{
    for (size_t i=0;i<allCustomVariables.size();i++)
        delete allCustomVariables[i];
    allCustomVariables.clear();
}

bool CLuaCustomFuncAndVarContainer::isVariableNamePresent(const char* name)
{
    for (size_t i=0;i<allCustomVariables.size();i++)
    {
        if (allCustomVariables[i]->getVariableName().compare(name)==0)
            return(true);
    }
    return(false);
}

int CLuaCustomFuncAndVarContainer::isFuncOrConstDeprecated(const char* name)
{
    for (size_t i=0;i<allCustomFunctions.size();i++)
    {
        if (allCustomFunctions[i]->getFunctionName().compare(name)==0)
        {
            if (allCustomFunctions[i]->isDeprecated())
                return(1);
            return(0);
        }
    }
    for (size_t i=0;i<allCustomVariables.size();i++)
    {
        if (allCustomVariables[i]->getVariableName().compare(name)==0)
        {
            if (allCustomVariables[i]->isDeprecated())
                return(1);
            return(0);
        }
    }
    return(-1);
}

bool CLuaCustomFuncAndVarContainer::removeCustomVariable(const char* fullVariableName)
{
    for (size_t i=0;i<allCustomVariables.size();i++)
    {
        if (allCustomVariables[i]->isVariableNameSame(fullVariableName))
        { // we have to remove this one
            delete allCustomVariables[i];
            allCustomVariables.erase(allCustomVariables.begin()+i);
            return(true);
        }
    }
    return(false);
}

bool CLuaCustomFuncAndVarContainer::insertCustomVariable(const char* fullVariableName,const char* variableValue,int stackHandle)
{
    if (variableValue!=nullptr)
    { // we register a simple variable
        removeCustomVariable(fullVariableName);
        CLuaCustomVariable* v=new CLuaCustomVariable(fullVariableName,variableValue,stackHandle);
        allCustomVariables.push_back(v);
        return(true);
    }
    else
    {
        if (stackHandle==0)
        { // we only register a variable name for auto-completion (variable has no value!)
            removeCustomVariable(fullVariableName);
            CLuaCustomVariable* v=new CLuaCustomVariable(fullVariableName,nullptr,0);
            allCustomVariables.push_back(v);
        }
        else
        { // register a stack variable
            CInterfaceStack* stack=App::ct->interfaceStackContainer->getStack(stackHandle);
            if (stack==nullptr)
                return(false);
            if (stack->getStackSize()<1)
                return(false);
            while (stack->getStackSize()>1)
            { // keep one item in the stack (the top item)
                stack->moveStackItemToTop(0);
                stack->popStackValue(1);
            }
            removeCustomVariable(fullVariableName);
            CLuaCustomVariable* v=new CLuaCustomVariable(fullVariableName,nullptr,stackHandle);
            allCustomVariables.push_back(v);
        }
        return(true);
    }
}

void CLuaCustomFuncAndVarContainer::assignCustomVariables(luaWrap_lua_State* L,bool handleOnlyRequireAssignments)
{
    for (size_t i=0;i<allCustomVariables.size();i++)
        allCustomVariables[i]->pushVariableOntoLuaStack(L,handleOnlyRequireAssignments);
}

void CLuaCustomFuncAndVarContainer::appendAllVariableNames_spaceSeparated_keywordHighlight(std::string& v)
{
    for (size_t i=0;i<allCustomVariables.size();i++)
    {
        if (allCustomVariables[i]->getHasAutoCompletion())
        {
            v+=allCustomVariables[i]->getVariableName();
            v+=" ";
        }
    }
}

void CLuaCustomFuncAndVarContainer::pushAllFunctionNamesThatStartSame_autoCompletionList(const std::string& txt,std::vector<std::string>& v,std::map<std::string,bool>& m)
{
    std::string ttxt(txt);
    bool hasDot=(ttxt.find('.')!=std::string::npos);
    for (size_t i=0;i<allCustomFunctions.size();i++)
    {
        if (allCustomFunctions[i]->hasAutocompletion())
        {
            std::string n(allCustomFunctions[i]->getFunctionName());
            if ((n.size()>=txt.size())&&(n.compare(0,txt.size(),txt)==0))
            {
                if (!hasDot)
                {
                    size_t dp=n.find('.');
                    if ( (dp!=std::string::npos)&&(txt.size()>0) )
                        n.erase(n.begin()+dp,n.end()); // we only push the text up to the dot, if txt is not empty
                }
                std::map<std::string,bool>::iterator it=m.find(n);
                if (it==m.end())
                {
                    m[n]=true;
                    v.push_back(n);
                }
            }
        }
    }
}

void CLuaCustomFuncAndVarContainer::pushAllVariableNamesThatStartSame_autoCompletionList(const std::string& txt,std::vector<std::string>& v,std::map<std::string,bool>& m)
{
    std::string ttxt(txt);
    bool hasDot=(ttxt.find('.')!=std::string::npos);
    for (size_t i=0;i<allCustomVariables.size();i++)
    {
        if (allCustomVariables[i]->getHasAutoCompletion())
        {
            std::string n(allCustomVariables[i]->getVariableName());
            if ((n.size()>=txt.size())&&(n.compare(0,txt.size(),txt)==0))
            {
                if (!hasDot)
                {
                    size_t dp=n.find('.');
                    if ( (dp!=std::string::npos)&&(txt.size()>0) )
                        n.erase(n.begin()+dp,n.end()); // we only push the text up to the dot, if txt is not empty
                }
                std::map<std::string,bool>::iterator it=m.find(n);
                if (it==m.end())
                {
                    m[n]=true;
                    v.push_back(n);
                }
            }
        }
    }
}

