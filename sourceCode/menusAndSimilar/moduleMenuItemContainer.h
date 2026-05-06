#pragma once

#include <moduleMenuItem.h>

class CModuleMenuItemContainer
{
  public:
    CModuleMenuItemContainer();
    virtual ~CModuleMenuItemContainer();
    int addMenuItem(const char* item, int detachedScriptHandle);
    void removeMenuItem(int h);
    CModuleMenuItem* getItemFromHandle(int h) const;
    size_t getItemCount() const;
    void announceScriptStateWillBeErased(int detachedScriptHandle);

  private:
    void _orderItems();

    std::vector<CModuleMenuItem*> _allItems;

#ifdef SIM_WITH_GUI
  public:
    bool processCommand(int commandID);
    bool addMenus(VMenu* myMenu);
    VMenu* _menuHandle;
#endif
};
