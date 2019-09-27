
#include "vrepMainHeader.h"
#include "v_rep_internal.h"
#include "toolBarCommand.h"
#include "simulation.h"
#include "oglSurface.h"
#include "threadPool.h"
#include "v_repStrings.h"
#include <boost/lexical_cast.hpp>
#include "app.h"

bool CToolBarCommand::processCommand(int commandID)
{ // Return value is true if the command belonged to toolbar menu and was executed
    if (commandID==CAMERA_SHIFT_TO_FRAME_SELECTION_CMD)
    {
        if ( (App::mainWindow!=nullptr)&&(!App::mainWindow->oglSurface->isScenePageOrViewSelectionActive()) )
        {
            if (!VThread::isCurrentThreadTheUiThread())
            { // we are NOT in the UI thread. We execute the command now:
                int pageIndex=App::ct->pageContainer->getActivePageIndex();
                CSPage* page=App::ct->pageContainer->getPage(pageIndex);
                if (page!=nullptr)
                {
                    int ind=page->getLastMouseDownViewIndex();
                    if (ind==-1)
                        ind=0;
                    CSView* view=page->getView(ind);
                    if (view!=nullptr)
                    {
                        CCamera* cam=App::ct->objCont->getCamera(view->getLinkedObjectID());
                        if ( (cam!=nullptr) )
                        {
                            int viewSize[2];
                            view->getViewSize(viewSize);

                            cam->frameSceneOrSelectedObjects(float(viewSize[0])/float(viewSize[1]),view->getPerspectiveDisplay(),nullptr,true,true,1.0f,view);
                            POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
                            cam->setFogTimer(4.0f);
                        }
                    }
                }
            }
            else
            { // We are in the UI thread. Execute the command via the main thread:
                SSimulationThreadCommand cmd;
                cmd.cmdId=commandID;
                App::appendSimulationThreadCommand(cmd);
            }
        }
        return(true);
    }
    if (commandID==CAMERA_SHIFT_NAVIGATION_CMD)
    {
        if (!App::mainWindow->oglSurface->isScenePageOrViewSelectionActive())
        {
            if (!VThread::isCurrentThreadTheUiThread())
            { // we are NOT in the UI thread. We execute the command now:
                App::setMouseMode((App::getMouseMode()&0xff00)|sim_navigation_camerashift);
            }
            else
            { // We are in the UI thread. Execute the command via the main thread:
                SSimulationThreadCommand cmd;
                cmd.cmdId=commandID;
                App::appendSimulationThreadCommand(cmd);
            }
        }
        return(true);
    }
    if (commandID==CAMERA_ROTATE_NAVIGATION_CMD)
    {
        if (!App::mainWindow->oglSurface->isScenePageOrViewSelectionActive())
        {
            if (!VThread::isCurrentThreadTheUiThread())
            { // we are NOT in the UI thread. We execute the command now:
                App::setMouseMode((App::getMouseMode()&0xff00)|sim_navigation_camerarotate);
            }
            else
            { // We are in the UI thread. Execute the command via the main thread:
                SSimulationThreadCommand cmd;
                cmd.cmdId=commandID;
                App::appendSimulationThreadCommand(cmd);
            }
        }
        return(true);
    }
    if (commandID==CAMERA_ZOOM_NAVIGATION_CMD)
    {
        if (!App::mainWindow->oglSurface->isScenePageOrViewSelectionActive())
        {
            if (!VThread::isCurrentThreadTheUiThread())
            { // we are NOT in the UI thread. We execute the command now:
                App::setMouseMode((App::getMouseMode()&0xff00)|sim_navigation_camerazoom);
            }
            else
            { // We are in the UI thread. Execute the command via the main thread:
                SSimulationThreadCommand cmd;
                cmd.cmdId=commandID;
                App::appendSimulationThreadCommand(cmd);
            }
        }
        return(true);
    }
    if (commandID==CAMERA_TILT_NAVIGATION_CMD)
    {
        if (!App::mainWindow->oglSurface->isScenePageOrViewSelectionActive())
        {
            if (!VThread::isCurrentThreadTheUiThread())
            { // we are NOT in the UI thread. We execute the command now:
                App::setMouseMode((App::getMouseMode()&0xff00)|sim_navigation_cameratilt);
            }
            else
            { // We are in the UI thread. Execute the command via the main thread:
                SSimulationThreadCommand cmd;
                cmd.cmdId=commandID;
                App::appendSimulationThreadCommand(cmd);
            }
        }
        return(true);
    }
    if (commandID==CAMERA_ANGLE_NAVIGATION_CMD)
    {
        if (!App::mainWindow->oglSurface->isScenePageOrViewSelectionActive())
        {
            if (!VThread::isCurrentThreadTheUiThread())
            { // we are NOT in the UI thread. We execute the command now:
                App::setMouseMode((App::getMouseMode()&0xff00)|sim_navigation_cameraangle);
            }
            else
            { // We are in the UI thread. Execute the command via the main thread:
                SSimulationThreadCommand cmd;
                cmd.cmdId=commandID;
                App::appendSimulationThreadCommand(cmd);
            }
        }
        return(true);
    }
    if (commandID==CAMERA_FLY_NAVIGATION_CMD)
    {
        if (!App::mainWindow->oglSurface->isScenePageOrViewSelectionActive())
        {
            if (!VThread::isCurrentThreadTheUiThread())
            { // we are NOT in the UI thread. We execute the command now:
                App::setMouseMode((App::getMouseMode()&0xff00)|sim_navigation_camerafly);
            }
            else
            { // We are in the UI thread. Execute the command via the main thread:
                SSimulationThreadCommand cmd;
                cmd.cmdId=commandID;
                App::appendSimulationThreadCommand(cmd);
            }
        }
        return(true);
    }
    if (commandID==OBJECT_SHIFT_NAVIGATION_CMD)
    {
        if (!App::mainWindow->oglSurface->isScenePageOrViewSelectionActive())
        {
            if (!VThread::isCurrentThreadTheUiThread())
            { // we are NOT in the UI thread. We execute the command now:
                App::setMouseMode((App::getMouseMode()&0xff00)|sim_navigation_objectshift);
            }
            else
            { // We are in the UI thread. Execute the command via the main thread:
                SSimulationThreadCommand cmd;
                cmd.cmdId=commandID;
                App::appendSimulationThreadCommand(cmd);
            }
        }
        return(true);
    }
    if (commandID==OBJECT_ROTATE_NAVIGATION_CMD)
    {
        bool rot=true;
        if ( (App::ct->objCont!=nullptr)&&(App::mainWindow!=nullptr) )
            rot=App::mainWindow->editModeContainer->pathPointManipulation->getSelectedPathPointIndicesSize_nonEditMode()==0;
        if ( (App::mainWindow!=nullptr)&&rot&&(!App::mainWindow->oglSurface->isScenePageOrViewSelectionActive()) )
        {
            if (!VThread::isCurrentThreadTheUiThread())
            { // we are NOT in the UI thread. We execute the command now:
                App::setMouseMode((App::getMouseMode()&0xff00)|sim_navigation_objectrotate);
            }
            else
            { // We are in the UI thread. Execute the command via the main thread:
                SSimulationThreadCommand cmd;
                cmd.cmdId=commandID;
                App::appendSimulationThreadCommand(cmd);
            }
        }
        return(true);
    }
    
    if (commandID==PAGE_SELECTOR_CMD)
    {
        if (App::mainWindow!=nullptr)
        {
            if (!VThread::isCurrentThreadTheUiThread())
            { // we are NOT in the UI thread. We execute the command now:
                if (App::mainWindow->oglSurface->isPageSelectionActive())
                    App::mainWindow->oglSurface->setPageSelectionActive(false);
                else
                    App::mainWindow->oglSurface->setPageSelectionActive(true);
            }
            else
            { // We are in the UI thread. Execute the command via the main thread:
                SSimulationThreadCommand cmd;
                cmd.cmdId=commandID;
                App::appendSimulationThreadCommand(cmd);
            }
        }
        return(true);
    }
    //-----------
    if (commandID==SCENE_SELECTOR_CMD)
    {
        SSimulationThreadCommand cmd;
        cmd.cmdId=SCENE_SELECTOR_PHASE2_CMD;
        if ( (App::mainWindow!=nullptr)&&(!App::userSettings->doNotShowSceneSelectionThumbnails) )
        {
            if (!App::mainWindow->oglSurface->isSceneSelectionActive())
                App::mainWindow->prepareSceneThumbnail(cmd);
            else
                App::appendSimulationThreadCommand(cmd);
        }
        else
            App::appendSimulationThreadCommand(cmd);
        return(true);
    }
    if (commandID==SCENE_SELECTOR_PHASE2_CMD)
    {
        if (App::mainWindow!=nullptr)
        {
            if (!VThread::isCurrentThreadTheUiThread())
            { // we are NOT in the UI thread. We execute the command now:
                if (App::mainWindow->oglSurface->isSceneSelectionActive())
                    App::mainWindow->oglSurface->setSceneSelectionActive(false);
                else
                    App::mainWindow->oglSurface->setSceneSelectionActive(true);
            }
            else
            { // We are in the UI thread. Execute the command via the main thread:
                SSimulationThreadCommand cmd;
                cmd.cmdId=commandID;
                App::appendSimulationThreadCommand(cmd);
            }
        }
        return(true);
    }
    //-----------

    if (commandID==OBJECT_SELECTION_SELECTION_CMD)
    {
        if ( (App::mainWindow!=nullptr)&&(!App::mainWindow->oglSurface->isScenePageOrViewSelectionActive()) )
        {
            if (!VThread::isCurrentThreadTheUiThread())
            { // we are NOT in the UI thread. We execute the command now:
                if ( (App::getMouseMode()&0x0300)&sim_navigation_clickselection )
                    App::setMouseMode(App::getMouseMode()&0xfcff);
                else
                    App::setMouseMode((App::getMouseMode()&0xfcff)|sim_navigation_clickselection);
            }
            else
            { // We are in the UI thread. Execute the command via the main thread:
                SSimulationThreadCommand cmd;
                cmd.cmdId=commandID;
                App::appendSimulationThreadCommand(cmd);
            }
        }
        return(true);
    }
    if (commandID==PATH_POINT_CREATION_MODE_CMD)
    {
        if ( (App::mainWindow!=nullptr)&&(!App::mainWindow->oglSurface->isScenePageOrViewSelectionActive()) )
        {
            if (!VThread::isCurrentThreadTheUiThread())
            { // we are NOT in the UI thread. We execute the command now:
                if ( (App::getMouseMode()&0x0300)&sim_navigation_createpathpoint )
                    App::setMouseMode(App::getMouseMode()&0xfcff);
                else
                    App::setMouseMode((App::getMouseMode()&0xfcff)|sim_navigation_createpathpoint);
            }
            else
            { // We are in the UI thread. Execute the command via the main thread:
                SSimulationThreadCommand cmd;
                cmd.cmdId=commandID;
                App::appendSimulationThreadCommand(cmd);
            }
        }
        return(true);
    }

    if (commandID==CLEAR_SELECTION_SELECTION_CMD)
    {
        if ( (App::mainWindow!=nullptr)&&(!App::mainWindow->oglSurface->isScenePageOrViewSelectionActive()) )
        {
            if (!VThread::isCurrentThreadTheUiThread())
            { // we are NOT in the UI thread. We execute the command now:
                App::ct->objCont->deselectObjects();
                App::mainWindow->editModeContainer->deselectEditModeBuffer();
            }
            else
            { // We are in the UI thread. Execute the command via the main thread:
                SSimulationThreadCommand cmd;
                cmd.cmdId=commandID;
                App::appendSimulationThreadCommand(cmd);
            }
        }
        return(true);
    }
    if ( (commandID==SIMULATION_COMMANDS_START_RESUME_SIMULATION_REQUEST_SCCMD)||(commandID==SIMULATION_COMMANDS_PAUSE_SIMULATION_REQUEST_SCCMD)||(commandID==SIMULATION_COMMANDS_STOP_SIMULATION_REQUEST_SCCMD) )
    {
        if ( (App::mainWindow!=nullptr)&&(!App::mainWindow->oglSurface->isScenePageOrViewSelectionActive()) )
            App::ct->simulation->processCommand(commandID);
        return(true);
    }

    if ( (commandID>=VIEW_1_CMD)&&(commandID<=VIEW_8_CMD) )
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            if (App::ct->pageContainer->getActivePageIndex()!=(commandID-VIEW_1_CMD))
            {
                App::ct->pageContainer->setActivePage(commandID-VIEW_1_CMD);
                POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
                std::string str(IDSNS_SWAPPED_TO_PAGE);
                str+=" ";
                str+=boost::lexical_cast<std::string>(commandID-VIEW_1_CMD+1)+".";
                App::addStatusbarMessage(str.c_str());
            }
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }
    return(false);
}
