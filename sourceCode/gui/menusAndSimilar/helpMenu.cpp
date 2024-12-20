#include <simInternal.h>
#include <helpMenu.h>
#include <oGL.h>
#include <global.h>
#include <threadPool_old.h>
#include <algos.h>
#include <tt.h>
#include <app.h>
#include <simStrings.h>
#include <utils.h>
#include <vVarious.h>
#include <qdlgabout.h>
#include <QDesktopServices>
#include <QUrl>
#include <vMessageBox.h>
#include <collisionRoutines.h>
#include <distanceRoutines.h>
#include <simFlavor.h>
#ifdef SIM_WITH_GUI
#include <guiApp.h>
#endif

CHelpMenu::CHelpMenu()
{
}

CHelpMenu::~CHelpMenu()
{
}

void CHelpMenu::addMenu(VMenu* menu)
{
    std::string tmp;
    tmp = CSimFlavor::getStringVal(5);
    if (tmp.size() > 0)
        menu->appendMenuItem(true, false, HELP_TOPICS_CMD, tmp.c_str());
    tmp = CSimFlavor::getStringVal(6);
    if (tmp.size() > 0)
        menu->appendMenuItem(true, false, ABOUT_CMD, tmp.c_str());
    tmp = CSimFlavor::getStringVal(7);
    if (tmp.size() > 0)
        menu->appendMenuItem(true, false, CREDITS_CMD, tmp.c_str());
    VMenu* debugMenu = new VMenu();
    debugMenu->appendMenuItem(true, !CViewableBase::getFrustumCullingEnabled(), DISABLE_FRUSTUM_CULLING_DEBUG_CMD,
                              "Disable frustum culling", true);
    debugMenu->appendMenuItem(true, !CDistanceRoutine::getDistanceCachingEnabled(), DISABLE_DISTANCE_CACHING_DEBUG_CMD,
                              "Disable distance caching", true);
    menu->appendMenuAndDetach(debugMenu, true, "Debug");
    if (CSimFlavor::getBoolVal(13))
    {
        menu->appendMenuSeparator();
        menu->appendMenuItem(true, false, EK_CMD, CSimFlavor::getStringVal(8).c_str());
    }
}

bool CHelpMenu::processCommand(int commandID)
{ // Return value is true if the command belonged to help menu and was executed
    if (commandID == HELP_TOPICS_CMD)
    {

        if (((SIM_PROGRAM_REVISION_NB % 2) > 0) || (!GuiApp::isOnline()))
        {
            std::string tmp(App::folders->getResourcesPath() + "/manual/index.html");
            App::logMsg(sim_verbosity_msgs, "Opening the locally stored user manual...");
            VVarious::openOfflineUrl(tmp.c_str());
        }
        else
        {
            App::logMsg(sim_verbosity_msgs,
                        "Opening the online user manual at https://manual.coppeliarobotics.com/index.html... if "
                        "this fails, check the locally stored user manual.");
            VVarious::openOnlineUrl("https://manual.coppeliarobotics.com/index.html");
        }
        return (true);
    }
    if (commandID == ABOUT_CMD)
    {
        if (VThread::isUiThread())
        { // We are in the UI thread. Execute the command via the main thread:
            IF_UI_EVENT_CAN_WRITE_DATA
            {
                CQDlgAbout aboutBox(GuiApp::mainWindow);
                aboutBox.makeDialogModal();
            }
        }
        return (true);
    }
    if (commandID == CREDITS_CMD)
    {
        if (VThread::isUiThread())
        { // We are in the UI thread. Execute the command via the main thread:
            std::string tmp(App::folders->getResourcesPath() + "/credits.txt");
            if (VFile::doesFileExist(tmp.c_str()))
            { // FILE is present!
                VVarious::openTextFile(tmp.c_str());
            }
            else
            { // file doesn't exist.
                GuiApp::uiThread->messageBox_warning(GuiApp::mainWindow, IDSN_CREDITS, IDS_FILE_COULD_NOT_BE_FOUND_,
                                                     VMESSAGEBOX_OKELI, VMESSAGEBOX_REPLY_OK);
            }
        }
        return (true);
    }
    if (commandID == DISABLE_FRUSTUM_CULLING_DEBUG_CMD)
    {
        IF_UI_EVENT_CAN_READ_DATA
        {
            CViewableBase::setFrustumCullingEnabled(!CViewableBase::getFrustumCullingEnabled());
        }
        return (true);
    }
    if (commandID == DISABLE_DISTANCE_CACHING_DEBUG_CMD)
    {
        IF_UI_EVENT_CAN_READ_DATA
        {
            CDistanceRoutine::setDistanceCachingEnabled(!CDistanceRoutine::getDistanceCachingEnabled());
        }
        return (true);
    }
    if (CSimFlavor::getBoolVal(13) && (commandID == EK_CMD))
    {
        CSimFlavor::run(11);
        App::appendSimulationThreadCommand(PLUS_LR_CMD);
        return (true);
    }
    return (false);
}
