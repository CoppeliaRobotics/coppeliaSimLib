
#include "vrepMainHeader.h"
#include "funcDebug.h"
#include "simThread.h"
#include "vThread.h"
#include "app.h"
#include "fileOperations.h"
#include "v_repStringTable.h"
#include "tt.h"
#include "vDateTime.h"
#include "proxSensorRoutine.h"
#include "sceneObjectOperations.h"
#include "fileOperations.h"
#include "addOperations.h"
#include "pluginContainer.h"
#include "ttUtil.h"
#include "vVarious.h"
#include "easyLock.h"
#include "geometric.h"
#include "threadPool.h"
#include "volInt.h"
#include "geometricConstraintSolverInt.h"
#include "graphingRoutines.h"
#include "v_repStringTable_openGl.h"
#ifdef SIM_WITH_GUI
    #include "toolBarCommand.h"
    #include "vMessageBox.h"
#endif

CSimThread::CSimThread()
{
    _renderingAllowed=true;
}

CSimThread::~CSimThread()
{
}

void CSimThread::setRenderingAllowed(bool a)
{
    _renderingAllowed=a;
}

void CSimThread::executeMessages()
{
    FUNCTION_DEBUG;
#ifndef SIM_WITHOUT_QT_AT_ALL
#ifdef SIM_WITH_GUI
    int triggerType=_prepareSceneForRenderIfNeeded();
#endif

    CSimAndUiThreadSync::simThread_allowUiThreadToWrite();
    CSimAndUiThreadSync::outputNakedDebugMessage("$$W\n");
    CSimAndUiThreadSync::outputNakedDebugMessage("$$W *******************************************************\n");
    CSimAndUiThreadSync::outputNakedDebugMessage("$$W *******************************************************\n");
    CSimAndUiThreadSync::outputDebugMessage(__func__,"SIM thread waiting (safe)");

#ifdef SIM_WITH_GUI
    if ((triggerType>0)&&_renderingAllowed)
    { // we need to render. Send the appropriate signal
        if (triggerType==1)
            App::uiThread->requestSceneRender(false); // threaded rendering
        if (triggerType==2)
        {
            void* returnVal=CPluginContainer::sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_beforerendering,nullptr,nullptr,nullptr);
            delete[] (char*)returnVal;
            App::uiThread->requestSceneRender_wait(); // non-threaded rendering
        }
    }

#endif

    int pass=0;
    while ((pass==0)||(!CSimAndUiThreadSync::simThread_forbidUiThreadToWrite(false)))
    {
        pass++;
        // Following very important: here the custom UI plugin triggers script callbacks. Also important for offscreen openGl contexts that can be the cause for strange UI locks
        App::qtApp->processEvents();
    }

    CSimAndUiThreadSync::outputDebugMessage(__func__,"SIM thread NOT waiting anymore");
    CSimAndUiThreadSync::outputNakedDebugMessage("$$W *******************************************************\n");
    CSimAndUiThreadSync::outputNakedDebugMessage("$$W *******************************************************\n");
    CSimAndUiThreadSync::outputNakedDebugMessage("$$W\n");
#endif // SIM_WITHOUT_QT_AT_ALL
    // Handle delayed commands:
    _handleSimulationThreadCommands();
}

void CSimThread::appendSimulationThreadCommand(SSimulationThreadCommand cmd,int executionDelay/*=0*/)
{ // CALLED FROM ANY THREAD
    cmd.sceneUniqueId=App::ct->environment->getSceneUniqueID();
    cmd.postTime=VDateTime::getTimeInMs();
    cmd.execDelay=executionDelay;
    EASYLOCK(_simulationThreadCommandsMutex);
    _simulationThreadCommands_tmp.push_back(cmd);
}

void CSimThread::_handleSimulationThreadCommands()
{ // CALLED ONLY FROM THE MAIN SIMULATION THREAD
    IF_C_API_SIM_OR_UI_THREAD_CAN_WRITE_DATA
    {
        { // Keep the parenthesis!
            EASYLOCK(_simulationThreadCommandsMutex);
            for (unsigned int i=0;i<_simulationThreadCommands_tmp.size();i++)
                _simulationThreadCommands.push_back(_simulationThreadCommands_tmp[i]);
            _simulationThreadCommands_tmp.clear();
        }

        std::vector<SSimulationThreadCommand> delayedCommands;
        while (_simulationThreadCommands.size()>0)
        {
            SSimulationThreadCommand cmd=_simulationThreadCommands[0];
            if (cmd.execDelay!=0)
            {
                if (VDateTime::getTimeDiffInMs(cmd.postTime)>cmd.execDelay)
                    cmd.execDelay=0; // delay triggered!
            }
            if (cmd.execDelay!=0)
            {
                delayedCommands.push_back(cmd);
                _simulationThreadCommands.erase(_simulationThreadCommands.begin());
            }
            else
            {
                _executeSimulationThreadCommand(cmd);
                _simulationThreadCommands.erase(_simulationThreadCommands.begin());

                if (cmd.cmdId==POSTPONE_PROCESSING_THIS_LOOP_CMD)
                {
                    if (cmd.intParams[0]>1)
                    {
                        cmd.intParams[0]--;
                        _simulationThreadCommands.insert(_simulationThreadCommands.begin(),cmd);
                    }
                    break;
                }
            }
        }
        // Now append the delayed commands:
        _simulationThreadCommands.insert(_simulationThreadCommands.end(),delayedCommands.begin(),delayedCommands.end());
    }
}

void CSimThread::_executeSimulationThreadCommand(SSimulationThreadCommand cmd)
{
    FUNCTION_DEBUG;
    if (cmd.sceneUniqueId==App::ct->environment->getSceneUniqueID())
    {
        if (cmd.cmdId==SET_SHAPE_TRANSPARENCY_CMD)
        {
            CShape* shape=App::ct->objCont->getShape(cmd.intParams[0]);
            if (shape!=nullptr)
            {
                if (!shape->isCompound())
                {
                    ((CGeometric*)shape->geomData->geomInfo)->color.translucid=cmd.boolParams[0];
                    ((CGeometric*)shape->geomData->geomInfo)->color.transparencyFactor=cmd.floatParams[0];
                    POST_SCENE_CHANGED_ANNOUNCEMENT("");
                }
            }
        }
        if (cmd.cmdId==MAKE_OBJECT_CHILD_OF_CMD)
        {
            C3DObject* child=App::ct->objCont->getObjectFromHandle(cmd.intParams[0]);
            C3DObject* parent=App::ct->objCont->getObjectFromHandle(cmd.intParams[1]);
            if (child!=nullptr)
            {
                App::ct->objCont->makeObjectChildOf(child,parent);
                POST_SCENE_CHANGED_ANNOUNCEMENT(""); // **************** UNDO THINGY ****************
            }
        }

        // Scene object operation commands:
        if ( (cmd.cmdId>SCENE_OBJECT_OPERATION_START_SOOCMD)&&(cmd.cmdId<SCENE_OBJECT_OPERATION_END_SOOCMD) )
            CSceneObjectOperations::processCommand(cmd.cmdId);

        if ( (cmd.cmdId>SCRIPT_CONT_COMMANDS_START_SCCMD)&&(cmd.cmdId<SCRIPT_CONT_COMMANDS_END_SCCMD) )
            App::ct->addOnScriptContainer->processCommand(cmd.cmdId);

        if ( (cmd.cmdId>PAGE_CONT_FUNCTIONS_START_PCCMD)&&(cmd.cmdId<PAGE_CONT_FUNCTIONS_END_PCCMD) )
            App::ct->pageContainer->processCommand(cmd.cmdId,cmd.intParams[0]);

        if ( (cmd.cmdId>FILE_OPERATION_START_FOCMD)&&(cmd.cmdId<FILE_OPERATION_END_FOCMD) )
            CFileOperations::processCommand(cmd);

        if ( (cmd.cmdId>SIMULATION_COMMANDS_START_SCCMD)&&(cmd.cmdId<SIMULATION_COMMANDS_END_SCCMD) )
            App::ct->simulation->processCommand(cmd.cmdId);

        if (cmd.cmdId==SET_SHAPE_SHADING_ANGLE_CMD)
        {
            CShape* shape=App::ct->objCont->getShape(cmd.intParams[0]);
            if ((shape!=nullptr)&&shape->geomData->geomInfo->isGeometric())
                ((CGeometric*)shape->geomData->geomInfo)->setGouraudShadingAngle(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_SHAPE_EDGE_ANGLE_CMD)
        {
            CShape* shape=App::ct->objCont->getShape(cmd.intParams[0]);
            if ((shape!=nullptr)&&shape->geomData->geomInfo->isGeometric())
                ((CGeometric*)shape->geomData->geomInfo)->setEdgeThresholdAngle(cmd.floatParams[0]);
        }

    }

    if (cmd.cmdId==FINAL_EXIT_REQUEST_CMD)
        App::postExitRequest();

    if (cmd.cmdId==DISPLAY_WARNING_IF_DEBUGGING_CMD)
    {
        bool debugging=false;
#ifndef SIM_WITHOUT_QT_AT_ALL
        debugging|=CSimAndUiThreadSync::getShowActivityInConsole();
        debugging|=CSimAndUiThreadSync::getShowLockFailsActivityInConsole();
#endif
        debugging|=(CFuncDebug::getDebugMask()!=0);
        debugging|=CEasyLock::getShowActivity();
        debugging|=CThreadPool::getShowThreadSwitches();
        if ( debugging&&(!App::userSettings->suppressStartupDialogs) )
#ifdef SIM_WITH_GUI
            App::uiThread->messageBox_information(App::mainWindow,"Debugging","Debugging is turned on: this might lead to drastic performance loss.",VMESSAGEBOX_OKELI);
#else
            printf("Debugging is turned on: this might lead to drastic performance loss.\n");
#endif
    }

    handleVerSpecExecuteSimulationThreadCommand1(&cmd);

#ifdef SIM_WITH_GUI
    if (cmd.sceneUniqueId==App::ct->environment->getSceneUniqueID())
    {
        if (cmd.cmdId==TOGGLE_EXPAND_COLLAPSE_HIERARCHY_OBJECT_CMD)
        {
            C3DObject* obj=App::ct->objCont->getObjectFromHandle(cmd.intParams[0]);
            if (obj!=nullptr)
            {
                obj->setLocalObjectProperty(obj->getLocalObjectProperty()^sim_objectproperty_collapsed);
                POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
            }
        }
        if (cmd.cmdId==ADD_OR_REMOVE_TO_FROM_OBJECT_SELECTION_CMD)
        {
            for (size_t i=0;i<cmd.intParams.size();i++)
            {
                C3DObject* obj=App::ct->objCont->getObjectFromHandle(cmd.intParams[i]);
                if (obj!=nullptr)
                {
                    if (cmd.boolParams[i])
                        App::ct->objCont->addObjectToSelection(cmd.intParams[i]);
                    else
                        App::ct->objCont->removeObjectFromSelection(cmd.intParams[i]);
                    POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
                }
            }
        }
        if (cmd.cmdId==CLICK_RAY_INTERSECTION_CMD)
            _handleClickRayIntersection(cmd);

        if (cmd.cmdId==SWAP_VIEWS_CMD)
        {
            CSPage* p=App::ct->pageContainer->getPage(cmd.intParams[0]);
            if (p!=nullptr)
            {
                p->swapViews(cmd.intParams[1],cmd.intParams[2],cmd.boolParams[0]);
                POST_SCENE_CHANGED_ANNOUNCEMENT(""); // **************** UNDO THINGY ****************
            }
        }


        if (cmd.cmdId==DELETE_SELECTED_PATH_POINTS_NON_EDIT_FROMUI_TOSIM_CMD)
        {
            App::addStatusbarMessage(IDSNS_DELETING_SELECTED_PATH_POINTS);
            CPath* path=App::ct->objCont->getPath(cmd.intParams[0]);
            if (path!=nullptr)
            {
                CPathCont* pc=path->pathContainer;
                std::vector<bool> toDelete(pc->getSimplePathPointCount(),false);
                for (int i=0;i<int(cmd.intVectorParams[0].size());i++)
                    toDelete[cmd.intVectorParams[0][i]]=true;
                for (int i=int(toDelete.size())-1;i>=0;i--)
                {
                    if (toDelete[i])
                        pc->removeSimplePathPoint(i);
                }
                POST_SCENE_CHANGED_ANNOUNCEMENT("");
                App::setLightDialogRefreshFlag();
            }
            App::addStatusbarMessage(IDSNS_DONE);
        }
        if (handleVerSpecExecuteSimulationThreadCommand2())
        {
            if (cmd.cmdId==OPEN_MODAL_SCRIPT_SIMULATION_PARAMETERS_CMD)
            {
                SUIThreadCommand cmdIn;
                SUIThreadCommand cmdOut;
                cmdIn.cmdId=OPEN_MODAL_SCRIPT_SIMULATION_PARAMETERS_UITHREADCMD;
                cmdIn.intParams.push_back(cmd.intParams[0]);
                {
                    // Following instruction very important in the function below tries to lock resources (or a plugin it calls!):
                    SIM_THREAD_INDICATE_UI_THREAD_CAN_DO_ANYTHING;
                    App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
                }
            }
            if (cmd.cmdId==OPEN_SCRIPT_EDITOR_CMD)
            {
                if (App::getEditModeType()==NO_EDIT_MODE)
                {
                    CLuaScriptObject* it=App::ct->luaScriptContainer->getScriptFromID_alsoAddOnsAndSandbox(cmd.intParams[0]);
                    if ((it!=nullptr)&&(App::mainWindow!=nullptr))
                    {
                        if (it->getScriptType()==sim_scripttype_customizationscript)
                            App::mainWindow->codeEditorContainer->openCustomizationScript(cmd.intParams[0],-1);
                        else
                            App::mainWindow->codeEditorContainer->openSimulationScript(cmd.intParams[0],-1);
                    }
                }
            }

            if (cmd.cmdId==OPEN_MODAL_MODEL_PROPERTIES_CMD)
            {
                if (App::getEditModeType()==NO_EDIT_MODE)
                {
                    SUIThreadCommand cmdIn;
                    SUIThreadCommand cmdOut;
                    cmdIn.cmdId=OPEN_MODAL_MODEL_PROPERTIES_UITHREADCMD;
                    cmdIn.intParams.push_back(cmd.intParams[0]);
                    {
                        // Following instruction very important in the function below tries to lock resources (or a plugin it calls!):
                        SIM_THREAD_INDICATE_UI_THREAD_CAN_DO_ANYTHING;
                        App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
                    }
                    POST_SCENE_CHANGED_ANNOUNCEMENT(""); // **************** UNDO THINGY ****************
                    App::setFullDialogRefreshFlag();
                }
            }
        }

        if (cmd.cmdId==CLOSE_FLOATING_VIEW_CMD)
        {
            CSPage* page=App::ct->pageContainer->getPage(App::ct->pageContainer->getActivePageIndex());
            if (page!=nullptr)
            {
                CSView* theFloatingView=page->getView(cmd.intParams[0]);
                if (theFloatingView!=nullptr)
                {
                    if (theFloatingView->getCanBeClosed())
                    {
                        page->removeFloatingView(cmd.intParams[0]);
                        POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
                    }
                }
            }
        }

        if (cmd.cmdId==REMOVE_CURRENT_PAGE_CMD)
        {
            App::ct->pageContainer->removePage(App::ct->pageContainer->getActivePageIndex());
            App::addStatusbarMessage(IDSNS_REMOVED_VIEW);
            POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
        }

        // Edit mode commands:
        if ( (App::mainWindow!=nullptr)&&(cmd.cmdId>EDIT_MODE_COMMANDS_START_EMCMD)&&(cmd.cmdId<EDIT_MODE_COMMANDS_END_EMCMD) )
        {
            C3DObject* additional3DObject=nullptr;
            if (cmd.cmdId==PATH_EDIT_MODE_APPEND_NEW_PATH_POINT_FROM_CAMERA_EMCMD)
            {
                additional3DObject=(C3DObject*)cmd.objectParams[0];
                if (additional3DObject!=nullptr)
                { // make sure that object is still valid:
                    if (!App::ct->objCont->doesObjectExist(additional3DObject))
                        additional3DObject=nullptr;
                }
            }
            App::mainWindow->editModeContainer->processCommand(cmd.cmdId,additional3DObject);
        }

        if (cmd.cmdId==DISPLAY_MESSAGE_CMD)
        {
            if (App::mainWindow!=nullptr)
            {
                if (cmd.intParams[0]==sim_msgbox_type_info)
                    App::uiThread->messageBox_information(App::mainWindow,cmd.stringParams[0],cmd.stringParams[1],VMESSAGEBOX_OKELI);
                if (cmd.intParams[0]==sim_msgbox_type_warning)
                    App::uiThread->messageBox_warning(App::mainWindow,cmd.stringParams[0],cmd.stringParams[1],VMESSAGEBOX_OKELI);
                if (cmd.intParams[0]==sim_msgbox_type_critical)
                    App::uiThread->messageBox_critical(App::mainWindow,cmd.stringParams[0],cmd.stringParams[1],VMESSAGEBOX_OKELI);
            }
        }

        if ( (cmd.cmdId>ADD_COMMANDS_START_ACCMD)&&(cmd.cmdId<ADD_COMMANDS_END_ACCMD) )
        {
            CSView* subview=nullptr;
            if (cmd.objectParams.size()>0)
            {
                subview=(CSView*)cmd.objectParams[0];
                if (subview!=nullptr)
                { // make sure that object is still valid:
                    CSPage* page=App::ct->pageContainer->getPage(App::ct->pageContainer->getActivePageIndex());
                    if (!page->isViewValid(subview))
                        subview=nullptr;
                }
            }
            CAddOperations::processCommand(cmd.cmdId,subview);
        }

        if (cmd.cmdId==CAMERA_SHIFT_TO_FRAME_SELECTION_CMD)
            CToolBarCommand::processCommand(cmd.cmdId);
        if (cmd.cmdId==CAMERA_SHIFT_NAVIGATION_CMD)
            CToolBarCommand::processCommand(cmd.cmdId);
        if (cmd.cmdId==CAMERA_ROTATE_NAVIGATION_CMD)
            CToolBarCommand::processCommand(cmd.cmdId);
        if (cmd.cmdId==CAMERA_ZOOM_NAVIGATION_CMD)
            CToolBarCommand::processCommand(cmd.cmdId);
        if (cmd.cmdId==CAMERA_TILT_NAVIGATION_CMD)
            CToolBarCommand::processCommand(cmd.cmdId);
        if (cmd.cmdId==CAMERA_ANGLE_NAVIGATION_CMD)
            CToolBarCommand::processCommand(cmd.cmdId);
        if (cmd.cmdId==CAMERA_FLY_NAVIGATION_CMD)
            CToolBarCommand::processCommand(cmd.cmdId);
        if (cmd.cmdId==OBJECT_SHIFT_NAVIGATION_CMD)
            CToolBarCommand::processCommand(cmd.cmdId);
        if (cmd.cmdId==OBJECT_ROTATE_NAVIGATION_CMD)
            CToolBarCommand::processCommand(cmd.cmdId);
        if (cmd.cmdId==PAGE_SELECTOR_CMD)
            CToolBarCommand::processCommand(cmd.cmdId);
        if ( (cmd.cmdId==SCENE_SELECTOR_CMD)||(cmd.cmdId==SCENE_SELECTOR_PHASE2_CMD) )
            CToolBarCommand::processCommand(cmd.cmdId);
        if (cmd.cmdId==OBJECT_SELECTION_SELECTION_CMD)
            CToolBarCommand::processCommand(cmd.cmdId);
        if (cmd.cmdId==PATH_POINT_CREATION_MODE_CMD)
            CToolBarCommand::processCommand(cmd.cmdId);
        if (cmd.cmdId==CLEAR_SELECTION_SELECTION_CMD)
            CToolBarCommand::processCommand(cmd.cmdId);
        if ( (cmd.cmdId>=VIEW_1_CMD)&&(cmd.cmdId<=VIEW_8_CMD) )
            CToolBarCommand::processCommand(cmd.cmdId);

        if (App::mainWindow!=nullptr)
        {
            if ( (cmd.cmdId==EXPAND_HIERARCHY_CMD)||(cmd.cmdId==COLLAPSE_HIERARCHY_CMD) )
                App::mainWindow->oglSurface->hierarchy->processCommand(cmd.cmdId);
            if ( (cmd.cmdId==EXPAND_SELECTED_HIERARCHY_CMD)||(cmd.cmdId==COLLAPSE_SELECTED_HIERARCHY_CMD) )
                App::mainWindow->oglSurface->hierarchy->processCommand(cmd.cmdId);
            if ( (cmd.cmdId>=HIERARCHY_COLORING_NONE_CMD)&&(cmd.cmdId<=HIERARCHY_COLORING_BLUE_CMD) )
                App::mainWindow->oglSurface->hierarchy->processCommand(cmd.cmdId);
        }

        if ( (cmd.cmdId>VIEW_FUNCTIONS_START_VFCMD)&&(cmd.cmdId<VIEW_FUNCTIONS_END_VFCMD) )
        {
            CSPage* page=App::ct->pageContainer->getPage(App::ct->pageContainer->getActivePageIndex());
            if (page!=nullptr)
            {
                CSView* view=page->getView(cmd.intParams[0]);
                if (view!=nullptr)
                    view->processCommand(cmd.cmdId,cmd.intParams[0]);
            }
        }

        if ( (cmd.cmdId>VIEW_SELECTOR_START_VSCMD)&&(cmd.cmdId<VIEW_SELECTOR_END_VSCMD) )
            App::mainWindow->oglSurface->viewSelector->processCommand(cmd.cmdId,cmd.intParams[0]);


        if (cmd.cmdId==MODEL_BROWSER_DRAG_AND_DROP_CMD)
        {
            CFileOperations::loadModel(cmd.stringParams[0].c_str(),true,true,false,nullptr,false,nullptr,false,false);
            if (App::ct->objCont->getSelSize()==1)
            { // we could have several model bases (in the old fileformat)
                C3DObject* obj=App::ct->objCont->getLastSelection_object();
                if (obj!=nullptr)
                {
// Not anymore! 30/12/2016                    if ( (obj->getObjectManipulationModePermissions()&0x03)||(obj->getObjectManipulationTranslationRelativeTo()!=0) )
//                    { // We can only place the model if the X and/or Y manip are set or if the placement is not relative to world
                        C7Vector tr(obj->getLocalTransformation());
                        float ss=obj->getNonDefaultTranslationStepSize();
                        if (ss==0.0)
                            ss=App::userSettings->getTranslationStepSize();
                        float x=cmd.floatParams[0]-fmod(cmd.floatParams[0],ss);
                        float y=cmd.floatParams[1]-fmod(cmd.floatParams[1],ss);
                        tr.X(0)+=x;
                        tr.X(1)+=y;
                        obj->setLocalTransformation(tr);
                        // To avoid flickering:
                        obj->bufferMainDisplayStateVariables();
                        obj->bufferedMainDisplayStateVariablesToDisplay();
//                    }
                }
            }
            App::mainWindow->openglWidget->clearModelDragAndDropInfo();
            POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
        }

        if (cmd.cmdId==DISPLAY_VARIOUS_WARNING_MESSAGES_DURING_SIMULATION_CMD)
        {
            if ( (!App::ct->simulation->isSimulationStopped())&&(!App::isFullScreen())&&(App::mainWindow!=nullptr) )
            {
                _displayVariousWaningMessagesDuringSimulation();
                App::appendSimulationThreadCommand(cmd,500);
            }
        }

        if (cmd.cmdId==SET_VIEW_ANGLE_CAMERAGUITRIGGEREDCMD)
        {
            CCamera* cam=App::ct->objCont->getCamera(cmd.intParams[0]);
            if (cam!=nullptr)
                cam->setViewAngle(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_ORTHO_VIEW_SIZE_CAMERAGUITRIGGEREDCMD)
        {
            CCamera* cam=App::ct->objCont->getCamera(cmd.intParams[0]);
            if (cam!=nullptr)
                cam->setOrthoViewSize(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_NEAR_CLIPPING_CAMERAGUITRIGGEREDCMD)
        {
            CCamera* cam=App::ct->objCont->getCamera(cmd.intParams[0]);
            if (cam!=nullptr)
                cam->setNearClippingPlane(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_FAR_CLIPPING_CAMERAGUITRIGGEREDCMD)
        {
            CCamera* cam=App::ct->objCont->getCamera(cmd.intParams[0]);
            if (cam!=nullptr)
                cam->setFarClippingPlane(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_TRACKED_OBJECT_CAMERAGUITRIGGEREDCMD)
        {
            CCamera* cam=App::ct->objCont->getCamera(cmd.intParams[0]);
            if (cam!=nullptr)
            {
                if (cmd.intParams[1]<0)
                    cam->setTrackedObjectID(-1);
                else
                {
                    C3DObject* obj=App::ct->objCont->getObjectFromHandle(cmd.intParams[1]);
                    if (obj!=nullptr)
                        cam->setTrackedObjectID(obj->getObjectHandle());
                }
            }
        }
        if (cmd.cmdId==TOGGLE_SHOWFOG_CAMERAGUITRIGGEREDCMD)
        {
            CCamera* cam=App::ct->objCont->getCamera(cmd.intParams[0]);
            if (cam!=nullptr)
                cam->setShowFogIfAvailable(!cam->getShowFogIfAvailable());
        }
        if (cmd.cmdId==TOGGLE_KEEPHEADUP_CAMERAGUITRIGGEREDCMD)
        {
            CCamera* cam=App::ct->objCont->getCamera(cmd.intParams[0]);
            if (cam!=nullptr)
                cam->setCameraManipulationModePermissions(cam->getCameraManipulationModePermissions()^0x020);
        }
        if (cmd.cmdId==TOGGLE_USEPARENTASMANIPPROXY_CAMERAGUITRIGGEREDCMD)
        {
            CCamera* cam=App::ct->objCont->getCamera(cmd.intParams[0]);
            if (cam!=nullptr)
                cam->setUseParentObjectAsManipulationProxy(!cam->getUseParentObjectAsManipulationProxy());
        }
        if (cmd.cmdId==TOGGLE_SHIFTALONGX_CAMERAGUITRIGGEREDCMD)
        {
            CCamera* cam=App::ct->objCont->getCamera(cmd.intParams[0]);
            if (cam!=nullptr)
                cam->setCameraManipulationModePermissions(cam->getCameraManipulationModePermissions()^0x001);
        }
        if (cmd.cmdId==TOGGLE_SHIFTALONGY_CAMERAGUITRIGGEREDCMD)
        {
            CCamera* cam=App::ct->objCont->getCamera(cmd.intParams[0]);
            if (cam!=nullptr)
                cam->setCameraManipulationModePermissions(cam->getCameraManipulationModePermissions()^0x002);
        }
        if (cmd.cmdId==TOGGLE_SHIFTALONGZ_CAMERAGUITRIGGEREDCMD)
        {
            CCamera* cam=App::ct->objCont->getCamera(cmd.intParams[0]);
            if (cam!=nullptr)
                cam->setCameraManipulationModePermissions(cam->getCameraManipulationModePermissions()^0x004);
        }
        if (cmd.cmdId==TOGGLE_FULLROTATION_CAMERAGUITRIGGEREDCMD)
        {
            CCamera* cam=App::ct->objCont->getCamera(cmd.intParams[0]);
            if (cam!=nullptr)
                cam->setCameraManipulationModePermissions(cam->getCameraManipulationModePermissions()^0x008);
        }
        if (cmd.cmdId==TOGGLE_TILTING_CAMERAGUITRIGGEREDCMD)
        {
            CCamera* cam=App::ct->objCont->getCamera(cmd.intParams[0]);
            if (cam!=nullptr)
                cam->setCameraManipulationModePermissions(cam->getCameraManipulationModePermissions()^0x010);
        }
        if (cmd.cmdId==SET_SIZE_CAMERAGUITRIGGEREDCMD)
        {
            CCamera* cam=App::ct->objCont->getCamera(cmd.intParams[0]);
            if (cam!=nullptr)
                cam->setCameraSize(cmd.floatParams[0]);
        }
        if (cmd.cmdId==TOGGLE_LOCALLIGHTS_CAMERAGUITRIGGEREDCMD)
        {
            CCamera* cam=App::ct->objCont->getCamera(cmd.intParams[0]);
            if (cam!=nullptr)
                cam->setUseLocalLights(!cam->getuseLocalLights());
        }
        if (cmd.cmdId==TOGGLE_ALLOWPICKING_CAMERAGUITRIGGEREDCMD)
        {
            CCamera* cam=App::ct->objCont->getCamera(cmd.intParams[0]);
            if (cam!=nullptr)
                cam->setAllowPicking(!cam->getAllowPicking());
        }
        if (cmd.cmdId==SET_RENDERMODE_CAMERAGUITRIGGEREDCMD)
        {
            CCamera* cam=App::ct->objCont->getCamera(cmd.intParams[0]);
            if (cam!=nullptr)
                cam->setRenderMode(cmd.intParams[1],cmd.boolParams[0],cmd.boolParams[1]);
        }
        if (cmd.cmdId==NEW_COLLECTION_COLLECTIONGUITRIGGEREDCMD)
        {
            CRegCollection* newGroup=new CRegCollection(IDSOGL_COLLECTION);
            App::ct->collections->addCollection(newGroup,false);
            // Now select the new collection in the UI. We need to post it so that it arrives after the dialog refresh!:
            SSimulationThreadCommand cmd2;
            cmd2.cmdId=CALL_DIALOG_FUNCTION_GUITRIGGEREDCMD;
            cmd2.intParams.push_back(COLLECTION_DLG);
            cmd2.intParams.push_back(0);
            cmd2.intParams.push_back(newGroup->getCollectionID());
            App::appendSimulationThreadCommand(cmd2);
        }
        if (cmd.cmdId==TOGGLE_OVERRIDE_COLLECTIONGUITRIGGEREDCMD)
        {
            CRegCollection* theGroup=App::ct->collections->getCollection(cmd.intParams[0]);
            if (theGroup!=nullptr)
                theGroup->setOverridesObjectMainProperties(!theGroup->getOverridesObjectMainProperties());
        }
        if (cmd.cmdId==REMOVE_COLLECTION_COLLECTIONGUITRIGGEREDCMD)
        {
            App::ct->collections->removeCollection(cmd.intParams[0]);
        }
        if (cmd.cmdId==REMOVE_COLLECTION_ITEM_COLLECTIONGUITRIGGEREDCMD)
        {
            CRegCollection* theGroup=App::ct->collections->getCollection(cmd.intParams[0]);
            if (theGroup!=nullptr)
            {
                for (size_t i=1;i<cmd.intParams.size();i++)
                    theGroup->removeSubCollection(cmd.intParams[i]);
                if (theGroup->subCollectionList.size()==0)
                { // The group is empty and we have to remove it
                    App::ct->collections->removeCollection(theGroup->getCollectionID());
                }
            }
        }
        if (cmd.cmdId==RENAME_COLLECTION_COLLECTIONGUITRIGGEREDCMD)
        {
            CRegCollection* it=App::ct->collections->getCollection(cmd.intParams[0]);
            if (it!=nullptr)
            {
                std::string newName(cmd.stringParams[0]);
                if (it->getCollectionName()!=newName)
                {
                    tt::removeIllegalCharacters(newName,true);
                    if (App::ct->collections->getCollection(newName)==nullptr)
                        it->setCollectionName(newName);
                }
            }
        }
        if (cmd.cmdId==ADD_COLLECTION_ITEM_EVERYTHING_COLLECTIONGUITRIGGEREDCMD)
        {
            CRegCollection* it=App::ct->collections->getCollection(cmd.intParams[0]);
            if (it!=nullptr)
            {
                CRegCollectionEl* grEl=new CRegCollectionEl(-1,GROUP_EVERYTHING,true);
                it->addSubCollection(grEl);
            }
        }
        if (cmd.cmdId==ADD_COLLECTION_ITEM_LOOS_COLLECTIONGUITRIGGEREDCMD)
        {
            CRegCollection* it=App::ct->collections->getCollection(cmd.intParams[0]);
            bool additive=cmd.boolParams[0];
            if (it!=nullptr)
            {
                for (size_t i=1;i<cmd.intParams.size();i++)
                {
                    C3DObject* obj=App::ct->objCont->getObjectFromHandle(cmd.intParams[i]);
                    if (obj!=nullptr)
                    {
                        CRegCollectionEl* grEl=new CRegCollectionEl(obj->getObjectHandle(),GROUP_LOOSE,additive);
                        it->addSubCollection(grEl);
                    }
                }
            }
        }
        if (cmd.cmdId==ADD_COLLECTION_ITEM_FROMBASE_COLLECTIONGUITRIGGEREDCMD)
        {
            CRegCollection* it=App::ct->collections->getCollection(cmd.intParams[0]);
            bool additive=cmd.boolParams[0];
            bool baseInclusive=cmd.boolParams[1];
            if (it!=nullptr)
            {
                C3DObject* lastSel=App::ct->objCont->getObjectFromHandle(cmd.intParams[1]);
                if (lastSel!=nullptr)
                {
                    int grpType=GROUP_FROM_BASE_INCLUDED;
                    if (!baseInclusive)
                        grpType=GROUP_FROM_BASE_EXCLUDED;
                    CRegCollectionEl* grEl=new CRegCollectionEl(lastSel->getObjectHandle(),grpType,additive);
                    it->addSubCollection(grEl);
                }
            }
        }
        if (cmd.cmdId==ADD_COLLECTION_ITEM_FROMTIP_COLLECTIONGUITRIGGEREDCMD)
        {
            CRegCollection* it=App::ct->collections->getCollection(cmd.intParams[0]);
            bool additive=cmd.boolParams[0];
            bool tipInclusive=cmd.boolParams[1];
            if (it!=nullptr)
            {
                C3DObject* lastSel=App::ct->objCont->getObjectFromHandle(cmd.intParams[1]);
                if (lastSel!=nullptr)
                {
                    int grpType=GROUP_FROM_TIP_INCLUDED;
                    if (!tipInclusive)
                        grpType=GROUP_FROM_TIP_EXCLUDED;
                    CRegCollectionEl* grEl=new CRegCollectionEl(lastSel->getObjectHandle(),grpType,additive);
                    it->addSubCollection(grEl);
                }
            }
        }


        if (cmd.cmdId==TOGGLE_ENABLE_ALL_VISIONSENSORGUITRIGGEREDCMD)
        {
            App::ct->mainSettings->visionSensorsEnabled=!App::ct->mainSettings->visionSensorsEnabled;
        }
        if (cmd.cmdId==TOGGLE_EXPLICITHANDLING_VISIONSENSORGUITRIGGEREDCMD)
        {
            CVisionSensor* it=App::ct->objCont->getVisionSensor(cmd.intParams[0]);
            if (it!=nullptr)
                it->setExplicitHandling(!it->getExplicitHandling());
        }
        if (cmd.cmdId==TOGGLE_EXTERNALINPUT_VISIONSENSORGUITRIGGEREDCMD)
        {
            CVisionSensor* it=App::ct->objCont->getVisionSensor(cmd.intParams[0]);
            if (it!=nullptr)
                it->setUseExternalImage(!it->getUseExternalImage());
        }
        if (cmd.cmdId==TOGGLE_PERSPECTIVE_VISIONSENSORGUITRIGGEREDCMD)
        {
            CVisionSensor* it=App::ct->objCont->getVisionSensor(cmd.intParams[0]);
            if (it!=nullptr)
                it->setPerspectiveOperation(!it->getPerspectiveOperation());
        }
        if (cmd.cmdId==TOGGLE_LOCALLIGHTS_VISIONSENSORGUITRIGGEREDCMD)
        {
            CVisionSensor* it=App::ct->objCont->getVisionSensor(cmd.intParams[0]);
            if (it!=nullptr)
                it->setUseLocalLights(!it->getuseLocalLights());
        }
        if (cmd.cmdId==TOGGLE_SHOWVOLUME_WHEN_NOT_DETECTING_VISIONSENSORGUITRIGGEREDCMD)
        {
            CVisionSensor* it=App::ct->objCont->getVisionSensor(cmd.intParams[0]);
            if (it!=nullptr)
                it->setShowVolumeWhenNotDetecting(!it->getShowVolumeWhenNotDetecting());
        }
        if (cmd.cmdId==TOGGLE_SHOWFOG_VISIONSENSORGUITRIGGEREDCMD)
        {
            CVisionSensor* it=App::ct->objCont->getVisionSensor(cmd.intParams[0]);
            if (it!=nullptr)
                it->setShowFogIfAvailable(!it->getShowFogIfAvailable());
        }
        if (cmd.cmdId==TOGGLE_SHOWVOLUME_WHEN_DETECTING_VISIONSENSORGUITRIGGEREDCMD)
        {
            CVisionSensor* it=App::ct->objCont->getVisionSensor(cmd.intParams[0]);
            if (it!=nullptr)
                it->setShowVolumeWhenDetecting(!it->getShowVolumeWhenDetecting());
        }
        if (cmd.cmdId==SET_NEARCLIPPING_VISIONSENSORGUITRIGGEREDCMD)
        {
            CVisionSensor* it=App::ct->objCont->getVisionSensor(cmd.intParams[0]);
            if (it!=nullptr)
                it->setNearClippingPlane(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_FARCLIPPING_VISIONSENSORGUITRIGGEREDCMD)
        {
            CVisionSensor* it=App::ct->objCont->getVisionSensor(cmd.intParams[0]);
            if (it!=nullptr)
                it->setFarClippingPlane(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_PERSPECTANGLE_OR_ORTHOSIZE_VISIONSENSORGUITRIGGEREDCMD)
        {
            CVisionSensor* it=App::ct->objCont->getVisionSensor(cmd.intParams[0]);
            if (it!=nullptr)
            {
                if (it->getPerspectiveOperation())
                    it->setViewAngle(cmd.floatParams[0]);
                else
                    it->setOrthoViewSize(cmd.floatParams[0]);
            }
        }
        if (cmd.cmdId==SET_RESOLUTION_VISIONSENSORGUITRIGGEREDCMD)
        {
            CVisionSensor* it=App::ct->objCont->getVisionSensor(cmd.intParams[0]);
            if (it!=nullptr)
                it->setDesiredResolution(&cmd.intParams[1]);
        }
        if (cmd.cmdId==SET_OBJECTSIZE_VISIONSENSORGUITRIGGEREDCMD)
        {
            CVisionSensor* it=App::ct->objCont->getVisionSensor(cmd.intParams[0]);
            if (it!=nullptr)
                it->setSize(C3Vector(&cmd.floatParams[0]));
        }
        if (cmd.cmdId==SET_DEFAULTIMGCOL_VISIONSENSORGUITRIGGEREDCMD)
        {
            CVisionSensor* it=App::ct->objCont->getVisionSensor(cmd.intParams[0]);
            if (it!=nullptr)
            {
                it->setDefaultBufferValues(&cmd.floatParams[0]);
                it->setUseEnvironmentBackgroundColor(cmd.boolParams[0]);
            }
        }
        if (cmd.cmdId==APPLY_MAINPROP_TO_SELECTION_VISIONSENSORGUITRIGGEREDCMD)
        {
            CVisionSensor* last=App::ct->objCont->getVisionSensor(cmd.intParams[0]);
            if (last!=nullptr)
            {
                int r[2];
                last->getDesiredResolution(r);
                float b[3];
                last->getDefaultBufferValues(b);
                for (size_t i=1;i<cmd.intParams.size();i++)
                {
                    CVisionSensor* it=App::ct->objCont->getVisionSensor(cmd.intParams[i]);
                    if (it!=nullptr)
                    {
                        it->setRenderMode(last->getRenderMode());
                        it->setExplicitHandling(last->getExplicitHandling());
                        it->setUseExternalImage(last->getUseExternalImage());
                        it->setPerspectiveOperation(last->getPerspectiveOperation());
                        it->setShowFogIfAvailable(last->getShowFogIfAvailable());
                        it->setIgnoreRGBInfo(last->getIgnoreRGBInfo());
                        it->setIgnoreDepthInfo(last->getIgnoreDepthInfo());
                        it->setComputeImageBasicStats(last->getComputeImageBasicStats());
                        it->setNearClippingPlane(last->getNearClippingPlane());
                        it->setFarClippingPlane(last->getFarClippingPlane());
                        it->setViewAngle(last->getViewAngle());
                        it->setOrthoViewSize(last->getOrthoViewSize());
                        it->setDesiredResolution(r);
                        it->setUseLocalLights(last->getuseLocalLights());
                        it->setUseEnvironmentBackgroundColor(last->getUseEnvironmentBackgroundColor());
                        it->setDefaultBufferValues(b);
                        it->setDetectableEntityID(last->getDetectableEntityID());
                    }
                }
            }
        }
        if (cmd.cmdId==APPLY_VISUALPROP_TO_SELECTION_VISIONSENSORGUITRIGGEREDCMD)
        {
            CVisionSensor* last=App::ct->objCont->getVisionSensor(cmd.intParams[0]);
            if (last!=nullptr)
            {
                for (size_t i=1;i<cmd.intParams.size();i++)
                {
                    CVisionSensor* it=App::ct->objCont->getVisionSensor(cmd.intParams[i]);
                    if (it!=nullptr)
                    {
                        last->getColor(false)->copyYourselfInto(it->getColor(false));
                        last->getColor(true)->copyYourselfInto(it->getColor(true));
                        it->setSize(last->getSize());
                        it->setShowVolumeWhenNotDetecting(last->getShowVolumeWhenNotDetecting());
                        it->setShowVolumeWhenDetecting(last->getShowVolumeWhenDetecting());
                    }
                }
            }
        }
        if (cmd.cmdId==SET_ENTITYTODETECT_VISIONSENSORGUITRIGGEREDCMD)
        {
            CVisionSensor* it=App::ct->objCont->getVisionSensor(cmd.intParams[0]);
            if (it!=nullptr)
                it->setDetectableEntityID(cmd.intParams[1]);
        }
        if (cmd.cmdId==TOGGLE_IGNORERGB_VISIONSENSORGUITRIGGEREDCMD)
        {
            CVisionSensor* it=App::ct->objCont->getVisionSensor(cmd.intParams[0]);
            if (it!=nullptr)
                it->setIgnoreRGBInfo(!it->getIgnoreRGBInfo());
        }
        if (cmd.cmdId==TOGGLE_IGNOREDEPTH_VISIONSENSORGUITRIGGEREDCMD)
        {
            CVisionSensor* it=App::ct->objCont->getVisionSensor(cmd.intParams[0]);
            if (it!=nullptr)
                it->setIgnoreDepthInfo(!it->getIgnoreDepthInfo());
        }
        if (cmd.cmdId==TOGGLE_PACKET1BLANK_VISIONSENSORGUITRIGGEREDCMD)
        {
            CVisionSensor* it=App::ct->objCont->getVisionSensor(cmd.intParams[0]);
            if (it!=nullptr)
                it->setComputeImageBasicStats(!it->getComputeImageBasicStats());
        }
        if (cmd.cmdId==SET_RENDERMODE_VISIONSENSORGUITRIGGEREDCMD)
        {
            CVisionSensor* it=App::ct->objCont->getVisionSensor(cmd.intParams[0]);
            if (it!=nullptr)
                it->setRenderMode(cmd.intParams[1]);
        }


        if (cmd.cmdId==ADD_NEW_COLLISIONGUITRIGGEREDCMD)
        {
            int h=App::ct->collisions->addNewObject(cmd.intParams[0],cmd.intParams[1],"");
            // Now select the new collection in the UI. We need to post it so that it arrives after the dialog refresh!:
            SSimulationThreadCommand cmd2;
            cmd2.cmdId=CALL_DIALOG_FUNCTION_GUITRIGGEREDCMD;
            cmd2.intParams.push_back(COLLISION_DLG);
            cmd2.intParams.push_back(0);
            cmd2.intParams.push_back(h);
            App::appendSimulationThreadCommand(cmd2);
        }
        if (cmd.cmdId==SET_OBJECTNAME_COLLISIONGUITRIGGEREDCMD)
        {
            CRegCollision* it=App::ct->collisions->getObject(cmd.intParams[0]);
            if (it!=nullptr)
            {
                std::string newName(cmd.stringParams[0]);
                if (it->getObjectName()!=newName)
                {
                    tt::removeIllegalCharacters(newName,true);
                    if (App::ct->collisions->getObject(newName)==nullptr)
                        it->setObjectName(newName);
                }
            }
        }
        if (cmd.cmdId==DELETE_OBJECT_COLLISIONGUITRIGGEREDCMD)
        {
            CRegCollision* it=App::ct->collisions->getObject(cmd.intParams[0]);
            if (it!=nullptr)
                App::ct->collisions->removeObject(it->getObjectID());
        }
        if (cmd.cmdId==TOGGLE_ENABLE_ALL_COLLISIONGUITRIGGEREDCMD)
        {
            App::ct->mainSettings->collisionDetectionEnabled=!App::ct->mainSettings->collisionDetectionEnabled;
        }
        if (cmd.cmdId==TOGGLE_EXPLICITHANDLING_COLLISIONGUITRIGGEREDCMD)
        {
            CRegCollision* it=App::ct->collisions->getObject(cmd.intParams[0]);
            if (it!=nullptr)
                it->setExplicitHandling(!it->getExplicitHandling());
        }
        if (cmd.cmdId==TOGGLE_COLLIDERCOLORCHANGES_COLLISIONGUITRIGGEREDCMD)
        {
            CRegCollision* it=App::ct->collisions->getObject(cmd.intParams[0]);
            if (it!=nullptr)
                it->setColliderChangesColor(!it->getColliderChangesColor());
        }
        if (cmd.cmdId==TOGGLE_COLLIDEECOLORCHANGES_COLLISIONGUITRIGGEREDCMD)
        {
            CRegCollision* it=App::ct->collisions->getObject(cmd.intParams[0]);
            if (it!=nullptr)
                it->setCollideeChangesColor(!it->getCollideeChangesColor());
        }
        if (cmd.cmdId==TOGGLE_COLLISIONCONTOUR_COLLISIONGUITRIGGEREDCMD)
        {
            CRegCollision* it=App::ct->collisions->getObject(cmd.intParams[0]);
            if (it!=nullptr)
                it->setExhaustiveDetection(!it->getExhaustiveDetection());
        }
        if (cmd.cmdId==SET_CONTOURWIDTH_COLLISIONGUITRIGGEREDCMD)
        {
            CRegCollision* it=App::ct->collisions->getObject(cmd.intParams[0]);
            if (it!=nullptr)
                it->setContourWidth(cmd.intParams[1]);
        }


        if (cmd.cmdId==ADD_NEW_DISTANCEGUITRIGGEREDCMD)
        {
            int h=App::ct->distances->addNewObject(cmd.intParams[0],cmd.intParams[1],"");
            // Now select the new collection in the UI. We need to post it so that it arrives after the dialog refresh!:
            SSimulationThreadCommand cmd2;
            cmd2.cmdId=CALL_DIALOG_FUNCTION_GUITRIGGEREDCMD;
            cmd2.intParams.push_back(DISTANCE_DLG);
            cmd2.intParams.push_back(0);
            cmd2.intParams.push_back(h);
            App::appendSimulationThreadCommand(cmd2);
        }
        if (cmd.cmdId==SET_OBJECTNAME_DISTANCEGUITRIGGEREDCMD)
        {
            CRegDist* it=App::ct->distances->getObject(cmd.intParams[0]);
            std::string newName(cmd.stringParams[0]);
            if ( (it!=nullptr)&&(newName!="") )
            {
                if (it->getObjectName()!=newName)
                {
                    tt::removeIllegalCharacters(newName,true);
                    if (App::ct->distances->getObject(newName)==nullptr)
                        it->setObjectName(newName);
                }
            }
        }
        if (cmd.cmdId==DELETE_OBJECT_DISTANCEGUITRIGGEREDCMD)
        {
            CRegDist* it=App::ct->distances->getObject(cmd.intParams[0]);
            if (it!=nullptr)
                App::ct->distances->removeObject(it->getObjectID());
        }
        if (cmd.cmdId==TOGGLE_ENABLE_ALL_DISTANCEGUITRIGGEREDCMD)
        {
            App::ct->mainSettings->distanceCalculationEnabled=!App::ct->mainSettings->distanceCalculationEnabled;
        }
        if (cmd.cmdId==TOGGLE_EXPLICITHANDLING_DISTANCEGUITRIGGEREDCMD)
        {
            CRegDist* it=App::ct->distances->getObject(cmd.intParams[0]);
            if (it!=nullptr)
                it->setExplicitHandling(!it->getExplicitHandling());
        }
        if (cmd.cmdId==TOGGLE_USETHRESHOLD_DISTANCEGUITRIGGEREDCMD)
        {
            CRegDist* it=App::ct->distances->getObject(cmd.intParams[0]);
            if (it!=nullptr)
                it->setTreshholdActive(!it->getTreshholdActive());
        }
        if (cmd.cmdId==SET_THRESHOLD_DISTANCEGUITRIGGEREDCMD)
        {
            CRegDist* it=App::ct->distances->getObject(cmd.intParams[0]);
            if (it!=nullptr)
                it->setTreshhold(cmd.floatParams[0]);
        }
        if (cmd.cmdId==TOGGLE_SEGMENTDISPLAY_DISTANCEGUITRIGGEREDCMD)
        {
            CRegDist* it=App::ct->distances->getObject(cmd.intParams[0]);
            if (it!=nullptr)
                it->setDisplaySegment(!it->getDisplaySegment());
        }
        if (cmd.cmdId==SET_SEGMENTWIDTH_DISTANCEGUITRIGGEREDCMD)
        {
            CRegDist* it=App::ct->distances->getObject(cmd.intParams[0]);
            if (it!=nullptr)
                it->setSegmentWidth(cmd.floatParams[0]);
        }


        if (cmd.cmdId==TOGGLE_SELECTABLE_COMMONPROPGUITRIGGEREDCMD)
        {
            C3DObject* it=App::ct->objCont->getObjectFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setLocalObjectProperty(it->getLocalObjectProperty()^sim_objectproperty_selectable);
        }
        if (cmd.cmdId==TOGGLE_SELECTBASEOFMODEL_COMMONPROPGUITRIGGEREDCMD)
        {
            C3DObject* it=App::ct->objCont->getObjectFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setLocalObjectProperty(it->getLocalObjectProperty()^sim_objectproperty_selectmodelbaseinstead);
        }
        if (cmd.cmdId==TOGGLE_DONGTSHOWINSIDEMODELSELECTION_COMMONPROPGUITRIGGEREDCMD)
        {
            C3DObject* it=App::ct->objCont->getObjectFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setLocalObjectProperty(it->getLocalObjectProperty()^sim_objectproperty_dontshowasinsidemodel);
        }
        if (cmd.cmdId==TOGGLE_VIEWFITTINGIGNORED_COMMONPROPGUITRIGGEREDCMD)
        {
            C3DObject* it=App::ct->objCont->getObjectFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setIgnoredByViewFitting(!it->getIgnoredByViewFitting());
        }
        if (cmd.cmdId==APPLY_GENERALPROP_COMMONPROPGUITRIGGEREDCMD)
        {
            C3DObject* last=App::ct->objCont->getObjectFromHandle(cmd.intParams[0]);
            if (last!=nullptr)
            {
                int lastType=last->getObjectType();
                for (size_t i=1;i<cmd.intParams.size();i++)
                {
                    C3DObject* it=App::ct->objCont->getObjectFromHandle(cmd.intParams[i]);
                    if (it!=nullptr)
                    {
                        int objPropToCopy=sim_objectproperty_selectable|sim_objectproperty_selectmodelbaseinstead|sim_objectproperty_dontshowasinsidemodel|sim_objectproperty_selectinvisible|sim_objectproperty_depthinvisible|sim_objectproperty_cannotdelete|sim_objectproperty_cannotdeleteduringsim;
                        it->setLocalObjectProperty(((it->getLocalObjectProperty()|objPropToCopy)-objPropToCopy)|(last->getLocalObjectProperty()&objPropToCopy));
                        it->setIgnoredByViewFitting(last->getIgnoredByViewFitting());
                        it->setSizeFactor(last->getSizeFactor());
                        if (it->getObjectType()==lastType)
                            it->setExtensionString(last->getExtensionString().c_str());
                    }
                }
            }
        }
        if (cmd.cmdId==SET_CAMERAVISIBILITYLAYERS_COMMONPROPGUITRIGGEREDCMD)
        {
            C3DObject* it=App::ct->objCont->getObjectFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->layer=cmd.intParams[1];
        }
        if (cmd.cmdId==APPLY_VISIBILITYPROP_COMMONPROPGUITRIGGEREDCMD)
        {
            C3DObject* last=App::ct->objCont->getObjectFromHandle(cmd.intParams[0]);
            if (last!=nullptr)
            {
                for (size_t i=1;i<cmd.intParams.size();i++)
                {
                    C3DObject* it=App::ct->objCont->getObjectFromHandle(cmd.intParams[i]);
                    if (it!=nullptr)
                    {
                        it->layer=last->layer;
                        it->setAuthorizedViewableObjects(last->getAuthorizedViewableObjects());
                    }
                }
            }
        }
        if (cmd.cmdId==TOGGLE_MODELBASE_COMMONPROPGUITRIGGEREDCMD)
        {
            C3DObject* it=App::ct->objCont->getObjectFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setModelBase(!it->getModelBase());
        }
        if (cmd.cmdId==TOGGLE_COLLIDABLE_COMMONPROPGUITRIGGEREDCMD)
        {
            C3DObject* it=App::ct->objCont->getObjectFromHandle(cmd.intParams[0]);
            if ((it!=nullptr)&&it->isPotentiallyCollidable())
                it->setLocalObjectSpecialProperty(it->getLocalObjectSpecialProperty()^sim_objectspecialproperty_collidable);
        }
        if (cmd.cmdId==TOGGLE_MEASURABLE_COMMONPROPGUITRIGGEREDCMD)
        {
            C3DObject* it=App::ct->objCont->getObjectFromHandle(cmd.intParams[0]);
            if ((it!=nullptr)&&it->isPotentiallyMeasurable())
                it->setLocalObjectSpecialProperty(it->getLocalObjectSpecialProperty()^sim_objectspecialproperty_measurable);
        }
        if (cmd.cmdId==TOGGLE_RENDERABLE_COMMONPROPGUITRIGGEREDCMD)
        {
            C3DObject* it=App::ct->objCont->getObjectFromHandle(cmd.intParams[0]);
            if ((it!=nullptr)&&it->isPotentiallyRenderable())
                it->setLocalObjectSpecialProperty(it->getLocalObjectSpecialProperty()^sim_objectspecialproperty_renderable);
        }
        if (cmd.cmdId==TOGGLE_CUTTABLE_COMMONPROPGUITRIGGEREDCMD)
        {
            C3DObject* it=App::ct->objCont->getObjectFromHandle(cmd.intParams[0]);
            if ((it!=nullptr)&&it->isPotentiallyCuttable())
                it->setLocalObjectSpecialProperty(it->getLocalObjectSpecialProperty()^sim_objectspecialproperty_cuttable);
        }
        if (cmd.cmdId==TOGGLE_DETECTABLE_COMMONPROPGUITRIGGEREDCMD)
        {
            C3DObject* it=App::ct->objCont->getObjectFromHandle(cmd.intParams[0]);
            if ((it!=nullptr)&&it->isPotentiallyDetectable())
            {
                int t=it->getLocalObjectSpecialProperty()&sim_objectspecialproperty_detectable_all;
                if (t==sim_objectspecialproperty_detectable_all)
                {
                    t=it->getLocalObjectSpecialProperty()|sim_objectspecialproperty_detectable_all;
                    t-=sim_objectspecialproperty_detectable_all;
                }
                else
                    t=it->getLocalObjectSpecialProperty()|sim_objectspecialproperty_detectable_all;
                it->setLocalObjectSpecialProperty(t);
            }
        }
        if (cmd.cmdId==SET_DETECTABLEITEMS_COMMONPROPGUITRIGGEREDCMD)
        {
            C3DObject* it=App::ct->objCont->getObjectFromHandle(cmd.intParams[0]);
            if ((it!=nullptr)&&it->isPotentiallyDetectable())
                it->setLocalObjectSpecialProperty(cmd.intParams[1]);
        }
        if (cmd.cmdId==APPLY_SPECIALPROP_COMMONPROPGUITRIGGEREDCMD)
        {
            C3DObject* last=App::ct->objCont->getObjectFromHandle(cmd.intParams[0]);
            if (last!=nullptr)
            {
                int settableBits=0;
                if (last->isPotentiallyCollidable())
                    settableBits|=sim_objectspecialproperty_collidable;
                if (last->isPotentiallyMeasurable())
                    settableBits|=sim_objectspecialproperty_measurable;
                if (last->isPotentiallyDetectable())
                    settableBits|=sim_objectspecialproperty_detectable_all;
                if (last->isPotentiallyRenderable())
                    settableBits|=sim_objectspecialproperty_renderable;
                if (last->isPotentiallyCuttable())
                    settableBits|=sim_objectspecialproperty_cuttable;
                int stateOfSettableBits=last->getLocalObjectSpecialProperty()&settableBits;
                for (size_t i=1;i<cmd.intParams.size();i++)
                {
                    C3DObject* it=App::ct->objCont->getObjectFromHandle(cmd.intParams[i]);
                    if (it!=nullptr)
                    {
                        int objProp=((it->getLocalObjectSpecialProperty()|settableBits)-settableBits)|stateOfSettableBits;
                        it->setLocalObjectSpecialProperty(objProp);
                    }
                }
            }
        }
        if (cmd.cmdId==TOGGLE_CANTRANSFERDNA_COMMONPROPGUITRIGGEREDCMD)
        {
            C3DObject* it=App::ct->objCont->getObjectFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
            {
                int p=it->getLocalObjectProperty();
                if (p&sim_objectproperty_canupdatedna)
                    it->setLocalObjectProperty(p-sim_objectproperty_canupdatedna);
                else
                {
                    it->setLocalObjectProperty(p|sim_objectproperty_canupdatedna);
                    it->generateDnaString();
                }
            }
        }
        if (cmd.cmdId==TOGGLE_SELECTINVISIBLE_COMMONPROPGUITRIGGEREDCMD)
        {
            C3DObject* it=App::ct->objCont->getObjectFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setLocalObjectProperty(it->getLocalObjectProperty()^sim_objectproperty_selectinvisible);
        }
        if (cmd.cmdId==TOGGLE_DEPTHMAPIGNORED_COMMONPROPGUITRIGGEREDCMD)
        {
            C3DObject* it=App::ct->objCont->getObjectFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setLocalObjectProperty(it->getLocalObjectProperty()^sim_objectproperty_depthinvisible);
        }
        if (cmd.cmdId==TOGGLE_CANNOTBEDELETED_COMMONPROPGUITRIGGEREDCMD)
        {
            C3DObject* it=App::ct->objCont->getObjectFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setLocalObjectProperty(it->getLocalObjectProperty()^sim_objectproperty_cannotdelete);
        }
        if (cmd.cmdId==TOGGLE_CANNOTBEDELETEDDURINGSIMULATION_COMMONPROPGUITRIGGEREDCMD)
        {
            C3DObject* it=App::ct->objCont->getObjectFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setLocalObjectProperty(it->getLocalObjectProperty()^sim_objectproperty_cannotdeleteduringsim);
        }
        if (cmd.cmdId==SET_SELFCOLLISIONINDICATOR_COMMONPROPGUITRIGGEREDCMD)
        {
            C3DObject* it=App::ct->objCont->getObjectFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setCollectionSelfCollisionIndicator(cmd.intParams[1]);
        }
        if (cmd.cmdId==SET_CANBESEENBY_COMMONPROPGUITRIGGEREDCMD)
        {
            C3DObject* it=App::ct->objCont->getObjectFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setAuthorizedViewableObjects(cmd.intParams[1]);
        }
        if (cmd.cmdId==SET_EXTENSIONSTRING_COMMONPROPGUITRIGGEREDCMD)
        {
            C3DObject* it=App::ct->objCont->getObjectFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setExtensionString(cmd.stringParams[0].c_str());
        }


        if (cmd.cmdId==SCALE_SCALINGGUITRIGGEREDCMD)
        {
            if (cmd.intParams.size()>0)
                CSceneObjectOperations::scaleObjects(cmd.intParams,cmd.floatParams[0],!cmd.boolParams[0]);
        }


        if (cmd.cmdId==SET_OVERRIDEPROPANDACKNOWLEDGMENT_MODELGUITRIGGEREDCMD)
        {
            C3DObject* it=App::ct->objCont->getObjectFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
            {
                it->setLocalModelProperty(cmd.intParams[1]);
                it->setModelAcknowledgement(cmd.stringParams[0]);
            }
        }


        if (cmd.cmdId==SET_TIMESTEP_SIMULATIONGUITRIGGEREDCMD)
        {
            if (App::ct->simulation->isSimulationStopped())
                App::ct->simulation->setSimulationTimeStep_raw_ns(cmd.uint64Params[0]);
        }
        if (cmd.cmdId==SET_PPF_SIMULATIONGUITRIGGEREDCMD)
        {
            if (App::ct->simulation->isSimulationStopped())
                App::ct->simulation->setSimulationPassesPerRendering_raw(cmd.intParams[0]);
        }
        if (cmd.cmdId==TOGGLE_REALTIME_SIMULATIONGUITRIGGEREDCMD)
        {
            if (App::ct->simulation->isSimulationStopped())
                App::ct->simulation->setRealTimeSimulation(!App::ct->simulation->getRealTimeSimulation());
        }
        if (cmd.cmdId==SET_REALTIMEFACTOR_SIMULATIONGUITRIGGEREDCMD)
        {
            if (App::ct->simulation->isSimulationStopped())
                App::ct->simulation->setRealTimeCoefficient_raw(cmd.floatParams[0]);
        }
        if (cmd.cmdId==TOGGLE_TRYCATCHINGUP_SIMULATIONGUITRIGGEREDCMD)
        {
            if (App::ct->simulation->isSimulationStopped())
                App::ct->simulation->setCatchUpIfLate(!App::ct->simulation->getCatchUpIfLate());
        }
        if (cmd.cmdId==TOGGLE_PAUSEATTIME_SIMULATIONGUITRIGGEREDCMD)
        {
            if (App::ct->simulation->isSimulationStopped())
                App::ct->simulation->setPauseAtSpecificTime(!App::ct->simulation->getPauseAtSpecificTime());
        }
        if (cmd.cmdId==SET_PAUSETIME_SIMULATIONGUITRIGGEREDCMD)
        {
            if (App::ct->simulation->isSimulationStopped())
                App::ct->simulation->setPauseTime_ns(cmd.uint64Params[0]);
        }
        if (cmd.cmdId==TOGGLE_PAUSEATERROR_SIMULATIONGUITRIGGEREDCMD)
        {
            if (App::ct->simulation->isSimulationStopped())
                App::ct->simulation->setPauseAtError(!App::ct->simulation->getPauseAtError());
        }
        if (cmd.cmdId==TOGGLE_RESETSCENETOINITIAL_SIMULATIONGUITRIGGEREDCMD)
        {
            App::ct->simulation->setResetSceneAtSimulationEnd(!App::ct->simulation->getResetSceneAtSimulationEnd());
        }
        if (cmd.cmdId==TOGGLE_REMOVENEWOBJECTS_SIMULATIONGUITRIGGEREDCMD)
        {
            App::ct->simulation->setRemoveNewObjectsAtSimulationEnd(!App::ct->simulation->getRemoveNewObjectsAtSimulationEnd());
        }
        if (cmd.cmdId==TOGGLE_FULLSCREENATSTART_SIMULATIONGUITRIGGEREDCMD)
        {
            if (App::ct->simulation->isSimulationStopped())
                App::ct->simulation->setFullscreenAtSimulationStart(!App::ct->simulation->getFullscreenAtSimulationStart());
        }
        if (cmd.cmdId==SET_TIMESTEPSCHEME_SIMULATIONGUITRIGGEREDCMD)
        {
            if (App::ct->simulation->isSimulationStopped())
                App::ct->simulation->setDefaultSimulationParameterIndex(cmd.intParams[0]);
        }


        if (cmd.cmdId==TOGGLE_DYNAMICS_DYNAMICSGUITRIGGEREDCMD)
        {
            App::ct->dynamicsContainer->setDynamicsEnabled(!App::ct->dynamicsContainer->getDynamicsEnabled());
            App::ct->dynamicsContainer->removeWorld();
        }
        if (cmd.cmdId==SET_ENGINE_DYNAMICSGUITRIGGEREDCMD)
        {
            if (App::ct->simulation->isSimulationStopped())
                App::ct->dynamicsContainer->setDynamicEngineType(cmd.intParams[0],cmd.intParams[1]);
        }
        if (cmd.cmdId==TOGGLE_DISPLAYCONTACTS_DYNAMICSGUITRIGGEREDCMD)
        {
            App::ct->dynamicsContainer->setDisplayContactPoints(!App::ct->dynamicsContainer->getDisplayContactPoints());
        }
        if (cmd.cmdId==SET_GRAVITY_DYNAMICSGUITRIGGEREDCMD)
        {
            App::ct->dynamicsContainer->setGravity(cmd.posParams[0]);
        }
        if (cmd.cmdId==SET_ALLGLOBALPARAMS_DYNAMICSGUITRIGGEREDCMD)
        {
            App::ct->dynamicsContainer->setUseDynamicDefaultCalculationParameters(cmd.intParams[0]);
            App::ct->dynamicsContainer->setBulletIntParams(cmd.intVectorParams[0],false);
            App::ct->dynamicsContainer->setBulletFloatParams(cmd.floatVectorParams[0],false);
            App::ct->dynamicsContainer->setOdeIntParams(cmd.intVectorParams[1],false);
            App::ct->dynamicsContainer->setOdeFloatParams(cmd.floatVectorParams[1],false);
            App::ct->dynamicsContainer->setVortexIntParams(cmd.intVectorParams[2],false);
            App::ct->dynamicsContainer->setVortexFloatParams(cmd.floatVectorParams[2],false);
            App::ct->dynamicsContainer->setNewtonIntParams(cmd.intVectorParams[3],false);
            App::ct->dynamicsContainer->setNewtonFloatParams(cmd.floatVectorParams[3],false);
        }




        if (cmd.cmdId==TOGGLE_ENABLEALL_PROXSENSORGUITRIGGEREDCMD)
        {
            App::ct->mainSettings->proximitySensorsEnabled=!App::ct->mainSettings->proximitySensorsEnabled;
        }
        if (cmd.cmdId==TOGGLE_EXPLICITHANDLING_PROXSENSORGUITRIGGEREDCMD)
        {
            CProxSensor* it=App::ct->objCont->getProximitySensor(cmd.intParams[0]);
            if (it!=nullptr)
                it->setExplicitHandling(!it->getExplicitHandling());
        }
        if (cmd.cmdId==SET_SENSORSUBTYPE_PROXSENSORGUITRIGGEREDCMD)
        {
            CProxSensor* it=App::ct->objCont->getProximitySensor(cmd.intParams[0]);
            if (it!=nullptr)
                it->setSensableType(cmd.intParams[1]);
        }
        if (cmd.cmdId==TOGGLE_SHOWVOLWHENDETECTING_PROXSENSORGUITRIGGEREDCMD)
        {
            CProxSensor* it=App::ct->objCont->getProximitySensor(cmd.intParams[0]);
            if (it!=nullptr)
                it->setShowVolumeWhenDetecting(!it->getShowVolumeWhenDetecting());
        }
        if (cmd.cmdId==TOGGLE_SHOWVOLWHENNOTDETECTING_PROXSENSORGUITRIGGEREDCMD)
        {
            CProxSensor* it=App::ct->objCont->getProximitySensor(cmd.intParams[0]);
            if (it!=nullptr)
                it->setShowVolumeWhenNotDetecting(!it->getShowVolumeWhenNotDetecting());
        }
        if (cmd.cmdId==SET_POINTSIZE_PROXSENSORGUITRIGGEREDCMD)
        {
            CProxSensor* it=App::ct->objCont->getProximitySensor(cmd.intParams[0]);
            if (it!=nullptr)
                it->setSize(cmd.floatParams[0]);
        }
        if (cmd.cmdId==APPLY_MAINPROP_PROXSENSORGUITRIGGEREDCMD)
        {
            CProxSensor* last=App::ct->objCont->getProximitySensor(cmd.intParams[0]);
            if (last!=nullptr)
            {
                for (size_t i=1;i<cmd.intParams.size();i++)
                {
                    CProxSensor* it=App::ct->objCont->getProximitySensor(cmd.intParams[i]);
                    if (it!=nullptr)
                    {
                        it->setSensableType(last->getSensableType());
                        it->setExplicitHandling(last->getExplicitHandling());
                        if (last->getRandomizedDetection())
                        {
                            it->setRandomizedDetectionSampleCount(last->getRandomizedDetectionSampleCount());
                            it->setRandomizedDetectionCountForDetection(last->getRandomizedDetectionCountForDetection());
                        }
                        it->setFrontFaceDetection(last->getFrontFaceDetection());
                        it->setBackFaceDetection(last->getBackFaceDetection());
                        it->setClosestObjectMode(last->getClosestObjectMode());
                        it->setNormalCheck(last->getNormalCheck());
                        it->setAllowedNormal(last->getAllowedNormal());
//                        it->setCheckOcclusions(last->getCheckOcclusions());
                        it->convexVolume->setSmallestDistanceEnabled(last->convexVolume->getSmallestDistanceEnabled());
                        it->convexVolume->setSmallestDistanceAllowed(last->convexVolume->getSmallestDistanceAllowed());
                        it->setSensableObject(last->getSensableObject());
                    }
                }
            }
        }
        if (cmd.cmdId==SET_ENTITYTODETECT_PROXSENSORGUITRIGGEREDCMD)
        {
            CProxSensor* it=App::ct->objCont->getProximitySensor(cmd.intParams[0]);
            if (it!=nullptr)
                it->setSensableObject(cmd.intParams[1]);
        }
        if (cmd.cmdId==APPLY_VISUALPROP_PROXSENSORGUITRIGGEREDCMD)
        {
            CProxSensor* it=App::ct->objCont->getProximitySensor(cmd.intParams[0]);
            if (it!=nullptr)
            {
                for (size_t i=1;i<cmd.intParams.size();i++)
                {
                    CProxSensor* it2=App::ct->objCont->getProximitySensor(cmd.intParams[i]);
                    if (it2!=nullptr)
                    {
                        if (it->getObjectHandle()!=it2->getObjectHandle())
                        {
                            it->getColor(0)->copyYourselfInto(it2->getColor(0));
                            it->getColor(1)->copyYourselfInto(it2->getColor(1));
                            it->getColor(2)->copyYourselfInto(it2->getColor(2));
                            it->getColor(3)->copyYourselfInto(it2->getColor(3));
                            it2->setSize(it->getSize());
                            it2->setShowVolumeWhenNotDetecting(it->getShowVolumeWhenNotDetecting());
                            it2->setShowVolumeWhenDetecting(it->getShowVolumeWhenDetecting());
                        }
                    }
                }
            }
        }
        if (cmd.cmdId==SET_DETECTIONPARAMS_PROXSENSORGUITRIGGEREDCMD)
        {
            CProxSensor* it=App::ct->objCont->getProximitySensor(cmd.intParams[0]);
            if (it!=nullptr)
            {
                it->setFrontFaceDetection(cmd.boolParams[0]);
                it->setBackFaceDetection(cmd.boolParams[1]);
                it->setClosestObjectMode(cmd.boolParams[2]);
                it->setNormalCheck(cmd.boolParams[3]);
                it->setAllowedNormal(cmd.floatParams[0]);
//                it->setCheckOcclusions(cmd.boolParams[4]);
                it->convexVolume->setSmallestDistanceEnabled(cmd.boolParams[5]);
                it->convexVolume->setSmallestDistanceAllowed(cmd.floatParams[1]);
                if (it->getRandomizedDetection())
                {
                    it->setRandomizedDetectionSampleCount(cmd.intParams[1]);
                    it->setRandomizedDetectionCountForDetection(cmd.intParams[2]);
                }
            }
        }




        if (cmd.cmdId==SET_TYPE_DETECTIONVOLUMEGUITRIGGEREDCMD)
        {
            CProxSensor* prox=App::ct->objCont->getProximitySensor(cmd.intParams[0]);
            CMill* mill=App::ct->objCont->getMill(cmd.intParams[0]);
            if (prox!=nullptr)
            {
                prox->setSensorType(cmd.intParams[1]);
                prox->setRandomizedDetection(cmd.boolParams[0]);
            }
            if (mill!=nullptr)
                mill->setMillType(cmd.intParams[1]);
        }
        if (cmd.cmdId==SET_OFFSET_DETECTIONVOLUMEGUITRIGGEREDCMD)
        {
            CProxSensor* prox=App::ct->objCont->getProximitySensor(cmd.intParams[0]);
            CMill* mill=App::ct->objCont->getMill(cmd.intParams[0]);
            if (prox!=nullptr)
                prox->convexVolume->setOffset(cmd.floatParams[0]);
            if (mill!=nullptr)
                mill->convexVolume->setOffset(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_RADIUS_DETECTIONVOLUMEGUITRIGGEREDCMD)
        {
            CProxSensor* prox=App::ct->objCont->getProximitySensor(cmd.intParams[0]);
            CMill* mill=App::ct->objCont->getMill(cmd.intParams[0]);
            if (prox!=nullptr)
                prox->convexVolume->setRadius(cmd.floatParams[0]);
            if (mill!=nullptr)
                mill->convexVolume->setRadius(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_RADIUSFAR_DETECTIONVOLUMEGUITRIGGEREDCMD)
        {
            CProxSensor* prox=App::ct->objCont->getProximitySensor(cmd.intParams[0]);
            CMill* mill=App::ct->objCont->getMill(cmd.intParams[0]);
            if (prox!=nullptr)
                prox->convexVolume->setRadiusFar(cmd.floatParams[0]);
            if (mill!=nullptr)
                mill->convexVolume->setRadiusFar(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_RANGE_DETECTIONVOLUMEGUITRIGGEREDCMD)
        {
            CProxSensor* prox=App::ct->objCont->getProximitySensor(cmd.intParams[0]);
            CMill* mill=App::ct->objCont->getMill(cmd.intParams[0]);
            if (prox!=nullptr)
                prox->convexVolume->setRange(cmd.floatParams[0]);
            if (mill!=nullptr)
                mill->convexVolume->setRange(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_XSIZE_DETECTIONVOLUMEGUITRIGGEREDCMD)
        {
            CProxSensor* prox=App::ct->objCont->getProximitySensor(cmd.intParams[0]);
            CMill* mill=App::ct->objCont->getMill(cmd.intParams[0]);
            if (prox!=nullptr)
                prox->convexVolume->setXSize(cmd.floatParams[0]);
            if (mill!=nullptr)
                mill->convexVolume->setXSize(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_YSIZE_DETECTIONVOLUMEGUITRIGGEREDCMD)
        {
            CProxSensor* prox=App::ct->objCont->getProximitySensor(cmd.intParams[0]);
            CMill* mill=App::ct->objCont->getMill(cmd.intParams[0]);
            if (prox!=nullptr)
                prox->convexVolume->setYSize(cmd.floatParams[0]);
            if (mill!=nullptr)
                mill->convexVolume->setYSize(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_XSIZEFAR_DETECTIONVOLUMEGUITRIGGEREDCMD)
        {
            CProxSensor* prox=App::ct->objCont->getProximitySensor(cmd.intParams[0]);
            CMill* mill=App::ct->objCont->getMill(cmd.intParams[0]);
            if (prox!=nullptr)
                prox->convexVolume->setXSizeFar(cmd.floatParams[0]);
            if (mill!=nullptr)
                mill->convexVolume->setXSizeFar(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_YSIZEFAR_DETECTIONVOLUMEGUITRIGGEREDCMD)
        {
            CProxSensor* prox=App::ct->objCont->getProximitySensor(cmd.intParams[0]);
            CMill* mill=App::ct->objCont->getMill(cmd.intParams[0]);
            if (prox!=nullptr)
                prox->convexVolume->setYSizeFar(cmd.floatParams[0]);
            if (mill!=nullptr)
                mill->convexVolume->setYSizeFar(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_ANGLE_DETECTIONVOLUMEGUITRIGGEREDCMD)
        {
            CProxSensor* prox=App::ct->objCont->getProximitySensor(cmd.intParams[0]);
            CMill* mill=App::ct->objCont->getMill(cmd.intParams[0]);
            if (prox!=nullptr)
                prox->convexVolume->setAngle(cmd.floatParams[0]);
            if (mill!=nullptr)
                mill->convexVolume->setAngle(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_FACECOUNT_DETECTIONVOLUMEGUITRIGGEREDCMD)
        {
            CProxSensor* prox=App::ct->objCont->getProximitySensor(cmd.intParams[0]);
            CMill* mill=App::ct->objCont->getMill(cmd.intParams[0]);
            if (prox!=nullptr)
                prox->convexVolume->setFaceNumber(cmd.intParams[1]);
            if (mill!=nullptr)
                mill->convexVolume->setFaceNumber(cmd.intParams[1]);
        }
        if (cmd.cmdId==SET_FACECOUNTFAR_DETECTIONVOLUMEGUITRIGGEREDCMD)
        {
            CProxSensor* prox=App::ct->objCont->getProximitySensor(cmd.intParams[0]);
            CMill* mill=App::ct->objCont->getMill(cmd.intParams[0]);
            if (prox!=nullptr)
                prox->convexVolume->setFaceNumberFar(cmd.intParams[1]);
            if (mill!=nullptr)
                mill->convexVolume->setFaceNumberFar(cmd.intParams[1]);
        }
        if (cmd.cmdId==SET_SUBDIVISIONS_DETECTIONVOLUMEGUITRIGGEREDCMD)
        {
            CProxSensor* prox=App::ct->objCont->getProximitySensor(cmd.intParams[0]);
            CMill* mill=App::ct->objCont->getMill(cmd.intParams[0]);
            if (prox!=nullptr)
                prox->convexVolume->setSubdivisions(cmd.intParams[1]);
            if (mill!=nullptr)
                mill->convexVolume->setSubdivisions(cmd.intParams[1]);
        }
        if (cmd.cmdId==SET_SUBDIVISIONSFAR_DETECTIONVOLUMEGUITRIGGEREDCMD)
        {
            CProxSensor* prox=App::ct->objCont->getProximitySensor(cmd.intParams[0]);
            CMill* mill=App::ct->objCont->getMill(cmd.intParams[0]);
            if (prox!=nullptr)
                prox->convexVolume->setSubdivisionsFar(cmd.intParams[1]);
            if (mill!=nullptr)
                mill->convexVolume->setSubdivisionsFar(cmd.intParams[1]);
        }
        if (cmd.cmdId==SET_INSIDEGAP_DETECTIONVOLUMEGUITRIGGEREDCMD)
        {
            CProxSensor* prox=App::ct->objCont->getProximitySensor(cmd.intParams[0]);
            CMill* mill=App::ct->objCont->getMill(cmd.intParams[0]);
            if (prox!=nullptr)
                prox->convexVolume->setInsideAngleThing(cmd.floatParams[0]);
            if (mill!=nullptr)
                mill->convexVolume->setInsideAngleThing(cmd.floatParams[0]);
        }
        if (cmd.cmdId==APPLY_DETECTIONVOLUMEGUITRIGGEREDCMD)
        {
            CProxSensor* prox=App::ct->objCont->getProximitySensor(cmd.intParams[0]);
            CMill* mill=App::ct->objCont->getMill(cmd.intParams[0]);
            if (prox!=nullptr)
            {
                for (size_t i=1;i<cmd.intParams.size();i++)
                {
                    CProxSensor* it=App::ct->objCont->getProximitySensor(cmd.intParams[i]);
                    if (it!=nullptr)
                    {
                        float w=it->convexVolume->getSmallestDistanceAllowed();
                        bool ww=it->convexVolume->getSmallestDistanceEnabled();
                        it->convexVolume->disableVolumeComputation(true);
                        // Volume parameters:
                        it->setSensorType(prox->getSensorType());
                        it->setRandomizedDetection(false); // somehow needed for next to work always...??
                        it->setRandomizedDetection(prox->getRandomizedDetection());
                        it->convexVolume->setOffset(prox->convexVolume->getOffset());
                        it->convexVolume->setRange(prox->convexVolume->getRange());
                        it->convexVolume->setXSize(prox->convexVolume->getXSize());
                        it->convexVolume->setYSize(prox->convexVolume->getYSize());
                        it->convexVolume->setXSizeFar(prox->convexVolume->getXSizeFar());
                        it->convexVolume->setYSizeFar(prox->convexVolume->getYSizeFar());
                        it->convexVolume->setRadius(prox->convexVolume->getRadius());
                        it->convexVolume->setRadiusFar(prox->convexVolume->getRadiusFar());
                        it->convexVolume->setAngle(prox->convexVolume->getAngle());
                        it->convexVolume->setFaceNumber(prox->convexVolume->getFaceNumber());
                        it->convexVolume->setFaceNumberFar(prox->convexVolume->getFaceNumberFar());
                        it->convexVolume->setSubdivisions(prox->convexVolume->getSubdivisions());
                        it->convexVolume->setSubdivisionsFar(prox->convexVolume->getSubdivisionsFar());
                        it->convexVolume->setInsideAngleThing(prox->convexVolume->getInsideAngleThing());
                        // We have to recompute the planes:
                        it->convexVolume->disableVolumeComputation(false);
                        it->convexVolume->computeVolumes();
                        it->convexVolume->setSmallestDistanceAllowed(w);
                        it->convexVolume->setSmallestDistanceEnabled(ww);
                    }
                }
            }
            if (mill!=nullptr)
            {
                for (size_t i=1;i<cmd.intParams.size();i++)
                {
                    CMill* it=App::ct->objCont->getMill(cmd.intParams[i]);
                    if (it!=nullptr)
                    {
                        it->convexVolume->disableVolumeComputation(true);
                        // Volume parameters:
                        it->setMillType(mill->getMillType());
                        it->convexVolume->setOffset(mill->convexVolume->getOffset());
                        it->convexVolume->setRange(mill->convexVolume->getRange());
                        it->convexVolume->setXSize(mill->convexVolume->getXSize());
                        it->convexVolume->setYSize(mill->convexVolume->getYSize());
                        it->convexVolume->setXSizeFar(mill->convexVolume->getXSizeFar());
                        it->convexVolume->setYSizeFar(mill->convexVolume->getYSizeFar());
                        it->convexVolume->setRadius(mill->convexVolume->getRadius());
                        it->convexVolume->setRadiusFar(mill->convexVolume->getRadiusFar());
                        it->convexVolume->setAngle(mill->convexVolume->getAngle());
                        it->convexVolume->setFaceNumber(mill->convexVolume->getFaceNumber());
                        it->convexVolume->setFaceNumberFar(mill->convexVolume->getFaceNumberFar());
                        it->convexVolume->setSubdivisionsFar(mill->convexVolume->getSubdivisionsFar());
                        // We have to recompute the planes:
                        it->convexVolume->disableVolumeComputation(false);
                        it->convexVolume->computeVolumes();
                    }
                }
            }
        }




        if (cmd.cmdId==TOGGLE_ENABLEALL_MILLGUITRIGGEREDCMD)
        {
            App::ct->mainSettings->millsEnabled=!App::ct->mainSettings->millsEnabled;
        }
        if (cmd.cmdId==TOGGLE_EXPLICITHANDLING_MILLGUITRIGGEREDCMD)
        {
            CMill* it=App::ct->objCont->getMill(cmd.intParams[0]);
            if (it!=nullptr)
                it->setExplicitHandling(!it->getExplicitHandling());
        }
        if (cmd.cmdId==SET_SIZE_MILLGUITRIGGEREDCMD)
        {
            CMill* it=App::ct->objCont->getMill(cmd.intParams[0]);
            if (it!=nullptr)
                it->setSize(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_ENTITYTOCUT_MILLGUITRIGGEREDCMD)
        {
            CMill* it=App::ct->objCont->getMill(cmd.intParams[0]);
            if (it!=nullptr)
                it->setMillableObject(cmd.intParams[1]);
        }



        if (cmd.cmdId==TOGGLE_ENABLED_LIGHTGUITRIGGEREDCMD)
        {
            CLight* it=App::ct->objCont->getLight(cmd.intParams[0]);
            if (it!=nullptr)
                it->setLightActive(!it->getLightActive());
        }
        if (cmd.cmdId==TOGGLE_LIGHTISLOCAL_LIGHTGUITRIGGEREDCMD)
        {
            CLight* it=App::ct->objCont->getLight(cmd.intParams[0]);
            if (it!=nullptr)
                it->setLightIsLocal(!it->getLightIsLocal());
        }
        if (cmd.cmdId==SET_SIZE_LIGHTGUITRIGGEREDCMD)
        {
            CLight* it=App::ct->objCont->getLight(cmd.intParams[0]);
            if (it!=nullptr)
                it->setLightSize(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_SPOTCUTOFF_LIGHTGUITRIGGEREDCMD)
        {
            CLight* it=App::ct->objCont->getLight(cmd.intParams[0]);
            if (it!=nullptr)
                it->setSpotCutoffAngle(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_SPOTEXPONENT_LIGHTGUITRIGGEREDCMD)
        {
            CLight* it=App::ct->objCont->getLight(cmd.intParams[0]);
            if (it!=nullptr)
                it->setSpotExponent(cmd.intParams[1]);
        }
        if (cmd.cmdId==SET_CONSTATTENUATION_LIGHTGUITRIGGEREDCMD)
        {
            CLight* it=App::ct->objCont->getLight(cmd.intParams[0]);
            if (it!=nullptr)
                it->setAttenuationFactor(cmd.floatParams[0],CONSTANT_ATTENUATION);
        }
        if (cmd.cmdId==SET_LINATTENUATION_LIGHTGUITRIGGEREDCMD)
        {
            CLight* it=App::ct->objCont->getLight(cmd.intParams[0]);
            if (it!=nullptr)
                it->setAttenuationFactor(cmd.floatParams[0],LINEAR_ATTENUATION);
        }
        if (cmd.cmdId==SET_QUADATTENUATION_LIGHTGUITRIGGEREDCMD)
        {
            CLight* it=App::ct->objCont->getLight(cmd.intParams[0]);
            if (it!=nullptr)
                it->setAttenuationFactor(cmd.floatParams[0],QUADRATIC_ATTENUATION);
        }



        if (cmd.cmdId==SET_SIZE_DUMMYGUITRIGGEREDCMD)
        {
            CDummy* it=App::ct->objCont->getDummy(cmd.intParams[0]);
            if (it!=nullptr)
                it->setSize(cmd.floatParams[0]);
        }
        if (cmd.cmdId==APPLY_VISUALPROP_DUMMYGUITRIGGEREDCMD)
        {
            CDummy* it=App::ct->objCont->getDummy(cmd.intParams[0]);
            if (it!=nullptr)
            {
                for (size_t i=1;i<cmd.intParams.size();i++)
                {
                    CDummy* it2=App::ct->objCont->getDummy(cmd.intParams[i]);
                    if (it2!=nullptr)
                    {
                        it->getColor()->copyYourselfInto(it2->getColor());
                        it2->setSize(it->getSize());
                    }
                }
            }
        }
        if (cmd.cmdId==SET_LINKEDDUMMY_DUMMYGUITRIGGEREDCMD)
        {
            CDummy* it=App::ct->objCont->getDummy(cmd.intParams[0]);
            if (it!=nullptr)
            {
                CDummy* it2=App::ct->objCont->getDummy(cmd.intParams[1]);
                if (it2!=nullptr)
                    it->setLinkedDummyID(it2->getObjectHandle(),false);
                else
                    it->setLinkedDummyID(-1,false);
            }
        }
        if (cmd.cmdId==SET_LINKTYPE_DUMMYGUITRIGGEREDCMD)
        {
            CDummy* it=App::ct->objCont->getDummy(cmd.intParams[0]);
            if (it!=nullptr)
                it->setLinkType(cmd.intParams[1],false);
        }
        if (cmd.cmdId==TOGGLE_FOLLOWORIENTATION_DUMMYGUITRIGGEREDCMD)
        {
            CDummy* it=App::ct->objCont->getDummy(cmd.intParams[0]);
            if (it!=nullptr)
                it->setAssignedToParentPathOrientation(!it->getAssignedToParentPathOrientation());
        }
        if (cmd.cmdId==TOGGLE_FOLLOWPOSITION_DUMMYGUITRIGGEREDCMD)
        {
            CDummy* it=App::ct->objCont->getDummy(cmd.intParams[0]);
            if (it!=nullptr)
                it->setAssignedToParentPath(!it->getAssignedToParentPath());
        }
        if (cmd.cmdId==SET_FREEORFIXEDONPATH_DUMMYGUITRIGGEREDCMD)
        {
            CDummy* it=App::ct->objCont->getDummy(cmd.intParams[0]);
            if (it!=nullptr)
                it->setFreeOnPathTrajectory(cmd.intParams[1]!=0);
        }
        if (cmd.cmdId==SET_OFFSET_DUMMYGUITRIGGEREDCMD)
        {
            CDummy* it=App::ct->objCont->getDummy(cmd.intParams[0]);
            if (it!=nullptr)
                it->setVirtualDistanceOffsetOnPath(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_COPYINCREMENT_DUMMYGUITRIGGEREDCMD)
        {
            CDummy* it=App::ct->objCont->getDummy(cmd.intParams[0]);
            if (it!=nullptr)
                it->setVirtualDistanceOffsetOnPath_variationWhenCopy(cmd.floatParams[0]);
        }



        if (cmd.cmdId==TOGGLE_ENABLED_MIRRORGUITRIGGEREDCMD)
        {
            CMirror* it=App::ct->objCont->getMirror(cmd.intParams[0]);
            if (it!=nullptr)
                it->setActive(!it->getActive());
        }
        if (cmd.cmdId==SET_WIDTH_MIRRORGUITRIGGEREDCMD)
        {
            CMirror* it=App::ct->objCont->getMirror(cmd.intParams[0]);
            if (it!=nullptr)
                it->setMirrorWidth(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_HEIGHT_MIRRORGUITRIGGEREDCMD)
        {
            CMirror* it=App::ct->objCont->getMirror(cmd.intParams[0]);
            if (it!=nullptr)
                it->setMirrorHeight(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_REFLECTANCE_MIRRORGUITRIGGEREDCMD)
        {
            CMirror* it=App::ct->objCont->getMirror(cmd.intParams[0]);
            if (it!=nullptr)
                it->setReflectance(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_MIRRORFUNC_MIRRORGUITRIGGEREDCMD)
        {
            CMirror* it=App::ct->objCont->getMirror(cmd.intParams[0]);
            if (it!=nullptr)
                it->setIsMirror(true);
        }
        if (cmd.cmdId==SET_CLIPPINGFUNC_MIRRORGUITRIGGEREDCMD)
        {
            CMirror* it=App::ct->objCont->getMirror(cmd.intParams[0]);
            if (it!=nullptr)
                it->setIsMirror(false);
        }
        if (cmd.cmdId==SET_CLIPPINGENTITY_MIRRORGUITRIGGEREDCMD)
        {
            CMirror* it=App::ct->objCont->getMirror(cmd.intParams[0]);
            if (it!=nullptr)
                it->setClippingObjectOrCollection(cmd.intParams[1]);
        }
        if (cmd.cmdId==TOGGLE_DISABLEALLCLIPPING_MIRRORGUITRIGGEREDCMD)
        {
            App::ct->mainSettings->clippingPlanesDisabled=!App::ct->mainSettings->clippingPlanesDisabled;
        }
        if (cmd.cmdId==TOGGLE_DISABLEALLMIRRORS_MIRRORGUITRIGGEREDCMD)
        {
            App::ct->mainSettings->mirrorsDisabled=!App::ct->mainSettings->mirrorsDisabled;
        }
        if (cmd.cmdId==SET_MAXTRIANGLESIZE_ENVIRONMENTGUITRIGGEREDCMD)
        {
            if (App::ct->simulation->isSimulationStopped())
            {
                App::ct->environment->setCalculationMaxTriangleSize(cmd.floatParams[0]);
                for (size_t i=0;i<App::ct->objCont->shapeList.size();i++)
                {
                    CShape* sh=App::ct->objCont->getShape(App::ct->objCont->shapeList[i]);
                    sh->removeCollisionInformation();
                }
            }
        }
        if (cmd.cmdId==SET_MINTRIANGLESIZE_ENVIRONMENTGUITRIGGEREDCMD)
        {
            App::ct->environment->setCalculationMinRelTriangleSize(cmd.floatParams[0]);
            for (size_t i=0;i<App::ct->objCont->shapeList.size();i++)
            {
                CShape* sh=App::ct->objCont->getShape(App::ct->objCont->shapeList[i]);
                sh->removeCollisionInformation();
            }
        }
        if (cmd.cmdId==TOGGLE_SAVECALCSTRUCT_ENVIRONMENTGUITRIGGEREDCMD)
        {
            App::ct->environment->setSaveExistingCalculationStructures(!App::ct->environment->getSaveExistingCalculationStructures());
        }
        if (cmd.cmdId==TOGGLE_SHAPETEXTURES_ENVIRONMENTGUITRIGGEREDCMD)
        {
            App::ct->environment->setShapeTexturesEnabled(!App::ct->environment->getShapeTexturesEnabled());
        }
        if (cmd.cmdId==TOGGLE_GLUITEXTURES_ENVIRONMENTGUITRIGGEREDCMD)
        {
            App::ct->environment->set2DElementTexturesEnabled(!App::ct->environment->get2DElementTexturesEnabled());
        }
        if (cmd.cmdId==TOGGLE_LOCKSCENE_ENVIRONMENTGUITRIGGEREDCMD)
        {
            App::ct->environment->setRequestFinalSave(!App::ct->environment->getRequestFinalSave());
        }
        if (cmd.cmdId==SET_ACKNOWLEDGMENT_ENVIRONMENTGUITRIGGEREDCMD)
        {
            App::ct->environment->setAcknowledgement(cmd.stringParams[0]);
        }
        if (cmd.cmdId==CLEANUP_OBJNAMES_ENVIRONMENTGUITRIGGEREDCMD)
        {
            if (App::ct->simulation->isSimulationStopped())
                App::ct->objCont->cleanupDashNames(-1);
        }
        if (cmd.cmdId==CLEANUP_GHOSTS_ENVIRONMENTGUITRIGGEREDCMD)
        {
            App::ct->ghostObjectCont->removeGhost(-1,-1);
        }
        if (cmd.cmdId==SET_EXTSTRING_ENVIRONMENTGUITRIGGEREDCMD)
        {
            App::ct->environment->setExtensionString(cmd.stringParams[0].c_str());
        }
        if (cmd.cmdId==TOGGLE_ENABLED_FOGGUITRIGGEREDCMD)
        {
            App::ct->environment->setFogEnabled(!App::ct->environment->getFogEnabled());
        }
        if (cmd.cmdId==SET_TYPE_FOGGUITRIGGEREDCMD)
        {
            App::ct->environment->setFogType(cmd.intParams[0]);
        }
        if (cmd.cmdId==SET_START_FOGGUITRIGGEREDCMD)
        {
            App::ct->environment->setFogStart(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_END_FOGGUITRIGGEREDCMD)
        {
            App::ct->environment->setFogEnd(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_DENSITY_FOGGUITRIGGEREDCMD)
        {
            App::ct->environment->setFogDensity(cmd.floatParams[0]);
        }



        if (cmd.cmdId==SET_ITEMRGB_COLORGUITRIGGEREDCMD)
        {
            float* col=App::getRGBPointerFromItem(cmd.intParams[0],cmd.intParams[1],cmd.intParams[2],cmd.intParams[3],nullptr);
            if (col!=nullptr)
            {
                col[0]=cmd.floatParams[0];
                col[1]=cmd.floatParams[1];
                col[2]=cmd.floatParams[2];
            }
        }



        if (cmd.cmdId==SET_SHININESS_MATERIALGUITRIGGEREDCMD)
        {
            int allowedParts=0; // Bit-coded: 1=ambient/difuse, 2=diffuse(light only) 4=spec, 8=emiss., 16=aux channels, 32=pulsation, 64=shininess, 128=opacity, 256=colorName, 512=ext. string
            CVisualParam* it=App::getVisualParamPointerFromItem(cmd.intParams[0],cmd.intParams[1],cmd.intParams[2],nullptr,&allowedParts);
            if ( (it!=nullptr)&&(allowedParts&64) )
                it->shininess=tt::getLimitedInt(0,128,cmd.intParams[3]);
        }
        if (cmd.cmdId==TOGGLE_TRANSPARENCY_MATERIALGUITRIGGEREDCMD)
        {
            int allowedParts=0; // Bit-coded: 1=ambient/difuse, 2=diffuse(light only) 4=spec, 8=emiss., 16=aux channels, 32=pulsation, 64=shininess, 128=opacity, 256=colorName, 512=ext. string
            CVisualParam* it=App::getVisualParamPointerFromItem(cmd.intParams[0],cmd.intParams[1],cmd.intParams[2],nullptr,&allowedParts);
            CShape* shape=App::ct->objCont->getShape(cmd.intParams[1]);
            if ( (it!=nullptr)&&(allowedParts&128)&&(shape!=nullptr) )
            {
                it->translucid=!it->translucid;
                shape->actualizeContainsTransparentComponent();
            }
        }
        if (cmd.cmdId==SET_TRANSPARENCYFACT_MATERIALGUITRIGGEREDCMD)
        {
            int allowedParts=0; // Bit-coded: 1=ambient/difuse, 2=diffuse(light only) 4=spec, 8=emiss., 16=aux channels, 32=pulsation, 64=shininess, 128=opacity, 256=colorName, 512=ext. string
            CVisualParam* it=App::getVisualParamPointerFromItem(cmd.intParams[0],cmd.intParams[1],cmd.intParams[2],nullptr,&allowedParts);
            if ( (it!=nullptr)&&(allowedParts&128) )
                it->transparencyFactor=tt::getLimitedFloat(0.0,1.0,cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_NAME_MATERIALGUITRIGGEREDCMD)
        {
            int allowedParts=0; // Bit-coded: 1=ambient/difuse, 2=diffuse(light only) 4=spec, 8=emiss., 16=aux channels, 32=pulsation, 64=shininess, 128=opacity, 256=colorName, 512=ext. string
            CVisualParam* it=App::getVisualParamPointerFromItem(cmd.intParams[0],cmd.intParams[1],cmd.intParams[2],nullptr,&allowedParts);
            if ( (it!=nullptr)&&(allowedParts&256) )
            {
                tt::removeIllegalCharacters(cmd.stringParams[0],false);
                it->colorName=cmd.stringParams[0];
            }
        }
        if (cmd.cmdId==SET_EXTSTRING_MATERIALGUITRIGGEREDCMD)
        {
            int allowedParts=0; // Bit-coded: 1=ambient/difuse, 2=diffuse(light only) 4=spec, 8=emiss., 16=aux channels, 32=pulsation, 64=shininess, 128=opacity, 256=colorName, 512=ext. string
            CVisualParam* it=App::getVisualParamPointerFromItem(cmd.intParams[0],cmd.intParams[1],cmd.intParams[2],nullptr,&allowedParts);
            if ( (it!=nullptr)&&(allowedParts&512) )
                it->extensionString=cmd.stringParams[0];
        }
        if (cmd.cmdId==SET_PULSATIONPARAMS_MATERIALGUITRIGGEREDCMD)
        {
            int allowedParts=0; // Bit-coded: 1=ambient/difuse, 2=diffuse(light only) 4=spec, 8=emiss., 16=aux channels, 32=pulsation, 64=shininess, 128=opacity, 256=colorName, 512=ext. string
            CVisualParam* it=App::getVisualParamPointerFromItem(cmd.intParams[0],cmd.intParams[1],cmd.intParams[2],nullptr,&allowedParts);
            if ( (it!=nullptr)&&(allowedParts&32) )
            {
                it->flash=cmd.boolParams[0];
                it->useSimulationTime=cmd.boolParams[1];
                it->flashFrequency=cmd.floatParams[0];
                it->flashPhase=cmd.floatParams[1];
                it->flashRatio=cmd.floatParams[2];
            }
        }



        if (cmd.cmdId==SET_VOXELSIZE_OCTREEGUITRIGGEREDCMD)
        {
            COctree* it=App::ct->objCont->getOctree(cmd.intParams[0]);
            if (it!=nullptr)
                it->setCellSize(cmd.floatParams[0]);
        }
        if (cmd.cmdId==TOGGLE_SHOWSTRUCTURE_OCTREEGUITRIGGEREDCMD)
        {
            COctree* it=App::ct->objCont->getOctree(cmd.intParams[0]);
            if (it!=nullptr)
                it->setShowOctree(!it->getShowOctree());
        }
        if (cmd.cmdId==TOGGLE_RANDOMCOLORS_OCTREEGUITRIGGEREDCMD)
        {
            COctree* it=App::ct->objCont->getOctree(cmd.intParams[0]);
            if (it!=nullptr)
                it->setUseRandomColors(!it->getUseRandomColors());
        }
        if (cmd.cmdId==TOGGLE_SHOWPOINTS_OCTREEGUITRIGGEREDCMD)
        {
            COctree* it=App::ct->objCont->getOctree(cmd.intParams[0]);
            if (it!=nullptr)
                it->setUsePointsInsteadOfCubes(!it->getUsePointsInsteadOfCubes());
        }
        if (cmd.cmdId==SET_POINTSIZE_OCTREEGUITRIGGEREDCMD)
        {
            COctree* it=App::ct->objCont->getOctree(cmd.intParams[0]);
            if (it!=nullptr)
                it->setPointSize(cmd.intParams[1]);
        }
        if (cmd.cmdId==CLEAR_OCTREEGUITRIGGEREDCMD)
        {
            COctree* it=App::ct->objCont->getOctree(cmd.intParams[0]);
            if (it!=nullptr)
                it->clear();
        }
        if (cmd.cmdId==INSERT_SELECTEDVISIBLEOBJECTS_OCTREEGUITRIGGEREDCMD)
        {
            COctree* it=App::ct->objCont->getOctree(cmd.intParams[0]);
            if (it!=nullptr)
            {
                std::vector<int> sel;
                for (size_t i=1;i<cmd.intParams.size();i++)
                    sel.push_back(cmd.intParams[i]);
                it->insertObjects(sel);
            }
        }
        if (cmd.cmdId==SUBTRACT_SELECTEDVISIBLEOBJECTS_OCTREEGUITRIGGEREDCMD)
        {
            COctree* it=App::ct->objCont->getOctree(cmd.intParams[0]);
            if (it!=nullptr)
            {
                std::vector<int> sel;
                for (size_t i=1;i<cmd.intParams.size();i++)
                    sel.push_back(cmd.intParams[i]);
                it->subtractObjects(sel);
            }
        }
        if (cmd.cmdId==TOGGLE_COLOREMISSIVE_OCTREEGUITRIGGEREDCMD)
        {
            COctree* it=App::ct->objCont->getOctree(cmd.intParams[0]);
            if (it!=nullptr)
                it->setColorIsEmissive(!it->getColorIsEmissive());
        }




        if (cmd.cmdId==SET_MAXVOXELSIZE_PTCLOUDGUITRIGGEREDCMD)
        {
            CPointCloud* it=App::ct->objCont->getPointCloud(cmd.intParams[0]);
            if (it!=nullptr)
                it->setCellSize(cmd.floatParams[0]);
        }
        if (cmd.cmdId==TOGGLE_SHOWOCTREE_PTCLOUDGUITRIGGEREDCMD)
        {
            CPointCloud* it=App::ct->objCont->getPointCloud(cmd.intParams[0]);
            if (it!=nullptr)
                it->setShowOctree(!it->getShowOctree());
        }
        if (cmd.cmdId==TOGGLE_RANDOMCOLORS_PTCLOUDGUITRIGGEREDCMD)
        {
            CPointCloud* it=App::ct->objCont->getPointCloud(cmd.intParams[0]);
            if (it!=nullptr)
                it->setUseRandomColors(!it->getUseRandomColors());
        }
        if (cmd.cmdId==SET_PTSIZE_PTCLOUDGUITRIGGEREDCMD)
        {
            CPointCloud* it=App::ct->objCont->getPointCloud(cmd.intParams[0]);
            if (it!=nullptr)
                it->setPointSize(cmd.intParams[1]);
        }
        if (cmd.cmdId==CLEAR_PTCLOUDGUITRIGGEREDCMD)
        {
            CPointCloud* it=App::ct->objCont->getPointCloud(cmd.intParams[0]);
            if (it!=nullptr)
                it->clear();
        }
        if (cmd.cmdId==INSERT_OBJECTS_PTCLOUDGUITRIGGEREDCMD)
        {
            CPointCloud* it=App::ct->objCont->getPointCloud(cmd.intParams[0]);
            if (it!=nullptr)
            {
                std::vector<int> sel;
                for (size_t i=1;i<cmd.intParams.size();i++)
                    sel.push_back(cmd.intParams[i]);
                it->insertObjects(sel);
            }
        }
        if (cmd.cmdId==SET_MAXPTCNT_PTCLOUDGUITRIGGEREDCMD)
        {
            CPointCloud* it=App::ct->objCont->getPointCloud(cmd.intParams[0]);
            if (it!=nullptr)
                it->setMaxPointCountPerCell(cmd.intParams[1]);
        }
        if (cmd.cmdId==SET_BUILDRESOLUTION_PTCLOUDGUITRIGGEREDCMD)
        {
            CPointCloud* it=App::ct->objCont->getPointCloud(cmd.intParams[0]);
            if (it!=nullptr)
                it->setBuildResolution(cmd.floatParams[0]);
        }
        if (cmd.cmdId==TOGGLE_USEOCTREE_PTCLOUDGUITRIGGEREDCMD)
        {
            CPointCloud* it=App::ct->objCont->getPointCloud(cmd.intParams[0]);
            if (it!=nullptr)
                it->setDoNotUseCalculationStructure(!it->getDoNotUseCalculationStructure());
        }
        if (cmd.cmdId==TOGGLE_EMISSIVECOLOR_PTCLOUDGUITRIGGEREDCMD)
        {
            CPointCloud* it=App::ct->objCont->getPointCloud(cmd.intParams[0]);
            if (it!=nullptr)
                it->setColorIsEmissive(!it->getColorIsEmissive());
        }
        if (cmd.cmdId==SET_DISPLAYRATIO_PTCLOUDGUITRIGGEREDCMD)
        {
            CPointCloud* it=App::ct->objCont->getPointCloud(cmd.intParams[0]);
            if (it!=nullptr)
                it->setPointDisplayRatio(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SUBTRACT_OBJECTS_PTCLOUDGUITRIGGEREDCMD)
        {
            CPointCloud* it=App::ct->objCont->getPointCloud(cmd.intParams[0]);
            if (it!=nullptr)
            {
                std::vector<int> sel;
                for (size_t i=1;i<cmd.intParams.size();i++)
                    sel.push_back(cmd.intParams[i]);
                it->subtractObjects(sel);
            }
        }
        if (cmd.cmdId==SET_SUBTRACTTOLERANCE_PTCLOUDGUITRIGGEREDCMD)
        {
            CPointCloud* it=App::ct->objCont->getPointCloud(cmd.intParams[0]);
            if (it!=nullptr)
                it->setRemovalDistanceTolerance(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_INSERTTOLERANCE_PTCLOUDGUITRIGGEREDCMD)
        {
            CPointCloud* it=App::ct->objCont->getPointCloud(cmd.intParams[0]);
            if (it!=nullptr)
                it->setInsertionDistanceTolerance(cmd.floatParams[0]);
        }



        if (cmd.cmdId==SET_SIZE_FORCESENSORGUITRIGGEREDCMD)
        {
            CForceSensor* it=App::ct->objCont->getForceSensor(cmd.intParams[0]);
            if (it!=nullptr)
                it->setSize(cmd.floatParams[0]);
        }
        if (cmd.cmdId==APPLY_VISUALPROP_FORCESENSORGUITRIGGEREDCMD)
        {
            CForceSensor* it=App::ct->objCont->getForceSensor(cmd.intParams[0]);
            if (it!=nullptr)
            {
                for (size_t i=1;i<cmd.intParams.size();i++)
                {
                    CForceSensor* it2=App::ct->objCont->getForceSensor(cmd.intParams[i]);
                    if (it2!=nullptr)
                    {
                        it->getColor(false)->copyYourselfInto(it2->getColor(false));
                        it->getColor(true)->copyYourselfInto(it2->getColor(true));
                        it2->setSize(it->getSize());
                    }
                }
            }
        }
        if (cmd.cmdId==SET_SAMPLESIZE_FORCESENSORGUITRIGGEREDCMD)
        {
            CForceSensor* it=App::ct->objCont->getForceSensor(cmd.intParams[0]);
            if (it!=nullptr)
                it->setValueCountForFilter(cmd.intParams[1]);
        }
        if (cmd.cmdId==SET_AVERAGEVALUE_FORCESENSORGUITRIGGEREDCMD)
        {
            CForceSensor* it=App::ct->objCont->getForceSensor(cmd.intParams[0]);
            if (it!=nullptr)
                it->setFilterType(0);
        }
        if (cmd.cmdId==SET_MEDIANVALUE_FORCESENSORGUITRIGGEREDCMD)
        {
            CForceSensor* it=App::ct->objCont->getForceSensor(cmd.intParams[0]);
            if (it!=nullptr)
                it->setFilterType(1);
        }
        if (cmd.cmdId==APPLY_FILER_FORCESENSORGUITRIGGEREDCMD)
        {
            CForceSensor* it=App::ct->objCont->getForceSensor(cmd.intParams[0]);
            if (it!=nullptr)
            {
                for (size_t i=1;i<cmd.intParams.size();i++)
                {
                    CForceSensor* it2=App::ct->objCont->getForceSensor(cmd.intParams[i]);
                    if (it2!=nullptr)
                    {
                        it2->setValueCountForFilter(it->getValueCountForFilter());
                        it2->setFilterType(it->getFilterType());
                    }
                }
            }
        }
        if (cmd.cmdId==TOGGLE_FORCETHRESHOLDENABLE_FORCESENSORGUITRIGGEREDCMD)
        {
            CForceSensor* it=App::ct->objCont->getForceSensor(cmd.intParams[0]);
            if (it!=nullptr)
                it->setEnableForceThreshold(!it->getEnableForceThreshold());
        }
        if (cmd.cmdId==TOGGLE_TORQUETHRESHOLDENABLE_FORCESENSORGUITRIGGEREDCMD)
        {
            CForceSensor* it=App::ct->objCont->getForceSensor(cmd.intParams[0]);
            if (it!=nullptr)
                it->setEnableTorqueThreshold(!it->getEnableTorqueThreshold());
        }
        if (cmd.cmdId==SET_FORCETHRESHOLD_FORCESENSORGUITRIGGEREDCMD)
        {
            CForceSensor* it=App::ct->objCont->getForceSensor(cmd.intParams[0]);
            if (it!=nullptr)
                it->setForceThreshold(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_TORQUETHRESHOLD_FORCESENSORGUITRIGGEREDCMD)
        {
            CForceSensor* it=App::ct->objCont->getForceSensor(cmd.intParams[0]);
            if (it!=nullptr)
                it->setTorqueThreshold(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_CONSECTHRESHOLDVIOLATIONS_FORCESENSORGUITRIGGEREDCMD)
        {
            CForceSensor* it=App::ct->objCont->getForceSensor(cmd.intParams[0]);
            if (it!=nullptr)
                it->setConsecutiveThresholdViolationsForBreaking(cmd.intParams[1]);
        }
        if (cmd.cmdId==APPLY_BREAKING_FORCESENSORGUITRIGGEREDCMD)
        {
            CForceSensor* it=App::ct->objCont->getForceSensor(cmd.intParams[0]);
            if (it!=nullptr)
            {
                for (size_t i=1;i<cmd.intParams.size();i++)
                {
                    CForceSensor* it2=App::ct->objCont->getForceSensor(cmd.intParams[i]);
                    if (it2!=nullptr)
                    {
                        it2->setEnableForceThreshold(it->getEnableForceThreshold());
                        it2->setEnableTorqueThreshold(it->getEnableTorqueThreshold());
                        it2->setForceThreshold(it->getForceThreshold());
                        it2->setTorqueThreshold(it->getTorqueThreshold());
                        it2->setConsecutiveThresholdViolationsForBreaking(it->getConsecutiveThresholdViolationsForBreaking());
                    }
                }
            }
        }
        if (cmd.cmdId==TOGGLE_BACKFACECULLING_SHAPEGUITRIGGEREDCMD)
        {
            CShape* it=App::ct->objCont->getShape(cmd.intParams[0]);
            if (it!=nullptr)
                it->setCulling(!it->getCulling());
        }
        if (cmd.cmdId==TOGGLE_WIREFRAME_SHAPEGUITRIGGEREDCMD)
        {
            CShape* it=App::ct->objCont->getShape(cmd.intParams[0]);
            if (it!=nullptr)
                it->setShapeWireframe(!it->getShapeWireframe());
        }
        if (cmd.cmdId==INVERT_FACES_SHAPEGUITRIGGEREDCMD)
        {
            CShape* it=App::ct->objCont->getShape(cmd.intParams[0]);
            if (it!=nullptr)
                it->geomData->invertFrontBack();
        }
        if (cmd.cmdId==TOGGLE_SHOWEDGES_SHAPEGUITRIGGEREDCMD)
        {
            CShape* it=App::ct->objCont->getShape(cmd.intParams[0]);
            if (it!=nullptr)
                it->setVisibleEdges(!it->getVisibleEdges());
        }
        if (cmd.cmdId==SET_SHADINGANGLE_SHAPEGUITRIGGEREDCMD)
        {
            CShape* it=App::ct->objCont->getShape(cmd.intParams[0]);
            if ((it!=nullptr)&&(!it->isCompound()))
                ((CGeometric*)it->geomData->geomInfo)->setGouraudShadingAngle(cmd.floatParams[0]);
        }
        if (cmd.cmdId==APPLY_OTHERPROP_SHAPEGUITRIGGEREDCMD)
        {
            CShape* last=App::ct->objCont->getShape(cmd.intParams[0]);
            if ((last!=nullptr)&&(!last->isCompound()))
            {
                for (size_t i=1;i<cmd.intParams.size();i++)
                {
                    CShape* it=App::ct->objCont->getShape(cmd.intParams[i]);
                    if ((it!=nullptr)&&(!it->isCompound()))
                    {
                        ((CGeometric*)it->geomData->geomInfo)->setVisibleEdges(((CGeometric*)last->geomData->geomInfo)->getVisibleEdges());
                        ((CGeometric*)it->geomData->geomInfo)->setCulling(((CGeometric*)last->geomData->geomInfo)->getCulling());
                        ((CGeometric*)it->geomData->geomInfo)->setInsideAndOutsideFacesSameColor_DEPRECATED(((CGeometric*)last->geomData->geomInfo)->getInsideAndOutsideFacesSameColor_DEPRECATED());
                        ((CGeometric*)it->geomData->geomInfo)->setEdgeWidth_DEPRECATED(((CGeometric*)last->geomData->geomInfo)->getEdgeWidth_DEPRECATED());
                        ((CGeometric*)it->geomData->geomInfo)->setWireframe(((CGeometric*)last->geomData->geomInfo)->getWireframe());
                        ((CGeometric*)it->geomData->geomInfo)->setGouraudShadingAngle(((CGeometric*)last->geomData->geomInfo)->getGouraudShadingAngle());
                        ((CGeometric*)it->geomData->geomInfo)->setEdgeThresholdAngle(((CGeometric*)last->geomData->geomInfo)->getEdgeThresholdAngle());
                        ((CGeometric*)it->geomData->geomInfo)->setHideEdgeBorders(((CGeometric*)last->geomData->geomInfo)->getHideEdgeBorders());
                        ((CGeometric*)it->geomData->geomInfo)->setDisplayInverted_DEPRECATED(((CGeometric*)last->geomData->geomInfo)->getDisplayInverted_DEPRECATED());
                    }
                }
            }
        }
        if (cmd.cmdId==APPLY_VISUALPROP_SHAPEGUITRIGGEREDCMD)
        {
            CShape* last=App::ct->objCont->getShape(cmd.intParams[0]);
            if ((last!=nullptr)&&(!last->isCompound()))
            {
                for (size_t i=1;i<cmd.intParams.size();i++)
                {
                    CShape* it=App::ct->objCont->getShape(cmd.intParams[i]);
                    if ((it!=nullptr)&&(!it->isCompound()))
                    {
                        ((CGeometric*)last->geomData->geomInfo)->color.copyYourselfInto(&((CGeometric*)it->geomData->geomInfo)->color);
                        ((CGeometric*)last->geomData->geomInfo)->insideColor_DEPRECATED.copyYourselfInto(&((CGeometric*)it->geomData->geomInfo)->insideColor_DEPRECATED);
                        ((CGeometric*)last->geomData->geomInfo)->edgeColor_DEPRECATED.copyYourselfInto(&((CGeometric*)it->geomData->geomInfo)->edgeColor_DEPRECATED);
                        it->actualizeContainsTransparentComponent();
                    }
                }
            }
        }
        if (cmd.cmdId==SET_EDGEANGLE_SHAPEGUITRIGGEREDCMD)
        {
            CShape* it=App::ct->objCont->getShape(cmd.intParams[0]);
            if ((it!=nullptr)&&(!it->isCompound()))
                ((CGeometric*)it->geomData->geomInfo)->setEdgeThresholdAngle(cmd.floatParams[0]);
        }
        if (cmd.cmdId==TOGGLE_HIDEEDGEBORDERS_SHAPEGUITRIGGEREDCMD)
        {
            CShape* it=App::ct->objCont->getShape(cmd.intParams[0]);
            if (it!=nullptr)
                it->setHideEdgeBorders(!it->getHideEdgeBorders());
        }
        if (cmd.cmdId==CLEAR_TEXTURES_SHAPEGUITRIGGEREDCMD)
        {
            for (size_t i=0;i<cmd.intParams.size();i++)
            {
                CShape* shape=App::ct->objCont->getShape(cmd.intParams[i]);
                if (shape!=nullptr)
                {
                    std::vector<CGeometric*> components;
                    shape->geomData->geomInfo->getAllShapeComponentsCumulative(components);
                    for (size_t j=0;j<components.size();j++)
                    {
                        bool keepTextCoords=false;
                        CTextureProperty* tp=components[j]->getTextureProperty();
                        if (tp!=nullptr)
                        {
                            // Keep the text. coords if simple shape:
                            if (!shape->isCompound())
                            {
                                std::vector<float>* tc=tp->getFixedTextureCoordinates();
                                if (tc!=nullptr)
                                {
                                    ((CGeometric*)shape->geomData->geomInfo)->textureCoords_notCopiedNorSerialized.assign(tc->begin(),tc->end());
                                    keepTextCoords=true;
                                }
                            }
                            App::ct->textureCont->announceGeneralObjectWillBeErased(shape->getObjectHandle(),-1);
                            delete tp;
                            components[j]->setTextureProperty(nullptr);
                        }
                        if (!keepTextCoords)
                            components[j]->textureCoords_notCopiedNorSerialized.clear(); // discard existing texture coordinates
                    }
                }
            }
        }
        if (cmd.cmdId==SET_QUICKTEXTURES_SHAPEGUITRIGGEREDCMD)
        {
            bool rgba=cmd.boolParams[0];
            int resX=cmd.intParams[cmd.intParams.size()-2];
            int resY=cmd.intParams[cmd.intParams.size()-1];
            std::string texName(cmd.stringParams[0]);
            int n=3;
            if (rgba)
                n=4;

            // 1. Remove existing textures:
            std::vector<CShape*> shapeList;
            for (size_t i=0;i<cmd.intParams.size()-2;i++)
            {
                CShape* shape=App::ct->objCont->getShape(cmd.intParams[i]);
                if (shape!=nullptr)
                {
                    shapeList.push_back(shape);
                    std::vector<CGeometric*> components;
                    shape->geomData->geomInfo->getAllShapeComponentsCumulative(components);
                    for (size_t j=0;j<components.size();j++)
                    {
                        bool useTexCoords=false;
                        CTextureProperty* tp=components[j]->getTextureProperty();
                        if (tp!=nullptr)
                        {
                            // Keep the text. coords if simple shape:
                            if (!shape->isCompound())
                            {
                                std::vector<float>* tc=tp->getFixedTextureCoordinates();
                                if (tc!=nullptr)
                                {
                                    useTexCoords=true;
                                    ((CGeometric*)shape->geomData->geomInfo)->textureCoords_notCopiedNorSerialized.assign(tc->begin(),tc->end());
                                }
                            }
                            App::ct->textureCont->announceGeneralObjectWillBeErased(shape->getObjectHandle(),-1);
                            delete tp;
                            components[j]->setTextureProperty(nullptr);
                        }
                        if (!useTexCoords)
                            components[j]->textureCoords_notCopiedNorSerialized.clear(); // discard existing texture coordinates
                    }
                }
            }

            // 2. Load and apply the "dirt" texture:
            if (shapeList.size()!=0)
            {
                CTextureObject* textureObj=new CTextureObject(resX,resY);
                textureObj->setImage(rgba,false,false,&cmd.uint8Params[0]); // keep false,false
                textureObj->setObjectName(texName.c_str());
                for (size_t i=0;i<shapeList.size();i++)
                {
                    CShape* shape=shapeList[i];
                    std::vector<CGeometric*> components;
                    shape->geomData->geomInfo->getAllShapeComponentsCumulative(components);
                    for (size_t j=0;j<components.size();j++)
                    {
                        CGeometric* geom=components[j];
                        textureObj->addDependentObject(shape->getObjectHandle(),geom->getUniqueID());
                    }
                }

                int textureID=App::ct->textureCont->addObject(textureObj,false); // might erase the textureObj and return a similar object already present!!

                for (size_t i=0;i<shapeList.size();i++)
                {
                    CShape* shape=shapeList[i];
                    C3Vector bbhs(shape->geomData->getBoundingBoxHalfSizes());
                    float s=SIM_MAX(SIM_MAX(bbhs(0),bbhs(1)),bbhs(2))*2.0f;
                    std::vector<CGeometric*> components;
                    shape->geomData->geomInfo->getAllShapeComponentsCumulative(components);
                    for (size_t j=0;j<components.size();j++)
                    {
                        CGeometric* geom=components[j];
                        CTextureProperty* tp=new CTextureProperty(textureID);
                        bool useTexCoords=false;
                        if (!shape->isCompound())
                        {
                            if (((CGeometric*)shape->geomData->geomInfo)->textureCoords_notCopiedNorSerialized.size()!=0)
                            {
                                std::vector<float> wvert;
                                std::vector<int> wind;
                                ((CGeometric*)shape->geomData->geomInfo)->getCumulativeMeshes(wvert,&wind,nullptr);
                                if (((CGeometric*)shape->geomData->geomInfo)->textureCoords_notCopiedNorSerialized.size()/2==wind.size())
                                { // we have texture coordinate data attached to the shape's geometry (was added during shape import)
                                    tp->setFixedCoordinates(&((CGeometric*)shape->geomData->geomInfo)->textureCoords_notCopiedNorSerialized);
                                    ((CGeometric*)shape->geomData->geomInfo)->textureCoords_notCopiedNorSerialized.clear();
                                    useTexCoords=true;
                                }
                            }
                        }
                        if (!useTexCoords)
                        {
                            tp->setRepeatU(true);
                            tp->setRepeatV(true);
                            tp->setTextureMapMode(sim_texturemap_cube);
                            tp->setInterpolateColors(true);
                            tp->setApplyMode(0);
                            tp->setTextureScaling(s,s);
                        }
                        geom->setTextureProperty(tp);
                    }
                }
            }
        }





        if (cmd.cmdId==DELETE_FILTER_VISIONSENSORFILTERGUITRIGGEREDCMD)
        {
            CVisionSensor* it=App::ct->objCont->getVisionSensor(cmd.intParams[0]);
            if (it!=nullptr)
            {
                CComposedFilter* filters=it->getComposedFilter();
                if (filters!=nullptr)
                    filters->removeSimpleFilter(cmd.intParams[1]);
            }
        }
        if (cmd.cmdId==ADD_FILTER_VISIONSENSORFILTERGUITRIGGEREDCMD)
        {
            CVisionSensor* it=App::ct->objCont->getVisionSensor(cmd.intParams[0]);
            if (it!=nullptr)
            {
                CComposedFilter* filters=it->getComposedFilter();
                if (filters!=nullptr)
                {
                    CSimpleFilter* sf=new CSimpleFilter();
                    sf->setFilterType(cmd.intParams[1]);
                    filters->insertSimpleFilter(sf);
                    // Now select the object in the UI. We need to post it so that it arrives after the dialog refresh!:
                    SSimulationThreadCommand cmd2;
                    cmd2.cmdId=CALL_DIALOG_FUNCTION_GUITRIGGEREDCMD;
                    cmd2.intParams.push_back(VISION_SENSOR_FILTER_DLG);
                    cmd2.intParams.push_back(0);
                    cmd2.intParams.push_back(filters->getSimpleFilterCount()-1);
                    App::appendSimulationThreadCommand(cmd2);
                }
            }
        }
        if (cmd.cmdId==MOVE_FILTER_VISIONSENSORFILTERGUITRIGGEREDCMD)
        {
            CVisionSensor* it=App::ct->objCont->getVisionSensor(cmd.intParams[0]);
            int data=cmd.intParams[1];
            if ( (it!=nullptr)&&(data>=0) )
            {
                CComposedFilter* filters=it->getComposedFilter();
                if (data<filters->getSimpleFilterCount())
                {
                    if (filters->moveSimpleFilter(data,cmd.boolParams[0]))
                    {
                        if (cmd.boolParams[0])
                            data--; // up
                        else
                            data++; // down
                        // Now select the object in the UI. We need to post it so that it arrives after the dialog refresh!:
                        SSimulationThreadCommand cmd2;
                        cmd2.cmdId=CALL_DIALOG_FUNCTION_GUITRIGGEREDCMD;
                        cmd2.intParams.push_back(VISION_SENSOR_FILTER_DLG);
                        cmd2.intParams.push_back(0);
                        cmd2.intParams.push_back(data);
                        App::appendSimulationThreadCommand(cmd2);
                    }
                }
            }
        }
        if (cmd.cmdId==TOGGLE_FILTERENABLE_VISIONSENSORFILTERGUITRIGGEREDCMD)
        {
            CVisionSensor* it=App::ct->objCont->getVisionSensor(cmd.intParams[0]);
            int data=cmd.intParams[1];
            if ( (it!=nullptr)&&(data>=0) )
            {
                CComposedFilter* filters=it->getComposedFilter();
                if (filters!=nullptr)
                {
                    if (data<filters->getSimpleFilterCount())
                        filters->getSimpleFilter(data)->setEnabled(!filters->getSimpleFilter(data)->getEnabled());
                }
            }
        }
        if (cmd.cmdId==APPLY_FILTERS_VISIONSENSORFILTERGUITRIGGEREDCMD)
        {
            CVisionSensor* last=App::ct->objCont->getVisionSensor(cmd.intParams[0]);
            if (last!=nullptr)
            {
                for (size_t i=1;i<cmd.intParams.size();i++)
                {
                    CVisionSensor* it=App::ct->objCont->getVisionSensor(cmd.intParams[i]);
                    if (it!=nullptr)
                        it->setComposedFilter(last->getComposedFilter()->copyYourself());
                }
            }
        }
        if (cmd.cmdId==SET_FILTERPARAMS_VISIONSENSORFILTERGUITRIGGEREDCMD)
        {
            CVisionSensor* it=App::ct->objCont->getVisionSensor(cmd.intParams[0]);
            if (it!=nullptr)
            {
                CComposedFilter* filters=it->getComposedFilter();
                if (filters!=nullptr)
                {
                    CSimpleFilter* filter=filters->getSimpleFilter(cmd.intParams[1]);
                    if ((filter!=nullptr)&&filter->canFilterBeEdited()&&(filter->getFilterType()==cmd.intParams[2]))
                    {
                        std::vector<int> intP;
                        for (size_t i=3;i<cmd.intParams.size()-1;i++)
                            intP.push_back(cmd.intParams[i]);
                        filter->setParameters(cmd.uint8Params,intP,cmd.floatParams,cmd.intParams[cmd.intParams.size()-1]);
                    }
                }
            }
        }
        if (cmd.cmdId==SET_CUSTOMFILTERPARAMS_VISIONSENSORFILTERGUITRIGGEREDCMD)
        {
            CVisionSensor* it=App::ct->objCont->getVisionSensor(cmd.intParams[0]);
            if (it!=nullptr)
            {
                CComposedFilter* filters=it->getComposedFilter();
                if (filters!=nullptr)
                {
                    CSimpleFilter* filter=filters->getSimpleFilter(cmd.intParams[1]);
                    if ((filter!=nullptr)&&filter->canFilterBeEdited()&&(filter->getFilterType()==cmd.intParams[2]))
                        filter->setCustomFilterParameters(cmd.uint8Params);
                }
            }
        }





        if (cmd.cmdId==APPLY_SIZE_GEOMETRYGUITRIGGEREDCMD)
        {
            CShape* it=App::ct->objCont->getShape(cmd.intParams[0]);
            if (it!=nullptr)
            {
                C3Vector bbhalfSizes(it->geomData->getBoundingBoxHalfSizes());
                float xSizeOriginal=2.0f*bbhalfSizes(0);
                float ySizeOriginal=2.0f*bbhalfSizes(1);
                float zSizeOriginal=2.0f*bbhalfSizes(2);
                float s[3]={1.0f,1.0f,1.0f}; // imagine we have a plane that has dims x*y*0! keep default at 1.0
                if (xSizeOriginal!=0.0f)
                    s[0]=cmd.floatParams[0]/xSizeOriginal;
                if (ySizeOriginal!=0.0f)
                    s[1]=cmd.floatParams[1]/ySizeOriginal;
                if (zSizeOriginal!=0.0f)
                    s[2]=cmd.floatParams[2]/zSizeOriginal;
                it->geomData->scale(s[0],s[1],s[2]);
            }
        }
        if (cmd.cmdId==APPLY_SCALING_GEOMETRYGUITRIGGEREDCMD)
        {
            CShape* it=App::ct->objCont->getShape(cmd.intParams[0]);
            if (it!=nullptr)
                it->geomData->scale(cmd.floatParams[0],cmd.floatParams[1],cmd.floatParams[2]);
        }
        if (cmd.cmdId==APPLY_FRAMEROTATION_GEOMETRYGUITRIGGEREDCMD)
        {
            CShape* it=App::ct->objCont->getShape(cmd.intParams[0]);
            if (it!=nullptr)
            {
                C7Vector localBefore(it->getLocalTransformationPart1());
                C7Vector parentCumul(it->getParentCumulativeTransformation());
                C7Vector tr;
                tr.setIdentity();
                tr.Q.setEulerAngles(cmd.floatParams[0],cmd.floatParams[1],cmd.floatParams[2]);
                it->setLocalTransformation(parentCumul.getInverse()*tr.getInverse());
                it->alignBoundingBoxWithWorld();
                it->setLocalTransformation(localBefore*tr*parentCumul*it->getLocalTransformation());
            }
        }
        if (cmd.cmdId==TOGGLE_BOOLPROP_TEXTUREGUITRIGGEREDCMD)
        {
            CTextureProperty* it=App::getTexturePropertyPointerFromItem(cmd.intParams[0],cmd.intParams[1],cmd.intParams[2],nullptr,nullptr,nullptr,nullptr);
            if (it!=nullptr)
            {
                if (cmd.intParams[3]==0)
                    it->setInterpolateColors(!it->getInterpolateColors());
                if (cmd.intParams[3]==2)
                    it->setRepeatU(!it->getRepeatU());
                if (cmd.intParams[3]==3)
                    it->setRepeatV(!it->getRepeatV());
            }
        }
        if (cmd.cmdId==SET_3DCONFIG_TEXTUREGUITRIGGEREDCMD)
        {
            CTextureProperty* it=App::getTexturePropertyPointerFromItem(cmd.intParams[0],cmd.intParams[1],cmd.intParams[2],nullptr,nullptr,nullptr,nullptr);
            if (it!=nullptr)
            {
                C7Vector tr(it->getTextureRelativeConfig());
                if (cmd.intParams[3]<3)
                { // position
                    float newVal=tt::getLimitedFloat(-100.0f,100.0f,cmd.floatParams[0]);
                    tr.X(cmd.intParams[3])=newVal;
                }
                else
                { // orientation
                    C3Vector euler(tr.Q.getEulerAngles());
                    euler(cmd.intParams[3]-3)=cmd.floatParams[0];
                    tr.Q.setEulerAngles(euler);
                }
                it->setTextureRelativeConfig(tr);
            }
        }
        if (cmd.cmdId==SET_SCALING_TEXTUREGUITRIGGEREDCMD)
        {
            CTextureProperty* it=App::getTexturePropertyPointerFromItem(cmd.intParams[0],cmd.intParams[1],cmd.intParams[2],nullptr,nullptr,nullptr,nullptr);
            if (it!=nullptr)
            {
                float x,y;
                it->getTextureScaling(x,y);
                float newVal=cmd.floatParams[0];
                if (newVal>=0.0f)
                    newVal=tt::getLimitedFloat(0.001f,1000.0f,newVal);
                else
                    newVal=tt::getLimitedFloat(-1000.0f,-0.001f,newVal);
                if (cmd.intParams[3]==0)
                    it->setTextureScaling(newVal,y);
                else
                    it->setTextureScaling(x,newVal);
            }
        }
        if (cmd.cmdId==SET_MAPPINGMODE_TEXTUREGUITRIGGEREDCMD)
        {
            CTextureProperty* it=App::getTexturePropertyPointerFromItem(cmd.intParams[0],cmd.intParams[1],cmd.intParams[2],nullptr,nullptr,nullptr,nullptr);
            if (it!=nullptr)
            {
                bool usingFixedTextureCoordinates=it->getFixedCoordinates();
                int mode=cmd.intParams[3];
                int previousMode=it->getTextureMapMode();
                bool setOk=false;
                if ((mode==sim_texturemap_plane)&&((previousMode!=mode)||usingFixedTextureCoordinates) )
                    setOk=true;
                if ((mode==sim_texturemap_cylinder)&&(previousMode!=mode))
                    setOk=true;
                if ((mode==sim_texturemap_sphere)&&(previousMode!=mode))
                    setOk=true;
                if ((mode==sim_texturemap_cube)&&(previousMode!=mode))
                    setOk=true;
                if (setOk)
                    it->setTextureMapMode(mode);
            }
        }
        if (cmd.cmdId==SET_APPLYMODE_TEXTUREGUITRIGGEREDCMD)
        {
            CTextureProperty* it=App::getTexturePropertyPointerFromItem(cmd.intParams[0],cmd.intParams[1],cmd.intParams[2],nullptr,nullptr,nullptr,nullptr);
            if (it!=nullptr)
                it->setApplyMode(cmd.intParams[3]);
        }
        if (cmd.cmdId==LOAD_ANDAPPLY_TEXTUREGUITRIGGEREDCMD)
        {
            CGeometric* geom=nullptr;
            bool valid=false;
            bool is3D=false;
            App::getTexturePropertyPointerFromItem(cmd.intParams[0],cmd.intParams[1],cmd.intParams[2],nullptr,&is3D,&valid,&geom);
            if (valid)
            {
                int resX=cmd.intParams[3];
                int resY=cmd.intParams[4];
                int n=cmd.intParams[5];
                bool rgba=(n==4);
                CTextureObject* textureObj=new CTextureObject(resX,resY);
                textureObj->setImage(rgba,false,false,&cmd.uint8Params[0]); // keep false,false
                textureObj->setObjectName(cmd.stringParams[0].c_str());
                if (geom!=nullptr)
                    textureObj->addDependentObject(cmd.intParams[1],geom->getUniqueID());
                else
                {
                    if (cmd.intParams[0]==TEXTURE_ID_OPENGL_GUI_BACKGROUND)
                        textureObj->addDependentObject(cmd.intParams[1],0); // 0 is for background texture on 2DElement
                    if (cmd.intParams[0]==TEXTURE_ID_OPENGL_GUI_BUTTON)
                    {
                        CButtonBlock* block=App::ct->buttonBlockContainer->getBlockWithID(cmd.intParams[1]);
                        CSoftButton* butt=block->getButtonWithID(cmd.intParams[2]);
                        textureObj->addDependentObject(cmd.intParams[1],butt->getUniqueID()); // Unique ID starts exceptionnally at 1
                    }
                }
                int textureID=App::ct->textureCont->addObject(textureObj,false); // might erase the textureObj and return a similar object already present!!
                CTextureProperty* tp=new CTextureProperty(textureID);
                if (geom!=nullptr)
                {
                    // Following 2 since 12/6/2011 because now by default we have the modulate mode (non-decal)
            //      ((CGeometric*)shape->geomData->geomInfo)->color.setColor(0.5f,0.5f,0.5f,sim_colorcomponent_ambient_diffuse);
            //      ((CGeometric*)shape->geomData->geomInfo)->insideColor.setColor(0.5f,0.5f,0.5f,sim_colorcomponent_ambient_diffuse);
                    geom->setTextureProperty(tp);
                    std::vector<float> wvert;
                    std::vector<int> wind;
                    geom->getCumulativeMeshes(wvert,&wind,nullptr);
                    if (geom->textureCoords_notCopiedNorSerialized.size()/2==wind.size())
                    { // we have texture coordinate data attached to the shape's geometry (was added during shape import)
                        App::uiThread->messageBox_information(App::mainWindow,strTranslate("Texture coordinates"),strTranslate(IDS_USING_EXISTING_TEXTURE_COORDINATES),VMESSAGEBOX_OKELI);
                        tp->setFixedCoordinates(&geom->textureCoords_notCopiedNorSerialized);
                        geom->textureCoords_notCopiedNorSerialized.clear();
                    }
                }
                else
                {
                    tp->setApplyMode(1); // 13/1/2012
                    CButtonBlock* block=App::ct->buttonBlockContainer->getBlockWithID(cmd.intParams[1]);
                    if (cmd.intParams[0]==TEXTURE_ID_OPENGL_GUI_BACKGROUND)
                        block->setTextureProperty(tp);
                    if (cmd.intParams[0]==TEXTURE_ID_OPENGL_GUI_BUTTON)
                    {
                        CSoftButton* butt=block->getButtonWithID(cmd.intParams[2]);
                        butt->setTextureProperty(tp);
                    }
                }
            }
        }
        if (cmd.cmdId==SELECT_REMOVE_TEXTUREGUITRIGGEREDCMD)
        {
            CGeometric* geom=nullptr;
            bool valid=false;
            bool is3D=false;
            int tObject=cmd.intParams[3];
            CTextureProperty* tp=App::getTexturePropertyPointerFromItem(cmd.intParams[0],cmd.intParams[1],cmd.intParams[2],nullptr,&is3D,&valid,&geom);
            if (valid)
            {
                if (geom!=nullptr)
                {
                    if (tp!=nullptr)
                    { // remove the texture
                        std::vector<float>* tc=tp->getFixedTextureCoordinates();
                        if (tc!=nullptr)
                            geom->textureCoords_notCopiedNorSerialized.assign(tc->begin(),tc->end());
                        App::ct->textureCont->announceGeneralObjectWillBeErased(cmd.intParams[1],geom->getUniqueID());
                        delete tp;
                        geom->setTextureProperty(nullptr);
                    }
                    else
                    { // add an existing texture
                        if (tObject!=-1)
                        {
                            if ((tObject>=SIM_IDSTART_TEXTURE)&&(tObject<=SIM_IDEND_TEXTURE))
                            {
                                CTextureObject* to=App::ct->textureCont->getObject(tObject);
                                to->addDependentObject(cmd.intParams[1],geom->getUniqueID());
                            }
                            tp=new CTextureProperty(tObject);
                            geom->setTextureProperty(tp);
                            // Following 2 since 12/6/2011 because now by default we have the modulate mode (non-decal)
                        //  ((CGeometric*)shape->geomData->geomInfo)->color.setColor(0.5f,0.5f,0.5f,sim_colorcomponent_ambient_diffuse);
                        //  ((CGeometric*)shape->geomData->geomInfo)->insideColor.setColor(0.5f,0.5f,0.5f,sim_colorcomponent_ambient_diffuse);

                            std::vector<float> wvert;
                            std::vector<int> wind;
                            geom->getCumulativeMeshes(wvert,&wind,nullptr);

                            if (geom->textureCoords_notCopiedNorSerialized.size()/2==wind.size())
                            { // we have texture coordinate data attached to the shape's geometry (was added during shape import)
                                App::uiThread->messageBox_information(App::mainWindow,strTranslate("Texture coordinates"),strTranslate(IDS_USING_EXISTING_TEXTURE_COORDINATES),VMESSAGEBOX_OKELI);
                                tp->setFixedCoordinates(&geom->textureCoords_notCopiedNorSerialized);
                                geom->textureCoords_notCopiedNorSerialized.clear();
                            }
                        }
                    }
                }
                if (cmd.intParams[0]==TEXTURE_ID_OPENGL_GUI_BACKGROUND)
                {
                    CButtonBlock* block=App::ct->buttonBlockContainer->getBlockWithID(cmd.intParams[1]);
                    if (tp!=nullptr)
                    { // remove the texture
                        App::ct->textureCont->announceGeneralObjectWillBeErased(cmd.intParams[1],0);
                        delete tp;
                        block->setTextureProperty(nullptr);
                    }
                    else
                    { // add an existing texture
                        if (tObject!=-1)
                        {
                            if ((tObject>=SIM_IDSTART_TEXTURE)&&(tObject<=SIM_IDEND_TEXTURE))
                            {
                                CTextureObject* to=App::ct->textureCont->getObject(tObject);
                                to->addDependentObject(cmd.intParams[1],0);
                            }
                            tp=new CTextureProperty(tObject);
                            tp->setApplyMode(1); // 13/1/2012
                            block->setTextureProperty(tp);
                        }
                    }
                }
                if (cmd.intParams[0]==TEXTURE_ID_OPENGL_GUI_BUTTON)
                { // texture is linked to a 2DElement button
                    CButtonBlock* block=App::ct->buttonBlockContainer->getBlockWithID(cmd.intParams[1]);
                    CSoftButton* button=block->getButtonWithID(cmd.intParams[2]);
                    if (tp!=nullptr)
                    { // remove the texture
                        App::ct->textureCont->announceGeneralObjectWillBeErased(cmd.intParams[1],cmd.intParams[2]);
                        delete tp;
                        button->setTextureProperty(nullptr);
                    }
                    else
                    { // add an existing texture
                        if (tObject!=-1)
                        {
                            if ((tObject>=SIM_IDSTART_TEXTURE)&&(tObject<=SIM_IDEND_TEXTURE))
                            {
                                CTextureObject* to=App::ct->textureCont->getObject(tObject);
                                to->addDependentObject(cmd.intParams[1],cmd.intParams[2]);
                            }
                            tp=new CTextureProperty(tObject);
                            tp->setApplyMode(1); // 13/1/2012
                            button->setTextureProperty(tp);
                        }
                    }
                }
            }
        }




        if (cmd.cmdId==TOGGLE_BACKFACECULLING_MULTISHAPEEDITIONGUITRIGGEREDCMD)
        {
            CShape* it=App::ct->objCont->getShape(cmd.intParams[0]);
            if (it!=nullptr)
            {
                std::vector<CGeometric*> geoms;
                it->geomData->geomInfo->getAllShapeComponentsCumulative(geoms);
                int index=cmd.intParams[1];
                if ((index>=0)&&(index<int(geoms.size())))
                {
                    CGeometric* geom=geoms[index];
                    geom->setCulling(!geom->getCulling());
                }
            }
        }
        if (cmd.cmdId==TOGGLE_WIREFRAME_MULTISHAPEEDITIONGUITRIGGEREDCMD)
        {
            CShape* it=App::ct->objCont->getShape(cmd.intParams[0]);
            if (it!=nullptr)
            {
                std::vector<CGeometric*> geoms;
                it->geomData->geomInfo->getAllShapeComponentsCumulative(geoms);
                int index=cmd.intParams[1];
                if ((index>=0)&&(index<int(geoms.size())))
                {
                    CGeometric* geom=geoms[index];
                    geom->setWireframe(!geom->getWireframe());
                }
            }
        }
        if (cmd.cmdId==TOGGLE_SHOWEDGES_MULTISHAPEEDITIONGUITRIGGEREDCMD)
        {
            CShape* it=App::ct->objCont->getShape(cmd.intParams[0]);
            if (it!=nullptr)
            {
                std::vector<CGeometric*> geoms;
                it->geomData->geomInfo->getAllShapeComponentsCumulative(geoms);
                int index=cmd.intParams[1];
                if ((index>=0)&&(index<int(geoms.size())))
                {
                    CGeometric* geom=geoms[index];
                    geom->setVisibleEdges(!geom->getVisibleEdges());
                }
            }
        }
        if (cmd.cmdId==SET_SHADINGANGLE_MULTISHAPEEDITIONGUITRIGGEREDCMD)
        {
            CShape* it=App::ct->objCont->getShape(cmd.intParams[0]);
            if (it!=nullptr)
            {
                std::vector<CGeometric*> geoms;
                it->geomData->geomInfo->getAllShapeComponentsCumulative(geoms);
                int index=cmd.intParams[1];
                if ((index>=0)&&(index<int(geoms.size())))
                {
                    CGeometric* geom=geoms[index];
                    geom->setGouraudShadingAngle(cmd.floatParams[0]);
                }
            }
        }
        if (cmd.cmdId==SET_EDGEANGLE_MULTISHAPEEDITIONGUITRIGGEREDCMD)
        {
            CShape* it=App::ct->objCont->getShape(cmd.intParams[0]);
            if (it!=nullptr)
            {
                std::vector<CGeometric*> geoms;
                it->geomData->geomInfo->getAllShapeComponentsCumulative(geoms);
                int index=cmd.intParams[1];
                if ((index>=0)&&(index<int(geoms.size())))
                {
                    CGeometric* geom=geoms[index];
                    geom->setEdgeThresholdAngle(cmd.floatParams[0]);
                }
            }
        }
        if (cmd.cmdId==TOGGLE_HIDDENBORDER_MULTISHAPEEDITIONGUITRIGGEREDCMD)
        {
            CShape* it=App::ct->objCont->getShape(cmd.intParams[0]);
            if (it!=nullptr)
            {
                std::vector<CGeometric*> geoms;
                it->geomData->geomInfo->getAllShapeComponentsCumulative(geoms);
                int index=cmd.intParams[1];
                if ((index>=0)&&(index<int(geoms.size())))
                {
                    CGeometric* geom=geoms[index];
                    geom->setHideEdgeBorders(!geom->getHideEdgeBorders());
                }
            }
        }



        if (cmd.cmdId==TOGGLE_LAYER_LAYERGUITRIGGEREDCMD)
        {
            App::ct->mainSettings->setActiveLayers(App::ct->mainSettings->getActiveLayers()^cmd.intParams[0]);
        }
        if (cmd.cmdId==TOGGLE_SHOWDYNCONTENT_LAYERGUITRIGGEREDCMD)
        {
            App::ct->simulation->setDynamicContentVisualizationOnly(!App::ct->simulation->getDynamicContentVisualizationOnly());
        }



        if (cmd.cmdId==SET_ROLLEDUPSIZES_ROLLEDUPGUITRIGGEREDCMD)
        {
            CButtonBlock* it=App::ct->buttonBlockContainer->getBlockWithID(cmd.intParams[0]);
            if (it!=nullptr)
            {
                VPoint s;
                s.x=cmd.intParams[1];
                s.y=cmd.intParams[2];
                it->getRollupMin(s);
                s.x=cmd.intParams[3];
                s.y=cmd.intParams[4];
                it->getRollupMax(s);
            }
        }


        if (cmd.cmdId==SET_OFFFSET_JOINTDEPENDENCYGUITRIGGEREDCMD)
        {
            CJoint* it=App::ct->objCont->getJoint(cmd.intParams[0]);
            if (it!=nullptr)
                it->setDependencyJointOffset(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_MULTFACT_JOINTDEPENDENCYGUITRIGGEREDCMD)
        {
            CJoint* it=App::ct->objCont->getJoint(cmd.intParams[0]);
            if (it!=nullptr)
                it->setDependencyJointCoeff(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_OTHERJOINT_JOINTDEPENDENCYGUITRIGGEREDCMD)
        {
            CJoint* it=App::ct->objCont->getJoint(cmd.intParams[0]);
            if (it!=nullptr)
                it->setDependencyJointID(cmd.intParams[1]);
        }


        if (cmd.cmdId==TOGGLE_CYCLIC_JOINTGUITRIGGEREDCMD)
        {
            CJoint* it=App::ct->objCont->getJoint(cmd.intParams[0]);
            if (it!=nullptr)
                it->setPositionIsCyclic(!it->getPositionIsCyclic());
        }
        if (cmd.cmdId==SET_PITCH_JOINTGUITRIGGEREDCMD)
        {
            CJoint* it=App::ct->objCont->getJoint(cmd.intParams[0]);
            if (it!=nullptr)
                it->setScrewPitch(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_MINPOS_JOINTGUITRIGGEREDCMD)
        {
            CJoint* it=App::ct->objCont->getJoint(cmd.intParams[0]);
            if (it!=nullptr)
                it->setPositionIntervalMin(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_RANGE_JOINTGUITRIGGEREDCMD)
        {
            CJoint* it=App::ct->objCont->getJoint(cmd.intParams[0]);
            if (it!=nullptr)
                it->setPositionIntervalRange(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_POS_JOINTGUITRIGGEREDCMD)
        {
            CJoint* it=App::ct->objCont->getJoint(cmd.intParams[0]);
            if (it!=nullptr)
            {
                it->setPosition(cmd.floatParams[0]);
                if (it->getJointMode()==sim_jointmode_force)
                    it->setDynamicMotorPositionControlTargetPosition(cmd.floatParams[0]);
            }
        }
        if (cmd.cmdId==SET_IKWEIGHT_JOINTGUITRIGGEREDCMD)
        {
            CJoint* it=App::ct->objCont->getJoint(cmd.intParams[0]);
            if (it!=nullptr)
                it->setIKWeight(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_MAXSTEPSIZE_JOINTGUITRIGGEREDCMD)
        {
            CJoint* it=App::ct->objCont->getJoint(cmd.intParams[0]);
            if (it!=nullptr)
                it->setMaxStepSize(cmd.floatParams[0]);
        }
        if (cmd.cmdId==APPLY_CONFIGPARAMS_JOINTGUITRIGGEREDCMD)
        {
            CJoint* last=App::ct->objCont->getJoint(cmd.intParams[0]);
            if (last!=nullptr)
            {
                for (size_t i=1;i<cmd.intParams.size();i++)
                {
                    CJoint* it=App::ct->objCont->getJoint(cmd.intParams[i]);
                    if ( (it!=nullptr)&&(last->getJointType()==it->getJointType()) )
                    {
                        it->setPositionIsCyclic(last->getPositionIsCyclic());
                        it->setPositionIntervalRange(last->getPositionIntervalRange());
                        it->setPositionIntervalMin(last->getPositionIntervalMin());
                        it->setPosition(last->getPosition());
                        it->setSphericalTransformation(last->getSphericalTransformation());
                        it->setScrewPitch(last->getScrewPitch());
                        it->setIKWeight(last->getIKWeight());
                        it->setMaxStepSize(last->getMaxStepSize());
                    }
                }
            }
        }
        if (cmd.cmdId==SET_MODE_JOINTGUITRIGGEREDCMD)
        {
            CJoint* it=App::ct->objCont->getJoint(cmd.intParams[0]);
            if (it!=nullptr)
                it->setJointMode(cmd.intParams[1]);
        }
        if (cmd.cmdId==TOGGLE_HYBRID_JOINTGUITRIGGEREDCMD)
        {
            CJoint* it=App::ct->objCont->getJoint(cmd.intParams[0]);
            if (it!=nullptr)
                it->setHybridFunctionality(!it->getHybridFunctionality());
        }
        if (cmd.cmdId==APPLY_MODEPARAMS_JOINTGUITRIGGEREDCMD)
        {
            CJoint* last=App::ct->objCont->getJoint(cmd.intParams[0]);
            if (last!=nullptr)
            {
                for (size_t i=1;i<cmd.intParams.size();i++)
                {
                    CJoint* it=App::ct->objCont->getJoint(cmd.intParams[i]);
                    if ( (it!=nullptr)&&(last->getJointType()==it->getJointType()) )
                    {
                        it->setJointMode(last->getJointMode());
                        it->setHybridFunctionality(last->getHybridFunctionality());
                    }
                }
            }
        }
        if (cmd.cmdId==APPLY_VISUALPARAMS_JOINTGUITRIGGEREDCMD)
        {
            CJoint* last=App::ct->objCont->getJoint(cmd.intParams[0]);
            if (last!=nullptr)
            {
                for (size_t i=1;i<cmd.intParams.size();i++)
                {
                    CJoint* it=App::ct->objCont->getJoint(cmd.intParams[i]);
                    if ( (it!=nullptr)&&(last->getJointType()==it->getJointType()) )
                    {
                        last->getColor(false)->copyYourselfInto(it->getColor(false));
                        last->getColor(true)->copyYourselfInto(it->getColor(true));
                        it->setLength(last->getLength());
                        it->setDiameter(last->getDiameter());
                    }
                }
            }
        }
        if (cmd.cmdId==SET_LENGTH_JOINTGUITRIGGEREDCMD)
        {
            CJoint* it=App::ct->objCont->getJoint(cmd.intParams[0]);
            if (it!=nullptr)
                it->setLength(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_DIAMETER_JOINTGUITRIGGEREDCMD)
        {
            CJoint* it=App::ct->objCont->getJoint(cmd.intParams[0]);
            if (it!=nullptr)
                it->setDiameter(cmd.floatParams[0]);
        }



        if (cmd.cmdId==TOGGLE_MOTORENABLED_JOINTDYNGUITRIGGEREDCMD)
        {
            CJoint* it=App::ct->objCont->getJoint(cmd.intParams[0]);
            if (it!=nullptr)
            {
                if (it->getJointMode()==sim_jointmode_force)
                    it->setEnableDynamicMotor(!it->getEnableDynamicMotor());
            }
        }
        if (cmd.cmdId==SET_TARGETVELOCITY_JOINTDYNGUITRIGGEREDCMD)
        {
            CJoint* it=App::ct->objCont->getJoint(cmd.intParams[0]);
            if (it!=nullptr)
            {
                if ( (it->getJointMode()==sim_jointmode_force)||it->getHybridFunctionality() )
                    it->setDynamicMotorTargetVelocity(cmd.floatParams[0]);
            }
        }
        if (cmd.cmdId==SET_MAXFORCE_JOINTDYNGUITRIGGEREDCMD)
        {
            CJoint* it=App::ct->objCont->getJoint(cmd.intParams[0]);
            if (it!=nullptr)
                it->setDynamicMotorMaximumForce(cmd.floatParams[0]);
        }
        if (cmd.cmdId==APPLY_MOTORPARAMS_JOINTDYNGUITRIGGEREDCMD)
        {
            CJoint* last=App::ct->objCont->getJoint(cmd.intParams[0]);
            if ( (last!=nullptr)&&((last->getJointMode()==sim_jointmode_force)||last->getHybridFunctionality()) )
            {
                for (size_t i=1;i<cmd.intParams.size();i++)
                {
                    CJoint* it=App::ct->objCont->getJoint(cmd.intParams[i]);
                    if ( (it!=nullptr)&&(last->getJointType()==it->getJointType())&&((it->getJointMode()==last->getJointMode())||(last->getHybridFunctionality()&&it->getHybridFunctionality())) )
                    {
                        it->setEnableDynamicMotor(last->getEnableDynamicMotor());
                        it->setDynamicMotorTargetVelocity(last->getDynamicMotorTargetVelocity());
                        it->setDynamicMotorLockModeWhenInVelocityControl(last->getDynamicMotorLockModeWhenInVelocityControl());
                        it->setDynamicMotorMaximumForce(last->getDynamicMotorMaximumForce());
                        last->copyEnginePropertiesTo(it);
                    }
                }
            }
        }
        if (cmd.cmdId==TOGGLE_CTRLLOOP_JOINTDYNGUITRIGGEREDCMD)
        {
            CJoint* it=App::ct->objCont->getJoint(cmd.intParams[0]);
            if (it!=nullptr)
            {
                if ( (it->getJointMode()==sim_jointmode_force)&&it->getEnableDynamicMotor())
                    it->setEnableDynamicMotorControlLoop(!it->getEnableDynamicMotorControlLoop());
            }
        }
        if (cmd.cmdId==SET_UPPERVELLIMIT_JOINTDYNGUITRIGGEREDCMD)
        {
            CJoint* it=App::ct->objCont->getJoint(cmd.intParams[0]);
            if (it!=nullptr)
            {
                if ( ((it->getJointMode()==sim_jointmode_force)&&it->getEnableDynamicMotor()&&it->getEnableDynamicMotorControlLoop())||it->getHybridFunctionality())
                    it->setDynamicMotorUpperLimitVelocity(cmd.floatParams[0]);
            }
        }
        if (cmd.cmdId==SET_TARGETPOSITION_JOINTDYNGUITRIGGEREDCMD)
        {
            CJoint* it=App::ct->objCont->getJoint(cmd.intParams[0]);
            if (it!=nullptr)
            {
                if ( (it->getJointMode()==sim_jointmode_force)&&it->getEnableDynamicMotor()&&it->getEnableDynamicMotorControlLoop())
                    it->setDynamicMotorPositionControlTargetPosition(cmd.floatParams[0]);
            }
        }
        if (cmd.cmdId==SET_PIDVALUES_JOINTDYNGUITRIGGEREDCMD)
        {
            CJoint* it=App::ct->objCont->getJoint(cmd.intParams[0]);
            if (it!=nullptr)
            {
                if ( ((it->getJointMode()==sim_jointmode_force)&&it->getEnableDynamicMotor()&&it->getEnableDynamicMotorControlLoop())||it->getHybridFunctionality())
                    it->setDynamicMotorPositionControlParameters(cmd.floatParams[0],cmd.floatParams[1],cmd.floatParams[2]);
            }
        }
        if (cmd.cmdId==APPLY_CTRLPARAMS_JOINTDYNGUITRIGGEREDCMD)
        {
            CJoint* last=App::ct->objCont->getJoint(cmd.intParams[0]);
            if ( (last!=nullptr)&&((last->getJointMode()==sim_jointmode_force)||last->getHybridFunctionality()) )
            {
                for (size_t i=1;i<cmd.intParams.size();i++)
                {
                    CJoint* it=App::ct->objCont->getJoint(cmd.intParams[i]);
                    if ( (it!=nullptr)&&((it->getJointMode()==last->getJointMode())||(last->getHybridFunctionality()&&it->getHybridFunctionality())) )
                    { // only when the two joints are in the same mode, or when both are in hybrid operation
                        it->setEnableDynamicMotorControlLoop(last->getEnableDynamicMotorControlLoop());
                        it->setDynamicMotorUpperLimitVelocity(last->getDynamicMotorUpperLimitVelocity());
                        it->setEnableTorqueModulation(last->getEnableTorqueModulation());
                        float pp,ip,dp;
                        last->getDynamicMotorPositionControlParameters(pp,ip,dp);
                        it->setDynamicMotorPositionControlParameters(pp,ip,dp);
                        float kp,cp;
                        last->getDynamicMotorSpringControlParameters(kp,cp);
                        it->setDynamicMotorSpringControlParameters(kp,cp);
                        it->setDynamicMotorPositionControlTargetPosition(last->getDynamicMotorPositionControlTargetPosition());
                    }
                }
            }
        }
        if (cmd.cmdId==SELECT_PIDCTRL_JOINTDYNGUITRIGGEREDCMD)
        {
            CJoint* it=App::ct->objCont->getJoint(cmd.intParams[0]);
            if (it!=nullptr)
                it->setEnableTorqueModulation(false);
        }
        if (cmd.cmdId==SELECT_SPRINGDAMPERCTRL_JOINTDYNGUITRIGGEREDCMD)
        {
            CJoint* it=App::ct->objCont->getJoint(cmd.intParams[0]);
            if (it!=nullptr)
                it->setEnableTorqueModulation(true);
        }
        if (cmd.cmdId==SET_KCVALUES_JOINTDYNGUITRIGGEREDCMD)
        {
            CJoint* it=App::ct->objCont->getJoint(cmd.intParams[0]);
            if (it!=nullptr)
                it->setDynamicMotorSpringControlParameters(cmd.floatParams[0],cmd.floatParams[1]);
        }
        if (cmd.cmdId==TOGGLE_LOCKMOTOR_JOINTDYNGUITRIGGEREDCMD)
        {
            CJoint* it=App::ct->objCont->getJoint(cmd.intParams[0]);
            if (it!=nullptr)
                it->setDynamicMotorLockModeWhenInVelocityControl(!it->getDynamicMotorLockModeWhenInVelocityControl());
        }
        if (cmd.cmdId==SET_ALLENGINEPARAMS_JOINTDYNGUITRIGGEREDCMD)
        {
            CJoint* it=App::ct->objCont->getJoint(cmd.intParams[0]);
            if (it!=nullptr)
            {
                it->setBulletIntParams(cmd.intVectorParams[0]);
                it->setBulletFloatParams(cmd.floatVectorParams[0]);
                it->setOdeIntParams(cmd.intVectorParams[1]);
                it->setOdeFloatParams(cmd.floatVectorParams[1]);
                it->setVortexIntParams(cmd.intVectorParams[2]);
                it->setVortexFloatParams(cmd.floatVectorParams[2]);
                it->setNewtonIntParams(cmd.intVectorParams[3]);
                it->setNewtonFloatParams(cmd.floatVectorParams[3]);
            }
        }
        if (cmd.cmdId==APPLY_ALLENGINEPARAMS_JOINTDYNGUITRIGGEREDCMD)
        {
            CJoint* it=App::ct->objCont->getJoint(cmd.intParams[0]);
            if (it!=nullptr)
            {
                for (size_t i=1;i<cmd.intParams.size();i++)
                {
                    CJoint* anotherJoint=App::ct->objCont->getJoint(cmd.intParams[i]);
                    if (anotherJoint!=nullptr)
                        it->copyEnginePropertiesTo(anotherJoint);
                }
            }
        }



        if (cmd.cmdId==SET_ATTRIBUTES_PATHGUITRIGGEREDCMD)
        {
            CPath* it=App::ct->objCont->getPath(cmd.intParams[0]);
            if ((it!=nullptr)&&(it->pathContainer!=nullptr))
                it->pathContainer->setAttributes(cmd.intParams[1]);
        }
        if (cmd.cmdId==SET_LINESIZE_PATHGUITRIGGEREDCMD)
        {
            CPath* it=App::ct->objCont->getPath(cmd.intParams[0]);
            if ((it!=nullptr)&&(it->pathContainer!=nullptr))
                it->pathContainer->setLineSize(cmd.intParams[1]);
        }
        if (cmd.cmdId==SET_CTRLPTSIZE_PATHGUITRIGGEREDCMD)
        {
            CPath* it=App::ct->objCont->getPath(cmd.intParams[0]);
            if ((it!=nullptr)&&(it->pathContainer!=nullptr))
                it->pathContainer->setSquareSize(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_DISTANCEUNIT_PATHGUITRIGGEREDCMD)
        {
            CPath* it=App::ct->objCont->getPath(cmd.intParams[0]);
            if ((it!=nullptr)&&(it->pathContainer!=nullptr))
                it->pathContainer->setPathLengthCalculationMethod(cmd.intParams[1]);
        }


        if (cmd.cmdId==TOGGLE_SHAPINGENABLED_PATHSHAPINGGUITRIGGEREDCMD)
        {
            CPath* it=App::ct->objCont->getPath(cmd.intParams[0]);
            if (it!=nullptr)
                it->setShapingEnabled(!it->getShapingEnabled());
        }
        if (cmd.cmdId==TOGGLE_FOLLOWORIENTATION_PATHSHAPINGGUITRIGGEREDCMD)
        {
            CPath* it=App::ct->objCont->getPath(cmd.intParams[0]);
            if (it!=nullptr)
                it->setShapingFollowFullOrientation(!it->getShapingFollowFullOrientation());
        }
        if (cmd.cmdId==TOGGLE_CONVEXHULLS_PATHSHAPINGGUITRIGGEREDCMD)
        {
            CPath* it=App::ct->objCont->getPath(cmd.intParams[0]);
            if (it!=nullptr)
                it->setShapingThroughConvexHull(!it->getShapingThroughConvexHull());
        }
        if (cmd.cmdId==TOGGLE_LASTCOORDLINKSTOFIRST_PATHSHAPINGGUITRIGGEREDCMD)
        {
            CPath* it=App::ct->objCont->getPath(cmd.intParams[0]);
            if (it!=nullptr)
                it->setShapingSectionClosed(!it->getShapingSectionClosed());
        }
        if (cmd.cmdId==GENERATE_SHAPE_PATHSHAPINGGUITRIGGEREDCMD)
        {
            CPath* it=App::ct->objCont->getPath(cmd.intParams[0]);
            if (it!=nullptr)
            {
                CGeomProxy* geom;
                CShape* shape;
                if (it->getShape(&geom,&shape))
                {
                    shape->geomData=geom;
                    App::ct->objCont->addObjectToScene(shape,false,true);
                }
            }
        }
        if (cmd.cmdId==SET_ELEMENTMAXLENGTH_PATHSHAPINGGUITRIGGEREDCMD)
        {
            CPath* it=App::ct->objCont->getPath(cmd.intParams[0]);
            if (it!=nullptr)
                it->setShapingElementMaxLength(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_TYPE_PATHSHAPINGGUITRIGGEREDCMD)
        {
            CPath* it=App::ct->objCont->getPath(cmd.intParams[0]);
            if (it!=nullptr)
                it->setShapingType(cmd.intParams[1]);
        }
        if (cmd.cmdId==SET_SCALINGFACTOR_PATHSHAPINGGUITRIGGEREDCMD)
        {
            CPath* it=App::ct->objCont->getPath(cmd.intParams[0]);
            if (it!=nullptr)
                it->setShapingScaling(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_COORDINATES_PATHSHAPINGGUITRIGGEREDCMD)
        {
            CPath* it=App::ct->objCont->getPath(cmd.intParams[0]);
            if (it!=nullptr)
            {
                it->shapingCoordinates.assign(cmd.floatParams.begin(),cmd.floatParams.end());
                it->setShapingElementMaxLength(it->getShapingElementMaxLength()); // To trigger an actualization!
            }
        }




        if (cmd.cmdId==TOGGLE_STATIC_SHAPEDYNGUITRIGGEREDCMD)
        {
            CShape* it=App::ct->objCont->getShape(cmd.intParams[0]);
            if (it!=nullptr)
                it->setShapeIsDynamicallyStatic(!it->getShapeIsDynamicallyStatic());
        }
        if (cmd.cmdId==TOGGLE_STARTINSLEEPMODE_SHAPEDYNGUITRIGGEREDCMD)
        {
            CShape* it=App::ct->objCont->getShape(cmd.intParams[0]);
            if (it!=nullptr)
                it->setStartInDynamicSleeping(!it->getStartInDynamicSleeping());
        }
        if (cmd.cmdId==TOGGLE_RESPONDABLE_SHAPEDYNGUITRIGGEREDCMD)
        {
            CShape* it=App::ct->objCont->getShape(cmd.intParams[0]);
            if (it!=nullptr)
                it->setRespondable(!it->getRespondable());
        }
        if (cmd.cmdId==SET_RESPONDABLEMASK_SHAPEDYNGUITRIGGEREDCMD)
        {
            CShape* it=App::ct->objCont->getShape(cmd.intParams[0]);
            if (it!=nullptr)
                it->setDynamicCollisionMask(cmd.intParams[1]);
        }
        if (cmd.cmdId==SET_MASS_SHAPEDYNGUITRIGGEREDCMD)
        {
            CShape* it=App::ct->objCont->getShape(cmd.intParams[0]);
            if (it!=nullptr)
                it->geomData->geomInfo->setMass(cmd.floatParams[0]);
        }
        if (cmd.cmdId==MULTIPLY_MASSFORSELECTION_SHAPEDYNGUITRIGGEREDCMD)
        {
            std::vector<CGeomWrap*> allComponents;
            for (size_t i=0;i<cmd.intParams.size();i++)
            {
                CShape* it=App::ct->objCont->getShape(cmd.intParams[i]);
                if ( (it!=nullptr)&&(!it->getShapeIsDynamicallyStatic()) )
                {
                    CGeomWrap* sc=it->geomData->geomInfo;
                    for (size_t j=0;j<allComponents.size();j++)
                    {
                        if (allComponents[j]==sc)
                        {
                            sc=nullptr;
                            break;
                        }
                    }
                    if (sc!=nullptr)
                        allComponents.push_back(sc);
                }
            }
            for (size_t i=0;i<allComponents.size();i++)
                allComponents[i]->setMass(allComponents[i]->getMass()*cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_PRINCIPALMOMENTOFINTERIA_SHAPEDYNGUITRIGGEREDCMD)
        {
            CShape* it=App::ct->objCont->getShape(cmd.intParams[0]);
            if (it!=nullptr)
                it->geomData->geomInfo->setPrincipalMomentsOfInertia(C3Vector(&cmd.floatParams[0]));
        }
        if (cmd.cmdId==MULTIPLY_INERTIAFORSELECTION_SHAPEDYNGUITRIGGEREDCMD)
        {
            std::vector<CGeomWrap*> allComponents;
            for (size_t i=0;i<cmd.intParams.size();i++)
            {
                CShape* it=App::ct->objCont->getShape(cmd.intParams[i]);
                if ( (it!=nullptr)&&(!it->getShapeIsDynamicallyStatic()) )
                {
                    CGeomWrap* sc=it->geomData->geomInfo;
                    for (size_t j=0;j<allComponents.size();j++)
                    {
                        if (allComponents[j]==sc)
                        {
                            sc=nullptr;
                            break;
                        }
                    }
                    if (sc!=nullptr)
                        allComponents.push_back(sc);
                }
            }
            for (size_t i=0;i<allComponents.size();i++)
            {
                C3Vector v(allComponents[i]->getPrincipalMomentsOfInertia());
                v*=cmd.floatParams[0];
                allComponents[i]->setPrincipalMomentsOfInertia(v);
            }
        }
        if (cmd.cmdId==SET_COMMATRIX_SHAPEDYNGUITRIGGEREDCMD)
        {
            CShape* it=App::ct->objCont->getShape(cmd.intParams[0]);
            if (it!=nullptr)
                it->geomData->geomInfo->setLocalInertiaFrame(cmd.transfParams[0]);
        }
        if (cmd.cmdId==APPLY_DYNPARAMS_SHAPEDYNGUITRIGGEREDCMD)
        {
            CShape* last=App::ct->objCont->getShape(cmd.intParams[0]);
            if (last!=nullptr)
            {
                C7Vector trLast(last->getCumulativeTransformation());
                C7Vector trfLast(last->geomData->geomInfo->getLocalInertiaFrame());
                bool lastIsHeightfield=(last->geomData->geomInfo->getPurePrimitiveType()==sim_pure_primitive_heightfield);
                for (size_t i=1;i<cmd.intParams.size();i++)
                {
                    CShape* it=App::ct->objCont->getShape(cmd.intParams[i]);
                    if (it!=nullptr)
                    {
                        bool itIsHeightfield=(it->geomData->geomInfo->getPurePrimitiveType()==sim_pure_primitive_heightfield);
                        if (lastIsHeightfield)
                        { // Heightfields cannot be non-static
                            if (!itIsHeightfield)
                                it->setShapeIsDynamicallyStatic(true);
                        }
                        else
                        {
                            if (!itIsHeightfield)
                            {
                                it->setShapeIsDynamicallyStatic(last->getShapeIsDynamicallyStatic());
                                it->setStartInDynamicSleeping(last->getStartInDynamicSleeping());
                                it->setSetAutomaticallyToNonStaticIfGetsParent(last->getSetAutomaticallyToNonStaticIfGetsParent());
                                it->geomData->geomInfo->setMass(last->geomData->geomInfo->getMass());
                                it->geomData->geomInfo->setPrincipalMomentsOfInertia(last->geomData->geomInfo->getPrincipalMomentsOfInertia());
                                it->geomData->geomInfo->setLocalInertiaFrame(last->geomData->geomInfo->getLocalInertiaFrame());
                            }
                        }
                    }
                }
            }
        }
        if (cmd.cmdId==TOGGLE_SETTODYNAMICIFGETSPARENT_SHAPEDYNGUITRIGGEREDCMD)
        {
            CShape* it=App::ct->objCont->getShape(cmd.intParams[0]);
            if (it!=nullptr)
                it->setSetAutomaticallyToNonStaticIfGetsParent(!it->getSetAutomaticallyToNonStaticIfGetsParent());
        }
        if (cmd.cmdId==APPLY_RESPONDABLEPARAMS_SHAPEDYNGUITRIGGEREDCMD)
        {
            CShape* last=App::ct->objCont->getShape(cmd.intParams[0]);
            if (last!=nullptr)
            {
                for (size_t i=1;i<cmd.intParams.size();i++)
                {
                    CShape* it=App::ct->objCont->getShape(cmd.intParams[i]);
                    if (it!=nullptr)
                    {
                        it->setRespondable(last->getRespondable());
                        it->setDynMaterial(last->getDynMaterial()->copyYourself());
                        it->setDynamicCollisionMask(last->getDynamicCollisionMask());
                    }
                }
            }
        }
        if (cmd.cmdId==COMPUTE_MASSANDINERTIA_SHAPEDYNGUITRIGGEREDCMD)
        {
            for (size_t i=0;i<cmd.intParams.size();i++)
            {
                CShape* it=App::ct->objCont->getShape(cmd.intParams[i]);
                if ((it!=nullptr)&&it->geomData->geomInfo->isConvex())
                {
                    std::vector<float> vert;
                    std::vector<int> ind;
                    it->geomData->geomInfo->getCumulativeMeshes(vert,&ind,nullptr);
                    C3Vector com;
                    C3X3Matrix tensor;
                    float mass=CVolInt::getMassCenterOfMassAndInertiaTensor(&vert[0],(int)vert.size()/3,&ind[0],(int)ind.size()/3,cmd.floatParams[0],com,tensor);
                    C4Vector rot;
                    C3Vector pmoment;
                    CGeomWrap::findPrincipalMomentOfInertia(tensor,rot,pmoment);
                    it->geomData->geomInfo->setPrincipalMomentsOfInertia(pmoment);
                    it->geomData->geomInfo->setLocalInertiaFrame(C7Vector(rot,com));
                    it->geomData->geomInfo->setMass(mass);
                }
            }
        }
        if (cmd.cmdId==SET_ENGINEPARAMS_SHAPEDYNGUITRIGGEREDCMD)
        {
            CShape* it=App::ct->objCont->getShape(cmd.intParams[0]);
            if (it!=nullptr)
            {
                CDynMaterialObject* mat=it->getDynMaterial();
                mat->setObjectName(cmd.stringParams[0].c_str());
                mat->setBulletIntParams(cmd.intVectorParams[0]);
                mat->setBulletFloatParams(cmd.floatVectorParams[0]);
                mat->setOdeIntParams(cmd.intVectorParams[1]);
                mat->setOdeFloatParams(cmd.floatVectorParams[1]);
                mat->setVortexIntParams(cmd.intVectorParams[2]);
                mat->setVortexFloatParams(cmd.floatVectorParams[2]);
                mat->setNewtonIntParams(cmd.intVectorParams[3]);
                mat->setNewtonFloatParams(cmd.floatVectorParams[3]);
            }
        }





        if (cmd.cmdId==DELETE_SCRIPT_SCRIPTGUITRIGGEREDCMD)
        {
            int scriptID=cmd.intParams[0];
            CLuaScriptObject* script=App::ct->luaScriptContainer->getScriptFromID_noAddOnsNorSandbox(scriptID);
            if (script!=nullptr)
            {
                if ((script->getScriptType()==sim_scripttype_mainscript)||(script->getScriptType()==sim_scripttype_childscript))
                    App::ct->luaScriptContainer->removeScript(scriptID);
                else if (script->getScriptType()==sim_scripttype_customizationscript)
                {
                    int objID=script->getObjectIDThatScriptIsAttachedTo_customization();
                    C3DObject* it=App::ct->objCont->getObjectFromHandle(objID);
                    if (it!=nullptr)
                        it->setEnableCustomizationScript(false,nullptr);
                    else
                        App::ct->luaScriptContainer->removeScript(scriptID); // unassociated
                }
            }
        }
        if (cmd.cmdId==INSERT_SCRIPT_SCRIPTGUITRIGGEREDCMD)
        {
            int scriptT=cmd.intParams[0];
            bool threaded=cmd.intParams[1]!=0;
            int newScriptID=-1;
            if ((scriptT==sim_scripttype_mainscript)||(scriptT==sim_scripttype_childscript))
                newScriptID=App::ct->luaScriptContainer->insertDefaultScript_mainAndChildScriptsOnly(scriptT,threaded);
            if (scriptT==sim_scripttype_customizationscript)
            {
                std::string filenameAndPath(App::directories->systemDirectory+VREP_SLASH);
                filenameAndPath+=DEFAULT_CUSTOMIZATIONSCRIPT_NAME;
                std::string scriptInitText;
                if (VFile::doesFileExist(filenameAndPath))
                {
                    try
                    {
                        VFile file(filenameAndPath,VFile::READ|VFile::SHARE_DENY_NONE);
                        VArchive archive(&file,VArchive::LOAD);
                        unsigned int archiveLength=(unsigned int)file.getLength();
                        char* defaultScript=new char[archiveLength+1];
                        for (int i=0;i<int(archiveLength);i++)
                            archive >> defaultScript[i];
                        defaultScript[archiveLength]=0;
                        scriptInitText=defaultScript;
                        delete[] defaultScript;
                        archive.close();
                        file.close();
                    }
                    catch(VFILE_EXCEPTION_TYPE e)
                    {
                        VFile::reportAndHandleFileExceptionError(e);
                        scriptInitText="Default customization script file could not be found!"; // do not use comments ("--"), we want to cause an execution error!
                    }
                }
                else
                    scriptInitText="Default customization script file could not be found!"; // do not use comments ("--"), we want to cause an execution error!

                CLuaScriptObject* script=new CLuaScriptObject(sim_scripttype_customizationscript);
                script->setScriptText(scriptInitText.c_str());
                newScriptID=App::ct->luaScriptContainer->insertScript(script);
            }
            // Now select the new collection in the UI. We need to post it so that it arrives after the dialog refresh!:
            SSimulationThreadCommand cmd2;
            cmd2.cmdId=CALL_DIALOG_FUNCTION_GUITRIGGEREDCMD;
            cmd2.intParams.push_back(LUA_SCRIPT_DLG);
            cmd2.intParams.push_back(0);
            cmd2.intParams.push_back(newScriptID);
            App::appendSimulationThreadCommand(cmd2);
        }
        if (cmd.cmdId==TOGGLE_DISABLED_SCRIPTGUITRIGGEREDCMD)
        {
            int scriptID=cmd.intParams[0];
            CLuaScriptObject* it=App::ct->luaScriptContainer->getScriptFromID_alsoAddOnsAndSandbox(scriptID);
            if (it!=nullptr)
            {
                if (it->getScriptType()==sim_scripttype_customizationscript)
                {
                    if (it->getCustomizationScriptIsTemporarilyDisabled())
                    {
                        it->killLuaState();
                        it->setCustomizationScriptIsTemporarilyDisabled(false);
                    }
                    else
                    {
                        it->killLuaState();
                        it->setScriptIsDisabled(!it->getScriptIsDisabled());
                    }
                }
                else
                    it->setScriptIsDisabled(!it->getScriptIsDisabled());
            }
        }
        if (cmd.cmdId==TOGGLE_DISABLE_CUSTOM_SCRIPT_WITH_ERROR_SCRIPTGUITRIGGEREDCMD)
        {
            int scriptID=cmd.intParams[0];
            CLuaScriptObject* it=App::ct->luaScriptContainer->getScriptFromID_noAddOnsNorSandbox(scriptID);
            if (it!=nullptr)
            {
                if (it->getScriptType()==sim_scripttype_customizationscript)
                    it->setDisableCustomizationScriptWithError(!it->getDisableCustomizationScriptWithError());
            }
        }
        if (cmd.cmdId==TOGGLE_EXECUTEONCE_SCRIPTGUITRIGGEREDCMD)
        {
            int scriptID=cmd.intParams[0];
            CLuaScriptObject* it=App::ct->luaScriptContainer->getScriptFromID_noAddOnsNorSandbox(scriptID);
            if ((it!=nullptr)&&it->getThreadedExecution())
                it->setExecuteJustOnce(!it->getExecuteJustOnce());
        }
        if (cmd.cmdId==SET_ASSOCIATEDOBJECT_SCRIPTGUITRIGGEREDCMD)
        {
            int scriptID=cmd.intParams[0];
            int objID=cmd.intParams[1];
            CLuaScriptObject* it=App::ct->luaScriptContainer->getScriptFromID_noAddOnsNorSandbox(scriptID);
            C3DObject* it2=App::ct->objCont->getObjectFromHandle(objID);
            if (it!=nullptr)
            {
                if (it->getScriptType()==sim_scripttype_childscript)
                {
                    if (it2!=nullptr)
                    {
                        // Check if the object doesn't already have a script attached:
                        if (App::ct->luaScriptContainer->getScriptFromObjectAttachedTo_child(objID)==nullptr)
                        {
                            it->setObjectIDThatScriptIsAttachedTo(objID);
                            App::ct->setModificationFlag(8192); // script added flag
                        }
                    }
                    else
                    {
                        it->setObjectIDThatScriptIsAttachedTo(-1);
                        App::ct->setModificationFlag(16384); // script deleted flag
                    }
                }
                if (it->getScriptType()==sim_scripttype_customizationscript)
                {
                    if (it2!=nullptr)
                    {
                        // Check if the object doesn't already have a script attached:
                        if (App::ct->luaScriptContainer->getScriptFromObjectAttachedTo_customization(objID)==nullptr)
                        {
                            it->setObjectIDThatScriptIsAttachedTo(objID);
                            App::ct->setModificationFlag(8192); // script added flag
                        }
                    }
                    else
                    {
                        it->setObjectIDThatScriptIsAttachedTo(-1);
                        App::ct->setModificationFlag(16384); // script deleted flag
                    }
                }
            }
        }
        if (cmd.cmdId==SET_EXECORDER_SCRIPTGUITRIGGEREDCMD)
        {
            int scriptID=cmd.intParams[0];
            CLuaScriptObject* it=App::ct->luaScriptContainer->getScriptFromID_alsoAddOnsAndSandbox(scriptID);
            if (it!=nullptr)
                it->setExecutionOrder(cmd.intParams[1]);
        }
        if (cmd.cmdId==SET_TREETRAVERSALDIR_SCRIPTGUITRIGGEREDCMD)
        {
            int scriptID=cmd.intParams[0];
            CLuaScriptObject* it=App::ct->luaScriptContainer->getScriptFromID_alsoAddOnsAndSandbox(scriptID);
            if (it!=nullptr)
                it->setTreeTraversalDirection(cmd.intParams[1]);
        }
        if (cmd.cmdId==SET_DEBUGMODE_SCRIPTGUITRIGGEREDCMD)
        {
            int scriptID=cmd.intParams[0];
            CLuaScriptObject* it=App::ct->luaScriptContainer->getScriptFromID_alsoAddOnsAndSandbox(scriptID);
            if (it!=nullptr)
                it->setDebugLevel(cmd.intParams[1]);
        }
        if (cmd.cmdId==SET_ALL_SCRIPTSIMULPARAMETERGUITRIGGEREDCMD)
        {
            int scriptID=cmd.intParams[0];
            CLuaScriptObject* it=App::ct->luaScriptContainer->getScriptFromID_noAddOnsNorSandbox(scriptID);
            if (it!=nullptr)
            {
                CLuaScriptParameters* sp=it->getScriptParametersObject();
                sp->scriptParamEntries.clear();
                for (size_t i=0;i<cmd.intParams.size()-1;i++)
                {
                    SScriptParamEntry e;
                    e.name=cmd.stringParams[3*i+0];
                    e.unit=cmd.stringParams[3*i+1];
                    e.value=cmd.stringParams[3*i+2];
                    e.properties=cmd.intParams[1+i];
                    sp->scriptParamEntries.push_back(e);
                }
            }
        }




        if (cmd.cmdId==TOGGLE_ENABLEALL_GEOMCONSTRSOLVERGUITRIGGEREDCMD)
        {
            App::ct->mainSettings->gcsCalculationEnabled=!App::ct->mainSettings->gcsCalculationEnabled;
        }
        if (cmd.cmdId==INSERT_GEOMCONSTRSOLVERGUITRIGGEREDCMD)
        {
            C3DObject* it=App::ct->objCont->getObjectFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
            {
                CGeometricConstraintSolverInt work(it->getObjectHandle(),-1);
                CConstraintSolverObject* similarObject=App::ct->constraintSolver->getObjectFromMechanismID(work.getMechanismID());
                if (similarObject!=nullptr)
                {
                    std::string tmp(IDS_GCSOBJECT_ALREADY_THERE_MESSAGE);
                    tmp+=" '";
                    tmp+=similarObject->getObjectName();
                    tmp+="'.";
                    App::uiThread->messageBox_warning(App::mainWindow,strTranslate(IDSN_CONSTRAINT_SOLVER),tmp,VMESSAGEBOX_OKELI);
                }
                else
                {
                    CConstraintSolverObject* no=new CConstraintSolverObject();
                    no->setBase3DObject(it->getObjectHandle());
                    App::ct->constraintSolver->addObject(no,false);
                    // Now select the object in the UI. We need to post it so that it arrives after the dialog refresh!:
                    SSimulationThreadCommand cmd2;
                    cmd2.cmdId=CALL_DIALOG_FUNCTION_GUITRIGGEREDCMD;
                    cmd2.intParams.push_back(CONSTRAINT_SOLVER_DLG);
                    cmd2.intParams.push_back(0);
                    cmd2.intParams.push_back(no->getObjectID());
                    App::appendSimulationThreadCommand(cmd2);
                }
            }
        }
        if (cmd.cmdId==RENAME_GEOMCONSTRSOLVERGUITRIGGEREDCMD)
        {
            std::string newName(cmd.stringParams[0]);
            CConstraintSolverObject* it=App::ct->constraintSolver->getObject(cmd.intParams[0]);
            if ( (it!=nullptr)&&(newName!="") )
            {
                if (it->getObjectName()!=newName)
                {
                    tt::removeIllegalCharacters(newName,true);
                    if (App::ct->constraintSolver->getObject(newName.c_str())==nullptr)
                        it->setObjectName(newName.c_str());
                }
            }
        }
        if (cmd.cmdId==REMOVE_GEOMCONSTRSOLVERGUITRIGGEREDCMD)
        {
            CConstraintSolverObject* it=App::ct->constraintSolver->getObject(cmd.intParams[0]);
            if (it!=nullptr)
                App::ct->constraintSolver->removeObject(cmd.intParams[0]);
        }
        if (cmd.cmdId==TOGGLE_EXPLICITHANDLING_GEOMCONSTRSOLVERGUITRIGGEREDCMD)
        {
            CConstraintSolverObject* it=App::ct->constraintSolver->getObject(cmd.intParams[0]);
            if (it!=nullptr)
                it->setExplicitHandling(!it->getExplicitHandling());
        }
        if (cmd.cmdId==SET_MAXITERATIONS_GEOMCONSTRSOLVERGUITRIGGEREDCMD)
        {
            CConstraintSolverObject* it=App::ct->constraintSolver->getObject(cmd.intParams[0]);
            if (it!=nullptr)
                it->setMaximumIterations(cmd.intParams[1]);
        }
        if (cmd.cmdId==SET_INTERPOLATION_GEOMCONSTRSOLVERGUITRIGGEREDCMD)
        {
            CConstraintSolverObject* it=App::ct->constraintSolver->getObject(cmd.intParams[0]);
            if (it!=nullptr)
                it->setInterpolation(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_DAMPING_GEOMCONSTRSOLVERGUITRIGGEREDCMD)
        {
            CConstraintSolverObject* it=App::ct->constraintSolver->getObject(cmd.intParams[0]);
            if (it!=nullptr)
                it->setGeneralDamping(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_MAXLINVARIATION_GEOMCONSTRSOLVERGUITRIGGEREDCMD)
        {
            CConstraintSolverObject* it=App::ct->constraintSolver->getObject(cmd.intParams[0]);
            if (it!=nullptr)
                it->setMaxLinearVariation(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_MAXANGVARIATION_GEOMCONSTRSOLVERGUITRIGGEREDCMD)
        {
            CConstraintSolverObject* it=App::ct->constraintSolver->getObject(cmd.intParams[0]);
            if (it!=nullptr)
                it->setMaxAngleVariation(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_LOOPCLOSUREPOSTOL_GEOMCONSTRSOLVERGUITRIGGEREDCMD)
        {
            CConstraintSolverObject* it=App::ct->constraintSolver->getObject(cmd.intParams[0]);
            if (it!=nullptr)
                it->setLoopClosurePositionTolerance(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_LOOPCLOSUREANGTOL_GEOMCONSTRSOLVERGUITRIGGEREDCMD)
        {
            CConstraintSolverObject* it=App::ct->constraintSolver->getObject(cmd.intParams[0]);
            if (it!=nullptr)
                it->setLoopClosureOrientationTolerance(cmd.floatParams[0]);
        }




        if (cmd.cmdId==TOGGLE_EXPLICITHANDLING_GRAPHGUITRIGGEREDCMD)
        {
            CGraph* it=App::ct->objCont->getGraph(cmd.intParams[0]);
            if (it!=nullptr)
                it->setExplicitHandling(!it->getExplicitHandling());
        }
        if (cmd.cmdId==TOGGLE_BUFFERCYCLIC_GRAPHGUITRIGGEREDCMD)
        {
            CGraph* it=App::ct->objCont->getGraph(cmd.intParams[0]);
            if (it!=nullptr)
                it->setCyclic(!it->getCyclic());
        }
        if (cmd.cmdId==TOGGLE_SHOWXYZPLANES_GRAPHGUITRIGGEREDCMD)
        {
            CGraph* it=App::ct->objCont->getGraph(cmd.intParams[0]);
            if (it!=nullptr)
                it->xYZPlanesDisplay=!it->xYZPlanesDisplay;
        }
        if (cmd.cmdId==TOGGLE_TIMEGRAPHVISIBLE_GRAPHGUITRIGGEREDCMD)
        {
            CGraph* it=App::ct->objCont->getGraph(cmd.intParams[0]);
            if (it!=nullptr)
            {
                CGraphData* grData=it->getGraphData(cmd.intParams[1]);
                if (grData!=nullptr)
                    grData->setVisible(!grData->getVisible());
            }
        }
        if (cmd.cmdId==TOGGLE_TIMEGRAPHSHOWLABEL_GRAPHGUITRIGGEREDCMD)
        {
            CGraph* it=App::ct->objCont->getGraph(cmd.intParams[0]);
            if (it!=nullptr)
            {
                CGraphData* grData=it->getGraphData(cmd.intParams[1]);
                if (grData!=nullptr)
                    grData->setLabel(!grData->getLabel());
            }
        }
        if (cmd.cmdId==TOGGLE_TIMEGRAPHLINKPOINTS_GRAPHGUITRIGGEREDCMD)
        {
            CGraph* it=App::ct->objCont->getGraph(cmd.intParams[0]);
            if (it!=nullptr)
            {
                CGraphData* grData=it->getGraphData(cmd.intParams[1]);
                if (grData!=nullptr)
                    grData->setLinkPoints(!grData->getLinkPoints());
            }
        }
        if (cmd.cmdId==INSERT_DATASTREAM_GRAPHGUITRIGGEREDCMD)
        {
            CGraph* it=App::ct->objCont->getGraph(cmd.intParams[0]);
            if (it!=nullptr)
            {
                int currentDataType=cmd.intParams[1];
                int index=cmd.intParams[2];
                int objID;
                CGraphingRoutines::loopThroughAllAndGetObjectsFromGraphCategory(index,currentDataType,objID);
                CGraphData* newGraphDat=new CGraphData(currentDataType,objID,-1);
                CGraph* it=App::ct->objCont->getLastSelection_graph();
                int h=it->addNewGraphData(newGraphDat);
                // Now select the object in the UI. We need to post it so that it arrives after the dialog refresh!:
                SSimulationThreadCommand cmd2;
                cmd2.cmdId=CALL_DIALOG_FUNCTION_GUITRIGGEREDCMD;
                cmd2.intParams.push_back(GRAPH_DLG);
                cmd2.intParams.push_back(0);
                cmd2.intParams.push_back(h);
                App::appendSimulationThreadCommand(cmd2);
            }
        }
        if (cmd.cmdId==REMOVE_DATASTREAM_GRAPHGUITRIGGEREDCMD)
        {
            CGraph* it=App::ct->objCont->getGraph(cmd.intParams[0]);
            if ((it!=nullptr)&&(cmd.intParams[1]!=-1))
                it->removeGraphData(cmd.intParams[1]);
        }
        if (cmd.cmdId==RENAME_DATASTREAM_GRAPHGUITRIGGEREDCMD)
        {
            CGraph* it=App::ct->objCont->getGraph(cmd.intParams[0]);
            if (it!=nullptr)
            {
                std::string newName(cmd.stringParams[0]);
                CGraphData* grData=it->getGraphData(cmd.intParams[1]);
                if ( (grData!=nullptr)&&(newName!="") )
                {
                    if (grData->getName()!=newName)
                    {
                        tt::removeIllegalCharacters(newName,false);
                        if (it->getGraphData(newName)==nullptr)
                            grData->setName(newName);
                    }
                }
            }
        }
        if (cmd.cmdId==SET_OBJECTSIZE_GRAPHGUITRIGGEREDCMD)
        {
            CGraph* it=App::ct->objCont->getGraph(cmd.intParams[0]);
            if (it!=nullptr)
                it->setSize(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_BUFFERSIZE_GRAPHGUITRIGGEREDCMD)
        {
            CGraph* it=App::ct->objCont->getGraph(cmd.intParams[0]);
            if (it!=nullptr)
                it->setBufferSize(cmd.intParams[1]);
        }
        if (cmd.cmdId==REMOVE_ALLSTATICCURVES_GRAPHGUITRIGGEREDCMD)
        {
            CGraph* it=App::ct->objCont->getGraph(cmd.intParams[0]);
            if (it!=nullptr)
                it->removeAllStatics();
        }
        if (cmd.cmdId==SET_VALUERAWSTATE_GRAPHGUITRIGGEREDCMD)
        {
            CGraph* it=App::ct->objCont->getGraph(cmd.intParams[0]);
            if (it!=nullptr)
            {
                CGraphData* grData=it->getGraphData(cmd.intParams[1]);
                if (grData!=nullptr)
                    grData->setDerivativeIntegralAndCumulative(cmd.intParams[2]);
            }
        }
        if (cmd.cmdId==SET_VALUEMULTIPLIER_GRAPHGUITRIGGEREDCMD)
        {
            CGraph* it=App::ct->objCont->getGraph(cmd.intParams[0]);
            if (it!=nullptr)
            {
                CGraphData* grData=it->getGraphData(cmd.intParams[1]);
                if (grData!=nullptr)
                    grData->setZoomFactor(cmd.floatParams[0]);
            }
        }
        if (cmd.cmdId==SET_VALUEOFFSET_GRAPHGUITRIGGEREDCMD)
        {
            CGraph* it=App::ct->objCont->getGraph(cmd.intParams[0]);
            if (it!=nullptr)
            {
                CGraphData* grData=it->getGraphData(cmd.intParams[1]);
                if (grData!=nullptr)
                    grData->setAddCoeff(cmd.floatParams[0]);
            }
        }
        if (cmd.cmdId==SET_MOVINGAVERAGEPERIOD_GRAPHGUITRIGGEREDCMD)
        {
            CGraph* it=App::ct->objCont->getGraph(cmd.intParams[0]);
            if (it!=nullptr)
            {
                CGraphData* grData=it->getGraphData(cmd.intParams[1]);
                if (grData!=nullptr)
                    grData->setMovingAverageCount(cmd.intParams[2]);
            }
        }
        if (cmd.cmdId==DUPLICATE_TOSTATIC_GRAPHGUITRIGGEREDCMD)
        {
            CGraph* it=App::ct->objCont->getGraph(cmd.intParams[0]);
            if (it!=nullptr)
            {
                CGraphData* grData=it->getGraphData(cmd.intParams[1]);
                if ((grData!=nullptr)&&(grData->getDataLength()!=0))
                {
                    it->makeCurveStatic(cmd.intParams[1],0);
                    App::uiThread->messageBox_information(App::mainWindow,strTranslate(IDSN_GRAPH_CURVE),strTranslate(IDSN_CURVE_WAS_DUPLICATED_TO_STATIC),VMESSAGEBOX_OKELI);
                }
            }
        }





        if (cmd.cmdId==DELETE_CURVE_GRAPHCURVEGUITRIGGEREDCMD)
        {
            CGraph* it=App::ct->objCont->getGraph(cmd.intParams[0]);
            if (it!=nullptr)
            {
                int objID=cmd.intParams[1];
                if (objID!=-1)
                {
                    if (cmd.boolParams[0])
                        it->remove2DPartners(objID);
                    else
                        it->remove3DPartners(objID);
                }
            }
        }
        if (cmd.cmdId==RENAME_CURVE_GRAPHCURVEGUITRIGGEREDCMD)
        {
            CGraph* it=App::ct->objCont->getGraph(cmd.intParams[0]);
            if (it!=nullptr)
            {
                int objID=cmd.intParams[1];
                if (objID!=-1)
                {
                    std::string newName(cmd.stringParams[0]);
                    CGraphDataComb* grData;
                    if (cmd.boolParams[0])
                        grData=it->getGraphData2D(objID);
                    else
                        grData=it->getGraphData3D(objID);
                    if ( (grData!=nullptr)&&(newName!="") )
                    {
                        if (grData->getName()!=newName)
                        {
                            tt::removeIllegalCharacters(newName,false);
                            if (cmd.boolParams[0])
                            {
                                if (it->getGraphData2D(newName)==nullptr)
                                    grData->setName(newName);
                            }
                            else
                            {
                                if (it->getGraphData3D(newName)==nullptr)
                                    grData->setName(newName);
                            }
                        }
                    }
                }
            }
        }
        if (cmd.cmdId==TOGGLE_VISIBLE_GRAPHCURVEGUITRIGGEREDCMD)
        {
            CGraph* it=App::ct->objCont->getGraph(cmd.intParams[0]);
            if ((it!=nullptr)&&(cmd.intParams[1]!=-1))
            {
                CGraphDataComb* grDataComb=nullptr;
                if (cmd.boolParams[0])
                    grDataComb=it->getGraphData2D(cmd.intParams[1]);
                else
                    grDataComb=it->getGraphData3D(cmd.intParams[1]);
                if (grDataComb!=nullptr)
                    grDataComb->setVisible(!grDataComb->getVisible());
            }
        }
        if (cmd.cmdId==TOGGLE_LABEL_GRAPHCURVEGUITRIGGEREDCMD)
        {
            CGraph* it=App::ct->objCont->getGraph(cmd.intParams[0]);
            if ((it!=nullptr)&&(cmd.intParams[1]!=-1))
            {
                CGraphDataComb* grDataComb=nullptr;
                if (cmd.boolParams[0])
                    grDataComb=it->getGraphData2D(cmd.intParams[1]);
                else
                    grDataComb=it->getGraphData3D(cmd.intParams[1]);
                if (grDataComb!=nullptr)
                    grDataComb->setLabel(!grDataComb->getLabel());
            }
        }
        if (cmd.cmdId==TOGGLE_LINKPTS_GRAPHCURVEGUITRIGGEREDCMD)
        {
            CGraph* it=App::ct->objCont->getGraph(cmd.intParams[0]);
            if ((it!=nullptr)&&(cmd.intParams[1]!=-1))
            {
                CGraphDataComb* grDataComb=nullptr;
                if (cmd.boolParams[0])
                    grDataComb=it->getGraphData2D(cmd.intParams[1]);
                else
                    grDataComb=it->getGraphData3D(cmd.intParams[1]);
                if (grDataComb!=nullptr)
                    grDataComb->setLinkPoints(!grDataComb->getLinkPoints());
            }
        }
        if (cmd.cmdId==TOGGLE_ONTOP_GRAPHCURVEGUITRIGGEREDCMD)
        {
            CGraph* it=App::ct->objCont->getGraph(cmd.intParams[0]);
            if ((it!=nullptr)&&(cmd.intParams[1]!=-1))
            {
                CGraphDataComb* grDataComb=nullptr;
                if (cmd.boolParams[0])
                    grDataComb=it->getGraphData2D(cmd.intParams[1]);
                else
                    grDataComb=it->getGraphData3D(cmd.intParams[1]);
                if (grDataComb!=nullptr)
                    grDataComb->setVisibleOnTopOfEverything(!grDataComb->getVisibleOnTopOfEverything());
            }
        }
        if (cmd.cmdId==SET_RELATIVETO_GRAPHCURVEGUITRIGGEREDCMD)
        {
            CGraph* it=App::ct->objCont->getGraph(cmd.intParams[0]);
            if ((it!=nullptr)&&(cmd.intParams[1]!=-1))
            {
                CGraphDataComb* grDataComb=it->getGraphData3D(cmd.intParams[1]);
                if (grDataComb!=nullptr)
                    grDataComb->setCurveRelativeToWorld(cmd.boolParams[0]);
            }
        }
        if (cmd.cmdId==SET_CURVEWIDTH_GRAPHCURVEGUITRIGGEREDCMD)
        {
            CGraph* it=App::ct->objCont->getGraph(cmd.intParams[0]);
            if ((it!=nullptr)&&(cmd.intParams[1]!=-1))
            {
                CGraphDataComb* grDataComb=it->getGraphData3D(cmd.intParams[1]);
                if (grDataComb!=nullptr)
                    grDataComb->set3DCurveWidth(float(cmd.intParams[2]));
            }
        }
        if (cmd.cmdId==DUPLICATE_TOSTATIC_GRAPHCURVEGUITRIGGEREDCMD)
        {
            CGraph* it=App::ct->objCont->getGraph(cmd.intParams[0]);
            if ((it!=nullptr)&&(cmd.intParams[1]!=-1))
            {
                CGraphDataComb* grDataComb=nullptr;
                int dim=1;
                if (cmd.boolParams[0])
                    grDataComb=it->getGraphData2D(cmd.intParams[1]);
                else
                {
                    dim=2;
                    grDataComb=it->getGraphData3D(cmd.intParams[1]);
                }
                if (grDataComb!=nullptr)
                {
                    it->makeCurveStatic(cmd.intParams[1],dim);
                    App::uiThread->messageBox_information(App::mainWindow,strTranslate(IDSN_GRAPH_CURVE),strTranslate(IDSN_CURVE_WAS_DUPLICATED_TO_STATIC),VMESSAGEBOX_OKELI);
                }
            }
        }
        if (cmd.cmdId==ADD_NEWCURVE_GRAPHCURVEGUITRIGGEREDCMD)
        {
            CGraph* it=App::ct->objCont->getGraph(cmd.intParams[0]);
            if (it!=nullptr)
            {
                CGraphDataComb* theNew=new CGraphDataComb();
                theNew->data[0]=cmd.intParams[1];
                theNew->data[1]=cmd.intParams[2];
                if (!cmd.boolParams[0])
                {
                    theNew->data[2]=cmd.intParams[3];
                    it->add3DPartners(theNew);
                }
                else
                    it->add2DPartners(theNew);
                // Now select the object in the UI. We need to post it so that it arrives after the dialog refresh!:
                SSimulationThreadCommand cmd2;
                cmd2.cmdId=CALL_DIALOG_FUNCTION_GUITRIGGEREDCMD;
                cmd2.intParams.push_back(GRAPH2DAND3DCURVES_DLG);
                cmd2.intParams.push_back(0);
                cmd2.intParams.push_back(theNew->getIdentifier());
                App::appendSimulationThreadCommand(cmd2);
            }
        }






        if (cmd.cmdId==SET_POSRELATIVETO_OBJECTMANIPGUITRIGGEREDCMD)
        {
            C3DObject* it=App::ct->objCont->getObjectFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setObjectManipulationTranslationRelativeTo(cmd.intParams[1]);
        }
        if (cmd.cmdId==SET_PERMISSIONS_OBJECTMANIPGUITRIGGEREDCMD)
        {
            C3DObject* it=App::ct->objCont->getObjectFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setObjectManipulationModePermissions(cmd.intParams[1]);
        }
        if (cmd.cmdId==SET_POSSTEPSIZE_OBJECTMANIPGUITRIGGEREDCMD)
        {
            C3DObject* it=App::ct->objCont->getObjectFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
            {
                if (cmd.floatParams[0]<0.0)
                {
                    if (App::ct->simulation->isSimulationStopped())
                        it->setObjectTranslationDisabledDuringNonSimulation(true);
                    else
                        it->setObjectTranslationDisabledDuringSimulation(true);
                }
                else
                {
                    if (App::ct->simulation->isSimulationStopped())
                        it->setObjectTranslationDisabledDuringNonSimulation(false);
                    else
                        it->setObjectTranslationDisabledDuringSimulation(false);
                    it->setNonDefaultTranslationStepSize(cmd.floatParams[0]);
                }
            }
        }
        if (cmd.cmdId==SET_ORSTEPSIZE_OBJECTMANIPGUITRIGGEREDCMD)
        {
            C3DObject* it=App::ct->objCont->getObjectFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
            {
                if (cmd.floatParams[0]<0.0)
                {
                    if (App::ct->simulation->isSimulationStopped())
                        it->setObjectRotationDisabledDuringNonSimulation(true);
                    else
                        it->setObjectRotationDisabledDuringSimulation(true);
                }
                else
                {
                    if (App::ct->simulation->isSimulationStopped())
                        it->setObjectRotationDisabledDuringNonSimulation(false);
                    else
                        it->setObjectRotationDisabledDuringSimulation(false);
                    it->setNonDefaultRotationStepSize(cmd.floatParams[0]);
                }
            }
        }
        if (cmd.cmdId==SET_ORRELATIVETO_OBJECTMANIPGUITRIGGEREDCMD)
        {
            C3DObject* it=App::ct->objCont->getObjectFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setObjectManipulationRotationRelativeTo(cmd.intParams[1]);
        }




        if (cmd.cmdId==SET_TRANSF_POSITIONTRANSLATIONGUITRIGGEREDCMD)
        {
            C3DObject* it=App::ct->objCont->getObjectFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
            {
                if (cmd.intParams[1]==0)
                    it->setLocalTransformation(it->getParentCumulativeTransformation().getInverse()*cmd.transfParams[0]);
                else
                    it->setLocalTransformation(cmd.transfParams[0]);
                if (!App::ct->simulation->isSimulationStopped())
                    simResetDynamicObject_internal(it->getObjectHandle()|sim_handleflag_model); // so that we can also manipulate dynamic objects
            }
        }
        if (cmd.cmdId==APPLY_POS_POSITIONTRANSLATIONGUITRIGGEREDCMD)
        {
            C3DObject* last=App::ct->objCont->getObjectFromHandle(cmd.intParams[0]);
            if (last!=nullptr)
            {
                int coordMode=cmd.intParams[cmd.intParams.size()-2];
                int mask=cmd.intParams[cmd.intParams.size()-1];
                C7Vector tr;
                if (coordMode==0)
                    tr=last->getCumulativeTransformationPart1();
                else
                    tr=last->getLocalTransformationPart1();
                for (size_t i=1;i<cmd.intParams.size()-2;i++)
                {
                    C3DObject* it=App::ct->objCont->getObjectFromHandle(cmd.intParams[i]);
                    C7Vector trIt;
                    if (coordMode==0)
                        trIt=it->getCumulativeTransformationPart1();
                    else
                        trIt=it->getLocalTransformationPart1();
                    if (mask&1)
                        trIt.X(0)=tr.X(0);
                    if (mask&2)
                        trIt.X(1)=tr.X(1);
                    if (mask&4)
                        trIt.X(2)=tr.X(2);
                    if (coordMode==0)
                        it->setLocalTransformation(it->getParentCumulativeTransformation().getInverse()*trIt);
                    else
                        it->setLocalTransformation(trIt);
                    if (!App::ct->simulation->isSimulationStopped())
                        simResetDynamicObject_internal(it->getObjectHandle()|sim_handleflag_model); // so that we can also manipulate dynamic objects
                }
            }
        }
        if (cmd.cmdId==TRANSLATESCALE_SELECTION_POSITIONTRANSLATIONGUITRIGGEREDCMD)
        {
            int transfMode=cmd.intParams[cmd.intParams.size()-2];
            int t=cmd.intParams[cmd.intParams.size()-1];
            float scalingValues[3];
            scalingValues[0]=cmd.floatParams[0];
            scalingValues[1]=cmd.floatParams[1];
            scalingValues[2]=cmd.floatParams[2];
            float translationValues[3];
            translationValues[0]=cmd.floatParams[3];
            translationValues[1]=cmd.floatParams[4];
            translationValues[2]=cmd.floatParams[5];
            // Prepare the object that will be translated/scaled, and all other objects in selection appropriately:
            // There is one master object that acts as the translation/scaling pivot. That object needs to be carefully selected
            std::vector<C3DObject*> allSelObj;
            for (size_t i=0;i<cmd.intParams.size()-2;i++)
            {
                C3DObject* it=App::ct->objCont->getObjectFromHandle(cmd.intParams[i]);
                allSelObj.push_back(it);
            }
            std::vector<C3DObject*> allSelObjects;
            std::map<C3DObject*,bool> occ;
            C3DObject* masterObj=nullptr;
            for (int i=int(allSelObj.size())-1;i>=0;i--)
            {
                C3DObject* it=allSelObj[i]->getLastParentInSelection(&allSelObj);
                if (it==nullptr)
                    it=allSelObj[i];
                std::map<C3DObject*,bool>::iterator it2=occ.find(it);
                if (it2==occ.end())
                {
                    occ[it]=true;
                    if (masterObj==nullptr)
                        masterObj=it;
                    else
                        allSelObjects.push_back(it);
                }
            }
            if (masterObj!=nullptr)
            {
                C7Vector oldTr(masterObj->getCumulativeTransformationPart1());
                // Translate/Scale the master's position:
                C7Vector tr;
                if (transfMode==0)
                    tr=masterObj->getCumulativeTransformationPart1();
                else
                    tr=masterObj->getLocalTransformationPart1();
                if (t==2)
                {
                    tr.X(0)=tr.X(0)*scalingValues[0];
                    tr.X(1)=tr.X(1)*scalingValues[1];
                    tr.X(2)=tr.X(2)*scalingValues[2];
                }
                else
                {
                    C7Vector m;
                    m.setIdentity();
                    if (t==1)
                        m.X.set(translationValues);
                    if (transfMode==2)
                        tr=tr*m;
                    else
                        tr=m*tr;
                }
                if (transfMode==0)
                    tr=masterObj->getParentCumulativeTransformation().getInverse()*tr;
                masterObj->setLocalTransformation(tr);
                if (!App::ct->simulation->isSimulationStopped())
                    simResetDynamicObject_internal(masterObj->getObjectHandle()|sim_handleflag_model); // so that we can also manipulate dynamic objects

                // Now move the "slaves" appropriately:
                C7Vector newTr(masterObj->getCumulativeTransformationPart1());
                C7Vector shift(newTr*oldTr.getInverse());
                for (size_t i=0;i<allSelObjects.size();i++)
                {
                    C3DObject* obj=allSelObjects[i];
                    C7Vector oldLTr=obj->getLocalTransformationPart1();
                    C7Vector parentTr=obj->getParentCumulativeTransformation();
                    obj->setLocalTransformation(parentTr.getInverse()*shift*parentTr*oldLTr);
                    if (!App::ct->simulation->isSimulationStopped())
                        simResetDynamicObject_internal(obj->getObjectHandle()|sim_handleflag_model); // so that we can also manipulate dynamic objects
                }
            }
        }

        if (cmd.cmdId==APPLY_OR_ORIENTATIONROTATIONGUITRIGGEREDCMD)
        {
            C3DObject* last=App::ct->objCont->getObjectFromHandle(cmd.intParams[0]);
            if (last!=nullptr)
            {
                int coordMode=cmd.intParams[cmd.intParams.size()-1];
                C7Vector tr;
                if (coordMode==0)
                    tr=last->getCumulativeTransformationPart1();
                else
                    tr=last->getLocalTransformationPart1();
                for (size_t i=1;i<cmd.intParams.size()-1;i++)
                {
                    C3DObject* it=App::ct->objCont->getObjectFromHandle(cmd.intParams[i]);
                    C7Vector trIt;
                    if (coordMode==0)
                        trIt=it->getCumulativeTransformationPart1();
                    else
                        trIt=it->getLocalTransformationPart1();
                    trIt.Q=tr.Q;
                    if (coordMode==0)
                        it->setLocalTransformation(it->getParentCumulativeTransformation().getInverse()*trIt);
                    else
                        it->setLocalTransformation(trIt);
                    if (!App::ct->simulation->isSimulationStopped())
                        simResetDynamicObject_internal(it->getObjectHandle()|sim_handleflag_model); // so that we can also manipulate dynamic objects
                }
            }
        }
        if (cmd.cmdId==ROTATE_SELECTION_ORIENTATIONROTATIONGUITRIGGEREDCMD)
        {
            int transfMode=cmd.intParams[cmd.intParams.size()-1];
            float rotAngles[3];
            rotAngles[0]=cmd.floatParams[0];
            rotAngles[1]=cmd.floatParams[1];
            rotAngles[2]=cmd.floatParams[2];
            // Prepare the object that will be rotated, and all other objects in selection appropriately:
            // There is one master object that acts as the rotation pivot. That object needs to be carefully selected
            std::vector<C3DObject*> allSelObj;
            for (size_t i=0;i<cmd.intParams.size()-1;i++)
            {
                C3DObject* it=App::ct->objCont->getObjectFromHandle(cmd.intParams[i]);
                allSelObj.push_back(it);
            }
            std::vector<C3DObject*> allSelObjects;
            std::map<C3DObject*,bool> occ;
            C3DObject* masterObj=nullptr;
            for (int i=int(allSelObj.size())-1;i>=0;i--)
            {
                C3DObject* it=allSelObj[i]->getLastParentInSelection(&allSelObj);
                if (it==nullptr)
                    it=allSelObj[i];
                std::map<C3DObject*,bool>::iterator it2=occ.find(it);
                if (it2==occ.end())
                {
                    occ[it]=true;
                    if (masterObj==nullptr)
                        masterObj=it;
                    else
                        allSelObjects.push_back(it);
                }
            }
            if (masterObj!=nullptr)
            {
                C7Vector oldTr(masterObj->getCumulativeTransformationPart1());
                // Rotate the master:
                C7Vector tr;
                if (transfMode==0)
                    tr=masterObj->getCumulativeTransformationPart1();
                else
                    tr=masterObj->getLocalTransformationPart1();
                C7Vector m;
                m.setIdentity();
                m.Q.setEulerAngles(rotAngles[0],rotAngles[1],rotAngles[2]);
                if (transfMode==2)
                    tr=tr*m;
                else
                    tr=m*tr;
                if (transfMode==0)
                    tr=masterObj->getParentCumulativeTransformation().getInverse()*tr;
                masterObj->setLocalTransformation(tr);
                if (!App::ct->simulation->isSimulationStopped())
                    simResetDynamicObject_internal(masterObj->getObjectHandle()|sim_handleflag_model); // so that we can also manipulate dynamic objects

                // Now rotate the "slaves":
                C7Vector newTr(masterObj->getCumulativeTransformationPart1());
                C7Vector shift(newTr*oldTr.getInverse());
                for (size_t i=0;i<allSelObjects.size();i++)
                {
                    C3DObject* obj=allSelObjects[i];
                    C7Vector oldLTr=obj->getLocalTransformationPart1();
                    C7Vector parentTr=obj->getParentCumulativeTransformation();
                    obj->setLocalTransformation(parentTr.getInverse()*shift*parentTr*oldLTr);
                    if (!App::ct->simulation->isSimulationStopped())
                        simResetDynamicObject_internal(obj->getObjectHandle()|sim_handleflag_model); // so that we can also manipulate dynamic objects
                }
            }
        }



        if (cmd.cmdId==REMOVE_ELEMENT_IKELEMENTGUITRIGGEREDCMD)
        {
            CikGroup* ikGroup=App::ct->ikGroups->getIkGroup(cmd.intParams[0]);
            if (ikGroup!=nullptr)
            {
                CikEl* it=ikGroup->getIkElement(cmd.intParams[1]);
                if (it!=nullptr)
                    ikGroup->removeIkElement(cmd.intParams[1]);
            }
        }
        if (cmd.cmdId==ADD_ELEMENT_IKELEMENTGUITRIGGEREDCMD)
        {
            CikGroup* ikGroup=App::ct->ikGroups->getIkGroup(cmd.intParams[0]);
            if (ikGroup!=nullptr)
            {
                CDummy* it=App::ct->objCont->getDummy(cmd.intParams[1]);
                if (it!=nullptr)
                {
                    CikEl* newIkEl=new CikEl(it->getObjectHandle());
                    if (!ikGroup->addIkElement(newIkEl))
                        delete newIkEl;
                    else
                    {
                        // Now select the object in the UI. We need to post it so that it arrives after the dialog refresh!:
                        SSimulationThreadCommand cmd2;
                        cmd2.cmdId=CALL_DIALOG_FUNCTION_GUITRIGGEREDCMD;
                        cmd2.intParams.push_back(IKELEMENT_DLG);
                        cmd2.intParams.push_back(0);
                        cmd2.intParams.push_back(newIkEl->getObjectID());
                        App::appendSimulationThreadCommand(cmd2);
                    }
                }
            }
        }
        if (cmd.cmdId==TOGGLE_ACTIVE_IKELEMENTGUITRIGGEREDCMD)
        {
            CikGroup* ikGroup=App::ct->ikGroups->getIkGroup(cmd.intParams[0]);
            if (ikGroup!=nullptr)
            {
                CikEl* it=ikGroup->getIkElement(cmd.intParams[1]);
                if (it!=nullptr)
                    it->setActive(!it->getActive());
            }
        }
        if (cmd.cmdId==SET_BASE_IKELEMENTGUITRIGGEREDCMD)
        {
            CikGroup* ikGroup=App::ct->ikGroups->getIkGroup(cmd.intParams[0]);
            if (ikGroup!=nullptr)
            {
                CikEl* it=ikGroup->getIkElement(cmd.intParams[1]);
                if (it!=nullptr)
                    it->setBase(cmd.intParams[2]);
            }
        }
        if (cmd.cmdId==TOGGLE_CONSTRAINT_IKELEMENTGUITRIGGEREDCMD)
        {
            CikGroup* ikGroup=App::ct->ikGroups->getIkGroup(cmd.intParams[0]);
            if (ikGroup!=nullptr)
            {
                CikEl* it=ikGroup->getIkElement(cmd.intParams[1]);
                if (it!=nullptr)
                {
                    int c=cmd.intParams[2];
                    if ((c==sim_ik_x_constraint)||(c==sim_ik_y_constraint)||(c==sim_ik_z_constraint)||(c==sim_ik_gamma_constraint))
                        it->setConstraints(it->getConstraints()^c);
                    if (c==sim_ik_alpha_beta_constraint)
                    {
                        it->setConstraints(it->getConstraints()^sim_ik_alpha_beta_constraint);
                        if ((it->getConstraints()&(sim_ik_alpha_beta_constraint|sim_ik_gamma_constraint))==sim_ik_gamma_constraint)
                            it->setConstraints(it->getConstraints()^sim_ik_gamma_constraint); // gamma constraint cannot be selected if alpha-beta constraint is not selected!
                    }
                }
            }
        }
        if (cmd.cmdId==SET_REFERENCEFRAME_IKELEMENTGUITRIGGEREDCMD)
        {
            CikGroup* ikGroup=App::ct->ikGroups->getIkGroup(cmd.intParams[0]);
            if (ikGroup!=nullptr)
            {
                CikEl* it=ikGroup->getIkElement(cmd.intParams[1]);
                if (it!=nullptr)
                    it->setAlternativeBaseForConstraints(cmd.intParams[2]);
            }
        }
        if (cmd.cmdId==SET_PRECISION_IKELEMENTGUITRIGGEREDCMD)
        {
            CikGroup* ikGroup=App::ct->ikGroups->getIkGroup(cmd.intParams[0]);
            if (ikGroup!=nullptr)
            {
                CikEl* it=ikGroup->getIkElement(cmd.intParams[1]);
                if (it!=nullptr)
                {
                    if (cmd.intParams[2]==0)
                        it->setMinLinearPrecision(cmd.floatParams[0]);
                    else
                        it->setMinAngularPrecision(cmd.floatParams[0]);
                }
            }
        }
        if (cmd.cmdId==SET_WEIGHT_IKELEMENTGUITRIGGEREDCMD)
        {
            CikGroup* ikGroup=App::ct->ikGroups->getIkGroup(cmd.intParams[0]);
            if (ikGroup!=nullptr)
            {
                CikEl* it=ikGroup->getIkElement(cmd.intParams[1]);
                if (it!=nullptr)
                {
                    if (cmd.intParams[2]==0)
                        it->setPositionWeight(cmd.floatParams[0]);
                    else
                        it->setOrientationWeight(cmd.floatParams[0]);
                }
            }
        }





        if (cmd.cmdId==TOGGLE_ALLENABLED_IKGROUPGUITRIGGEREDCMD)
        {
            App::ct->mainSettings->ikCalculationEnabled=!App::ct->mainSettings->ikCalculationEnabled;
        }
        if (cmd.cmdId==REMOVE_IKGROUP_IKGROUPGUITRIGGEREDCMD)
        {
            CikGroup* it=App::ct->ikGroups->getIkGroup(cmd.intParams[0]);
            if (it!=nullptr)
                App::ct->ikGroups->removeIkGroup(it->getObjectID());
        }
        if (cmd.cmdId==ADD_IKGROUP_IKGROUPGUITRIGGEREDCMD)
        {
            CikGroup* newGroup=new CikGroup();
            newGroup->setObjectName("IK_Group");
            App::ct->ikGroups->addIkGroup(newGroup,false);
            // Now select the object in the UI. We need to post it so that it arrives after the dialog refresh!:
            SSimulationThreadCommand cmd2;
            cmd2.cmdId=CALL_DIALOG_FUNCTION_GUITRIGGEREDCMD;
            cmd2.intParams.push_back(IK_DLG);
            cmd2.intParams.push_back(0);
            cmd2.intParams.push_back(newGroup->getObjectID());
            App::appendSimulationThreadCommand(cmd2);
            // Following second refresh is needed so that the up/down buttons become enabled:
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
        if (cmd.cmdId==RENAME_IKGROUP_IKGROUPGUITRIGGEREDCMD)
        {
            CikGroup* it=App::ct->ikGroups->getIkGroup(cmd.intParams[0]);
            std::string newName(cmd.stringParams[0]);
            if ( (it!=nullptr)&&(newName!="") )
            {
                if (it->getObjectName()!=newName)
                {
                    tt::removeIllegalCharacters(newName,true);
                    if (App::ct->ikGroups->getIkGroup(newName)==nullptr)
                        it->setObjectName(newName);
                }
            }
        }
        if (cmd.cmdId==SHIFT_IKGROUP_IKGROUPGUITRIGGEREDCMD)
        {
            CikGroup* it=App::ct->ikGroups->getIkGroup(cmd.intParams[0]);
            bool up=cmd.intParams[1]!=0;
            if (it!=nullptr)
            {
                for (int i=0;i<int(App::ct->ikGroups->ikGroups.size());i++)
                {
                    if (up)
                    {
                        if ( (App::ct->ikGroups->ikGroups[i]->getObjectID()==it->getObjectID())&&(i>0) )
                        {
                            App::ct->ikGroups->ikGroups.erase(App::ct->ikGroups->ikGroups.begin()+i);
                            App::ct->ikGroups->ikGroups.insert(App::ct->ikGroups->ikGroups.begin()+i-1,it);
                            break;
                        }
                    }
                    else
                    {
                        if ( (App::ct->ikGroups->ikGroups[i]->getObjectID()==it->getObjectID())&&(i<int(App::ct->ikGroups->ikGroups.size())-1) )
                        {
                            App::ct->ikGroups->ikGroups.erase(App::ct->ikGroups->ikGroups.begin()+i);
                            App::ct->ikGroups->ikGroups.insert(App::ct->ikGroups->ikGroups.begin()+i+1,it);
                            break;
                        }
                    }
                    // Now select the object in the UI. We need to post it so that it arrives after the dialog refresh!:
                    SSimulationThreadCommand cmd2;
                    cmd2.cmdId=CALL_DIALOG_FUNCTION_GUITRIGGEREDCMD;
                    cmd2.intParams.push_back(IK_DLG);
                    cmd2.intParams.push_back(0);
                    cmd2.intParams.push_back(it->getObjectID());
                    App::appendSimulationThreadCommand(cmd2);
                }
                // Now we have to check if the getDoOnFailOrSuccessOf is still consistent for all IKGroups:
                std::vector<int> seenIDs;
                for (int i=0;i<int(App::ct->ikGroups->ikGroups.size());i++)
                {
                    CikGroup* theGroup=App::ct->ikGroups->ikGroups[i];
                    int dep=theGroup->getDoOnFailOrSuccessOf();
                    if (dep!=-1)
                    {
                        bool valid=false;
                        for (int j=0;j<int(seenIDs.size());j++)
                        {
                            if (seenIDs[j]==dep)
                            {
                                valid=true;
                                break;
                            }
                        }
                        if (!valid)
                            theGroup->setDoOnFailOrSuccessOf(-1);
                    }
                    seenIDs.push_back(theGroup->getObjectID());
                }
            }
        }
        if (cmd.cmdId==TOGGLE_EXPLICITHANDLING_IKGROUPGUITRIGGEREDCMD)
        {
            CikGroup* it=App::ct->ikGroups->getIkGroup(cmd.intParams[0]);
            if (it!=nullptr)
                it->setExplicitHandling(!it->getExplicitHandling());
        }
        if (cmd.cmdId==TOGGLE_ACTIVE_IKGROUPGUITRIGGEREDCMD)
        {
            CikGroup* it=App::ct->ikGroups->getIkGroup(cmd.intParams[0]);
            if (it!=nullptr)
                it->setActive(!it->getActive());
        }
        if (cmd.cmdId==TOGGLE_REDUNDANT_IKGROUPGUITRIGGEREDCMD)
        {
            CikGroup* it=App::ct->ikGroups->getIkGroup(cmd.intParams[0]);
            if (it!=nullptr)
                it->setCorrectJointLimits(!it->getCorrectJointLimits());
        }
        if (cmd.cmdId==TOGGLE_IGNOREMAXSTEPSIZES_IKGROUPGUITRIGGEREDCMD)
        {
            CikGroup* it=App::ct->ikGroups->getIkGroup(cmd.intParams[0]);
            if (it!=nullptr)
                it->setIgnoreMaxStepSizes(!it->getIgnoreMaxStepSizes());
        }
        if (cmd.cmdId==SET_CALCMETHOD_IKGROUPGUITRIGGEREDCMD)
        {
            CikGroup* it=App::ct->ikGroups->getIkGroup(cmd.intParams[0]);
            if (it!=nullptr)
                it->setCalculationMethod(cmd.intParams[1]);
        }
        if (cmd.cmdId==SET_DAMPING_IKGROUPGUITRIGGEREDCMD)
        {
            CikGroup* it=App::ct->ikGroups->getIkGroup(cmd.intParams[0]);
            if (it!=nullptr)
                it->setDlsFactor(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_ITERATIONS_IKGROUPGUITRIGGEREDCMD)
        {
            CikGroup* it=App::ct->ikGroups->getIkGroup(cmd.intParams[0]);
            if (it!=nullptr)
                it->setMaxIterations(cmd.intParams[1]);
        }
        if (cmd.cmdId==SET_CALCWEIGHT_IKGROUPGUITRIGGEREDCMD)
        {
            CikGroup* it=App::ct->ikGroups->getIkGroup(cmd.intParams[0]);
            if (it!=nullptr)
            {
                if (cmd.intParams[1]==0)
                    it->setJointLimitWeight(cmd.floatParams[0]);
                else
                    it->setAvoidanceWeight(cmd.floatParams[0]);
            }
        }
        if (cmd.cmdId==SET_LIMITTHRESHOLD_IKGROUPGUITRIGGEREDCMD)
        {
            CikGroup* it=App::ct->ikGroups->getIkGroup(cmd.intParams[0]);
            if (it!=nullptr)
            {
                if (cmd.intParams[1]==0)
                    it->setJointTreshholdLinear(cmd.floatParams[0]);
                else
                    it->setJointTreshholdAngular(cmd.floatParams[0]);
            }
        }
        if (cmd.cmdId==SET_CONDITIONALPARAMS_IKGROUPGUITRIGGEREDCMD)
        {
            CikGroup* it=App::ct->ikGroups->getIkGroup(cmd.intParams[0]);
            if (it!=nullptr)
            {
                CikGroup* it2=App::ct->ikGroups->getIkGroup(cmd.intParams[1]);
                int id=-1;
                if (it2!=nullptr)
                    id=it2->getObjectID();
                it->setDoOnFailOrSuccessOf(id);
                it->setDoOnPerformed(cmd.intParams[2]<2);
                it->setDoOnFail(cmd.intParams[2]==0);
                it->setRestoreIfPositionNotReached(cmd.boolParams[0]);
                it->setRestoreIfOrientationNotReached(cmd.boolParams[1]);
            }
        }
        if (cmd.cmdId==SET_AVOIDANCEPARAMS_IKGROUPGUITRIGGEREDCMD)
        {
            CikGroup* it=App::ct->ikGroups->getIkGroup(cmd.intParams[0]);
            if (it!=nullptr)
            {
                it->setAvoidanceRobot(cmd.intParams[1]);
                it->setAvoidanceObstacle(cmd.intParams[2]);
                if (cmd.boolParams[0])
                    it->setConstraints(it->getConstraints()|sim_ik_avoidance_constraint);
                else
                    it->setConstraints((it->getConstraints()|sim_ik_avoidance_constraint)-sim_ik_avoidance_constraint);
                it->setAvoidanceThreshold(cmd.floatParams[0]);
            }
        }








        if (cmd.cmdId==REMOVE_OBJECT_MOTIONPLANNINGGUITRIGGEREDCMD)
        {
            CMotionPlanningTask* it=App::ct->motionPlanning->getObject(cmd.intParams[0]);
            if (it!=nullptr)
                App::ct->motionPlanning->removeObject(cmd.intParams[0]);
        }
        if (cmd.cmdId==RENAME_OBJECT_MOTIONPLANNINGGUITRIGGEREDCMD)
        {
            CMotionPlanningTask* it=App::ct->motionPlanning->getObject(cmd.intParams[0]);
            std::string newName(cmd.stringParams[0]);
            if ( (it!=nullptr)&&(newName!="") )
            {
                if (it->getObjectName()!=newName)
                {
                    tt::removeIllegalCharacters(newName,true);
                    if (App::ct->motionPlanning->getObject(newName)==nullptr)
                        it->setObjectName(newName);
                }
            }
        }
        if (cmd.cmdId==ADD_OBJECT_MOTIONPLANNINGGUITRIGGEREDCMD)
        {
            std::vector<int> joints;
            for (size_t i=0;i<cmd.intParams.size();i++)
            {
                CJoint* it=App::ct->objCont->getJoint(cmd.intParams[i]);
                if (it!=nullptr)
                    joints.push_back(it->getObjectHandle());
            }
            if (joints.size()>0)
            {
                CMotionPlanningTask* task=new CMotionPlanningTask(joints);
                App::ct->motionPlanning->addObject(task,false);
                // Now select the object in the UI. We need to post it so that it arrives after the dialog refresh!:
                SSimulationThreadCommand cmd2;
                cmd2.cmdId=CALL_DIALOG_FUNCTION_GUITRIGGEREDCMD;
                cmd2.intParams.push_back(MOTION_PLANNING_DLG);
                cmd2.intParams.push_back(0);
                cmd2.intParams.push_back(task->getObjectID());
                App::appendSimulationThreadCommand(cmd2);
                // Following second refresh is needed so that the up/down buttons become enabled:
                App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
            }
        }
        if (cmd.cmdId==SET_ASSOCIATEDIKGROUP_MOTIONPLANNINGGUITRIGGEREDCMD)
        {
            CMotionPlanningTask* it=App::ct->motionPlanning->getObject(cmd.intParams[0]);
            CikGroup* ik=App::ct->ikGroups->getIkGroup(cmd.intParams[1]);
            if (it!=nullptr)
            {
                int ikid=-1;
                if (ik!=nullptr)
                    ikid=cmd.intParams[1];
                it->setIkGroup(ikid);
            }
        }
        if (cmd.cmdId==SET_ENTITY1_MOTIONPLANNINGGUITRIGGEREDCMD)
        {
            CMotionPlanningTask* it=App::ct->motionPlanning->getObject(cmd.intParams[0]);
            if (it!=nullptr)
            {
                if (cmd.boolParams[0])
                    it->setRobotSelfCollEntity1(cmd.intParams[1]);
                else
                    it->setRobotEntity(cmd.intParams[1]);
            }
        }
        if (cmd.cmdId==SET_ENTITY2_MOTIONPLANNINGGUITRIGGEREDCMD)
        {
            CMotionPlanningTask* it=App::ct->motionPlanning->getObject(cmd.intParams[0]);
            if (it!=nullptr)
            {
                if (cmd.boolParams[0])
                    it->setRobotSelfCollEntity2(cmd.intParams[1]);
                else
                    it->setObstacleEntity(cmd.intParams[1]);
            }
        }
        if (cmd.cmdId==SET_COLLDIST_MOTIONPLANNINGGUITRIGGEREDCMD)
        {
            CMotionPlanningTask* it=App::ct->motionPlanning->getObject(cmd.intParams[0]);
            if (it!=nullptr)
            {
                if (cmd.boolParams[0])
                    it->setSelfCollDistanceThreshold(cmd.floatParams[0]);
                else
                    it->setDistanceThreshold(cmd.floatParams[0]);
            }
        }
        if (cmd.cmdId==COMPUTE_PHASE1NODES_MOTIONPLANNINGGUITRIGGEREDCMD)
        {
            CMotionPlanningTask* it=App::ct->motionPlanning->getObject(cmd.intParams[0]);
            if (it!=nullptr)
            {
                it->clearDataStructure();
                int collidingNodes,totalNodes;
                if (it->calculateDataStructureIfNeeded(&collidingNodes,&totalNodes,false))
                {
                    std::string tmp(IDSN_DONE_TOTAL_NODES);
                    tmp+=tt::FNb(0,totalNodes);
                    App::uiThread->messageBox_information(App::mainWindow,strTranslate(IDSN_MOTION_PLANNING),tmp.c_str(),VMESSAGEBOX_OKELI);
                }
                else
                    App::uiThread->messageBox_information(App::mainWindow,strTranslate(IDSN_MOTION_PLANNING),strTranslate(IDSN_MOTION_PLANNING_PARAMETERS_ILL_DEFINED),VMESSAGEBOX_OKELI);
            }
        }
        if (cmd.cmdId==TOGGLE_PHASE1NODES_MOTIONPLANNINGGUITRIGGEREDCMD)
        {
            CMotionPlanningTask* it=App::ct->motionPlanning->getObject(cmd.intParams[0]);
            if (it!=nullptr)
                it->setPhase1NodeVisualization(!it->getPhase1NodeVisualization());
        }
        if (cmd.cmdId==SET_JOINTPROPERTIES_MOTIONPLANNINGGUITRIGGEREDCMD)
        {
            CMotionPlanningTask* it=App::ct->motionPlanning->getObject(cmd.intParams[0]);
            if (it!=nullptr)
            {
                for (size_t i=0;i<cmd.intVectorParams[0].size();i++)
                {
                    it->setJointStepCount(cmd.intVectorParams[0][i],cmd.intVectorParams[1][i]);
                    it->setRobotMetric(cmd.intVectorParams[0][i],cmd.floatVectorParams[0][i]);
                }
            }
        }
        if (cmd.cmdId==SET_WORKSPACEMETRIC_MOTIONPLANNINGGUITRIGGEREDCMD)
        {
            CMotionPlanningTask* it=App::ct->motionPlanning->getObject(cmd.intParams[0]);
            if (it!=nullptr)
            {
                it->setTipMetric(0,cmd.floatParams[0]);
                it->setTipMetric(1,cmd.floatParams[1]);
                it->setTipMetric(2,cmd.floatParams[2]);
                it->setTipMetric(3,cmd.floatParams[3]);
            }
        }





        if (cmd.cmdId==REMOVE_OBJECT_PATHPLANNINGGUITRIGGEREDCMD)
        {
            CPathPlanningTask* it=App::ct->pathPlanning->getObject(cmd.intParams[0]);
            if (it!=nullptr)
                App::ct->pathPlanning->removeObject(cmd.intParams[0]);
        }
        if (cmd.cmdId==RENAME_OBJECT_PATHPLANNINGGUITRIGGEREDCMD)
        {
            CPathPlanningTask* it=App::ct->pathPlanning->getObject(cmd.intParams[0]);
            std::string newName(cmd.stringParams[0]);
            if ( (it!=nullptr)&&(newName!="") )
            {
                if (it->getObjectName()!=newName)
                {
                    tt::removeIllegalCharacters(newName,true);
                    if (App::ct->pathPlanning->getObject(newName)==nullptr)
                        it->setObjectName(newName);
                }
            }
        }
        if (cmd.cmdId==ADD_OBJECT_PATHPLANNINGGUITRIGGEREDCMD)
        {
            CDummy* it=App::ct->objCont->getDummy(cmd.intParams[1]);
            if (it!=nullptr)
            {
                CPathPlanningTask* task=new CPathPlanningTask(it->getObjectHandle(),cmd.intParams[0]);
                App::ct->pathPlanning->addObject(task,false);
                // Now select the object in the UI. We need to post it so that it arrives after the dialog refresh!:
                SSimulationThreadCommand cmd2;
                cmd2.cmdId=CALL_DIALOG_FUNCTION_GUITRIGGEREDCMD;
                cmd2.intParams.push_back(PATH_PLANNING_DLG);
                cmd2.intParams.push_back(0);
                cmd2.intParams.push_back(task->getObjectID());
                App::appendSimulationThreadCommand(cmd2);
                // Following second refresh is needed so that the up/down buttons become enabled:
                App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
            }
        }
        if (cmd.cmdId==SET_GOALDUMMY_PATHPLANNINGGUITRIGGEREDCMD)
        {
            CPathPlanningTask* it=App::ct->pathPlanning->getObject(cmd.intParams[0]);
            if (it!=nullptr)
                it->setGoalDummyID(cmd.intParams[1]);
        }
        if (cmd.cmdId==SET_PATHOBJECT_PATHPLANNINGGUITRIGGEREDCMD)
        {
            CPathPlanningTask* it=App::ct->pathPlanning->getObject(cmd.intParams[0]);
            if (it!=nullptr)
                it->setPathID(cmd.intParams[1]);
        }
        if (cmd.cmdId==SET_CHECKTYPE_PATHPLANNINGGUITRIGGEREDCMD)
        {
            CPathPlanningTask* it=App::ct->pathPlanning->getObject(cmd.intParams[0]);
            if (it!=nullptr)
            {
                it->setCollisionDetection(cmd.intParams[1]==0);
                it->setObstacleMaxDistanceEnabled(cmd.intParams[1]==2);
            }
        }
        if (cmd.cmdId==SET_ROBOT_PATHPLANNINGGUITRIGGEREDCMD)
        {
            CPathPlanningTask* it=App::ct->pathPlanning->getObject(cmd.intParams[0]);
            if (it!=nullptr)
                it->setRobotEntityID(cmd.intParams[1]);
        }
        if (cmd.cmdId==SET_OBSTACLE_PATHPLANNINGGUITRIGGEREDCMD)
        {
            CPathPlanningTask* it=App::ct->pathPlanning->getObject(cmd.intParams[0]);
            if (it!=nullptr)
                it->setObstacleEntityID(cmd.intParams[1]);
        }
        if (cmd.cmdId==SET_DISTANCES_PATHPLANNINGGUITRIGGEREDCMD)
        {
            CPathPlanningTask* it=App::ct->pathPlanning->getObject(cmd.intParams[0]);
            if (it!=nullptr)
            {
                if (cmd.intParams[1]==0)
                    it->setObstacleClearance(cmd.floatParams[0]);
                else
                    it->setObstacleMaxDistance(cmd.floatParams[0]);
            }
        }
        if (cmd.cmdId==TOGGLE_USEPARTIALPATH_PATHPLANNINGGUITRIGGEREDCMD)
        {
            CPathPlanningTask* it=App::ct->pathPlanning->getObject(cmd.intParams[0]);
            if (it!=nullptr)
                it->setPartialPathIsOk(!it->getPartialPathIsOk());
        }
        if (cmd.cmdId==TOGGLE_SHOWSEARCHEDNODES_PATHPLANNINGGUITRIGGEREDCMD)
        {
            CPathPlanningTask* it=App::ct->pathPlanning->getObject(cmd.intParams[0]);
            if (it!=nullptr)
                it->setShowSearchTrees(!it->getShowSearchTrees());
        }
        if (cmd.cmdId==SET_CALCTIME_PATHPLANNINGGUITRIGGEREDCMD)
        {
            CPathPlanningTask* it=App::ct->pathPlanning->getObject(cmd.intParams[0]);
            if (it!=nullptr)
                it->setMaxTime(float(cmd.intParams[1]));
        }
        if (cmd.cmdId==SET_POSTPROCESSINGPASSES_PATHPLANNINGGUITRIGGEREDCMD)
        {
            CPathPlanningTask* it=App::ct->pathPlanning->getObject(cmd.intParams[0]);
            if (it!=nullptr)
                it->setPostProcessingPassCount(cmd.intParams[1]);
        }
        if (cmd.cmdId==COMPUTE_PATHPLANNINGGUITRIGGEREDCMD)
        {
            CPathPlanningTask* it=App::ct->pathPlanning->getObject(cmd.intParams[0]);
            if (it!=nullptr)
                it->performSearch(true,0.0f);
        }
        if (cmd.cmdId==SET_VARIOUSPARAMS_PATHPLANNINGGUITRIGGEREDCMD)
        {
            CPathPlanningTask* it=App::ct->pathPlanning->getObject(cmd.intParams[0]);
            if (it!=nullptr)
            {
                it->setHolonomicTaskType(cmd.intParams[2]);
                it->setVisualizeSearchArea(cmd.boolParams[0]);
                it->setStepSize(cmd.floatParams[0]);
                it->setAngularStepSize(cmd.floatParams[1]);
                it->setMinTurningCircleDiameter(cmd.floatParams[2]);
                it->setGammaAxis(cmd.posParams[0]);
                it->setSearchRange(&cmd.floatVectorParams[0][0],&cmd.floatVectorParams[1][0]);
                it->setSearchDirection(&cmd.intVectorParams[0][0]);
            }
        }






        if (cmd.cmdId==ADD_UI_OPENGLUIBLOCKGUITRIGGEREDCMD)
        {
            if (App::getEditModeType()==BUTTON_EDIT_MODE)
            {
                App::ct->buttonBlockContainer->deselectButtons();
                int s[2]={cmd.intParams[0],cmd.intParams[1]};
                int cs[2]={14,14};
                int hs[20];
                int h=simCreateUI_internal("UI",cmd.intParams[2],s,cs,hs);
                App::ct->buttonBlockContainer->setBlockInEdition(h);
            }
        }
        if (cmd.cmdId==SET_CELLCNT_OPENGLUIBLOCKGUITRIGGEREDCMD)
        {
            if (App::getEditModeType()==BUTTON_EDIT_MODE)
            {
                CButtonBlock* itBlock=App::ct->buttonBlockContainer->getBlockWithID(cmd.intParams[0]);
                if (itBlock!=nullptr)
                {
                    VPoint size;
                    itBlock->getBlockSize(size);
                    if (cmd.boolParams[0])
                        size.y=cmd.intParams[1];
                    else
                        size.x=cmd.intParams[1];
                    itBlock->setBlockSize(size);
                }
            }
        }
        if (cmd.cmdId==SET_CELLSIZE_OPENGLUIBLOCKGUITRIGGEREDCMD)
        {
            if (App::getEditModeType()==BUTTON_EDIT_MODE)
            {
                CButtonBlock* itBlock=App::ct->buttonBlockContainer->getBlockWithID(cmd.intParams[0]);
                if (itBlock!=nullptr)
                {
                    VPoint size;
                    itBlock->getButtonSizeOriginal(size);
                    if (cmd.boolParams[0])
                        size.y=cmd.intParams[1];
                    else
                        size.x=cmd.intParams[1];
                    itBlock->setButtonSizeOriginal(size);
                }
            }
        }
        if (cmd.cmdId==SET_UIPOSITION_OPENGLUIBLOCKGUITRIGGEREDCMD)
        {
            if (App::getEditModeType()==BUTTON_EDIT_MODE)
            {
                CButtonBlock* itBlock=App::ct->buttonBlockContainer->getBlockWithID(cmd.intParams[0]);
                if (itBlock!=nullptr)
                {
                    VPoint blockPos;
                    itBlock->getDesiredBlockPosition(blockPos);
                    if (cmd.boolParams[0])
                        blockPos.y=cmd.intParams[1];
                    else
                        blockPos.x=cmd.intParams[1];
                    itBlock->setDesiredBlockPosition(blockPos.x,blockPos.y);
                }
            }
        }
        if (cmd.cmdId==SET_ATTRIBUTES_OPENGLUIBLOCKGUITRIGGEREDCMD)
        {
            if (App::getEditModeType()==BUTTON_EDIT_MODE)
            {
                CButtonBlock* itBlock=App::ct->buttonBlockContainer->getBlockWithID(cmd.intParams[0]);
                if (itBlock!=nullptr)
                    itBlock->setAttributes(cmd.intParams[1]);
            }
        }
        if (cmd.cmdId==SET_DISPLAYPAGE_OPENGLUIBLOCKGUITRIGGEREDCMD)
        {
            if (App::getEditModeType()==BUTTON_EDIT_MODE)
            {
                CButtonBlock* itBlock=App::ct->buttonBlockContainer->getBlockWithID(cmd.intParams[0]);
                if (itBlock!=nullptr)
                    itBlock->setViewToAppearOn(cmd.intParams[1]);
            }
        }
        if (cmd.cmdId==SET_ASSOCIATEDWITHOBJECT_OPENGLUIBLOCKGUITRIGGEREDCMD)
        {
            if (App::getEditModeType()==BUTTON_EDIT_MODE)
            {
                CButtonBlock* itBlock=App::ct->buttonBlockContainer->getBlockWithID(cmd.intParams[0]);
                if (itBlock!=nullptr)
                    itBlock->setObjectIDAttachedTo(cmd.intParams[1]);
            }
        }
        if (cmd.cmdId==RENAME_UI_OPENGLUIBLOCKGUITRIGGEREDCMD)
        {
            if (App::getEditModeType()==BUTTON_EDIT_MODE)
            {
                CButtonBlock* itBlock=App::ct->buttonBlockContainer->getBlockWithID(cmd.intParams[0]);
                if (itBlock!=nullptr)
                {
                    std::string editionText(cmd.stringParams[0]);
                    if (App::ct->buttonBlockContainer->getBlockWithName(editionText)==nullptr)
                        itBlock->setBlockName(editionText);
                }
            }
        }








        if (cmd.cmdId==INSERT_BUTTONS_OPENGLUIBUTTONGUITRIGGEREDCMD)
        {
            if (App::getEditModeType()==BUTTON_EDIT_MODE)
            {
                CButtonBlock* itBlock=App::ct->buttonBlockContainer->getBlockWithID(cmd.intParams[0]);
                if ( (itBlock!=nullptr)&&(cmd.intParams.size()>1) )
                {
                    VPoint size;
                    itBlock->getBlockSize(size);

                    // We first check how many buttons we have to insert:
                    int btnCnt=0;
                    for (size_t i=1;i<cmd.intParams.size();i++)
                    {
                        int val=cmd.intParams[i];
                        CSoftButton* itButton=itBlock->getButtonAtPos(val%size.x,val/size.x);
                        if (itButton==nullptr)
                            btnCnt++;
                    }
                    // if we insert just one button, its ID will be the smallest available ID, otherwise
                    // the buttons IDs will start from biggestButtonID+1:
                    int freeID=itBlock->getFreeButtonHandle();
                    if (btnCnt>1)
                    { // We find the biggest button handle and increment it by one:
                        freeID=-1;
                        for (size_t i=0;i<itBlock->allButtons.size();i++)
                        {
                            if (itBlock->allButtons[i]->buttonID>freeID)
                                freeID=itBlock->allButtons[i]->buttonID;
                        }
                        freeID++;
                    }
                    for (size_t i=1;i<cmd.intParams.size();i++)
                    {
                        int val=cmd.intParams[i];
                        CSoftButton* itButton=itBlock->getButtonAtPos(val%size.x,val/size.x);
                        if (itButton==nullptr)
                        {
                            CSoftButton* newButton=new CSoftButton("",val%size.x,val/size.x,1,1);
                            newButton->buttonID=freeID;
                            freeID++;
                            if (!itBlock->insertButtonWithoutChecking(newButton))
                                delete newButton;
                        }
                    }
                }
            }
        }
        if (cmd.cmdId==INSERT_MERGEDBUTTON_OPENGLUIBUTTONGUITRIGGEREDCMD)
        {
            if (App::getEditModeType()==BUTTON_EDIT_MODE)
            {
                CButtonBlock* itBlock=App::ct->buttonBlockContainer->getBlockWithID(cmd.intParams[0]);
                if ( (itBlock!=nullptr)&&(cmd.intParams.size()>2) )
                {
                    VPoint size;
                    itBlock->getBlockSize(size);
                    int smallestCo=600000000;
                    int biggestCo=-1;
                    for (size_t i=1;i<cmd.intParams.size();i++)
                    {
                        int butt=cmd.intParams[i];
                        if (butt<smallestCo)
                            smallestCo=butt;
                        if (butt>biggestCo)
                            biggestCo=butt;
                    }
                    VPoint smallest(smallestCo%size.x,smallestCo/size.x);
                    VPoint biggest(biggestCo%size.x,biggestCo/size.x);
                    // Now we check if all selected buttons are inside the selection square:
                    bool isOk=true;
                    for (size_t i=1;i<cmd.intParams.size();i++)
                    {
                        int butt=cmd.intParams[i];
                        int x=butt%size.x;
                        int y=butt/size.x;
                        if (x<smallest.x)
                            isOk=false;
                        if (x>biggest.x)
                            isOk=false;
                        if (y<smallest.y)
                            isOk=false;
                        if (y>biggest.y)
                            isOk=false;
                    }
                    // Now we check that the number of selected buttons is equal to the number of cells
                    // inside the selection square:
                    VPoint cellSize(biggest.x-smallest.x+1,biggest.y-smallest.y+1);
                    if (isOk)
                    {
                        if (cellSize.x*cellSize.y!=int(cmd.intParams.size()-1))
                            isOk=false;
                    }
                    // Last verification: we check that all selected cells are free:
                    if (isOk)
                    {
                        for (size_t i=1;i<cmd.intParams.size();i++)
                        {
                            int butt=cmd.intParams[i];
                            if (itBlock->getButtonAtPos(butt%size.x,butt/size.x)!=nullptr)
                                isOk=false;
                        }
                    }
                    // Now we can create the merged button:
                    if (isOk)
                    {
                        CSoftButton* itButton=new CSoftButton("",smallest.x,smallest.y,cellSize.x,cellSize.y);
                        if (itBlock->insertButton(itButton))
                        {
                            App::ct->buttonBlockContainer->deselectButtons();
                            App::ct->buttonBlockContainer->addToSelection(smallest.x+smallest.y*size.x);
                        }
                        else
                        {
                            App::ct->buttonBlockContainer->deselectButtons();
                            delete itButton;
                        }
                    }
                }
            }
        }
        if (cmd.cmdId==SET_BUTTONHANDLE_OPENGLUIBUTTONGUITRIGGEREDCMD)
        {
            if (App::getEditModeType()==BUTTON_EDIT_MODE)
            {
                CButtonBlock* itBlock=App::ct->buttonBlockContainer->getBlockWithID(cmd.intParams[0]);
                if (itBlock!=nullptr)
                {
                    CSoftButton* itButton=itBlock->getButtonWithID(cmd.intParams[1]);
                    if (itButton!=nullptr)
                    {
                        if ((cmd.intParams[2]>=0)&&(!itBlock->doesButtonIDExist(cmd.intParams[2])))
                            itButton->buttonID=cmd.intParams[2];
                    }
                }
            }
        }
        if (cmd.cmdId==SET_ATTRIBUTES_OPENGLUIBUTTONGUITRIGGEREDCMD)
        {
            if (App::getEditModeType()==BUTTON_EDIT_MODE)
            {
                CButtonBlock* itBlock=App::ct->buttonBlockContainer->getBlockWithID(cmd.intParams[0]);
                if (itBlock!=nullptr)
                {
                    CSoftButton* itButton=itBlock->getButtonWithID(cmd.intParams[1]);
                    if (itButton!=nullptr)
                        itButton->setAttributes(cmd.intParams[2]);
                }
            }
        }
        if (cmd.cmdId==SET_LABEL_OPENGLUIBUTTONGUITRIGGEREDCMD)
        {
            if (App::getEditModeType()==BUTTON_EDIT_MODE)
            {
                CButtonBlock* itBlock=App::ct->buttonBlockContainer->getBlockWithID(cmd.intParams[0]);
                if (itBlock!=nullptr)
                {
                    CSoftButton* itButton=itBlock->getButtonWithID(cmd.intParams[1]);
                    if (itButton!=nullptr)
                    {
                        if (cmd.boolParams[0])
                            itButton->label=cmd.stringParams[0];
                        else
                            itButton->downLabel=cmd.stringParams[0];
                    }
                }
            }
        }
        if (cmd.cmdId==APPLY_BUTTONTYPE_OPENGLUIBUTTONGUITRIGGEREDCMD)
        {
            if (App::getEditModeType()==BUTTON_EDIT_MODE)
            {
                CButtonBlock* itBlock=App::ct->buttonBlockContainer->getBlockWithID(cmd.intParams[0]);
                if (itBlock!=nullptr)
                {
                    CSoftButton* itButton=itBlock->getButtonWithID(cmd.intParams[1]);
                    if (itButton!=nullptr)
                    {
                        for (size_t i=2;i<cmd.intParams.size();i++)
                        {
                            CSoftButton* itButton2=itBlock->getButtonWithID(cmd.intParams[i]);
                            if (itButton2!=nullptr)
                            {
                                int atrToCopy=0x0007|sim_buttonproperty_staydown|sim_buttonproperty_enabled|
                                    sim_buttonproperty_horizontallycentered|sim_buttonproperty_ignoremouse|
                                    sim_buttonproperty_borderless|sim_buttonproperty_verticallycentered|sim_buttonproperty_downupevent;
                                int oldAtr=itButton2->getAttributes()|atrToCopy;
                                oldAtr-=atrToCopy;
                                oldAtr|=itButton->getAttributes()&atrToCopy;
                                itButton2->setAttributes(oldAtr);
                            }
                        }
                    }
                }
            }
        }
        if (cmd.cmdId==APPLY_BUTTONLABELS_OPENGLUIBUTTONGUITRIGGEREDCMD)
        {
            if (App::getEditModeType()==BUTTON_EDIT_MODE)
            {
                CButtonBlock* itBlock=App::ct->buttonBlockContainer->getBlockWithID(cmd.intParams[0]);
                if (itBlock!=nullptr)
                {
                    CSoftButton* itButton=itBlock->getButtonWithID(cmd.intParams[1]);
                    if (itButton!=nullptr)
                    {
                        for (size_t i=2;i<cmd.intParams.size();i++)
                        {
                            CSoftButton* itButton2=itBlock->getButtonWithID(cmd.intParams[i]);
                            if (itButton2!=nullptr)
                            {
                                itButton2->label=itButton->label;
                                itButton2->downLabel=itButton->downLabel;
                            }
                        }
                    }
                }
            }
        }
        if (cmd.cmdId==APPLY_BUTTONCOLORS_OPENGLUIBUTTONGUITRIGGEREDCMD)
        {
            if (App::getEditModeType()==BUTTON_EDIT_MODE)
            {
                CButtonBlock* itBlock=App::ct->buttonBlockContainer->getBlockWithID(cmd.intParams[0]);
                if (itBlock!=nullptr)
                {
                    CSoftButton* itButton=itBlock->getButtonWithID(cmd.intParams[1]);
                    if (itButton!=nullptr)
                    {
                        for (size_t i=2;i<cmd.intParams.size();i++)
                        {
                            CSoftButton* itButton2=itBlock->getButtonWithID(cmd.intParams[i]);
                            if (itButton2!=nullptr)
                            {
                                for (int j=0;j<3;j++)
                                {
                                    itButton2->backgroundColor[j]=itButton->backgroundColor[j];
                                    itButton2->downBackgroundColor[j]=itButton->downBackgroundColor[j];
                                    itButton2->textColor[j]=itButton->textColor[j];
                                }
                            }
                        }
                    }
                }
            }
        }
        if (cmd.cmdId==APPLY_BUTTONOTHERPROP_OPENGLUIBUTTONGUITRIGGEREDCMD)
        {
            if (App::getEditModeType()==BUTTON_EDIT_MODE)
            {
                CButtonBlock* itBlock=App::ct->buttonBlockContainer->getBlockWithID(cmd.intParams[0]);
                if (itBlock!=nullptr)
                {
                    CSoftButton* itButton=itBlock->getButtonWithID(cmd.intParams[1]);
                    if (itButton!=nullptr)
                    {
                        for (size_t i=2;i<cmd.intParams.size();i++)
                        {
                            CSoftButton* itButton2=itBlock->getButtonWithID(cmd.intParams[i]);
                            if (itButton2!=nullptr)
                            {
                                int atrToCopy=sim_buttonproperty_transparent|sim_buttonproperty_nobackgroundcolor;
                                int oldAtr=itButton2->getAttributes()|atrToCopy;
                                oldAtr-=atrToCopy;
                                oldAtr|=itButton->getAttributes()&atrToCopy;
                                itButton2->setAttributes(oldAtr);

                                if (itButton2->getTextureProperty()!=nullptr)
                                { // We anyway remove the texture first
                                    App::ct->textureCont->announceGeneralObjectWillBeErased(itBlock->getBlockID(),itButton2->getUniqueID());
                                    delete itButton2->getTextureProperty();
                                    itButton2->setTextureProperty(nullptr);
                                }
                                if (itButton->getTextureProperty()!=nullptr)
                                {
                                    int textObjID=itButton->getTextureProperty()->getTextureObjectID();
                                    if ((textObjID>=SIM_IDSTART_TEXTURE)&&(textObjID<=SIM_IDEND_TEXTURE))
                                    {
                                        CTextureObject* to=App::ct->textureCont->getObject(textObjID);
                                        to->addDependentObject(itBlock->getBlockID(),itButton2->getUniqueID());
                                    }
                                    itButton2->setTextureProperty(itButton->getTextureProperty()->copyYourself());
                                }
                            }
                        }
                    }
                }
            }
        }

        if (cmd.cmdId==SAVE_USERSETTINGSGUITRIGGEREDCMD)
            App::userSettings->saveUserSettings();
        if (cmd.cmdId==SET_TRANSLATIONSTEPSIZE_USERSETTINGSGUITRIGGEREDCMD)
        {
            App::userSettings->setTranslationStepSize(cmd.floatParams[0]);
            App::userSettings->saveUserSettings();
        }
        if (cmd.cmdId==SET_ROTATIONSTEPSIZE_USERSETTINGSGUITRIGGEREDCMD)
        {
            App::userSettings->setRotationStepSize(cmd.floatParams[0]);
            App::userSettings->saveUserSettings();
        }
        if (cmd.cmdId==TOGGLE_REMOVEIDENTICALVERTICES_USERSETTINGSGUITRIGGEREDCMD)
        {
            App::userSettings->identicalVerticesCheck=!App::userSettings->identicalVerticesCheck;
            App::userSettings->saveUserSettings();
        }
        if (cmd.cmdId==SET_IDENTICALVERTICESTOLERANCE_USERSETTINGSGUITRIGGEREDCMD)
        {
            App::userSettings->identicalVerticesTolerance=cmd.floatParams[0];
            App::userSettings->saveUserSettings();
        }
        if (cmd.cmdId==TOGGLE_REMOVEIDENTICALTRIANGLES_USERSETTINGSGUITRIGGEREDCMD)
        {
            App::userSettings->identicalTrianglesCheck=!App::userSettings->identicalTrianglesCheck;
            App::userSettings->saveUserSettings();
        }
        if (cmd.cmdId==TOGGLE_IGNORETRIANGLEWINDING_USERSETTINGSGUITRIGGEREDCMD)
        {
            App::userSettings->identicalTrianglesWindingCheck=!App::userSettings->identicalTrianglesWindingCheck;
            App::userSettings->saveUserSettings();
        }
        if (cmd.cmdId==TOGGLE_HIDEHIERARCHY_USERSETTINGSGUITRIGGEREDCMD)
        {
            App::userSettings->sceneHierarchyHiddenDuringSimulation=!App::userSettings->sceneHierarchyHiddenDuringSimulation;
            App::userSettings->saveUserSettings();
        }
        if (cmd.cmdId==TOGGLE_SHOWWORLDREF_USERSETTINGSGUITRIGGEREDCMD)
        {
            App::userSettings->displayWorldReference=!App::userSettings->displayWorldReference;
            App::userSettings->saveUserSettings();
        }
        if (cmd.cmdId==TOGGLE_SHOWBOUNDINGBOX_USERSETTINGSGUITRIGGEREDCMD)
        {
            App::userSettings->displayBoundingBoxeWhenObjectSelected=!App::userSettings->displayBoundingBoxeWhenObjectSelected;
            App::userSettings->saveUserSettings();
        }
        if (cmd.cmdId==TOGGLE_UNDOREDO_USERSETTINGSGUITRIGGEREDCMD)
        {
            App::userSettings->setUndoRedoEnabled(!App::userSettings->getUndoRedoEnabled());
            App::userSettings->saveUserSettings();
        }
        if (cmd.cmdId==TOGGLE_HIDECONSOLE_USERSETTINGSGUITRIGGEREDCMD)
        {
            App::userSettings->alwaysShowConsole=!App::userSettings->alwaysShowConsole;
            App::setShowConsole(App::userSettings->alwaysShowConsole);
            App::userSettings->saveUserSettings();
        }
        if (cmd.cmdId==TOGGLE_AUTOSAVE_USERSETTINGSGUITRIGGEREDCMD)
        {
            if (App::userSettings->autoSaveDelay!=0)
                App::userSettings->autoSaveDelay=0;
            else
                App::userSettings->autoSaveDelay=2; // 2 minutes
            App::userSettings->saveUserSettings();
        }
        if (cmd.cmdId==SET_OPENGLSETTINGS_USERSETTINGSGUITRIGGEREDCMD)
        {
            App::userSettings->offscreenContextType=cmd.intParams[0];
            App::userSettings->fboType=cmd.intParams[1];
            App::userSettings->vboOperation=cmd.intParams[2];
            App::userSettings->visionSensorsUseGuiThread_windowed=cmd.intParams[3];
            App::userSettings->oglCompatibilityTweak1=cmd.boolParams[0];
            App::userSettings->useGlFinish=cmd.boolParams[1];
            App::userSettings->useGlFinish_visionSensors=cmd.boolParams[2];
            App::userSettings->setIdleFps(cmd.intParams[4]);
            App::userSettings->forceFboViaExt=cmd.boolParams[3];
            App::userSettings->desiredOpenGlMajor=cmd.intParams[5];
            App::userSettings->desiredOpenGlMinor=cmd.intParams[6];
            App::userSettings->saveUserSettings();
        }

        if ( (cmd.cmdId>=BR_COMMAND_1_SCCMD)&&(cmd.cmdId<BR_COMMANDS_END_SCCMD) )
        {
            int executeBrCallIndex=-1; // no call
            if (cmd.cmdId<BR_COMMAND_1_SCCMD+296)
                executeBrCallIndex=cmd.cmdId-BR_COMMAND_1_SCCMD;
            else if (cmd.cmdId==BR_COMMAND_1_SCCMD+297)
            { // create new job
                std::string nn(App::ct->environment->getCurrentJob());
                while (true)
                {
                    nn=tt::generateNewName_noDash(nn);
                    if (App::ct->environment->getJobIndex(nn)==-1)
                        break;
                }
                SUIThreadCommand cmdIn;
                SUIThreadCommand cmdOut;
                cmdIn.cmdId=JOB_NAME_UITHREADCMD;
                cmdIn.stringParams.push_back(nn);
                App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
                if ( (cmdOut.stringParams.size()>0)&&(cmdOut.stringParams[0].size()>0) )
                {
                    nn=cmdOut.stringParams[0];
                    tt::removeIllegalCharacters(nn,false);
                    while (App::ct->environment->getJobIndex(nn)!=-1)
                        nn=tt::generateNewName_noDash(nn);
                    if (App::ct->environment->createNewJob(nn))
                        executeBrCallIndex=cmd.cmdId-BR_COMMAND_1_SCCMD;
                }
            }
            else if (cmd.cmdId==BR_COMMAND_1_SCCMD+298)
            { // delete current job
                if (App::ct->environment->deleteCurrentJob())
                    executeBrCallIndex=cmd.cmdId-BR_COMMAND_1_SCCMD;
            }
            else if (cmd.cmdId==BR_COMMAND_1_SCCMD+299)
            { // rename current job
                std::string nn(App::ct->environment->getCurrentJob());
                SUIThreadCommand cmdIn;
                SUIThreadCommand cmdOut;
                cmdIn.cmdId=JOB_NAME_UITHREADCMD;
                cmdIn.stringParams.push_back(nn);
                App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
                if ( (cmdOut.stringParams.size()>0)&&(cmdOut.stringParams[0].size()>0) )
                {
                    nn=cmdOut.stringParams[0];
                    tt::removeIllegalCharacters(nn,false);
                    if (nn.compare(App::ct->environment->getCurrentJob())!=0)
                    {
                        while (App::ct->environment->getJobIndex(nn)!=-1)
                            nn=tt::generateNewName_noDash(nn);
                        if (App::ct->environment->renameCurrentJob(nn))
                            executeBrCallIndex=cmd.cmdId-BR_COMMAND_1_SCCMD;
                    }
                }
            }
            else if (cmd.cmdId>=BR_COMMAND_1_SCCMD+300)
            { // switch to job
                if (App::ct->environment->switchJob(cmd.cmdId-(BR_COMMAND_1_SCCMD+300)))
                    executeBrCallIndex=cmd.cmdId-BR_COMMAND_1_SCCMD;
            }

            if (executeBrCallIndex>=0)
            {
                CInterfaceStack stack;
                stack.pushTableOntoStack();
                stack.pushStringOntoStack("brCallIndex",0);
                stack.pushNumberOntoStack(int(executeBrCallIndex));
                stack.insertDataIntoStackTable();
                App::ct->luaScriptContainer->handleCascadedScriptExecution(sim_scripttype_customizationscript,sim_syscb_br,&stack,nullptr,nullptr);
                App::ct->addOnScriptContainer->handleAddOnScriptExecution(sim_syscb_br,&stack,nullptr);
                if (App::ct->sandboxScript!=nullptr)
                    App::ct->sandboxScript->runSandboxScript(sim_syscb_br,&stack,nullptr);
            }
        }

        if (cmd.cmdId==PATHEDIT_MAKEDUMMY_GUITRIGGEREDCMD)
        {
            CDummy* newDummy=new CDummy();
            newDummy->setObjectName_objectNotYetInScene(cmd.stringParams[0]);
            newDummy->setObjectAltName_objectNotYetInScene(tt::getObjectAltNameFromObjectName(newDummy->getObjectName()));
            newDummy->setSize(cmd.floatParams[0]);
            App::ct->objCont->addObjectToScene(newDummy,false,true);
            newDummy->setLocalTransformation(cmd.transfParams[0]);
        }
        if (cmd.cmdId==SHAPEEDIT_MAKESHAPE_GUITRIGGEREDCMD)
        {
            App::addStatusbarMessage(IDSNS_GENERATING_SHAPE);
            int toid=cmd.intParams[0];
            CGeomProxy* newGeom;
            if (toid!=-1)
                newGeom=new CGeomProxy(nullptr,cmd.floatVectorParams[0],cmd.intVectorParams[0],nullptr,&cmd.floatVectorParams[2]);
            else
                newGeom=new CGeomProxy(nullptr,cmd.floatVectorParams[0],cmd.intVectorParams[0],nullptr,nullptr);
            CShape* newShape=new CShape();
            newShape->setLocalTransformation(newGeom->getCreationTransformation());
            newGeom->setCreationTransformation(C7Vector::identityTransformation);
            newShape->setVisibleEdges(true);
            ((CGeometric*)newGeom->geomInfo)->setGouraudShadingAngle(20.0f*degToRad_f);
            ((CGeometric*)newGeom->geomInfo)->setEdgeThresholdAngle(20.0f*degToRad_f);
            newShape->setObjectName_objectNotYetInScene("Extracted_shape");
            newShape->setObjectAltName_objectNotYetInScene(tt::getObjectAltNameFromObjectName(newShape->getObjectName()));
            //          newShape->layer=it->layer;
            newShape->geomData=newGeom;
            App::ct->objCont->addObjectToScene(newShape,false,true);
            if (toid!=-1)
            {
                CTextureObject* to=App::ct->textureCont->getObject(toid);
                if (to!=nullptr)
                {
                    to->addDependentObject(newShape->getObjectHandle(),((CGeometric*)newShape->geomData->geomInfo)->getUniqueID());
                    CTextureProperty* tp=new CTextureProperty(to->getObjectID());
                    ((CGeometric*)newShape->geomData->geomInfo)->setTextureProperty(tp);
                    tp->setFixedCoordinates(&((CGeometric*)newShape->geomData->geomInfo)->textureCoords_notCopiedNorSerialized);
                    ((CGeometric*)newShape->geomData->geomInfo)->textureCoords_notCopiedNorSerialized.clear();
                }
            }
            App::addStatusbarMessage(IDSNS_DONE);
            App::setFullDialogRefreshFlag();
        }
        if (cmd.cmdId==SHAPEEDIT_MAKEPRIMITIVE_GUITRIGGEREDCMD)
        {
            CGeomProxy* newGeom=new CGeomProxy(nullptr,cmd.floatVectorParams[0],cmd.intVectorParams[0],nullptr,nullptr);
            CShape* newShape=new CShape();
            newShape->setLocalTransformation(newGeom->getCreationTransformation());
            newGeom->setCreationTransformation(C7Vector::identityTransformation);
            newShape->geomData=newGeom;
            C3Vector size(newGeom->getBoundingBoxHalfSizes()*2.0f);
            C7Vector conf(newShape->getLocalTransformationPart1());
            delete newShape;
            CShape* shape=nullptr;

            if (cmd.intParams[0]==0)
            { // Cuboid
                App::addStatusbarMessage("Generating cuboid...");
                shape=CAddOperations::addPrimitive_withDialog(ADD_COMMANDS_ADD_PRIMITIVE_RECTANGLE_ACCMD,&size);
                if (shape!=nullptr)
                    shape->setLocalTransformation(conf);
            }

            if (cmd.intParams[0]==1)
            { // sphere
                App::addStatusbarMessage("Generating sphere...");
                float mm=SIM_MAX(SIM_MAX(size(0),size(1)),size(2));
                size(0)=mm;
                size(1)=mm;
                size(2)=mm;
                shape=CAddOperations::addPrimitive_withDialog(ADD_COMMANDS_ADD_PRIMITIVE_SPHERE_ACCMD,&size);
                if (shape!=nullptr)
                    shape->setLocalTransformation(conf);
            }

            if (cmd.intParams[0]==2)
            { // spheroid
                App::addStatusbarMessage("Generating spheroid...");
                shape=CAddOperations::addPrimitive_withDialog(ADD_COMMANDS_ADD_PRIMITIVE_SPHERE_ACCMD,&size);
                if (shape!=nullptr)
                    shape->setLocalTransformation(conf);
            }

            if (cmd.intParams[0]==3)
            { // cylinder
                App::addStatusbarMessage("Generating cylinder...");
                C3Vector diff(fabs(size(0)-size(1)),fabs(size(0)-size(2)),fabs(size(1)-size(2)));
                int t=2;
                if (SIM_MIN(SIM_MIN(diff(0),diff(1)),diff(2))==diff(0))
                    t=0;
                if (SIM_MIN(SIM_MIN(diff(0),diff(1)),diff(2))==diff(1))
                    t=1;
                if (t==0)
                {
                    C3Vector s(size);
                    size(0)=(s(0)+s(1))*0.5f;
                    size(1)=(s(0)+s(1))*0.5f;
                    size(2)=s(2);
                }
                if (t==1)
                {
                    C3Vector s(size);
                    size(0)=(s(0)+s(2))*0.5f;
                    size(1)=(s(0)+s(2))*0.5f;
                    size(2)=s(1);
                }
                if (t==2)
                {
                    C3Vector s(size);
                    size(0)=(s(2)+s(1))*0.5f;
                    size(1)=(s(2)+s(1))*0.5f;
                    size(2)=s(0);
                }
                shape=CAddOperations::addPrimitive_withDialog(ADD_COMMANDS_ADD_PRIMITIVE_CYLINDER_ACCMD,&size);
                if (shape!=nullptr)
                {
                    C7Vector r;
                    r.setIdentity();
                    if (t==1)
                        r.Q.setEulerAngles(C3Vector(piValD2_f,0.0f,0.0f));
                    if (t==2)
                        r.Q.setEulerAngles(C3Vector(0.0f,piValD2_f,0.0f));
                    shape->setLocalTransformation(conf*r);
                }
            }
            if (shape!=nullptr)
                App::addStatusbarMessage(IDSNS_DONE);
            else
                App::addStatusbarMessage(IDSNS_OPERATION_ABORTED);
            App::setFullDialogRefreshFlag();
        }

        if (cmd.cmdId==ADD_OBJECTTOSCENE_GUITRIGGEREDCMD)
        {
            if (cmd.intParams[0]==sim_object_path_type)
            {
                CPath* it=(CPath*)cmd.objectParams[0];
                // Use a copy (the original was created in the UI thread):
                App::ct->objCont->addObjectToScene(it->copyYourself(),false,true);
                // Have the original destroyed by the UI thread:
                SUIThreadCommand cmdIn;
                SUIThreadCommand cmdOut;
                cmdIn.cmdId=DESTROY_UIOBJECT_UITHREADCMD;
                cmdIn.intParams.push_back(cmd.intParams[0]);
                cmdIn.objectParams.push_back(cmd.objectParams[0]);
                App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
            }
        }
        if (cmd.cmdId==SWITCH_TOINSTANCEINDEX_GUITRIGGEREDCMD)
        {
            App::ct->setInstanceIndex(cmd.intParams[0]);
        }
        if (cmd.cmdId==SET_ACTIVEPAGE_GUITRIGGEREDCMD)
        {
            App::ct->pageContainer->setActivePage(cmd.intParams[0]);
        }
        if (cmd.cmdId==SET_MOUSEMODE_GUITRIGGEREDCMD)
        {
            App::setMouseMode(cmd.intParams[0]);
        }
        if (cmd.cmdId==SELECT_VIEW_GUITRIGGEREDCMD)
        {
            C3DObject* it=App::ct->objCont->getObjectFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
            {
                CSPage* view=App::ct->pageContainer->getPage(cmd.intParams[1]);
                if (view!=nullptr)
                {
                    CSView* subView=view->getView(cmd.intParams[2]);
                    if (subView!=nullptr)
                    {
                        subView->setLinkedObjectID(it->getObjectHandle(),false);
                        subView->setTimeGraph(cmd.boolParams[0]);
                    }
                }
            }
        }





        if (cmd.cmdId==SET_CURRENTDIRECTORY_GUITRIGGEREDCMD)
        {
            if (cmd.intParams[0]==DIRECTORY_ID_EXECUTABLE)
                App::directories->executableDirectory=cmd.stringParams[0];
            if (cmd.intParams[0]==DIRECTORY_ID_SYSTEM)
                App::directories->systemDirectory=cmd.stringParams[0];
            if (cmd.intParams[0]==DIRECTORY_ID_SCENE)
                App::directories->sceneDirectory=cmd.stringParams[0];
            if (cmd.intParams[0]==DIRECTORY_ID_MODEL)
                App::directories->modelDirectory=cmd.stringParams[0];
            if (cmd.intParams[0]==DIRECTORY_ID_OPENGLUI)
                App::directories->uiDirectory=cmd.stringParams[0];
            if (cmd.intParams[0]==DIRECTORY_ID_CADFORMAT)
                App::directories->cadFormatDirectory=cmd.stringParams[0];
            if (cmd.intParams[0]==DIRECTORY_ID_TEXTURE)
                App::directories->textureDirectory=cmd.stringParams[0];
            if (cmd.intParams[0]==DIRECTORY_ID_OTHER)
                App::directories->otherFilesDirectory=cmd.stringParams[0];
            if (cmd.intParams[0]==DIRECTORY_ID_REMOTEAPIFILETRANSFER)
                App::directories->remoteApiFileTransferDirectory=cmd.stringParams[0];
        }
        if (cmd.cmdId==SHOW_PROGRESSDLGGUITRIGGEREDCMD)
        {
            App::uiThread->showOrHideProgressBar(true,cmd.intParams[0],cmd.stringParams[0].c_str());
        }
        if (cmd.cmdId==HIDE_PROGRESSDLGGUITRIGGEREDCMD)
        {
            App::uiThread->showOrHideProgressBar(false);
        }
        if (cmd.cmdId==SET_THUMBNAIL_GUITRIGGEREDCMD)
        {
            App::ct->environment->modelThumbnail_notSerializedHere.setUncompressedThumbnailImage((char*)&cmd.uint8Params[0],true,false);
        }
        if (cmd.cmdId==SET_OBJECT_SELECTION_GUITRIGGEREDCMD)
        {
            App::ct->objCont->deselectObjects();
            for (size_t i=0;i<cmd.intParams.size();i++)
                App::ct->objCont->addObjectToSelection(cmd.intParams[i]);
        }
        if (cmd.cmdId==CLEAR_OBJECT_SELECTION_GUITRIGGEREDCMD)
        {
            App::ct->objCont->deselectObjects();
        }
        if (cmd.cmdId==ADD_OBJECTS_TO_SELECTION_GUITRIGGEREDCMD)
        {
            for (size_t i=0;i<cmd.intParams.size();i++)
                App::ct->objCont->addObjectToSelection(cmd.intParams[i]);
        }
        if (cmd.cmdId==INVERT_SELECTION_GUITRIGGEREDCMD)
        {
            for (size_t i=0;i<App::ct->objCont->objectList.size();i++)
                App::ct->objCont->xorAddObjectToSelection(App::ct->objCont->objectList[i]);
        }
        if (cmd.cmdId==POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD)
        {
            App::ct->undoBufferContainer->announceChange();
        }
        if (cmd.cmdId==FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD)
        {   // delay later this call until the resources have been actualized on the UI SIDE!!
            App::setFullDialogRefreshFlag();
            SUIThreadCommand cmdIn;
            SUIThreadCommand cmdOut;
            cmdIn.cmdId=REFRESH_DIALOGS_UITHREADCMD;
            {
                // Following instruction very important in the function below tries to lock resources (or a plugin it calls!):
                SIM_THREAD_INDICATE_UI_THREAD_CAN_DO_ANYTHING;
                App::executeUiThreadCommand(&cmdIn,&cmdOut);
            }
        }
        if (cmd.cmdId==CALL_DIALOG_FUNCTION_GUITRIGGEREDCMD)
        {   // delay later this call until the resources have been actualized on the UI SIDE!!
            SUIThreadCommand cmdIn;
            SUIThreadCommand cmdOut;
            cmdIn.cmdId=CALL_DIALOG_FUNCTION_UITHREADCMD;
            cmdIn.intParams.push_back(cmd.intParams[0]);
            cmdIn.intParams.push_back(cmd.intParams[1]);
            cmdIn.intParams.push_back(cmd.intParams[2]);
            {
                // Following instruction very important in the function below tries to lock resources (or a plugin it calls!):
                SIM_THREAD_INDICATE_UI_THREAD_CAN_DO_ANYTHING;
                App::executeUiThreadCommand(&cmdIn,&cmdOut);
            }
        }

        if (cmd.cmdId==FULLREFRESH_HIERARCHY_GUITRIGGEREDCMD)
        {   // delay later this call until the resources have been actualized on the UI SIDE!!
            App::setRefreshHierarchyViewFlag();
        }
        if (cmd.cmdId==REFRESH_TOOLBARS_GUITRIGGEREDCMD)
        {   // delay later this call until the resources have been actualized on the UI SIDE!!
            App::setToolbarRefreshFlag();
        }

        // UNDO/REDO:
        if (cmd.cmdId==999995)
            App::ct->undoBufferContainer->emptyRedoBuffer();
        if (cmd.cmdId==999996)
            App::ct->undoBufferContainer->announceChangeGradual();
        if (cmd.cmdId==999997)
            App::ct->undoBufferContainer->announceChangeStart();
        if (cmd.cmdId==999998)
            App::ct->undoBufferContainer->announceChangeEnd();
        if (cmd.cmdId==999999)
            App::ct->undoBufferContainer->announceChange();

        if ( (cmd.cmdId>START_GUITRIGGEREDCMD)&&(cmd.cmdId<END_GUITRIGGEREDCMD) )
        {
        //  printf("GUI cmd: %i\n",cmd.cmdId);
        }
    }

    if (cmd.cmdId==OPEN_DRAG_AND_DROP_SCENE_CMD)
    {
        if ( (App::mainWindow!=nullptr)&&(!App::userSettings->doNotShowSceneSelectionThumbnails) )
        {
            SSimulationThreadCommand cmd2=cmd;
            cmd2.cmdId=OPEN_DRAG_AND_DROP_SCENE_PHASE2_CMD;
            if (!App::mainWindow->prepareSceneThumbnail(cmd2))
            { // several scenes might have been dropped onto the main window!
                cmd2.cmdId=OPEN_DRAG_AND_DROP_SCENE_CMD;
                App::appendSimulationThreadCommand(cmd2); // re-post the original msg!
            }
        }
        else
        {
            SSimulationThreadCommand cmd2=cmd;
            cmd2.cmdId=OPEN_DRAG_AND_DROP_SCENE_PHASE2_CMD;
            App::appendSimulationThreadCommand(cmd2);
        }
    }

    if (cmd.cmdId==OPEN_DRAG_AND_DROP_SCENE_PHASE2_CMD)
    {
        CFileOperations::createNewScene(false,true);
        CFileOperations::loadScene(cmd.stringParams[0].c_str(),true,cmd.boolParams[0],false);
        App::ct->undoBufferContainer->clearSceneSaveMaybeNeededFlag();
    }

    if (cmd.cmdId==DISPLAY_ACKNOWLEDGMENT_MESSAGE_CMD)
        App::uiThread->messageBox_information(App::mainWindow,cmd.stringParams[0],cmd.stringParams[1],VMESSAGEBOX_OKELI);

    if (cmd.cmdId==AUTO_SAVE_SCENE_CMD)
        _handleAutoSaveSceneCommand(cmd);

    if (cmd.cmdId==MEMORIZE_UNDO_STATE_IF_NEEDED_CMD)
    {
        App::ct->undoBufferContainer->memorizeStateIfNeeded();
        App::appendSimulationThreadCommand(cmd,200);
    }

    if (cmd.cmdId==REFRESH_DIALOGS_CMD)
    {
        SUIThreadCommand cmdIn;
        SUIThreadCommand cmdOut;
        cmdIn.cmdId=REFRESH_DIALOGS_UITHREADCMD;
        {
            // Following instruction very important in the function below tries to lock resources (or a plugin it calls!):
            SIM_THREAD_INDICATE_UI_THREAD_CAN_DO_ANYTHING;
            App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
        }
        appendSimulationThreadCommand(cmd,300);
    }
#endif
}

#ifdef SIM_WITH_GUI
void CSimThread::_handleClickRayIntersection(SSimulationThreadCommand cmd)
{
    float nearClipp=cmd.floatParams[0];
    C7Vector transf=cmd.transfParams[0];
    bool mouseDown=cmd.boolParams[0];
    int cameraHandle=cmd.intParams[0];

    CCamera* cam=App::ct->objCont->getCamera(cameraHandle);
    if (cam!=nullptr)
    {
        std::vector<int> currentSelectionState;
        App::ct->objCont->getSelectedObjects(currentSelectionState);
        C3Vector pt,triNormal;
        int obj=-1;

        bool allObjectsAlsoNonDetectable=false;
        if (App::mainWindow!=nullptr)
        {
            if (mouseDown)
                allObjectsAlsoNonDetectable=(App::mainWindow->getProxSensorClickSelectDown()<0);
            else
                allObjectsAlsoNonDetectable=(App::mainWindow->getProxSensorClickSelectUp()<0);
        }

        int intParams[8]={0,0,0,0,0,0,0,0};
        float floatParams[15]={nearClipp,999999.9f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.01f,0.0f,0.0f};
        int psh=simCreateProximitySensor_internal(sim_proximitysensor_ray_subtype,sim_objectspecialproperty_detectable_all,0,intParams,floatParams,nullptr);
        simSetObjectPosition_internal(psh,cameraHandle,transf.X.data);
        simSetObjectOrientation_internal(psh,cameraHandle,transf.Q.getEulerAngles().data);
        int displayAttrib=sim_displayattribute_renderpass;
        if (App::ct->simulation->getDynamicContentVisualizationOnly())
            displayAttrib|=sim_displayattribute_dynamiccontentonly;
        CProxSensor* prox=App::ct->objCont->getProximitySensor(psh);
        float dist=SIM_MAX_FLOAT;
        bool ptValid=false;
        for (int i=0;i<int(App::ct->objCont->objectList.size());i++)
        {
            int objectHandle=App::ct->objCont->objectList[i];
            C3DObject* object=App::ct->objCont->getObjectFromHandle(objectHandle);
            if (object->getShouldObjectBeDisplayed(cameraHandle,displayAttrib)&&object->isPotentiallyMeasurable())
            {
                int theObj;
                bool valid=CProxSensorRoutine::detectEntity(psh,objectHandle,true,false,0.0f,pt,dist,true,true,theObj,0.0f,triNormal,allObjectsAlsoNonDetectable,false);
                ptValid|=valid;
                if (valid)
                    obj=theObj;
            }
        }
        C7Vector sensTr(prox->getCumulativeTransformation());
        pt*=sensTr;
        triNormal=sensTr.Q*triNormal;
        simRemoveObject_internal(psh);

        // Now generate a plugin callback:
        float ptdata[6]={pt(0),pt(1),pt(2),triNormal(0),triNormal(1),triNormal(2)};
        int msg;
        if (mouseDown)
            msg=sim_message_eventcallback_proxsensorselectdown;
        else
            msg=sim_message_eventcallback_proxsensorselectup;
        int data[4]={obj,0,0,0};
        void* retVal=CPluginContainer::sendEventCallbackMessageToAllPlugins(msg,data,ptdata,nullptr);
        delete[] (char*)retVal;
        // Now generate a script message:
        if (mouseDown)
            msg=sim_message_prox_sensor_select_down;
        else
            msg=sim_message_prox_sensor_select_up;
        App::ct->outsideCommandQueue->addCommand(msg,obj,0,0,0,ptdata,6);
        App::ct->objCont->setSelectedObjects(currentSelectionState);
    }
}

void CSimThread::_handleAutoSaveSceneCommand(SSimulationThreadCommand cmd)
{
    if ( handleVerSpecHandleAutoSaveSceneCommand1()&&(App::mainWindow!=nullptr)&&App::ct->simulation->isSimulationStopped()&&(App::getEditModeType()==NO_EDIT_MODE) )
    {
        if (cmd.intParams[0]==0)
        { // Here we maybe need to load auto-saved scenes:
            // First post the next command in the sequence:
            cmd.intParams[0]=1;
            App::appendSimulationThreadCommand(cmd,1000);
            CPersistentDataContainer cont(FILENAME_OF_USER_SETTINGS_IN_BINARY_FILE);
            std::string val;
            cont.readData("SIMSETTINGS_VREP_CRASHED",val);
            if (val=="Yes")
            { // ask what to do:
                if (!App::isFullScreen())
                {
                    if ( (!App::userSettings->doNotShowCrashRecoveryMessage)&&(!App::userSettings->suppressStartupDialogs) )
                    {
                        if (VMESSAGEBOX_REPLY_YES==App::uiThread->messageBox_question(App::mainWindow,IDSN_VREP_CRASH,IDSN_VREP_CRASH_OR_NEW_INSTANCE_INFO,VMESSAGEBOX_YES_NO))
                        {
                            std::string testScene=App::directories->executableDirectory+VREP_SLASH;
                            testScene.append("AUTO_SAVED_INSTANCE_1.");
                            testScene+=VREP_SCENE_EXTENSION;
                            if (CFileOperations::loadScene(testScene.c_str(),false,false,false))
                            {
                                App::ct->mainSettings->setScenePathAndName("");
                                App::addStatusbarMessage(IDSNS_SCENE_WAS_RESTORED_FROM_AUTO_SAVED_SCENE);
                            }
                            int instanceNb=2;
                            while (true)
                            {
                                testScene=App::directories->executableDirectory+VREP_SLASH;
                                testScene.append("AUTO_SAVED_INSTANCE_");
                                testScene+=tt::FNb(instanceNb);
                                testScene+=".";
                                testScene+=VREP_SCENE_EXTENSION;
                                if (VFile::doesFileExist(testScene))
                                {
                                    App::ct->createNewInstance();
                                    if (CFileOperations::loadScene(testScene.c_str(),false,false,false))
                                    {
                                        App::ct->mainSettings->setScenePathAndName("");
                                        App::addStatusbarMessage(IDSNS_SCENE_WAS_RESTORED_FROM_AUTO_SAVED_SCENE);
                                    }
                                    else
                                        break;
                                    instanceNb++;
                                }
                                else
                                    break;
                            }
                            App::ct->makeInstanceCurrentFromIndex(0,false);
                        }
                    }
                    else
                        App::addStatusbarMessage("It seems that V-REP crashed in last session (or you might be running several instances of V-REP in parallel).");
                }
            }
        }
        else if (cmd.intParams[0]==1)
        { // Set the TAG: V-REP started normally
            // First post the auto-save command:
            cmd.intParams[0]=2;
            App::appendSimulationThreadCommand(cmd,1000);
            CPersistentDataContainer cont(FILENAME_OF_USER_SETTINGS_IN_BINARY_FILE);
            cont.writeData("SIMSETTINGS_VREP_CRASHED","Yes",!App::userSettings->doNotWritePersistentData);
        }
        else if (cmd.intParams[0]==2)
        {
            // First repost a same command:
            App::appendSimulationThreadCommand(cmd,1000);
            if ( handleVerSpecHandleAutoSaveSceneCommand2()&&(App::userSettings->autoSaveDelay>0)&&(!App::ct->environment->getSceneLocked()) )
            {
                if (VDateTime::getSecondsSince1970()>(App::ct->environment->autoSaveLastSaveTimeInSecondsSince1970+App::userSettings->autoSaveDelay*60))
                {
                    std::string savedLoc=App::ct->mainSettings->getScenePathAndName();
                    std::string testScene=App::directories->executableDirectory+VREP_SLASH;
                    testScene+="AUTO_SAVED_INSTANCE_";
                    testScene+=tt::FNb(App::ct->getCurrentInstanceIndex()+1);
                    testScene+=".";
                    testScene+=VREP_SCENE_EXTENSION;
                    CFileOperations::saveScene(testScene.c_str(),false,false,false,false);
                    //std::string info=IDSNS_AUTO_SAVED_SCENE;
                    //info+=" ("+testScene+")";
                    //App::addStatusbarMessage(info.c_str());
                    App::ct->mainSettings->setScenePathAndName(savedLoc.c_str());
                    App::ct->environment->autoSaveLastSaveTimeInSecondsSince1970=VDateTime::getSecondsSince1970();
                }
            }
        }
    }
    else
        App::appendSimulationThreadCommand(cmd,1000); // repost the same message a bit later
}

void CSimThread::_displayVariousWaningMessagesDuringSimulation()
{
    FUNCTION_DEBUG;

    bool displayNonStandardParams=App::ct->simulation->getDisplayWarningAboutNonDefaultParameters();
    bool displayNonPureNonConvexShapeUseWarning=false;
    bool displayStaticShapeOnDynamicConstructionWarning=false;

    App::ct->dynamicsContainer->displayWarningsIfNeeded(); // Warnings when something not supported by the dynamics engine
    displayNonPureNonConvexShapeUseWarning|=App::ct->dynamicsContainer->displayNonPureNonConvexShapeWarningRequired();
    if (App::ct->dynamicsContainer->isWorldThere())
    {
        displayNonStandardParams|=App::ct->dynamicsContainer->displayNonDefaultParameterWarningRequired();
        displayStaticShapeOnDynamicConstructionWarning|=App::ct->dynamicsContainer->displayStaticShapeOnDynamicConstructionWarningRequired();
    }

    if (displayNonStandardParams)
    {
        CPersistentDataContainer cont(FILENAME_OF_USER_SETTINGS_IN_BINARY_FILE);
        std::string val;
        cont.readData("SIMSETTINGS_WARNING_NO_SHOW",val);
        int intVal=0;
        tt::getValidInt(val,intVal);
        if (intVal<3)
        {
            if (App::uiThread->messageBox_checkbox(App::mainWindow,IDSN_SIMULATION_PARAMETERS,IDSN_NON_STANDARD_SIM_PARAMS_WARNING,IDSN_DO_NOT_SHOW_THIS_MESSAGE_AGAIN_3X))
            {
                intVal++;
                val=tt::FNb(intVal);
                cont.writeData("SIMSETTINGS_WARNING_NO_SHOW",val,!App::userSettings->doNotWritePersistentData);
            }
        }
    }

    if (App::ct->dynamicsContainer->displayVortexPluginIsDemoRequired())
#ifdef WIN_VREP
        App::uiThread->messageBox_information(App::mainWindow,strTranslate(IDSN_PHYSICS_ENGINE),strTranslate(IDS_WARNING_WITH_VORTEX_DEMO_PLUGIN_WINDOWS),VMESSAGEBOX_OKELI);
#endif
#ifdef LIN_VREP
        App::uiThread->messageBox_information(App::mainWindow,strTranslate(IDSN_PHYSICS_ENGINE),strTranslate(IDS_WARNING_WITH_VORTEX_DEMO_PLUGIN_LINUX),VMESSAGEBOX_OKELI);
#endif

    if (displayNonPureNonConvexShapeUseWarning)
    {
        CPersistentDataContainer cont(FILENAME_OF_USER_SETTINGS_IN_BINARY_FILE);
        std::string val;
        cont.readData("NONPURESHAPEFORDYNAMICS_WARNING_NO_SHOW",val);
        int intVal=0;
        tt::getValidInt(val,intVal);
        if (intVal<3)
        {
            if (App::uiThread->messageBox_checkbox(App::mainWindow,IDSN_DYNAMIC_CONTENT,IDSN_USING_NON_PURE_NON_CONVEX_SHAPES_FOR_DYNAMICS_WARNING,IDSN_DO_NOT_SHOW_THIS_MESSAGE_AGAIN_3X))
            {
                intVal++;
                val=tt::FNb(intVal);
                cont.writeData("NONPURESHAPEFORDYNAMICS_WARNING_NO_SHOW",val,!App::userSettings->doNotWritePersistentData);
            }
        }
    }

    if (displayStaticShapeOnDynamicConstructionWarning)
    {
        CPersistentDataContainer cont(FILENAME_OF_USER_SETTINGS_IN_BINARY_FILE);
        std::string val;
        cont.readData("STATICSHAPEONTOPOFDYNAMICCONSTRUCTION_WARNING_NO_SHOW",val);
        int intVal=0;
        tt::getValidInt(val,intVal);
        if (intVal<3)
        {
            if (App::uiThread->messageBox_checkbox(App::mainWindow,IDSN_DYNAMIC_CONTENT,IDSN_USING_STATIC_SHAPE_ON_TOP_OF_DYNAMIC_CONSTRUCTION_WARNING,IDSN_DO_NOT_SHOW_THIS_MESSAGE_AGAIN_3X))
            {
                intVal++;
                val=tt::FNb(intVal);
                cont.writeData("STATICSHAPEONTOPOFDYNAMICCONSTRUCTION_WARNING_NO_SHOW",val,!App::userSettings->doNotWritePersistentData);
            }
        }
    }
}

int CSimThread::_prepareSceneForRenderIfNeeded()
{
    static int lastRenderingTime=0;
    static int frameCount=1000;
    bool render=false;
    if (App::ct->simulation->getSimulationState()&sim_simulation_advancing)
    {
        frameCount++;
        render=(frameCount>=App::ct->simulation->getSimulationPassesPerRendering_speedModified());
        if (render)
            frameCount=0;
    }
    else
    {
        frameCount=1000;
        render=true;
        if (App::userSettings->getIdleFps()>0)
        { // When idleFps is 0, then we don't sleep (like when simulating)
            int sleepTime=(1000/App::userSettings->getIdleFps())-(VDateTime::getTimeDiffInMs(lastRenderingTime));
            const int minSleepTime=20;//4; From 4 to 20 on 8/7/2014 (to avoid saturation)
            int effectiveSleepTime=sleepTime;
            if ((App::mainWindow!=nullptr)&&(App::mainWindow->getMouseButtonState()&13))
                effectiveSleepTime=minSleepTime;
            if (effectiveSleepTime<minSleepTime)
                effectiveSleepTime=minSleepTime;
            VThread::sleep(effectiveSleepTime);
        }
    }

    if (render)
    {
        lastRenderingTime=VDateTime::getTimeInMs();
        if (App::mainWindow!=nullptr)
        {
            App::mainWindow->simThread_prepareToRenderScene();

            if (App::ct->simulation->getThreadedRendering())
                return(1); // means: do not wait
            else
                return(2); // means: wait until rendering finished
        }
        return(0); // we do not want to render
    }
    return(0); // we do not want to render
}
#endif
