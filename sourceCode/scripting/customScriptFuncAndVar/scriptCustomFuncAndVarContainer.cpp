#include "app.h"
#include "scriptCustomFuncAndVarContainer.h"

CScriptCustomFuncAndVarContainer::CScriptCustomFuncAndVarContainer()
{
    _warningAboutFunctionNamesWithoutPlugin=false;
}

CScriptCustomFuncAndVarContainer::~CScriptCustomFuncAndVarContainer()
{
    removeAllCustomFunctions();
    removeAllCustomVariables();
}

void CScriptCustomFuncAndVarContainer::outputWarningWithFunctionNamesWithoutPlugin(bool o)
{
    _warningAboutFunctionNamesWithoutPlugin=o;
}

void CScriptCustomFuncAndVarContainer::removeAllCustomFunctions()
{
    for (size_t i=0;i<_allCustomFunctions.size();i++)
        delete _allCustomFunctions[i];
    _allCustomFunctions.clear();
}

bool CScriptCustomFuncAndVarContainer::removeCustomFunction(const char* fullFunctionName)
{
    for (size_t i=0;i<_allCustomFunctions.size();i++)
    {
        if (_allCustomFunctions[i]->isFunctionNameSame(fullFunctionName))
        { // we have to remove this one
            delete _allCustomFunctions[i];
            _allCustomFunctions.erase(_allCustomFunctions.begin()+i);
            return(true);
        }
    }
    return(false);
}

void CScriptCustomFuncAndVarContainer::announcePluginWasKilled(const char* pluginName)
{
    for (size_t i=0;i<_allCustomFunctions.size();i++)
    {
        if (_allCustomFunctions[i]->isPluginNameSame(pluginName))
        { // we have to remove this one
            delete _allCustomFunctions[i];
            _allCustomFunctions.erase(_allCustomFunctions.begin()+i);
            i--; // reprocess this position
        }
    }

    for (size_t i=0;i<_allCustomVariables.size();i++)
    {
        if (_allCustomVariables[i]->isPluginNameSame(pluginName))
        { // we have to remove this one
            delete _allCustomVariables[i];
            _allCustomVariables.erase(_allCustomVariables.begin()+i);
            i--; // reprocess this position
        }
    }
}

bool CScriptCustomFuncAndVarContainer::insertCustomFunction(CScriptCustomFunction* function)
{
    if (doesCustomFunctionAlreadyExist(function))
        return(false);
    if (function->getFunctionName().length()==0)
        return(false);

    if ((function->getPluginName()=="")&&_warningAboutFunctionNamesWithoutPlugin)
    {
        std::string msg("detected a custom function name that is not formatted as");
        msg+="\n         funcName@pluginName: ";
        msg+=function->getFunctionName();
        msg+="\n         Unloading this plugin dynamically might lead to a crash.";
        App::logMsg(sim_verbosity_warnings,msg.c_str());
    }

    int newID=0;
    while (getCustomFunctionFromID(newID)!=nullptr)
        newID++;
    function->setFunctionID(newID);
    _allCustomFunctions.push_back(function);
    return(true);
}

bool CScriptCustomFuncAndVarContainer::doesCustomFunctionAlreadyExist(CScriptCustomFunction* function)
{
    for (int i=0;i<int(_allCustomFunctions.size());i++)
    {
        if (function->getFunctionName()==_allCustomFunctions[i]->getFunctionName())
            return(true);
    }
    return(false);
}

CScriptCustomFunction* CScriptCustomFuncAndVarContainer::getCustomFunctionFromID(int functionID)
{
    for (size_t i=0;i<_allCustomFunctions.size();i++)
    {
        if (functionID==_allCustomFunctions[i]->getFunctionID())
            return(_allCustomFunctions[i]);
    }
    return(nullptr);
}

size_t CScriptCustomFuncAndVarContainer::getCustomFunctionCount() const
{
    return(_allCustomFunctions.size());
}

CScriptCustomFunction* CScriptCustomFuncAndVarContainer::getCustomFunctionFromIndex(size_t index)
{
    return(_allCustomFunctions[index]);
}

size_t CScriptCustomFuncAndVarContainer::getCustomVariableCount() const
{
    return(_allCustomVariables.size());
}

CScriptCustomVariable* CScriptCustomFuncAndVarContainer::getCustomVariableFromIndex(size_t index)
{
    return(_allCustomVariables[index]);
}

void CScriptCustomFuncAndVarContainer::appendAllFunctionNames_spaceSeparated(std::string& v)
{
    for (size_t i=0;i<_allCustomFunctions.size();i++)
    {
        if (_allCustomFunctions[i]->hasCalltipsAndSyntaxHighlighing())
        {
            v+=_allCustomFunctions[i]->getFunctionName();
            v+=" ";
        }
    }
}

void CScriptCustomFuncAndVarContainer::removeAllCustomVariables()
{
    for (size_t i=0;i<_allCustomVariables.size();i++)
        delete _allCustomVariables[i];
    _allCustomVariables.clear();
}

bool CScriptCustomFuncAndVarContainer::isVariableNamePresent(const char* name)
{
    for (size_t i=0;i<_allCustomVariables.size();i++)
    {
        if (_allCustomVariables[i]->getVariableName().compare(name)==0)
            return(true);
    }
    return(false);
}

int CScriptCustomFuncAndVarContainer::isFuncOrConstDeprecated(const char* name)
{
    for (size_t i=0;i<_allCustomFunctions.size();i++)
    {
        if (_allCustomFunctions[i]->getFunctionName().compare(name)==0)
        {
            if (_allCustomFunctions[i]->isDeprecated())
                return(1);
            return(0);
        }
    }
    for (size_t i=0;i<_allCustomVariables.size();i++)
    {
        if (_allCustomVariables[i]->getVariableName().compare(name)==0)
        {
            if (_allCustomVariables[i]->isDeprecated())
                return(1);
            return(0);
        }
    }
    return(-1);
}

bool CScriptCustomFuncAndVarContainer::removeCustomVariable(const char* fullVariableName)
{
    for (size_t i=0;i<_allCustomVariables.size();i++)
    {
        if (_allCustomVariables[i]->isVariableNameSame(fullVariableName))
        { // we have to remove this one
            delete _allCustomVariables[i];
            _allCustomVariables.erase(_allCustomVariables.begin()+i);
            return(true);
        }
    }
    return(false);
}

bool CScriptCustomFuncAndVarContainer::insertCustomVariable(const char* fullVariableName,const char* variableValue,int stackHandle)
{
    if (variableValue!=nullptr)
    { // we register a simple variable
        removeCustomVariable(fullVariableName);
        CScriptCustomVariable* v=new CScriptCustomVariable(fullVariableName,variableValue,stackHandle);
        _allCustomVariables.push_back(v);
        return(true);
    }
    else
    {
        if (stackHandle==0)
        { // we only register a variable name for auto-completion (variable has no value!)
            removeCustomVariable(fullVariableName);
            CScriptCustomVariable* v=new CScriptCustomVariable(fullVariableName,nullptr,0);
            _allCustomVariables.push_back(v);
        }
        else
        { // register a stack variable
            CInterfaceStack* stack=App::worldContainer->interfaceStackContainer->getStack(stackHandle);
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
            CScriptCustomVariable* v=new CScriptCustomVariable(fullVariableName,nullptr,stackHandle);
            _allCustomVariables.push_back(v);
        }
        return(true);
    }
}

void CScriptCustomFuncAndVarContainer::appendAllVariableNames_spaceSeparated_keywordHighlight(std::string& v)
{
    for (size_t i=0;i<_allCustomVariables.size();i++)
    {
        if (_allCustomVariables[i]->getHasAutoCompletion())
        {
            v+=_allCustomVariables[i]->getVariableName();
            v+=" ";
        }
    }
}

void CScriptCustomFuncAndVarContainer::pushAllFunctionNamesThatStartSame_autoCompletionList(const char* txt,std::vector<std::string>& v,std::map<std::string,bool>& m)
{
    std::string ttxt(txt);
    bool hasDot=(ttxt.find('.')!=std::string::npos);
    for (size_t i=0;i<_allCustomFunctions.size();i++)
    {
        if (_allCustomFunctions[i]->hasAutocompletion())
        {
            std::string n(_allCustomFunctions[i]->getFunctionName());

            if ((n.size()>=ttxt.size())&&(n.compare(0,ttxt.size(),ttxt)==0))
            {
                if (!hasDot)
                {
                    size_t dp=n.find('.');
                    if ( (dp!=std::string::npos)&&(ttxt.size()>0) )
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

void CScriptCustomFuncAndVarContainer::pushAllVariableNamesThatStartSame_autoCompletionList(const char* txt,std::vector<std::string>& v,std::map<std::string,bool>& m)
{
    std::string ttxt(txt);
    bool hasDot=(ttxt.find('.')!=std::string::npos);
    for (size_t i=0;i<_allCustomVariables.size();i++)
    {
        if (_allCustomVariables[i]->getHasAutoCompletion())
        {
            std::string n(_allCustomVariables[i]->getVariableName());
            if ((n.size()>=ttxt.size())&&(n.compare(0,ttxt.size(),ttxt)==0))
            {
                if (!hasDot)
                {
                    size_t dp=n.find('.');
                    if ( (dp!=std::string::npos)&&(ttxt.size()>0) )
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

