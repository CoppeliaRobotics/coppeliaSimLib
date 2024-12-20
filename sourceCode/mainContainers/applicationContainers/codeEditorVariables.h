#pragma once

#include <string>
#include <vector>
#include <set>

class CCodeEditorVariables
{
  public:
    CCodeEditorVariables();
    virtual ~CCodeEditorVariables();

    bool set(const char* info);
    void print() const;
    void clear();

    void insertWhatStartsSame(const char* txt, std::set<std::string>& v) const;

  protected:
    std::vector<std::string> _allVars;
};
