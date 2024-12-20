#pragma once

#include <string>
#include <map>
#include <set>

class CCodeEditorFunctions
{
  public:
    CCodeEditorFunctions();
    virtual ~CCodeEditorFunctions();

    bool set(const char* info);
    void print() const;
    void clear();

    void insertWhatStartsSame(const char* txt, std::set<std::string>& v) const;
    std::string getFunctionCalltip(const char* txt) const;

  protected:
    std::map<std::string, std::string> _allFuncs;
};
