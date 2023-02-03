#pragma once

#include <moduleMenuItem.h>

class CModuleMenuItemContainer  
{
public:
    CModuleMenuItemContainer();
    virtual ~CModuleMenuItemContainer();
    int addMenuItem(const char* item,int scriptHandle);
    void removeMenuItem(int h);
    CModuleMenuItem* getItemFromHandle(int h) const;
    size_t getItemCount() const;
    void announceScriptStateWillBeErased(int scriptHandle);

    bool processCommand(int commandID);

#ifdef SIM_WITH_GUI
    bool addMenus(VMenu* myMenu);
    VMenu* _menuHandle;
#endif

private:
    void _orderItems();

    std::vector<CModuleMenuItem*> _allItems;
};
