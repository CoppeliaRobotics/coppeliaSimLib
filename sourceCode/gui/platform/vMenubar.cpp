
#include "vMenubar.h"
#include "app.h"

VMenu::VMenu()
{
    _qMenu=new QMenu(App::mainWindow);
}

VMenu::~VMenu()
{
    clear();
}

void VMenu::appendMenuAndDetach(VMenu* childMenu,bool enabled,const char* label)
{
    childMenu->_qMenu->setTitle(label);
    childMenu->_qMenu->setEnabled(enabled);
    _qMenu->addMenu(childMenu->_qMenu);
    _children.push_back(childMenu);
}

void VMenu::appendMenuItem(bool enabled,bool checkMark,int commandID,const char* label,bool showCheckmarkSpot)
{
    QAction* action=_qMenu->addAction(label);
    action->setEnabled(enabled);
    action->setCheckable(showCheckmarkSpot);
    action->setChecked(checkMark);
    App::mainWindow->connect(action,SIGNAL(triggered()),App::mainWindow->getPopupSignalMapper(),SLOT(map()));
    App::mainWindow->getPopupSignalMapper()->setMapping(action,commandID);
}

void VMenu::appendMenuSeparator()
{
    _qMenu->addSeparator();
}

int VMenu::trackPopupMenu()
{
    VMenubar::doNotExecuteCommandButMemorizeIt=true;
    _qMenu->exec(QCursor::pos());
    if (!VMenubar::doNotExecuteCommandButMemorizeIt)
        return(VMenubar::memorizedCommand);
    VMenubar::doNotExecuteCommandButMemorizeIt=false;
    return(-1);
}

QMenu* VMenu::getQMenu()
{
    return(_qMenu);
}

void VMenu::clear()
{
    for (size_t i=0;i<_children.size();i++)
        delete _children[i];
    _children.clear();

    _qMenu->clear();
}


int VMenubar::memorizedCommand=-1;
bool VMenubar::doNotExecuteCommandButMemorizeIt=false;

VMenubar::VMenubar()
{
    _qMenubar=App::mainWindow->menuBar();
    #ifdef MAC_SIM
        _qMenubar->setNativeMenuBar(false); // otherwise we have plenty of crashes!!! The result with this instruction is that the menu bar is not in the Mac top bar, but in the main window as in Windows!
    #endif
}

VMenubar::~VMenubar()
{
    clear();
}

void VMenubar::appendMenuAndDetach(VMenu* childMenu,bool enabled,const char* label)
{
    childMenu->getQMenu()->setTitle(label);
    childMenu->getQMenu()->setEnabled(enabled);
    _qMenubar->addMenu(childMenu->getQMenu());
    _children.push_back(childMenu);
}

void VMenubar::appendMenuItem(bool enabled,bool checkMark,int commandID,const char* label,bool showCheckmarkSpot)
{
    QAction* action=_qMenubar->addAction(label);
    action->setEnabled(enabled);
    action->setCheckable(showCheckmarkSpot);
    action->setChecked(checkMark);
    App::mainWindow->connect(action,SIGNAL(triggered()),App::mainWindow->getPopupSignalMapper(),SLOT(map()));
    App::mainWindow->getPopupSignalMapper()->setMapping(action,commandID);
}

void VMenubar::appendMenuSeparator()
{
    _qMenubar->addSeparator();
}

void VMenubar::clear()
{
    for (size_t i=0;i<_children.size();i++)
        delete _children[i];
    _children.clear();

    _qMenubar->clear();
}

