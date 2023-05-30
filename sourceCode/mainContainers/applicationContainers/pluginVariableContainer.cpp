#include <pluginVariableContainer.h>
#include <app.h>

CPluginVariableContainer::CPluginVariableContainer()
{
}

CPluginVariableContainer::~CPluginVariableContainer()
{
    clear();
}

SPluginVariable* CPluginVariableContainer::getVariableFromName(const char* name)
{
    SPluginVariable* retVal=nullptr;
    auto it=_allVariables_map.find(name);
    if (it!=_allVariables_map.end())
        retVal=&_allVariables_vect[it->second];
    return(retVal);
}

SPluginVariable* CPluginVariableContainer::getVariableFromIndex(size_t ind)
{
    SPluginVariable* retVal=nullptr;
    if (ind<_allVariables_vect.size())
        retVal=&_allVariables_vect[ind];
    return(retVal);
}

void CPluginVariableContainer::clear()
{
    for (size_t i=0;i<_allVariables_vect.size();i++)
    {
        if (_allVariables_vect[i].stackHandle!=-1)
            App::worldContainer->interfaceStackContainer->destroyStack(_allVariables_vect[i].stackHandle);
    }
    _allVariables_vect.clear();
    _allVariables_map.clear();
}

bool CPluginVariableContainer::addVariable(const char* varName,const char* varValue,int stackHandle/*=-1*/)
{
    bool retVal=false;
    SPluginVariable* it=getVariableFromName(varName);
    if (it==nullptr)
    {
        retVal=true;
        SPluginVariable s;
        s.varName=varName;
        s.varValue=varValue;
        s.stackHandle=stackHandle;
        _allVariables_map[varName]=_allVariables_vect.size();
        _allVariables_vect.push_back(s);
    }
    else
    { // variable already exists. Replace it
        it->varValue=varValue;
        if (it->stackHandle!=-1)
            App::worldContainer->interfaceStackContainer->destroyStack(it->stackHandle);
        it->stackHandle=stackHandle;
    }
    return(retVal);
}
