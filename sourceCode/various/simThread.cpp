#include "simThread.h"
#include "vThread.h"
#include "app.h"
#include "fileOperations.h"
#include "simStringTable.h"
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
#include "mesh.h"
#include "threadPool_old.h"
#include "volInt.h"
#include "graphingRoutines_old.h"
#include "simStringTable_openGl.h"
#include "simFlavor.h"
#ifdef SIM_WITH_GUI
    #include "toolBarCommand.h"
    #include "vMessageBox.h"
    #include "annJson.h""
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
    TRACE_INTERNAL;
#ifdef SIM_WITH_QT
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
#endif
    // Handle delayed commands:
    _handleSimulationThreadCommands();
}

void CSimThread::appendSimulationThreadCommand(SSimulationThreadCommand cmd,int executionDelay/*=0*/)
{ // CALLED FROM ANY THREAD
    cmd.sceneUniqueId=App::currentWorld->environment->getSceneUniqueID();
    cmd.postTime=(int)VDateTime::getTimeInMs();
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
            }
        }
        // Now append the delayed commands:
        _simulationThreadCommands.insert(_simulationThreadCommands.end(),delayedCommands.begin(),delayedCommands.end());
    }
}

void CSimThread::_executeSimulationThreadCommand(SSimulationThreadCommand cmd)
{
    TRACE_INTERNAL;
    if (cmd.sceneUniqueId==App::currentWorld->environment->getSceneUniqueID())
    {
        if (cmd.cmdId==SET_SHAPE_TRANSPARENCY_CMD)
        {
            CShape* shape=App::currentWorld->sceneObjects->getShapeFromHandle(cmd.intParams[0]);
            if (shape!=nullptr)
            {
                if (!shape->isCompound())
                {
                    shape->getSingleMesh()->color.setTranslucid(cmd.boolParams[0]);
                    shape->getSingleMesh()->color.setOpacity(cmd.floatParams[0]);
                    App::undoRedo_sceneChanged("");
                }
            }
        }
        if (cmd.cmdId==MAKE_OBJECT_CHILD_OF_CMD)
        {
            CSceneObject* child=App::currentWorld->sceneObjects->getObjectFromHandle(cmd.intParams[0]);
            CSceneObject* parent=App::currentWorld->sceneObjects->getObjectFromHandle(cmd.intParams[1]);
            if (child!=nullptr)
            {
                App::currentWorld->sceneObjects->setObjectParent(child,parent,true);
                App::undoRedo_sceneChanged(""); // **************** UNDO THINGY ****************
            }
        }

        // Scene object operation commands:
        if ( (cmd.cmdId>SCENE_OBJECT_OPERATION_START_SOOCMD)&&(cmd.cmdId<SCENE_OBJECT_OPERATION_END_SOOCMD) )
            CSceneObjectOperations::processCommand(cmd.cmdId);

        if ( (cmd.cmdId>=UI_MODULE_MENU_CMDS_START)&&(cmd.cmdId<=UI_MODULE_MENU_CMDS_END) )
            App::worldContainer->addOnScriptContainer->processCommand(cmd.cmdId);

        if ( (cmd.cmdId>PAGE_CONT_FUNCTIONS_START_PCCMD)&&(cmd.cmdId<PAGE_CONT_FUNCTIONS_END_PCCMD) )
            App::currentWorld->pageContainer->processCommand(cmd.cmdId,cmd.intParams[0]);

        if ( (cmd.cmdId>FILE_OPERATION_START_FOCMD)&&(cmd.cmdId<FILE_OPERATION_END_FOCMD) )
            CFileOperations::processCommand(cmd);

        if ( (cmd.cmdId>SIMULATION_COMMANDS_START_SCCMD)&&(cmd.cmdId<SIMULATION_COMMANDS_END_SCCMD) )
            App::currentWorld->simulation->processCommand(cmd.cmdId);

        if (cmd.cmdId==SET_SHAPE_SHADING_ANGLE_CMD)
        {
            CShape* shape=App::currentWorld->sceneObjects->getShapeFromHandle(cmd.intParams[0]);
            if ((shape!=nullptr)&&shape->getMeshWrapper()->isMesh())
                shape->getSingleMesh()->setShadingAngle(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_SHAPE_EDGE_ANGLE_CMD)
        {
            CShape* shape=App::currentWorld->sceneObjects->getShapeFromHandle(cmd.intParams[0]);
            if ((shape!=nullptr)&&shape->getMeshWrapper()->isMesh())
                shape->getSingleMesh()->setEdgeThresholdAngle(cmd.floatParams[0]);
        }

    }

    if (cmd.cmdId==FINAL_EXIT_REQUEST_CMD)
        App::postExitRequest();
    if (cmd.cmdId==EXIT_REQUEST_CMD)
    {
        SUIThreadCommand cmdIn;
        SUIThreadCommand cmdOut;
        cmdIn.cmdId=CHKFLTLIC_UITHREADCMD;
        App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
        int dl=0;
        int res=cmdOut.intParams[0];
        if (res>0)
        {
            if (res!=1)
            {
                while (true)
                {
                    cmdOut.intParams.clear();
                    App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
                    if (cmdOut.intParams[0]!=2)
                        break;
                }
            }
            dl=3000;
        }
        cmd.cmdId=FINAL_EXIT_REQUEST_CMD;
        appendSimulationThreadCommand(cmd,dl);
    }

    if (cmd.cmdId==EDU_EXPIRED_CMD)
    {
        App::logMsg(sim_verbosity_errors,cmd.stringParams[0].c_str());
#ifdef SIM_WITH_GUI
        App::uiThread->messageBox_informationSystemModal(App::mainWindow,"New CoppeliaSim Release",cmd.stringParams[0].c_str(),VMESSAGEBOX_OKELI,VMESSAGEBOX_REPLY_OK);
#endif
        App::postExitRequest();
    }

    if (cmd.cmdId==DISPLAY_WARNING_IF_DEBUGGING_CMD)
    {
        if ( (App::getConsoleVerbosity()>=sim_verbosity_trace)&&(!App::userSettings->suppressStartupDialogs) )
        {
#ifdef SIM_WITH_GUI
            App::uiThread->messageBox_information(App::mainWindow,"Tracing","Tracing is turned on: this might lead to drastic performance loss.",VMESSAGEBOX_OKELI,VMESSAGEBOX_REPLY_OK);
#else
            App::logMsg(sim_verbosity_warnings,"tracing is turned on: this might lead to drastic performance loss.");
#endif
        }
    }

#ifdef SIM_WITH_GUI
    if (cmd.cmdId==PLUS_CVU_CMD)
    {
        SUIThreadCommand cmdIn;
        SUIThreadCommand cmdOut;
        cmdIn.cmdId=PLUS_CVU_CMD_UITHREADCMD;
        {
            // Following instruction very important in the function below tries to lock resources (or a plugin it calls!):
            SIM_THREAD_INDICATE_UI_THREAD_CAN_DO_ANYTHING;
            App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
        }
    }
    if (cmd.cmdId==PLUS_HVUD_CMD)
    {
        SUIThreadCommand cmdIn;
        SUIThreadCommand cmdOut;
        cmdIn.cmdId=PLUS_HVUD_CMD_UITHREADCMD;
        {
            // Following instruction very important in the function below tries to lock resources (or a plugin it calls!):
            SIM_THREAD_INDICATE_UI_THREAD_CAN_DO_ANYTHING;
            App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
        }
    }

    if (cmd.sceneUniqueId==App::currentWorld->environment->getSceneUniqueID())
    {
        if (cmd.cmdId==TOGGLE_EXPAND_COLLAPSE_HIERARCHY_OBJECT_CMD)
        {
            CSceneObject* obj=App::currentWorld->sceneObjects->getObjectFromHandle(cmd.intParams[0]);
            if (obj!=nullptr)
            {
                obj->setObjectProperty(obj->getObjectProperty()^sim_objectproperty_collapsed);
                App::undoRedo_sceneChanged(""); // ************************** UNDO thingy **************************
            }
        }
        if (cmd.cmdId==ADD_OR_REMOVE_TO_FROM_OBJECT_SELECTION_CMD)
        {
            for (size_t i=0;i<cmd.intParams.size();i++)
            {
                CSceneObject* obj=App::currentWorld->sceneObjects->getObjectFromHandle(cmd.intParams[i]);
                if (obj!=nullptr)
                {
                    if (cmd.boolParams[i])
                        App::currentWorld->sceneObjects->addObjectToSelection(cmd.intParams[i]);
                    else
                        App::currentWorld->sceneObjects->removeObjectFromSelection(cmd.intParams[i]);
                    App::undoRedo_sceneChanged(""); // ************************** UNDO thingy **************************
                }
            }
        }
        if (cmd.cmdId==CLICK_RAY_INTERSECTION_CMD_OLD)
            _handleClickRayIntersection_old(cmd);

        if (cmd.cmdId==SWAP_VIEWS_CMD)
        {
            CSPage* p=App::currentWorld->pageContainer->getPage(cmd.intParams[0]);
            if (p!=nullptr)
            {
                p->swapViews(size_t(cmd.intParams[1]),size_t(cmd.intParams[2]),cmd.boolParams[0]);
                App::undoRedo_sceneChanged(""); // **************** UNDO THINGY ****************
            }
        }

        if (cmd.cmdId==CALL_MODULE_ENTRY_CMD)
        {
            CScriptObject* script=App::worldContainer->getScriptFromHandle(cmd.intParams[0]);
            if (script!=nullptr)
            {
                CInterfaceStack* inStack=App::worldContainer->interfaceStackContainer->createStack();
                inStack->pushTableOntoStack();
                inStack->pushStringOntoStack("handle",0);
                inStack->pushInt32OntoStack(cmd.intParams[1]);
                inStack->insertDataIntoStackTable();
                script->systemCallScript(sim_syscb_moduleentry,inStack,nullptr);
                App::worldContainer->interfaceStackContainer->destroyStack(inStack);
            }
        }

        if (cmd.cmdId==CALL_USER_CONFIG_CALLBACK_CMD)
        {
            CScriptObject* script=App::currentWorld->embeddedScriptContainer->getScriptFromObjectAttachedTo(sim_scripttype_customizationscript,cmd.intParams[0]);
            if ( (script!=nullptr)&&(script->getContainsUserConfigCallbackFunction()) )
            { // we have a user config callback
                script->systemCallScript(sim_syscb_userconfig,nullptr,nullptr);
            }
            else
            {
                SUIThreadCommand cmdIn;
                SUIThreadCommand cmdOut;
                cmdIn.cmdId=OPEN_MODAL_SCRIPT_SIMULATION_PARAMETERS_UITHREADCMD;
                cmdIn.intParams.push_back(cmd.intParams[0]);
                // Make sure we have a script param object:
                CSceneObject* object=App::currentWorld->sceneObjects->getObjectFromHandle(cmd.intParams[0]);
                if (object->getUserScriptParameterObject()==nullptr)
                    object->setUserScriptParameterObject(new CUserParameters());
                {
                    // Following instruction very important in the function below tries to lock resources (or a plugin it calls!):
                    SIM_THREAD_INDICATE_UI_THREAD_CAN_DO_ANYTHING;
                    App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
                }
            }
        }
        if (cmd.cmdId==OPEN_SCRIPT_EDITOR_CMD)
        {
            if (App::getEditModeType()==NO_EDIT_MODE)
            {
                CScriptObject* it=App::worldContainer->getScriptFromHandle(cmd.intParams[0]);
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
                App::undoRedo_sceneChanged(""); // **************** UNDO THINGY ****************
                App::setFullDialogRefreshFlag();
            }
        }

        if (cmd.cmdId==CLOSE_FLOATING_VIEW_CMD)
        {
            CSPage* page=App::currentWorld->pageContainer->getPage(App::currentWorld->pageContainer->getActivePageIndex());
            if (page!=nullptr)
            {
                CSView* theFloatingView=page->getView(size_t(cmd.intParams[0]));
                if (theFloatingView!=nullptr)
                {
                    if (theFloatingView->getCanBeClosed())
                    {
                        page->removeFloatingView(size_t(cmd.intParams[0]));
                        App::undoRedo_sceneChanged(""); // ************************** UNDO thingy **************************
                    }
                }
            }
        }

        if (cmd.cmdId==REMOVE_CURRENT_PAGE_CMD)
        {
            App::currentWorld->pageContainer->removePage(App::currentWorld->pageContainer->getActivePageIndex());
            App::logMsg(sim_verbosity_msgs,IDSNS_REMOVED_VIEW);
            App::undoRedo_sceneChanged(""); // ************************** UNDO thingy **************************
        }

        // Edit mode commands:
        if ( (App::mainWindow!=nullptr)&&(cmd.cmdId>EDIT_MODE_COMMANDS_START_EMCMD)&&(cmd.cmdId<EDIT_MODE_COMMANDS_END_EMCMD) )
        {
            CSceneObject* additionalSceneObject=nullptr;
            if (cmd.cmdId==PATH_EDIT_MODE_OLD_APPEND_NEW_PATH_POINT_FROM_CAMERA_EMCMD)
            {
                additionalSceneObject=(CSceneObject*)cmd.objectParams[0];
                if (additionalSceneObject!=nullptr)
                { // make sure that object is still valid:
                    if (!App::currentWorld->sceneObjects->doesObjectExist(additionalSceneObject))
                        additionalSceneObject=nullptr;
                }
            }
            App::mainWindow->editModeContainer->processCommand(cmd.cmdId,additionalSceneObject);
        }

        if (cmd.cmdId==DISPLAY_MESSAGE_CMD)
        {
            if (App::mainWindow!=nullptr)
            {
                if (cmd.intParams[0]==sim_msgbox_type_info)
                    App::uiThread->messageBox_information(App::mainWindow,cmd.stringParams[0].c_str(),cmd.stringParams[1].c_str(),VMESSAGEBOX_OKELI,VMESSAGEBOX_REPLY_OK);
                if (cmd.intParams[0]==sim_msgbox_type_warning)
                    App::uiThread->messageBox_warning(App::mainWindow,cmd.stringParams[0].c_str(),cmd.stringParams[1].c_str(),VMESSAGEBOX_OKELI,VMESSAGEBOX_REPLY_OK);
                if (cmd.intParams[0]==sim_msgbox_type_critical)
                    App::uiThread->messageBox_critical(App::mainWindow,cmd.stringParams[0].c_str(),cmd.stringParams[1].c_str(),VMESSAGEBOX_OKELI,VMESSAGEBOX_REPLY_OK);
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
                    CSPage* page=App::currentWorld->pageContainer->getPage(App::currentWorld->pageContainer->getActivePageIndex());
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
        if (cmd.cmdId==OBJECT_SHIFT_NAVIGATION_CMD)
            CToolBarCommand::processCommand(cmd.cmdId);
        if (cmd.cmdId==OBJECT_ROTATE_NAVIGATION_CMD)
            CToolBarCommand::processCommand(cmd.cmdId);
        if (cmd.cmdId==PAGE_SELECTOR_CMD)
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
            CSPage* page=App::currentWorld->pageContainer->getPage(App::currentWorld->pageContainer->getActivePageIndex());
            if (page!=nullptr)
            {
                CSView* view=page->getView(size_t(cmd.intParams[0]));
                if (view!=nullptr)
                    view->processCommand(cmd.cmdId,cmd.intParams[0]);
            }
        }

        if ( (cmd.cmdId>VIEW_SELECTOR_START_VSCMD)&&(cmd.cmdId<VIEW_SELECTOR_END_VSCMD) )
            App::mainWindow->oglSurface->viewSelector->processCommand(cmd.cmdId,cmd.intParams[0]);


        if (cmd.cmdId==MODEL_BROWSER_DRAG_AND_DROP_CMD)
        {
            CFileOperations::loadModel(cmd.stringParams[0].c_str(),true,false,false,nullptr,false,false);
            if (App::currentWorld->sceneObjects->getSelectionCount()==1)
            { // we could have several model bases (in the old fileformat)
                CSceneObject* obj=App::currentWorld->sceneObjects->getLastSelectionObject();
                if (obj!=nullptr)
                {
// Not anymore! 30/12/2016                    if ( (obj->getObjectMovementPreferredAxes()&0x03)||(obj->getObjectMovementRelativity(0)!=0) )
//                    { // We can only place the model if the X and/or Y manip are set or if the placement is not relative to world
                        C7Vector tr(obj->getFullLocalTransformation());
                        float ss=obj->getObjectMovementStepSize(0);
                        if (ss==0.0)
                            ss=App::userSettings->getTranslationStepSize();
                        float x=cmd.floatParams[0]-fmod(cmd.floatParams[0],ss);
                        float y=cmd.floatParams[1]-fmod(cmd.floatParams[1],ss);
                        tr.X(0)+=x;
                        tr.X(1)+=y;
                        obj->setLocalTransformation(tr);
//                    }
                }
            }
            App::mainWindow->openglWidget->clearModelDragAndDropInfo();
            App::undoRedo_sceneChanged(""); // ************************** UNDO thingy **************************
        }

        if (cmd.cmdId==DISPLAY_VARIOUS_WARNING_MESSAGES_DURING_SIMULATION_CMD)
        {
            if ( (!App::currentWorld->simulation->isSimulationStopped())&&(!App::isFullScreen())&&(App::mainWindow!=nullptr) )
            {
                _displayVariousWaningMessagesDuringSimulation();
                App::appendSimulationThreadCommand(cmd,500);
            }
        }

        if (cmd.cmdId==SET_VIEW_ANGLE_CAMERAGUITRIGGEREDCMD)
        {
            CCamera* cam=App::currentWorld->sceneObjects->getCameraFromHandle(cmd.intParams[0]);
            if (cam!=nullptr)
                cam->setViewAngle(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_ORTHO_VIEW_SIZE_CAMERAGUITRIGGEREDCMD)
        {
            CCamera* cam=App::currentWorld->sceneObjects->getCameraFromHandle(cmd.intParams[0]);
            if (cam!=nullptr)
                cam->setOrthoViewSize(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_NEAR_CLIPPING_CAMERAGUITRIGGEREDCMD)
        {
            CCamera* cam=App::currentWorld->sceneObjects->getCameraFromHandle(cmd.intParams[0]);
            if (cam!=nullptr)
                cam->setNearClippingPlane(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_FAR_CLIPPING_CAMERAGUITRIGGEREDCMD)
        {
            CCamera* cam=App::currentWorld->sceneObjects->getCameraFromHandle(cmd.intParams[0]);
            if (cam!=nullptr)
                cam->setFarClippingPlane(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_TRACKED_OBJECT_CAMERAGUITRIGGEREDCMD)
        {
            CCamera* cam=App::currentWorld->sceneObjects->getCameraFromHandle(cmd.intParams[0]);
            if (cam!=nullptr)
            {
                if (cmd.intParams[1]<0)
                    cam->setTrackedObjectHandle(-1);
                else
                {
                    CSceneObject* obj=App::currentWorld->sceneObjects->getObjectFromHandle(cmd.intParams[1]);
                    if (obj!=nullptr)
                        cam->setTrackedObjectHandle(obj->getObjectHandle());
                }
            }
        }
        if (cmd.cmdId==TOGGLE_SHOWFOG_CAMERAGUITRIGGEREDCMD)
        {
            CCamera* cam=App::currentWorld->sceneObjects->getCameraFromHandle(cmd.intParams[0]);
            if (cam!=nullptr)
                cam->setShowFogIfAvailable(!cam->getShowFogIfAvailable());
        }
        if (cmd.cmdId==TOGGLE_SHOWVOLUME_CAMERAGUITRIGGEREDCMD)
        {
            CCamera* cam=App::currentWorld->sceneObjects->getCameraFromHandle(cmd.intParams[0]);
            if (cam!=nullptr)
                cam->setShowVolume(!cam->getShowVolume());
        }
        if (cmd.cmdId==TOGGLE_USEPARENTASMANIPPROXY_CAMERAGUITRIGGEREDCMD)
        {
            CCamera* cam=App::currentWorld->sceneObjects->getCameraFromHandle(cmd.intParams[0]);
            if (cam!=nullptr)
                cam->setUseParentObjectAsManipulationProxy(!cam->getUseParentObjectAsManipulationProxy());
        }
        if (cmd.cmdId==TOGGLE_SHIFTALONGX_CAMERAGUITRIGGEREDCMD)
        {
            CCamera* cam=App::currentWorld->sceneObjects->getCameraFromHandle(cmd.intParams[0]);
            if (cam!=nullptr)
                cam->setCameraManipulationModePermissions(cam->getCameraManipulationModePermissions()^0x001);
        }
        if (cmd.cmdId==TOGGLE_SHIFTALONGY_CAMERAGUITRIGGEREDCMD)
        {
            CCamera* cam=App::currentWorld->sceneObjects->getCameraFromHandle(cmd.intParams[0]);
            if (cam!=nullptr)
                cam->setCameraManipulationModePermissions(cam->getCameraManipulationModePermissions()^0x002);
        }
        if (cmd.cmdId==TOGGLE_SHIFTALONGZ_CAMERAGUITRIGGEREDCMD)
        {
            CCamera* cam=App::currentWorld->sceneObjects->getCameraFromHandle(cmd.intParams[0]);
            if (cam!=nullptr)
                cam->setCameraManipulationModePermissions(cam->getCameraManipulationModePermissions()^0x004);
        }
        if (cmd.cmdId==TOGGLE_FULLROTATION_CAMERAGUITRIGGEREDCMD)
        {
            CCamera* cam=App::currentWorld->sceneObjects->getCameraFromHandle(cmd.intParams[0]);
            if (cam!=nullptr)
                cam->setCameraManipulationModePermissions(cam->getCameraManipulationModePermissions()^0x008);
        }
        if (cmd.cmdId==TOGGLE_TILTING_CAMERAGUITRIGGEREDCMD)
        {
            CCamera* cam=App::currentWorld->sceneObjects->getCameraFromHandle(cmd.intParams[0]);
            if (cam!=nullptr)
                cam->setCameraManipulationModePermissions(cam->getCameraManipulationModePermissions()^0x010);
        }
        if (cmd.cmdId==SET_SIZE_CAMERAGUITRIGGEREDCMD)
        {
            CCamera* cam=App::currentWorld->sceneObjects->getCameraFromHandle(cmd.intParams[0]);
            if (cam!=nullptr)
                cam->setCameraSize(cmd.floatParams[0]);
        }
        if (cmd.cmdId==TOGGLE_LOCALLIGHTS_CAMERAGUITRIGGEREDCMD)
        {
            CCamera* cam=App::currentWorld->sceneObjects->getCameraFromHandle(cmd.intParams[0]);
            if (cam!=nullptr)
                cam->setUseLocalLights(!cam->getuseLocalLights());
        }
        if (cmd.cmdId==TOGGLE_ALLOWPICKING_CAMERAGUITRIGGEREDCMD)
        {
            CCamera* cam=App::currentWorld->sceneObjects->getCameraFromHandle(cmd.intParams[0]);
            if (cam!=nullptr)
                cam->setAllowPicking(!cam->getAllowPicking());
        }
        if (cmd.cmdId==SET_RENDERMODE_CAMERAGUITRIGGEREDCMD)
        {
            CCamera* cam=App::currentWorld->sceneObjects->getCameraFromHandle(cmd.intParams[0]);
            if (cam!=nullptr)
                cam->setRenderMode(cmd.intParams[1],cmd.boolParams[0],cmd.boolParams[1]);
        }
        if (cmd.cmdId==NEW_COLLECTION_COLLECTIONGUITRIGGEREDCMD)
        {
            CCollection* newGroup=new CCollection(-2);
            newGroup->setCollectionName(IDSOGL_COLLECTION,false);
            App::currentWorld->collections->addCollection(newGroup,false);
            // Now select the new collection in the UI. We need to post it so that it arrives after the dialog refresh!:
            SSimulationThreadCommand cmd2;
            cmd2.cmdId=CALL_DIALOG_FUNCTION_GUITRIGGEREDCMD;
            cmd2.intParams.push_back(COLLECTION_DLG);
            cmd2.intParams.push_back(0);
            cmd2.intParams.push_back(newGroup->getCollectionHandle());
            App::appendSimulationThreadCommand(cmd2);
        }
        if (cmd.cmdId==TOGGLE_OVERRIDE_COLLECTIONGUITRIGGEREDCMD)
        {
            CCollection* theGroup=App::currentWorld->collections->getObjectFromHandle(cmd.intParams[0]);
            if (theGroup!=nullptr)
                theGroup->setOverridesObjectMainProperties(!theGroup->getOverridesObjectMainProperties());
        }
        if (cmd.cmdId==REMOVE_COLLECTION_COLLECTIONGUITRIGGEREDCMD)
        {
            App::currentWorld->collections->removeCollection(cmd.intParams[0]);
        }
        if (cmd.cmdId==REMOVE_COLLECTION_ITEM_COLLECTIONGUITRIGGEREDCMD)
        {
            CCollection* theGroup=App::currentWorld->collections->getObjectFromHandle(cmd.intParams[0]);
            if (theGroup!=nullptr)
            {
                for (size_t i=1;i<cmd.intParams.size();i++)
                    theGroup->removeCollectionElementFromHandle(cmd.intParams[i]);
                if (theGroup->getElementCount()==0)
                { // The group is empty and we have to remove it
                    App::currentWorld->collections->removeCollection(theGroup->getCollectionHandle());
                }
            }
        }
        if (cmd.cmdId==RENAME_COLLECTION_COLLECTIONGUITRIGGEREDCMD)
        {
            CCollection* it=App::currentWorld->collections->getObjectFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setCollectionName(cmd.stringParams[0].c_str(),true);
        }
        if (cmd.cmdId==ADD_COLLECTION_ITEM_EVERYTHING_COLLECTIONGUITRIGGEREDCMD)
        {
            CCollection* it=App::currentWorld->collections->getObjectFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
            {
                CCollectionElement* grEl=new CCollectionElement(-1,sim_collectionelement_all,true);
                it->addCollectionElement(grEl);
            }
        }
        if (cmd.cmdId==ADD_COLLECTION_ITEM_LOOS_COLLECTIONGUITRIGGEREDCMD)
        {
            CCollection* it=App::currentWorld->collections->getObjectFromHandle(cmd.intParams[0]);
            bool additive=cmd.boolParams[0];
            if (it!=nullptr)
            {
                for (size_t i=1;i<cmd.intParams.size();i++)
                {
                    CSceneObject* obj=App::currentWorld->sceneObjects->getObjectFromHandle(cmd.intParams[i]);
                    if (obj!=nullptr)
                    {
                        CCollectionElement* grEl=new CCollectionElement(obj->getObjectHandle(),sim_collectionelement_loose,additive);
                        it->addCollectionElement(grEl);
                    }
                }
            }
        }
        if (cmd.cmdId==ADD_COLLECTION_ITEM_FROMBASE_COLLECTIONGUITRIGGEREDCMD)
        {
            CCollection* it=App::currentWorld->collections->getObjectFromHandle(cmd.intParams[0]);
            bool additive=cmd.boolParams[0];
            bool baseInclusive=cmd.boolParams[1];
            if (it!=nullptr)
            {
                CSceneObject* lastSel=App::currentWorld->sceneObjects->getObjectFromHandle(cmd.intParams[1]);
                if (lastSel!=nullptr)
                {
                    int grpType=sim_collectionelement_frombaseincluded;
                    if (!baseInclusive)
                        grpType=sim_collectionelement_frombaseexcluded;
                    CCollectionElement* grEl=new CCollectionElement(lastSel->getObjectHandle(),grpType,additive);
                    it->addCollectionElement(grEl);
                }
            }
        }
        if (cmd.cmdId==ADD_COLLECTION_ITEM_FROMTIP_COLLECTIONGUITRIGGEREDCMD)
        {
            CCollection* it=App::currentWorld->collections->getObjectFromHandle(cmd.intParams[0]);
            bool additive=cmd.boolParams[0];
            bool tipInclusive=cmd.boolParams[1];
            if (it!=nullptr)
            {
                CSceneObject* lastSel=App::currentWorld->sceneObjects->getObjectFromHandle(cmd.intParams[1]);
                if (lastSel!=nullptr)
                {
                    int grpType=sim_collectionelement_fromtipincluded;
                    if (!tipInclusive)
                        grpType=sim_collectionelement_fromtipexcluded;
                    CCollectionElement* grEl=new CCollectionElement(lastSel->getObjectHandle(),grpType,additive);
                    it->addCollectionElement(grEl);
                }
            }
        }


        if (cmd.cmdId==TOGGLE_EXPLICITHANDLING_VISIONSENSORGUITRIGGEREDCMD)
        {
            CVisionSensor* it=App::currentWorld->sceneObjects->getVisionSensorFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setExplicitHandling(!it->getExplicitHandling());
        }
        if (cmd.cmdId==TOGGLE_EXTERNALINPUT_VISIONSENSORGUITRIGGEREDCMD)
        {
            CVisionSensor* it=App::currentWorld->sceneObjects->getVisionSensorFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setUseExternalImage(!it->getUseExternalImage());
        }
        if (cmd.cmdId==TOGGLE_LOCALLIGHTS_VISIONSENSORGUITRIGGEREDCMD)
        {
            CVisionSensor* it=App::currentWorld->sceneObjects->getVisionSensorFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setUseLocalLights(!it->getuseLocalLights());
        }
        if (cmd.cmdId==TOGGLE_SHOWVOLUME_VISIONSENSORGUITRIGGEREDCMD)
        {
            CVisionSensor* it=App::currentWorld->sceneObjects->getVisionSensorFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setShowVolume(!it->getShowVolume());
        }
        if (cmd.cmdId==TOGGLE_SHOWFOG_VISIONSENSORGUITRIGGEREDCMD)
        {
            CVisionSensor* it=App::currentWorld->sceneObjects->getVisionSensorFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setShowFogIfAvailable(!it->getShowFogIfAvailable());
        }
        if (cmd.cmdId==SET_NEARCLIPPING_VISIONSENSORGUITRIGGEREDCMD)
        {
            CVisionSensor* it=App::currentWorld->sceneObjects->getVisionSensorFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setNearClippingPlane(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_FARCLIPPING_VISIONSENSORGUITRIGGEREDCMD)
        {
            CVisionSensor* it=App::currentWorld->sceneObjects->getVisionSensorFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setFarClippingPlane(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_PERSPECTANGLE_OR_ORTHOSIZE_VISIONSENSORGUITRIGGEREDCMD)
        {
            CVisionSensor* it=App::currentWorld->sceneObjects->getVisionSensorFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
            {
                if (it->getPerspective())
                    it->setViewAngle(cmd.floatParams[0]);
                else
                    it->setOrthoViewSize(cmd.floatParams[0]);
            }
        }
        if (cmd.cmdId==SET_RESOLUTION_VISIONSENSORGUITRIGGEREDCMD)
        {
            CVisionSensor* it=App::currentWorld->sceneObjects->getVisionSensorFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setResolution(&cmd.intParams[1]);
        }
        if (cmd.cmdId==SET_OBJECTSIZE_VISIONSENSORGUITRIGGEREDCMD)
        {
            CVisionSensor* it=App::currentWorld->sceneObjects->getVisionSensorFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setVisionSensorSize(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_DEFAULTIMGCOL_VISIONSENSORGUITRIGGEREDCMD)
        {
            CVisionSensor* it=App::currentWorld->sceneObjects->getVisionSensorFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
            {
                it->setDefaultBufferValues(&cmd.floatParams[0]);
                it->setUseEnvironmentBackgroundColor(cmd.boolParams[0]);
            }
        }
        if (cmd.cmdId==APPLY_MAINPROP_TO_SELECTION_VISIONSENSORGUITRIGGEREDCMD)
        {
            CVisionSensor* last=App::currentWorld->sceneObjects->getVisionSensorFromHandle(cmd.intParams[0]);
            if (last!=nullptr)
            {
                int r[2];
                last->getResolution(r);
                float b[3];
                last->getDefaultBufferValues(b);
                for (size_t i=1;i<cmd.intParams.size();i++)
                {
                    CVisionSensor* it=App::currentWorld->sceneObjects->getVisionSensorFromHandle(cmd.intParams[i]);
                    if (it!=nullptr)
                    {
                        it->setRenderMode(last->getRenderMode());
                        it->setExplicitHandling(last->getExplicitHandling());
                        it->setUseExternalImage(last->getUseExternalImage());
                        it->setPerspective(last->getPerspective());
                        it->setShowFogIfAvailable(last->getShowFogIfAvailable());
                        it->setIgnoreRGBInfo(last->getIgnoreRGBInfo());
                        it->setIgnoreDepthInfo(last->getIgnoreDepthInfo());
                        it->setComputeImageBasicStats(last->getComputeImageBasicStats());
                        it->setNearClippingPlane(last->getNearClippingPlane());
                        it->setFarClippingPlane(last->getFarClippingPlane());
                        it->setViewAngle(last->getViewAngle());
                        it->setOrthoViewSize(last->getOrthoViewSize());
                        it->setResolution(r);
                        it->setUseLocalLights(last->getuseLocalLights());
                        it->setUseEnvironmentBackgroundColor(last->getUseEnvironmentBackgroundColor());
                        it->setDefaultBufferValues(b);
                        it->setDetectableEntityHandle(last->getDetectableEntityHandle());
                    }
                }
            }
        }
        if (cmd.cmdId==APPLY_VISUALPROP_TO_SELECTION_VISIONSENSORGUITRIGGEREDCMD)
        {
            CVisionSensor* last=App::currentWorld->sceneObjects->getVisionSensorFromHandle(cmd.intParams[0]);
            if (last!=nullptr)
            {
                for (size_t i=1;i<cmd.intParams.size();i++)
                {
                    CVisionSensor* it=App::currentWorld->sceneObjects->getVisionSensorFromHandle(cmd.intParams[i]);
                    if (it!=nullptr)
                    {
                        last->getColor()->copyYourselfInto(it->getColor());
                        it->setVisionSensorSize(last->getVisionSensorSize());
                        it->setShowVolume(last->getShowVolume());
                    }
                }
            }
        }
        if (cmd.cmdId==TOGGLE_IGNORERGB_VISIONSENSORGUITRIGGEREDCMD)
        {
            CVisionSensor* it=App::currentWorld->sceneObjects->getVisionSensorFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setIgnoreRGBInfo(!it->getIgnoreRGBInfo());
        }
        if (cmd.cmdId==TOGGLE_IGNOREDEPTH_VISIONSENSORGUITRIGGEREDCMD)
        {
            CVisionSensor* it=App::currentWorld->sceneObjects->getVisionSensorFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setIgnoreDepthInfo(!it->getIgnoreDepthInfo());
        }
        if (cmd.cmdId==TOGGLE_PACKET1BLANK_VISIONSENSORGUITRIGGEREDCMD)
        {
            CVisionSensor* it=App::currentWorld->sceneObjects->getVisionSensorFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setComputeImageBasicStats(!it->getComputeImageBasicStats());
        }
        if (cmd.cmdId==SET_RENDERMODE_VISIONSENSORGUITRIGGEREDCMD)
        {
            CVisionSensor* it=App::currentWorld->sceneObjects->getVisionSensorFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setRenderMode(cmd.intParams[1]);
        }


        if (cmd.cmdId==ADD_NEW_COLLISIONGUITRIGGEREDCMD)
        {
            int h=App::currentWorld->collisions->addNewObject(cmd.intParams[0],cmd.intParams[1],"");
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
            CCollisionObject_old* it=App::currentWorld->collisions->getObjectFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setObjectName(cmd.stringParams[0].c_str(),true);
        }
        if (cmd.cmdId==DELETE_OBJECT_COLLISIONGUITRIGGEREDCMD)
        {
            CCollisionObject_old* it=App::currentWorld->collisions->getObjectFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                App::currentWorld->collisions->removeObject(it->getObjectHandle());
        }
        if (cmd.cmdId==TOGGLE_ENABLE_ALL_COLLISIONGUITRIGGEREDCMD)
        {
            App::currentWorld->mainSettings->collisionDetectionEnabled=!App::currentWorld->mainSettings->collisionDetectionEnabled;
        }
        if (cmd.cmdId==TOGGLE_EXPLICITHANDLING_COLLISIONGUITRIGGEREDCMD)
        {
            CCollisionObject_old* it=App::currentWorld->collisions->getObjectFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setExplicitHandling(!it->getExplicitHandling());
        }
        if (cmd.cmdId==TOGGLE_COLLIDERCOLORCHANGES_COLLISIONGUITRIGGEREDCMD)
        {
            CCollisionObject_old* it=App::currentWorld->collisions->getObjectFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setColliderChangesColor(!it->getColliderChangesColor());
        }
        if (cmd.cmdId==TOGGLE_COLLIDEECOLORCHANGES_COLLISIONGUITRIGGEREDCMD)
        {
            CCollisionObject_old* it=App::currentWorld->collisions->getObjectFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setCollideeChangesColor(!it->getCollideeChangesColor());
        }
        if (cmd.cmdId==TOGGLE_COLLISIONCONTOUR_COLLISIONGUITRIGGEREDCMD)
        {
            CCollisionObject_old* it=App::currentWorld->collisions->getObjectFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setExhaustiveDetection(!it->getExhaustiveDetection());
        }
        if (cmd.cmdId==SET_CONTOURWIDTH_COLLISIONGUITRIGGEREDCMD)
        {
            CCollisionObject_old* it=App::currentWorld->collisions->getObjectFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setContourWidth(cmd.intParams[1]);
        }


        if (cmd.cmdId==ADD_NEW_DISTANCEGUITRIGGEREDCMD)
        {
            int h=App::currentWorld->distances->addNewObject(cmd.intParams[0],cmd.intParams[1],"");
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
            CDistanceObject_old* it=App::currentWorld->distances->getObjectFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setObjectName(cmd.stringParams[0].c_str(),true);
        }
        if (cmd.cmdId==DELETE_OBJECT_DISTANCEGUITRIGGEREDCMD)
        {
            CDistanceObject_old* it=App::currentWorld->distances->getObjectFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                App::currentWorld->distances->removeObject(it->getObjectHandle());
        }
        if (cmd.cmdId==TOGGLE_ENABLE_ALL_DISTANCEGUITRIGGEREDCMD)
        {
            App::currentWorld->mainSettings->distanceCalculationEnabled=!App::currentWorld->mainSettings->distanceCalculationEnabled;
        }
        if (cmd.cmdId==TOGGLE_EXPLICITHANDLING_DISTANCEGUITRIGGEREDCMD)
        {
            CDistanceObject_old* it=App::currentWorld->distances->getObjectFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setExplicitHandling(!it->getExplicitHandling());
        }
        if (cmd.cmdId==TOGGLE_USETHRESHOLD_DISTANCEGUITRIGGEREDCMD)
        {
            CDistanceObject_old* it=App::currentWorld->distances->getObjectFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setThresholdEnabled(!it->getTreshholdEnabled());
        }
        if (cmd.cmdId==SET_THRESHOLD_DISTANCEGUITRIGGEREDCMD)
        {
            CDistanceObject_old* it=App::currentWorld->distances->getObjectFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setThreshold(cmd.floatParams[0]);
        }
        if (cmd.cmdId==TOGGLE_SEGMENTDISPLAY_DISTANCEGUITRIGGEREDCMD)
        {
            CDistanceObject_old* it=App::currentWorld->distances->getObjectFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setDisplaySegment(!it->getDisplaySegment());
        }
        if (cmd.cmdId==SET_SEGMENTWIDTH_DISTANCEGUITRIGGEREDCMD)
        {
            CDistanceObject_old* it=App::currentWorld->distances->getObjectFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setSegmentWidth(cmd.floatParams[0]);
        }


        if (cmd.cmdId==TOGGLE_SELECTABLE_COMMONPROPGUITRIGGEREDCMD)
        {
            CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setObjectProperty(it->getObjectProperty()^sim_objectproperty_selectable);
        }
        if (cmd.cmdId==TOGGLE_SELECTBASEOFMODEL_COMMONPROPGUITRIGGEREDCMD)
        {
            CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setObjectProperty(it->getObjectProperty()^sim_objectproperty_selectmodelbaseinstead);
        }
        if (cmd.cmdId==TOGGLE_DONGTSHOWINSIDEMODELSELECTION_COMMONPROPGUITRIGGEREDCMD)
        {
            CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setObjectProperty(it->getObjectProperty()^sim_objectproperty_dontshowasinsidemodel);
        }
        if (cmd.cmdId==TOGGLE_VIEWFITTINGIGNORED_COMMONPROPGUITRIGGEREDCMD)
        {
            CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setObjectProperty(it->getObjectProperty()^sim_objectproperty_ignoreviewfitting);
        }
        if (cmd.cmdId==APPLY_GENERALPROP_COMMONPROPGUITRIGGEREDCMD)
        {
            CSceneObject* last=App::currentWorld->sceneObjects->getObjectFromHandle(cmd.intParams[0]);
            if (last!=nullptr)
            {
                int lastType=last->getObjectType();
                for (size_t i=1;i<cmd.intParams.size();i++)
                {
                    CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(cmd.intParams[i]);
                    if (it!=nullptr)
                    {
                        int objPropToCopy=sim_objectproperty_selectable|sim_objectproperty_selectmodelbaseinstead|sim_objectproperty_dontshowasinsidemodel|sim_objectproperty_selectinvisible|sim_objectproperty_depthinvisible|sim_objectproperty_cannotdelete|sim_objectproperty_cannotdeleteduringsim;
                        it->setObjectProperty(((it->getObjectProperty()|objPropToCopy)-objPropToCopy)|(last->getObjectProperty()&objPropToCopy));
                        it->setSizeFactor(last->getSizeFactor());
                        if (it->getObjectType()==lastType)
                            it->setExtensionString(last->getExtensionString().c_str());
                    }
                }
            }
        }
        if (cmd.cmdId==SET_CAMERAVISIBILITYLAYERS_COMMONPROPGUITRIGGEREDCMD)
        {
            CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
            {
                it->setVisibilityLayer(cmd.intParams[1]);
                // For backward compatibility: always renderable (now following the visibility layers as for cameras):
                if (it->isPotentiallyRenderable())
                    it->setLocalObjectSpecialProperty(it->getLocalObjectSpecialProperty()|sim_objectspecialproperty_renderable);
            }
        }
        if (cmd.cmdId==APPLY_VISIBILITYPROP_COMMONPROPGUITRIGGEREDCMD)
        {
            CSceneObject* last=App::currentWorld->sceneObjects->getObjectFromHandle(cmd.intParams[0]);
            if (last!=nullptr)
            {
                for (size_t i=1;i<cmd.intParams.size();i++)
                {
                    CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(cmd.intParams[i]);
                    if (it!=nullptr)
                    {
                        it->setVisibilityLayer(last->getVisibilityLayer());
                        it->setAuthorizedViewableObjects(last->getAuthorizedViewableObjects());
                    }
                }
            }
        }
        if (cmd.cmdId==TOGGLE_MODELBASE_COMMONPROPGUITRIGGEREDCMD)
        {
            CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
            {
                it->setModelBase(!it->getModelBase());
                if (it->getModelBase())
                    it->generateDnaString();
            }
        }
        if (cmd.cmdId==TOGGLE_COLLIDABLE_COMMONPROPGUITRIGGEREDCMD)
        {
            CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(cmd.intParams[0]);
            if ((it!=nullptr)&&it->isPotentiallyCollidable())
                it->setLocalObjectSpecialProperty(it->getLocalObjectSpecialProperty()^sim_objectspecialproperty_collidable);
        }
        if (cmd.cmdId==TOGGLE_MEASURABLE_COMMONPROPGUITRIGGEREDCMD)
        {
            CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(cmd.intParams[0]);
            if ((it!=nullptr)&&it->isPotentiallyMeasurable())
                it->setLocalObjectSpecialProperty(it->getLocalObjectSpecialProperty()^sim_objectspecialproperty_measurable);
        }
        if (cmd.cmdId==TOGGLE_RENDERABLE_COMMONPROPGUITRIGGEREDCMD)
        {
            CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(cmd.intParams[0]);
            if ((it!=nullptr)&&it->isPotentiallyRenderable())
                it->setLocalObjectSpecialProperty(it->getLocalObjectSpecialProperty()^sim_objectspecialproperty_renderable);
        }
        if (cmd.cmdId==TOGGLE_DETECTABLE_COMMONPROPGUITRIGGEREDCMD)
        {
            CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(cmd.intParams[0]);
            if ((it!=nullptr)&&it->isPotentiallyDetectable())
            {
                int t=it->getLocalObjectSpecialProperty()&sim_objectspecialproperty_detectable;
                if (t==sim_objectspecialproperty_detectable)
                {
                    t=it->getLocalObjectSpecialProperty()|sim_objectspecialproperty_detectable;
                    t-=sim_objectspecialproperty_detectable;
                }
                else
                    t=it->getLocalObjectSpecialProperty()|sim_objectspecialproperty_detectable;
                it->setLocalObjectSpecialProperty(t);
            }
        }
        if (cmd.cmdId==SET_DETECTABLEITEMS_COMMONPROPGUITRIGGEREDCMD)
        {
            CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(cmd.intParams[0]);
            if ((it!=nullptr)&&it->isPotentiallyDetectable())
                it->setLocalObjectSpecialProperty(cmd.intParams[1]);
        }
        if (cmd.cmdId==APPLY_SPECIALPROP_COMMONPROPGUITRIGGEREDCMD)
        {
            CSceneObject* last=App::currentWorld->sceneObjects->getObjectFromHandle(cmd.intParams[0]);
            if (last!=nullptr)
            {
                int settableBits=0;
                if (last->isPotentiallyCollidable())
                    settableBits|=sim_objectspecialproperty_collidable;
                if (last->isPotentiallyMeasurable())
                    settableBits|=sim_objectspecialproperty_measurable;
                if (last->isPotentiallyDetectable())
                    settableBits|=sim_objectspecialproperty_detectable;
                if (last->isPotentiallyRenderable())
                    settableBits|=sim_objectspecialproperty_renderable;
                int stateOfSettableBits=last->getLocalObjectSpecialProperty()&settableBits;
                for (size_t i=1;i<cmd.intParams.size();i++)
                {
                    CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(cmd.intParams[i]);
                    if (it!=nullptr)
                    {
                        int objProp=((it->getLocalObjectSpecialProperty()|settableBits)-settableBits)|stateOfSettableBits;
                        it->setLocalObjectSpecialProperty(objProp);
                    }
                }
            }
        }
        if (cmd.cmdId==TOGGLE_SELECTINVISIBLE_COMMONPROPGUITRIGGEREDCMD)
        {
            CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setObjectProperty(it->getObjectProperty()^sim_objectproperty_selectinvisible);
        }
        if (cmd.cmdId==TOGGLE_DEPTHMAPIGNORED_COMMONPROPGUITRIGGEREDCMD)
        {
            CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setObjectProperty(it->getObjectProperty()^sim_objectproperty_depthinvisible);
        }
        if (cmd.cmdId==TOGGLE_CANNOTBEDELETED_COMMONPROPGUITRIGGEREDCMD)
        {
            CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setObjectProperty(it->getObjectProperty()^sim_objectproperty_cannotdelete);
        }
        if (cmd.cmdId==TOGGLE_CANNOTBEDELETEDDURINGSIMULATION_COMMONPROPGUITRIGGEREDCMD)
        {
            CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setObjectProperty(it->getObjectProperty()^sim_objectproperty_cannotdeleteduringsim);
        }
        if (cmd.cmdId==SET_SELFCOLLISIONINDICATOR_COMMONPROPGUITRIGGEREDCMD)
        {
            CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setCollectionSelfCollisionIndicator(cmd.intParams[1]);
        }
        if (cmd.cmdId==SET_CANBESEENBY_COMMONPROPGUITRIGGEREDCMD)
        {
            CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setAuthorizedViewableObjects(cmd.intParams[1]);
        }
        if (cmd.cmdId==SET_EXTENSIONSTRING_COMMONPROPGUITRIGGEREDCMD)
        {
            CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(cmd.intParams[0]);
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
            CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
            {
                it->setModelProperty(cmd.intParams[1]);
                it->setModelAcknowledgement(cmd.stringParams[0].c_str());
            }
        }


        if (cmd.cmdId==SET_TIMESTEP_SIMULATIONGUITRIGGEREDCMD)
        {
            if (App::currentWorld->simulation->isSimulationStopped())
                App::currentWorld->simulation->setSimulationTimeStep_raw_us(cmd.uint64Params[0]);
        }
        if (cmd.cmdId==SET_PPF_SIMULATIONGUITRIGGEREDCMD)
        {
            if (App::currentWorld->simulation->isSimulationStopped())
                App::currentWorld->simulation->setSimulationPassesPerRendering_raw(cmd.intParams[0]);
        }
        if (cmd.cmdId==TOGGLE_REALTIME_SIMULATIONGUITRIGGEREDCMD)
        {
            if (App::currentWorld->simulation->isSimulationStopped())
                App::currentWorld->simulation->setRealTimeSimulation(!App::currentWorld->simulation->getRealTimeSimulation());
        }
        if (cmd.cmdId==SET_REALTIMEFACTOR_SIMULATIONGUITRIGGEREDCMD)
        {
            if (App::currentWorld->simulation->isSimulationStopped())
                App::currentWorld->simulation->setRealTimeCoefficient_raw(cmd.floatParams[0]);
        }
        if (cmd.cmdId==TOGGLE_TRYCATCHINGUP_SIMULATIONGUITRIGGEREDCMD)
        {
            if (App::currentWorld->simulation->isSimulationStopped())
                App::currentWorld->simulation->setCatchUpIfLate(!App::currentWorld->simulation->getCatchUpIfLate());
        }
        if (cmd.cmdId==TOGGLE_PAUSEATTIME_SIMULATIONGUITRIGGEREDCMD)
        {
            if (App::currentWorld->simulation->isSimulationStopped())
                App::currentWorld->simulation->setPauseAtSpecificTime(!App::currentWorld->simulation->getPauseAtSpecificTime());
        }
        if (cmd.cmdId==SET_PAUSETIME_SIMULATIONGUITRIGGEREDCMD)
        {
            if (App::currentWorld->simulation->isSimulationStopped())
                App::currentWorld->simulation->setPauseTime_us(cmd.uint64Params[0]);
        }
        if (cmd.cmdId==TOGGLE_PAUSEATERROR_SIMULATIONGUITRIGGEREDCMD)
        {
            if (App::currentWorld->simulation->isSimulationStopped())
                App::currentWorld->simulation->setPauseAtError(!App::currentWorld->simulation->getPauseAtError());
        }
        if (cmd.cmdId==TOGGLE_RESETSCENETOINITIAL_SIMULATIONGUITRIGGEREDCMD)
        {
            App::currentWorld->simulation->setResetSceneAtSimulationEnd(!App::currentWorld->simulation->getResetSceneAtSimulationEnd());
        }
        if (cmd.cmdId==TOGGLE_REMOVENEWOBJECTS_SIMULATIONGUITRIGGEREDCMD)
        {
            App::currentWorld->simulation->setRemoveNewObjectsAtSimulationEnd(!App::currentWorld->simulation->getRemoveNewObjectsAtSimulationEnd());
        }
        if (cmd.cmdId==TOGGLE_FULLSCREENATSTART_SIMULATIONGUITRIGGEREDCMD)
        {
            if (App::currentWorld->simulation->isSimulationStopped())
                App::currentWorld->simulation->setFullscreenAtSimulationStart(!App::currentWorld->simulation->getFullscreenAtSimulationStart());
        }
        if (cmd.cmdId==SET_TIMESTEPSCHEME_SIMULATIONGUITRIGGEREDCMD)
        {
            if (App::currentWorld->simulation->isSimulationStopped())
                App::currentWorld->simulation->setDefaultSimulationParameterIndex(cmd.intParams[0]);
        }


        if (cmd.cmdId==TOGGLE_DYNAMICS_DYNAMICSGUITRIGGEREDCMD)
        {
            App::currentWorld->dynamicsContainer->setDynamicsEnabled(!App::currentWorld->dynamicsContainer->getDynamicsEnabled());
            App::currentWorld->dynamicsContainer->removeWorld();
        }
        if (cmd.cmdId==SET_ENGINE_DYNAMICSGUITRIGGEREDCMD)
        {
            if (App::currentWorld->simulation->isSimulationStopped())
                App::currentWorld->dynamicsContainer->setDynamicEngineType(cmd.intParams[0],cmd.intParams[1]);
        }
        if (cmd.cmdId==TOGGLE_DISPLAYCONTACTS_DYNAMICSGUITRIGGEREDCMD)
        {
            App::currentWorld->dynamicsContainer->setDisplayContactPoints(!App::currentWorld->dynamicsContainer->getDisplayContactPoints());
        }
        if (cmd.cmdId==SET_GRAVITY_DYNAMICSGUITRIGGEREDCMD)
        {
            App::currentWorld->dynamicsContainer->setGravity(cmd.posParams[0]);
        }
        if (cmd.cmdId==SET_ALLGLOBALPARAMS_DYNAMICSGUITRIGGEREDCMD)
        {
            App::currentWorld->dynamicsContainer->setDynamicsSettingsMode(cmd.intParams[0]);
            App::currentWorld->dynamicsContainer->setBulletIntParams(cmd.intVectorParams[0],false);
            App::currentWorld->dynamicsContainer->setBulletFloatParams(cmd.floatVectorParams[0],false);
            App::currentWorld->dynamicsContainer->setOdeIntParams(cmd.intVectorParams[1],false);
            App::currentWorld->dynamicsContainer->setOdeFloatParams(cmd.floatVectorParams[1],false);
            App::currentWorld->dynamicsContainer->setVortexIntParams(cmd.intVectorParams[2],false);
            App::currentWorld->dynamicsContainer->setVortexFloatParams(cmd.floatVectorParams[2],false);
            App::currentWorld->dynamicsContainer->setNewtonIntParams(cmd.intVectorParams[3],false);
            App::currentWorld->dynamicsContainer->setNewtonFloatParams(cmd.floatVectorParams[3],false);
        }

        if (cmd.cmdId==TOGGLE_EXPLICITHANDLING_PROXSENSORGUITRIGGEREDCMD)
        {
            CProxSensor* it=App::currentWorld->sceneObjects->getProximitySensorFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setExplicitHandling(!it->getExplicitHandling());
        }
        if (cmd.cmdId==SET_SENSORSUBTYPE_PROXSENSORGUITRIGGEREDCMD)
        {
            CProxSensor* it=App::currentWorld->sceneObjects->getProximitySensorFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setSensableType(cmd.intParams[1]);
        }
        if (cmd.cmdId==TOGGLE_SHOWVOLUME_PROXSENSORGUITRIGGEREDCMD)
        {
            CProxSensor* it=App::currentWorld->sceneObjects->getProximitySensorFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setShowVolume(!it->getShowVolume());
        }
        if (cmd.cmdId==SET_POINTSIZE_PROXSENSORGUITRIGGEREDCMD)
        {
            CProxSensor* it=App::currentWorld->sceneObjects->getProximitySensorFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setProxSensorSize(cmd.floatParams[0]);
        }
        if (cmd.cmdId==APPLY_MAINPROP_PROXSENSORGUITRIGGEREDCMD)
        {
            CProxSensor* last=App::currentWorld->sceneObjects->getProximitySensorFromHandle(cmd.intParams[0]);
            if (last!=nullptr)
            {
                for (size_t i=1;i<cmd.intParams.size();i++)
                {
                    CProxSensor* it=App::currentWorld->sceneObjects->getProximitySensorFromHandle(cmd.intParams[i]);
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
        if (cmd.cmdId==APPLY_VISUALPROP_PROXSENSORGUITRIGGEREDCMD)
        {
            CProxSensor* it=App::currentWorld->sceneObjects->getProximitySensorFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
            {
                for (size_t i=1;i<cmd.intParams.size();i++)
                {
                    CProxSensor* it2=App::currentWorld->sceneObjects->getProximitySensorFromHandle(cmd.intParams[i]);
                    if (it2!=nullptr)
                    {
                        if (it->getObjectHandle()!=it2->getObjectHandle())
                        {
                            it->getColor(0)->copyYourselfInto(it2->getColor(0));
                            it->getColor(1)->copyYourselfInto(it2->getColor(1));
                            it2->setProxSensorSize(it->getProxSensorSize());
                            it2->setShowVolume(it->getShowVolume());
                        }
                    }
                }
            }
        }
        if (cmd.cmdId==SET_DETECTIONPARAMS_PROXSENSORGUITRIGGEREDCMD)
        {
            CProxSensor* it=App::currentWorld->sceneObjects->getProximitySensorFromHandle(cmd.intParams[0]);
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

        if (cmd.cmdId==SET_OFFSET_DETECTIONVOLUMEGUITRIGGEREDCMD)
        {
            CProxSensor* prox=App::currentWorld->sceneObjects->getProximitySensorFromHandle(cmd.intParams[0]);
            CMill* mill=App::currentWorld->sceneObjects->getMillFromHandle(cmd.intParams[0]);
            if (prox!=nullptr)
                prox->convexVolume->setOffset(cmd.floatParams[0]);
            if (mill!=nullptr)
                mill->convexVolume->setOffset(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_RADIUS_DETECTIONVOLUMEGUITRIGGEREDCMD)
        {
            CProxSensor* prox=App::currentWorld->sceneObjects->getProximitySensorFromHandle(cmd.intParams[0]);
            CMill* mill=App::currentWorld->sceneObjects->getMillFromHandle(cmd.intParams[0]);
            if (prox!=nullptr)
                prox->convexVolume->setRadius(cmd.floatParams[0]);
            if (mill!=nullptr)
                mill->convexVolume->setRadius(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_RADIUSFAR_DETECTIONVOLUMEGUITRIGGEREDCMD)
        {
            CProxSensor* prox=App::currentWorld->sceneObjects->getProximitySensorFromHandle(cmd.intParams[0]);
            CMill* mill=App::currentWorld->sceneObjects->getMillFromHandle(cmd.intParams[0]);
            if (prox!=nullptr)
                prox->convexVolume->setRadiusFar(cmd.floatParams[0]);
            if (mill!=nullptr)
                mill->convexVolume->setRadiusFar(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_RANGE_DETECTIONVOLUMEGUITRIGGEREDCMD)
        {
            CProxSensor* prox=App::currentWorld->sceneObjects->getProximitySensorFromHandle(cmd.intParams[0]);
            CMill* mill=App::currentWorld->sceneObjects->getMillFromHandle(cmd.intParams[0]);
            if (prox!=nullptr)
                prox->convexVolume->setRange(cmd.floatParams[0]);
            if (mill!=nullptr)
                mill->convexVolume->setRange(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_XSIZE_DETECTIONVOLUMEGUITRIGGEREDCMD)
        {
            CProxSensor* prox=App::currentWorld->sceneObjects->getProximitySensorFromHandle(cmd.intParams[0]);
            CMill* mill=App::currentWorld->sceneObjects->getMillFromHandle(cmd.intParams[0]);
            if (prox!=nullptr)
                prox->convexVolume->setXSize(cmd.floatParams[0]);
            if (mill!=nullptr)
                mill->convexVolume->setXSize(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_YSIZE_DETECTIONVOLUMEGUITRIGGEREDCMD)
        {
            CProxSensor* prox=App::currentWorld->sceneObjects->getProximitySensorFromHandle(cmd.intParams[0]);
            CMill* mill=App::currentWorld->sceneObjects->getMillFromHandle(cmd.intParams[0]);
            if (prox!=nullptr)
                prox->convexVolume->setYSize(cmd.floatParams[0]);
            if (mill!=nullptr)
                mill->convexVolume->setYSize(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_XSIZEFAR_DETECTIONVOLUMEGUITRIGGEREDCMD)
        {
            CProxSensor* prox=App::currentWorld->sceneObjects->getProximitySensorFromHandle(cmd.intParams[0]);
            CMill* mill=App::currentWorld->sceneObjects->getMillFromHandle(cmd.intParams[0]);
            if (prox!=nullptr)
                prox->convexVolume->setXSizeFar(cmd.floatParams[0]);
            if (mill!=nullptr)
                mill->convexVolume->setXSizeFar(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_YSIZEFAR_DETECTIONVOLUMEGUITRIGGEREDCMD)
        {
            CProxSensor* prox=App::currentWorld->sceneObjects->getProximitySensorFromHandle(cmd.intParams[0]);
            CMill* mill=App::currentWorld->sceneObjects->getMillFromHandle(cmd.intParams[0]);
            if (prox!=nullptr)
                prox->convexVolume->setYSizeFar(cmd.floatParams[0]);
            if (mill!=nullptr)
                mill->convexVolume->setYSizeFar(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_ANGLE_DETECTIONVOLUMEGUITRIGGEREDCMD)
        {
            CProxSensor* prox=App::currentWorld->sceneObjects->getProximitySensorFromHandle(cmd.intParams[0]);
            CMill* mill=App::currentWorld->sceneObjects->getMillFromHandle(cmd.intParams[0]);
            if (prox!=nullptr)
                prox->convexVolume->setAngle(cmd.floatParams[0]);
            if (mill!=nullptr)
                mill->convexVolume->setAngle(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_FACECOUNT_DETECTIONVOLUMEGUITRIGGEREDCMD)
        {
            CProxSensor* prox=App::currentWorld->sceneObjects->getProximitySensorFromHandle(cmd.intParams[0]);
            CMill* mill=App::currentWorld->sceneObjects->getMillFromHandle(cmd.intParams[0]);
            if (prox!=nullptr)
                prox->convexVolume->setFaceNumber(cmd.intParams[1]);
            if (mill!=nullptr)
                mill->convexVolume->setFaceNumber(cmd.intParams[1]);
        }
        if (cmd.cmdId==SET_FACECOUNTFAR_DETECTIONVOLUMEGUITRIGGEREDCMD)
        {
            CProxSensor* prox=App::currentWorld->sceneObjects->getProximitySensorFromHandle(cmd.intParams[0]);
            CMill* mill=App::currentWorld->sceneObjects->getMillFromHandle(cmd.intParams[0]);
            if (prox!=nullptr)
                prox->convexVolume->setFaceNumberFar(cmd.intParams[1]);
            if (mill!=nullptr)
                mill->convexVolume->setFaceNumberFar(cmd.intParams[1]);
        }
        if (cmd.cmdId==SET_SUBDIVISIONS_DETECTIONVOLUMEGUITRIGGEREDCMD)
        {
            CProxSensor* prox=App::currentWorld->sceneObjects->getProximitySensorFromHandle(cmd.intParams[0]);
            CMill* mill=App::currentWorld->sceneObjects->getMillFromHandle(cmd.intParams[0]);
            if (prox!=nullptr)
                prox->convexVolume->setSubdivisions(cmd.intParams[1]);
            if (mill!=nullptr)
                mill->convexVolume->setSubdivisions(cmd.intParams[1]);
        }
        if (cmd.cmdId==SET_SUBDIVISIONSFAR_DETECTIONVOLUMEGUITRIGGEREDCMD)
        {
            CProxSensor* prox=App::currentWorld->sceneObjects->getProximitySensorFromHandle(cmd.intParams[0]);
            CMill* mill=App::currentWorld->sceneObjects->getMillFromHandle(cmd.intParams[0]);
            if (prox!=nullptr)
                prox->convexVolume->setSubdivisionsFar(cmd.intParams[1]);
            if (mill!=nullptr)
                mill->convexVolume->setSubdivisionsFar(cmd.intParams[1]);
        }
        if (cmd.cmdId==SET_INSIDEGAP_DETECTIONVOLUMEGUITRIGGEREDCMD)
        {
            CProxSensor* prox=App::currentWorld->sceneObjects->getProximitySensorFromHandle(cmd.intParams[0]);
            CMill* mill=App::currentWorld->sceneObjects->getMillFromHandle(cmd.intParams[0]);
            if (prox!=nullptr)
                prox->convexVolume->setInsideAngleThing(cmd.floatParams[0]);
            if (mill!=nullptr)
                mill->convexVolume->setInsideAngleThing(cmd.floatParams[0]);
        }
        if (cmd.cmdId==APPLY_DETECTIONVOLUMEGUITRIGGEREDCMD)
        {
            CProxSensor* prox=App::currentWorld->sceneObjects->getProximitySensorFromHandle(cmd.intParams[0]);
            CMill* mill=App::currentWorld->sceneObjects->getMillFromHandle(cmd.intParams[0]);
            if (prox!=nullptr)
            {
                for (size_t i=1;i<cmd.intParams.size();i++)
                {
                    CProxSensor* it=App::currentWorld->sceneObjects->getProximitySensorFromHandle(cmd.intParams[i]);
                    if ( (it!=nullptr)&&(it->getSensorType()==prox->getSensorType()) )
                    {
                        float w=it->convexVolume->getSmallestDistanceAllowed();
                        bool ww=it->convexVolume->getSmallestDistanceEnabled();
                        it->convexVolume->disableVolumeComputation(true);
                        // Volume parameters:
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
                    CMill* it=App::currentWorld->sceneObjects->getMillFromHandle(cmd.intParams[i]);
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
            App::currentWorld->mainSettings->millsEnabled=!App::currentWorld->mainSettings->millsEnabled;
        }
        if (cmd.cmdId==TOGGLE_EXPLICITHANDLING_MILLGUITRIGGEREDCMD)
        {
            CMill* it=App::currentWorld->sceneObjects->getMillFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setExplicitHandling(!it->getExplicitHandling());
        }
        if (cmd.cmdId==SET_SIZE_MILLGUITRIGGEREDCMD)
        {
            CMill* it=App::currentWorld->sceneObjects->getMillFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setSize(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_ENTITYTOCUT_MILLGUITRIGGEREDCMD)
        {
            CMill* it=App::currentWorld->sceneObjects->getMillFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setMillableObject(cmd.intParams[1]);
        }



        if (cmd.cmdId==TOGGLE_ENABLED_LIGHTGUITRIGGEREDCMD)
        {
            CLight* it=App::currentWorld->sceneObjects->getLightFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setLightActive(!it->getLightActive());
        }
        if (cmd.cmdId==TOGGLE_LIGHTISLOCAL_LIGHTGUITRIGGEREDCMD)
        {
            CLight* it=App::currentWorld->sceneObjects->getLightFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setLightIsLocal(!it->getLightIsLocal());
        }
        if (cmd.cmdId==SET_SIZE_LIGHTGUITRIGGEREDCMD)
        {
            CLight* it=App::currentWorld->sceneObjects->getLightFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setLightSize(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_SPOTCUTOFF_LIGHTGUITRIGGEREDCMD)
        {
            CLight* it=App::currentWorld->sceneObjects->getLightFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setSpotCutoffAngle(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_SPOTEXPONENT_LIGHTGUITRIGGEREDCMD)
        {
            CLight* it=App::currentWorld->sceneObjects->getLightFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setSpotExponent(cmd.intParams[1]);
        }
        if (cmd.cmdId==SET_CONSTATTENUATION_LIGHTGUITRIGGEREDCMD)
        {
            CLight* it=App::currentWorld->sceneObjects->getLightFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setAttenuationFactor(cmd.floatParams[0],CONSTANT_ATTENUATION);
        }
        if (cmd.cmdId==SET_LINATTENUATION_LIGHTGUITRIGGEREDCMD)
        {
            CLight* it=App::currentWorld->sceneObjects->getLightFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setAttenuationFactor(cmd.floatParams[0],LINEAR_ATTENUATION);
        }
        if (cmd.cmdId==SET_QUADATTENUATION_LIGHTGUITRIGGEREDCMD)
        {
            CLight* it=App::currentWorld->sceneObjects->getLightFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setAttenuationFactor(cmd.floatParams[0],QUADRATIC_ATTENUATION);
        }



        if (cmd.cmdId==SET_SIZE_DUMMYGUITRIGGEREDCMD)
        {
            CDummy* it=App::currentWorld->sceneObjects->getDummyFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setDummySize(cmd.floatParams[0]);
        }
        if (cmd.cmdId==APPLY_VISUALPROP_DUMMYGUITRIGGEREDCMD)
        {
            CDummy* it=App::currentWorld->sceneObjects->getDummyFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
            {
                for (size_t i=1;i<cmd.intParams.size();i++)
                {
                    CDummy* it2=App::currentWorld->sceneObjects->getDummyFromHandle(cmd.intParams[i]);
                    if (it2!=nullptr)
                    {
                        it->getDummyColor()->copyYourselfInto(it2->getDummyColor());
                        it2->setDummySize(it->getDummySize());
                    }
                }
            }
        }
        if (cmd.cmdId==SET_LINKEDDUMMY_DUMMYGUITRIGGEREDCMD)
        {
            CDummy* it=App::currentWorld->sceneObjects->getDummyFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
            {
                CDummy* it2=App::currentWorld->sceneObjects->getDummyFromHandle(cmd.intParams[1]);
                if (it2!=nullptr)
                    it->setLinkedDummyHandle(it2->getObjectHandle(),true);
                else
                    it->setLinkedDummyHandle(-1,true);
            }
        }
        if (cmd.cmdId==SET_LINKTYPE_DUMMYGUITRIGGEREDCMD)
        {
            CDummy* it=App::currentWorld->sceneObjects->getDummyFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setLinkType(cmd.intParams[1],true);
        }
        if (cmd.cmdId==TOGGLE_FOLLOWORIENTATION_DUMMYGUITRIGGEREDCMD)
        {
            CDummy* it=App::currentWorld->sceneObjects->getDummyFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setAssignedToParentPathOrientation(!it->getAssignedToParentPathOrientation());
        }
        if (cmd.cmdId==TOGGLE_FOLLOWPOSITION_DUMMYGUITRIGGEREDCMD)
        {
            CDummy* it=App::currentWorld->sceneObjects->getDummyFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setAssignedToParentPath(!it->getAssignedToParentPath());
        }
        if (cmd.cmdId==SET_FREEORFIXEDONPATH_DUMMYGUITRIGGEREDCMD)
        {
            CDummy* it=App::currentWorld->sceneObjects->getDummyFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setFreeOnPathTrajectory(cmd.intParams[1]!=0);
        }
        if (cmd.cmdId==SET_OFFSET_DUMMYGUITRIGGEREDCMD)
        {
            CDummy* it=App::currentWorld->sceneObjects->getDummyFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setVirtualDistanceOffsetOnPath(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_COPYINCREMENT_DUMMYGUITRIGGEREDCMD)
        {
            CDummy* it=App::currentWorld->sceneObjects->getDummyFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setVirtualDistanceOffsetOnPath_variationWhenCopy(cmd.floatParams[0]);
        }



        if (cmd.cmdId==TOGGLE_ENABLED_MIRRORGUITRIGGEREDCMD)
        {
            CMirror* it=App::currentWorld->sceneObjects->getMirrorFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setActive(!it->getActive());
        }
        if (cmd.cmdId==SET_WIDTH_MIRRORGUITRIGGEREDCMD)
        {
            CMirror* it=App::currentWorld->sceneObjects->getMirrorFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setMirrorWidth(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_HEIGHT_MIRRORGUITRIGGEREDCMD)
        {
            CMirror* it=App::currentWorld->sceneObjects->getMirrorFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setMirrorHeight(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_REFLECTANCE_MIRRORGUITRIGGEREDCMD)
        {
            CMirror* it=App::currentWorld->sceneObjects->getMirrorFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setReflectance(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_MIRRORFUNC_MIRRORGUITRIGGEREDCMD)
        {
            CMirror* it=App::currentWorld->sceneObjects->getMirrorFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setIsMirror(true);
        }
        if (cmd.cmdId==SET_CLIPPINGFUNC_MIRRORGUITRIGGEREDCMD)
        {
            CMirror* it=App::currentWorld->sceneObjects->getMirrorFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setIsMirror(false);
        }
        if (cmd.cmdId==SET_CLIPPINGENTITY_MIRRORGUITRIGGEREDCMD)
        {
            CMirror* it=App::currentWorld->sceneObjects->getMirrorFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setClippingObjectOrCollection(cmd.intParams[1]);
        }
        if (cmd.cmdId==TOGGLE_DISABLEALLCLIPPING_MIRRORGUITRIGGEREDCMD)
        {
            App::currentWorld->mainSettings->clippingPlanesDisabled=!App::currentWorld->mainSettings->clippingPlanesDisabled;
        }
        if (cmd.cmdId==TOGGLE_DISABLEALLMIRRORS_MIRRORGUITRIGGEREDCMD)
        {
            App::currentWorld->mainSettings->mirrorsDisabled=!App::currentWorld->mainSettings->mirrorsDisabled;
        }
        if (cmd.cmdId==SET_MAXTRIANGLESIZE_ENVIRONMENTGUITRIGGEREDCMD)
        {
            if (App::currentWorld->simulation->isSimulationStopped())
            {
                App::currentWorld->environment->setCalculationMaxTriangleSize(cmd.floatParams[0]);
                for (size_t i=0;i<App::currentWorld->sceneObjects->getShapeCount();i++)
                {
                    CShape* sh=App::currentWorld->sceneObjects->getShapeFromIndex(i);
                    sh->removeMeshCalculationStructure();
                }
            }
        }
        if (cmd.cmdId==SET_MINTRIANGLESIZE_ENVIRONMENTGUITRIGGEREDCMD)
        {
            App::currentWorld->environment->setCalculationMinRelTriangleSize(cmd.floatParams[0]);
            for (size_t i=0;i<App::currentWorld->sceneObjects->getShapeCount();i++)
            {
                CShape* sh=App::currentWorld->sceneObjects->getShapeFromIndex(i);
                sh->removeMeshCalculationStructure();
            }
        }
        if (cmd.cmdId==TOGGLE_SAVECALCSTRUCT_ENVIRONMENTGUITRIGGEREDCMD)
        {
            App::currentWorld->environment->setSaveExistingCalculationStructures(!App::currentWorld->environment->getSaveExistingCalculationStructures());
        }
        if (cmd.cmdId==TOGGLE_SHAPETEXTURES_ENVIRONMENTGUITRIGGEREDCMD)
        {
            App::currentWorld->environment->setShapeTexturesEnabled(!App::currentWorld->environment->getShapeTexturesEnabled());
        }
        if (cmd.cmdId==TOGGLE_GLUITEXTURES_ENVIRONMENTGUITRIGGEREDCMD)
        {
            App::currentWorld->environment->set2DElementTexturesEnabled(!App::currentWorld->environment->get2DElementTexturesEnabled());
        }
        if (cmd.cmdId==TOGGLE_LOCKSCENE_ENVIRONMENTGUITRIGGEREDCMD)
        {
            App::currentWorld->environment->setRequestFinalSave(!App::currentWorld->environment->getRequestFinalSave());
        }
        if (cmd.cmdId==SET_ACKNOWLEDGMENT_ENVIRONMENTGUITRIGGEREDCMD)
        {
            App::currentWorld->environment->setAcknowledgement(cmd.stringParams[0].c_str());
        }
        if (cmd.cmdId==CLEANUP_OBJNAMES_ENVIRONMENTGUITRIGGEREDCMD)
        {
            if (App::currentWorld->simulation->isSimulationStopped())
                App::currentWorld->cleanupHashNames_allObjects(-1);
        }
        if (cmd.cmdId==CLEANUP_GHOSTS_ENVIRONMENTGUITRIGGEREDCMD)
        {
            App::currentWorld->ghostObjectCont->removeGhost(-1,-1);
        }
        if (cmd.cmdId==SET_EXTSTRING_ENVIRONMENTGUITRIGGEREDCMD)
        {
            App::currentWorld->environment->setExtensionString(cmd.stringParams[0].c_str());
        }
        if (cmd.cmdId==TOGGLE_ENABLED_FOGGUITRIGGEREDCMD)
        {
            App::currentWorld->environment->setFogEnabled(!App::currentWorld->environment->getFogEnabled());
        }
        if (cmd.cmdId==SET_TYPE_FOGGUITRIGGEREDCMD)
        {
            App::currentWorld->environment->setFogType(cmd.intParams[0]);
        }
        if (cmd.cmdId==SET_START_FOGGUITRIGGEREDCMD)
        {
            App::currentWorld->environment->setFogStart(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_END_FOGGUITRIGGEREDCMD)
        {
            App::currentWorld->environment->setFogEnd(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_DENSITY_FOGGUITRIGGEREDCMD)
        {
            App::currentWorld->environment->setFogDensity(cmd.floatParams[0]);
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
            CColorObject* it=App::getVisualParamPointerFromItem(cmd.intParams[0],cmd.intParams[1],cmd.intParams[2],nullptr,&allowedParts);
            if ( (it!=nullptr)&&(allowedParts&64) )
                it->setShininess(tt::getLimitedInt(0,128,cmd.intParams[3]));
        }
        if (cmd.cmdId==TOGGLE_TRANSPARENCY_MATERIALGUITRIGGEREDCMD)
        {
            int allowedParts=0; // Bit-coded: 1=ambient/difuse, 2=diffuse(light only) 4=spec, 8=emiss., 16=aux channels, 32=pulsation, 64=shininess, 128=opacity, 256=colorName, 512=ext. string
            CColorObject* it=App::getVisualParamPointerFromItem(cmd.intParams[0],cmd.intParams[1],cmd.intParams[2],nullptr,&allowedParts);
            CShape* shape=App::currentWorld->sceneObjects->getShapeFromHandle(cmd.intParams[1]);
            if ( (it!=nullptr)&&(allowedParts&128)&&(shape!=nullptr) )
            {
                it->setTranslucid(!it->getTranslucid());
                shape->actualizeContainsTransparentComponent();
            }
        }
        if (cmd.cmdId==SET_TRANSPARENCYFACT_MATERIALGUITRIGGEREDCMD)
        {
            int allowedParts=0; // Bit-coded: 1=ambient/difuse, 2=diffuse(light only) 4=spec, 8=emiss., 16=aux channels, 32=pulsation, 64=shininess, 128=opacity, 256=colorName, 512=ext. string
            CColorObject* it=App::getVisualParamPointerFromItem(cmd.intParams[0],cmd.intParams[1],cmd.intParams[2],nullptr,&allowedParts);
            if ( (it!=nullptr)&&(allowedParts&128) )
                it->setOpacity(tt::getLimitedFloat(0.0,1.0,cmd.floatParams[0]));
        }
        if (cmd.cmdId==SET_NAME_MATERIALGUITRIGGEREDCMD)
        {
            int allowedParts=0; // Bit-coded: 1=ambient/difuse, 2=diffuse(light only) 4=spec, 8=emiss., 16=aux channels, 32=pulsation, 64=shininess, 128=opacity, 256=colorName, 512=ext. string
            CColorObject* it=App::getVisualParamPointerFromItem(cmd.intParams[0],cmd.intParams[1],cmd.intParams[2],nullptr,&allowedParts);
            if ( (it!=nullptr)&&(allowedParts&256) )
            {
                tt::removeIllegalCharacters(cmd.stringParams[0],false);
                it->setColorName(cmd.stringParams[0].c_str());
            }
        }
        if (cmd.cmdId==SET_EXTSTRING_MATERIALGUITRIGGEREDCMD)
        {
            int allowedParts=0; // Bit-coded: 1=ambient/difuse, 2=diffuse(light only) 4=spec, 8=emiss., 16=aux channels, 32=pulsation, 64=shininess, 128=opacity, 256=colorName, 512=ext. string
            CColorObject* it=App::getVisualParamPointerFromItem(cmd.intParams[0],cmd.intParams[1],cmd.intParams[2],nullptr,&allowedParts);
            if ( (it!=nullptr)&&(allowedParts&512) )
                it->setExtensionString(cmd.stringParams[0].c_str());
        }
        if (cmd.cmdId==SET_PULSATIONPARAMS_MATERIALGUITRIGGEREDCMD)
        {
            int allowedParts=0; // Bit-coded: 1=ambient/difuse, 2=diffuse(light only) 4=spec, 8=emiss., 16=aux channels, 32=pulsation, 64=shininess, 128=opacity, 256=colorName, 512=ext. string
            CColorObject* it=App::getVisualParamPointerFromItem(cmd.intParams[0],cmd.intParams[1],cmd.intParams[2],nullptr,&allowedParts);
            if ( (it!=nullptr)&&(allowedParts&32) )
            {
                it->setFlash(cmd.boolParams[0]);
                it->setUseSimulationTime(cmd.boolParams[1]);
                it->setFlashFrequency(cmd.floatParams[0]);
                it->setFlashPhase(cmd.floatParams[1]);
                it->setFlashRatio(cmd.floatParams[2]);
            }
        }



        if (cmd.cmdId==SET_VOXELSIZE_OCTREEGUITRIGGEREDCMD)
        {
            COctree* it=App::currentWorld->sceneObjects->getOctreeFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setCellSize(cmd.floatParams[0]);
        }
        if (cmd.cmdId==TOGGLE_SHOWSTRUCTURE_OCTREEGUITRIGGEREDCMD)
        {
            COctree* it=App::currentWorld->sceneObjects->getOctreeFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setShowOctree(!it->getShowOctree());
        }
        if (cmd.cmdId==TOGGLE_RANDOMCOLORS_OCTREEGUITRIGGEREDCMD)
        {
            COctree* it=App::currentWorld->sceneObjects->getOctreeFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setUseRandomColors(!it->getUseRandomColors());
        }
        if (cmd.cmdId==TOGGLE_SHOWPOINTS_OCTREEGUITRIGGEREDCMD)
        {
            COctree* it=App::currentWorld->sceneObjects->getOctreeFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setUsePointsInsteadOfCubes(!it->getUsePointsInsteadOfCubes());
        }
        if (cmd.cmdId==SET_POINTSIZE_OCTREEGUITRIGGEREDCMD)
        {
            COctree* it=App::currentWorld->sceneObjects->getOctreeFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setPointSize(cmd.intParams[1]);
        }
        if (cmd.cmdId==CLEAR_OCTREEGUITRIGGEREDCMD)
        {
            COctree* it=App::currentWorld->sceneObjects->getOctreeFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->clear();
        }
        if (cmd.cmdId==INSERT_SELECTEDVISIBLEOBJECTS_OCTREEGUITRIGGEREDCMD)
        {
            COctree* it=App::currentWorld->sceneObjects->getOctreeFromHandle(cmd.intParams[0]);
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
            COctree* it=App::currentWorld->sceneObjects->getOctreeFromHandle(cmd.intParams[0]);
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
            COctree* it=App::currentWorld->sceneObjects->getOctreeFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setColorIsEmissive(!it->getColorIsEmissive());
        }




        if (cmd.cmdId==SET_MAXVOXELSIZE_PTCLOUDGUITRIGGEREDCMD)
        {
            CPointCloud* it=App::currentWorld->sceneObjects->getPointCloudFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setCellSize(cmd.floatParams[0]);
        }
        if (cmd.cmdId==TOGGLE_SHOWOCTREE_PTCLOUDGUITRIGGEREDCMD)
        {
            CPointCloud* it=App::currentWorld->sceneObjects->getPointCloudFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setShowOctree(!it->getShowOctree());
        }
        if (cmd.cmdId==TOGGLE_RANDOMCOLORS_PTCLOUDGUITRIGGEREDCMD)
        {
            CPointCloud* it=App::currentWorld->sceneObjects->getPointCloudFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setUseRandomColors(!it->getUseRandomColors());
        }
        if (cmd.cmdId==SET_PTSIZE_PTCLOUDGUITRIGGEREDCMD)
        {
            CPointCloud* it=App::currentWorld->sceneObjects->getPointCloudFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setPointSize(cmd.intParams[1]);
        }
        if (cmd.cmdId==CLEAR_PTCLOUDGUITRIGGEREDCMD)
        {
            CPointCloud* it=App::currentWorld->sceneObjects->getPointCloudFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->clear();
        }
        if (cmd.cmdId==INSERT_OBJECTS_PTCLOUDGUITRIGGEREDCMD)
        {
            CPointCloud* it=App::currentWorld->sceneObjects->getPointCloudFromHandle(cmd.intParams[0]);
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
            CPointCloud* it=App::currentWorld->sceneObjects->getPointCloudFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setMaxPointCountPerCell(cmd.intParams[1]);
        }
        if (cmd.cmdId==SET_BUILDRESOLUTION_PTCLOUDGUITRIGGEREDCMD)
        {
            CPointCloud* it=App::currentWorld->sceneObjects->getPointCloudFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setBuildResolution(cmd.floatParams[0]);
        }
        if (cmd.cmdId==TOGGLE_USEOCTREE_PTCLOUDGUITRIGGEREDCMD)
        {
            CPointCloud* it=App::currentWorld->sceneObjects->getPointCloudFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setDoNotUseCalculationStructure(!it->getDoNotUseCalculationStructure());
        }
        if (cmd.cmdId==TOGGLE_EMISSIVECOLOR_PTCLOUDGUITRIGGEREDCMD)
        {
            CPointCloud* it=App::currentWorld->sceneObjects->getPointCloudFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setColorIsEmissive(!it->getColorIsEmissive());
        }
        if (cmd.cmdId==SET_DISPLAYRATIO_PTCLOUDGUITRIGGEREDCMD)
        {
            CPointCloud* it=App::currentWorld->sceneObjects->getPointCloudFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setPointDisplayRatio(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SUBTRACT_OBJECTS_PTCLOUDGUITRIGGEREDCMD)
        {
            CPointCloud* it=App::currentWorld->sceneObjects->getPointCloudFromHandle(cmd.intParams[0]);
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
            CPointCloud* it=App::currentWorld->sceneObjects->getPointCloudFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setRemovalDistanceTolerance(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_INSERTTOLERANCE_PTCLOUDGUITRIGGEREDCMD)
        {
            CPointCloud* it=App::currentWorld->sceneObjects->getPointCloudFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setInsertionDistanceTolerance(cmd.floatParams[0]);
        }



        if (cmd.cmdId==SET_SIZE_FORCESENSORGUITRIGGEREDCMD)
        {
            CForceSensor* it=App::currentWorld->sceneObjects->getForceSensorFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setForceSensorSize(cmd.floatParams[0]);
        }
        if (cmd.cmdId==APPLY_VISUALPROP_FORCESENSORGUITRIGGEREDCMD)
        {
            CForceSensor* it=App::currentWorld->sceneObjects->getForceSensorFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
            {
                for (size_t i=1;i<cmd.intParams.size();i++)
                {
                    CForceSensor* it2=App::currentWorld->sceneObjects->getForceSensorFromHandle(cmd.intParams[i]);
                    if (it2!=nullptr)
                    {
                        it->getColor(false)->copyYourselfInto(it2->getColor(false));
                        it->getColor(true)->copyYourselfInto(it2->getColor(true));
                        it2->setForceSensorSize(it->getForceSensorSize());
                    }
                }
            }
        }
        if (cmd.cmdId==SET_SAMPLESIZE_FORCESENSORGUITRIGGEREDCMD)
        {
            CForceSensor* it=App::currentWorld->sceneObjects->getForceSensorFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setValueCountForFilter(cmd.intParams[1]);
        }
        if (cmd.cmdId==SET_AVERAGEVALUE_FORCESENSORGUITRIGGEREDCMD)
        {
            CForceSensor* it=App::currentWorld->sceneObjects->getForceSensorFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setFilterType(0);
        }
        if (cmd.cmdId==SET_MEDIANVALUE_FORCESENSORGUITRIGGEREDCMD)
        {
            CForceSensor* it=App::currentWorld->sceneObjects->getForceSensorFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setFilterType(1);
        }
        if (cmd.cmdId==APPLY_FILER_FORCESENSORGUITRIGGEREDCMD)
        {
            CForceSensor* it=App::currentWorld->sceneObjects->getForceSensorFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
            {
                for (size_t i=1;i<cmd.intParams.size();i++)
                {
                    CForceSensor* it2=App::currentWorld->sceneObjects->getForceSensorFromHandle(cmd.intParams[i]);
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
            CForceSensor* it=App::currentWorld->sceneObjects->getForceSensorFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setEnableForceThreshold(!it->getEnableForceThreshold());
        }
        if (cmd.cmdId==TOGGLE_TORQUETHRESHOLDENABLE_FORCESENSORGUITRIGGEREDCMD)
        {
            CForceSensor* it=App::currentWorld->sceneObjects->getForceSensorFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setEnableTorqueThreshold(!it->getEnableTorqueThreshold());
        }
        if (cmd.cmdId==SET_FORCETHRESHOLD_FORCESENSORGUITRIGGEREDCMD)
        {
            CForceSensor* it=App::currentWorld->sceneObjects->getForceSensorFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setForceThreshold(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_TORQUETHRESHOLD_FORCESENSORGUITRIGGEREDCMD)
        {
            CForceSensor* it=App::currentWorld->sceneObjects->getForceSensorFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setTorqueThreshold(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_CONSECTHRESHOLDVIOLATIONS_FORCESENSORGUITRIGGEREDCMD)
        {
            CForceSensor* it=App::currentWorld->sceneObjects->getForceSensorFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setConsecutiveThresholdViolationsForBreaking(cmd.intParams[1]);
        }
        if (cmd.cmdId==APPLY_BREAKING_FORCESENSORGUITRIGGEREDCMD)
        {
            CForceSensor* it=App::currentWorld->sceneObjects->getForceSensorFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
            {
                for (size_t i=1;i<cmd.intParams.size();i++)
                {
                    CForceSensor* it2=App::currentWorld->sceneObjects->getForceSensorFromHandle(cmd.intParams[i]);
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
            CShape* it=App::currentWorld->sceneObjects->getShapeFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setCulling(!it->getCulling());
        }
        if (cmd.cmdId==TOGGLE_WIREFRAME_SHAPEGUITRIGGEREDCMD)
        {
            CShape* it=App::currentWorld->sceneObjects->getShapeFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setShapeWireframe_OLD(!it->getShapeWireframe_OLD());
        }
        if (cmd.cmdId==INVERT_FACES_SHAPEGUITRIGGEREDCMD)
        {
            CShape* it=App::currentWorld->sceneObjects->getShapeFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->invertFrontBack();
        }
        if (cmd.cmdId==TOGGLE_SHOWEDGES_SHAPEGUITRIGGEREDCMD)
        {
            CShape* it=App::currentWorld->sceneObjects->getShapeFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setVisibleEdges(!it->getVisibleEdges());
        }
        if (cmd.cmdId==SET_SHADINGANGLE_SHAPEGUITRIGGEREDCMD)
        {
            CShape* it=App::currentWorld->sceneObjects->getShapeFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setShadingAngle(cmd.floatParams[0]);
        }
        if (cmd.cmdId==APPLY_OTHERPROP_SHAPEGUITRIGGEREDCMD)
        {
            CShape* last=App::currentWorld->sceneObjects->getShapeFromHandle(cmd.intParams[0]);
            if ((last!=nullptr)&&(!last->isCompound()))
            {
                for (size_t i=1;i<cmd.intParams.size();i++)
                {
                    CShape* it=App::currentWorld->sceneObjects->getShapeFromHandle(cmd.intParams[i]);
                    if ((it!=nullptr)&&(!it->isCompound()))
                    {
                        it->getSingleMesh()->setVisibleEdges(last->getSingleMesh()->getVisibleEdges());
                        it->getSingleMesh()->setCulling(last->getSingleMesh()->getCulling());
                        it->getSingleMesh()->setInsideAndOutsideFacesSameColor_DEPRECATED(last->getSingleMesh()->getInsideAndOutsideFacesSameColor_DEPRECATED());
                        it->getSingleMesh()->setEdgeWidth_DEPRECATED(last->getSingleMesh()->getEdgeWidth_DEPRECATED());
                        it->getSingleMesh()->setWireframe_OLD(last->getSingleMesh()->getWireframe_OLD());
                        it->getSingleMesh()->setShadingAngle(last->getSingleMesh()->getShadingAngle());
                        it->getSingleMesh()->setEdgeThresholdAngle(last->getSingleMesh()->getEdgeThresholdAngle());
                        it->getSingleMesh()->setHideEdgeBorders_OLD(last->getSingleMesh()->getHideEdgeBorders_OLD());
                        it->getSingleMesh()->setDisplayInverted_DEPRECATED(last->getSingleMesh()->getDisplayInverted_DEPRECATED());
                    }
                }
            }
        }
        if (cmd.cmdId==APPLY_VISUALPROP_SHAPEGUITRIGGEREDCMD)
        {
            CShape* last=App::currentWorld->sceneObjects->getShapeFromHandle(cmd.intParams[0]);
            if ((last!=nullptr)&&(!last->isCompound()))
            {
                for (size_t i=1;i<cmd.intParams.size();i++)
                {
                    CShape* it=App::currentWorld->sceneObjects->getShapeFromHandle(cmd.intParams[i]);
                    if ((it!=nullptr)&&(!it->isCompound()))
                    {
                        last->getSingleMesh()->color.copyYourselfInto(&it->getSingleMesh()->color);
                        last->getSingleMesh()->insideColor_DEPRECATED.copyYourselfInto(&it->getSingleMesh()->insideColor_DEPRECATED);
                        last->getSingleMesh()->edgeColor_DEPRECATED.copyYourselfInto(&it->getSingleMesh()->edgeColor_DEPRECATED);
                        it->actualizeContainsTransparentComponent();
                    }
                }
            }
        }
        if (cmd.cmdId==TOGGLE_HIDEEDGEBORDERS_SHAPEGUITRIGGEREDCMD)
        {
            CShape* it=App::currentWorld->sceneObjects->getShapeFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setHideEdgeBorders_OLD(!it->getHideEdgeBorders_OLD());
        }
        if (cmd.cmdId==CLEAR_TEXTURES_SHAPEGUITRIGGEREDCMD)
        {
            for (size_t i=0;i<cmd.intParams.size();i++)
            {
                CShape* shape=App::currentWorld->sceneObjects->getShapeFromHandle(cmd.intParams[i]);
                if (shape!=nullptr)
                {
                    std::vector<CMesh*> components;
                    shape->getMeshWrapper()->getAllShapeComponentsCumulative(components);
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
                                    shape->getSingleMesh()->textureCoords_notCopiedNorSerialized.assign(tc->begin(),tc->end());
                                    keepTextCoords=true;
                                }
                            }
                            App::currentWorld->textureContainer->announceGeneralObjectWillBeErased(shape->getObjectHandle(),-1);
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
                CShape* shape=App::currentWorld->sceneObjects->getShapeFromHandle(cmd.intParams[i]);
                if (shape!=nullptr)
                {
                    shapeList.push_back(shape);
                    std::vector<CMesh*> components;
                    shape->getMeshWrapper()->getAllShapeComponentsCumulative(components);
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
                                    shape->getSingleMesh()->textureCoords_notCopiedNorSerialized.assign(tc->begin(),tc->end());
                                }
                            }
                            App::currentWorld->textureContainer->announceGeneralObjectWillBeErased(shape->getObjectHandle(),-1);
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
                    std::vector<CMesh*> components;
                    shape->getMeshWrapper()->getAllShapeComponentsCumulative(components);
                    for (size_t j=0;j<components.size();j++)
                    {
                        CMesh* geom=components[j];
                        textureObj->addDependentObject(shape->getObjectHandle(),geom->getUniqueID());
                    }
                }

                int textureID=App::currentWorld->textureContainer->addObject(textureObj,false); // might erase the textureObj and return a similar object already present!!

                for (size_t i=0;i<shapeList.size();i++)
                {
                    CShape* shape=shapeList[i];
                    C3Vector bbhs(shape->getBoundingBoxHalfSizes());
                    float s=std::max<float>(std::max<float>(bbhs(0),bbhs(1)),bbhs(2))*2.0f;
                    std::vector<CMesh*> components;
                    shape->getMeshWrapper()->getAllShapeComponentsCumulative(components);
                    for (size_t j=0;j<components.size();j++)
                    {
                        CMesh* geom=components[j];
                        CTextureProperty* tp=new CTextureProperty(textureID);
                        bool useTexCoords=false;
                        if (!shape->isCompound())
                        {
                            if (shape->getSingleMesh()->textureCoords_notCopiedNorSerialized.size()!=0)
                            {
                                std::vector<float> wvert;
                                std::vector<int> wind;
                                shape->getSingleMesh()->getCumulativeMeshes(wvert,&wind,nullptr);
                                if (shape->getSingleMesh()->textureCoords_notCopiedNorSerialized.size()/2==wind.size())
                                { // we have texture coordinate data attached to the shape's geometry (was added during shape import)
                                    tp->setFixedCoordinates(&shape->getSingleMesh()->textureCoords_notCopiedNorSerialized);
                                    shape->getSingleMesh()->textureCoords_notCopiedNorSerialized.clear();
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

        if (cmd.cmdId==APPLY_SIZE_GEOMETRYGUITRIGGEREDCMD)
        {
            CShape* it=App::currentWorld->sceneObjects->getShapeFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
            {
                C3Vector bbhalfSizes(it->getBoundingBoxHalfSizes());
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
                it->scaleMesh(s[0],s[1],s[2]);
            }
        }
        if (cmd.cmdId==APPLY_SCALING_GEOMETRYGUITRIGGEREDCMD)
        {
            CShape* it=App::currentWorld->sceneObjects->getShapeFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->scaleMesh(cmd.floatParams[0],cmd.floatParams[1],cmd.floatParams[2]);
        }
        if (cmd.cmdId==APPLY_FRAMEROTATION_GEOMETRYGUITRIGGEREDCMD)
        {
            CShape* it=App::currentWorld->sceneObjects->getShapeFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
            {
                C7Vector localBefore(it->getLocalTransformation());
                C7Vector parentCumul(it->getFullParentCumulativeTransformation());
                C7Vector tr;
                tr.setIdentity();
                tr.Q.setEulerAngles(cmd.floatParams[0],cmd.floatParams[1],cmd.floatParams[2]);
                it->setLocalTransformation(parentCumul.getInverse()*tr.getInverse());
                it->alignBoundingBoxWithWorld();
                it->setLocalTransformation(localBefore*tr*parentCumul*it->getFullLocalTransformation());
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
            CMesh* geom=nullptr;
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
                        CButtonBlock* block=App::currentWorld->buttonBlockContainer->getBlockWithID(cmd.intParams[1]);
                        CSoftButton* butt=block->getButtonWithID(cmd.intParams[2]);
                        textureObj->addDependentObject(cmd.intParams[1],butt->getUniqueID()); // Unique ID starts exceptionnally at 1
                    }
                }
                int textureID=App::currentWorld->textureContainer->addObject(textureObj,false); // might erase the textureObj and return a similar object already present!!
                CTextureProperty* tp=new CTextureProperty(textureID);
                if (geom!=nullptr)
                {
                    // Following 2 since 12/6/2011 because now by default we have the modulate mode (non-decal)
            //      ((CMesh*)shape->geomInfo)->color.setColor(0.5f,0.5f,0.5f,sim_colorcomponent_ambient_diffuse);
            //      ((CMesh*)shape->geomInfo)->insideColor.setColor(0.5f,0.5f,0.5f,sim_colorcomponent_ambient_diffuse);
                    geom->setTextureProperty(tp);
                    std::vector<float> wvert;
                    std::vector<int> wind;
                    geom->getCumulativeMeshes(wvert,&wind,nullptr);
                    if (geom->textureCoords_notCopiedNorSerialized.size()/2==wind.size())
                    { // we have texture coordinate data attached to the shape's geometry (was added during shape import)
                        App::uiThread->messageBox_information(App::mainWindow,"Texture coordinates",IDS_USING_EXISTING_TEXTURE_COORDINATES,VMESSAGEBOX_OKELI,VMESSAGEBOX_REPLY_OK);
                        tp->setFixedCoordinates(&geom->textureCoords_notCopiedNorSerialized);
                        geom->textureCoords_notCopiedNorSerialized.clear();
                    }
                }
                else
                {
                    tp->setApplyMode(1); // 13/1/2012
                    CButtonBlock* block=App::currentWorld->buttonBlockContainer->getBlockWithID(cmd.intParams[1]);
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
            CMesh* geom=nullptr;
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
                        App::currentWorld->textureContainer->announceGeneralObjectWillBeErased(cmd.intParams[1],geom->getUniqueID());
                        delete tp;
                        geom->setTextureProperty(nullptr);
                    }
                    else
                    { // add an existing texture
                        if (tObject!=-1)
                        {
                            if ((tObject>=SIM_IDSTART_TEXTURE)&&(tObject<=SIM_IDEND_TEXTURE))
                            {
                                CTextureObject* to=App::currentWorld->textureContainer->getObject(tObject);
                                to->addDependentObject(cmd.intParams[1],geom->getUniqueID());
                            }
                            tp=new CTextureProperty(tObject);
                            geom->setTextureProperty(tp);
                            // Following 2 since 12/6/2011 because now by default we have the modulate mode (non-decal)
                        //  ((CMesh*)shape->geomInfo)->color.setColor(0.5f,0.5f,0.5f,sim_colorcomponent_ambient_diffuse);
                        //  ((CMesh*)shape->geomInfo)->insideColor.setColor(0.5f,0.5f,0.5f,sim_colorcomponent_ambient_diffuse);

                            std::vector<float> wvert;
                            std::vector<int> wind;
                            geom->getCumulativeMeshes(wvert,&wind,nullptr);

                            if (geom->textureCoords_notCopiedNorSerialized.size()/2==wind.size())
                            { // we have texture coordinate data attached to the shape's geometry (was added during shape import)
                                App::uiThread->messageBox_information(App::mainWindow,"Texture coordinates",IDS_USING_EXISTING_TEXTURE_COORDINATES,VMESSAGEBOX_OKELI,VMESSAGEBOX_REPLY_OK);
                                tp->setFixedCoordinates(&geom->textureCoords_notCopiedNorSerialized);
                                geom->textureCoords_notCopiedNorSerialized.clear();
                            }
                        }
                    }
                }
                if (cmd.intParams[0]==TEXTURE_ID_OPENGL_GUI_BACKGROUND)
                {
                    CButtonBlock* block=App::currentWorld->buttonBlockContainer->getBlockWithID(cmd.intParams[1]);
                    if (tp!=nullptr)
                    { // remove the texture
                        App::currentWorld->textureContainer->announceGeneralObjectWillBeErased(cmd.intParams[1],0);
                        delete tp;
                        block->setTextureProperty(nullptr);
                    }
                    else
                    { // add an existing texture
                        if (tObject!=-1)
                        {
                            if ((tObject>=SIM_IDSTART_TEXTURE)&&(tObject<=SIM_IDEND_TEXTURE))
                            {
                                CTextureObject* to=App::currentWorld->textureContainer->getObject(tObject);
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
                    CButtonBlock* block=App::currentWorld->buttonBlockContainer->getBlockWithID(cmd.intParams[1]);
                    CSoftButton* button=block->getButtonWithID(cmd.intParams[2]);
                    if (tp!=nullptr)
                    { // remove the texture
                        App::currentWorld->textureContainer->announceGeneralObjectWillBeErased(cmd.intParams[1],cmd.intParams[2]);
                        delete tp;
                        button->setTextureProperty(nullptr);
                    }
                    else
                    { // add an existing texture
                        if (tObject!=-1)
                        {
                            if ((tObject>=SIM_IDSTART_TEXTURE)&&(tObject<=SIM_IDEND_TEXTURE))
                            {
                                CTextureObject* to=App::currentWorld->textureContainer->getObject(tObject);
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
            CShape* it=App::currentWorld->sceneObjects->getShapeFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
            {
                std::vector<CMesh*> geoms;
                it->getMeshWrapper()->getAllShapeComponentsCumulative(geoms);
                int index=cmd.intParams[1];
                if ((index>=0)&&(index<int(geoms.size())))
                {
                    CMesh* geom=geoms[index];
                    geom->setCulling(!geom->getCulling());
                }
            }
        }
        if (cmd.cmdId==TOGGLE_SHOWEDGES_MULTISHAPEEDITIONGUITRIGGEREDCMD)
        {
            CShape* it=App::currentWorld->sceneObjects->getShapeFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
            {
                std::vector<CMesh*> geoms;
                it->getMeshWrapper()->getAllShapeComponentsCumulative(geoms);
                int index=cmd.intParams[1];
                if ((index>=0)&&(index<int(geoms.size())))
                {
                    CMesh* geom=geoms[index];
                    geom->setVisibleEdges(!geom->getVisibleEdges());
                }
            }
        }
        if (cmd.cmdId==SET_SHADINGANGLE_MULTISHAPEEDITIONGUITRIGGEREDCMD)
        {
            CShape* it=App::currentWorld->sceneObjects->getShapeFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
            {
                std::vector<CMesh*> geoms;
                it->getMeshWrapper()->getAllShapeComponentsCumulative(geoms);
                int index=cmd.intParams[1];
                if ((index>=0)&&(index<int(geoms.size())))
                {
                    CMesh* geom=geoms[index];
                    geom->setShadingAngle(cmd.floatParams[0]);
                }
            }
        }

        if (cmd.cmdId==TOGGLE_LAYER_LAYERGUITRIGGEREDCMD)
        {
            App::currentWorld->environment->setActiveLayers(App::currentWorld->environment->getActiveLayers()^cmd.intParams[0]);
        }
        if (cmd.cmdId==TOGGLE_SHOWDYNCONTENT_LAYERGUITRIGGEREDCMD)
        {
            App::currentWorld->simulation->setDynamicContentVisualizationOnly(!App::currentWorld->simulation->getDynamicContentVisualizationOnly());
        }



        if (cmd.cmdId==SET_ROLLEDUPSIZES_ROLLEDUPGUITRIGGEREDCMD)
        {
            CButtonBlock* it=App::currentWorld->buttonBlockContainer->getBlockWithID(cmd.intParams[0]);
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
            CJoint* it=App::currentWorld->sceneObjects->getJointFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setDependencyJointOffset(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_MULTFACT_JOINTDEPENDENCYGUITRIGGEREDCMD)
        {
            CJoint* it=App::currentWorld->sceneObjects->getJointFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setDependencyJointMult(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_OTHERJOINT_JOINTDEPENDENCYGUITRIGGEREDCMD)
        {
            CJoint* it=App::currentWorld->sceneObjects->getJointFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setDependencyMasterJointHandle(cmd.intParams[1]);
        }


        if (cmd.cmdId==TOGGLE_CYCLIC_JOINTGUITRIGGEREDCMD)
        {
            CJoint* it=App::currentWorld->sceneObjects->getJointFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setIsCyclic(!it->getIsCyclic());
        }
        if (cmd.cmdId==SET_PITCH_JOINTGUITRIGGEREDCMD)
        {
            CJoint* it=App::currentWorld->sceneObjects->getJointFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setScrewPitch(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_MINPOS_JOINTGUITRIGGEREDCMD)
        {
            CJoint* it=App::currentWorld->sceneObjects->getJointFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setPositionMin(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_RANGE_JOINTGUITRIGGEREDCMD)
        {
            CJoint* it=App::currentWorld->sceneObjects->getJointFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setPositionRange(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_POS_JOINTGUITRIGGEREDCMD)
        {
            CJoint* it=App::currentWorld->sceneObjects->getJointFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
            {
                it->setPosition(cmd.floatParams[0],false);
                it->setTargetPosition(cmd.floatParams[0]);
            }
        }
        if (cmd.cmdId==APPLY_CONFIGPARAMS_JOINTGUITRIGGEREDCMD)
        {
            CJoint* last=App::currentWorld->sceneObjects->getJointFromHandle(cmd.intParams[0]);
            if (last!=nullptr)
            {
                for (size_t i=1;i<cmd.intParams.size();i++)
                {
                    CJoint* it=App::currentWorld->sceneObjects->getJointFromHandle(cmd.intParams[i]);
                    if ( (it!=nullptr)&&(last->getJointType()==it->getJointType()) )
                    {
                        it->setIsCyclic(last->getIsCyclic());
                        it->setPositionRange(last->getPositionRange());
                        it->setPositionMin(last->getPositionMin());
                        it->setPosition(last->getPosition(),false);
                        it->setSphericalTransformation(last->getSphericalTransformation());
                        it->setScrewPitch(last->getScrewPitch());
                        it->setIKWeight_old(last->getIKWeight_old());
                        it->setMaxStepSize_old(last->getMaxStepSize_old());
                    }
                }
            }
        }
        if (cmd.cmdId==SET_MODE_JOINTGUITRIGGEREDCMD)
        {
            CJoint* it=App::currentWorld->sceneObjects->getJointFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
            {
                if ((cmd.intParams[1]&sim_jointmode_hybrid_deprecated)!=0)
                {
                    it->setJointMode(cmd.intParams[1]-sim_jointmode_hybrid_deprecated);
                    it->setHybridFunctionality_old(true);
                }
                else
                {
                    it->setJointMode(cmd.intParams[1]);
                    it->setHybridFunctionality_old(false);
                }
            }
        }
        if (cmd.cmdId==APPLY_MODEPARAMS_JOINTGUITRIGGEREDCMD)
        {
            CJoint* last=App::currentWorld->sceneObjects->getJointFromHandle(cmd.intParams[0]);
            if (last!=nullptr)
            {
                for (size_t i=1;i<cmd.intParams.size();i++)
                {
                    CJoint* it=App::currentWorld->sceneObjects->getJointFromHandle(cmd.intParams[i]);
                    if ( (it!=nullptr)&&(last->getJointType()==it->getJointType()) )
                    {
                        it->setJointMode(last->getJointMode());
                        it->setHybridFunctionality_old(last->getHybridFunctionality_old());
                    }
                }
            }
        }
        if (cmd.cmdId==APPLY_VISUALPARAMS_JOINTGUITRIGGEREDCMD)
        {
            CJoint* last=App::currentWorld->sceneObjects->getJointFromHandle(cmd.intParams[0]);
            if (last!=nullptr)
            {
                for (size_t i=1;i<cmd.intParams.size();i++)
                {
                    CJoint* it=App::currentWorld->sceneObjects->getJointFromHandle(cmd.intParams[i]);
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
            CJoint* it=App::currentWorld->sceneObjects->getJointFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setLength(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_DIAMETER_JOINTGUITRIGGEREDCMD)
        {
            CJoint* it=App::currentWorld->sceneObjects->getJointFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setDiameter(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_TARGETVELOCITY_JOINTDYNGUITRIGGEREDCMD)
        {
            CJoint* it=App::currentWorld->sceneObjects->getJointFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setTargetVelocity(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_FORCE_JOINTDYNGUITRIGGEREDCMD)
        {
            CJoint* it=App::currentWorld->sceneObjects->getJointFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setTargetForce(cmd.floatParams[0],true);
        }
        if (cmd.cmdId==SET_MAXFORCE_JOINTDYNGUITRIGGEREDCMD)
        {
            CJoint* it=App::currentWorld->sceneObjects->getJointFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setTargetForce(cmd.floatParams[0],false);
        }
        if (cmd.cmdId==APPLY_PARAMS_JOINTDYNGUITRIGGEREDCMD)
        {
            CJoint* last=App::currentWorld->sceneObjects->getJointFromHandle(cmd.intParams[0]);
            if ( (last!=nullptr)&&((last->getJointMode()==sim_jointmode_dynamic)||last->getHybridFunctionality_old()) )
            {
                for (size_t i=1;i<cmd.intParams.size();i++)
                {
                    CJoint* it=App::currentWorld->sceneObjects->getJointFromHandle(cmd.intParams[i]);
                    if ( (it!=nullptr)&&(last->getJointType()==it->getJointType())&&((it->getJointMode()==last->getJointMode())||(last->getHybridFunctionality_old()&&it->getHybridFunctionality_old())) )
                    {
                        it->setDynCtrlMode(last->getDynCtrlMode());
                        it->setDynPosCtrlType(last->getDynPosCtrlType());
                        it->setTargetForce(last->getTargetForce(true),true);
                        it->setTargetVelocity(last->getTargetVelocity());
                        it->setMotorLock(last->getMotorLock());
                        it->setTargetPosition(last->getTargetPosition());
                        float maxVelAccelJerk[3];
                        last->getMaxVelAccelJerk(maxVelAccelJerk);
                        it->setMaxVelAccelJerk(maxVelAccelJerk);
                        float pp,ip,dp;
                        last->getPid(pp,ip,dp);
                        it->setPid(pp,ip,dp);
                        float kp,cp;
                        last->getKc(kp,cp);
                        it->setKc(kp,cp);
                        last->copyEnginePropertiesTo(it);
                    }
                }
            }
        }
        if (cmd.cmdId==SET_MOTIONPROFILEVALS_JOINTDYNGUITRIGGEREDCMD)
        {
            CJoint* it=App::currentWorld->sceneObjects->getJointFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
            {
                float maxVelAccelJerk[3];
                it->getMaxVelAccelJerk(maxVelAccelJerk);
                maxVelAccelJerk[cmd.intParams[1]]=cmd.floatParams[0];
                it->setMaxVelAccelJerk(maxVelAccelJerk);
            }
        }
        if (cmd.cmdId==SET_TARGETPOSITION_JOINTDYNGUITRIGGEREDCMD)
        {
            CJoint* it=App::currentWorld->sceneObjects->getJointFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setTargetPosition(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_PIDVALUES_JOINTDYNGUITRIGGEREDCMD)
        {
            CJoint* it=App::currentWorld->sceneObjects->getJointFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setPid(cmd.floatParams[0],cmd.floatParams[1],cmd.floatParams[2]);
        }
        if (cmd.cmdId==SET_KCVALUES_JOINTDYNGUITRIGGEREDCMD)
        {
            CJoint* it=App::currentWorld->sceneObjects->getJointFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setKc(cmd.floatParams[0],cmd.floatParams[1]);
        }
        if (cmd.cmdId==SET_JOINTCTRLMODE_JOINTDYNGUITRIGGEREDCMD)
        {
            CJoint* it=App::currentWorld->sceneObjects->getJointFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setDynCtrlMode(cmd.intParams[1]);
        }
        if (cmd.cmdId==SET_JOINTPOSCTRLMODETYPE_JOINTDYNGUITRIGGEREDCMD)
        {
            CJoint* it=App::currentWorld->sceneObjects->getJointFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setDynPosCtrlType(cmd.intParams[1]);
        }
        if (cmd.cmdId==TOGGLE_JOINTVELCTRLMODETYPE_JOINTDYNGUITRIGGEREDCMD)
        {
            CJoint* it=App::currentWorld->sceneObjects->getJointFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
            {
                if (it->getDynVelCtrlType()==0)
                    it->setDynVelCtrlType(1);
                else
                    it->setDynVelCtrlType(0);
            }
        }

        if (cmd.cmdId==TOGGLE_LOCKMOTOR_JOINTDYNGUITRIGGEREDCMD)
        {
            CJoint* it=App::currentWorld->sceneObjects->getJointFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setMotorLock(!it->getMotorLock());
        }
        if (cmd.cmdId==SET_ALLENGINEPARAMS_JOINTDYNGUITRIGGEREDCMD)
        {
            CJoint* it=App::currentWorld->sceneObjects->getJointFromHandle(cmd.intParams[0]);
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
            CJoint* it=App::currentWorld->sceneObjects->getJointFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
            {
                for (size_t i=1;i<cmd.intParams.size();i++)
                {
                    CJoint* anotherJoint=App::currentWorld->sceneObjects->getJointFromHandle(cmd.intParams[i]);
                    if (anotherJoint!=nullptr)
                        it->copyEnginePropertiesTo(anotherJoint);
                }
            }
        }



        if (cmd.cmdId==SET_ATTRIBUTES_PATH_OLD_GUITRIGGEREDCMD)
        {
            CPath_old* it=App::currentWorld->sceneObjects->getPathFromHandle(cmd.intParams[0]);
            if ((it!=nullptr)&&(it->pathContainer!=nullptr))
                it->pathContainer->setAttributes(cmd.intParams[1]);
        }
        if (cmd.cmdId==SET_LINESIZE_PATH_OLD_GUITRIGGEREDCMD)
        {
            CPath_old* it=App::currentWorld->sceneObjects->getPathFromHandle(cmd.intParams[0]);
            if ((it!=nullptr)&&(it->pathContainer!=nullptr))
                it->pathContainer->setLineSize(cmd.intParams[1]);
        }
        if (cmd.cmdId==SET_CTRLPTSIZE_PATH_OLD_GUITRIGGEREDCMD)
        {
            CPath_old* it=App::currentWorld->sceneObjects->getPathFromHandle(cmd.intParams[0]);
            if ((it!=nullptr)&&(it->pathContainer!=nullptr))
                it->pathContainer->setSquareSize(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_DISTANCEUNIT_PATH_OLD_GUITRIGGEREDCMD)
        {
            CPath_old* it=App::currentWorld->sceneObjects->getPathFromHandle(cmd.intParams[0]);
            if ((it!=nullptr)&&(it->pathContainer!=nullptr))
                it->pathContainer->setPathLengthCalculationMethod(cmd.intParams[1]);
        }
        if (cmd.cmdId==COPY_TO_CLIPBOARD_PATH_OLD_GUITRIGGEREDCMD)
        {
            CPath_old* it=App::currentWorld->sceneObjects->getPathFromHandle(cmd.intParams[0]);
            if ((it!=nullptr)&&(it->pathContainer!=nullptr))
            {
                it->pathContainer->copyPointsToClipboard();
                App::logMsg(sim_verbosity_scriptinfos,"Path points copied to status bar.");
            }
        }
        if (cmd.cmdId==CREATE_EQUIVALENT_PATH_OLD_GUITRIGGEREDCMD)
        {
            CPath_old* it=App::currentWorld->sceneObjects->getPathFromHandle(cmd.intParams[0]);
            if ((it!=nullptr)&&(it->pathContainer!=nullptr))
            {
                it->pathContainer->createEquivalent(it->getObjectHandle());
                App::logMsg(sim_verbosity_scriptinfos,"Equivalent path object created.");
            }
        }
        if (cmd.cmdId==TOGGLE_SHAPINGENABLED_PATH_OLD_SHAPINGGUITRIGGEREDCMD)
        {
            CPath_old* it=App::currentWorld->sceneObjects->getPathFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setShapingEnabled(!it->getShapingEnabled());
        }
        if (cmd.cmdId==TOGGLE_FOLLOWORIENTATION_PATH_OLD_SHAPINGGUITRIGGEREDCMD)
        {
            CPath_old* it=App::currentWorld->sceneObjects->getPathFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setShapingFollowFullOrientation(!it->getShapingFollowFullOrientation());
        }
        if (cmd.cmdId==TOGGLE_CONVEXHULLS_PATH_OLD_SHAPINGGUITRIGGEREDCMD)
        {
            CPath_old* it=App::currentWorld->sceneObjects->getPathFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setShapingThroughConvexHull(!it->getShapingThroughConvexHull());
        }
        if (cmd.cmdId==TOGGLE_LASTCOORDLINKSTOFIRST_PATH_OLD_SHAPINGGUITRIGGEREDCMD)
        {
            CPath_old* it=App::currentWorld->sceneObjects->getPathFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setShapingSectionClosed(!it->getShapingSectionClosed());
        }
        if (cmd.cmdId==GENERATE_SHAPE_PATH_OLD_SHAPINGGUITRIGGEREDCMD)
        {
            CPath_old* it=App::currentWorld->sceneObjects->getPathFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
            {
                CShape* shape=it->getShape();
                if (shape!=nullptr)
                    App::currentWorld->sceneObjects->addObjectToScene(shape,false,true);
            }
        }
        if (cmd.cmdId==SET_ELEMENTMAXLENGTH_PATH_OLD_SHAPINGGUITRIGGEREDCMD)
        {
            CPath_old* it=App::currentWorld->sceneObjects->getPathFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setShapingElementMaxLength(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_TYPE_PATH_OLD_SHAPINGGUITRIGGEREDCMD)
        {
            CPath_old* it=App::currentWorld->sceneObjects->getPathFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setShapingType(cmd.intParams[1]);
        }
        if (cmd.cmdId==SET_SCALINGFACTOR_PATH_OLD_SHAPINGGUITRIGGEREDCMD)
        {
            CPath_old* it=App::currentWorld->sceneObjects->getPathFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setShapingScaling(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_COORDINATES_PATH_OLD_SHAPINGGUITRIGGEREDCMD)
        {
            CPath_old* it=App::currentWorld->sceneObjects->getPathFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
            {
                it->shapingCoordinates.assign(cmd.floatParams.begin(),cmd.floatParams.end());
                it->setShapingElementMaxLength(it->getShapingElementMaxLength()); // To trigger an actualization!
            }
        }




        if (cmd.cmdId==TOGGLE_STATIC_SHAPEDYNGUITRIGGEREDCMD)
        {
            CShape* it=App::currentWorld->sceneObjects->getShapeFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setShapeIsDynamicallyStatic(!it->getShapeIsDynamicallyStatic());
        }
        if (cmd.cmdId==TOGGLE_STARTINSLEEPMODE_SHAPEDYNGUITRIGGEREDCMD)
        {
            CShape* it=App::currentWorld->sceneObjects->getShapeFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setStartInDynamicSleeping(!it->getStartInDynamicSleeping());
        }
        if (cmd.cmdId==TOGGLE_RESPONDABLE_SHAPEDYNGUITRIGGEREDCMD)
        {
            CShape* it=App::currentWorld->sceneObjects->getShapeFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setRespondable(!it->getRespondable());
        }
        if (cmd.cmdId==SET_RESPONDABLEMASK_SHAPEDYNGUITRIGGEREDCMD)
        {
            CShape* it=App::currentWorld->sceneObjects->getShapeFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setDynamicCollisionMask(cmd.intParams[1]);
        }
        if (cmd.cmdId==SET_MASS_SHAPEDYNGUITRIGGEREDCMD)
        {
            CShape* it=App::currentWorld->sceneObjects->getShapeFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->getMeshWrapper()->setMass(cmd.floatParams[0]);
        }
        if (cmd.cmdId==MULTIPLY_MASSFORSELECTION_SHAPEDYNGUITRIGGEREDCMD)
        {
            std::vector<CMeshWrapper*> allComponents;
            for (size_t i=0;i<cmd.intParams.size();i++)
            {
                CShape* it=App::currentWorld->sceneObjects->getShapeFromHandle(cmd.intParams[i]);
                if ( (it!=nullptr)&&(!it->getShapeIsDynamicallyStatic()) )
                {
                    CMeshWrapper* sc=it->getMeshWrapper();
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
            CShape* it=App::currentWorld->sceneObjects->getShapeFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->getMeshWrapper()->setPrincipalMomentsOfInertia(C3Vector(&cmd.floatParams[0]));
        }
        if (cmd.cmdId==MULTIPLY_INERTIAFORSELECTION_SHAPEDYNGUITRIGGEREDCMD)
        {
            std::vector<CMeshWrapper*> allComponents;
            for (size_t i=0;i<cmd.intParams.size();i++)
            {
                CShape* it=App::currentWorld->sceneObjects->getShapeFromHandle(cmd.intParams[i]);
                if ( (it!=nullptr)&&(!it->getShapeIsDynamicallyStatic()) )
                {
                    CMeshWrapper* sc=it->getMeshWrapper();
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
            CShape* it=App::currentWorld->sceneObjects->getShapeFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->getMeshWrapper()->setLocalInertiaFrame(cmd.transfParams[0]);
        }
        if (cmd.cmdId==APPLY_DYNPARAMS_SHAPEDYNGUITRIGGEREDCMD)
        {
            CShape* last=App::currentWorld->sceneObjects->getShapeFromHandle(cmd.intParams[0]);
            if (last!=nullptr)
            {
                C7Vector trLast(last->getFullCumulativeTransformation());
                C7Vector trfLast(last->getMeshWrapper()->getLocalInertiaFrame());
                bool lastIsHeightfield=(last->getMeshWrapper()->getPurePrimitiveType()==sim_primitiveshape_heightfield);
                for (size_t i=1;i<cmd.intParams.size();i++)
                {
                    CShape* it=App::currentWorld->sceneObjects->getShapeFromHandle(cmd.intParams[i]);
                    if (it!=nullptr)
                    {
                        bool itIsHeightfield=(it->getMeshWrapper()->getPurePrimitiveType()==sim_primitiveshape_heightfield);
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
                                it->getMeshWrapper()->setMass(last->getMeshWrapper()->getMass());
                                it->getMeshWrapper()->setPrincipalMomentsOfInertia(last->getMeshWrapper()->getPrincipalMomentsOfInertia());
                                it->getMeshWrapper()->setLocalInertiaFrame(last->getMeshWrapper()->getLocalInertiaFrame());
                            }
                        }
                    }
                }
            }
        }
        if (cmd.cmdId==TOGGLE_SETTODYNAMICIFGETSPARENT_SHAPEDYNGUITRIGGEREDCMD)
        {
            CShape* it=App::currentWorld->sceneObjects->getShapeFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setSetAutomaticallyToNonStaticIfGetsParent(!it->getSetAutomaticallyToNonStaticIfGetsParent());
        }
        if (cmd.cmdId==APPLY_RESPONDABLEPARAMS_SHAPEDYNGUITRIGGEREDCMD)
        {
            CShape* last=App::currentWorld->sceneObjects->getShapeFromHandle(cmd.intParams[0]);
            if (last!=nullptr)
            {
                for (size_t i=1;i<cmd.intParams.size();i++)
                {
                    CShape* it=App::currentWorld->sceneObjects->getShapeFromHandle(cmd.intParams[i]);
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
                CShape* it=App::currentWorld->sceneObjects->getShapeFromHandle(cmd.intParams[i]);
                if (it!=nullptr)
                {
                    C7Vector localTr;
                    C3Vector diagI; // massless
                    bool doIt=false;
                    float mass=CPluginContainer::dyn_computeInertia(it->getObjectHandle(),localTr,diagI);
                    float desiredDensity=cmd.floatParams[0];
                    if (mass>0.0)
                    {
                        doIt=true;
                        mass=desiredDensity*mass/1000.0;
                    }
                    else
                    { // fallback algo
                        if (it->getMeshWrapper()->isConvex())
                        {
                            doIt=true;
                            std::vector<float> vert;
                            std::vector<int> ind;
                            it->getMeshWrapper()->getCumulativeMeshes(vert,&ind,nullptr);
                            C3Vector com;
                            C3X3Matrix tensor;
                            float mass=CVolInt::getMassCenterOfMassAndInertiaTensor(&vert[0],(int)vert.size()/3,&ind[0],(int)ind.size()/3,desiredDensity,com,tensor);
                            C4Vector rot;
                            CMeshWrapper::findPrincipalMomentOfInertia(tensor,rot,diagI);
                            localTr=C7Vector(rot,com);
                        }
                    }
                    if (doIt)
                    {
                        it->getMeshWrapper()->setPrincipalMomentsOfInertia(diagI);
                        it->getMeshWrapper()->setLocalInertiaFrame(localTr);
                        it->getMeshWrapper()->setMass(mass);
                    }
                }
            }
        }
        if (cmd.cmdId==SET_ENGINEPARAMS_SHAPEDYNGUITRIGGEREDCMD)
        { // old
            CShape* it=App::currentWorld->sceneObjects->getShapeFromHandle(cmd.intParams[0]);
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
        if (cmd.cmdId==SET_MATERIAL_SHAPEDYNGUITRIGGEREDCMD)
        {
            _editShapeMaterial(cmd.intParams[0]);
        }
        if (cmd.cmdId==SET_ENGINEPARAMS_JOINTDYNGUITRIGGEREDCMD)
        {
            _editJointDynProp(cmd.intParams[0]);
        }
        if (cmd.cmdId==DELETE_SCRIPT_SCRIPTGUITRIGGEREDCMD)
        {
            int scriptID=cmd.intParams[0];
            CScriptObject* script=App::currentWorld->embeddedScriptContainer->getScriptFromHandle(scriptID);
            if (script!=nullptr)
            {
                if (App::mainWindow!=nullptr)
                    App::mainWindow->codeEditorContainer->closeFromScriptHandle(scriptID,nullptr,true);
                App::currentWorld->embeddedScriptContainer->removeScript(scriptID);
            }
        }
        if (cmd.cmdId==TOGGLE_DISABLED_SCRIPTGUITRIGGEREDCMD)
        {
            int scriptID=cmd.intParams[0];
            CScriptObject* it=App::worldContainer->getScriptFromHandle(scriptID);
            if (it!=nullptr)
            {
                if (it->getScriptType()==sim_scripttype_customizationscript)
                    it->resetScript();
                it->setScriptIsDisabled(!it->getScriptIsDisabled());
            }
        }
        if (cmd.cmdId==TOGGLE_EXECUTEONCE_SCRIPTGUITRIGGEREDCMD)
        {
            int scriptID=cmd.intParams[0];
            CScriptObject* it=App::currentWorld->embeddedScriptContainer->getScriptFromHandle(scriptID);
            if ((it!=nullptr)&&it->getThreadedExecution_oldThreads())
                it->setExecuteJustOnce_oldThreads(!it->getExecuteJustOnce_oldThreads());
        }
        if (cmd.cmdId==SET_EXECORDER_SCRIPTGUITRIGGEREDCMD)
        {
            int scriptID=cmd.intParams[0];
            CScriptObject* it=App::worldContainer->getScriptFromHandle(scriptID);
            if (it!=nullptr)
                it->setExecutionPriority(cmd.intParams[1]);
        }
        if (cmd.cmdId==SET_TREETRAVERSALDIR_SCRIPTGUITRIGGEREDCMD)
        {
            int scriptID=cmd.intParams[0];
            CScriptObject* it=App::worldContainer->getScriptFromHandle(scriptID);
            if (it!=nullptr)
                it->setTreeTraversalDirection(cmd.intParams[1]);
        }
        if (cmd.cmdId==SET_ALL_SCRIPTSIMULPARAMETERGUITRIGGEREDCMD)
        {
            int objID=cmd.intParams[0];
            CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(objID);
            if (it!=nullptr)
            {
                CUserParameters* sp=it->getUserScriptParameterObject();
                sp->userParamEntries.clear();
                for (size_t i=0;i<cmd.intParams.size()-1;i++)
                {
                    SUserParamEntry e;
                    e.name=cmd.stringParams[3*i+0];
                    e.unit=cmd.stringParams[3*i+1];
                    e.value=cmd.stringParams[3*i+2];
                    e.properties=cmd.intParams[1+i];
                    sp->userParamEntries.push_back(e);
                }
                if (sp->userParamEntries.size()==0)
                    it->setUserScriptParameterObject(nullptr);
            }
        }


        if (cmd.cmdId==TOGGLE_EXPLICITHANDLING_GRAPHGUITRIGGEREDCMD)
        {
            CGraph* it=App::currentWorld->sceneObjects->getGraphFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setExplicitHandling(!it->getExplicitHandling());
        }
        if (cmd.cmdId==TOGGLE_BUFFERCYCLIC_GRAPHGUITRIGGEREDCMD)
        {
            CGraph* it=App::currentWorld->sceneObjects->getGraphFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setCyclic(!it->getCyclic());
        }
        if (cmd.cmdId==TOGGLE_TIMEGRAPHVISIBLE_GRAPHGUITRIGGEREDCMD)
        {
            CGraph* it=App::currentWorld->sceneObjects->getGraphFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
            {
                CGraphData_old* grData=it->getGraphData(cmd.intParams[1]);
                if (grData!=nullptr)
                    grData->setVisible(!grData->getVisible());
            }
        }
        if (cmd.cmdId==TOGGLE_TIMEGRAPHSHOWLABEL_GRAPHGUITRIGGEREDCMD)
        {
            CGraph* it=App::currentWorld->sceneObjects->getGraphFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
            {
                CGraphData_old* grData=it->getGraphData(cmd.intParams[1]);
                if (grData!=nullptr)
                    grData->setLabel(!grData->getLabel());
            }
        }
        if (cmd.cmdId==TOGGLE_TIMEGRAPHLINKPOINTS_GRAPHGUITRIGGEREDCMD)
        {
            CGraph* it=App::currentWorld->sceneObjects->getGraphFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
            {
                CGraphData_old* grData=it->getGraphData(cmd.intParams[1]);
                if (grData!=nullptr)
                    grData->setLinkPoints(!grData->getLinkPoints());
            }
        }
        if (cmd.cmdId==INSERT_DATASTREAM_GRAPHGUITRIGGEREDCMD)
        {
            CGraph* it=App::currentWorld->sceneObjects->getGraphFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
            {
                int currentDataType=cmd.intParams[1];
                int index=cmd.intParams[2];
                int objID;
                CGraphingRoutines_old::loopThroughAllAndGetObjectsFromGraphCategory(index,currentDataType,objID);
                CGraphData_old* newGraphDat=new CGraphData_old(currentDataType,objID,-1);
                CGraph* it=App::currentWorld->sceneObjects->getLastSelectionGraph();
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
            CGraph* it=App::currentWorld->sceneObjects->getGraphFromHandle(cmd.intParams[0]);
            if ((it!=nullptr)&&(cmd.intParams[1]!=-1))
                it->removeGraphData(cmd.intParams[1]);
        }
        if (cmd.cmdId==RENAME_DATASTREAM_GRAPHGUITRIGGEREDCMD)
        {
            CGraph* it=App::currentWorld->sceneObjects->getGraphFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
            {
                std::string newName(cmd.stringParams[0]);
                CGraphData_old* grData=it->getGraphData(cmd.intParams[1]);
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
            CGraph* it=App::currentWorld->sceneObjects->getGraphFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setGraphSize(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_BUFFERSIZE_GRAPHGUITRIGGEREDCMD)
        {
            CGraph* it=App::currentWorld->sceneObjects->getGraphFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setBufferSize(cmd.intParams[1]);
        }
        if (cmd.cmdId==REMOVE_ALLSTATICCURVES_GRAPHGUITRIGGEREDCMD)
        {
            CGraph* it=App::currentWorld->sceneObjects->getGraphFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->removeAllStatics();
        }
        if (cmd.cmdId==REMOVE_ALLCURVES_GRAPHGUITRIGGEREDCMD)
        {
            CGraph* it=App::currentWorld->sceneObjects->getGraphFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->removeAllStreamsAndCurves();
        }
        if (cmd.cmdId==SET_VALUERAWSTATE_GRAPHGUITRIGGEREDCMD)
        {
            CGraph* it=App::currentWorld->sceneObjects->getGraphFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
            {
                CGraphData_old* grData=it->getGraphData(cmd.intParams[1]);
                if (grData!=nullptr)
                    grData->setDerivativeIntegralAndCumulative(cmd.intParams[2]);
            }
        }
        if (cmd.cmdId==SET_VALUEMULTIPLIER_GRAPHGUITRIGGEREDCMD)
        {
            CGraph* it=App::currentWorld->sceneObjects->getGraphFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
            {
                CGraphData_old* grData=it->getGraphData(cmd.intParams[1]);
                if (grData!=nullptr)
                    grData->setZoomFactor(cmd.floatParams[0]);
            }
        }
        if (cmd.cmdId==SET_VALUEOFFSET_GRAPHGUITRIGGEREDCMD)
        {
            CGraph* it=App::currentWorld->sceneObjects->getGraphFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
            {
                CGraphData_old* grData=it->getGraphData(cmd.intParams[1]);
                if (grData!=nullptr)
                    grData->setAddCoeff(cmd.floatParams[0]);
            }
        }
        if (cmd.cmdId==SET_MOVINGAVERAGEPERIOD_GRAPHGUITRIGGEREDCMD)
        {
            CGraph* it=App::currentWorld->sceneObjects->getGraphFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
            {
                CGraphData_old* grData=it->getGraphData(cmd.intParams[1]);
                if (grData!=nullptr)
                    grData->setMovingAverageCount(cmd.intParams[2]);
            }
        }
        if (cmd.cmdId==DUPLICATE_TOSTATIC_GRAPHGUITRIGGEREDCMD)
        {
            CGraph* it=App::currentWorld->sceneObjects->getGraphFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
            {
                CGraphData_old* grData=it->getGraphData(cmd.intParams[1]);
                if ((grData!=nullptr)&&(grData->getDataLength()!=0))
                {
                    it->makeCurveStatic(cmd.intParams[1],0);
                    App::uiThread->messageBox_information(App::mainWindow,IDSN_GRAPH_CURVE,IDSN_CURVE_WAS_DUPLICATED_TO_STATIC,VMESSAGEBOX_OKELI,VMESSAGEBOX_REPLY_OK);
                }
            }
        }





        if (cmd.cmdId==DELETE_CURVE_GRAPHCURVEGUITRIGGEREDCMD)
        {
            CGraph* it=App::currentWorld->sceneObjects->getGraphFromHandle(cmd.intParams[0]);
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
            CGraph* it=App::currentWorld->sceneObjects->getGraphFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
            {
                int objID=cmd.intParams[1];
                if (objID!=-1)
                {
                    std::string newName(cmd.stringParams[0]);
                    CGraphDataComb_old* grData;
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
            CGraph* it=App::currentWorld->sceneObjects->getGraphFromHandle(cmd.intParams[0]);
            if ((it!=nullptr)&&(cmd.intParams[1]!=-1))
            {
                CGraphDataComb_old* grDataComb=nullptr;
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
            CGraph* it=App::currentWorld->sceneObjects->getGraphFromHandle(cmd.intParams[0]);
            if ((it!=nullptr)&&(cmd.intParams[1]!=-1))
            {
                CGraphDataComb_old* grDataComb=nullptr;
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
            CGraph* it=App::currentWorld->sceneObjects->getGraphFromHandle(cmd.intParams[0]);
            if ((it!=nullptr)&&(cmd.intParams[1]!=-1))
            {
                CGraphDataComb_old* grDataComb=nullptr;
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
            CGraph* it=App::currentWorld->sceneObjects->getGraphFromHandle(cmd.intParams[0]);
            if ((it!=nullptr)&&(cmd.intParams[1]!=-1))
            {
                CGraphDataComb_old* grDataComb=nullptr;
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
            CGraph* it=App::currentWorld->sceneObjects->getGraphFromHandle(cmd.intParams[0]);
            if ((it!=nullptr)&&(cmd.intParams[1]!=-1))
            {
                CGraphDataComb_old* grDataComb=it->getGraphData3D(cmd.intParams[1]);
                if (grDataComb!=nullptr)
                    grDataComb->setCurveRelativeToWorld(cmd.boolParams[0]);
            }
        }
        if (cmd.cmdId==SET_CURVEWIDTH_GRAPHCURVEGUITRIGGEREDCMD)
        {
            CGraph* it=App::currentWorld->sceneObjects->getGraphFromHandle(cmd.intParams[0]);
            if ((it!=nullptr)&&(cmd.intParams[1]!=-1))
            {
                CGraphDataComb_old* grDataComb=it->getGraphData3D(cmd.intParams[1]);
                if (grDataComb!=nullptr)
                    grDataComb->set3DCurveWidth(float(cmd.intParams[2]));
            }
        }
        if (cmd.cmdId==DUPLICATE_TOSTATIC_GRAPHCURVEGUITRIGGEREDCMD)
        {
            CGraph* it=App::currentWorld->sceneObjects->getGraphFromHandle(cmd.intParams[0]);
            if ((it!=nullptr)&&(cmd.intParams[1]!=-1))
            {
                CGraphDataComb_old* grDataComb=nullptr;
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
                    App::uiThread->messageBox_information(App::mainWindow,IDSN_GRAPH_CURVE,IDSN_CURVE_WAS_DUPLICATED_TO_STATIC,VMESSAGEBOX_OKELI,VMESSAGEBOX_REPLY_OK);
                }
            }
        }
        if (cmd.cmdId==ADD_NEWCURVE_GRAPHCURVEGUITRIGGEREDCMD)
        {
            CGraph* it=App::currentWorld->sceneObjects->getGraphFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
            {
                CGraphDataComb_old* theNew=new CGraphDataComb_old();
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
            CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setObjectMovementRelativity(0,cmd.intParams[1]);
        }
        if (cmd.cmdId==SET_PERMISSIONS_OBJECTMANIPGUITRIGGEREDCMD)
        {
            CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setObjectMovementPreferredAxes(cmd.intParams[1]);
        }
        if (cmd.cmdId==SET_POSSTEPSIZE_OBJECTMANIPGUITRIGGEREDCMD)
        {
            CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
            {
                int a=it->getObjectMovementOptions();
                if (cmd.floatParams[0]<0.0)
                {
                    if (App::currentWorld->simulation->isSimulationStopped())
                        a=a|1;
                    else
                        a=a|2;
                }
                else
                {
                    if (App::currentWorld->simulation->isSimulationStopped())
                        a=(a|1)-1;
                    else
                        a=(a|2)-2;
                    it->setObjectMovementStepSize(0,cmd.floatParams[0]);
                }
                it->setObjectMovementOptions(a);
            }
        }
        if (cmd.cmdId==SET_ORSTEPSIZE_OBJECTMANIPGUITRIGGEREDCMD)
        {
            CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
            {
                int a=it->getObjectMovementOptions();
                if (cmd.floatParams[0]<0.0)
                {
                    if (App::currentWorld->simulation->isSimulationStopped())
                        a=a|4;
                    else
                        a=a|8;
                }
                else
                {
                    if (App::currentWorld->simulation->isSimulationStopped())
                        a=(a|4)-4;
                    else
                        a=(a|8)-8;
                    it->setObjectMovementStepSize(1,cmd.floatParams[0]);
                }
                it->setObjectMovementOptions(a);
            }
        }
        if (cmd.cmdId==SET_ORRELATIVETO_OBJECTMANIPGUITRIGGEREDCMD)
        {
            CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setObjectMovementRelativity(1,cmd.intParams[1]);
        }




        if (cmd.cmdId==SET_TRANSF_POSITIONTRANSLATIONGUITRIGGEREDCMD)
        {
            CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
            {
                if (cmd.intParams[1]==0)
                    it->setLocalTransformation(it->getFullParentCumulativeTransformation().getInverse()*cmd.transfParams[0]);
                else
                    it->setLocalTransformation(cmd.transfParams[0]);
                if (!App::currentWorld->simulation->isSimulationStopped())
                    simResetDynamicObject_internal(it->getObjectHandle()|sim_handleflag_model); // so that we can also manipulate dynamic objects
            }
        }
        if (cmd.cmdId==APPLY_POS_POSITIONTRANSLATIONGUITRIGGEREDCMD)
        {
            CSceneObject* last=App::currentWorld->sceneObjects->getObjectFromHandle(cmd.intParams[0]);
            if (last!=nullptr)
            {
                int coordMode=cmd.intParams[cmd.intParams.size()-2];
                int mask=cmd.intParams[cmd.intParams.size()-1];
                C7Vector tr;
                if (coordMode==0)
                    tr=last->getCumulativeTransformation();
                else
                    tr=last->getLocalTransformation();
                for (size_t i=1;i<cmd.intParams.size()-2;i++)
                {
                    CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(cmd.intParams[i]);
                    C7Vector trIt;
                    if (coordMode==0)
                        trIt=it->getCumulativeTransformation();
                    else
                        trIt=it->getLocalTransformation();
                    if (mask&1)
                        trIt.X(0)=tr.X(0);
                    if (mask&2)
                        trIt.X(1)=tr.X(1);
                    if (mask&4)
                        trIt.X(2)=tr.X(2);
                    if (coordMode==0)
                        it->setLocalTransformation(it->getFullParentCumulativeTransformation().getInverse()*trIt);
                    else
                        it->setLocalTransformation(trIt);
                    if (!App::currentWorld->simulation->isSimulationStopped())
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
            std::vector<CSceneObject*> allSelObj;
            for (size_t i=0;i<cmd.intParams.size()-2;i++)
            {
                CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(cmd.intParams[i]);
                allSelObj.push_back(it);
            }
            std::vector<CSceneObject*> allSelObjects;
            std::map<CSceneObject*,bool> occ;
            CSceneObject* masterObj=nullptr;
            for (int i=int(allSelObj.size())-1;i>=0;i--)
            {
                CSceneObject* it=allSelObj[i]->getLastParentInSelection(&allSelObj);
                if (it==nullptr)
                    it=allSelObj[i];
                std::map<CSceneObject*,bool>::iterator it2=occ.find(it);
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
                C7Vector oldTr(masterObj->getCumulativeTransformation());
                // Translate/Scale the master's position:
                C7Vector tr;
                if (transfMode==0)
                    tr=masterObj->getCumulativeTransformation();
                else
                    tr=masterObj->getLocalTransformation();
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
                    tr=masterObj->getFullParentCumulativeTransformation().getInverse()*tr;
                masterObj->setLocalTransformation(tr);
                if (!App::currentWorld->simulation->isSimulationStopped())
                    simResetDynamicObject_internal(masterObj->getObjectHandle()|sim_handleflag_model); // so that we can also manipulate dynamic objects

                // Now move the "slaves" appropriately:
                C7Vector newTr(masterObj->getCumulativeTransformation());
                C7Vector shift(newTr*oldTr.getInverse());
                for (size_t i=0;i<allSelObjects.size();i++)
                {
                    CSceneObject* obj=allSelObjects[i];
                    C7Vector oldLTr=obj->getLocalTransformation();
                    C7Vector parentTr=obj->getFullParentCumulativeTransformation();
                    obj->setLocalTransformation(parentTr.getInverse()*shift*parentTr*oldLTr);
                    if (!App::currentWorld->simulation->isSimulationStopped())
                        simResetDynamicObject_internal(obj->getObjectHandle()|sim_handleflag_model); // so that we can also manipulate dynamic objects
                }
            }
        }

        if (cmd.cmdId==APPLY_OR_ORIENTATIONROTATIONGUITRIGGEREDCMD)
        {
            CSceneObject* last=App::currentWorld->sceneObjects->getObjectFromHandle(cmd.intParams[0]);
            if (last!=nullptr)
            {
                int coordMode=cmd.intParams[cmd.intParams.size()-1];
                C7Vector tr;
                if (coordMode==0)
                    tr=last->getCumulativeTransformation();
                else
                    tr=last->getLocalTransformation();
                for (size_t i=1;i<cmd.intParams.size()-1;i++)
                {
                    CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(cmd.intParams[i]);
                    C7Vector trIt;
                    if (coordMode==0)
                        trIt=it->getCumulativeTransformation();
                    else
                        trIt=it->getLocalTransformation();
                    trIt.Q=tr.Q;
                    if (coordMode==0)
                        it->setLocalTransformation(it->getFullParentCumulativeTransformation().getInverse()*trIt);
                    else
                        it->setLocalTransformation(trIt);
                    if (!App::currentWorld->simulation->isSimulationStopped())
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
            std::vector<CSceneObject*> allSelObj;
            for (size_t i=0;i<cmd.intParams.size()-1;i++)
            {
                CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(cmd.intParams[i]);
                allSelObj.push_back(it);
            }
            std::vector<CSceneObject*> allSelObjects;
            std::map<CSceneObject*,bool> occ;
            CSceneObject* masterObj=nullptr;
            for (int i=int(allSelObj.size())-1;i>=0;i--)
            {
                CSceneObject* it=allSelObj[i]->getLastParentInSelection(&allSelObj);
                if (it==nullptr)
                    it=allSelObj[i];
                std::map<CSceneObject*,bool>::iterator it2=occ.find(it);
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
                C7Vector oldTr(masterObj->getCumulativeTransformation());
                // Rotate the master:
                C7Vector tr;
                if (transfMode==0)
                    tr=masterObj->getCumulativeTransformation();
                else
                    tr=masterObj->getLocalTransformation();
                C7Vector m;
                m.setIdentity();
                m.Q.setEulerAngles(rotAngles[0],rotAngles[1],rotAngles[2]);
                if (transfMode==2)
                    tr=tr*m;
                else
                    tr=m*tr;
                if (transfMode==0)
                    tr=masterObj->getFullParentCumulativeTransformation().getInverse()*tr;
                masterObj->setLocalTransformation(tr);
                if (!App::currentWorld->simulation->isSimulationStopped())
                    simResetDynamicObject_internal(masterObj->getObjectHandle()|sim_handleflag_model); // so that we can also manipulate dynamic objects

                // Now rotate the "slaves":
                C7Vector newTr(masterObj->getCumulativeTransformation());
                C7Vector shift(newTr*oldTr.getInverse());
                for (size_t i=0;i<allSelObjects.size();i++)
                {
                    CSceneObject* obj=allSelObjects[i];
                    C7Vector oldLTr=obj->getLocalTransformation();
                    C7Vector parentTr=obj->getFullParentCumulativeTransformation();
                    obj->setLocalTransformation(parentTr.getInverse()*shift*parentTr*oldLTr);
                    if (!App::currentWorld->simulation->isSimulationStopped())
                        simResetDynamicObject_internal(obj->getObjectHandle()|sim_handleflag_model); // so that we can also manipulate dynamic objects
                }
            }
        }



        if (cmd.cmdId==REMOVE_ELEMENT_IKELEMENTGUITRIGGEREDCMD)
        {
            CIkGroup_old* ikGroup=App::currentWorld->ikGroups->getObjectFromHandle(cmd.intParams[0]);
            if (ikGroup!=nullptr)
            {
                CIkElement_old* it=ikGroup->getIkElementFromHandle(cmd.intParams[1]);
                if (it!=nullptr)
                    ikGroup->removeIkElement(cmd.intParams[1]);
            }
        }
        if (cmd.cmdId==ADD_ELEMENT_IKELEMENTGUITRIGGEREDCMD)
        {
            CIkGroup_old* ikGroup=App::currentWorld->ikGroups->getObjectFromHandle(cmd.intParams[0]);
            if (ikGroup!=nullptr)
            {
                CDummy* it=App::currentWorld->sceneObjects->getDummyFromHandle(cmd.intParams[1]);
                if (it!=nullptr)
                {
                    CIkElement_old* newIkEl=new CIkElement_old(it->getObjectHandle());
                    if (!ikGroup->addIkElement(newIkEl))
                        delete newIkEl;
                    else
                    {
                        // Now select the object in the UI. We need to post it so that it arrives after the dialog refresh!:
                        SSimulationThreadCommand cmd2;
                        cmd2.cmdId=CALL_DIALOG_FUNCTION_GUITRIGGEREDCMD;
                        cmd2.intParams.push_back(IKELEMENT_DLG);
                        cmd2.intParams.push_back(0);
                        cmd2.intParams.push_back(newIkEl->getObjectHandle());
                        App::appendSimulationThreadCommand(cmd2);
                    }
                }
            }
        }
        if (cmd.cmdId==TOGGLE_ACTIVE_IKELEMENTGUITRIGGEREDCMD)
        {
            CIkGroup_old* ikGroup=App::currentWorld->ikGroups->getObjectFromHandle(cmd.intParams[0]);
            if (ikGroup!=nullptr)
            {
                CIkElement_old* it=ikGroup->getIkElementFromHandle(cmd.intParams[1]);
                if (it!=nullptr)
                    it->setEnabled(!it->getEnabled());
            }
        }
        if (cmd.cmdId==SET_BASE_IKELEMENTGUITRIGGEREDCMD)
        {
            CIkGroup_old* ikGroup=App::currentWorld->ikGroups->getObjectFromHandle(cmd.intParams[0]);
            if (ikGroup!=nullptr)
            {
                CIkElement_old* it=ikGroup->getIkElementFromHandle(cmd.intParams[1]);
                if (it!=nullptr)
                    it->setBase(cmd.intParams[2]);
            }
        }
        if (cmd.cmdId==TOGGLE_CONSTRAINT_IKELEMENTGUITRIGGEREDCMD)
        {
            CIkGroup_old* ikGroup=App::currentWorld->ikGroups->getObjectFromHandle(cmd.intParams[0]);
            if (ikGroup!=nullptr)
            {
                CIkElement_old* it=ikGroup->getIkElementFromHandle(cmd.intParams[1]);
                if (it!=nullptr)
                {
                    int c=cmd.intParams[2];
//                    if ((c==sim_ik_x_constraint)||(c==sim_ik_y_constraint)||(c==sim_ik_z_constraint)||(c==sim_ik_gamma_constraint))
                        it->setConstraints(it->getConstraints()^c);
//                    if (c==sim_ik_alpha_beta_constraint)
//                    {
//                        it->setConstraints(it->getConstraints()^sim_ik_alpha_beta_constraint);
//                        if ((it->getConstraints()&(sim_ik_alpha_beta_constraint|sim_ik_gamma_constraint))==sim_ik_gamma_constraint)
//                            it->setConstraints(it->getConstraints()^sim_ik_gamma_constraint); // gamma constraint cannot be selected if alpha-beta constraint is not selected!
//                    }
                }
            }
        }
        if (cmd.cmdId==SET_REFERENCEFRAME_IKELEMENTGUITRIGGEREDCMD)
        {
            CIkGroup_old* ikGroup=App::currentWorld->ikGroups->getObjectFromHandle(cmd.intParams[0]);
            if (ikGroup!=nullptr)
            {
                CIkElement_old* it=ikGroup->getIkElementFromHandle(cmd.intParams[1]);
                if (it!=nullptr)
                    it->setAlternativeBaseForConstraints(cmd.intParams[2]);
            }
        }
        if (cmd.cmdId==SET_PRECISION_IKELEMENTGUITRIGGEREDCMD)
        {
            CIkGroup_old* ikGroup=App::currentWorld->ikGroups->getObjectFromHandle(cmd.intParams[0]);
            if (ikGroup!=nullptr)
            {
                CIkElement_old* it=ikGroup->getIkElementFromHandle(cmd.intParams[1]);
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
            CIkGroup_old* ikGroup=App::currentWorld->ikGroups->getObjectFromHandle(cmd.intParams[0]);
            if (ikGroup!=nullptr)
            {
                CIkElement_old* it=ikGroup->getIkElementFromHandle(cmd.intParams[1]);
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
            App::currentWorld->mainSettings->ikCalculationEnabled=!App::currentWorld->mainSettings->ikCalculationEnabled;
        }
        if (cmd.cmdId==REMOVE_IKGROUP_IKGROUPGUITRIGGEREDCMD)
        {
            CIkGroup_old* it=App::currentWorld->ikGroups->getObjectFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                App::currentWorld->ikGroups->removeIkGroup(it->getObjectHandle());
        }
        if (cmd.cmdId==ADD_IKGROUP_IKGROUPGUITRIGGEREDCMD)
        {
            CIkGroup_old* newGroup=new CIkGroup_old();
            newGroup->setObjectName("IK_Group",false);
            App::currentWorld->ikGroups->addIkGroup(newGroup,false);
            // Now select the object in the UI. We need to post it so that it arrives after the dialog refresh!:
            SSimulationThreadCommand cmd2;
            cmd2.cmdId=CALL_DIALOG_FUNCTION_GUITRIGGEREDCMD;
            cmd2.intParams.push_back(IK_DLG);
            cmd2.intParams.push_back(0);
            cmd2.intParams.push_back(newGroup->getObjectHandle());
            App::appendSimulationThreadCommand(cmd2);
            // Following second refresh is needed so that the up/down buttons become enabled:
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
        if (cmd.cmdId==RENAME_IKGROUP_IKGROUPGUITRIGGEREDCMD)
        {
            CIkGroup_old* it=App::currentWorld->ikGroups->getObjectFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setObjectName(cmd.stringParams[0].c_str(),true);
        }
        if (cmd.cmdId==SHIFT_IKGROUP_IKGROUPGUITRIGGEREDCMD)
        {
            CIkGroup_old* it=App::currentWorld->ikGroups->getObjectFromHandle(cmd.intParams[0]);
            bool up=cmd.intParams[1]!=0;
            if (it!=nullptr)
            {
                App::currentWorld->ikGroups->shiftIkGroup(cmd.intParams[0],up);
                SSimulationThreadCommand cmd2;
                cmd2.cmdId=CALL_DIALOG_FUNCTION_GUITRIGGEREDCMD;
                cmd2.intParams.push_back(IK_DLG);
                cmd2.intParams.push_back(0);
                cmd2.intParams.push_back(it->getObjectHandle());
                App::appendSimulationThreadCommand(cmd2);
            }
        }
        if (cmd.cmdId==TOGGLE_EXPLICITHANDLING_IKGROUPGUITRIGGEREDCMD)
        {
            CIkGroup_old* it=App::currentWorld->ikGroups->getObjectFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setExplicitHandling(!it->getExplicitHandling());
        }
        if (cmd.cmdId==TOGGLE_ACTIVE_IKGROUPGUITRIGGEREDCMD)
        {
            CIkGroup_old* it=App::currentWorld->ikGroups->getObjectFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setEnabled(!it->getEnabled());
        }
        if (cmd.cmdId==TOGGLE_IGNOREMAXSTEPSIZES_IKGROUPGUITRIGGEREDCMD)
        {
            CIkGroup_old* it=App::currentWorld->ikGroups->getObjectFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setIgnoreMaxStepSizes(!it->getIgnoreMaxStepSizes());
        }
        if (cmd.cmdId==SET_CALCMETHOD_IKGROUPGUITRIGGEREDCMD)
        {
            CIkGroup_old* it=App::currentWorld->ikGroups->getObjectFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setCalculationMethod(cmd.intParams[1]);
        }
        if (cmd.cmdId==SET_DAMPING_IKGROUPGUITRIGGEREDCMD)
        {
            CIkGroup_old* it=App::currentWorld->ikGroups->getObjectFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setDampingFactor(cmd.floatParams[0]);
        }
        if (cmd.cmdId==SET_ITERATIONS_IKGROUPGUITRIGGEREDCMD)
        {
            CIkGroup_old* it=App::currentWorld->ikGroups->getObjectFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
                it->setMaxIterations(cmd.intParams[1]);
        }
        if (cmd.cmdId==SET_CONDITIONALPARAMS_IKGROUPGUITRIGGEREDCMD)
        {
            CIkGroup_old* it=App::currentWorld->ikGroups->getObjectFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
            {
                CIkGroup_old* it2=App::currentWorld->ikGroups->getObjectFromHandle(cmd.intParams[1]);
                int id=-1;
                if (it2!=nullptr)
                    id=it2->getObjectHandle();
                it->setDoOnFailOrSuccessOf(id,true);
                it->setDoOnPerformed(cmd.intParams[2]<2);
                it->setDoOnFail(cmd.intParams[2]==0);
                it->setRestoreIfPositionNotReached(cmd.boolParams[0]);
                it->setRestoreIfOrientationNotReached(cmd.boolParams[1]);
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

        if (cmd.cmdId==PATHEDIT_MAKEDUMMY_GUITRIGGEREDCMD)
        {
            CDummy* newDummy=new CDummy();
            newDummy->setObjectAlias_direct(cmd.stringParams[0].c_str());
            newDummy->setObjectName_direct_old(cmd.stringParams[0].c_str());
            newDummy->setObjectAltName_direct_old(tt::getObjectAltNameFromObjectName(newDummy->getObjectName_old().c_str()).c_str());
            newDummy->setDummySize(cmd.floatParams[0]);
            App::currentWorld->sceneObjects->addObjectToScene(newDummy,false,true);
            newDummy->setLocalTransformation(cmd.transfParams[0]);
        }
        if (cmd.cmdId==SHAPEEDIT_MAKESHAPE_GUITRIGGEREDCMD)
        {
            App::logMsg(sim_verbosity_msgs,IDSNS_GENERATING_SHAPE);
            int toid=cmd.intParams[0];
            CShape* newShape;
            if (toid!=-1)
                newShape=new CShape(nullptr,cmd.floatVectorParams[0],cmd.intVectorParams[0],nullptr,&cmd.floatVectorParams[2]);
            else
                newShape=new CShape(nullptr,cmd.floatVectorParams[0],cmd.intVectorParams[0],nullptr,nullptr);
            newShape->setVisibleEdges(false);
            newShape->getSingleMesh()->setShadingAngle(0.0f);
            newShape->getSingleMesh()->setEdgeThresholdAngle(0.0f);
            newShape->setObjectAlias_direct("Extracted_shape");
            newShape->setObjectName_direct_old("Extracted_shape");
            newShape->setObjectAltName_direct_old(tt::getObjectAltNameFromObjectName(newShape->getObjectName_old().c_str()).c_str());
            App::currentWorld->sceneObjects->addObjectToScene(newShape,false,true);
            if (toid!=-1)
            {
                CTextureObject* to=App::currentWorld->textureContainer->getObject(toid);
                if (to!=nullptr)
                {
                    to->addDependentObject(newShape->getObjectHandle(),newShape->getSingleMesh()->getUniqueID());
                    CTextureProperty* tp=new CTextureProperty(to->getObjectID());
                    newShape->getSingleMesh()->setTextureProperty(tp);
                    tp->setFixedCoordinates(&newShape->getSingleMesh()->textureCoords_notCopiedNorSerialized);
                    newShape->getSingleMesh()->textureCoords_notCopiedNorSerialized.clear();
                }
            }
            App::logMsg(sim_verbosity_msgs,IDSNS_DONE);
            App::setFullDialogRefreshFlag();
        }
        if (cmd.cmdId==SHAPEEDIT_MAKEPRIMITIVE_GUITRIGGEREDCMD)
        {
            CShape* newShape=new CShape(nullptr,cmd.floatVectorParams[0],cmd.intVectorParams[0],nullptr,nullptr);
            C3Vector size(newShape->getBoundingBoxHalfSizes()*2.0f);
            C7Vector conf(newShape->getLocalTransformation());
            delete newShape;
            CShape* shape=nullptr;

            if (cmd.intParams[0]==0)
            { // Cuboid
                App::logMsg(sim_verbosity_msgs,"Generating cuboid...");
                shape=CAddOperations::addPrimitive_withDialog(ADD_COMMANDS_ADD_PRIMITIVE_RECTANGLE_ACCMD,&size);
                if (shape!=nullptr)
                    shape->setLocalTransformation(conf);
            }

            if (cmd.intParams[0]==1)
            { // sphere
                App::logMsg(sim_verbosity_msgs,"Generating sphere...");
                float mm=std::max<float>(std::max<float>(size(0),size(1)),size(2));
                size(0)=mm;
                size(1)=mm;
                size(2)=mm;
                shape=CAddOperations::addPrimitive_withDialog(ADD_COMMANDS_ADD_PRIMITIVE_SPHERE_ACCMD,&size);
                if (shape!=nullptr)
                    shape->setLocalTransformation(conf);
            }

            if (cmd.intParams[0]==2)
            { // spheroid
                App::logMsg(sim_verbosity_msgs,"Generating spheroid...");
                shape=CAddOperations::addPrimitive_withDialog(ADD_COMMANDS_ADD_PRIMITIVE_SPHERE_ACCMD,&size);
                if (shape!=nullptr)
                    shape->setLocalTransformation(conf);
            }

            if (cmd.intParams[0]==3)
            { // cylinder
                App::logMsg(sim_verbosity_msgs,"Generating cylinder...");
                C3Vector diff(fabs(size(0)-size(1)),fabs(size(0)-size(2)),fabs(size(1)-size(2)));
                int t=2;
                if (std::min<float>(std::min<float>(diff(0),diff(1)),diff(2))==diff(0))
                    t=0;
                if (std::min<float>(std::min<float>(diff(0),diff(1)),diff(2))==diff(1))
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
                App::logMsg(sim_verbosity_msgs,IDSNS_DONE);
            else
                App::logMsg(sim_verbosity_msgs,IDSNS_OPERATION_ABORTED);
            App::setFullDialogRefreshFlag();
        }

        if (cmd.cmdId==SWITCH_TOINSTANCEINDEX_GUITRIGGEREDCMD)
        {
            App::worldContainer->switchToWorld(cmd.intParams[0]);
        }
        if (cmd.cmdId==SET_ACTIVEPAGE_GUITRIGGEREDCMD)
        {
            App::currentWorld->pageContainer->setActivePage(cmd.intParams[0]);
        }
        if (cmd.cmdId==SET_MOUSEMODE_GUITRIGGEREDCMD)
        {
            App::setMouseMode(cmd.intParams[0]);
        }
        if (cmd.cmdId==SELECT_VIEW_GUITRIGGEREDCMD)
        {
            CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(cmd.intParams[0]);
            if (it!=nullptr)
            {
                CSPage* view=App::currentWorld->pageContainer->getPage(cmd.intParams[1]);
                if (view!=nullptr)
                {
                    CSView* subView=view->getView(size_t(cmd.intParams[2]));
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
            if (cmd.intParams[0]==DIRECTORY_ID_TEXTURE)
                App::folders->setTexturesPath(cmd.stringParams[0].c_str());
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
            App::currentWorld->environment->modelThumbnail_notSerializedHere.setUncompressedThumbnailImage((char*)&cmd.uint8Params[0],true,false);
        }
        if (cmd.cmdId==SET_OBJECT_SELECTION_GUITRIGGEREDCMD)
        {
            App::currentWorld->sceneObjects->deselectObjects();
            for (size_t i=0;i<cmd.intParams.size();i++)
                App::currentWorld->sceneObjects->addObjectToSelection(cmd.intParams[i]);
        }
        if (cmd.cmdId==CLEAR_OBJECT_SELECTION_GUITRIGGEREDCMD)
        {
            App::currentWorld->sceneObjects->deselectObjects();
        }
        if (cmd.cmdId==ADD_OBJECTS_TO_SELECTION_GUITRIGGEREDCMD)
        {
            for (size_t i=0;i<cmd.intParams.size();i++)
                App::currentWorld->sceneObjects->addObjectToSelection(cmd.intParams[i]);
        }
        if (cmd.cmdId==INVERT_SELECTION_GUITRIGGEREDCMD)
        {
            for (size_t i=0;i<App::currentWorld->sceneObjects->getObjectCount();i++)
                App::currentWorld->sceneObjects->xorAddObjectToSelection(App::currentWorld->sceneObjects->getObjectFromIndex(i)->getObjectHandle());
        }
        if (cmd.cmdId==POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD)
        {
            App::currentWorld->undoBufferContainer->announceChange();
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
//        if (cmd.cmdId==999995)
//            App::currentWorld->undoBufferContainer->emptyRedoBuffer();
        if (cmd.cmdId==999996)
            App::currentWorld->undoBufferContainer->announceChangeGradual();
        if (cmd.cmdId==999997)
            App::currentWorld->undoBufferContainer->announceChangeStart();
        if (cmd.cmdId==999998)
            App::currentWorld->undoBufferContainer->announceChangeEnd();
        if (cmd.cmdId==999999)
            App::currentWorld->undoBufferContainer->announceChange();

        if ( (cmd.cmdId>START_GUITRIGGEREDCMD)&&(cmd.cmdId<END_GUITRIGGEREDCMD) )
        {
        //  printf("GUI cmd: %i\n",cmd.cmdId);
        }
    }

    if (cmd.cmdId==OPEN_DRAG_AND_DROP_SCENE_CMD)
    {
        CFileOperations::createNewScene(false,true);
        CFileOperations::loadScene(cmd.stringParams[0].c_str(),true,false);
        App::currentWorld->undoBufferContainer->clearSceneSaveMaybeNeededFlag();
    }

    if (cmd.cmdId==AUTO_SAVE_SCENE_CMD)
        _handleAutoSaveSceneCommand(cmd);

    if (cmd.cmdId==MEMORIZE_UNDO_STATE_IF_NEEDED_CMD)
    {
        App::currentWorld->undoBufferContainer->memorizeStateIfNeeded();
        App::appendSimulationThreadCommand(cmd,200);
    }

    if (cmd.cmdId==CHKLICM_CMD)
    {
        std::string v=CSimFlavor::getStringVal(19);
        if (v.size()>0)
        {
            if (v[v.size()-1]!=' ')
                App::logMsg(sim_verbosity_errors,v.c_str());
            else
            {
    #ifdef SIM_WITH_GUI
                App::logMsg(sim_verbosity_errors,v.c_str());
                App::uiThread->messageBox_critical(App::mainWindow,CSimFlavor::getStringVal(20).c_str(),v.c_str(),VMESSAGEBOX_OKELI,VMESSAGEBOX_REPLY_OK);
    #endif
            }
        }
        App::appendSimulationThreadCommand(cmd,CSimFlavor::getIntVal(4));
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
void CSimThread::_handleClickRayIntersection_old(SSimulationThreadCommand cmd)
{
    float nearClipp=cmd.floatParams[0];
    C7Vector transf=cmd.transfParams[0];
    bool mouseDown=cmd.boolParams[0];
    int cameraHandle=cmd.intParams[0];

    CCamera* cam=App::currentWorld->sceneObjects->getCameraFromHandle(cameraHandle);
    if (cam!=nullptr)
    {
        const std::vector<int>* currentSelectionState=App::currentWorld->sceneObjects->getSelectedObjectHandlesPtr();
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
        int psh=simCreateProximitySensor_internal(sim_proximitysensor_ray_subtype,sim_objectspecialproperty_detectable,0,intParams,floatParams,nullptr);
        simSetObjectPosition_internal(psh,cameraHandle,transf.X.data);
        simSetObjectOrientation_internal(psh,cameraHandle,transf.Q.getEulerAngles().data);
        int displayAttrib=sim_displayattribute_renderpass;
        if (App::currentWorld->simulation->getDynamicContentVisualizationOnly())
            displayAttrib|=sim_displayattribute_dynamiccontentonly;
        CProxSensor* prox=App::currentWorld->sceneObjects->getProximitySensorFromHandle(psh);
        float dist=SIM_MAX_FLOAT;
        bool ptValid=false;
        for (size_t i=0;i<App::currentWorld->sceneObjects->getObjectCount();i++)
        {
            CSceneObject* object=App::currentWorld->sceneObjects->getObjectFromIndex(i);
            if (object->getShouldObjectBeDisplayed(cameraHandle,displayAttrib)&&object->isPotentiallyMeasurable())
            {
                int theObj;
                bool valid=CProxSensorRoutine::detectEntity(psh,object->getObjectHandle(),true,false,0.0f,pt,dist,true,true,theObj,0.0f,triNormal,allObjectsAlsoNonDetectable);
                ptValid=ptValid||valid;
                if (valid)
                    obj=theObj;
            }
        }
        C7Vector sensTr(prox->getFullCumulativeTransformation());
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
        App::currentWorld->outsideCommandQueue->addCommand(msg,obj,0,0,0,ptdata,6);
        App::currentWorld->sceneObjects->setSelectedObjectHandles(currentSelectionState);
    }
}

void CSimThread::_handleAutoSaveSceneCommand(SSimulationThreadCommand cmd)
{
    if ( (!CSimFlavor::getBoolVal(15))&&(App::mainWindow!=nullptr)&&App::currentWorld->simulation->isSimulationStopped()&&(App::getEditModeType()==NO_EDIT_MODE) )
    {
        if (cmd.intParams[0]==0)
        { // Here we maybe need to load auto-saved scenes:
            // First post the next command in the sequence:
            cmd.intParams[0]=1;
            App::appendSimulationThreadCommand(cmd,1000);
            CPersistentDataContainer cont(SIM_FILENAME_OF_USER_SETTINGS_IN_BINARY_FILE);
            std::string val;
            cont.readData("SIMSETTINGS_SIM_CRASHED",val);
            if (val=="Yes")
            { // ask what to do:
                if (!App::isFullScreen())
                {
                    if ( (!App::userSettings->doNotShowCrashRecoveryMessage)&&(!App::userSettings->suppressStartupDialogs) )
                    {
                        if (VMESSAGEBOX_REPLY_YES==App::uiThread->messageBox_question(App::mainWindow,CSimFlavor::getStringVal(11).c_str(),CSimFlavor::getStringVal(12).c_str(),VMESSAGEBOX_YES_NO,VMESSAGEBOX_REPLY_NO))
                        {
                            std::string testScene=App::folders->getExecutablePath()+"/";
                            testScene.append("AUTO_SAVED_INSTANCE_1.");
                            testScene+=SIM_SCENE_EXTENSION;
                            if (CFileOperations::loadScene(testScene.c_str(),false,false))
                            {
                                App::currentWorld->mainSettings->setScenePathAndName("");
                                App::logMsg(sim_verbosity_msgs,IDSNS_SCENE_WAS_RESTORED_FROM_AUTO_SAVED_SCENE);
                            }
                            int instanceNb=2;
                            while (true)
                            {
                                testScene=App::folders->getExecutablePath()+"/";
                                testScene.append("AUTO_SAVED_INSTANCE_");
                                testScene+=tt::FNb(instanceNb);
                                testScene+=".";
                                testScene+=SIM_SCENE_EXTENSION;
                                if (VFile::doesFileExist(testScene.c_str()))
                                {
                                    App::worldContainer->createNewWorld();
                                    if (CFileOperations::loadScene(testScene.c_str(),false,false))
                                    {
                                        App::currentWorld->mainSettings->setScenePathAndName("");
                                        App::logMsg(sim_verbosity_msgs,IDSNS_SCENE_WAS_RESTORED_FROM_AUTO_SAVED_SCENE);
                                    }
                                    else
                                        break;
                                    instanceNb++;
                                }
                                else
                                    break;
                            }
                            App::worldContainer->switchToWorld(0);
                        }
                    }
                    else
                        App::logMsg(sim_verbosity_msgs,"It seems that CoppeliaSim crashed in last session (or you might be running several instances of CoppeliaSim in parallel).");
                }
            }
        }
        else if (cmd.intParams[0]==1)
        { // Set the TAG: CoppeliaSim started normally
            // First post the auto-save command:
            cmd.intParams[0]=2;
            App::appendSimulationThreadCommand(cmd,1000);
            CPersistentDataContainer cont(SIM_FILENAME_OF_USER_SETTINGS_IN_BINARY_FILE);
            cont.writeData("SIMSETTINGS_SIM_CRASHED","Yes",!App::userSettings->doNotWritePersistentData);
        }
        else if (cmd.intParams[0]==2)
        {
            // First repost a same command:
            App::appendSimulationThreadCommand(cmd,1000);
            if ( CSimFlavor::getBoolVal(16)&&(App::userSettings->autoSaveDelay>0)&&(!App::currentWorld->environment->getSceneLocked()) )
            {
                if (VDateTime::getSecondsSince1970()>(App::currentWorld->environment->autoSaveLastSaveTimeInSecondsSince1970+App::userSettings->autoSaveDelay*60))
                {
                    std::string savedLoc=App::currentWorld->mainSettings->getScenePathAndName();
                    std::string testScene=App::folders->getExecutablePath()+"/";
                    testScene+="AUTO_SAVED_INSTANCE_";
                    testScene+=tt::FNb(App::worldContainer->getCurrentWorldIndex()+1);
                    testScene+=".";
                    testScene+=SIM_SCENE_EXTENSION;
                    CFileOperations::saveScene(testScene.c_str(),false,false,false);
                    App::currentWorld->mainSettings->setScenePathAndName(savedLoc.c_str());
                    App::currentWorld->environment->autoSaveLastSaveTimeInSecondsSince1970=VDateTime::getSecondsSince1970();
                }
            }
        }
    }
    else
        App::appendSimulationThreadCommand(cmd,1000); // repost the same message a bit later
}

void CSimThread::_displayVariousWaningMessagesDuringSimulation()
{
    TRACE_INTERNAL;

    bool displayNonStandardParams=App::currentWorld->simulation->getDisplayWarningAboutNonDefaultParameters();
    bool displayNonPureNonConvexShapeUseWarning=false;
    bool displayStaticShapeOnDynamicConstructionWarning=false;

    App::currentWorld->dynamicsContainer->displayWarningsIfNeeded(); // Warnings when something not supported by the dynamics engine
    displayNonPureNonConvexShapeUseWarning=App::currentWorld->dynamicsContainer->displayNonPureNonConvexShapeWarningRequired()||displayNonPureNonConvexShapeUseWarning;
    if (App::currentWorld->dynamicsContainer->isWorldThere())
    {
        displayNonStandardParams=App::currentWorld->dynamicsContainer->displayNonDefaultParameterWarningRequired()||displayNonStandardParams;
        displayStaticShapeOnDynamicConstructionWarning=App::currentWorld->dynamicsContainer->displayStaticShapeOnDynamicConstructionWarningRequired()||displayStaticShapeOnDynamicConstructionWarning;
    }

    if (displayNonStandardParams)
    {
        CPersistentDataContainer cont(SIM_FILENAME_OF_USER_SETTINGS_IN_BINARY_FILE);
        std::string val;
        cont.readData("SIMSETTINGS_WARNING_NO_SHOW",val);
        int intVal=0;
        tt::getValidInt(val.c_str(),intVal);
        if (intVal<3)
        {
            if (App::uiThread->messageBox_checkbox(App::mainWindow,IDSN_SIMULATION_PARAMETERS,IDSN_NON_STANDARD_SIM_PARAMS_WARNING,IDSN_DO_NOT_SHOW_THIS_MESSAGE_AGAIN_3X,true))
            {
                intVal++;
                val=tt::FNb(intVal);
                cont.writeData("SIMSETTINGS_WARNING_NO_SHOW",val,!App::userSettings->doNotWritePersistentData);
            }
        }
    }

    if (App::currentWorld->dynamicsContainer->displayVortexPluginIsDemoRequired())
#ifdef WIN_SIM
        App::uiThread->messageBox_information(App::mainWindow,IDSN_PHYSICS_ENGINE,IDS_WARNING_WITH_VORTEX_DEMO_PLUGIN,VMESSAGEBOX_OKELI,VMESSAGEBOX_REPLY_OK);
#endif
#ifdef LIN_SIM
        App::uiThread->messageBox_information(App::mainWindow,IDSN_PHYSICS_ENGINE,IDS_WARNING_WITH_VORTEX_DEMO_PLUGIN,VMESSAGEBOX_OKELI,VMESSAGEBOX_REPLY_OK);
#endif

    if (displayNonPureNonConvexShapeUseWarning)
    {
        CPersistentDataContainer cont(SIM_FILENAME_OF_USER_SETTINGS_IN_BINARY_FILE);
        std::string val;
        cont.readData("NONPURESHAPEFORDYNAMICS_WARNING_NO_SHOW",val);
        int intVal=0;
        tt::getValidInt(val.c_str(),intVal);
        if (intVal<3)
        {
            if (App::uiThread->messageBox_checkbox(App::mainWindow,IDSN_DYNAMIC_CONTENT,IDSN_USING_NON_PURE_NON_CONVEX_SHAPES_FOR_DYNAMICS_WARNING,IDSN_DO_NOT_SHOW_THIS_MESSAGE_AGAIN_3X,true))
            {
                intVal++;
                val=tt::FNb(intVal);
                cont.writeData("NONPURESHAPEFORDYNAMICS_WARNING_NO_SHOW",val,!App::userSettings->doNotWritePersistentData);
            }
        }
    }

    if (displayStaticShapeOnDynamicConstructionWarning)
    {
        CPersistentDataContainer cont(SIM_FILENAME_OF_USER_SETTINGS_IN_BINARY_FILE);
        std::string val;
        cont.readData("STATICSHAPEONTOPOFDYNAMICCONSTRUCTION_WARNING_NO_SHOW",val);
        int intVal=0;
        tt::getValidInt(val.c_str(),intVal);
        if (intVal<3)
        {
            if (App::uiThread->messageBox_checkbox(App::mainWindow,IDSN_DYNAMIC_CONTENT,IDSN_USING_STATIC_SHAPE_ON_TOP_OF_DYNAMIC_CONSTRUCTION_WARNING,IDSN_DO_NOT_SHOW_THIS_MESSAGE_AGAIN_3X,true))
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
    if (App::currentWorld->simulation->getSimulationState()&sim_simulation_advancing)
    {
        frameCount++;
        render=(frameCount>=App::currentWorld->simulation->getSimulationPassesPerRendering_speedModified());
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
        lastRenderingTime=(int)VDateTime::getTimeInMs();
        if (App::mainWindow!=nullptr)
        {
            App::mainWindow->simThread_prepareToRenderScene();
            return(2); // means: wait until rendering finished
        }
        return(0); // we do not want to render
    }
    return(0); // we do not want to render
}

void CSimThread::_editShapeMaterial(int shapeHandle)
{
    CShape* shape=App::currentWorld->sceneObjects->getShapeFromHandle(shapeHandle);
    if (shape==nullptr)
        return;
    CDynMaterialObject* mat=shape->getDynMaterial();
    QJsonObject jmain;
    CAnnJson annJson(&jmain);

    // --------------------------------------------
    QJsonObject jbullet;
    annJson.addJson(jbullet,"stickyContact_v278",mat->getEngineBoolParam(sim_bullet_body_sticky,nullptr),"only Bullet V2.78");
    annJson.addJson(jbullet,"friction_v278",mat->getEngineFloatParam(sim_bullet_body_oldfriction,nullptr),"only Bullet V2.78");
    annJson.addJson(jbullet,"friction",mat->getEngineFloatParam(sim_bullet_body_friction,nullptr),"only Bullet V2.83 and later");
    annJson.addJson(jbullet,"restitution",mat->getEngineFloatParam(sim_bullet_body_restitution,nullptr));
    annJson.addJson(jbullet,"linearDamping",mat->getEngineFloatParam(sim_bullet_body_lineardamping,nullptr));
    annJson.addJson(jbullet,"angularDamping",mat->getEngineFloatParam(sim_bullet_body_angulardamping,nullptr));
    QJsonObject jbulletCollMargin;
    annJson.addJson(jbulletCollMargin,"enabled",mat->getEngineBoolParam(sim_bullet_body_usenondefaultcollisionmargin,nullptr));
    annJson.addJson(jbulletCollMargin,"margin",mat->getEngineFloatParam(sim_bullet_body_nondefaultcollisionmargingfactor,nullptr));
    annJson.addJson(jbullet,"customCollisionMargin",jbulletCollMargin);
    QJsonObject jbulletCollMarginConvex;
    annJson.addJson(jbulletCollMarginConvex,"enabled",mat->getEngineBoolParam(sim_bullet_body_usenondefaultcollisionmarginconvex,nullptr));
    annJson.addJson(jbulletCollMarginConvex,"margin",mat->getEngineFloatParam(sim_bullet_body_nondefaultcollisionmargingfactorconvex,nullptr));
    annJson.addJson(jbullet,"customCollisionMargin_convex",jbulletCollMarginConvex);
    annJson.addJson(jbullet,"autoShrinkConvexMeshes",mat->getEngineBoolParam(sim_bullet_body_autoshrinkconvex,nullptr));
    annJson.addJson(jmain,"bullet",jbullet);

    QJsonObject jode;
    annJson.addJson(jode,"friction",mat->getEngineFloatParam(sim_ode_body_friction,nullptr));
    annJson.addJson(jode,"softERP",mat->getEngineFloatParam(sim_ode_body_softerp,nullptr));
    annJson.addJson(jode,"softCFM",mat->getEngineFloatParam(sim_ode_body_softcfm,nullptr));
    annJson.addJson(jode,"linearDamping",mat->getEngineFloatParam(sim_ode_body_lineardamping,nullptr));
    annJson.addJson(jode,"angularDamping",mat->getEngineFloatParam(sim_ode_body_angulardamping,nullptr));
    annJson.addJson(jode,"maxContacts",mat->getEngineIntParam(sim_ode_body_maxcontacts,nullptr));
    annJson.addJson(jmain,"ode",jode);

    QJsonObject jnewton;
    annJson.addJson(jnewton,"staticFriction",mat->getEngineFloatParam(sim_newton_body_staticfriction,nullptr));
    annJson.addJson(jnewton,"kineticFriction",mat->getEngineFloatParam(sim_newton_body_kineticfriction,nullptr));
    annJson.addJson(jnewton,"restitution",mat->getEngineFloatParam(sim_newton_body_restitution,nullptr));
    annJson.addJson(jnewton,"linearDrag",mat->getEngineFloatParam(sim_newton_body_lineardrag,nullptr));
    annJson.addJson(jnewton,"angularDrag",mat->getEngineFloatParam(sim_newton_body_angulardrag,nullptr));
    annJson.addJson(jnewton,"fastMoving",mat->getEngineBoolParam(sim_newton_body_fastmoving,nullptr));
    annJson.addJson(jmain,"newton",jnewton);

    QJsonObject jmujoco;
    double v[5];
    v[0]=mat->getEngineFloatParam(sim_mujoco_body_friction1,nullptr);
    v[1]=mat->getEngineFloatParam(sim_mujoco_body_friction2,nullptr);
    v[2]=mat->getEngineFloatParam(sim_mujoco_body_friction3,nullptr);
    annJson.addJson(jmujoco,"friction",v,3);
    v[0]=mat->getEngineFloatParam(sim_mujoco_body_solref1,nullptr);
    v[1]=mat->getEngineFloatParam(sim_mujoco_body_solref2,nullptr);
    annJson.addJson(jmujoco,"solref",v,2);
    v[0]=mat->getEngineFloatParam(sim_mujoco_body_solimp1,nullptr);
    v[1]=mat->getEngineFloatParam(sim_mujoco_body_solimp2,nullptr);
    v[2]=mat->getEngineFloatParam(sim_mujoco_body_solimp3,nullptr);
    v[3]=mat->getEngineFloatParam(sim_mujoco_body_solimp4,nullptr);
    v[4]=mat->getEngineFloatParam(sim_mujoco_body_solimp5,nullptr);
    annJson.addJson(jmujoco,"solimp",v,5);
    annJson.addJson(jmujoco,"condim",mat->getEngineIntParam(sim_mujoco_body_condim,nullptr));
    annJson.addJson(jmujoco,"solmix",mat->getEngineFloatParam(sim_mujoco_body_solmix,nullptr));
    annJson.addJson(jmain,"mujoco",jmujoco);

    QJsonObject jvortex;
    annJson.addJson(jvortex,"restitution",mat->getEngineFloatParam(sim_vortex_body_restitution,nullptr));
    annJson.addJson(jvortex,"restitutionThreshold",mat->getEngineFloatParam(sim_vortex_body_restitutionthreshold,nullptr));
    annJson.addJson(jvortex,"compliance",mat->getEngineFloatParam(sim_vortex_body_compliance,nullptr));
    annJson.addJson(jvortex,"damping",mat->getEngineFloatParam(sim_vortex_body_damping,nullptr));
    annJson.addJson(jvortex,"adhesiveForce",mat->getEngineFloatParam(sim_vortex_body_adhesiveforce,nullptr));
    annJson.addJson(jvortex,"linearVelocityDamping",mat->getEngineFloatParam(sim_vortex_body_linearvelocitydamping,nullptr));
    annJson.addJson(jvortex,"angularVelocityDamping",mat->getEngineFloatParam(sim_vortex_body_angularvelocitydamping,nullptr));
    QJsonObject jvortexAutoAngularDamping;
    annJson.addJson(jvortexAutoAngularDamping,"enabled",mat->getEngineBoolParam(sim_vortex_body_autoangulardamping,nullptr));
    annJson.addJson(jvortexAutoAngularDamping,"tensionRatio",mat->getEngineFloatParam(sim_vortex_body_autoangulardampingtensionratio,nullptr));
    annJson.addJson(jvortex,"autoAngularDamping",jvortexAutoAngularDamping);
    annJson.addJson(jvortex,"skinThickness",mat->getEngineFloatParam(sim_vortex_body_skinthickness,nullptr));
    annJson.addJson(jvortex,"autoSlip",mat->getEngineBoolParam(sim_vortex_body_autoslip,nullptr));
    annJson.addJson(jvortex,"fastMoving",mat->getEngineBoolParam(sim_vortex_body_fastmoving,nullptr));
    annJson.addJson(jvortex,"pureShapesAsConvex",mat->getEngineBoolParam(sim_vortex_body_pureshapesasconvex,nullptr));
    annJson.addJson(jvortex,"convexShapesAsVxTriangleMeshBvTree",mat->getEngineBoolParam(sim_vortex_body_convexshapesasrandom,nullptr));
    annJson.addJson(jvortex,"randomShapesAsVxTriangleMeshUvGrid",mat->getEngineBoolParam(sim_vortex_body_randomshapesasterrain,nullptr));
    QJsonObject jvortexautoSleep;
    annJson.addJson(jvortexautoSleep,"linearSpeed",mat->getEngineFloatParam(sim_vortex_body_autosleeplinearspeedthreshold,nullptr));
    annJson.addJson(jvortexautoSleep,"linearAccel",mat->getEngineFloatParam(sim_vortex_body_autosleeplinearaccelthreshold,nullptr));
    annJson.addJson(jvortexautoSleep,"angularSpeed",mat->getEngineFloatParam(sim_vortex_body_autosleepangularspeedthreshold,nullptr));
    annJson.addJson(jvortexautoSleep,"angularAccel",mat->getEngineFloatParam(sim_vortex_body_autosleepangularaccelthreshold,nullptr));
    annJson.addJson(jvortexautoSleep,"steps",mat->getEngineIntParam(sim_vortex_body_autosleepsteplivethreshold,nullptr));
    annJson.addJson(jvortex,"autoSleepThreshold",jvortexautoSleep);
    std::string fricModelInfo("0=box, 1=scaled box, 2=prop low, 3=prop high, 4=scaled box fast, 5=neutral, 6=none");
    QJsonObject jvortexlinPrimAxis;
    v[0]=mat->getEngineFloatParam(sim_vortex_body_primaxisvectorx,nullptr);
    v[1]=mat->getEngineFloatParam(sim_vortex_body_primaxisvectory,nullptr);
    v[2]=mat->getEngineFloatParam(sim_vortex_body_primaxisvectorz,nullptr);
    annJson.addJson(jvortexlinPrimAxis,"axisOrientation",v,3);
    annJson.addJson(jvortexlinPrimAxis,"frictionModel",mat->getEngineIntParam(sim_vortex_body_primlinearaxisfrictionmodel,nullptr),fricModelInfo.c_str());
    annJson.addJson(jvortexlinPrimAxis,"frictionCoeff",mat->getEngineFloatParam(sim_vortex_body_primlinearaxisfriction,nullptr));
    annJson.addJson(jvortexlinPrimAxis,"staticFrictionScale",mat->getEngineFloatParam(sim_vortex_body_primlinearaxisstaticfrictionscale,nullptr));
    annJson.addJson(jvortexlinPrimAxis,"slip",mat->getEngineFloatParam(sim_vortex_body_primlinearaxisslip,nullptr));
    annJson.addJson(jvortexlinPrimAxis,"slide",mat->getEngineFloatParam(sim_vortex_body_primlinearaxisslide,nullptr));
    annJson.addJson(jvortex,"linearPrimaryAxis",jvortexlinPrimAxis);
    QJsonObject jvortexlinSecAxis;
    annJson.addJson(jvortexlinSecAxis,"sameValuesAsPrimaryAxis",mat->getEngineBoolParam(sim_vortex_body_seclinaxissameasprimlinaxis,nullptr));
    annJson.addJson(jvortexlinSecAxis,"frictionModel",mat->getEngineIntParam(sim_vortex_body_seclinearaxisfrictionmodel,nullptr),fricModelInfo.c_str());
    annJson.addJson(jvortexlinSecAxis,"frictionCoeff",mat->getEngineFloatParam(sim_vortex_body_seclinearaxisfriction,nullptr));
    annJson.addJson(jvortexlinSecAxis,"staticFrictionScale",mat->getEngineFloatParam(sim_vortex_body_seclinearaxisstaticfrictionscale,nullptr));
    annJson.addJson(jvortexlinSecAxis,"slip",mat->getEngineFloatParam(sim_vortex_body_seclinearaxisslip,nullptr));
    annJson.addJson(jvortexlinSecAxis,"slide",mat->getEngineFloatParam(sim_vortex_body_seclinearaxisslide,nullptr));
    annJson.addJson(jvortex,"linearSecondaryAxis",jvortexlinSecAxis);
    QJsonObject jvortexangPrimAxis;
    annJson.addJson(jvortexangPrimAxis,"frictionModel",mat->getEngineIntParam(sim_vortex_body_primangulararaxisfrictionmodel,nullptr),fricModelInfo.c_str());
    annJson.addJson(jvortexangPrimAxis,"frictionCoeff",mat->getEngineFloatParam(sim_vortex_body_primangularaxisfriction,nullptr));
    annJson.addJson(jvortexangPrimAxis,"staticFrictionScale",mat->getEngineFloatParam(sim_vortex_body_primangularaxisstaticfrictionscale,nullptr));
    annJson.addJson(jvortexangPrimAxis,"slip",mat->getEngineFloatParam(sim_vortex_body_primangularaxisslip,nullptr));
    annJson.addJson(jvortexangPrimAxis,"slide",mat->getEngineFloatParam(sim_vortex_body_primangularaxisslide,nullptr));
    annJson.addJson(jvortex,"angularPrimaryAxis",jvortexangPrimAxis);
    QJsonObject jvortexangSecAxis;
    annJson.addJson(jvortexangSecAxis,"sameValuesAsPrimaryAxis",mat->getEngineBoolParam(sim_vortex_body_secangaxissameasprimangaxis,nullptr));
    annJson.addJson(jvortexangSecAxis,"frictionModel",mat->getEngineIntParam(sim_vortex_body_secmangulararaxisfrictionmodel,nullptr),fricModelInfo.c_str());
    annJson.addJson(jvortexangSecAxis,"frictionCoeff",mat->getEngineFloatParam(sim_vortex_body_secangularaxisfriction,nullptr));
    annJson.addJson(jvortexangSecAxis,"staticFrictionScale",mat->getEngineFloatParam(sim_vortex_body_secangularaxisstaticfrictionscale,nullptr));
    annJson.addJson(jvortexangSecAxis,"slip",mat->getEngineFloatParam(sim_vortex_body_secangularaxisslip,nullptr));
    annJson.addJson(jvortexangSecAxis,"slide",mat->getEngineFloatParam(sim_vortex_body_secangularaxisslide,nullptr));
    annJson.addJson(jvortex,"angularSecondaryAxis",jvortexangSecAxis);
    QJsonObject jvortexangNormAxis;
    annJson.addJson(jvortexangNormAxis,"sameValuesAsPrimaryAxis",mat->getEngineBoolParam(sim_vortex_body_normangaxissameasprimangaxis,nullptr));
    annJson.addJson(jvortexangNormAxis,"frictionModel",mat->getEngineIntParam(sim_vortex_body_normalmangulararaxisfrictionmodel,nullptr),fricModelInfo.c_str());
    annJson.addJson(jvortexangNormAxis,"frictionCoeff",mat->getEngineFloatParam(sim_vortex_body_normalangularaxisfriction,nullptr));
    annJson.addJson(jvortexangNormAxis,"staticFrictionScale",mat->getEngineFloatParam(sim_vortex_body_normalangularaxisstaticfrictionscale,nullptr));
    annJson.addJson(jvortexangNormAxis,"slip",mat->getEngineFloatParam(sim_vortex_body_normalangularaxisslip,nullptr));
    annJson.addJson(jvortexangNormAxis,"slide",mat->getEngineFloatParam(sim_vortex_body_normalangularaxisslide,nullptr));
    annJson.addJson(jvortex,"angularNormalAxis",jvortexangNormAxis);
    annJson.addJson(jmain,"vortex",jvortex);
    // --------------------------------------------

    std::string initText(annJson.getAnnotatedString());
    printf("Material:\n%s\n",initText.c_str());
    QJsonObject obj;
    std::string modifiedText(initText);
    while(true)
    {
        if (false)
        {
            std::string xml("<editor title=\"Material\" lang=\"json\"/>");
            modifiedText=App::mainWindow->codeEditorContainer->openModalTextEditor(modifiedText.c_str(),xml.c_str(),nullptr);
        }

        std::string modifiedText_noComments=annJson.stripComments(modifiedText.c_str());
        //printf("Material(no comments):\n%s\n",modifiedText_noComments.c_str());

        QJsonParseError parseError;
        QJsonDocument doc=QJsonDocument::fromJson(modifiedText_noComments.c_str(),&parseError);

        if ( (!doc.isNull())&&doc.isObject() )
        {
            obj=doc.object();
            break;
        }

        std::string msg("Invalid JSON data:\n\n");
        msg+=parseError.errorString().toStdString()+"\nat line ";
        msg+=std::to_string(CTTUtil::lineCountAtOffset(modifiedText_noComments.c_str(),parseError.offset))+"\n\nDiscard changes?";
        if (VMESSAGEBOX_REPLY_CANCEL!=App::uiThread->messageBox_warning(App::mainWindow,"Material",msg.c_str(),VMESSAGEBOX_OK_CANCEL,VMESSAGEBOX_REPLY_CANCEL))
            return;
    }
    std::string allErrors;
    QJsonValue val;
    // --------------------------------------------
    if (annJson.getValue(obj,"bullet",QJsonValue::Object,val,&allErrors))
    {
        QJsonObject bullet(val.toObject());
        if (annJson.getValue(bullet,"stickyContact_v278",QJsonValue::Bool,val,&allErrors))
            mat->setEngineBoolParam(sim_bullet_body_sticky,val.toBool());
        if (annJson.getValue(bullet,"friction_v278",QJsonValue::Double,val,&allErrors))
            mat->setEngineFloatParam(sim_bullet_body_oldfriction,val.toDouble());
        if (annJson.getValue(bullet,"friction",QJsonValue::Double,val,&allErrors))
            mat->setEngineFloatParam(sim_bullet_body_friction,val.toDouble());
        if (annJson.getValue(bullet,"restitution",QJsonValue::Double,val,&allErrors))
            mat->setEngineFloatParam(sim_bullet_body_restitution,val.toDouble());
        if (annJson.getValue(bullet,"linearDamping",QJsonValue::Double,val,&allErrors))
            mat->setEngineFloatParam(sim_bullet_body_lineardamping,val.toDouble());
        if (annJson.getValue(bullet,"angularDamping",QJsonValue::Double,val,&allErrors))
            mat->setEngineFloatParam(sim_bullet_body_angulardamping,val.toDouble());
        if (annJson.getValue(bullet,"customCollisionMargin",QJsonValue::Object,val,&allErrors))
        {
            QJsonObject sub(val.toObject());
            if (annJson.getValue(sub,"enabled",QJsonValue::Bool,val,&allErrors))
                mat->setEngineBoolParam(sim_bullet_body_usenondefaultcollisionmargin,val.toBool());
            if (annJson.getValue(sub,"margin",QJsonValue::Double,val,&allErrors))
                mat->setEngineFloatParam(sim_bullet_body_nondefaultcollisionmargingfactor,val.toDouble());
        }
        if (annJson.getValue(bullet,"customCollisionMargin_convex",QJsonValue::Object,val,&allErrors))
        {
            QJsonObject sub(val.toObject());
            if (annJson.getValue(sub,"enabled",QJsonValue::Bool,val,&allErrors))
                mat->setEngineBoolParam(sim_bullet_body_usenondefaultcollisionmarginconvex,val.toBool());
            if (annJson.getValue(sub,"margin",QJsonValue::Double,val,&allErrors))
                mat->setEngineFloatParam(sim_bullet_body_nondefaultcollisionmargingfactorconvex,val.toDouble());
        }
        if (annJson.getValue(bullet,"autoShrinkConvexMeshes",QJsonValue::Bool,val,&allErrors))
            mat->setEngineBoolParam(sim_bullet_body_autoshrinkconvex,val.toBool());
    }

    if (annJson.getValue(obj,"ode",QJsonValue::Object,val,&allErrors))
    {
        QJsonObject ode(val.toObject());
        if (annJson.getValue(ode,"friction",QJsonValue::Double,val,&allErrors))
            mat->setEngineFloatParam(sim_ode_body_friction,val.toDouble());
        if (annJson.getValue(ode,"softERP",QJsonValue::Double,val,&allErrors))
            mat->setEngineFloatParam(sim_ode_body_softerp,val.toDouble());
        if (annJson.getValue(ode,"softCFM",QJsonValue::Double,val,&allErrors))
            mat->setEngineFloatParam(sim_ode_body_softcfm,val.toDouble());
        if (annJson.getValue(ode,"linearDamping",QJsonValue::Double,val,&allErrors))
            mat->setEngineFloatParam(sim_ode_body_lineardamping,val.toDouble());
        if (annJson.getValue(ode,"angularDamping",QJsonValue::Double,val,&allErrors))
            mat->setEngineFloatParam(sim_ode_body_angulardamping,val.toDouble());
        if (annJson.getValue(ode,"maxContacts",QJsonValue::Double,val,&allErrors))
            mat->setEngineIntParam(sim_ode_body_maxcontacts,val.toInt());
    }

    if (annJson.getValue(obj,"newton",QJsonValue::Object,val,&allErrors))
    {
        QJsonObject newton(val.toObject());
        if (annJson.getValue(newton,"staticFriction",QJsonValue::Double,val,&allErrors))
            mat->setEngineFloatParam(sim_newton_body_staticfriction,val.toDouble());
        if (annJson.getValue(newton,"kineticFriction",QJsonValue::Double,val,&allErrors))
            mat->setEngineFloatParam(sim_newton_body_kineticfriction,val.toDouble());
        if (annJson.getValue(newton,"restitution",QJsonValue::Double,val,&allErrors))
            mat->setEngineFloatParam(sim_newton_body_restitution,val.toDouble());
        if (annJson.getValue(newton,"linearDrag",QJsonValue::Double,val,&allErrors))
            mat->setEngineFloatParam(sim_newton_body_lineardrag,val.toDouble());
        if (annJson.getValue(newton,"angularDrag",QJsonValue::Double,val,&allErrors))
            mat->setEngineFloatParam(sim_newton_body_angulardrag,val.toDouble());
        if (annJson.getValue(newton,"fastMoving",QJsonValue::Bool,val,&allErrors))
            mat->setEngineBoolParam(sim_newton_body_fastmoving,val.toBool());
    }

    if (annJson.getValue(obj,"mujoco",QJsonValue::Object,val,&allErrors))
    {
        QJsonObject mujoco(val.toObject());
        double v[5];
        if (annJson.getValue(mujoco,"friction",v,3,&allErrors))
        {
            mat->setEngineFloatParam(sim_mujoco_body_friction1,v[0]);
            mat->setEngineFloatParam(sim_mujoco_body_friction2,v[1]);
            mat->setEngineFloatParam(sim_mujoco_body_friction3,v[2]);
        }
        if (annJson.getValue(mujoco,"solref",v,2,&allErrors))
        {
            mat->setEngineFloatParam(sim_mujoco_body_solref1,v[0]);
            mat->setEngineFloatParam(sim_mujoco_body_solref2,v[1]);
        }
        if (annJson.getValue(mujoco,"solimp",v,5,&allErrors))
        {
            mat->setEngineFloatParam(sim_mujoco_body_solimp1,v[0]);
            mat->setEngineFloatParam(sim_mujoco_body_solimp2,v[1]);
            mat->setEngineFloatParam(sim_mujoco_body_solimp3,v[2]);
            mat->setEngineFloatParam(sim_mujoco_body_solimp4,v[3]);
            mat->setEngineFloatParam(sim_mujoco_body_solimp5,v[4]);
        }
        if (annJson.getValue(mujoco,"condim",QJsonValue::Double,val,&allErrors))
            mat->setEngineIntParam(sim_mujoco_body_condim,val.toInt());
        if (annJson.getValue(mujoco,"solmix",QJsonValue::Double,val,&allErrors))
            mat->setEngineFloatParam(sim_mujoco_body_solmix,val.toDouble());
    }

    if (annJson.getValue(obj,"vortex",QJsonValue::Object,val,&allErrors))
    {
        QJsonObject vortex(val.toObject());
        if (annJson.getValue(vortex,"restitution",QJsonValue::Double,val,&allErrors))
            mat->setEngineFloatParam(sim_vortex_body_restitution,val.toDouble());
        if (annJson.getValue(vortex,"restitutionThreshold",QJsonValue::Double,val,&allErrors))
            mat->setEngineFloatParam(sim_vortex_body_restitutionthreshold,val.toDouble());
        if (annJson.getValue(vortex,"compliance",QJsonValue::Double,val,&allErrors))
            mat->setEngineFloatParam(sim_vortex_body_compliance,val.toDouble());
        if (annJson.getValue(vortex,"damping",QJsonValue::Double,val,&allErrors))
            mat->setEngineFloatParam(sim_vortex_body_damping,val.toDouble());
        if (annJson.getValue(vortex,"adhesiveForce",QJsonValue::Double,val,&allErrors))
            mat->setEngineFloatParam(sim_vortex_body_adhesiveforce,val.toDouble());
        if (annJson.getValue(vortex,"linearVelocityDamping",QJsonValue::Double,val,&allErrors))
            mat->setEngineFloatParam(sim_vortex_body_linearvelocitydamping,val.toDouble());
        if (annJson.getValue(vortex,"angularVelocityDamping",QJsonValue::Double,val,&allErrors))
            mat->setEngineFloatParam(sim_vortex_body_angularvelocitydamping,val.toDouble());
        if (annJson.getValue(vortex,"autoAngularDamping",QJsonValue::Object,val,&allErrors))
        {
            QJsonObject sub(val.toObject());
            if (annJson.getValue(sub,"enabled",QJsonValue::Bool,val,&allErrors))
                mat->setEngineBoolParam(sim_vortex_body_autoangulardamping,val.toBool());
            if (annJson.getValue(sub,"tensionRatio",QJsonValue::Double,val,&allErrors))
                mat->setEngineFloatParam(sim_vortex_body_autoangulardampingtensionratio,val.toDouble());
        }
        if (annJson.getValue(vortex,"skinThickness",QJsonValue::Double,val,&allErrors))
            mat->setEngineFloatParam(sim_vortex_body_skinthickness,val.toDouble());
        if (annJson.getValue(vortex,"autoSlip",QJsonValue::Bool,val,&allErrors))
            mat->setEngineBoolParam(sim_vortex_body_autoslip,val.toBool());
        if (annJson.getValue(vortex,"fastMoving",QJsonValue::Bool,val,&allErrors))
            mat->setEngineBoolParam(sim_vortex_body_fastmoving,val.toBool());
        if (annJson.getValue(vortex,"pureShapesAsConvex",QJsonValue::Bool,val,&allErrors))
            mat->setEngineBoolParam(sim_vortex_body_pureshapesasconvex,val.toBool());
        if (annJson.getValue(vortex,"convexShapesAsVxTriangleMeshBvTree",QJsonValue::Bool,val,&allErrors))
            mat->setEngineBoolParam(sim_vortex_body_convexshapesasrandom,val.toBool());
        if (annJson.getValue(vortex,"randomShapesAsVxTriangleMeshUvGrid",QJsonValue::Bool,val,&allErrors))
            mat->setEngineBoolParam(sim_vortex_body_randomshapesasterrain,val.toBool());
        if (annJson.getValue(vortex,"autoSleepThreshold",QJsonValue::Object,val,&allErrors))
        {
            QJsonObject sub(val.toObject());
            if (annJson.getValue(sub,"linearSpeed",QJsonValue::Double,val,&allErrors))
                mat->setEngineFloatParam(sim_vortex_body_autosleeplinearspeedthreshold,val.toDouble());
            if (annJson.getValue(sub,"linearAccel",QJsonValue::Double,val,&allErrors))
                mat->setEngineFloatParam(sim_vortex_body_autosleeplinearaccelthreshold,val.toDouble());
            if (annJson.getValue(sub,"angularSpeed",QJsonValue::Double,val,&allErrors))
                mat->setEngineFloatParam(sim_vortex_body_autosleepangularspeedthreshold,val.toDouble());
            if (annJson.getValue(sub,"angularAccel",QJsonValue::Double,val,&allErrors))
                mat->setEngineFloatParam(sim_vortex_body_autosleepangularaccelthreshold,val.toDouble());
            if (annJson.getValue(sub,"steps",QJsonValue::Double,val,&allErrors))
                mat->setEngineIntParam(sim_vortex_body_autosleepsteplivethreshold,val.toInt());
        }
        if (annJson.getValue(vortex,"linearPrimaryAxis",QJsonValue::Object,val,&allErrors))
        {
            QJsonObject sub(val.toObject());
            double v[3];
            if (annJson.getValue(sub,"axisOrientation",v,3,&allErrors))
            {
                mat->setEngineFloatParam(sim_vortex_body_primaxisvectorx,v[0]);
                mat->setEngineFloatParam(sim_vortex_body_primaxisvectory,v[1]);
                mat->setEngineFloatParam(sim_vortex_body_primaxisvectorz,v[2]);
            }
            if (annJson.getValue(sub,"frictionModel",QJsonValue::Double,val,&allErrors))
                mat->setEngineIntParam(sim_vortex_body_primlinearaxisfrictionmodel,val.toInt());
            if (annJson.getValue(sub,"frictionCoeff",QJsonValue::Double,val,&allErrors))
                mat->setEngineFloatParam(sim_vortex_body_primlinearaxisfriction,val.toDouble());
            if (annJson.getValue(sub,"staticFrictionScale",QJsonValue::Double,val,&allErrors))
                mat->setEngineFloatParam(sim_vortex_body_primlinearaxisstaticfrictionscale,val.toDouble());
            if (annJson.getValue(sub,"slip",QJsonValue::Double,val,&allErrors))
                mat->setEngineFloatParam(sim_vortex_body_primlinearaxisslip,val.toDouble());
            if (annJson.getValue(sub,"slide",QJsonValue::Double,val,&allErrors))
                mat->setEngineFloatParam(sim_vortex_body_primlinearaxisslide,val.toDouble());
        }
        if (annJson.getValue(vortex,"linearSecondaryAxis",QJsonValue::Object,val,&allErrors))
        {
            QJsonObject sub(val.toObject());
            if (annJson.getValue(sub,"sameValuesAsPrimaryAxis",QJsonValue::Bool,val,&allErrors))
                mat->setEngineBoolParam(sim_vortex_body_seclinaxissameasprimlinaxis,val.toInt());
            if (annJson.getValue(sub,"frictionModel",QJsonValue::Double,val,&allErrors))
                mat->setEngineIntParam(sim_vortex_body_seclinearaxisfrictionmodel,val.toInt());
            if (annJson.getValue(sub,"frictionCoeff",QJsonValue::Double,val,&allErrors))
                mat->setEngineFloatParam(sim_vortex_body_seclinearaxisfriction,val.toDouble());
            if (annJson.getValue(sub,"staticFrictionScale",QJsonValue::Double,val,&allErrors))
                mat->setEngineFloatParam(sim_vortex_body_seclinearaxisstaticfrictionscale,val.toDouble());
            if (annJson.getValue(sub,"slip",QJsonValue::Double,val,&allErrors))
                mat->setEngineFloatParam(sim_vortex_body_seclinearaxisslip,val.toDouble());
            if (annJson.getValue(sub,"slide",QJsonValue::Double,val,&allErrors))
                mat->setEngineFloatParam(sim_vortex_body_seclinearaxisslide,val.toDouble());
        }
        if (annJson.getValue(vortex,"angularPrimaryAxis",QJsonValue::Object,val,&allErrors))
        {
            QJsonObject sub(val.toObject());
            if (annJson.getValue(sub,"frictionModel",QJsonValue::Double,val,&allErrors))
                mat->setEngineIntParam(sim_vortex_body_primangulararaxisfrictionmodel,val.toInt());
            if (annJson.getValue(sub,"frictionCoeff",QJsonValue::Double,val,&allErrors))
                mat->setEngineFloatParam(sim_vortex_body_primangularaxisfriction,val.toDouble());
            if (annJson.getValue(sub,"staticFrictionScale",QJsonValue::Double,val,&allErrors))
                mat->setEngineFloatParam(sim_vortex_body_primangularaxisstaticfrictionscale,val.toDouble());
            if (annJson.getValue(sub,"slip",QJsonValue::Double,val,&allErrors))
                mat->setEngineFloatParam(sim_vortex_body_primangularaxisslip,val.toDouble());
            if (annJson.getValue(sub,"slide",QJsonValue::Double,val,&allErrors))
                mat->setEngineFloatParam(sim_vortex_body_primangularaxisslide,val.toDouble());
        }
        if (annJson.getValue(vortex,"angularSecondaryAxis",QJsonValue::Object,val,&allErrors))
        {
            QJsonObject sub(val.toObject());
            if (annJson.getValue(sub,"sameValuesAsPrimaryAxis",QJsonValue::Bool,val,&allErrors))
                mat->setEngineBoolParam(sim_vortex_body_secangaxissameasprimangaxis,val.toInt());
            if (annJson.getValue(sub,"frictionModel",QJsonValue::Double,val,&allErrors))
                mat->setEngineIntParam(sim_vortex_body_secmangulararaxisfrictionmodel,val.toInt());
            if (annJson.getValue(sub,"frictionCoeff",QJsonValue::Double,val,&allErrors))
                mat->setEngineFloatParam(sim_vortex_body_secangularaxisfriction,val.toDouble());
            if (annJson.getValue(sub,"staticFrictionScale",QJsonValue::Double,val,&allErrors))
                mat->setEngineFloatParam(sim_vortex_body_secangularaxisstaticfrictionscale,val.toDouble());
            if (annJson.getValue(sub,"slip",QJsonValue::Double,val,&allErrors))
                mat->setEngineFloatParam(sim_vortex_body_secangularaxisslip,val.toDouble());
            if (annJson.getValue(sub,"slide",QJsonValue::Double,val,&allErrors))
                mat->setEngineFloatParam(sim_vortex_body_secangularaxisslide,val.toDouble());
        }
        if (annJson.getValue(vortex,"angularNormalAxis",QJsonValue::Object,val,&allErrors))
        {
            QJsonObject sub(val.toObject());
            if (annJson.getValue(sub,"sameValuesAsPrimaryAxis",QJsonValue::Bool,val,&allErrors))
                mat->setEngineBoolParam(sim_vortex_body_normangaxissameasprimangaxis,val.toInt());
            if (annJson.getValue(sub,"frictionModel",QJsonValue::Double,val,&allErrors))
                mat->setEngineIntParam(sim_vortex_body_normalmangulararaxisfrictionmodel,val.toInt());
            if (annJson.getValue(sub,"frictionCoeff",QJsonValue::Double,val,&allErrors))
                mat->setEngineFloatParam(sim_vortex_body_normalangularaxisfriction,val.toDouble());
            if (annJson.getValue(sub,"staticFrictionScale",QJsonValue::Double,val,&allErrors))
                mat->setEngineFloatParam(sim_vortex_body_normalangularaxisstaticfrictionscale,val.toDouble());
            if (annJson.getValue(sub,"slip",QJsonValue::Double,val,&allErrors))
                mat->setEngineFloatParam(sim_vortex_body_normalangularaxisslip,val.toDouble());
            if (annJson.getValue(sub,"slide",QJsonValue::Double,val,&allErrors))
                mat->setEngineFloatParam(sim_vortex_body_normalangularaxisslide,val.toDouble());
        }
    }
    // --------------------------------------------

    if (allErrors.size()>0)
    {
        allErrors=std::string("The JSON parser found following error(s):\n")+allErrors;
        App::logMsg(sim_verbosity_scripterrors,allErrors.c_str());
    }
}

void CSimThread::_editJointDynProp(int jointHandle)
{
    CJoint* joint=App::currentWorld->sceneObjects->getJointFromHandle(jointHandle);
    if (joint==nullptr)
        return;

    QJsonObject jmain;
    CAnnJson annJson(&jmain);
    // --------------------------------------------
    QJsonObject jbullet;
    annJson.addJson(jbullet,"normalCFM",joint->getEngineFloatParam(sim_bullet_joint_normalcfm,nullptr));
    annJson.addJson(jbullet,"stopERP",joint->getEngineFloatParam(sim_bullet_joint_stoperp,nullptr));
    annJson.addJson(jbullet,"stopCFM",joint->getEngineFloatParam(sim_bullet_joint_stopcfm,nullptr));
    annJson.addJson(jmain,"bullet",jbullet);

    QJsonObject jode;
    annJson.addJson(jode,"normalCFM",joint->getEngineFloatParam(sim_ode_joint_normalcfm,nullptr));
    annJson.addJson(jode,"stopERP",joint->getEngineFloatParam(sim_ode_joint_stoperp,nullptr));
    annJson.addJson(jode,"stopCFM",joint->getEngineFloatParam(sim_ode_joint_stopcfm,nullptr));
    annJson.addJson(jode,"bounce",joint->getEngineFloatParam(sim_ode_joint_bounce,nullptr));
    annJson.addJson(jode,"fudgeFactor",joint->getEngineFloatParam(sim_ode_joint_fudgefactor,nullptr));
    annJson.addJson(jmain,"ode",jode);

    QJsonObject jnewton;
    QJsonObject jnewtonDependency;
    int h=joint->getEngineIntParam(sim_newton_joint_dependentobjectid,nullptr);
    std::string nameAndPath("");
    if (h>=0)
    {
        CSceneObject* obj=App::currentWorld->sceneObjects->getObjectFromHandle(h);
        nameAndPath=obj->getObjectAlias_shortPath();
    }
    annJson.addJson(jnewtonDependency,"dependentJoint",nameAndPath.c_str(),"specify the full, unique path");
    annJson.addJson(jnewtonDependency,"multiplicationFactor",joint->getEngineFloatParam(sim_newton_joint_dependencyfactor,nullptr));
    annJson.addJson(jnewtonDependency,"offset",joint->getEngineFloatParam(sim_newton_joint_dependencyoffset,nullptr));
    annJson.addJson(jnewton,"jointDependency",jnewtonDependency);
    annJson.addJson(jmain,"newton",jnewton);

    QJsonObject jmujoco;
    QJsonObject jmujocoLimits;
    double v[5];
    v[0]=joint->getEngineFloatParam(sim_mujoco_joint_solreflimit1,nullptr);
    v[1]=joint->getEngineFloatParam(sim_mujoco_joint_solreflimit2,nullptr);
    annJson.addJson(jmujocoLimits,"solref",v,2);
    for (size_t j=0;j<5;j++)
        v[j]=joint->getEngineFloatParam(sim_mujoco_joint_solimplimit1+j,nullptr);
    annJson.addJson(jmujocoLimits,"solimp",v,5);
    annJson.addJson(jmujoco,"limits",jmujocoLimits);
    QJsonObject jmujocoFriction;
    annJson.addJson(jmujocoFriction,"frictionloss",joint->getEngineFloatParam(sim_mujoco_joint_frictionloss,nullptr));
    v[0]=joint->getEngineFloatParam(sim_mujoco_joint_solreffriction1,nullptr);
    v[1]=joint->getEngineFloatParam(sim_mujoco_joint_solreffriction2,nullptr);
    annJson.addJson(jmujocoFriction,"solref",v,2);
    for (size_t j=0;j<5;j++)
        v[j]=joint->getEngineFloatParam(sim_mujoco_joint_solimpfriction1+j,nullptr);
    annJson.addJson(jmujocoFriction,"solimp",v,5);
    annJson.addJson(jmujoco,"friction",jmujocoFriction);
    QJsonObject jmujocoSpring;
    annJson.addJson(jmujocoSpring,"stiffness",joint->getEngineFloatParam(sim_mujoco_joint_stiffness,nullptr));
    annJson.addJson(jmujocoSpring,"damping",joint->getEngineFloatParam(sim_mujoco_joint_damping,nullptr));
    annJson.addJson(jmujocoSpring,"springref",joint->getEngineFloatParam(sim_mujoco_joint_springref,nullptr));
    v[0]=joint->getEngineFloatParam(sim_mujoco_joint_springdamper1,nullptr);
    v[1]=joint->getEngineFloatParam(sim_mujoco_joint_springdamper2,nullptr);
    annJson.addJson(jmujocoSpring,"springdamper",v,2);
    annJson.addJson(jmujoco,"spring",jmujocoSpring);
    annJson.addJson(jmujoco,"armature",joint->getEngineFloatParam(sim_mujoco_joint_armature,nullptr));
    annJson.addJson(jmujoco,"margin",joint->getEngineFloatParam(sim_mujoco_joint_margin,nullptr));
    QJsonObject jmujocoDependency;
    h=joint->getEngineIntParam(sim_mujoco_joint_dependentobjectid,nullptr);
    nameAndPath="";
    if (h>=0)
    {
        CSceneObject* obj=App::currentWorld->sceneObjects->getObjectFromHandle(h);
        nameAndPath=obj->getObjectAlias_shortPath();
    }
    annJson.addJson(jmujocoDependency,"dependentJoint",nameAndPath.c_str(),"specify the full, unique path");
    for (size_t j=0;j<5;j++)
        v[j]=joint->getEngineFloatParam(sim_mujoco_joint_polycoef1+j,nullptr);
    annJson.addJson(jmujocoDependency,"polycoef",v,5);
    annJson.addJson(jmujoco,"jointDependency",jmujocoDependency);
    annJson.addJson(jmain,"mujoco",jmujoco);

    QJsonObject jvortex;
    QJsonObject jvortexAxisFriction;
    annJson.addJson(jvortexAxisFriction,"enabled",joint->getEngineBoolParam(sim_vortex_joint_motorfrictionenabled,nullptr));
    annJson.addJson(jvortexAxisFriction,"proportional",joint->getEngineBoolParam(sim_vortex_joint_proportionalmotorfriction,nullptr));
    annJson.addJson(jvortexAxisFriction,"coeff",joint->getEngineFloatParam(sim_vortex_joint_motorconstraintfrictioncoeff,nullptr));
    annJson.addJson(jvortexAxisFriction,"maxForce",joint->getEngineFloatParam(sim_vortex_joint_motorconstraintfrictionmaxforce,nullptr));
    annJson.addJson(jvortexAxisFriction,"loss",joint->getEngineFloatParam(sim_vortex_joint_motorconstraintfrictionloss,nullptr));
    annJson.addJson(jvortex,"axisFriction",jvortexAxisFriction);
    QJsonObject jvortexAxisLimits;
    annJson.addJson(jvortexAxisLimits,"lowerRestitution",joint->getEngineFloatParam(sim_vortex_joint_lowerlimitrestitution,nullptr));
    annJson.addJson(jvortexAxisLimits,"upperRestitution",joint->getEngineFloatParam(sim_vortex_joint_upperlimitrestitution,nullptr));
    annJson.addJson(jvortexAxisLimits,"lowerStiffness",joint->getEngineFloatParam(sim_vortex_joint_lowerlimitstiffness,nullptr));
    annJson.addJson(jvortexAxisLimits,"upperStiffness",joint->getEngineFloatParam(sim_vortex_joint_upperlimitstiffness,nullptr));
    annJson.addJson(jvortexAxisLimits,"lowerDamping",joint->getEngineFloatParam(sim_vortex_joint_lowerlimitdamping,nullptr));
    annJson.addJson(jvortexAxisLimits,"upperDamping",joint->getEngineFloatParam(sim_vortex_joint_upperlimitdamping,nullptr));
    annJson.addJson(jvortexAxisLimits,"lowerMaxForce",joint->getEngineFloatParam(sim_vortex_joint_lowerlimitmaxforce,nullptr));
    annJson.addJson(jvortexAxisLimits,"upperMaxForce",joint->getEngineFloatParam(sim_vortex_joint_upperlimitmaxforce,nullptr));
    annJson.addJson(jvortex,"axisLimits",jvortexAxisLimits);
    QJsonObject jvortexDependency;
    h=joint->getEngineIntParam(sim_vortex_joint_dependentobjectid,nullptr);
    nameAndPath="";
    if (h>=0)
    {
        CSceneObject* obj=App::currentWorld->sceneObjects->getObjectFromHandle(h);
        nameAndPath=obj->getObjectAlias_shortPath();
    }
    annJson.addJson(jvortexDependency,"dependentJoint",nameAndPath.c_str(),"specify the full, unique path");
    annJson.addJson(jvortexDependency,"multiplicationFactor",joint->getEngineFloatParam(sim_vortex_joint_dependencyfactor,nullptr));
    annJson.addJson(jvortexDependency,"offset",joint->getEngineFloatParam(sim_vortex_joint_dependencyoffset,nullptr));
    annJson.addJson(jvortex,"jointDependency",jvortexDependency);

    int vval;
    vval=joint->getEngineIntParam(sim_vortex_joint_relaxationenabledbc,nullptr);
    bool P0_relaxation_enabled=((vval&1)!=0);
    bool P1_relaxation_enabled=((vval&2)!=0);
    bool P2_relaxation_enabled=((vval&4)!=0);
    bool A0_relaxation_enabled=((vval&8)!=0);
    bool A1_relaxation_enabled=((vval&16)!=0);
    bool A2_relaxation_enabled=((vval&32)!=0);
    vval=joint->getEngineIntParam(sim_vortex_joint_frictionenabledbc,nullptr);
    bool P0_friction_enabled=((vval&1)!=0);
    bool P1_friction_enabled=((vval&2)!=0);
    bool P2_friction_enabled=((vval&4)!=0);
    bool A0_friction_enabled=((vval&8)!=0);
    bool A1_friction_enabled=((vval&16)!=0);
    bool A2_friction_enabled=((vval&32)!=0);
    vval=joint->getEngineIntParam(sim_vortex_joint_frictionproportionalbc,nullptr);
    bool P0_friction_proportional=((vval&1)!=0);
    bool P1_friction_proportional=((vval&2)!=0);
    bool P2_friction_proportional=((vval&4)!=0);
    bool A0_friction_proportional=((vval&8)!=0);
    bool A1_friction_proportional=((vval&16)!=0);
    bool A2_friction_proportional=((vval&32)!=0);

    QJsonObject jvortexXaxisPosition;
    QJsonObject jvortexXaxisPositionRelax;
    annJson.addJson(jvortexXaxisPositionRelax,"enabled",P0_relaxation_enabled);
    annJson.addJson(jvortexXaxisPositionRelax,"stiffness",joint->getEngineFloatParam(sim_vortex_joint_p0stiffness,nullptr));
    annJson.addJson(jvortexXaxisPositionRelax,"damping",joint->getEngineFloatParam(sim_vortex_joint_p0damping,nullptr));
    annJson.addJson(jvortexXaxisPositionRelax,"loss",joint->getEngineFloatParam(sim_vortex_joint_p0loss,nullptr));
    annJson.addJson(jvortexXaxisPosition,"relaxation",jvortexXaxisPositionRelax);
    QJsonObject jvortexXaxisPositionFric;
    annJson.addJson(jvortexXaxisPositionFric,"enabled",P0_friction_enabled);
    annJson.addJson(jvortexXaxisPositionFric,"proportional",P0_friction_proportional);
    annJson.addJson(jvortexXaxisPositionFric,"coeff",joint->getEngineFloatParam(sim_vortex_joint_p0frictioncoeff,nullptr));
    annJson.addJson(jvortexXaxisPositionFric,"maxForce",joint->getEngineFloatParam(sim_vortex_joint_p0frictionmaxforce,nullptr));
    annJson.addJson(jvortexXaxisPositionFric,"loss",joint->getEngineFloatParam(sim_vortex_joint_p0frictionloss,nullptr));
    annJson.addJson(jvortexXaxisPosition,"friction",jvortexXaxisPositionFric);
    annJson.addJson(jvortex,"x-axisPosition",jvortexXaxisPosition);

    QJsonObject jvortexYaxisPosition;
    QJsonObject jvortexYaxisPositionRelax;
    annJson.addJson(jvortexYaxisPositionRelax,"enabled",P1_relaxation_enabled);
    annJson.addJson(jvortexYaxisPositionRelax,"stiffness",joint->getEngineFloatParam(sim_vortex_joint_p1stiffness,nullptr));
    annJson.addJson(jvortexYaxisPositionRelax,"damping",joint->getEngineFloatParam(sim_vortex_joint_p1damping,nullptr));
    annJson.addJson(jvortexYaxisPositionRelax,"loss",joint->getEngineFloatParam(sim_vortex_joint_p1loss,nullptr));
    annJson.addJson(jvortexYaxisPosition,"relaxation",jvortexYaxisPositionRelax);
    QJsonObject jvortexYaxisPositionFric;
    annJson.addJson(jvortexYaxisPositionFric,"enabled",P1_friction_enabled);
    annJson.addJson(jvortexYaxisPositionFric,"proportional",P1_friction_proportional);
    annJson.addJson(jvortexYaxisPositionFric,"coeff",joint->getEngineFloatParam(sim_vortex_joint_p1frictioncoeff,nullptr));
    annJson.addJson(jvortexYaxisPositionFric,"maxForce",joint->getEngineFloatParam(sim_vortex_joint_p1frictionmaxforce,nullptr));
    annJson.addJson(jvortexYaxisPositionFric,"loss",joint->getEngineFloatParam(sim_vortex_joint_p1frictionloss,nullptr));
    annJson.addJson(jvortexYaxisPosition,"friction",jvortexYaxisPositionFric);
    annJson.addJson(jvortex,"y-axisPosition",jvortexYaxisPosition);

    QJsonObject jvortexZaxisPosition;
    QJsonObject jvortexZaxisPositionRelax;
    annJson.addJson(jvortexZaxisPositionRelax,"enabled",P2_relaxation_enabled);
    annJson.addJson(jvortexZaxisPositionRelax,"stiffness",joint->getEngineFloatParam(sim_vortex_joint_p2stiffness,nullptr));
    annJson.addJson(jvortexZaxisPositionRelax,"damping",joint->getEngineFloatParam(sim_vortex_joint_p2damping,nullptr));
    annJson.addJson(jvortexZaxisPositionRelax,"loss",joint->getEngineFloatParam(sim_vortex_joint_p2loss,nullptr));
    annJson.addJson(jvortexZaxisPosition,"relaxation",jvortexZaxisPositionRelax);
    QJsonObject jvortexZaxisPositionFric;
    annJson.addJson(jvortexZaxisPositionFric,"enabled",P2_friction_enabled);
    annJson.addJson(jvortexZaxisPositionFric,"proportional",P2_friction_proportional);
    annJson.addJson(jvortexZaxisPositionFric,"coeff",joint->getEngineFloatParam(sim_vortex_joint_p2frictioncoeff,nullptr));
    annJson.addJson(jvortexZaxisPositionFric,"maxForce",joint->getEngineFloatParam(sim_vortex_joint_p2frictionmaxforce,nullptr));
    annJson.addJson(jvortexZaxisPositionFric,"loss",joint->getEngineFloatParam(sim_vortex_joint_p2frictionloss,nullptr));
    annJson.addJson(jvortexZaxisPosition,"friction",jvortexZaxisPositionFric);
    annJson.addJson(jvortex,"z-axisPosition",jvortexZaxisPosition);

    QJsonObject jvortexXaxisOrientation;
    QJsonObject jvortexXaxisOrientationRelax;
    annJson.addJson(jvortexXaxisOrientationRelax,"enabled",A0_relaxation_enabled);
    annJson.addJson(jvortexXaxisOrientationRelax,"stiffness",joint->getEngineFloatParam(sim_vortex_joint_a0stiffness,nullptr));
    annJson.addJson(jvortexXaxisOrientationRelax,"damping",joint->getEngineFloatParam(sim_vortex_joint_a0damping,nullptr));
    annJson.addJson(jvortexXaxisOrientationRelax,"loss",joint->getEngineFloatParam(sim_vortex_joint_a0loss,nullptr));
    annJson.addJson(jvortexXaxisOrientation,"relaxation",jvortexXaxisOrientationRelax);
    QJsonObject jvortexXaxisOrientationFric;
    annJson.addJson(jvortexXaxisOrientationFric,"enabled",A0_friction_enabled);
    annJson.addJson(jvortexXaxisOrientationFric,"proportional",A0_friction_proportional);
    annJson.addJson(jvortexXaxisOrientationFric,"coeff",joint->getEngineFloatParam(sim_vortex_joint_a0frictioncoeff,nullptr));
    annJson.addJson(jvortexXaxisOrientationFric,"maxTorque",joint->getEngineFloatParam(sim_vortex_joint_a0frictionmaxforce,nullptr));
    annJson.addJson(jvortexXaxisOrientationFric,"loss",joint->getEngineFloatParam(sim_vortex_joint_a0frictionloss,nullptr));
    annJson.addJson(jvortexXaxisOrientation,"friction",jvortexXaxisOrientationFric);
    annJson.addJson(jvortex,"x-axisOrientation",jvortexXaxisOrientation);

    QJsonObject jvortexYaxisOrientation;
    QJsonObject jvortexYaxisOrientationRelax;
    annJson.addJson(jvortexYaxisOrientationRelax,"enabled",A1_relaxation_enabled);
    annJson.addJson(jvortexYaxisOrientationRelax,"stiffness",joint->getEngineFloatParam(sim_vortex_joint_a1stiffness,nullptr));
    annJson.addJson(jvortexYaxisOrientationRelax,"damping",joint->getEngineFloatParam(sim_vortex_joint_a1damping,nullptr));
    annJson.addJson(jvortexYaxisOrientationRelax,"loss",joint->getEngineFloatParam(sim_vortex_joint_a1loss,nullptr));
    annJson.addJson(jvortexYaxisOrientation,"relaxation",jvortexYaxisOrientationRelax);
    QJsonObject jvortexYaxisOrientationFric;
    annJson.addJson(jvortexYaxisOrientationFric,"enabled",A1_friction_enabled);
    annJson.addJson(jvortexYaxisOrientationFric,"proportional",A1_friction_proportional);
    annJson.addJson(jvortexYaxisOrientationFric,"coeff",joint->getEngineFloatParam(sim_vortex_joint_a1frictioncoeff,nullptr));
    annJson.addJson(jvortexYaxisOrientationFric,"maxTorque",joint->getEngineFloatParam(sim_vortex_joint_a1frictionmaxforce,nullptr));
    annJson.addJson(jvortexYaxisOrientationFric,"loss",joint->getEngineFloatParam(sim_vortex_joint_a1frictionloss,nullptr));
    annJson.addJson(jvortexYaxisOrientation,"friction",jvortexYaxisOrientationFric);
    annJson.addJson(jvortex,"y-axisOrientation",jvortexYaxisOrientation);

    QJsonObject jvortexZaxisOrientation;
    QJsonObject jvortexZaxisOrientationRelax;
    annJson.addJson(jvortexZaxisOrientationRelax,"enabled",A2_relaxation_enabled);
    annJson.addJson(jvortexZaxisOrientationRelax,"stiffness",joint->getEngineFloatParam(sim_vortex_joint_a2stiffness,nullptr));
    annJson.addJson(jvortexZaxisOrientationRelax,"damping",joint->getEngineFloatParam(sim_vortex_joint_a2damping,nullptr));
    annJson.addJson(jvortexZaxisOrientationRelax,"loss",joint->getEngineFloatParam(sim_vortex_joint_a2loss,nullptr));
    annJson.addJson(jvortexZaxisOrientation,"relaxation",jvortexZaxisOrientationRelax);
    QJsonObject jvortexZaxisOrientationFric;
    annJson.addJson(jvortexZaxisOrientationFric,"enabled",A2_friction_enabled);
    annJson.addJson(jvortexZaxisOrientationFric,"proportional",A2_friction_proportional);
    annJson.addJson(jvortexZaxisOrientationFric,"coeff",joint->getEngineFloatParam(sim_vortex_joint_a2frictioncoeff,nullptr));
    annJson.addJson(jvortexZaxisOrientationFric,"maxTorque",joint->getEngineFloatParam(sim_vortex_joint_a2frictionmaxforce,nullptr));
    annJson.addJson(jvortexZaxisOrientationFric,"loss",joint->getEngineFloatParam(sim_vortex_joint_a2frictionloss,nullptr));
    annJson.addJson(jvortexZaxisOrientation,"friction",jvortexZaxisOrientationFric);
    annJson.addJson(jvortex,"z-axisOrientation",jvortexZaxisOrientation);

    annJson.addJson(jmain,"vortex",jvortex);

    // --------------------------------------------

    std::string initText(annJson.getAnnotatedString());
    printf("Joint dynamic properties:\n%s\n",initText.c_str());
    QJsonObject obj;
    std::string modifiedText(initText);
    while(true)
    {
        if (false)
        {
            std::string xml("<editor title=\"Joint Dynamic Properties\" lang=\"json\"/>");
            modifiedText=App::mainWindow->codeEditorContainer->openModalTextEditor(modifiedText.c_str(),xml.c_str(),nullptr);
        }

        std::string modifiedText_noComments=annJson.stripComments(modifiedText.c_str());
        QJsonParseError parseError;
        QJsonDocument doc=QJsonDocument::fromJson(modifiedText_noComments.c_str(),&parseError);

        if ( (!doc.isNull())&&doc.isObject() )
        {
            obj=doc.object();
            break;
        }

        std::string msg("Invalid JSON data:\n\n");
        msg+=parseError.errorString().toStdString()+"\nat line ";
        msg+=std::to_string(CTTUtil::lineCountAtOffset(modifiedText_noComments.c_str(),parseError.offset))+"\n\nDiscard changes?";

        if (VMESSAGEBOX_REPLY_CANCEL!=App::uiThread->messageBox_warning(App::mainWindow,"Joint Dynamic Properties",msg.c_str(),VMESSAGEBOX_OK_CANCEL,VMESSAGEBOX_REPLY_CANCEL))
            return;
    }
    std::string allErrors;
    QJsonValue val;
    // --------------------------------------------
    if (annJson.getValue(obj,"bullet",QJsonValue::Object,val,&allErrors))
    {
        QJsonObject bullet(val.toObject());
        if (annJson.getValue(bullet,"normalCFM",QJsonValue::Double,val,&allErrors))
            joint->setEngineFloatParam(sim_bullet_joint_normalcfm,val.toDouble());
        if (annJson.getValue(bullet,"stopERP",QJsonValue::Double,val,&allErrors))
            joint->setEngineFloatParam(sim_bullet_joint_stoperp,val.toDouble());
        if (annJson.getValue(bullet,"stopCFM",QJsonValue::Double,val,&allErrors))
            joint->setEngineFloatParam(sim_bullet_joint_stopcfm,val.toDouble());
    }

    if (annJson.getValue(obj,"ode",QJsonValue::Object,val,&allErrors))
    {
        QJsonObject ode(val.toObject());
        if (annJson.getValue(ode,"normalCFM",QJsonValue::Double,val,&allErrors))
            joint->setEngineFloatParam(sim_ode_joint_normalcfm,val.toDouble());
        if (annJson.getValue(ode,"stopERP",QJsonValue::Double,val,&allErrors))
            joint->setEngineFloatParam(sim_ode_joint_stoperp,val.toDouble());
        if (annJson.getValue(ode,"stopCFM",QJsonValue::Double,val,&allErrors))
            joint->setEngineFloatParam(sim_ode_joint_stopcfm,val.toDouble());
        if (annJson.getValue(ode,"bounce",QJsonValue::Double,val,&allErrors))
            joint->setEngineFloatParam(sim_ode_joint_bounce,val.toDouble());
        if (annJson.getValue(ode,"fudgeFactor",QJsonValue::Double,val,&allErrors))
            joint->setEngineFloatParam(sim_ode_joint_fudgefactor,val.toDouble());
    }


    if (annJson.getValue(obj,"newton",QJsonValue::Object,val,&allErrors))
    {
        QJsonObject newton(val.toObject());
        if (annJson.getValue(newton,"jointDependency",QJsonValue::Object,val,&allErrors))
        {
            QJsonObject sub(val.toObject());
            if (annJson.getValue(sub,"dependentJoint",QJsonValue::String,val,&allErrors))
            {
                int h=-1;
                std::string str(val.toString().toStdString());
                if (str.size()>0)
                {
                    CSceneObject* obj=App::currentWorld->sceneObjects->getObjectFromPath(nullptr,str.c_str(),0,nullptr);
                    if ( (obj!=nullptr)&&(obj->getObjectType()==sim_object_joint_type)&&(obj->getObjectHandle()!=joint->getObjectHandle()) )
                    {
                        h=obj->getObjectHandle();
                        joint->setEngineIntParam(sim_newton_joint_dependentobjectid,h);
                    }
                    else
                    {
                        if (allErrors.size()>0)
                            allErrors+="\n";
                        allErrors+="Key 'dependentJoint' does not point to a valid joint object and will be ignored.";
                    }
                }
                else
                    joint->setEngineIntParam(sim_newton_joint_dependentobjectid,h);
            }
            if (annJson.getValue(sub,"multiplicationFactor",QJsonValue::Double,val,&allErrors))
                joint->setEngineFloatParam(sim_newton_joint_dependencyfactor,val.toDouble());
            if (annJson.getValue(sub,"offset",QJsonValue::Double,val,&allErrors))
                joint->setEngineFloatParam(sim_newton_joint_dependencyoffset,val.toDouble());
        }
    }

    if (annJson.getValue(obj,"mujoco",QJsonValue::Object,val,&allErrors))
    {
        QJsonObject mujoco(val.toObject());
        double w[5];
        if (annJson.getValue(mujoco,"limits",QJsonValue::Object,val,&allErrors))
        {
            QJsonObject sub(val.toObject());
            if (annJson.getValue(sub,"solref",w,2,&allErrors))
            {
                joint->setEngineFloatParam(sim_mujoco_joint_solreflimit1,w[0]);
                joint->setEngineFloatParam(sim_mujoco_joint_solreflimit2,w[1]);
            }
            if (annJson.getValue(sub,"solimp",w,5,&allErrors))
            {
                for (size_t j=0;j<5;j++)
                    joint->setEngineFloatParam(sim_mujoco_joint_solimplimit1+j,w[j]);
            }
        }
        if (annJson.getValue(mujoco,"friction",QJsonValue::Object,val,&allErrors))
        {
            QJsonObject sub(val.toObject());
            if (annJson.getValue(sub,"frictionloss",QJsonValue::Double,val,&allErrors))
                joint->setEngineFloatParam(sim_mujoco_joint_frictionloss,val.toDouble());
            if (annJson.getValue(sub,"solref",w,2,&allErrors))
            {
                joint->setEngineFloatParam(sim_mujoco_joint_solreffriction1,w[0]);
                joint->setEngineFloatParam(sim_mujoco_joint_solreffriction2,w[1]);
            }
            if (annJson.getValue(sub,"solimp",w,5,&allErrors))
            {
                for (size_t j=0;j<5;j++)
                    joint->setEngineFloatParam(sim_mujoco_joint_solimpfriction1+j,w[j]);
            }
        }
        if (annJson.getValue(mujoco,"spring",QJsonValue::Object,val,&allErrors))
        {
            QJsonObject sub(val.toObject());
            if (annJson.getValue(sub,"stiffness",QJsonValue::Double,val,&allErrors))
                joint->setEngineFloatParam(sim_mujoco_joint_stiffness,val.toDouble());
            if (annJson.getValue(sub,"damping",QJsonValue::Double,val,&allErrors))
                joint->setEngineFloatParam(sim_mujoco_joint_damping,val.toDouble());
            if (annJson.getValue(sub,"springref",QJsonValue::Double,val,&allErrors))
                joint->setEngineFloatParam(sim_mujoco_joint_springref,val.toDouble());
            if (annJson.getValue(sub,"springdamper",w,2,&allErrors))
            {
                joint->setEngineFloatParam(sim_mujoco_joint_springdamper1,w[0]);
                joint->setEngineFloatParam(sim_mujoco_joint_springdamper2,w[1]);
            }
        }
        if (annJson.getValue(mujoco,"armature",QJsonValue::Double,val,&allErrors))
            joint->setEngineFloatParam(sim_mujoco_joint_armature,val.toDouble());
        if (annJson.getValue(mujoco,"margin",QJsonValue::Double,val,&allErrors))
            joint->setEngineFloatParam(sim_mujoco_joint_margin,val.toDouble());
        if (annJson.getValue(mujoco,"jointDependency",QJsonValue::Object,val,&allErrors))
        {
            QJsonObject sub(val.toObject());
            if (annJson.getValue(sub,"dependentJoint",QJsonValue::String,val,&allErrors))
            {
                int h=-1;
                std::string str(val.toString().toStdString());
                if (str.size()>0)
                {
                    CSceneObject* obj=App::currentWorld->sceneObjects->getObjectFromPath(nullptr,str.c_str(),0,nullptr);
                    if ( (obj!=nullptr)&&(obj->getObjectType()==sim_object_joint_type)&&(obj->getObjectHandle()!=joint->getObjectHandle()) )
                    {
                        h=obj->getObjectHandle();
                        joint->setEngineIntParam(sim_mujoco_joint_dependentobjectid,h);
                    }
                    else
                    {
                        if (allErrors.size()>0)
                            allErrors+="\n";
                        allErrors+="Key 'dependentJoint' does not point to a valid joint object and will be ignored.";
                    }
                }
                else
                    joint->setEngineIntParam(sim_mujoco_joint_dependentobjectid,h);
            }
            if (annJson.getValue(sub,"polycoef",w,5,&allErrors))
            {
                for (size_t j=0;j<5;j++)
                    joint->setEngineFloatParam(sim_mujoco_joint_polycoef1+j,w[j]);
            }
        }
    }

    if (annJson.getValue(obj,"vortex",QJsonValue::Object,val,&allErrors))
    {
        QJsonObject vortex(val.toObject());
        if (annJson.getValue(vortex,"axisFriction",QJsonValue::Object,val,&allErrors))
        {
            QJsonObject sub(val.toObject());
            if (annJson.getValue(sub,"enabled",QJsonValue::Bool,val,&allErrors))
                joint->setEngineBoolParam(sim_vortex_joint_motorfrictionenabled,val.toBool());
            if (annJson.getValue(sub,"proportional",QJsonValue::Bool,val,&allErrors))
                joint->setEngineBoolParam(sim_vortex_joint_proportionalmotorfriction,val.toBool());
            if (annJson.getValue(sub,"coeff",QJsonValue::Double,val,&allErrors))
                joint->setEngineFloatParam(sim_vortex_joint_motorconstraintfrictioncoeff,val.toDouble());
            if (annJson.getValue(sub,"maxForce",QJsonValue::Double,val,&allErrors))
                joint->setEngineFloatParam(sim_vortex_joint_motorconstraintfrictionmaxforce,val.toDouble());
            if (annJson.getValue(sub,"loss",QJsonValue::Double,val,&allErrors))
                joint->setEngineFloatParam(sim_vortex_joint_motorconstraintfrictionloss,val.toDouble());
        }
        if (annJson.getValue(vortex,"axisLimits",QJsonValue::Object,val,&allErrors))
        {
            QJsonObject sub(val.toObject());
            if (annJson.getValue(sub,"lowerRestitution",QJsonValue::Double,val,&allErrors))
                joint->setEngineFloatParam(sim_vortex_joint_lowerlimitrestitution,val.toDouble());
            if (annJson.getValue(sub,"upperRestitution",QJsonValue::Double,val,&allErrors))
                joint->setEngineFloatParam(sim_vortex_joint_upperlimitrestitution,val.toDouble());
            if (annJson.getValue(sub,"lowerStiffness",QJsonValue::Double,val,&allErrors))
                joint->setEngineFloatParam(sim_vortex_joint_lowerlimitstiffness,val.toDouble());
            if (annJson.getValue(sub,"upperStiffness",QJsonValue::Double,val,&allErrors))
                joint->setEngineFloatParam(sim_vortex_joint_upperlimitstiffness,val.toDouble());
            if (annJson.getValue(sub,"lowerDamping",QJsonValue::Double,val,&allErrors))
                joint->setEngineFloatParam(sim_vortex_joint_lowerlimitdamping,val.toDouble());
            if (annJson.getValue(sub,"upperDamping",QJsonValue::Double,val,&allErrors))
                joint->setEngineFloatParam(sim_vortex_joint_upperlimitdamping,val.toDouble());
            if (annJson.getValue(sub,"lowerMaxForce",QJsonValue::Double,val,&allErrors))
                joint->setEngineFloatParam(sim_vortex_joint_lowerlimitmaxforce,val.toDouble());
            if (annJson.getValue(sub,"upperMaxForce",QJsonValue::Double,val,&allErrors))
                joint->setEngineFloatParam(sim_vortex_joint_upperlimitmaxforce,val.toDouble());
        }
        if (annJson.getValue(vortex,"jointDependency",QJsonValue::Object,val,&allErrors))
        {
            QJsonObject sub(val.toObject());
            if (annJson.getValue(sub,"dependentJoint",QJsonValue::String,val,&allErrors))
            {
                int h=-1;
                std::string str(val.toString().toStdString());
                if (str.size()>0)
                {
                    CSceneObject* obj=App::currentWorld->sceneObjects->getObjectFromPath(nullptr,str.c_str(),0,nullptr);
                    if ( (obj!=nullptr)&&(obj->getObjectType()==sim_object_joint_type)&&(obj->getObjectHandle()!=joint->getObjectHandle()) )
                    {
                        h=obj->getObjectHandle();
                        joint->setEngineIntParam(sim_vortex_joint_dependentobjectid,h);
                    }
                    else
                    {
                        if (allErrors.size()>0)
                            allErrors+="\n";
                        allErrors+="Key 'dependentJoint' does not point to a valid joint object and will be ignored.";
                    }
                }
                else
                    joint->setEngineIntParam(sim_vortex_joint_dependentobjectid,h);
            }
            if (annJson.getValue(sub,"multiplicationFactor",QJsonValue::Double,val,&allErrors))
                joint->setEngineFloatParam(sim_vortex_joint_dependencyfactor,val.toDouble());
            if (annJson.getValue(sub,"offset",QJsonValue::Double,val,&allErrors))
                joint->setEngineFloatParam(sim_vortex_joint_dependencyoffset,val.toDouble());
        }
        if (annJson.getValue(vortex,"x-axisPosition",QJsonValue::Object,val,&allErrors))
        {
            QJsonObject sub(val.toObject());
            if (annJson.getValue(sub,"relaxation",QJsonValue::Object,val,&allErrors))
            {
                QJsonObject sub2(val.toObject());
                if (annJson.getValue(sub2,"enabled",QJsonValue::Bool,val,&allErrors))
                {
                    vval=joint->getEngineIntParam(sim_vortex_joint_relaxationenabledbc,nullptr);
                    vval=(vval|1)-1;
                    if (val.toBool())
                        vval|=1;
                    joint->setEngineIntParam(sim_vortex_joint_relaxationenabledbc,vval);
                }
                if (annJson.getValue(sub2,"stiffness",QJsonValue::Double,val,&allErrors))
                    joint->setEngineFloatParam(sim_vortex_joint_p0stiffness,val.toDouble());
                if (annJson.getValue(sub2,"damping",QJsonValue::Double,val,&allErrors))
                    joint->setEngineFloatParam(sim_vortex_joint_p0damping,val.toDouble());
                if (annJson.getValue(sub2,"loss",QJsonValue::Double,val,&allErrors))
                    joint->setEngineFloatParam(sim_vortex_joint_p0loss,val.toDouble());
            }
            if (annJson.getValue(sub,"friction",QJsonValue::Object,val,&allErrors))
            {
                QJsonObject sub2(val.toObject());
                if (annJson.getValue(sub2,"enabled",QJsonValue::Bool,val,&allErrors))
                {
                    vval=joint->getEngineIntParam(sim_vortex_joint_frictionenabledbc,nullptr);
                    vval=(vval|1)-1;
                    if (val.toBool())
                        vval|=1;
                    joint->setEngineIntParam(sim_vortex_joint_frictionenabledbc,vval);
                }
                if (annJson.getValue(sub2,"proportional",QJsonValue::Bool,val,&allErrors))
                {
                    vval=joint->getEngineIntParam(sim_vortex_joint_frictionproportionalbc,nullptr);
                    vval=(vval|1)-1;
                    if (val.toBool())
                        vval|=1;
                    joint->setEngineIntParam(sim_vortex_joint_frictionproportionalbc,vval);
                }
                if (annJson.getValue(sub2,"coeff",QJsonValue::Double,val,&allErrors))
                    joint->setEngineFloatParam(sim_vortex_joint_p0frictioncoeff,val.toDouble());
                if (annJson.getValue(sub2,"maxForce",QJsonValue::Double,val,&allErrors))
                    joint->setEngineFloatParam(sim_vortex_joint_p0frictionmaxforce,val.toDouble());
                if (annJson.getValue(sub2,"loss",QJsonValue::Double,val,&allErrors))
                    joint->setEngineFloatParam(sim_vortex_joint_p0frictionloss,val.toDouble());
            }
        }
        if (annJson.getValue(vortex,"y-axisPosition",QJsonValue::Object,val,&allErrors))
        {
            QJsonObject sub(val.toObject());
            if (annJson.getValue(sub,"relaxation",QJsonValue::Object,val,&allErrors))
            {
                QJsonObject sub2(val.toObject());
                if (annJson.getValue(sub2,"enabled",QJsonValue::Bool,val,&allErrors))
                {
                    vval=joint->getEngineIntParam(sim_vortex_joint_relaxationenabledbc,nullptr);
                    vval=(vval|2)-2;
                    if (val.toBool())
                        vval|=2;
                    joint->setEngineIntParam(sim_vortex_joint_relaxationenabledbc,vval);
                }
                if (annJson.getValue(sub2,"stiffness",QJsonValue::Double,val,&allErrors))
                    joint->setEngineFloatParam(sim_vortex_joint_p1stiffness,val.toDouble());
                if (annJson.getValue(sub2,"damping",QJsonValue::Double,val,&allErrors))
                    joint->setEngineFloatParam(sim_vortex_joint_p1damping,val.toDouble());
                if (annJson.getValue(sub2,"loss",QJsonValue::Double,val,&allErrors))
                    joint->setEngineFloatParam(sim_vortex_joint_p1loss,val.toDouble());
            }
            if (annJson.getValue(sub,"friction",QJsonValue::Object,val,&allErrors))
            {
                QJsonObject sub2(val.toObject());
                if (annJson.getValue(sub2,"enabled",QJsonValue::Bool,val,&allErrors))
                {
                    vval=joint->getEngineIntParam(sim_vortex_joint_frictionenabledbc,nullptr);
                    vval=(vval|2)-2;
                    if (val.toBool())
                        vval|=2;
                    joint->setEngineIntParam(sim_vortex_joint_frictionenabledbc,vval);
                }
                if (annJson.getValue(sub2,"proportional",QJsonValue::Bool,val,&allErrors))
                {
                    vval=joint->getEngineIntParam(sim_vortex_joint_frictionproportionalbc,nullptr);
                    vval=(vval|2)-2;
                    if (val.toBool())
                        vval|=2;
                    joint->setEngineIntParam(sim_vortex_joint_frictionproportionalbc,vval);
                }
                if (annJson.getValue(sub2,"coeff",QJsonValue::Double,val,&allErrors))
                    joint->setEngineFloatParam(sim_vortex_joint_p1frictioncoeff,val.toDouble());
                if (annJson.getValue(sub2,"maxForce",QJsonValue::Double,val,&allErrors))
                    joint->setEngineFloatParam(sim_vortex_joint_p1frictionmaxforce,val.toDouble());
                if (annJson.getValue(sub2,"loss",QJsonValue::Double,val,&allErrors))
                    joint->setEngineFloatParam(sim_vortex_joint_p1frictionloss,val.toDouble());
            }
        }
        if (annJson.getValue(vortex,"z-axisPosition",QJsonValue::Object,val,&allErrors))
        {
            QJsonObject sub(val.toObject());
            if (annJson.getValue(sub,"relaxation",QJsonValue::Object,val,&allErrors))
            {
                QJsonObject sub2(val.toObject());
                if (annJson.getValue(sub2,"enabled",QJsonValue::Bool,val,&allErrors))
                {
                    vval=joint->getEngineIntParam(sim_vortex_joint_relaxationenabledbc,nullptr);
                    vval=(vval|4)-4;
                    if (val.toBool())
                        vval|=4;
                    joint->setEngineIntParam(sim_vortex_joint_relaxationenabledbc,vval);
                }
                if (annJson.getValue(sub2,"stiffness",QJsonValue::Double,val,&allErrors))
                    joint->setEngineFloatParam(sim_vortex_joint_p2stiffness,val.toDouble());
                if (annJson.getValue(sub2,"damping",QJsonValue::Double,val,&allErrors))
                    joint->setEngineFloatParam(sim_vortex_joint_p2damping,val.toDouble());
                if (annJson.getValue(sub2,"loss",QJsonValue::Double,val,&allErrors))
                    joint->setEngineFloatParam(sim_vortex_joint_p2loss,val.toDouble());
            }
            if (annJson.getValue(sub,"friction",QJsonValue::Object,val,&allErrors))
            {
                QJsonObject sub2(val.toObject());
                if (annJson.getValue(sub2,"enabled",QJsonValue::Bool,val,&allErrors))
                {
                    vval=joint->getEngineIntParam(sim_vortex_joint_frictionenabledbc,nullptr);
                    vval=(vval|4)-4;
                    if (val.toBool())
                        vval|=4;
                    joint->setEngineIntParam(sim_vortex_joint_frictionenabledbc,vval);
                }
                if (annJson.getValue(sub2,"proportional",QJsonValue::Bool,val,&allErrors))
                {
                    vval=joint->getEngineIntParam(sim_vortex_joint_frictionproportionalbc,nullptr);
                    vval=(vval|4)-4;
                    if (val.toBool())
                        vval|=4;
                    joint->setEngineIntParam(sim_vortex_joint_frictionproportionalbc,vval);
                }
                if (annJson.getValue(sub2,"coeff",QJsonValue::Double,val,&allErrors))
                    joint->setEngineFloatParam(sim_vortex_joint_p2frictioncoeff,val.toDouble());
                if (annJson.getValue(sub2,"maxForce",QJsonValue::Double,val,&allErrors))
                    joint->setEngineFloatParam(sim_vortex_joint_p2frictionmaxforce,val.toDouble());
                if (annJson.getValue(sub2,"loss",QJsonValue::Double,val,&allErrors))
                    joint->setEngineFloatParam(sim_vortex_joint_p2frictionloss,val.toDouble());
            }
        }
        if (annJson.getValue(vortex,"x-axisOrientation",QJsonValue::Object,val,&allErrors))
        {
            QJsonObject sub(val.toObject());
            if (annJson.getValue(sub,"relaxation",QJsonValue::Object,val,&allErrors))
            {
                QJsonObject sub2(val.toObject());
                if (annJson.getValue(sub2,"enabled",QJsonValue::Bool,val,&allErrors))
                {
                    vval=joint->getEngineIntParam(sim_vortex_joint_relaxationenabledbc,nullptr);
                    vval=(vval|8)-8;
                    if (val.toBool())
                        vval|=8;
                    joint->setEngineIntParam(sim_vortex_joint_relaxationenabledbc,vval);
                }
                if (annJson.getValue(sub2,"stiffness",QJsonValue::Double,val,&allErrors))
                    joint->setEngineFloatParam(sim_vortex_joint_a0stiffness,val.toDouble());
                if (annJson.getValue(sub2,"damping",QJsonValue::Double,val,&allErrors))
                    joint->setEngineFloatParam(sim_vortex_joint_a0damping,val.toDouble());
                if (annJson.getValue(sub2,"loss",QJsonValue::Double,val,&allErrors))
                    joint->setEngineFloatParam(sim_vortex_joint_a0loss,val.toDouble());
            }
            if (annJson.getValue(sub,"friction",QJsonValue::Object,val,&allErrors))
            {
                QJsonObject sub2(val.toObject());
                if (annJson.getValue(sub2,"enabled",QJsonValue::Bool,val,&allErrors))
                {
                    vval=joint->getEngineIntParam(sim_vortex_joint_frictionenabledbc,nullptr);
                    vval=(vval|8)-8;
                    if (val.toBool())
                        vval|=8;
                    joint->setEngineIntParam(sim_vortex_joint_frictionenabledbc,vval);
                }
                if (annJson.getValue(sub2,"proportional",QJsonValue::Bool,val,&allErrors))
                {
                    vval=joint->getEngineIntParam(sim_vortex_joint_frictionproportionalbc,nullptr);
                    vval=(vval|8)-8;
                    if (val.toBool())
                        vval|=8;
                    joint->setEngineIntParam(sim_vortex_joint_frictionproportionalbc,vval);
                }
                if (annJson.getValue(sub2,"coeff",QJsonValue::Double,val,&allErrors))
                    joint->setEngineFloatParam(sim_vortex_joint_a0frictioncoeff,val.toDouble());
                if (annJson.getValue(sub2,"maxTorque",QJsonValue::Double,val,&allErrors))
                    joint->setEngineFloatParam(sim_vortex_joint_a0frictionmaxforce,val.toDouble());
                if (annJson.getValue(sub2,"loss",QJsonValue::Double,val,&allErrors))
                    joint->setEngineFloatParam(sim_vortex_joint_a0frictionloss,val.toDouble());
            }
        }
        if (annJson.getValue(vortex,"y-axisOrientation",QJsonValue::Object,val,&allErrors))
        {
            QJsonObject sub(val.toObject());
            if (annJson.getValue(sub,"relaxation",QJsonValue::Object,val,&allErrors))
            {
                QJsonObject sub2(val.toObject());
                if (annJson.getValue(sub2,"enabled",QJsonValue::Bool,val,&allErrors))
                {
                    vval=joint->getEngineIntParam(sim_vortex_joint_relaxationenabledbc,nullptr);
                    vval=(vval|16)-16;
                    if (val.toBool())
                        vval|=16;
                    joint->setEngineIntParam(sim_vortex_joint_relaxationenabledbc,vval);
                }
                if (annJson.getValue(sub2,"stiffness",QJsonValue::Double,val,&allErrors))
                    joint->setEngineFloatParam(sim_vortex_joint_a1stiffness,val.toDouble());
                if (annJson.getValue(sub2,"damping",QJsonValue::Double,val,&allErrors))
                    joint->setEngineFloatParam(sim_vortex_joint_a1damping,val.toDouble());
                if (annJson.getValue(sub2,"loss",QJsonValue::Double,val,&allErrors))
                    joint->setEngineFloatParam(sim_vortex_joint_a1loss,val.toDouble());
            }
            if (annJson.getValue(sub,"friction",QJsonValue::Object,val,&allErrors))
            {
                QJsonObject sub2(val.toObject());
                if (annJson.getValue(sub2,"enabled",QJsonValue::Bool,val,&allErrors))
                {
                    vval=joint->getEngineIntParam(sim_vortex_joint_frictionenabledbc,nullptr);
                    vval=(vval|16)-16;
                    if (val.toBool())
                        vval|=16;
                    joint->setEngineIntParam(sim_vortex_joint_frictionenabledbc,vval);
                }
                if (annJson.getValue(sub2,"proportional",QJsonValue::Bool,val,&allErrors))
                {
                    vval=joint->getEngineIntParam(sim_vortex_joint_frictionproportionalbc,nullptr);
                    vval=(vval|16)-16;
                    if (val.toBool())
                        vval|=16;
                    joint->setEngineIntParam(sim_vortex_joint_frictionproportionalbc,vval);
                }
                if (annJson.getValue(sub2,"coeff",QJsonValue::Double,val,&allErrors))
                    joint->setEngineFloatParam(sim_vortex_joint_a1frictioncoeff,val.toDouble());
                if (annJson.getValue(sub2,"maxTorque",QJsonValue::Double,val,&allErrors))
                    joint->setEngineFloatParam(sim_vortex_joint_a1frictionmaxforce,val.toDouble());
                if (annJson.getValue(sub2,"loss",QJsonValue::Double,val,&allErrors))
                    joint->setEngineFloatParam(sim_vortex_joint_a1frictionloss,val.toDouble());
            }
        }
        if (annJson.getValue(vortex,"z-axisOrientation",QJsonValue::Object,val,&allErrors))
        {
            QJsonObject sub(val.toObject());
            if (annJson.getValue(sub,"relaxation",QJsonValue::Object,val,&allErrors))
            {
                QJsonObject sub2(val.toObject());
                if (annJson.getValue(sub2,"enabled",QJsonValue::Bool,val,&allErrors))
                {
                    vval=joint->getEngineIntParam(sim_vortex_joint_relaxationenabledbc,nullptr);
                    vval=(vval|32)-32;
                    if (val.toBool())
                        vval|=32;
                    joint->setEngineIntParam(sim_vortex_joint_relaxationenabledbc,vval);
                }
                if (annJson.getValue(sub2,"stiffness",QJsonValue::Double,val,&allErrors))
                    joint->setEngineFloatParam(sim_vortex_joint_a2stiffness,val.toDouble());
                if (annJson.getValue(sub2,"damping",QJsonValue::Double,val,&allErrors))
                    joint->setEngineFloatParam(sim_vortex_joint_a2damping,val.toDouble());
                if (annJson.getValue(sub2,"loss",QJsonValue::Double,val,&allErrors))
                    joint->setEngineFloatParam(sim_vortex_joint_a2loss,val.toDouble());
            }
            if (annJson.getValue(sub,"friction",QJsonValue::Object,val,&allErrors))
            {
                QJsonObject sub2(val.toObject());
                if (annJson.getValue(sub2,"enabled",QJsonValue::Bool,val,&allErrors))
                {
                    vval=joint->getEngineIntParam(sim_vortex_joint_frictionenabledbc,nullptr);
                    vval=(vval|32)-32;
                    if (val.toBool())
                        vval|=32;
                    joint->setEngineIntParam(sim_vortex_joint_frictionenabledbc,vval);
                }
                if (annJson.getValue(sub2,"proportional",QJsonValue::Bool,val,&allErrors))
                {
                    vval=joint->getEngineIntParam(sim_vortex_joint_frictionproportionalbc,nullptr);
                    vval=(vval|32)-32;
                    if (val.toBool())
                        vval|=32;
                    joint->setEngineIntParam(sim_vortex_joint_frictionproportionalbc,vval);
                }
                if (annJson.getValue(sub2,"coeff",QJsonValue::Double,val,&allErrors))
                    joint->setEngineFloatParam(sim_vortex_joint_a2frictioncoeff,val.toDouble());
                if (annJson.getValue(sub2,"maxTorque",QJsonValue::Double,val,&allErrors))
                    joint->setEngineFloatParam(sim_vortex_joint_a2frictionmaxforce,val.toDouble());
                if (annJson.getValue(sub2,"loss",QJsonValue::Double,val,&allErrors))
                    joint->setEngineFloatParam(sim_vortex_joint_a2frictionloss,val.toDouble());
            }
        }
    }

    // --------------------------------------------
    if (allErrors.size()>0)
    {
        allErrors=std::string("The JSON parser found following error(s):\n")+allErrors;
        App::logMsg(sim_verbosity_scripterrors,allErrors.c_str());
    }
}

#endif
