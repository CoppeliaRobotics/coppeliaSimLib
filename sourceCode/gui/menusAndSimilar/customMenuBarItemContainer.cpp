
#include "vrepMainHeader.h"
#include "customMenuBarItemContainer.h"
#include "global.h"

CCustomMenuBarItemContainer::CCustomMenuBarItemContainer()
{
    _nextSmallItemCommandID=ADDITIONAL_TOOL_MENU_ITEM_START_CMD;
    _menuHandle=nullptr;
}

CCustomMenuBarItemContainer::~CCustomMenuBarItemContainer()
{
    for (unsigned int i=0;i<allItems.size();i++)
        delete allItems[i];
}

bool CCustomMenuBarItemContainer::setItemState(int commandID,bool checked,bool enabled,const char* newLabel)
{
    for (unsigned int i=0;i<allItems.size();i++)
    {
        if (allItems[i]->setItemState(commandID,checked,enabled,newLabel))
            return(true);
    }
    return(false);
}

bool CCustomMenuBarItemContainer::addMenuBarItem(const char* title,int subItemCount,std::vector<int>& commandIDs)
{
    commandIDs.clear();
    if (subItemCount+_nextSmallItemCommandID-1>ADDITIONAL_TOOL_MENU_ITEM_END_CMD)
        return(false); // Too many!!
    CCustomMenuBarItem* it=nullptr;
    for (int i=0;i<int(allItems.size());i++)
    {
        if ( (allItems[i]->getMenuBarLabel()==title)&&(strlen(title)!=0) )
        {
            it=allItems[i];
            break;
        }
    }
    if (it==nullptr)
    { // That menu doesn't yet exist! We create it
        it=new CCustomMenuBarItem(title);
        allItems.push_back(it);
    }
    for (int i=0;i<subItemCount;i++)
    {
        commandIDs.push_back(_nextSmallItemCommandID);
        it->addItem(_nextSmallItemCommandID++);
    }
    return(true);
}

bool CCustomMenuBarItemContainer::addMenus(VMenu* myMenu)
{
    // From Qt this is only called if the custom menu was clicked (unlike in Windows)
    for (unsigned int i=0;i<allItems.size();i++)
        allItems[i]->addMenus(myMenu);
    return(true);
    return(false);
}

bool CCustomMenuBarItemContainer::processCommand(int commandID)
{ // Return value is true if the command belonged to object edition menu and was executed
    for (unsigned int i=0;i<allItems.size();i++)
    {
        if (allItems[i]->processCommand(commandID))
            return(true);
    }
    return(false);

}

