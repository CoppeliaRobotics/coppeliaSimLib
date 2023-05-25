#include <codeEditorInfos.h>
#include <app.h>
#include <apiErrors.h>

CCodeEditorInfos::CCodeEditorInfos()
{
}

CCodeEditorInfos::~CCodeEditorInfos()
{
    clear();
}

SCodeEditorInfo* CCodeEditorInfos::getInfoFromNamespace(const char* ns)
{
    SCodeEditorInfo* retVal=nullptr;
    auto it=_allInfos.find(ns);
    if (it!=_allInfos.end())
        retVal=&it->second;
    return(retVal);
}

void CCodeEditorInfos::setInfo(const char* namespaceAndVersion,const char* info,std::string* errorString/*=nullptr*/)
{
    std::string nmspNver(namespaceAndVersion);
    size_t p=nmspNver.find('-');
    std::string ns(nmspNver);
    std::string v;
    if (p!=std::string::npos)
    {
        ns.assign(nmspNver.begin(),nmspNver.begin()+p);
        v.assign(nmspNver.begin()+p+1,nmspNver.end());
    }
    if (v!="default")
    { // specialized API for code editors, when such a namespace plugin exists and is loaded
        CPlugin* plug=App::worldContainer->pluginContainer->getPluginFromName(namespaceAndVersion);
        if (plug!=nullptr)
        {
            bool res=plug->getCodeEditorFunctions()->set(info);
            res=res&&plug->getCodeEditorVariables()->set(info);
/*
            plug->getCodeEditorFunctions()->print();
            plug->getCodeEditorVariables()->print();
*/
            if ( (!res)&&(errorString!=nullptr) )
                errorString[0]=SIM_ERROR_INVALID_INFO_STRING;
        }
        else
        {
            if (errorString!=nullptr)
                errorString[0]=SIM_ERROR_INVALID_PLUGIN;
        }
    }
    else
    { // overall, default API for code editors, when such namespace plugin not explicitely loaded
        SCodeEditorInfo nf;
        SCodeEditorInfo* nsinfo=getInfoFromNamespace(ns.c_str());
        if (nsinfo==nullptr)
        {
            nf.funcs=new CCodeEditorFunctions();
            nf.vars=new CCodeEditorVariables();
            _allInfos[ns]=nf;
            auto it=_allInfos.find(ns);
            nsinfo=&it->second;
        }
        else
        {
            nsinfo->funcs->clear();
            nsinfo->vars->clear();
        }

        bool res=nsinfo->funcs->set(info);
        res=res&&nsinfo->vars->set(info);

        if (!res)
        {
            removeInfo(ns.c_str());
            if (errorString!=nullptr)
                errorString[0]=SIM_ERROR_INVALID_INFO_STRING;
        }
/*        else
        {
            nsinfo->funcs->print();
            nsinfo->vars->print();
        }*/
    }
}

void CCodeEditorInfos::removeInfo(const char* ns)
{
    auto it=_allInfos.find(ns);
    if (it!=_allInfos.end())
    {
        delete it->second.funcs;
        delete it->second.vars;
        _allInfos.erase(it);
    }
}

void CCodeEditorInfos::clear()
{
    for (auto it=_allInfos.begin();it!=_allInfos.end();it++)
    {
        delete it->second.funcs;
        delete it->second.vars;
    }
    _allInfos.clear();
}

void CCodeEditorInfos::insertWhatStartsSame(const char* txt,std::set<std::string>& v,int what) const
{
    for (auto it=_allInfos.begin();it!=_allInfos.end();it++)
    {
        if ((what&1)!=0)
            it->second.funcs->insertWhatStartsSame(txt,v);
        if ((what&2)!=0)
            it->second.vars->insertWhatStartsSame(txt,v);
    }
}

std::string CCodeEditorInfos::getFunctionCalltip(const char* txt) const
{
    std::string retVal;
    for (auto it=_allInfos.begin();it!=_allInfos.end();it++)
    {
        retVal=it->second.funcs->getFunctionCalltip(txt);
        if (retVal.size()!=0)
            break;
    }
    return(retVal);
}
