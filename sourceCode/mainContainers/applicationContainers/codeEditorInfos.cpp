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
{ // txt: e.g. "sim.getObject", "sim-2.getObject", "sim-*.getObject", "getAsString"
    std::string txtNoVer(txt); // e.g. "sim.getObject", "getAsString"
    bool allVers = false;
    std::string verNs = ""; // e.g. "" (for latest version), "sim-2" (for specific version)
    auto pos1 = txtNoVer.find('.');
    if (pos1 != std::string::npos)
    {
        std::string n1(txtNoVer.substr(0, pos1)); // e.g. "sim", "sim-2", "sim-*"
        std::string n2(txtNoVer.substr(pos1 + 1)); // e.g. "getObject"
        auto pos2 = n1.find('-');
        if (pos2 != std::string::npos)
        {
            std::string sn(n1.substr(0, pos2)); // e.g. "sim"
            txtNoVer = sn + "." + n2;
            if (n1[n1.size() - 1] == '*')
            {
                allVers = true;
                n1 = sn;
            }
            else
                verNs = n1;
        }
    }
    std::vector<std::pair<std::string, std::string>> callTipsAndModuleNames;
    for (auto it = _allInfos.begin(); it != _allInfos.end(); it++)
    {
        if ((requestOrigin == nullptr) || (requestOrigin->wasModulePreviouslyUsed(it->first.c_str())) || (it->first == ""))
        { // only if that item was previously required by that script
            std::string tip = it->second.funcs->getFunctionCalltip(txtNoVer.c_str());
            if (tip.size() != 0)
            {
                std::string modName;
                if (it->first != "")
                    modName = it->first;
                callTipsAndModuleNames.push_back(std::make_pair(tip, modName));
            }
        }
    }
    std::sort(callTipsAndModuleNames.begin(), callTipsAndModuleNames.end(), [](auto const& a, auto const& b)
    {
        return a.second < b.second;
    });
    std::string retVal;
    for (size_t i = 0; i < callTipsAndModuleNames.size(); i++)
    {
        if (allVers)
        {
            if (i != 0)
                retVal += "\n";
            retVal += "[" + callTipsAndModuleNames[i].second + "]  " + callTipsAndModuleNames[i].first;
        }
        else
        {
            if ( (verNs.size() == 0) && (i == callTipsAndModuleNames.size() - 1) )
                retVal += callTipsAndModuleNames[i].first; // latest version
            else if (verNs == callTipsAndModuleNames[i].second)
                retVal += callTipsAndModuleNames[i].first; // specific version
        }
    }
    return retVal;
}
