#include <app.h>
#include <moduleMenuItemContainer.h>
#include <global.h>
#ifdef SIM_WITH_GUI
#include <guiApp.h>
#endif

CModuleMenuItemContainer::CModuleMenuItemContainer()
{
#ifdef SIM_WITH_GUI
    _menuHandle = nullptr;
#endif
}

CModuleMenuItemContainer::~CModuleMenuItemContainer()
{
    for (size_t i = 0; i < _allItems.size(); i++)
        delete _allItems[i];
}

CModuleMenuItem* CModuleMenuItemContainer::getItemFromHandle(int h) const
{
    CModuleMenuItem* retVal = nullptr;
    for (size_t i = 0; i < _allItems.size(); i++)
    {
        if (_allItems[i]->getHandle() == h)
            return (_allItems[i]);
    }
    return (nullptr);
}

size_t CModuleMenuItemContainer::getItemCount() const
{
    return (_allItems.size());
}

void CModuleMenuItemContainer::removeMenuItem(int h)
{
    for (size_t i = 0; i < _allItems.size(); i++)
    {
        if (_allItems[i]->getHandle() == h)
        {
            delete _allItems[i];
            _allItems.erase(_allItems.begin() + i);
            return;
        }
    }
}

void CModuleMenuItemContainer::announceScriptStateWillBeErased(int detachedScriptHandle)
{
    for (size_t i = 0; i < _allItems.size(); i++)
    {
        if (_allItems[i]->getDetachedScriptHandle() == detachedScriptHandle)
        {
            removeMenuItem(_allItems[i]->getHandle());
            i--;
        }
    }
}

int CModuleMenuItemContainer::addMenuItem(const char* item, int detachedScriptHandle)
{
    int h = UI_MODULE_MENU_CMDS_START;
    while (getItemFromHandle(h) != nullptr)
        h++;
    CModuleMenuItem* it = new CModuleMenuItem(item, detachedScriptHandle);
    it->setHandle(h);
    _allItems.push_back(it);
    return h;
}

void CModuleMenuItemContainer::_orderItems()
{
    while (true)
    {
        bool swapped = false;
        for (size_t i = 0; i < _allItems.size() - 1; i++)
        {
            if (_allItems[i]->getPath().compare(_allItems[i + 1]->getPath()) > 0)
            {
                std::iter_swap(_allItems.begin() + i, _allItems.begin() + i + 1);
                swapped = true;
            }
        }
        if (!swapped)
            break;
    }
}

#ifdef SIM_WITH_GUI
bool CModuleMenuItemContainer::processCommand(int commandID)
{
    for (size_t i = 0; i < _allItems.size(); i++)
    {
        CModuleMenuItem* it = _allItems[i];
        if ((it->getHandle() == commandID) && (it->getDetachedScriptHandle() != -1))
        {
            SSimulationThreadCommand cmd;
            cmd.cmdId = CALL_MODULE_ENTRY_CMD;
            cmd.intParams.push_back(it->getDetachedScriptHandle());
            cmd.intParams.push_back(commandID);
            App::appendSimulationThreadCommand(cmd);
            return (true);
        }
    }

    if (App::scenes->addOnScriptContainer->processCommand(commandID))
        return (true);
    else
    {
        for (size_t i = 0; i < _allItems.size(); i++)
        {
            CModuleMenuItem* it = _allItems[i];
            if (it->getHandle() == commandID)
            {
                int data[4] = {commandID, it->getState(), 0, 0};
                App::scenes->pluginContainer->sendEventCallbackMessageToAllPlugins_old(
                    sim_message_eventcallback_menuitemselected, data);

                App::scenes->pluginContainer->uiCallAllPlugins(sim_message_eventcallback_menuitemselected,
                                                                       data);
                return (true);
            }
        }
    }
    return (false);
}

bool CModuleMenuItemContainer::addMenus(VMenu* myMenu)
{
    _orderItems();
    std::vector<VMenu*> m;
    m.push_back(myMenu);
    std::vector<std::string> l;
    l.push_back("0");
    for (size_t i = 0; i < _allItems.size(); i++)
        _allItems[i]->addMenu(m, l);
    return (true);
}
#endif
