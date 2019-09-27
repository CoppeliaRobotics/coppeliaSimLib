
#include "vrepMainHeader.h"
#include "funcDebug.h"
#include "easyLock.h"
#include "v_rep_internal.h"
#include "helpMenu.h"
#include "oGL.h"
#include "global.h"
#include "threadPool.h"
#include "algos.h"
#include "tt.h"
#include "app.h"
#include "v_repStrings.h"
#include "ttUtil.h"
#include "vVarious.h"
#include "qdlgabout.h"
#include <QDesktopServices>
#include <QUrl>
#include "debugLogFile.h"
#include "vMessageBox.h"
#include "collisionRoutine.h"
#include "distanceRoutine.h"
#include "helpMenuBase.h"

CHelpMenu::CHelpMenu()
{

}

CHelpMenu::~CHelpMenu()
{

}

void CHelpMenu::addMenu(VMenu* menu)
{
    CHelpMenuBase::handleVerSpec_addMenu1(menu);
    if (CHelpMenuBase::handleVerSpec_addMenu2())
    {
        VMenu* debugMenu=new VMenu();
        debugMenu->appendMenuItem(true,CFuncDebug::getDebugMask()&1,SHOW_INTERNAL_FUNCTION_ACCESS_DEBUG_CMD,IDSN_SHOW_INTERNAL_FUNCTION_ACCESS_DEBUG_MENU_ITEM,true);
        debugMenu->appendMenuItem(true,CFuncDebug::getDebugMask()&2,SHOW_C_API_ACCESS_DEBUG_CMD,IDSN_SHOW_C_API_ACCESS_DEBUG_MENU_ITEM,true);
        debugMenu->appendMenuItem(true,CFuncDebug::getDebugMask()&4,SHOW_LUA_API_ACCESS_DEBUG_CMD,IDSN_SHOW_LUA_API_ACCESS_DEBUG_MENU_ITEM,true);
        debugMenu->appendMenuItem(true,CDebugLogFile::getDebugToFile(),DEBUG_TO_FILE_DEBUG_CMD,IDSN_SEND_DEBUG_INFO_TO_FILE_MENU_ITEM,true);
        debugMenu->appendMenuSeparator();
        debugMenu->appendMenuItem(true,!CViewableBase::getFrustumCullingEnabled(),DISABLE_FRUSTUM_CULLING_DEBUG_CMD,IDSN_DISABLE_FRUSTUM_CULLING_DEBUG_MENU_ITEM,true);
        debugMenu->appendMenuItem(true,!CDistanceRoutine::getDistanceCachingEnabled(),DISABLE_DISTANCE_CACHING_DEBUG_CMD,IDSN_DISABLE_DISTANCE_CACHING_DEBUG_MENU_ITEM,true);
        debugMenu->appendMenuItem(true,CShape::getDebugObbStructures(),VISUALIZE_OBB_STRUCTURE_DEBUG_CMD,IDSN_VISUALIZE_OBB_STRUCTURE_DEBUG_MENU_ITEM,true);
        menu->appendMenuAndDetach(debugMenu,true,IDSN_DEBUG_MENU_ITEM);
    }
    CHelpMenuBase::handleVerSpec_addMenu3(menu);
}

bool CHelpMenu::processCommand(int commandID)
{ // Return value is true if the command belonged to help menu and was executed
    if (commandID==HELP_TOPICS_CMD)
    {
        if (VThread::isCurrentThreadTheUiThread())
        { // We are in the UI thread. Execute the command via the main thread:
            #ifdef MAC_VREP
                std::string tmp(App::directories->executableDirectory+"/../../../"+"helpFiles"+VREP_SLASH+"index.html");
            #else
                std::string tmp(App::directories->executableDirectory+VREP_SLASH+"helpFiles"+VREP_SLASH+"index.html");
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
            #ifdef MAC_VREP
                std::string tmp(App::directories->executableDirectory+"/../../../"+"credits.txt");
            #else
                std::string tmp(App::directories->executableDirectory+VREP_SLASH+"credits.txt");
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
    if (commandID==SHOW_INTERNAL_FUNCTION_ACCESS_DEBUG_CMD)
    {
        IF_UI_EVENT_CAN_READ_DATA_CMD("SHOW_INTERNAL_FUNCTION_ACCESS_DEBUG_CMD")
        {
            CFuncDebug::setDebugMask(CFuncDebug::getDebugMask()^1);
            App::userSettings->saveUserSettings();
        }
        return(true);
    }
    if (commandID==SHOW_C_API_ACCESS_DEBUG_CMD)
    {
        IF_UI_EVENT_CAN_READ_DATA_CMD("SHOW_C_API_ACCESS_DEBUG_CMD")
        {
            CFuncDebug::setDebugMask(CFuncDebug::getDebugMask()^2);
            App::userSettings->saveUserSettings();
        }
        return(true);
    }
    if (commandID==SHOW_LUA_API_ACCESS_DEBUG_CMD)
    {
        IF_UI_EVENT_CAN_READ_DATA_CMD("SHOW_LUA_API_ACCESS_DEBUG_CMD")
        {
            CFuncDebug::setDebugMask(CFuncDebug::getDebugMask()^4);
            App::userSettings->saveUserSettings();
        }
        return(true);
    }
    if (commandID==DEBUG_TO_FILE_DEBUG_CMD)
    {
        IF_UI_EVENT_CAN_READ_DATA_CMD("DEBUG_TO_FILE_DEBUG_CMD")
        {
            CDebugLogFile::setDebugToFile(!CDebugLogFile::getDebugToFile());
            App::userSettings->saveUserSettings();
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
    if (CHelpMenuBase::handleVerSpec_processCommand1(commandID))
        return(true);
    return(false);
}
