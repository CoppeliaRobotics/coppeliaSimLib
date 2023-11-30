#pragma once

#include <string>
#include <map>
#include <set>
#include <codeEditorFunctions.h>
#include <codeEditorVariables.h>
#include <scriptObject.h>

struct SCodeEditorInfo
{
    CCodeEditorFunctions *funcs;
    CCodeEditorVariables *vars;
};

class CCodeEditorInfos
{
  public:
    CCodeEditorInfos();
    virtual ~CCodeEditorInfos();

    SCodeEditorInfo *getInfoFromNamespace(const char *ns);
    void setInfo(const char *namespaceAndVersion, const char *info, std::string *errorString = nullptr);
    void removeInfo(const char *ns);

    void insertWhatStartsSame(const char *txt, std::set<std::string> &v, int what,
                              const CScriptObject *requestOrigin) const;
    std::string getFunctionCalltip(const char *txt, const CScriptObject *requestOrigin) const;

    void clear();

  protected:
    std::map<std::string, SCodeEditorInfo> _allInfos;
};
