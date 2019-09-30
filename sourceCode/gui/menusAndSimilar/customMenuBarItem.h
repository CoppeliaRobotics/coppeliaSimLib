
#pragma once

#include "vMenubar.h"

class CCustomMenuBarItem  
{
public:
    CCustomMenuBarItem(const char* menuBarText);
    virtual ~CCustomMenuBarItem();
    std::string getMenuBarLabel();
    void addItem(int cmdID);
    bool setItemState(int commandID,bool checked,bool enabled,const char* newLabel);
    void setCommandID(int index,int theCommand);
    int getItemCount();
    void addMenus(VMenu* menu);
    bool processCommand(int commandID);

private:
    std::string _menuBarText;
    std::vector<std::string> _labels; // Empty labels are separators! 
    std::vector<unsigned char> _states; // bit0=checked, bit1=enabled
    std::vector<int> _commandIDs; 
};
