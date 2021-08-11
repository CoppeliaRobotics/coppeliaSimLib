
#pragma once

#include "moduleMenuItem.h"

class CModuleMenuItemContainer  
{
public:
    CModuleMenuItemContainer();
    virtual ~CModuleMenuItemContainer();
    bool addMenuBarItem(const char* title,int subItemCount,std::vector<int>& commandIDs);
    bool setItemState(int commandID,bool checked,bool enabled,const char* newLabel);
    bool addMenus(VMenu* myMenu);
    bool processCommand(int commandID);
    std::vector<CModuleMenuItem*> allItems;
    VMenu* _menuHandle;

private:
    int _nextSmallItemCommandID;
};
