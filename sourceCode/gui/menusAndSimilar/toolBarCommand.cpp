#include "simInternal.h"
#include "toolBarCommand.h"
#include "simulation.h"
#include "oglSurface.h"
#include "threadPool_old.h"
#include "simStrings.h"
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
                int pageIndex=App::currentWorld->pageContainer->getActivePageIndex();
                CSPage* page=App::currentWorld->pageContainer->getPage(pageIndex);
                if (page!=nullptr)
                {
                    int ind=page->getLastMouseDownViewIndex();
                    if (ind==-1)
                        ind=0;
                    CSView* view=page->getView(size_t(ind));
                    if (view!=nullptr)
                    {
                        CCamera* cam=App::currentWorld->sceneObjects->getCameraFromHandle(view->getLinkedObjectID());
                        if ( (cam!=nullptr) )
                        {
                            int viewSize[2];
                            view->getViewSize(viewSize);

                            cam->frameSceneOrSelectedObjects(float(viewSize[0])/float(viewSize[1]),view->getPerspectiveDisplay(),nullptr,true,true,1.0,view);
                            App::undoRedo_sceneChanged(""); // ************************** UNDO thingy **************************
                            cam->setFogTimer(4.0);
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
        if ( (App::currentWorld->sceneObjects!=nullptr)&&(App::mainWindow!=nullptr) )
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
                App::currentWorld->sceneObjects->deselectObjects();
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
            App::currentWorld->simulation->processCommand(commandID);
        return(true);
    }

    if ( (commandID>=VIEW_1_CMD)&&(commandID<=VIEW_8_CMD) )
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            if (App::currentWorld->pageContainer->getActivePageIndex()!=(commandID-VIEW_1_CMD))
            {
                App::currentWorld->pageContainer->setActivePage(commandID-VIEW_1_CMD);
                App::undoRedo_sceneChanged(""); // ************************** UNDO thingy **************************
                std::string str(IDSNS_SWAPPED_TO_PAGE);
                str+=" ";
                str+=boost::lexical_cast<std::string>(commandID-VIEW_1_CMD+1)+".";
                App::logMsg(sim_verbosity_msgs,str.c_str());
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
