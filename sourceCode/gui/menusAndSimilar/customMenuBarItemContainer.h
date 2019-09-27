
#pragma once

#include "vrepMainHeader.h"
#include "customMenuBarItem.h"

class CCustomMenuBarItemContainer  
{
public:
    CCustomMenuBarItemContainer();
    virtual ~CCustomMenuBarItemContainer();
    bool addMenuBarItem(const char* title,int subItemCount,std::vector<int>& commandIDs);
    bool setItemState(int commandID,bool checked,bool enabled,const char* newLabel);
    bool addMenus(VMenu* myMenu);
    bool processCommand(int commandID);
    std::vector<CCustomMenuBarItem*> allItems;
    VMenu* _menuHandle;

private:
    int _nextSmallItemCommandID;
};
