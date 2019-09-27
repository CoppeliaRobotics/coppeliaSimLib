
#include "vrepMainHeader.h"
#include "funcDebug.h"
#include "uiThread.h"
#include "vThread.h"
#include "app.h"
#include "global.h"
#include "fileOperations.h"
#include "persistentDataContainer.h"
#include "tt.h"
#include "threadPool.h"
#include "pluginContainer.h"
#include "v_repStrings.h"
#include "vDateTime.h"
#include "sceneObjectOperations.h"
#include "addOperations.h"
#include "rendering.h"
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
    #include "qdlgscriptparameters.h"
    #include "qdlgstopscripts.h"
    #include "vMessageBox.h"
    #include "qdlgprogress.h"
    #include <QEvent>
    #include <QInputDialog>
#endif

CUiThread::CUiThread()
{
    _frameId=0;
    _frame_bufferMainDisplayStateVariables=false;
    _lastFrameId=0;

#ifndef SIM_WITHOUT_QT_AT_ALL
    connect(this,SIGNAL(_executeCommandViaUiThread(SUIThreadCommand*,SUIThreadCommand*)),this,SLOT(__executeCommandViaUiThread(SUIThreadCommand*,SUIThreadCommand*)),Qt::BlockingQueuedConnection);
#else
    _noSigSlot_cmdIn=nullptr;
    _noSigSlot_cmdOut=nullptr;
    _noSigSlot_cnter=0;
#endif
#ifdef SIM_WITH_GUI
    // Queued:
    connect(this,SIGNAL(_requestSceneRender(bool)),this,SLOT(__requestSceneRender(bool)),Qt::QueuedConnection);
    // Blocking:
    connect(this,SIGNAL(_requestSceneRender_wait(bool)),this,SLOT(__requestSceneRender_wait(bool)),Qt::QueuedConnection);
#endif
}

CUiThread::~CUiThread()
{
}

#ifdef SIM_WITHOUT_QT_AT_ALL
void CUiThread::processGuiEventsUntilQuit_noSignalSlots()
{
    _noSigSlotMutex.lock_simple();
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
    FUNCTION_DEBUG;
    if (!VThread::isCurrentThreadTheUiThread())
    {
#ifdef SIM_WITHOUT_QT_AT_ALL
        _noSigSlotMutex.lock_simple();
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
    FUNCTION_DEBUG;

    handleVerSpecExecuteCommandViaUiThread1(cmdIn,cmdOut);

    if ( (cmdIn->cmdId>PLUGIN_START_PLUGUITHREADCMD)&&(cmdIn->cmdId<PLUGIN_END_PLUGUITHREADCMD) )
    {
        if (cmdIn->cmdId==PLUGIN_LOAD_AND_START_PLUGUITHREADCMD)
            cmdOut->intParams.push_back(CPluginContainer::addPlugin(cmdIn->stringParams[0].c_str(),cmdIn->stringParams[1].c_str()));
        if (cmdIn->cmdId==PLUGIN_STOP_AND_UNLOAD_PLUGUITHREADCMD)
            cmdOut->boolParams.push_back(CPluginContainer::killPlugin(cmdIn->intParams[0]));
    }

    if (cmdIn->cmdId==DESTROY_GL_TEXTURE_UITHREADCMD)
        destroyGlTexture(cmdIn->uintParams[0]);

#ifdef SIM_WITH_GUI
    if (cmdIn->cmdId==JOB_NAME_UITHREADCMD)
    {
        bool ok;
        QString text=QInputDialog::getText(App::mainWindow,"Job Name","Job name:",QLineEdit::Normal,cmdIn->stringParams[0].c_str(),&ok);
        if (ok)
            cmdOut->stringParams.push_back(text.toStdString());
    }
#endif

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

    if (cmdIn->cmdId==DESTROY_UIOBJECT_UITHREADCMD)
    {
        if (cmdIn->intParams[0]==sim_object_path_type)
        {
            CPath* it=(CPath*)cmdIn->objectParams[0];
            delete it;
        }
    }

    if (cmdIn->cmdId==COPY_TEXT_TO_CLIPBOARD_UITHREADCMD)
        VVarious::copyTextToClipboard(cmdIn->stringParams[0]);

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

    if (cmdIn->cmdId==ADD_STATUSBAR_MESSAGE_UITHREADCMD)
        App::addStatusbarMessage(cmdIn->stringParams[0],cmdIn->boolParams[0]);
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
        dlg.applyThumbnail(&App::ct->environment->modelThumbnail_notSerializedHere);
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
            App::ct->environment->modelThumbnail_notSerializedHere.copyFrom(&dlg.thumbnail);
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
        CLuaScriptObject* it=App::ct->luaScriptContainer->getScriptFromID_noAddOnsNorSandbox(cmdIn->intParams[0]);
        if (it!=nullptr)
        {
            CQDlgScriptParameters theDialog(App::mainWindow);
            theDialog.script=it;
            theDialog.refresh();
            theDialog.makeDialogModal(); // modifications are done in here directly
            // We however still need to report the changes to the SIM thread ressources:
            SSimulationThreadCommand cmd;
            cmd.cmdId=SET_ALL_SCRIPTSIMULPARAMETERGUITRIGGEREDCMD;
            cmd.intParams.push_back(it->getScriptID());
            CLuaScriptParameters* sp=it->getScriptParametersObject();
            for (size_t i=0;i<sp->scriptParamEntries.size();i++)
            {
                cmd.intParams.push_back(sp->scriptParamEntries[i].properties);
                cmd.stringParams.push_back(sp->scriptParamEntries[i].name);
                cmd.stringParams.push_back(sp->scriptParamEntries[i].unit);
                cmd.stringParams.push_back(sp->scriptParamEntries[i].value);
            }
            App::appendSimulationThreadCommand(cmd);
        }
    }
    if ( (!App::isFullScreen())&&(App::mainWindow!=nullptr)&&(cmdIn->cmdId==OPEN_MODAL_MODEL_PROPERTIES_UITHREADCMD) )
    {
        C3DObject* it=App::ct->objCont->getObjectFromHandle(cmdIn->intParams[0]);
        if (it!=nullptr)
        {
            CQDlgModelProperties theDialog(App::mainWindow);
            theDialog.modelBaseObject=it;
            theDialog.refresh();
            theDialog.makeDialogModal(); // things are modified / messages sent in the modal dlg
        }
    }
    if ( (App::mainWindow!=nullptr)&&(cmdIn->cmdId==FLASH_STATUSBAR_UITHREADCMD) )
        App::mainWindow->flashStatusbar();

    if ( (App::mainWindow!=nullptr)&&(cmdIn->cmdId==OPEN_HIERARCHY_UITHREADCMD) )
        App::mainWindow->dlgCont->processCommand(OPEN_HIERARCHY_DLG_CMD);

    if ( (App::mainWindow!=nullptr)&&(cmdIn->cmdId==CLOSE_HIERARCHY_UITHREADCMD) )
        App::mainWindow->dlgCont->processCommand(CLOSE_HIERARCHY_DLG_CMD);

    if ( (App::mainWindow!=nullptr)&&(cmdIn->cmdId>MAIN_WINDOW_START_MWUITHREADCMD)&&(cmdIn->cmdId<MAIN_WINDOW_END_MWUITHREADCMD) )
        App::mainWindow->executeCommand(cmdIn,cmdOut);

    if ( (!App::isFullScreen())&&(App::mainWindow!=nullptr)&&(cmdIn->cmdId==DISPLAY_MSGBOX_API_UITHREADCMD) )
        cmdOut->intParams.push_back(messageBox_api(cmdIn->intParams[0],cmdIn->intParams[1],cmdIn->stringParams[0].c_str(),cmdIn->stringParams[1].c_str()));

    if ( (!App::isFullScreen())&&(App::mainWindow!=nullptr)&&(cmdIn->cmdId==DISPLAY_FILE_DLG_UITHREADCMD) )
        cmdOut->stringParams.push_back(getOpenOrSaveFileName_api(cmdIn->intParams[0],cmdIn->stringParams[0].c_str(),cmdIn->stringParams[1].c_str(),cmdIn->stringParams[2].c_str(),cmdIn->stringParams[3].c_str(),cmdIn->stringParams[4].c_str()));

    if ( (App::mainWindow!=nullptr)&&(cmdIn->cmdId==REFRESH_DIALOGS_UITHREADCMD) )
        App::mainWindow->refreshDialogs_uiThread();

    if ( (App::mainWindow!=nullptr)&&(cmdIn->cmdId==CALL_DIALOG_FUNCTION_UITHREADCMD) )
        App::mainWindow->callDialogFunction(cmdIn,cmdOut);


    if ( (App::mainWindow!=nullptr)&&(!App::isFullScreen())&&(cmdIn->cmdId==DISPLAY_MSG_WITH_CHECKBOX_UITHREADCMD) )
        cmdOut->boolParams.push_back(messageBox_checkbox((QWidget*)cmdIn->objectParams[0],cmdIn->stringParams[0],cmdIn->stringParams[1],cmdIn->stringParams[2]));

    if ( (App::mainWindow!=nullptr)&&(!App::isFullScreen())&&(cmdIn->cmdId==DISPLAY_MSGBOX_UITHREADCMD) )
        cmdOut->uintParams.push_back(_messageBox(cmdIn->intParams[0],(QWidget*)cmdIn->objectParams[0],cmdIn->stringParams[0],cmdIn->stringParams[1],cmdIn->uintParams[0]));

    if ( (App::mainWindow!=nullptr)&&(!App::isFullScreen())&&(cmdIn->cmdId==DISPLAY_SAVE_DLG_UITHREADCMD) )
        cmdOut->stringParams.push_back(getSaveFileName((QWidget*)cmdIn->objectParams[0],cmdIn->uintParams[0],cmdIn->stringParams[0],cmdIn->stringParams[1],cmdIn->stringParams[2],cmdIn->boolParams[0],cmdIn->stringParams[3],cmdIn->stringParams[4],cmdIn->stringParams[5],cmdIn->stringParams[6],cmdIn->stringParams[7],cmdIn->stringParams[8],cmdIn->stringParams[9],cmdIn->stringParams[10],cmdIn->stringParams[11],cmdIn->stringParams[12],cmdIn->stringParams[13]));

    if ( (App::mainWindow!=nullptr)&&(!App::isFullScreen())&&(cmdIn->cmdId==DLG_INPUT_GET_FLOAT_UITHREADCMD) )
    {
        cmdOut->floatParams.push_back(0.0f);
        cmdOut->boolParams.push_back(dialogInputGetFloat((QWidget*)cmdIn->objectParams[0],cmdIn->stringParams[0].c_str(),cmdIn->stringParams[1].c_str(),cmdIn->floatParams[0],cmdIn->floatParams[1],cmdIn->floatParams[2],cmdIn->intParams[0],&cmdOut->floatParams[0]));
    }

    if ( (App::mainWindow!=nullptr)&&(!App::isFullScreen())&&(cmdIn->cmdId==DISPLAY_OPEN_DLG_UITHREADCMD) )
        cmdOut->stringParams.push_back(getOpenFileName((QWidget*)cmdIn->objectParams[0],cmdIn->uintParams[0],cmdIn->stringParams[0],cmdIn->stringParams[1],cmdIn->stringParams[2],cmdIn->boolParams[0],cmdIn->stringParams[3],cmdIn->stringParams[4],cmdIn->stringParams[5],cmdIn->stringParams[6],cmdIn->stringParams[7],cmdIn->stringParams[8],cmdIn->stringParams[9],cmdIn->stringParams[10],cmdIn->stringParams[11],cmdIn->stringParams[12],cmdIn->stringParams[13]));

    if ( (App::mainWindow!=nullptr)&&(!App::isFullScreen())&&(cmdIn->cmdId==DISPLAY_OPEN_DLG_MULTIFILE_UITHREADCMD) )
        getOpenFileNames(cmdOut->stringParams,(QWidget*)cmdIn->objectParams[0],cmdIn->uintParams[0],cmdIn->stringParams[0],cmdIn->stringParams[1],cmdIn->stringParams[2],cmdIn->boolParams[0],cmdIn->stringParams[3],cmdIn->stringParams[4],cmdIn->stringParams[5],cmdIn->stringParams[6],cmdIn->stringParams[7],cmdIn->stringParams[8],cmdIn->stringParams[9],cmdIn->stringParams[10],cmdIn->stringParams[11],cmdIn->stringParams[12],cmdIn->stringParams[13]);

    if ( (App::mainWindow!=nullptr)&&(!App::isFullScreen())&&(cmdIn->cmdId==SET_FILEDIALOG_NATIVE_UITHREADCMD) )
        setFileDialogsNative(cmdIn->intParams[0]);

    if ( (App::mainWindow!=nullptr)&&(!App::isFullScreen())&&(cmdIn->cmdId==SHOW_PRIMITIVE_SHAPE_DLG_UITHREADCMD) )
    {
        C3Vector sizes;
        int subdiv[3];
        int faces,sides,discSubdiv,openEnds;
        bool smooth,dynamic,pure,cone,negVolume;
        float density,negVolumeScaling;
        if (showPrimitiveShapeDialog(cmdIn->intParams[0],(C3Vector*)cmdIn->objectParams[0],sizes,subdiv,faces,sides,discSubdiv,smooth,openEnds,dynamic,pure,cone,density,negVolume,negVolumeScaling))
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
            cmdOut->boolParams.push_back(negVolume);
            cmdOut->floatParams.push_back(negVolumeScaling);
            cmdOut->boolParams.push_back(dynamic);
        }
    }
#endif

#ifdef SIM_WITH_SERIAL
    if ( (cmdIn->cmdId>SERIAL_PORT_START_SPUITHREADCMD)&&(cmdIn->cmdId<SERIAL_PORT_END_SPUITHREADCMD) )
        App::ct->serialPortContainer->executeCommand(cmdIn,cmdOut);
#endif

}

void CUiThread::showOrHideProgressBar(bool show,float pos,const char* txt)
{ // pos and txt can be omitted (then previously provided values will be used)
    FUNCTION_DEBUG;
#ifdef SIM_WITH_GUI
    if (App::userSettings->doNotShowProgressBars)
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

bool CUiThread::showOrHideEmergencyStop(bool show,const char* txt)
{
    FUNCTION_DEBUG;
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

#ifdef SIM_WITH_GUI
void CUiThread::requestSceneRender(bool bufferMainDisplayStateVariables)
{ // is called by the non-UI thread
    FUNCTION_DEBUG;
    if (App::mainWindow!=nullptr)
    { // make sure we are not in headless mode
        static int lastHere=VDateTime::getTimeInMs();
        if (VDateTime::getTimeDiffInMs(lastHere)>33) // max 30 requ./sec
        { // avoid flooding the UI thread with potentially thousands of render requests/seconds
            _frame_bufferMainDisplayStateVariables=bufferMainDisplayStateVariables;
            // bufferMainDisplayStateVariables is true when the UI thread holds the SIM thread (so the UI thread can modify the scene). The SIM thread still fires rendering signals with this set to true
            _frameId++;
            _requestSceneRender(bufferMainDisplayStateVariables);
            lastHere=VDateTime::getTimeInMs();
        }
    }
}

void CUiThread::requestSceneRender_wait()
{ // is called by the non-UI thread
    FUNCTION_DEBUG;
    if (App::mainWindow!=nullptr)
    { // make sure we are not in headless mode
        _frameId++;
        _frame_bufferMainDisplayStateVariables=false;
        _requestSceneRender_wait(false);
        while (_frameId!=_lastFrameId)
        {
            VThread::sleep(1);
            // App::simThread->executeMessages();
        }
        _lastFrameId=_frameId;
    }
}

void CUiThread::__requestSceneRender(bool bufferMainDisplayStateVariables)
{ // is called by the UI thread. Check also __requestSceneRender_wait
    FUNCTION_DEBUG;
    if ((_frameId!=_lastFrameId)&&(App::mainWindow!=nullptr))
    {
        static int lastHere=VDateTime::getTimeInMs();
        if (VDateTime::getTimeDiffInMs(lastHere)>10)
        { // if many requests are comming in, or if the rendering takes time, leave some time to other tasks!
            App::mainWindow->uiThread_renderScene(_frame_bufferMainDisplayStateVariables);
            lastHere=VDateTime::getTimeInMs();
        }
        _lastFrameId=_frameId;
    }
}

void CUiThread::__requestSceneRender_wait(bool bufferMainDisplayStateVariables)
{ // is called by the UI thread. Check also __requestSceneRender
    FUNCTION_DEBUG;
    if ((_frameId!=_lastFrameId)&&(App::mainWindow!=nullptr))
    {
        App::mainWindow->uiThread_renderScene(_frame_bufferMainDisplayStateVariables);
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
    FUNCTION_DEBUG;
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
                    retVal=VFileDialog::getOpenFileName(App::mainWindow,0,title,startPath,initName,false,extName,e[0].toStdString(),e[1].toStdString(),e[2].toStdString(),e[3].toStdString(),e[4].toStdString(),e[5].toStdString(),e[6].toStdString(),e[7].toStdString(),e[8].toStdString(),e[9].toStdString());
                else
                {
                    std::vector<std::string> files;
                    VFileDialog::getOpenFileNames(files,App::mainWindow,0,title,startPath,initName,false,extName,e[0].toStdString(),e[1].toStdString(),e[2].toStdString(),e[3].toStdString(),e[4].toStdString(),e[5].toStdString(),e[6].toStdString(),e[7].toStdString(),e[8].toStdString(),e[9].toStdString());
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

int CUiThread::messageBox_api(int boxType,int buttons,const char* title,const char* message)
{
    FUNCTION_DEBUG;
    int retVal=sim_msgbox_return_error;
    if ( (App::mainWindow!=nullptr)&&(!App::isFullScreen()) )
    { // make sure we are not in headless mode
        if (VThread::isCurrentThreadTheUiThread())
        { // we are in the UI thread. We execute the command now:
            int buts=VMESSAGEBOX_OKELI;
            if (buttons==sim_msgbox_buttons_ok)
                buts=VMESSAGEBOX_OKELI;
            if (buttons==sim_msgbox_buttons_yesno)
                buts=VMESSAGEBOX_YES_NO;
            if (buttons==sim_msgbox_buttons_yesnocancel)
                buts=VMESSAGEBOX_YES_NO_CANCEL;
            if (buttons==sim_msgbox_buttons_okcancel)
                buts=VMESSAGEBOX_OK_CANCEL;

            unsigned short returned=999;
            if (boxType==sim_msgbox_type_info)
                returned=messageBox_information(App::mainWindow,title,message,buts);
            if (boxType==sim_msgbox_type_question)
                returned=messageBox_question(App::mainWindow,title,message,buts);
            if (boxType==sim_msgbox_type_warning)
                returned=messageBox_warning(App::mainWindow,title,message,buts);
            if (boxType==sim_msgbox_type_critical)
                returned=messageBox_critical(App::mainWindow,title,message,buts);

            if (returned==VMESSAGEBOX_REPLY_CANCEL)
                retVal=sim_msgbox_return_cancel;
            if (returned==VMESSAGEBOX_REPLY_NO)
                retVal=sim_msgbox_return_no;
            if (returned==VMESSAGEBOX_REPLY_YES)
                retVal=sim_msgbox_return_yes;
            if (returned==VMESSAGEBOX_REPLY_OK)
                retVal=sim_msgbox_return_ok;
        }
        else
        { // We are NOT in the UI thread. We execute the command via the UI thread:
            SUIThreadCommand cmdIn;
            SUIThreadCommand cmdOut;
            cmdIn.cmdId=DISPLAY_MSGBOX_API_UITHREADCMD;
            cmdIn.intParams.push_back(boxType);
            cmdIn.intParams.push_back(buttons);
            cmdIn.stringParams.push_back(title);
            cmdIn.stringParams.push_back(message);
            executeCommandViaUiThread(&cmdIn,&cmdOut);
            retVal=cmdOut.intParams[0];
        }
    }
    return(retVal);
}

unsigned short CUiThread::messageBox_informationSystemModal(void* parentWidget,const std::string& title,const std::string& message,unsigned short flags)
{
    FUNCTION_DEBUG;
    return(_messageBox(4,parentWidget,title,message,flags));
}

unsigned short CUiThread::messageBox_information(void* parentWidget,const std::string& title,const std::string& message,unsigned short flags)
{
    FUNCTION_DEBUG;
    return(_messageBox(0,parentWidget,title,message,flags));
}

unsigned short CUiThread::messageBox_question(void* parentWidget,const std::string& title,const std::string& message,unsigned short flags)
{
    FUNCTION_DEBUG;
    return(_messageBox(1,parentWidget,title,message,flags));
}

unsigned short CUiThread::messageBox_warning(void* parentWidget,const std::string& title,const std::string& message,unsigned short flags)
{
    FUNCTION_DEBUG;
    return(_messageBox(2,parentWidget,title,message,flags));
}

unsigned short CUiThread::messageBox_critical(void* parentWidget,const std::string& title,const std::string& message,unsigned short flags)
{
    FUNCTION_DEBUG;
    return(_messageBox(3,parentWidget,title,message,flags));
}

unsigned short CUiThread::_messageBox(int type,void* parentWidget,const std::string& title,const std::string& message,unsigned short flags)
{ // type: 0=info, 1=question, 2=warning, 3=critical, 4=info, system modal
    FUNCTION_DEBUG;
    unsigned short retVal=VMESSAGEBOX_REPLY_ERROR;
    if ( (App::mainWindow!=nullptr)&&(!App::isFullScreen()) )
    { // make sure we are not in headless mode
        if (VThread::isCurrentThreadTheUiThread())
        { // we are in the UI thread. We execute the command now:
            if (type==0)
                retVal=VMessageBox::information((QWidget*)parentWidget,title,message,flags);
            if (type==1)
                retVal=VMessageBox::question((QWidget*)parentWidget,title,message,flags);
            if (type==2)
                retVal=VMessageBox::warning((QWidget*)parentWidget,title,message,flags);
            if (type==3)
                retVal=VMessageBox::critical((QWidget*)parentWidget,title,message,flags);
            if (type==4)
                retVal=VMessageBox::informationSystemModal((QWidget*)parentWidget,title,message,flags);
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
            executeCommandViaUiThread(&cmdIn,&cmdOut);
            if (cmdOut.uintParams.size()>0)
                retVal=cmdOut.uintParams[0];
        }
    }
    return(retVal);
}

bool CUiThread::messageBox_checkbox(void* parentWidget,const std::string& title,const std::string& message,const std::string& checkboxMessage)
{
    FUNCTION_DEBUG;
    bool retVal=false;
    if ( (App::mainWindow!=nullptr)&&(!App::isFullScreen()) )
    { // make sure we are not in headless mode
        if (VThread::isCurrentThreadTheUiThread())
        { // we are in the UI thread. We execute the command now:
            CQDlgMessageAndCheckbox dlg((QWidget*)parentWidget);
            dlg.title=title;
            dlg.text=message;
            dlg.checkbox=checkboxMessage;
            dlg.refresh();
            dlg.makeDialogModal();
            retVal=dlg.checkboxState;
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
            executeCommandViaUiThread(&cmdIn,&cmdOut);
            if (cmdOut.boolParams.size()>0)
                retVal=cmdOut.boolParams[0];
        }
    }
    return(retVal);
}

bool CUiThread::getOpenFileNames(std::vector<std::string>& files,void* parentWidget,unsigned short option,const std::string& title,const std::string& startPath,const std::string& initFilename,bool allowAnyFile,const std::string& extensionName,const std::string& extension1,const std::string& extension2,const std::string& extension3,const std::string& extension4,const std::string& extension5,const std::string& extension6,const std::string& extension7,const std::string& extension8,const std::string& extension9,const std::string& extension10)
{
    FUNCTION_DEBUG;
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
    FUNCTION_DEBUG;
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

std::string CUiThread::getOpenFileName(void* parentWidget,unsigned short option,const std::string& title,const std::string& startPath,const std::string& initFilename,bool allowAnyFile,const std::string& extensionName,const std::string& extension1,const std::string& extension2,const std::string& extension3,const std::string& extension4,const std::string& extension5,const std::string& extension6,const std::string& extension7,const std::string& extension8,const std::string& extension9,const std::string& extension10)
{
    FUNCTION_DEBUG;
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

std::string CUiThread::getSaveFileName(void* parentWidget,unsigned short option,const std::string& title,const std::string& startPath,const std::string& initFilename,bool allowAnyFile,const std::string& extensionName,const std::string& extension1,const std::string& extension2,const std::string& extension3,const std::string& extension4,const std::string& extension5,const std::string& extension6,const std::string& extension7,const std::string& extension8,const std::string& extension9,const std::string& extension10)
{
    FUNCTION_DEBUG;
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
    FUNCTION_DEBUG;
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


bool CUiThread::showPrimitiveShapeDialog(int type,const C3Vector* optionalSizesIn,C3Vector& sizes,int subdiv[3],int& faces,int& sides,int& discSubdiv,bool& smooth,int& openEnds,bool& dynamic,bool& pure,bool& cone,float& density,bool& negVolume,float& negVolumeScaling)
{
    FUNCTION_DEBUG;
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
                negVolume=theDialog.negativeVolume;
                negVolumeScaling=theDialog.negativeVolumeScaling;
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
                negVolume=cmdOut.boolParams[3];
                negVolumeScaling=cmdOut.floatParams[1];
                dynamic=cmdOut.boolParams[4];
                retVal=true;
            }
        }
    }
    return(retVal);
}
#endif
