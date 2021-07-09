#include "uiThread.h"
#include "vThread.h"
#include "app.h"
#include "global.h"
#include "fileOperations.h"
#include "persistentDataContainer.h"
#include "tt.h"
#include "threadPool_old.h"
#include "pluginContainer.h"
#include "simStrings.h"
#include "vDateTime.h"
#include "sceneObjectOperations.h"
#include "addOperations.h"
#include "rendering.h"
#include "simFlavor.h"
#ifdef SIM_WITH_GUI
    #include "qdlgprimitives.h"
    #include "qdlgslider.h"
    #include "qdlgslider2.h"
    #include "vFileDialog.h"
    #include "qdlgconvexdecomposition.h"
    #include "qdlgopenglsettings.h"
    #include "qdlgmessageandcheckbox.h"
    #include "qdlgmodelthumbnailvisu.h"
    #include "qdlgmodelthumbnail.h"
    #include "qdlgtextureloadoptions.h"
    #include "qdlgheightfielddimension.h"
    #include "qdlgmodelproperties.h"
    #include "qdlguserparameters.h"
    #include "qdlgstopscripts.h"
    #include "vMessageBox.h"
    #include "qdlgprogress.h"
    #include <QEvent>
    #include <QInputDialog>
#endif

CUiThread::CUiThread()
{
    _frameId=0;
    _lastFrameId=0;

#ifdef SIM_WITH_QT
    connect(this,SIGNAL(_executeCommandViaUiThread(SUIThreadCommand*,SUIThreadCommand*)),this,SLOT(__executeCommandViaUiThread(SUIThreadCommand*,SUIThreadCommand*)),Qt::BlockingQueuedConnection);
#else
    _noSigSlot_cmdIn=nullptr;
    _noSigSlot_cmdOut=nullptr;
    _noSigSlot_cnter=0;
#endif
#ifdef SIM_WITH_GUI
    // Blocking:
    connect(this,SIGNAL(_requestSceneRender_wait()),this,SLOT(__requestSceneRender_wait()),Qt::QueuedConnection);
#endif
}

CUiThread::~CUiThread()
{
}

#ifndef SIM_WITH_QT
void CUiThread::processGuiEventsUntilQuit_noSignalSlots()
{
    _noSigSlotMutex.lock_simple("CUiThread::processGuiEventsUntilQuit_noSignalSlots");
    while (true)
    {
        _noSigSlotMutex.wait_simple();
        // something has arrived!
        __executeCommandViaUiThread(_noSigSlot_cmdIn,_noSigSlot_cmdOut);
        int cmdId=_noSigSlot_cmdIn->cmdId;
        _noSigSlot_cnter++;
        if (cmdId==NO_SIGNAL_SLOT_EXIT_UITHREADCMD)
            break;
    }
    _noSigSlotMutex.unlock_simple();
}
#endif

bool CUiThread::executeCommandViaUiThread(SUIThreadCommand* cmdIn,SUIThreadCommand* cmdOut)
{ // Called by any thread
    if (!VThread::isCurrentThreadTheUiThread())
    {
#ifndef SIM_WITH_QT
        _noSigSlotMutex.lock_simple("CUiThread::executeCommandViaUiThread");
        _noSigSlot_cmdIn=cmdIn;
        _noSigSlot_cmdOut=cmdOut;
        int cnt=_noSigSlot_cnter;
        _noSigSlotMutex.wakeAll_simple(); // let the UI thread handle the job!
        _noSigSlotMutex.unlock_simple();
        while (_noSigSlot_cnter==cnt)
            VThread::switchThread();
        _noSigSlot_cmdIn=nullptr;
        _noSigSlot_cmdOut=nullptr;
        return(true);
#else
        _executeCommandViaUiThread(cmdIn,cmdOut);
        return(true);
#endif
    }
    else
    {
        __executeCommandViaUiThread(cmdIn,cmdOut);
        return(true);
    }
    return(false);
}

void CUiThread::__executeCommandViaUiThread(SUIThreadCommand* cmdIn,SUIThreadCommand* cmdOut)
{ // called by the UI thread.

#ifdef SIM_WITH_GUI
    if ( (App::mainWindow!=nullptr)&&(!App::isFullScreen())&&(cmdIn->cmdId==PLUS_HVUD_CMD_UITHREADCMD) )
    {
        std::string txt(CSimFlavor::getStringVal(9));
        if (txt.length()!=0)
        {
            if ( (!App::userSettings->doNotShowUpdateCheckMessage)&&(!App::userSettings->suppressStartupDialogs) )
                App::uiThread->messageBox_informationSystemModal(App::mainWindow,"Update information",txt.c_str(),VMESSAGEBOX_OKELI,VMESSAGEBOX_REPLY_OK);
            App::logMsg(sim_verbosity_msgs,txt.c_str());
        }
    }

    if (cmdIn->cmdId==PLUS_CVU_CMD_UITHREADCMD)
    {
        CSimFlavor::setHld(App::mainWindow);
        CSimFlavor::run(6);
    }
#endif

    if ( (cmdIn->cmdId>PLUGIN_START_PLUGUITHREADCMD)&&(cmdIn->cmdId<PLUGIN_END_PLUGUITHREADCMD) )
    {
        if (cmdIn->cmdId==PLUGIN_LOAD_AND_START_PLUGUITHREADCMD)
            cmdOut->intParams.push_back(CPluginContainer::addPlugin(cmdIn->stringParams[0].c_str(),cmdIn->stringParams[1].c_str()));
        if (cmdIn->cmdId==PLUGIN_STOP_AND_UNLOAD_PLUGUITHREADCMD)
            cmdOut->boolParams.push_back(CPluginContainer::unloadPlugin(cmdIn->intParams[0]));
    }

    if (cmdIn->cmdId==DESTROY_GL_TEXTURE_UITHREADCMD)
        destroyGlTexture(cmdIn->uintParams[0]);

#ifdef SIM_WITH_OPENGL
    if (cmdIn->cmdId==CREATE_GL_CONTEXT_FBO_TEXTURE_IF_NEEDED_UITHREADCMD)
        ((CVisionSensor*)cmdIn->objectParams[0])->createGlContextAndFboAndTextureObjectIfNeeded_executedViaUiThread(cmdIn->boolParams[0]);
#endif

    if (cmdIn->cmdId==DETECT_VISION_SENSOR_ENTITY_UITHREADCMD)
        ((CVisionSensor*)cmdIn->objectParams[0])->detectVisionSensorEntity_executedViaUiThread(cmdIn->intParams[0],cmdIn->boolParams[0],cmdIn->boolParams[1],cmdIn->boolParams[2],cmdIn->boolParams[3]);


#ifdef SIM_WITH_GUI
    if ( (App::mainWindow!=nullptr)&&(cmdIn->cmdId==MENUBAR_COLOR_UITHREADCMD) )
    {
        QMenuBar* menubar=App::mainWindow->menuBar();
        if (menubar!=nullptr)
        {
            static QPalette originalPalette;
            static bool first=true;
            QPalette pal;
            if (cmdIn->uintParams.size()>=3)
            {
                if (first)
                    originalPalette=App::mainWindow->menuBar()->palette();
                first=false;
                pal.setColor(QPalette::Window,QColor(cmdIn->uintParams[0],cmdIn->uintParams[1],cmdIn->uintParams[2]));
            }
            else
                pal=originalPalette;
            menubar->setPalette(pal);
        }
    }

    if ( (App::mainWindow!=nullptr)&&(cmdIn->cmdId==SHOW_HIDE_EMERGENCY_STOP_BUTTON_UITHREADCMD) )
        cmdOut->boolParams.push_back(showOrHideEmergencyStop(cmdIn->boolParams[0],cmdIn->stringParams[0].c_str()));

    if ( (!App::isFullScreen())&&(App::mainWindow!=nullptr)&&(cmdIn->cmdId==DISPLAY_MESH_DECIMATION_DIALOG_UITHREADCMD) )
    {
        CQDlgSlider theDialog(App::mainWindow);
        theDialog.opMode=0;
        theDialog.triCnt=cmdIn->intParams[0];
        theDialog.decimationPercent=cmdIn->floatParams[0];
        theDialog.refresh();
        bool cancel=(theDialog.makeDialogModal()==VDIALOG_MODAL_RETURN_CANCEL);
        cmdOut->boolParams.push_back(!cancel);
        cmdOut->floatParams.push_back(theDialog.decimationPercent);
    }

    if ( (!App::isFullScreen())&&(App::mainWindow!=nullptr)&&(cmdIn->cmdId==DISPLAY_OR_HIDE_PROGRESS_DIALOG_UITHREADCMD) )
        showOrHideProgressBar(cmdIn->boolParams[0],cmdIn->floatParams[0],cmdIn->stringParams[0].c_str());

    if ( (!App::isFullScreen())&&(App::mainWindow!=nullptr)&&(cmdIn->cmdId==DISPLAY_MESH_INSIDE_EXTRACTION_ITERATIONS_DIALOG_UITHREADCMD) )
    {
        CQDlgSlider2 theDialog(App::mainWindow);
        theDialog.opMode=0;
        theDialog.resolution=cmdIn->intParams[0];
        theDialog.iterationCnt=cmdIn->intParams[1];
        theDialog.refresh();
        bool cancel=(theDialog.makeDialogModal()==VDIALOG_MODAL_RETURN_CANCEL);
        cmdOut->boolParams.push_back(!cancel);
        cmdOut->intParams.push_back(theDialog.resolution);
        cmdOut->intParams.push_back(theDialog.iterationCnt);
    }

    if (cmdIn->cmdId==COPY_TEXT_TO_CLIPBOARD_UITHREADCMD)
        VVarious::copyTextToClipboard(cmdIn->stringParams[0].c_str());

    if ( (!App::isFullScreen())&&(App::mainWindow!=nullptr)&&(cmdIn->cmdId==DISPLAY_CONVEX_DECOMPOSITION_DIALOG_UITHREADCMD) )
    {
        CQDlgConvexDecomposition theDialog(App::mainWindow);
        if (cmdIn->boolParams.size()>0)
        { // we want to apply the values passed as initial dialog settings:
            theDialog.addExtraDistPoints=cmdIn->boolParams[0];
            theDialog.addFacesPoints=cmdIn->boolParams[1];
            theDialog.nClusters=cmdIn->intParams[0];
            theDialog.maxHullVertices=cmdIn->intParams[1];
            theDialog.maxConcavity=cmdIn->floatParams[0];
            theDialog.smallClusterThreshold=cmdIn->floatParams[1];
            theDialog.maxTrianglesInDecimatedMesh=cmdIn->intParams[2];
            theDialog.maxConnectDist=cmdIn->floatParams[2];
            theDialog.individuallyConsiderMultishapeComponents=cmdIn->boolParams[2];
            // reserved=cmdIn->boolParams[3];
            theDialog.maxIterations=cmdIn->intParams[3];
            // reserved=cmdIn->boolParams[4];
            theDialog.useHACD=cmdIn->boolParams[5];
            theDialog.resolution=cmdIn->intParams[4];
            theDialog.depth=cmdIn->intParams[5];
            theDialog.concavity=cmdIn->floatParams[3];
            theDialog.planeDownsampling=cmdIn->intParams[6];
            theDialog.convexHullDownsampling=cmdIn->intParams[7];
            theDialog.alpha=cmdIn->floatParams[4];
            theDialog.beta=cmdIn->floatParams[5];
            theDialog.gamma=cmdIn->floatParams[6];
            theDialog.pca=cmdIn->boolParams[6];
            theDialog.voxelBasedMode=cmdIn->boolParams[7];
            theDialog.maxNumVerticesPerCH=cmdIn->intParams[8];
            theDialog.minVolumePerCH=cmdIn->floatParams[7];
        }

        theDialog.refresh();
        bool cancel=(theDialog.makeDialogModal()==VDIALOG_MODAL_RETURN_CANCEL);

        cmdOut->boolParams.push_back(theDialog.addExtraDistPoints);
        cmdOut->boolParams.push_back(theDialog.addFacesPoints);
        cmdOut->intParams.push_back(theDialog.nClusters);
        cmdOut->intParams.push_back(theDialog.maxHullVertices);
        cmdOut->floatParams.push_back(theDialog.maxConcavity);
        cmdOut->floatParams.push_back(theDialog.smallClusterThreshold);
        cmdOut->intParams.push_back(theDialog.maxTrianglesInDecimatedMesh);
        cmdOut->floatParams.push_back(theDialog.maxConnectDist);
        cmdOut->boolParams.push_back(theDialog.individuallyConsiderMultishapeComponents);
        cmdOut->boolParams.push_back(false);
        cmdOut->intParams.push_back(theDialog.maxIterations);
        cmdOut->boolParams.push_back(cancel);
        cmdOut->boolParams.push_back(theDialog.useHACD);
        cmdOut->intParams.push_back(theDialog.resolution);
        cmdOut->intParams.push_back(theDialog.depth);
        cmdOut->floatParams.push_back(theDialog.concavity);
        cmdOut->intParams.push_back(theDialog.planeDownsampling);
        cmdOut->intParams.push_back(theDialog.convexHullDownsampling);
        cmdOut->floatParams.push_back(theDialog.alpha);
        cmdOut->floatParams.push_back(theDialog.beta);
        cmdOut->floatParams.push_back(theDialog.gamma);
        cmdOut->boolParams.push_back(theDialog.pca);
        cmdOut->boolParams.push_back(theDialog.voxelBasedMode);
        cmdOut->intParams.push_back(theDialog.maxNumVerticesPerCH);
        cmdOut->floatParams.push_back(theDialog.minVolumePerCH);
    }

    if (App::mainWindow!=nullptr)
    {
        if (cmdIn->cmdId==INSTANCE_PASS_FROM_UITHREAD_UITHREADCMD)
        {
            int auxData[4]={0,0,0,0};
            void* replyBuffer=CPluginContainer::sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_uipass,auxData,nullptr,nullptr);
            if (replyBuffer!=nullptr)
                simReleaseBuffer_internal((simChar*)replyBuffer);
        }
        if (cmdIn->cmdId==INSTANCE_ABOUT_TO_BE_CREATED_UITHREADCMD)
            App::mainWindow->newInstanceAboutToBeCreated();
        if (cmdIn->cmdId==INSTANCE_WAS_JUST_CREATED_UITHREADCMD)
            App::mainWindow->newInstanceWasJustCreated();
        if (cmdIn->cmdId==INSTANCE_ABOUT_TO_CHANGE_UITHREADCMD)
            App::mainWindow->instanceAboutToChange(cmdIn->intParams[0]);
        if (cmdIn->cmdId==INSTANCE_HAS_CHANGE_UITHREADCMD)
            App::mainWindow->instanceHasChanged(cmdIn->intParams[0]);
        if (cmdIn->cmdId==INSTANCE_ABOUT_TO_BE_DESTROYED_UITHREADCMD)
            App::mainWindow->instanceAboutToBeDestroyed(cmdIn->intParams[0]);
        if (cmdIn->cmdId==NEW_SCENE_NAME_UITHREADCMD)
            App::mainWindow->newSceneNameWasSet(cmdIn->stringParams[0].c_str());
        if (cmdIn->cmdId==SIMULATION_ABOUT_TO_START_UITHREADCMD)
            App::mainWindow->simulationAboutToStart();
        if (cmdIn->cmdId==SIMULATION_JUST_ENDED_UITHREADCMD)
            App::mainWindow->simulationEnded();
        if (cmdIn->cmdId==EDIT_MODE_ABOUT_TO_START_UITHREADCMD)
            App::mainWindow->editModeAboutToStart();
        if (cmdIn->cmdId==EDIT_MODE_JUST_ENDED_UITHREADCMD)
            App::mainWindow->editModeEnded();
    }

    if (cmdIn->cmdId==LOG_MSG_TO_STATUSBAR_UITHREADCMD)
        App::_logMsgToStatusbar(cmdIn->stringParams[0].c_str(),cmdIn->boolParams[0]);
    if ( (App::mainWindow!=nullptr)&&(cmdIn->cmdId==CLEAR_STATUSBAR_UITHREADCMD) )
        App::clearStatusbar();

    if ( (App::mainWindow!=nullptr)&&(cmdIn->cmdId==VISUALIZATION_OFF_UITHREADCMD) )
        App::mainWindow->setOpenGlDisplayEnabled(false);

    if ( (App::mainWindow!=nullptr)&&(cmdIn->cmdId==VISUALIZATION_ON_UITHREADCMD) )
        App::mainWindow->setOpenGlDisplayEnabled(true);

    if ( (App::mainWindow!=nullptr)&&(cmdIn->cmdId==TOGGLE_VISUALIZATION_UITHREADCMD) )
        App::mainWindow->setOpenGlDisplayEnabled(!App::mainWindow->getOpenGlDisplayEnabled());

    if ( (App::mainWindow!=nullptr)&&(cmdIn->cmdId==OPEN_OR_CLOSE_UITHREADCMD) )
        App::mainWindow->dlgCont->processCommand(cmdIn->intParams[0]);

    if ( (App::mainWindow!=nullptr)&&(cmdIn->cmdId==CREATE_DEFAULT_MENU_BAR_UITHREADCMD) )
        App::mainWindow->createDefaultMenuBar();

    if ( (!App::isFullScreen())&&(App::mainWindow!=nullptr)&&(cmdIn->cmdId==KEEP_THUMBNAIL_QUESTION_DLG_UITHREADCMD) )
    {
        CQDlgModelThumbnailVisu dlg;
        dlg.applyThumbnail(&App::currentWorld->environment->modelThumbnail_notSerializedHere);
        cmdOut->boolParams.push_back(dlg.makeDialogModal()!=VDIALOG_MODAL_RETURN_CANCEL);
    }

    if ( (!App::isFullScreen())&&(App::mainWindow!=nullptr)&&(cmdIn->cmdId==SELECT_THUMBNAIL_DLG_UITHREADCMD) )
    {
        CQDlgModelThumbnail dlg;
        dlg.modelBaseDummyID=cmdIn->intParams[0];
        dlg.initialize();
        dlg.actualizeBitmap();
        if (dlg.makeDialogModal()!=VDIALOG_MODAL_RETURN_CANCEL)
        {
            // We first apply the thumbnail in the UI thread scene (needed), then post a message for the sim thread
            App::currentWorld->environment->modelThumbnail_notSerializedHere.copyFrom(&dlg.thumbnail);
            SSimulationThreadCommand cmd;
            cmd.cmdId=SET_THUMBNAIL_GUITRIGGEREDCMD;
            unsigned char* img=(unsigned char*)dlg.thumbnail.getPointerToUncompressedImage();
            for (size_t i=0;i<128*128*4;i++)
                cmd.uint8Params.push_back(img[i]);
            App::appendSimulationThreadCommand(cmd);
            cmdOut->boolParams.push_back(false);
            cmdOut->boolParams.push_back(!dlg.thumbnailIsFromFile);
        }
        else
        {
            cmdOut->boolParams.push_back(true);
            cmdOut->boolParams.push_back(false);
        }
    }
    if ( (!App::isFullScreen())&&(App::mainWindow!=nullptr)&&(cmdIn->cmdId==HEIGHTFIELD_DIMENSION_DLG_UITHREADCMD) )
    {
        CQDlgHeightfieldDimension theDialog(App::mainWindow);
        theDialog.xSize=cmdIn->floatParams[0];
        theDialog.xSizeTimesThisGivesYSize=cmdIn->floatParams[1];
        theDialog.ySize=theDialog.xSize*theDialog.xSizeTimesThisGivesYSize;
        theDialog.refresh();
        theDialog.makeDialogModal();
        cmdOut->floatParams.push_back(theDialog.xSize);
        cmdOut->floatParams.push_back(theDialog.zScaling);
    }
    if ( (!App::isFullScreen())&&(App::mainWindow!=nullptr)&&(cmdIn->cmdId==OPEN_MODAL_SCRIPT_SIMULATION_PARAMETERS_UITHREADCMD) )
    {
        CSceneObject* object=App::currentWorld->sceneObjects->getObjectFromHandle(cmdIn->intParams[0]);
        if (object!=nullptr)
        {
            CQDlgUserParameters theDialog(App::mainWindow);
            theDialog.object=object;
            theDialog.refresh();
            theDialog.makeDialogModal(); // modifications are done in here directly
            // We however still need to report the changes to the SIM thread ressources:
            SSimulationThreadCommand cmd;
            cmd.cmdId=SET_ALL_SCRIPTSIMULPARAMETERGUITRIGGEREDCMD;
            cmd.intParams.push_back(cmdIn->intParams[0]);
            CUserParameters* sp=object->getUserScriptParameterObject();
            for (size_t i=0;i<sp->userParamEntries.size();i++)
            {
                cmd.intParams.push_back(sp->userParamEntries[i].properties);
                cmd.stringParams.push_back(sp->userParamEntries[i].name);
                cmd.stringParams.push_back(sp->userParamEntries[i].unit);
                cmd.stringParams.push_back(sp->userParamEntries[i].value);
            }
            App::appendSimulationThreadCommand(cmd);
        }
    }
    if ( (!App::isFullScreen())&&(App::mainWindow!=nullptr)&&(cmdIn->cmdId==OPEN_MODAL_MODEL_PROPERTIES_UITHREADCMD) )
    {
        CSceneObject* it=App::currentWorld->sceneObjects->getObjectFromHandle(cmdIn->intParams[0]);
        if (it!=nullptr)
        {
            CQDlgModelProperties theDialog(App::mainWindow);
            theDialog.modelBaseObject=it;
            theDialog.refresh();
            theDialog.makeDialogModal(); // things are modified / messages sent in the modal dlg
        }
    }
//    if ( (App::mainWindow!=nullptr)&&(cmdIn->cmdId==FLASH_STATUSBAR_UITHREADCMD) )
//        App::mainWindow->flashStatusbar();

    if ( (App::mainWindow!=nullptr)&&(cmdIn->cmdId==OPEN_HIERARCHY_UITHREADCMD) )
        App::mainWindow->dlgCont->processCommand(OPEN_HIERARCHY_DLG_CMD);

    if ( (App::mainWindow!=nullptr)&&(cmdIn->cmdId==CLOSE_HIERARCHY_UITHREADCMD) )
        App::mainWindow->dlgCont->processCommand(CLOSE_HIERARCHY_DLG_CMD);

    if ( (App::mainWindow!=nullptr)&&(cmdIn->cmdId>MAIN_WINDOW_START_MWUITHREADCMD)&&(cmdIn->cmdId<MAIN_WINDOW_END_MWUITHREADCMD) )
        App::mainWindow->executeCommand(cmdIn,cmdOut);

    if ( (!App::isFullScreen())&&(App::mainWindow!=nullptr)&&(cmdIn->cmdId==DISPLAY_FILE_DLG_UITHREADCMD) )
        cmdOut->stringParams.push_back(getOpenOrSaveFileName_api(cmdIn->intParams[0],cmdIn->stringParams[0].c_str(),cmdIn->stringParams[1].c_str(),cmdIn->stringParams[2].c_str(),cmdIn->stringParams[3].c_str(),cmdIn->stringParams[4].c_str()));

    if ( (App::mainWindow!=nullptr)&&(cmdIn->cmdId==REFRESH_DIALOGS_UITHREADCMD) )
        App::mainWindow->refreshDialogs_uiThread();

    if ( (App::mainWindow!=nullptr)&&(cmdIn->cmdId==CALL_DIALOG_FUNCTION_UITHREADCMD) )
        App::mainWindow->callDialogFunction(cmdIn,cmdOut);


    if ( (App::mainWindow!=nullptr)&&(!App::isFullScreen())&&(cmdIn->cmdId==DISPLAY_MSG_WITH_CHECKBOX_UITHREADCMD) )
        cmdOut->boolParams.push_back(messageBox_checkbox((QWidget*)cmdIn->objectParams[0],cmdIn->stringParams[0].c_str(),cmdIn->stringParams[1].c_str(),cmdIn->stringParams[2].c_str(),cmdIn->boolParams[0]));

    if ( (App::mainWindow!=nullptr)&&(!App::isFullScreen())&&(cmdIn->cmdId==DISPLAY_MSGBOX_UITHREADCMD) )
        cmdOut->uintParams.push_back(_messageBox(cmdIn->intParams[0],(QWidget*)cmdIn->objectParams[0],cmdIn->stringParams[0].c_str(),cmdIn->stringParams[1].c_str(),cmdIn->uintParams[0],cmdIn->uintParams[1]));

    if ( (App::mainWindow!=nullptr)&&(!App::isFullScreen())&&(cmdIn->cmdId==DISPLAY_SAVE_DLG_UITHREADCMD) )
        cmdOut->stringParams.push_back(getSaveFileName((QWidget*)cmdIn->objectParams[0],cmdIn->uintParams[0],cmdIn->stringParams[0].c_str(),cmdIn->stringParams[1].c_str(),cmdIn->stringParams[2].c_str(),cmdIn->boolParams[0],cmdIn->stringParams[3].c_str(),cmdIn->stringParams[4].c_str(),cmdIn->stringParams[5].c_str(),cmdIn->stringParams[6].c_str(),cmdIn->stringParams[7].c_str(),cmdIn->stringParams[8].c_str(),cmdIn->stringParams[9].c_str(),cmdIn->stringParams[10].c_str(),cmdIn->stringParams[11].c_str(),cmdIn->stringParams[12].c_str(),cmdIn->stringParams[13].c_str()));

    if ( (App::mainWindow!=nullptr)&&(!App::isFullScreen())&&(cmdIn->cmdId==DLG_INPUT_GET_FLOAT_UITHREADCMD) )
    {
        cmdOut->floatParams.push_back(0.0f);
        cmdOut->boolParams.push_back(dialogInputGetFloat((QWidget*)cmdIn->objectParams[0],cmdIn->stringParams[0].c_str(),cmdIn->stringParams[1].c_str(),cmdIn->floatParams[0],cmdIn->floatParams[1],cmdIn->floatParams[2],cmdIn->intParams[0],&cmdOut->floatParams[0]));
    }

    if ( (App::mainWindow!=nullptr)&&(!App::isFullScreen())&&(cmdIn->cmdId==DISPLAY_OPEN_DLG_UITHREADCMD) )
        cmdOut->stringParams.push_back(getOpenFileName((QWidget*)cmdIn->objectParams[0],cmdIn->uintParams[0],cmdIn->stringParams[0].c_str(),cmdIn->stringParams[1].c_str(),cmdIn->stringParams[2].c_str(),cmdIn->boolParams[0],cmdIn->stringParams[3].c_str(),cmdIn->stringParams[4].c_str(),cmdIn->stringParams[5].c_str(),cmdIn->stringParams[6].c_str(),cmdIn->stringParams[7].c_str(),cmdIn->stringParams[8].c_str(),cmdIn->stringParams[9].c_str(),cmdIn->stringParams[10].c_str(),cmdIn->stringParams[11].c_str(),cmdIn->stringParams[12].c_str(),cmdIn->stringParams[13].c_str()));

    if ( (App::mainWindow!=nullptr)&&(!App::isFullScreen())&&(cmdIn->cmdId==DISPLAY_OPEN_DLG_MULTIFILE_UITHREADCMD) )
        getOpenFileNames(cmdOut->stringParams,(QWidget*)cmdIn->objectParams[0],cmdIn->uintParams[0],cmdIn->stringParams[0].c_str(),cmdIn->stringParams[1].c_str(),cmdIn->stringParams[2].c_str(),cmdIn->boolParams[0],cmdIn->stringParams[3].c_str(),cmdIn->stringParams[4].c_str(),cmdIn->stringParams[5].c_str(),cmdIn->stringParams[6].c_str(),cmdIn->stringParams[7].c_str(),cmdIn->stringParams[8].c_str(),cmdIn->stringParams[9].c_str(),cmdIn->stringParams[10].c_str(),cmdIn->stringParams[11].c_str(),cmdIn->stringParams[12].c_str(),cmdIn->stringParams[13].c_str());

    if ( (App::mainWindow!=nullptr)&&(!App::isFullScreen())&&(cmdIn->cmdId==SET_FILEDIALOG_NATIVE_UITHREADCMD) )
        setFileDialogsNative(cmdIn->intParams[0]);

    if ( (App::mainWindow!=nullptr)&&(!App::isFullScreen())&&(cmdIn->cmdId==SHOW_PRIMITIVE_SHAPE_DLG_UITHREADCMD) )
    {
        C3Vector sizes;
        int subdiv[3];
        int faces,sides,discSubdiv,openEnds;
        bool smooth,dynamic,pure,cone;
        float density;
        if (showPrimitiveShapeDialog(cmdIn->intParams[0],(C3Vector*)cmdIn->objectParams[0],sizes,subdiv,faces,sides,discSubdiv,smooth,openEnds,dynamic,pure,cone,density))
        {
            cmdOut->posParams.push_back(sizes);
            cmdOut->intParams.push_back(subdiv[0]);
            cmdOut->intParams.push_back(subdiv[1]);
            cmdOut->intParams.push_back(subdiv[2]);
            cmdOut->intParams.push_back(faces);
            cmdOut->intParams.push_back(sides);
            cmdOut->intParams.push_back(discSubdiv);
            cmdOut->boolParams.push_back(smooth);
            cmdOut->intParams.push_back(openEnds);
            cmdOut->boolParams.push_back(pure);
            cmdOut->boolParams.push_back(cone);
            cmdOut->floatParams.push_back(density);
            cmdOut->boolParams.push_back(false); // was neg volume
            cmdOut->boolParams.push_back(dynamic);
        }
    }
#endif

#ifdef SIM_WITH_SERIAL
    if ( (cmdIn->cmdId>SERIAL_PORT_START_SPUITHREADCMD)&&(cmdIn->cmdId<SERIAL_PORT_END_SPUITHREADCMD) )
        App::worldContainer->serialPortContainer->executeCommand(cmdIn,cmdOut);
#endif

}

void CUiThread::showOrHideProgressBar(bool show,float pos,const char* txt)
{ // pos and txt can be omitted (then previously provided values will be used)
    TRACE_INTERNAL;
#ifdef SIM_WITH_GUI
    if ( App::userSettings->doNotShowProgressBars||(App::getDlgVerbosity()<sim_verbosity_infos) )
        return;
    static float p=0.0f;
    static std::string t("");
    if (pos<101.0f)
        p=pos;
    if (txt!=nullptr)
        t=txt;
    if ( (App::mainWindow!=nullptr)&&(!App::isFullScreen()) )
    { // make sure we are not in headless mode
        if (VThread::isCurrentThreadTheUiThread())
        { // we are in the UI thread. We execute the command now:
            static CQDlgProgress* theDialog=nullptr;
            if (show)
            { // Show/update the dialog:
                if (theDialog==nullptr)
                {
                    theDialog=new CQDlgProgress(App::mainWindow);
                    theDialog->setModal(true);
                    theDialog->show();
                }
                theDialog->updateProgress(p,t.c_str());
            }
            else
            { // hide the dialog again:
                if (theDialog!=nullptr)
                    delete theDialog;
                theDialog=nullptr;
            }
        }
        else
        { // We are NOT in the UI thread. We execute the command via the UI thread:
            SUIThreadCommand cmdIn;
            SUIThreadCommand cmdOut;
            cmdIn.cmdId=DISPLAY_OR_HIDE_PROGRESS_DIALOG_UITHREADCMD;
            cmdIn.boolParams.push_back(show);
            cmdIn.floatParams.push_back(p);
            cmdIn.stringParams.push_back(t);
            executeCommandViaUiThread(&cmdIn,&cmdOut);
        }
    }
#endif
}

/*
bool CUiThread::showOrHideEmergencyStop(bool show,const char* txt)
{
    TRACE_INTERNAL;
#ifdef SIM_WITH_GUI
    if ( (App::mainWindow!=nullptr)&&(!App::isFullScreen()) )
    { // make sure we are not in headless mode
        if (VThread::isCurrentThreadTheUiThread())
        { // we are in the UI thread. We execute the command now:
            bool buttonWasPressed=false;
            static CQDlgStopScripts* _emergencyStopDlg=nullptr;
            if (show)
            {
                if (_emergencyStopDlg==nullptr)
                {
                    CQDlgStopScripts::stopScriptNow=false;
                    _emergencyStopDlg=new CQDlgStopScripts(App::mainWindow);
                    _emergencyStopDlg->setScriptName(txt);
                    _emergencyStopDlg->show();
                }
                buttonWasPressed=CQDlgStopScripts::stopScriptNow;
                if (buttonWasPressed)
                {
                    delete _emergencyStopDlg;
                    _emergencyStopDlg=nullptr;
                    CQDlgStopScripts::stopScriptNow=false;
                }
            }
            else
            {
                if (_emergencyStopDlg!=nullptr)
                {
                    delete _emergencyStopDlg;
                    _emergencyStopDlg=nullptr;
                }
            }
            return(buttonWasPressed);
        }
        else
        { // We are NOT in the UI thread. We execute the command via the UI thread:
            SUIThreadCommand cmdIn;
            SUIThreadCommand cmdOut;
            cmdIn.cmdId=SHOW_HIDE_EMERGENCY_STOP_BUTTON_UITHREADCMD;
            cmdIn.boolParams.push_back(show);
            cmdIn.stringParams.push_back(txt);
            executeCommandViaUiThread(&cmdIn,&cmdOut);
            return(cmdOut.boolParams[0]);
        }
    }
#endif
    return(false);
}
*/

bool CUiThread::showOrHideEmergencyStop(bool show,const char* txt)
{
    TRACE_INTERNAL;
    bool retVal=false; // button was pressed
#ifdef SIM_WITH_GUI
    if ( (App::mainWindow!=nullptr)&&(!App::isFullScreen()) )
    { // make sure we are not in headless mode
        static CQDlgStopScripts* _emergencyStopDlg=nullptr;
        if (show)
        {
            if (_emergencyStopDlg==nullptr)
            { // need to show it
                if (VThread::isCurrentThreadTheUiThread())
                {
                    CQDlgStopScripts::stopScriptNow=false;
                    _emergencyStopDlg=new CQDlgStopScripts(App::mainWindow);
                    _emergencyStopDlg->setScriptName(txt);
                    _emergencyStopDlg->show();
                }
                else
                { // We are NOT in the UI thread. We execute the command via the UI thread:
                    SUIThreadCommand cmdIn;
                    SUIThreadCommand cmdOut;
                    cmdIn.cmdId=SHOW_HIDE_EMERGENCY_STOP_BUTTON_UITHREADCMD;
                    cmdIn.boolParams.push_back(show);
                    cmdIn.stringParams.push_back(txt);
                    executeCommandViaUiThread(&cmdIn,&cmdOut);
                }
            }
            else
            { // already showing it
                retVal=CQDlgStopScripts::stopScriptNow;
                if (retVal)
                { // hide the dlg
                    if (VThread::isCurrentThreadTheUiThread())
                    {
                        delete _emergencyStopDlg;
                        _emergencyStopDlg=nullptr;
                        CQDlgStopScripts::stopScriptNow=false;
                    }
                    else
                    {
                        SUIThreadCommand cmdIn;
                        SUIThreadCommand cmdOut;
                        cmdIn.cmdId=SHOW_HIDE_EMERGENCY_STOP_BUTTON_UITHREADCMD;
                        cmdIn.boolParams.push_back(false);
                        cmdIn.stringParams.push_back("");
                        executeCommandViaUiThread(&cmdIn,&cmdOut);
                    }
                }
            }
        }
        else
        {
            if (_emergencyStopDlg!=nullptr)
            {
                if (VThread::isCurrentThreadTheUiThread())
                {
                    delete _emergencyStopDlg;
                    _emergencyStopDlg=nullptr;
                    CQDlgStopScripts::stopScriptNow=false;
                }
                else
                {
                    SUIThreadCommand cmdIn;
                    SUIThreadCommand cmdOut;
                    cmdIn.cmdId=SHOW_HIDE_EMERGENCY_STOP_BUTTON_UITHREADCMD;
                    cmdIn.boolParams.push_back(false);
                    cmdIn.stringParams.push_back("");
                    executeCommandViaUiThread(&cmdIn,&cmdOut);
                }
            }
        }
    }
#endif
    return(retVal);
}

#ifdef SIM_WITH_GUI
void CUiThread::requestSceneRender_wait()
{ // is called by the non-UI thread
    TRACE_INTERNAL;
    if (App::mainWindow!=nullptr)
    { // make sure we are not in headless mode
        _frameId++;
        _requestSceneRender_wait();
        while (_frameId!=_lastFrameId)
        {
            VThread::sleep(1);
            // App::simThread->executeMessages();
        }
        _lastFrameId=_frameId;
    }
}

void CUiThread::__requestSceneRender_wait()
{ // is called by the UI thread.
    TRACE_INTERNAL;
    if ((_frameId!=_lastFrameId)&&(App::mainWindow!=nullptr))
    {
        App::mainWindow->uiThread_renderScene();
        _lastFrameId=_frameId;
    }
}

int CUiThread::getLastFrameId()
{
    return(_lastFrameId);
}

void CUiThread::setLastFrameId(int fid)
{
    _lastFrameId=fid;
}

std::string CUiThread::getOpenOrSaveFileName_api(int mode,const char* title,const char* startPath,const char* initName,const char* extName,const char* ext)
{ // mode= 1: save, 0: load single, >1: load multiple
    TRACE_INTERNAL;
    std::string retVal;
    if ( (App::mainWindow!=nullptr)&&(!App::isFullScreen()) )
    {
        if (VThread::isCurrentThreadTheUiThread())
        { // we are in the UI thread. We execute the command now:
            if (mode==sim_filedlg_type_save)
                retVal=VFileDialog::getSaveFileName(App::mainWindow,0,title,startPath,initName,false,extName,ext);
            if ( (mode==sim_filedlg_type_load)||(mode==sim_filedlg_type_load_multiple) )
            {
                QString _ext(ext);
                QStringList e(_ext.split(";"));
                while (e.size()<10)
                    e.append("");
                if (mode==sim_filedlg_type_load)
                    retVal=VFileDialog::getOpenFileName(App::mainWindow,0,title,startPath,initName,false,extName,e[0].toStdString().c_str(),e[1].toStdString().c_str(),e[2].toStdString().c_str(),e[3].toStdString().c_str(),e[4].toStdString().c_str(),e[5].toStdString().c_str(),e[6].toStdString().c_str(),e[7].toStdString().c_str(),e[8].toStdString().c_str(),e[9].toStdString().c_str());
                else
                {
                    std::vector<std::string> files;
                    VFileDialog::getOpenFileNames(files,App::mainWindow,0,title,startPath,initName,false,extName,e[0].toStdString().c_str(),e[1].toStdString().c_str(),e[2].toStdString().c_str(),e[3].toStdString().c_str(),e[4].toStdString().c_str(),e[5].toStdString().c_str(),e[6].toStdString().c_str(),e[7].toStdString().c_str(),e[8].toStdString().c_str(),e[9].toStdString().c_str());
                    for (size_t i=0;i<files.size();i++)
                    {
                        retVal+=files[i];
                        if (i<files.size()-1)
                            retVal+=";";
                    }
                }
            }
            if (mode==sim_filedlg_type_folder)
                retVal=VFileDialog::getExistingFolder(App::mainWindow,title,startPath);
        }
        else
        { // We are NOT in the UI thread. We execute the command via the UI thread:
            SUIThreadCommand cmdIn;
            SUIThreadCommand cmdOut;
            cmdIn.cmdId=DISPLAY_FILE_DLG_UITHREADCMD;
            cmdIn.intParams.push_back(mode);
            cmdIn.stringParams.push_back(title);
            cmdIn.stringParams.push_back(startPath);
            cmdIn.stringParams.push_back(initName);
            cmdIn.stringParams.push_back(extName);
            cmdIn.stringParams.push_back(ext);
            executeCommandViaUiThread(&cmdIn,&cmdOut);
            retVal=cmdOut.stringParams[0];
        }
    }
    return(retVal);
}

unsigned short CUiThread::messageBox_informationSystemModal(void* parentWidget,const char* title,const char* message,unsigned short flags,unsigned short defaultAnswer)
{
    TRACE_INTERNAL;
    return(_messageBox(4,parentWidget,title,message,flags,defaultAnswer));
}

unsigned short CUiThread::messageBox_information(void* parentWidget,const char* title,const char* message,unsigned short flags,unsigned short defaultAnswer)
{
    TRACE_INTERNAL;
    return(_messageBox(0,parentWidget,title,message,flags,defaultAnswer));
}

unsigned short CUiThread::messageBox_question(void* parentWidget,const char* title,const char* message,unsigned short flags,unsigned short defaultAnswer)
{
    TRACE_INTERNAL;
    return(_messageBox(1,parentWidget,title,message,flags,defaultAnswer));
}

unsigned short CUiThread::messageBox_warning(void* parentWidget,const char* title,const char* message,unsigned short flags,unsigned short defaultAnswer)
{
    TRACE_INTERNAL;
    return(_messageBox(2,parentWidget,title,message,flags,defaultAnswer));
}

unsigned short CUiThread::messageBox_critical(void* parentWidget,const char* title,const char* message,unsigned short flags,unsigned short defaultAnswer)
{
    TRACE_INTERNAL;
    return(_messageBox(3,parentWidget,title,message,flags,defaultAnswer));
}

unsigned short CUiThread::_messageBox(int type,void* parentWidget,const char* title,const char* message,unsigned short flags,unsigned short defaultAnswer)
{ // type: 0=info, 1=question, 2=warning, 3=critical, 4=info, system modal
    TRACE_INTERNAL;
    unsigned short retVal=VMESSAGEBOX_REPLY_ERROR;
    if ( (App::mainWindow!=nullptr)&&(!App::isFullScreen()) )
    { // make sure we are not in headless mode
        if (VThread::isCurrentThreadTheUiThread())
        { // we are in the UI thread. We execute the command now:
            if (type==0)
                retVal=VMessageBox::information((QWidget*)parentWidget,title,message,flags,defaultAnswer);
            if (type==1)
                retVal=VMessageBox::question((QWidget*)parentWidget,title,message,flags,defaultAnswer);
            if (type==2)
                retVal=VMessageBox::warning((QWidget*)parentWidget,title,message,flags,defaultAnswer);
            if (type==3)
                retVal=VMessageBox::critical((QWidget*)parentWidget,title,message,flags,defaultAnswer);
            if (type==4)
                retVal=VMessageBox::informationSystemModal((QWidget*)parentWidget,title,message,flags,defaultAnswer);
        }
        else
        { // We are NOT in the UI thread. We execute the command via the UI thread:
            SUIThreadCommand cmdIn;
            SUIThreadCommand cmdOut;
            cmdIn.cmdId=DISPLAY_MSGBOX_UITHREADCMD;
            cmdIn.intParams.push_back(type);
            cmdIn.objectParams.push_back(parentWidget);
            cmdIn.stringParams.push_back(title);
            cmdIn.stringParams.push_back(message);
            cmdIn.uintParams.push_back(flags);
            cmdIn.uintParams.push_back(defaultAnswer);
            executeCommandViaUiThread(&cmdIn,&cmdOut);
            if (cmdOut.uintParams.size()>0)
                retVal=cmdOut.uintParams[0];
        }
    }
    return(retVal);
}

bool CUiThread::messageBox_checkbox(void* parentWidget,const char* title,const char* message,const char* checkboxMessage,bool isWarning)
{
    TRACE_INTERNAL;
    bool retVal=false;
    if ( (App::mainWindow!=nullptr)&&(!App::isFullScreen()) )
    { // make sure we are not in headless mode
        if (VThread::isCurrentThreadTheUiThread())
        { // we are in the UI thread. We execute the command now:
            int v=sim_verbosity_infos;
            if (isWarning)
                v=sim_verbosity_warnings;
            if (App::getDlgVerbosity()>=v)
            {
                CQDlgMessageAndCheckbox dlg((QWidget*)parentWidget);
                dlg.title=title;
                dlg.text=message;
                dlg.checkbox=checkboxMessage;
                dlg.refresh();
                dlg.makeDialogModal();
                retVal=dlg.checkboxState;
            }
            else
                retVal=false;
        }
        else
        { // We are NOT in the UI thread. We execute the command via the UI thread:
            SUIThreadCommand cmdIn;
            SUIThreadCommand cmdOut;
            cmdIn.cmdId=DISPLAY_MSG_WITH_CHECKBOX_UITHREADCMD;
            cmdIn.objectParams.push_back(parentWidget);
            cmdIn.stringParams.push_back(title);
            cmdIn.stringParams.push_back(message);
            cmdIn.stringParams.push_back(checkboxMessage);
            cmdIn.boolParams.push_back(isWarning);
            executeCommandViaUiThread(&cmdIn,&cmdOut);
            if (cmdOut.boolParams.size()>0)
                retVal=cmdOut.boolParams[0];
        }
    }
    return(retVal);
}

bool CUiThread::getOpenFileNames(std::vector<std::string>& files,void* parentWidget,unsigned short option,const char* title,const char* startPath,const char* initFilename,bool allowAnyFile,const char* extensionName,const char* extension1,const char* extension2,const char* extension3,const char* extension4,const char* extension5,const char* extension6,const char* extension7,const char* extension8,const char* extension9,const char* extension10)
{
    TRACE_INTERNAL;
    bool retVal=false;
    if ( (App::mainWindow!=nullptr)&&(!App::isFullScreen()) )
    { // make sure we are not in headless mode
        if (VThread::isCurrentThreadTheUiThread())
        { // we are in the UI thread. We execute the command now:
            retVal=VFileDialog::getOpenFileNames(files,(QWidget*)parentWidget,option,title,startPath,initFilename,allowAnyFile,extensionName,extension1,extension2,extension3,extension4,extension5,extension6,extension7,extension8,extension9,extension10);
        }
        else
        { // We are NOT in the UI thread. We execute the command via the UI thread:
            SUIThreadCommand cmdIn;
            SUIThreadCommand cmdOut;
            cmdIn.cmdId=DISPLAY_OPEN_DLG_MULTIFILE_UITHREADCMD;
            cmdIn.objectParams.push_back(parentWidget);
            cmdIn.uintParams.push_back(option);
            cmdIn.stringParams.push_back(title);
            cmdIn.stringParams.push_back(startPath);
            cmdIn.stringParams.push_back(initFilename);
            cmdIn.boolParams.push_back(allowAnyFile);
            cmdIn.stringParams.push_back(extensionName);
            cmdIn.stringParams.push_back(extension1);
            cmdIn.stringParams.push_back(extension2);
            cmdIn.stringParams.push_back(extension3);
            cmdIn.stringParams.push_back(extension4);
            cmdIn.stringParams.push_back(extension5);
            cmdIn.stringParams.push_back(extension6);
            cmdIn.stringParams.push_back(extension7);
            cmdIn.stringParams.push_back(extension8);
            cmdIn.stringParams.push_back(extension9);
            cmdIn.stringParams.push_back(extension10);
            executeCommandViaUiThread(&cmdIn,&cmdOut);
            files.clear();
            for (size_t i=0;i<cmdOut.stringParams.size();i++)
                files.push_back(cmdOut.stringParams[i]);
            retVal=(files.size()>0);
        }
    }
    return(retVal);
}

void CUiThread::setFileDialogsNative(int n)
{
    TRACE_INTERNAL;
    if (VThread::isCurrentThreadTheUiThread())
    { // we are in the UI thread. We execute the command now:
        VFileDialog::setFileDialogNative(n);
    }
    else
    { // We are NOT in the UI thread. We execute the command via the UI thread:
        SUIThreadCommand cmdIn;
        SUIThreadCommand cmdOut;
        cmdIn.cmdId=SET_FILEDIALOG_NATIVE_UITHREADCMD;
        cmdIn.intParams.push_back(n);
        executeCommandViaUiThread(&cmdIn,&cmdOut);
    }
}

std::string CUiThread::getOpenFileName(void* parentWidget,unsigned short option,const char* title,const char* startPath,const char* initFilename,bool allowAnyFile,const char* extensionName,const char* extension1,const char* extension2,const char* extension3,const char* extension4,const char* extension5,const char* extension6,const char* extension7,const char* extension8,const char* extension9,const char* extension10)
{
    TRACE_INTERNAL;
    std::string retVal;
    if ( (App::mainWindow!=nullptr)&&(!App::isFullScreen()) )
    { // make sure we are not in headless mode
        if (VThread::isCurrentThreadTheUiThread())
        { // we are in the UI thread. We execute the command now:
            retVal=VFileDialog::getOpenFileName((QWidget*)parentWidget,option,title,startPath,initFilename,allowAnyFile,extensionName,extension1,extension2,extension3,extension4,extension5,extension6,extension7,extension8,extension9,extension10);
        }
        else
        { // We are NOT in the UI thread. We execute the command via the UI thread:
            SUIThreadCommand cmdIn;
            SUIThreadCommand cmdOut;
            cmdIn.cmdId=DISPLAY_OPEN_DLG_UITHREADCMD;
            cmdIn.objectParams.push_back(parentWidget);
            cmdIn.uintParams.push_back(option);
            cmdIn.stringParams.push_back(title);
            cmdIn.stringParams.push_back(startPath);
            cmdIn.stringParams.push_back(initFilename);
            cmdIn.boolParams.push_back(allowAnyFile);
            cmdIn.stringParams.push_back(extensionName);
            cmdIn.stringParams.push_back(extension1);
            cmdIn.stringParams.push_back(extension2);
            cmdIn.stringParams.push_back(extension3);
            cmdIn.stringParams.push_back(extension4);
            cmdIn.stringParams.push_back(extension5);
            cmdIn.stringParams.push_back(extension6);
            cmdIn.stringParams.push_back(extension7);
            cmdIn.stringParams.push_back(extension8);
            cmdIn.stringParams.push_back(extension9);
            cmdIn.stringParams.push_back(extension10);
            executeCommandViaUiThread(&cmdIn,&cmdOut);
            if (cmdOut.stringParams.size()>0)
                retVal=cmdOut.stringParams[0];
        }
    }
    return(retVal);
}

std::string CUiThread::getSaveFileName(void* parentWidget,unsigned short option,const char* title,const char* startPath,const char* initFilename,bool allowAnyFile,const char* extensionName,const char* extension1,const char* extension2,const char* extension3,const char* extension4,const char* extension5,const char* extension6,const char* extension7,const char* extension8,const char* extension9,const char* extension10)
{
    TRACE_INTERNAL;
    std::string retVal;
    if ( (App::mainWindow!=nullptr)&&(!App::isFullScreen()) )
    { // make sure we are not in headless mode
        if (VThread::isCurrentThreadTheUiThread())
        { // we are in the UI thread. We execute the command now:
            retVal=VFileDialog::getSaveFileName((QWidget*)parentWidget,option,title,startPath,initFilename,allowAnyFile,extensionName,extension1,extension2,extension3,extension4,extension5,extension6,extension7,extension8,extension9,extension10);
        }
        else
        { // We are NOT in the UI thread. We execute the command via the UI thread:
            SUIThreadCommand cmdIn;
            SUIThreadCommand cmdOut;
            cmdIn.cmdId=DISPLAY_SAVE_DLG_UITHREADCMD;
            cmdIn.objectParams.push_back(parentWidget);
            cmdIn.uintParams.push_back(option);
            cmdIn.stringParams.push_back(title);
            cmdIn.stringParams.push_back(startPath);
            cmdIn.stringParams.push_back(initFilename);
            cmdIn.boolParams.push_back(allowAnyFile);
            cmdIn.stringParams.push_back(extensionName);
            cmdIn.stringParams.push_back(extension1);
            cmdIn.stringParams.push_back(extension2);
            cmdIn.stringParams.push_back(extension3);
            cmdIn.stringParams.push_back(extension4);
            cmdIn.stringParams.push_back(extension5);
            cmdIn.stringParams.push_back(extension6);
            cmdIn.stringParams.push_back(extension7);
            cmdIn.stringParams.push_back(extension8);
            cmdIn.stringParams.push_back(extension9);
            cmdIn.stringParams.push_back(extension10);
            executeCommandViaUiThread(&cmdIn,&cmdOut);
            if (cmdOut.stringParams.size()>0)
                retVal=cmdOut.stringParams[0];
        }
    }
    return(retVal);
}

bool CUiThread::dialogInputGetFloat(void* parentWidget,const char* title,const char* msg,float def,float minV,float maxV,int decimals,float* outFloat)
{
    TRACE_INTERNAL;
    bool retVal=false;
    if ( (App::mainWindow!=nullptr)&&(!App::isFullScreen()) )
    { // make sure we are not in headless mode
        if (VThread::isCurrentThreadTheUiThread())
        { // we are in the UI thread. We execute the command now:
            outFloat[0]=(float)QInputDialog::getDouble((QWidget*)parentWidget,title,msg,def,minV,maxV,decimals,&retVal);
        }
        else
        { // We are NOT in the UI thread. We execute the command via the UI thread:
            SUIThreadCommand cmdIn;
            SUIThreadCommand cmdOut;
            cmdIn.cmdId=DLG_INPUT_GET_FLOAT_UITHREADCMD;
            cmdIn.stringParams.push_back(title);
            cmdIn.stringParams.push_back(msg);
            cmdIn.floatParams.push_back(def);
            cmdIn.floatParams.push_back(minV);
            cmdIn.floatParams.push_back(maxV);
            cmdIn.intParams.push_back(decimals);
            cmdIn.objectParams.push_back(parentWidget);
            executeCommandViaUiThread(&cmdIn,&cmdOut);
            if (cmdOut.boolParams.size()>0)
            {
                outFloat[0]=cmdOut.floatParams[0];
                retVal=cmdOut.boolParams[0];
            }
        }
    }
    return(retVal);
}


bool CUiThread::showPrimitiveShapeDialog(int type,const C3Vector* optionalSizesIn,C3Vector& sizes,int subdiv[3],int& faces,int& sides,int& discSubdiv,bool& smooth,int& openEnds,bool& dynamic,bool& pure,bool& cone,float& density)
{
    TRACE_INTERNAL;
    bool retVal=false;
    if ( (App::mainWindow!=nullptr)&&(!App::isFullScreen()) )
    { // make sure we are not in headless mode
        if (VThread::isCurrentThreadTheUiThread())
        { // we are in the UI thread. We execute the command now:
            CQDlgPrimitives theDialog(App::mainWindow);
            theDialog.initialize(type,optionalSizesIn);
            retVal=(theDialog.makeDialogModal()!=VDIALOG_MODAL_RETURN_CANCEL);
            if (retVal)
            {
                sizes.set(theDialog.xSize,theDialog.ySize,theDialog.zSize);
                subdiv[0]=theDialog.subdivX;
                subdiv[1]=theDialog.subdivY;
                subdiv[2]=theDialog.subdivZ;
                faces=theDialog.faces;
                sides=theDialog.sides;
                discSubdiv=theDialog.discSubdiv;
                smooth=theDialog.smooth;
                openEnds=theDialog.openEnds;
                pure=theDialog.pure;
                cone=theDialog.cone;
                density=theDialog.density;
                dynamic=theDialog.dynamic;
            }
        }
        else
        { // We are NOT in the UI thread. We execute the command via the UI thread:
            SUIThreadCommand cmdIn;
            SUIThreadCommand cmdOut;
            cmdIn.cmdId=SHOW_PRIMITIVE_SHAPE_DLG_UITHREADCMD;
            cmdIn.intParams.push_back(type);
            cmdIn.objectParams.push_back((void*)optionalSizesIn);
            executeCommandViaUiThread(&cmdIn,&cmdOut);
            if (cmdOut.intParams.size()>0)
            {
                sizes=cmdOut.posParams[0];
                subdiv[0]=cmdOut.intParams[0];
                subdiv[1]=cmdOut.intParams[1];
                subdiv[2]=cmdOut.intParams[2];
                faces=cmdOut.intParams[3];
                sides=cmdOut.intParams[4];
                discSubdiv=cmdOut.intParams[5];
                smooth=cmdOut.boolParams[0];
                openEnds=cmdOut.intParams[6];
                pure=cmdOut.boolParams[1];
                cone=cmdOut.boolParams[2];
                density=cmdOut.floatParams[0];
                dynamic=cmdOut.boolParams[4];
                retVal=true;
            }
        }
    }
    return(retVal);
}
#endif
