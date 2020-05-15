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
#include "libLic.h"

CHelpMenu::CHelpMenu()
{

}

CHelpMenu::~CHelpMenu()
{

}

void CHelpMenu::addMenu(VMenu* menu)
{
    std::string tmp;
    tmp=CLibLic::getStringVal(5);
    if (tmp.size()>0)
        menu->appendMenuItem(true,false,HELP_TOPICS_CMD,tmp.c_str());
    tmp=CLibLic::getStringVal(6);
    if (tmp.size()>0)
        menu->appendMenuItem(true,false,ABOUT_CMD,tmp.c_str());
    tmp=CLibLic::getStringVal(7);
    if (tmp.size()>0)
        menu->appendMenuItem(true,false,CREDITS_CMD,tmp.c_str());
    if (CLibLic::getBoolVal(11))
    {
        VMenu* debugMenu=new VMenu();
        debugMenu->appendMenuItem(true,!CViewableBase::getFrustumCullingEnabled(),DISABLE_FRUSTUM_CULLING_DEBUG_CMD,IDSN_DISABLE_FRUSTUM_CULLING_DEBUG_MENU_ITEM,true);
        debugMenu->appendMenuItem(true,!CDistanceRoutine::getDistanceCachingEnabled(),DISABLE_DISTANCE_CACHING_DEBUG_CMD,IDSN_DISABLE_DISTANCE_CACHING_DEBUG_MENU_ITEM,true);
        debugMenu->appendMenuItem(true,CShape::getDebugObbStructures(),VISUALIZE_OBB_STRUCTURE_DEBUG_CMD,IDSN_VISUALIZE_OBB_STRUCTURE_DEBUG_MENU_ITEM,true);
        menu->appendMenuAndDetach(debugMenu,true,IDSN_DEBUG_MENU_ITEM);
    }
    if ( CLibLic::getBoolVal(13)&&(!CLibLic::getBoolVal(14)) )
    {
        menu->appendMenuSeparator();
        menu->appendMenuItem(true,false,EK_CMD,CLibLic::getStringVal(8).c_str());
    }
}

bool CHelpMenu::processCommand(int commandID)
{ // Return value is true if the command belonged to help menu and was executed
    if (commandID==HELP_TOPICS_CMD)
    {
        if (VThread::isCurrentThreadTheUiThread())
        { // We are in the UI thread. Execute the command via the main thread:
            #ifdef MAC_SIM
                std::string tmp(App::directories->executableDirectory+"/../../../"+"helpFiles"+"/"+"index.html");
            #else
                std::string tmp(App::directories->executableDirectory+"/"+"helpFiles"+"/"+"index.html");
            #endif
            VVarious::openUrl(tmp);
        }
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
            #ifdef MAC_SIM
                std::string tmp(App::directories->executableDirectory+"/../../../"+"credits.txt");
            #else
                std::string tmp(App::directories->executableDirectory+"/"+"credits.txt");
            #endif
            if (VFile::doesFileExist(tmp))
            { // FILE is present!
                VVarious::openTextFile(tmp);
            }
            else
            { // file doesn't exist.
                App::uiThread->messageBox_warning(App::mainWindow,strTranslate(IDSN_CREDITS),strTranslate(IDS_FILE_COULD_NOT_BE_FOUND_),VMESSAGEBOX_OKELI);
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
    if ( CLibLic::getBoolVal(13)&&(commandID==EK_CMD) )
    {
        CLibLic::setHld(App::mainWindow);
        CLibLic::ekd();
        return(true);
    }
    return(false);
}
