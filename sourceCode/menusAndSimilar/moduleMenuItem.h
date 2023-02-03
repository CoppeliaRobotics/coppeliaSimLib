#pragma once

#include <string>
#include <vector>
#ifdef SIM_WITH_GUI
#include <vMenubar.h>
#endif

class CModuleMenuItem  
{
public:
    CModuleMenuItem(const char* item,int scriptHandle);
    virtual ~CModuleMenuItem();

    void setHandle(int h);
    int getHandle() const;
    std::string getPath() const;
    void setState(int s);
    int getState() const;
    std::string getLabel() const;
    void setLabel(const char* label);
    int getScriptHandle() const;

#ifdef SIM_WITH_GUI
    void addMenu(std::vector<VMenu*>& menus,std::vector<std::string>& labels);
#endif

private:
    std::string _path;
    std::string _label;
    int _state; // bit0=enabled, bit1=checked, bit2=check box present
    int _handle;
    int _scriptHandle;
};
