#pragma once

#include <string>
#include <vector>
#include <map>

struct SPluginVariable
{
    std::string varName;
    std::string varValue;
    int stackHandle;
};

class CPluginVariableContainer
{
  public:
    CPluginVariableContainer();
    virtual ~CPluginVariableContainer();

    SPluginVariable* getVariableFromName(const char* name);
    SPluginVariable* getVariableFromIndex(size_t ind);

    void clear();
    bool addVariable(const char* varName, const char* varValue, int stackHandle = -1);

  protected:
    std::vector<SPluginVariable> _allVariables_vect;
    std::map<std::string, size_t> _allVariables_map;
};
