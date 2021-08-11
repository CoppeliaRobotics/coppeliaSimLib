#include "moduleMenuItem.h"
#include "app.h"
#include "oGL.h"
#include "simInternal.h"
#include "pluginContainer.h"

CModuleMenuItem::CModuleMenuItem(const char* menuBarText)
{
    _menuBarText=menuBarText;
}

CModuleMenuItem::~CModuleMenuItem()
{
}

void CModuleMenuItem::addItem(int cmdID)
{
    std::string tmp("");
    _labels.push_back(tmp);
    _states.push_back(0);
    _commandIDs.push_back(cmdID);
}

void CModuleMenuItem::addMenus(VMenu* myMenu)
{
    if (_menuBarText!="")
    {
        VMenu* prim=new VMenu();
        for (unsigned int i=0;i<_commandIDs.size();i++)
        {
            if (_labels[i]=="")
                prim->appendMenuSeparator();
            else
                prim->appendMenuItem((_states[i]&2)!=0,(_states[i]&1)!=0,_commandIDs[i],_labels[i].c_str(),(_states[i]&1)!=0);
        }
        myMenu->appendMenuAndDetach(prim,true,_menuBarText.c_str());
    }
    else
    {
        if (_commandIDs.size()!=0)
            myMenu->appendMenuItem((_states[0]&2)!=0,(_states[0]&1)!=0,_commandIDs[0],_labels[0].c_str(),(_states[0]&1)!=0);
    }
}

std::string CModuleMenuItem::getMenuBarLabel()
{
    return(_menuBarText);
}

int CModuleMenuItem::getItemCount()
{
    return(int(_commandIDs.size()));
}

void CModuleMenuItem::setCommandID(int index,int theCommand)
{
    _commandIDs[index]=theCommand;
}

bool CModuleMenuItem::setItemState(int commandID,bool checked,bool enabled,const char* newLabel)
{
    for (unsigned int i=0;i<_commandIDs.size();i++)
    {
        if (_commandIDs[i]==commandID)
        {
            if (newLabel!=nullptr)
                _labels[i]=newLabel;
            _states[i]=0;
            if (checked)
                _states[i]|=1;
            if (enabled)
                _states[i]|=2;
            App::setFullDialogRefreshFlag();
            return(true);
        }
    }
    return(false);
}

bool CModuleMenuItem::processCommand(int commandID)
{ // Return value is true if the command belonged to object edition menu and was executed
    for (unsigned int i=0;i<_commandIDs.size();i++)
    {
        if (_commandIDs[i]==commandID)
        { // We have to post a message here!!
            int data[4]={commandID,_states[i],0,0};
            void* returnVal=CPluginContainer::sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_menuitemselected,data,nullptr,nullptr);
            delete[] (char*)returnVal;
            return(true);
        }
    }
    return(false);
}




