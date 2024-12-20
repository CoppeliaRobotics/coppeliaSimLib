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

SCodeEditorInfo* CCodeEditorInfos::getInfoFromFilename(const char* filename)
{
    SCodeEditorInfo* retVal = nullptr;
    auto it = _allInfos.find(filename);
    if (it != _allInfos.end())
        retVal = &it->second;
    return (retVal);
}

void CCodeEditorInfos::setInfo(const char* filename, const char* info, std::string* errorString /*=nullptr*/)
{
    SCodeEditorInfo nf;
    SCodeEditorInfo* nsinfo = getInfoFromFilename(filename);
    if (nsinfo == nullptr)
    {
        nf.funcs = new CCodeEditorFunctions();
        nf.vars = new CCodeEditorVariables();
        _allInfos[filename] = nf;
        auto it = _allInfos.find(filename);
        nsinfo = &it->second;
    }
    else
    {
        nsinfo->funcs->clear();
        nsinfo->vars->clear();
    }

    bool res = nsinfo->funcs->set(info);
    res = res && nsinfo->vars->set(info);

    if (!res)
    {
        removeInfo(filename);
        if (errorString != nullptr)
            errorString[0] = SIM_ERROR_INVALID_INFO_STRING;
    }
    /*    else
        {
            nsinfo->funcs->print();
            nsinfo->vars->print();
        }*/
}

void CCodeEditorInfos::removeInfo(const char* filename)
{
    auto it = _allInfos.find(filename);
    if (it != _allInfos.end())
    {
        delete it->second.funcs;
        delete it->second.vars;
        _allInfos.erase(it);
    }
}

void CCodeEditorInfos::clear()
{
    for (auto it = _allInfos.begin(); it != _allInfos.end(); it++)
    {
        delete it->second.funcs;
        delete it->second.vars;
    }
    _allInfos.clear();
}

void CCodeEditorInfos::insertWhatStartsSame(const char* txt, std::set<std::string>& v, int what, const CScriptObject* requestOrigin) const
{
    for (auto it = _allInfos.begin(); it != _allInfos.end(); it++)
    {
        if ((requestOrigin == nullptr) || (requestOrigin->wasModulePreviouslyUsed(it->first.c_str())) || (it->first == ""))
        { // only if that item was previously required by that script
            if ((what & 1) != 0)
                it->second.funcs->insertWhatStartsSame(txt, v);
            if ((what & 2) != 0)
                it->second.vars->insertWhatStartsSame(txt, v);
        }
    }
}

std::string CCodeEditorInfos::getFunctionCalltip(const char* txt, const CScriptObject* requestOrigin) const
{
    std::string retVal;
    for (auto it = _allInfos.begin(); it != _allInfos.end(); it++)
    {
        if ((requestOrigin == nullptr) || (requestOrigin->wasModulePreviouslyUsed(it->first.c_str())) || (it->first == ""))
        { // only if that item was previously required by that script
            retVal = it->second.funcs->getFunctionCalltip(txt);
            if (retVal.size() != 0)
                break;
        }
    }
    return (retVal);
}
