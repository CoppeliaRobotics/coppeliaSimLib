#include "easyLock.h"
#include "simInternal.h"
#include "helpMenu.h"
#include "oGL.h"
#include "global.h"
#include "threadPool.h"
#include "algos.h"
#include "tt.h"
#include "app.h"
#include "simStrings.h"
#include "ttUtil.h"
#include "vVarious.h"
#include "qdlgabout.h"
#include <QDesktopServices>
#include <QUrl>
#include "vMessageBox.h"
#include "collisionRoutines.h"
#include "distanceRoutines.h"
#include "simFlavor.h"
#include <QHostInfo>

CHelpMenu::CHelpMenu()
{

}

CHelpMenu::~CHelpMenu()
{

}

void CHelpMenu::addMenu(VMenu* menu)
{
    std::string tmp;
    tmp=CSimFlavor::getStringVal(5);
    if (tmp.size()>0)
        menu->appendMenuItem(true,false,HELP_TOPICS_CMD,tmp.c_str());
    tmp=CSimFlavor::getStringVal(6);
    if (tmp.size()>0)
        menu->appendMenuItem(true,false,ABOUT_CMD,tmp.c_str());
    tmp=CSimFlavor::getStringVal(7);
    if (tmp.size()>0)
        menu->appendMenuItem(true,false,CREDITS_CMD,tmp.c_str());
    if (CSimFlavor::getBoolVal(11))
    {
        VMenu* debugMenu=new VMenu();
        debugMenu->appendMenuItem(true,!CViewableBase::getFrustumCullingEnabled(),DISABLE_FRUSTUM_CULLING_DEBUG_CMD,IDSN_DISABLE_FRUSTUM_CULLING_DEBUG_MENU_ITEM,true);
        debugMenu->appendMenuItem(true,!CDistanceRoutine::getDistanceCachingEnabled(),DISABLE_DISTANCE_CACHING_DEBUG_CMD,IDSN_DISABLE_DISTANCE_CACHING_DEBUG_MENU_ITEM,true);
        debugMenu->appendMenuItem(true,CShape::getDebugObbStructures(),VISUALIZE_OBB_STRUCTURE_DEBUG_CMD,IDSN_VISUALIZE_OBB_STRUCTURE_DEBUG_MENU_ITEM,true);
        menu->appendMenuAndDetach(debugMenu,true,IDSN_DEBUG_MENU_ITEM);
    }
    if ( CSimFlavor::getBoolVal(13)&&(!CSimFlavor::getBoolVal(14)) )
    {
        menu->appendMenuSeparator();
        menu->appendMenuItem(true,false,EK_CMD,CSimFlavor::getStringVal(8).c_str());
    }
}

bool CHelpMenu::processCommand(int commandID)
{ // Return value is true if the command belonged to help menu and was executed
    if (commandID==HELP_TOPICS_CMD)
    {
        std::string tmp("https://coppeliarobotics.com/helpFiles/index.html");
        if ( ((SIM_PROGRAM_REVISION_NB)==0) || (QHostInfo::fromName("coppeliarobotics.com").error()!=QHostInfo::NoError) )
        {
            #ifdef MAC_SIM
                tmp=App::folders->getExecutablePath()+"/../Resources/"+"helpFiles"+"/"+"index.html";
            #else
                tmp=App::folders->getExecutablePath()+"/"+"helpFiles"+"/"+"index.html";
            #endif
        }
        VVarious::openUrl(tmp.c_str());
        return(true);
    }
    if (commandID==ABOUT_CMD)
    {
        if (VThread::isCurrentThreadTheUiThread())
        { // We are in the UI thread. Execute the command via the main thread:
            IF_UI_EVENT_CAN_WRITE_DATA
            {
                CQDlgAbout aboutBox(App::mainWindow);
                aboutBox.makeDialogModal();
            }
        }
        return(true);
    }
    if (commandID==CREDITS_CMD)
    {
        if (VThread::isCurrentThreadTheUiThread())
        { // We are in the UI thread. Execute the command via the main thread:
            std::string tmp(App::folders->getResourcesPath()+"/credits.txt");
            if (VFile::doesFileExist(tmp.c_str()))
            { // FILE is present!
                VVarious::openTextFile(tmp.c_str());
            }
            else
            { // file doesn't exist.
                App::uiThread->messageBox_warning(App::mainWindow,IDSN_CREDITS,IDS_FILE_COULD_NOT_BE_FOUND_,VMESSAGEBOX_OKELI,VMESSAGEBOX_REPLY_OK);
            }
        }
        return(true);
    }
    if (commandID==DISABLE_FRUSTUM_CULLING_DEBUG_CMD)
    {
        IF_UI_EVENT_CAN_READ_DATA_CMD("DISABLE_FRUSTUM_CULLING_DEBUG_CMD")
        {
            CViewableBase::setFrustumCullingEnabled(!CViewableBase::getFrustumCullingEnabled());
        }
        return(true);
    }
    if (commandID==DISABLE_DISTANCE_CACHING_DEBUG_CMD)
    {
        IF_UI_EVENT_CAN_READ_DATA_CMD("DISABLE_DISTANCE_CACHING_DEBUG_CMD")
        {
            CDistanceRoutine::setDistanceCachingEnabled(!CDistanceRoutine::getDistanceCachingEnabled());
        }
        return(true);
    }
    if (commandID==VISUALIZE_OBB_STRUCTURE_DEBUG_CMD)
    {
        IF_UI_EVENT_CAN_READ_DATA_CMD("VISUALIZE_OBB_STRUCTURE_DEBUG_CMD")
        {
            CShape::setDebugObbStructures(!CShape::getDebugObbStructures());
        }
        return(true);
    }
    if ( CSimFlavor::getBoolVal(13)&&(commandID==EK_CMD) )
    {
        CSimFlavor::setHld(App::mainWindow);
        CSimFlavor::ekd();
        return(true);
    }
    return(false);
}
