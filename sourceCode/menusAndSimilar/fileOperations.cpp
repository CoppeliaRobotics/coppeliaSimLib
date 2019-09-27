#include "vrepMainHeader.h"
#include "funcDebug.h"
#include "v_rep_internal.h"
#include "fileOperations.h"
#include "simulation.h"
#include "tt.h"
#include "xmlSer.h"
#include "persistentDataContainer.h"
#include "sceneObjectOperations.h"
#include "algos.h"
#include "app.h"
#include "pluginContainer.h"
#include "meshManip.h"
#include "geometric.h"
#include "v_repStrings.h"
#include <boost/lexical_cast.hpp>
#include "imgLoaderSaver.h"
#include "vVarious.h"
#include "vDateTime.h"
#include "ttUtil.h"
#include "fileOperationsBase.h"
#include <boost/algorithm/string/predicate.hpp>
#ifdef SIM_WITH_GUI
    #include "vFileDialog.h"
    #include "vMessageBox.h"
#endif

bool CFileOperations::processCommand(int commandID)
{ // Return value is true if the command belonged to file menu and was executed
    if ( (commandID>FILE_OPERATION_START_FOCMD)&&(commandID<FILE_OPERATION_END_FOCMD) )
    {
        SSimulationThreadCommand cmd;
        cmd.cmdId=commandID;
        App::appendSimulationThreadCommand(cmd);
        return(true);
    }
    return(false);
}

bool CFileOperations::processCommand(const SSimulationThreadCommand& cmd)
{
    //-----------
    if (cmd.cmdId==FILE_OPERATION_NEW_SCENE_FOCMD)
    {
        SSimulationThreadCommand cmd2;
        cmd2.cmdId=FILE_OPERATION_NEW_SCENE_PHASE2_FOCMD;
#ifdef SIM_WITH_GUI
        if ( (App::mainWindow!=nullptr)&&(!App::userSettings->doNotShowSceneSelectionThumbnails) )
            App::mainWindow->prepareSceneThumbnail(cmd2);
        else
#endif
            App::appendSimulationThreadCommand(cmd2);
        return(true);
    }
    if (cmd.cmdId==FILE_OPERATION_NEW_SCENE_PHASE2_FOCMD)
    { // Cannot undo this command
        if ( App::ct->simulation->isSimulationStopped()&&(App::getEditModeType()==NO_EDIT_MODE) )
        { // execute the command only when simulation is not running and not in an edit mode
            if (!VThread::isCurrentThreadTheUiThread())
            { // we are NOT in the UI thread. We execute the command now:
                createNewScene(true,true);
            }
            else
                App::appendSimulationThreadCommand(cmd); // We are in the UI thread. Execute the command via the main thread
        }
        return(true);
    }
    //-----------

    if (cmd.cmdId==FILE_OPERATION_CLOSE_SCENE_FOCMD)
    { // Cannot undo this command
        if (App::ct->simulation->isSimulationStopped()&&(App::getEditModeType()==NO_EDIT_MODE) )
        { // execute the command only when simulation is not running and not in an edit mode
            if (!VThread::isCurrentThreadTheUiThread())
            { // we are NOT in the UI thread. We execute the command now:
                closeScene(true,true);
            }
            else
                App::appendSimulationThreadCommand(cmd); // We are in the UI thread. Execute the command via the main thread
        }
        return(true);
    }

    //-----------
#ifdef SIM_WITH_GUI
    if (cmd.cmdId==FILE_OPERATION_OPEN_SCENE_FOCMD)
    {
        SSimulationThreadCommand cmd2;
        cmd2.cmdId=FILE_OPERATION_OPEN_SCENE_PHASE2_FOCMD;
        if ( (App::mainWindow!=nullptr)&&(!App::userSettings->doNotShowSceneSelectionThumbnails) )
            App::mainWindow->prepareSceneThumbnail(cmd2);
        else
            App::appendSimulationThreadCommand(cmd2);
        return(true);
    }
    if (cmd.cmdId==FILE_OPERATION_OPEN_SCENE_PHASE2_FOCMD)
    {
        if ( App::ct->simulation->isSimulationStopped()&&(App::getEditModeType()==NO_EDIT_MODE) )
        { // execute the command only when simulation is not running and not in an edit mode
            if (!VThread::isCurrentThreadTheUiThread())
            { // we are NOT in the UI thread. We execute the command now:
                App::addStatusbarMessage(tt::decorateString("",IDSNS_LOADING_SCENE,"...").c_str());
                std::string filenameAndPath=CFileOperationsBase::handleVerSpec_openScenePhase2();
                if (filenameAndPath.length()!=0)
                {
                    App::setRebuildHierarchyFlag();
                    App::setDefaultMouseMode();
                    App::ct->createNewInstance();
                    createNewScene(true,false);
                    if (loadScene(filenameAndPath.c_str(),true,true,true))
                        addToRecentlyOpenedScenes(filenameAndPath);
                    else
                        _removeFromRecentlyOpenedScenes(filenameAndPath);
                }
                else
                    App::addStatusbarMessage(IDSNS_ABORTED);
                App::ct->undoBufferContainer->clearSceneSaveMaybeNeededFlag();
            }
            else
                App::appendSimulationThreadCommand(cmd); // We are in the UI thread. Execute the command via the main thread
        }
        return(true);
    }
    //-----------

    //-----------
    if ((cmd.cmdId>=FILE_OPERATION_OPEN_RECENT_SCENE0_FOCMD)&&(cmd.cmdId<=FILE_OPERATION_OPEN_RECENT_SCENE9_FOCMD))
    {
        SSimulationThreadCommand cmd2;
        cmd2.cmdId=cmd.cmdId-FILE_OPERATION_OPEN_RECENT_SCENE0_FOCMD+FILE_OPERATION_OPEN_RECENT_SCENE0_PHASE2_FOCMD;
        if ( (App::mainWindow!=nullptr)&&(!App::userSettings->doNotShowSceneSelectionThumbnails) )
            App::mainWindow->prepareSceneThumbnail(cmd2);
        else
            App::appendSimulationThreadCommand(cmd2);
        return(true);
    }
    if ((cmd.cmdId>=FILE_OPERATION_OPEN_RECENT_SCENE0_PHASE2_FOCMD)&&(cmd.cmdId<=FILE_OPERATION_OPEN_RECENT_SCENE9_PHASE2_FOCMD))
    {
        if (App::ct->simulation->isSimulationStopped()&&(App::getEditModeType()==NO_EDIT_MODE) )
        { // execute the command only when simulation is not running and not in an edit mode
            if (!VThread::isCurrentThreadTheUiThread())
            { // we are NOT in the UI thread. We execute the command now:
                CFileOperationsBase::handleVerSpec_openRecentScene();
                CPersistentDataContainer cont(FILENAME_OF_USER_SETTINGS_IN_BINARY_FILE);
                std::string filenameAndPath;
                int recentScenesCnt=0;
                for (int i=0;i<10;i++)
                {
                    std::string tmp("SIMSETTINGS_RECENTSCENE0");
                    tmp[23]=48+i;
                    cont.readData(tmp.c_str(),filenameAndPath);
                    if (filenameAndPath.length()>3)
                        recentScenesCnt++;
                    if (recentScenesCnt==cmd.cmdId-FILE_OPERATION_OPEN_RECENT_SCENE0_PHASE2_FOCMD+1)
                        break;
                }

                if (VFile::doesFileExist(filenameAndPath))
                {
                    App::setDefaultMouseMode();
                    App::ct->createNewInstance();
                    CFileOperations::createNewScene(true,false);

                    if (loadScene(filenameAndPath.c_str(),true,true,true))
                        addToRecentlyOpenedScenes(filenameAndPath);
                    else
                        _removeFromRecentlyOpenedScenes(filenameAndPath);
                    App::ct->undoBufferContainer->clearSceneSaveMaybeNeededFlag();
                }
                else
                { // file does not exist anymore
                    App::uiThread->messageBox_information(App::mainWindow,"Open Recent Scene","File does not exist anymore.",VMESSAGEBOX_OKELI);
                    _removeFromRecentlyOpenedScenes(filenameAndPath);
                }
            }
            else
                App::appendSimulationThreadCommand(cmd); // We are in the UI thread. Execute the command via the main thread
        }
        return(true);
    }
    //-----------

    if (cmd.cmdId==FILE_OPERATION_LOAD_MODEL_FOCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            std::string filenameAndPath=CFileOperationsBase::handleVerSpec_loadModel2();
            if (filenameAndPath.length()!=0)
                loadModel(filenameAndPath.c_str(),true,true,true,nullptr,true,nullptr,false,false); // Undo things is in here.
            else
                App::addStatusbarMessage(IDSNS_ABORTED);
        }
        else
            App::appendSimulationThreadCommand(cmd); // We are in the UI thread. Execute the command via the main thread
        return(true);
    }
    if (cmd.cmdId==FILE_OPERATION_SAVE_SCENE_FOCMD)
    {
        if (App::ct->simulation->isSimulationStopped()&&(App::getEditModeType()==NO_EDIT_MODE) )
        { // execute the command only when simulation is not running and not in an edit mode
            if (!VThread::isCurrentThreadTheUiThread())
            { // we are NOT in the UI thread. We execute the command now:
                _saveSceneWithDialogAndEverything(); // will call save as if needed!
            }
            else
                App::appendSimulationThreadCommand(cmd); // We are in the UI thread. Execute the command via the main thread
        }
        return(true);
    }
    if (cmd.cmdId==FILE_OPERATION_EXPORT_IK_CONTENT_FOCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            if (!App::ct->environment->getSceneLocked())
            {
                App::addStatusbarMessage(IDSNS_EXPORTING_IK_CONTENT);
                std::string tst(App::directories->otherFilesDirectory);
                std::string filenameAndPath=App::uiThread->getSaveFileName(App::mainWindow,0,strTranslate(IDS_EXPORTING_IK_CONTENT___),tst,"",false,"V-REP IK Content Files","ik");
                if (filenameAndPath.length()!=0)
                {
                    App::directories->otherFilesDirectory=App::directories->getPathFromFull(filenameAndPath);
                    if (apiExportIkContent(filenameAndPath.c_str(),true))
                        App::addStatusbarMessage(IDSNS_DONE);
                    else
                        App::addStatusbarMessage(IDSNS_OPERATION_FAILED);
                }
                else
                    App::addStatusbarMessage(IDSNS_ABORTED);
            }
            else
                App::uiThread->messageBox_warning(App::mainWindow,strTranslate(IDSN_EXPORT),strTranslate(IDS_SCENE_IS_LOCKED_WARNING),VMESSAGEBOX_OKELI);
        }
        else
            App::appendSimulationThreadCommand(cmd); // We are in the UI thread. Execute the command via the main thread
        return(true);
    }
    if ( (cmd.cmdId==FILE_OPERATION_SAVE_SCENE_AS_VREP_FOCMD)||(cmd.cmdId==FILE_OPERATION_SAVE_SCENE_AS_BR_FOCMD) )
    {
        if (App::ct->simulation->isSimulationStopped()&&(App::getEditModeType()==NO_EDIT_MODE) )
        { // execute the command only when simulation is not running and not in an edit mode
            if (!VThread::isCurrentThreadTheUiThread())
            { // we are NOT in the UI thread. We execute the command now:
                int filetype=CSer::filetype_unspecified_file;
                if (cmd.cmdId==FILE_OPERATION_SAVE_SCENE_AS_VREP_FOCMD)
                    filetype=CSer::filetype_vrep_bin_scene_file;
                if (cmd.cmdId==FILE_OPERATION_SAVE_SCENE_AS_BR_FOCMD)
                    filetype=CSer::filetype_br_bin_scene_file;
                _saveSceneAsWithDialogAndEverything(filetype);
            }
            else
                App::appendSimulationThreadCommand(cmd); // We are in the UI thread. Execute the command via the main thread
        }
        return(true);
    }
    if ( (cmd.cmdId==FILE_OPERATION_SAVE_MODEL_AS_VREP_FOCMD)||(cmd.cmdId==FILE_OPERATION_SAVE_MODEL_AS_BR_FOCMD) )
    {
        if (App::ct->simulation->isSimulationStopped()&&(App::getEditModeType()==NO_EDIT_MODE) )
        { // execute the command only when simulation is not running and not in an edit mode
            if (!VThread::isCurrentThreadTheUiThread())
            { // we are NOT in the UI thread. We execute the command now:
                std::vector<int> sel;
                for (int i=0;i<App::ct->objCont->getSelSize();i++)
                    sel.push_back(App::ct->objCont->getSelID(i));
                if (!App::ct->environment->getSceneLocked())
                {
                    std::string infoM(IDSNS_SAVING_MODEL);
                    infoM+="...";
                    App::addStatusbarMessage(infoM.c_str());
                    if (sel.size()!=0)
                    {
                        int modelBase=App::ct->objCont->getLastSelectionID();

                        // Display a warning if needed
                        CPersistentDataContainer cont(FILENAME_OF_USER_SETTINGS_IN_BINARY_FILE);
                        std::string val;
                        cont.readData("SIMSETTINGS_MODEL_SAVE_OFFSET_WARNING",val);
                        int intVal=0;
                        tt::getValidInt(val,intVal);
                        if (intVal<1)
                        {
                            if (App::uiThread->messageBox_checkbox(App::mainWindow,IDSN_MODEL,IDSN_MODEL_SAVE_POSITION_OFFSET_INFO,IDSN_DO_NOT_SHOW_THIS_MESSAGE_AGAIN))
                            {
                                intVal++;
                                val=tt::FNb(intVal);
                                cont.writeData("SIMSETTINGS_MODEL_SAVE_OFFSET_WARNING",val,!App::userSettings->doNotWritePersistentData);
                            }
                        }

                        bool keepCurrentThumbnail=false;
                        bool operationCancelled=false;
                        while (true)
                        {
                            if (App::ct->environment->modelThumbnail_notSerializedHere.hasImage())
                            { // we already have a thumbnail!
                                SUIThreadCommand cmdIn;
                                SUIThreadCommand cmdOut;
                                cmdIn.cmdId=KEEP_THUMBNAIL_QUESTION_DLG_UITHREADCMD;
                                App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
                                if (cmdOut.boolParams.size()>0)
                                    keepCurrentThumbnail=cmdOut.boolParams[0];
                            }
                            if (!keepCurrentThumbnail)
                            {
                                SUIThreadCommand cmdIn;
                                SUIThreadCommand cmdOut;
                                cmdIn.intParams.push_back(modelBase);
                                cmdIn.cmdId=SELECT_THUMBNAIL_DLG_UITHREADCMD;
                                App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
                                if (cmdOut.boolParams.size()>0)
                                {
                                    if (!cmdOut.boolParams[0])
                                    {
                                        if (cmdOut.boolParams[1])
                                            break;
                                    }
                                    else
                                    {
                                        operationCancelled=true;
                                        break;
                                    }
                                }
                                else
                                {
                                    operationCancelled=true;
                                    break;
                                }
                            }
                            else
                                break;
                        }
                        if (!operationCancelled)
                        {
                            std::string filenameAndPath=CFileOperationsBase::handleVerSpec_saveModel(cmd.cmdId-FILE_OPERATION_SAVE_MODEL_AS_VREP_FOCMD);
                            if (filenameAndPath.length()!=0)
                            {


                                saveModel(modelBase,filenameAndPath.c_str(),true,true,true);
                                App::ct->objCont->deselectObjects();
                            }
                            else
                                App::addStatusbarMessage(IDSNS_ABORTED);
                        }
                        else
                            App::addStatusbarMessage(IDSNS_ABORTED);
                    }
                    else
                        App::addStatusbarMessage(IDSNS_CANNOT_PROCEED_SELECTION_IS_EMPTY);
                }
                else
                    App::uiThread->messageBox_warning(App::mainWindow,strTranslate(IDSN_MODEL),strTranslate(IDS_SCENE_IS_LOCKED_WARNING),VMESSAGEBOX_OKELI);
            }
            else
                App::appendSimulationThreadCommand(cmd); // We are in the UI thread. Execute the command via the main thread
        }
        return(true);
    }
    if (cmd.cmdId==FILE_OPERATION_IMPORT_MESH_FOCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            if (CPluginContainer::isAssimpPluginAvailable())
            {
                App::addStatusbarMessage(IDS_IMPORTING_MESH___);
                App::ct->objCont->deselectObjects();
                std::string tst(App::directories->cadFormatDirectory);

                std::vector<std::string> filenamesAndPaths;
                if (App::uiThread->getOpenFileNames(filenamesAndPaths,App::mainWindow,0,IDS_IMPORTING_MESH___,tst,"",false,"Mesh files","obj","dxf","ply","stl","dae"))
                {
                    std::string files;
                    for (size_t i=0;i<filenamesAndPaths.size();i++)
                    {
                        if (i!=0)
                            files+=";";
                        files+=filenamesAndPaths[i];
                    }
                    CInterfaceStack stack;
                    stack.pushStringOntoStack(files.c_str(),0);
                    App::ct->sandboxScript->callScriptFunctionEx("simAssimp.importShapesDlg",&stack);
                }
                else
                    App::addStatusbarMessage(IDSNS_ABORTED);
            }
            else
                App::uiThread->messageBox_critical(App::mainWindow,strTranslate(IDSN_EXPORT),"Assimp plugin was not found, cannot import",VMESSAGEBOX_OKELI);
        }
        else
            App::appendSimulationThreadCommand(cmd); // We are in the UI thread. Execute the command via the main thread
        return(true);
    }
    if (cmd.cmdId==FILE_OPERATION_IMPORT_PATH_FOCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            App::addStatusbarMessage(IDSNS_IMPORTING_PATH_FROM_CSV_FILE);
            App::ct->objCont->deselectObjects();
            std::string tst(App::directories->cadFormatDirectory);
            std::string filenameAndPath=App::uiThread->getOpenFileName(App::mainWindow,0,strTranslate(IDS_IMPORTING_PATH_FROM_CSV_FILE),tst,"",false,"CSV Files","csv");
            if (filenameAndPath.length()!=0)
            {
                if (VFile::doesFileExist(filenameAndPath))
                {
                    if (_pathImportRoutine(filenameAndPath,true))
                        App::addStatusbarMessage(IDSNS_DONE);
                    else
                        App::addStatusbarMessage(IDSNS_AN_ERROR_OCCURRED_DURING_THE_IMPORT_OPERATION);
                    POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
                }
                else
                    App::addStatusbarMessage(IDSNS_ABORTED_FILE_DOES_NOT_EXIST);
            }
            else
                App::addStatusbarMessage(IDSNS_ABORTED);
        }
        else
            App::appendSimulationThreadCommand(cmd); // We are in the UI thread. Execute the command via the main thread
        return(true);
    }

    if (cmd.cmdId==FILE_OPERATION_IMPORT_HEIGHTFIELD_FOCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            App::addStatusbarMessage(IDSNS_IMPORTING_HEIGHTFIELD_SHAPE);
            App::ct->objCont->deselectObjects();
            std::string tst(App::directories->cadFormatDirectory);
            std::string filenameAndPath=App::uiThread->getOpenFileName(App::mainWindow,0,strTranslate(IDS_IMPORTING_HEIGHTFIELD___),tst,"",true,"Image, CSV and TXT files","tga","jpg","jpeg","png","gif","bmp","tiff","csv","txt");

            if (filenameAndPath.length()!=0)
            {
                if (VFile::doesFileExist(filenameAndPath))
                {
                    if (heightfieldImportRoutine(filenameAndPath))
                        App::addStatusbarMessage(IDSNS_DONE);
                    else
                        App::addStatusbarMessage(IDSNS_AN_ERROR_OCCURRED_DURING_THE_IMPORT_OPERATION);
                    POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
                }
                else
                    App::addStatusbarMessage(IDSNS_ABORTED_FILE_DOES_NOT_EXIST);
            }
            else
                App::addStatusbarMessage(IDSNS_ABORTED);
        }
        else
            App::appendSimulationThreadCommand(cmd); // We are in the UI thread. Execute the command via the main thread
        return(true);
    }

    if (cmd.cmdId==FILE_OPERATION_EXPORT_SHAPE_FOCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            if (CPluginContainer::isAssimpPluginAvailable())
            {
                std::vector<int> sel;
                for (int i=0;i<App::ct->objCont->getSelSize();i++)
                    sel.push_back(App::ct->objCont->getSelID(i));
                if (!App::ct->environment->getSceneLocked())
                {
                    App::addStatusbarMessage(IDSNS_EXPORTING_SHAPES);
                    CSceneObjectOperations::addRootObjectChildrenToSelection(sel);
                    if (0==App::ct->objCont->getShapeNumberInSelection(&sel))
                        return(true); // Selection contains nothing that can be exported!
                    std::string tst(App::directories->cadFormatDirectory);
                    std::string filenameAndPath=App::uiThread->getSaveFileName(App::mainWindow,0,strTranslate(IDSNS_EXPORTING_SHAPES),tst,"",false,"Mesh files","obj","ply","stl","dae");
                    if (filenameAndPath.length()!=0)
                    {
                        App::directories->cadFormatDirectory=App::directories->getPathFromFull(filenameAndPath);

                        CInterfaceStack stack;
                        stack.pushStringOntoStack(filenameAndPath.c_str(),0);
                        stack.pushTableOntoStack();
                        for (size_t i=0;i<sel.size();i++)
                        {
                            stack.pushNumberOntoStack(double(i+1)); // key or index
                            stack.pushNumberOntoStack(sel[i]);
                            stack.insertDataIntoStackTable();
                        }
                        App::ct->sandboxScript->callScriptFunctionEx("simAssimp.exportShapesDlg",&stack);
                    }
                    else
                        App::addStatusbarMessage(IDSNS_ABORTED);
                }
                else
                    App::uiThread->messageBox_warning(App::mainWindow,strTranslate(IDSN_EXPORT),strTranslate(IDS_SCENE_IS_LOCKED_WARNING),VMESSAGEBOX_OKELI);
            }
            else
                App::uiThread->messageBox_critical(App::mainWindow,strTranslate(IDSN_EXPORT),"Assimp plugin was not found, cannot export",VMESSAGEBOX_OKELI);
        }
        else
            App::appendSimulationThreadCommand(cmd); // We are in the UI thread. Execute the command via the main thread
        return(true);
    }
    if (cmd.cmdId==FILE_OPERATION_EXPORT_GRAPHS_FOCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            std::vector<int> sel;
            for (int i=0;i<App::ct->objCont->getSelSize();i++)
                sel.push_back(App::ct->objCont->getSelID(i));
            App::addStatusbarMessage(IDSNS_EXPORTING_GRAPH_DATA);
            App::ct->simulation->stopSimulation();
            if (App::ct->objCont->getGraphNumberInSelection(&sel)!=0)
            {
                std::string tst(App::directories->otherFilesDirectory);
                std::string filenameAndPath=App::uiThread->getSaveFileName(App::mainWindow,0,strTranslate(IDS_SAVING_GRAPHS___),tst,"",false,"CSV Files","csv");
                if (filenameAndPath.length()!=0)
                {
                    VFile myFile(filenameAndPath.c_str(),VFile::CREATE_WRITE|VFile::SHARE_EXCLUSIVE);
                    VArchive ar(&myFile,VArchive::STORE);
                    App::directories->otherFilesDirectory=App::directories->getPathFromFull(filenameAndPath);
                    for (int i=0;i<int(sel.size());i++)
                    {
                        CGraph* it=App::ct->objCont->getGraph(sel[i]);
                        if (it!=nullptr)
                            it->exportGraphData(ar);
                    }
                    ar.close();
                    myFile.close();
                    App::addStatusbarMessage(IDSNS_DONE);
                }
                else
                    App::addStatusbarMessage(IDSNS_ABORTED);
            }
        }
        else
            App::appendSimulationThreadCommand(cmd); // We are in the UI thread. Execute the command via the main thread
        return(true);
    }

    if ( (cmd.cmdId==FILE_OPERATION_EXPORT_PATH_SIMPLE_POINTS_FOCMD)||(cmd.cmdId==FILE_OPERATION_EXPORT_PATH_BEZIER_POINTS_FOCMD) )
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            std::vector<int> sel;
            for (int i=0;i<App::ct->objCont->getSelSize();i++)
                sel.push_back(App::ct->objCont->getSelID(i));
            if (App::ct->objCont->isLastSelectionAPath(&sel))
            {
                CPath* it=(CPath*)App::ct->objCont->getLastSelection(&sel);
                if (it->pathContainer->getSimplePathPointCount()!=0)
                {
                    if (cmd.cmdId==FILE_OPERATION_EXPORT_PATH_SIMPLE_POINTS_FOCMD)
                        App::addStatusbarMessage(IDSNS_EXPORTING_PATH);
                    else
                        App::addStatusbarMessage(IDSNS_EXPORTING_PATHS_BEZIER_CURVE);
                    App::ct->simulation->stopSimulation();
                    std::string titleString;
                    if (cmd.cmdId==FILE_OPERATION_EXPORT_PATH_SIMPLE_POINTS_FOCMD)
                        titleString=strTranslate(IDS_EXPORTING_PATH___);
                    else
                        titleString=strTranslate(IDS_EXPORTING_PATH_BEZIER_CURVE___);
                    std::string filenameAndPath=App::uiThread->getSaveFileName(App::mainWindow,0,titleString,App::directories->executableDirectory,"",false,"CSV Files","csv");
                    if (filenameAndPath.length()!=0)
                    {
                        _pathExportPoints(filenameAndPath,it->getObjectHandle(),cmd.cmdId==FILE_OPERATION_EXPORT_PATH_BEZIER_POINTS_FOCMD,true);
                        App::addStatusbarMessage(IDSNS_DONE);
                    }
                    else
                        App::addStatusbarMessage(IDSNS_ABORTED);
                }
                else
                    App::addStatusbarMessage(IDSNS_CANNOT_EXPORT_AN_EMPTY_PATH);
            }
            else
                App::addStatusbarMessage(IDSNS_LAST_SELECTION_IS_NOT_A_PATH);
            App::ct->objCont->deselectObjects();
        }
        else
            App::appendSimulationThreadCommand(cmd); // We are in the UI thread. Execute the command via the main thread
        return(true);
    }
    if (cmd.cmdId==FILE_OPERATION_EXPORT_DYNAMIC_CONTENT_FOCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            if (!App::ct->environment->getSceneLocked())
            {
                App::addStatusbarMessage(IDSNS_EXPORTING_DYNAMIC_CONTENT);
                if (CPluginContainer::dyn_isDynamicContentAvailable()!=0)
                {
                    int eng=App::ct->dynamicsContainer->getDynamicEngineType(nullptr);
                    if (eng==sim_physics_ode)
                    {
                        std::string tst(App::directories->otherFilesDirectory);
                        std::string filenameAndPath=App::uiThread->getSaveFileName(App::mainWindow,0,strTranslate(IDS_EXPORTING_DYNAMIC_CONTENT___),tst,"",false,"ODE Dynamics World Files","ode");
                        if (filenameAndPath.length()!=0)
                        {
                            App::directories->otherFilesDirectory=App::directories->getPathFromFull(filenameAndPath);
                            CPluginContainer::dyn_serializeDynamicContent(filenameAndPath.c_str(),0);
                            App::addStatusbarMessage(IDSNS_DONE);
                        }
                        else
                            App::addStatusbarMessage(IDSNS_ABORTED);
                    }
                    if (eng==sim_physics_bullet)
                    {
                        std::string tst(App::directories->otherFilesDirectory);
                        std::string filenameAndPath=App::uiThread->getSaveFileName(App::mainWindow,0,strTranslate(IDS_EXPORTING_DYNAMIC_CONTENT___),tst,"",false,"Bullet Dynamics World Files","bullet");
                        if (filenameAndPath.length()!=0)
                        {
                            App::directories->otherFilesDirectory=App::directories->getPathFromFull(filenameAndPath);
                            CPluginContainer::dyn_serializeDynamicContent(filenameAndPath.c_str(),App::userSettings->bulletSerializationBuffer);
                            App::addStatusbarMessage(IDSNS_DONE);
                        }
                        else
                            App::addStatusbarMessage(IDSNS_ABORTED);
                    }
                    if (eng==sim_physics_vortex)
                    {
                        std::string tst(App::directories->otherFilesDirectory);
                        std::string filenameAndPath=App::uiThread->getSaveFileName(App::mainWindow,0,strTranslate(IDS_EXPORTING_DYNAMIC_CONTENT___),tst,"",false,"Vortex Dynamics World Files","vortex");
                        if (filenameAndPath.length()!=0)
                        {
                            App::directories->otherFilesDirectory=App::directories->getPathFromFull(filenameAndPath);
                            CPluginContainer::dyn_serializeDynamicContent(filenameAndPath.c_str(),App::userSettings->bulletSerializationBuffer);
                            App::addStatusbarMessage(IDSNS_DONE);
                        }
                        else
                            App::addStatusbarMessage(IDSNS_ABORTED);
                    }
                    if (eng==sim_physics_newton)
                    {
                        std::string tst(App::directories->otherFilesDirectory);
                        std::string filenameAndPath=App::uiThread->getSaveFileName(App::mainWindow,0,strTranslate(IDS_EXPORTING_DYNAMIC_CONTENT___),tst,"",false,"Newton Dynamics World Files","newton");
                        if (filenameAndPath.length()!=0)
                        {
                            App::directories->otherFilesDirectory=App::directories->getPathFromFull(filenameAndPath);
                            CPluginContainer::dyn_serializeDynamicContent(filenameAndPath.c_str(),App::userSettings->bulletSerializationBuffer);
                            App::addStatusbarMessage(IDSNS_DONE);
                        }
                        else
                            App::addStatusbarMessage(IDSNS_ABORTED);
                    }
                }
                else
                    App::addStatusbarMessage(IDSNS_CANNOT_PROCEED_NO_DYNAMIC_CONTENT_AVAILABLE);
            }
            else
                App::uiThread->messageBox_warning(App::mainWindow,strTranslate(IDSN_EXPORT),strTranslate(IDS_SCENE_IS_LOCKED_WARNING),VMESSAGEBOX_OKELI);
        }
        else
            App::appendSimulationThreadCommand(cmd); // We are in the UI thread. Execute the command via the main thread
        return(true);
    }
#endif

    if (cmd.cmdId==FILE_OPERATION_EXIT_SIMULATOR_FOCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            bool displayed=false;
#ifdef SIM_WITH_GUI
            int ci=-1;
            int si=-1;
            int ei=-1;
            if (App::mainWindow!=nullptr)
                App::mainWindow->simulationRecorder->stopRecording(false);

            ci=App::ct->getInstanceIndexOfASceneNotYetSaved(App::ct->environment->getSceneLocked());
            if (!App::ct->simulation->isSimulationStopped())
                si=App::ct->getCurrentInstanceIndex();
            if (App::getEditModeType()!=NO_EDIT_MODE)
                ei=App::ct->getCurrentInstanceIndex();

            if (!App::ct->environment->getSceneLocked())
            {
                if (App::ct->undoBufferContainer->isSceneSaveMaybeNeededFlagSet())
                    ci=App::ct->getCurrentInstanceIndex();
            }
            if ((ei==App::ct->getCurrentInstanceIndex())&&(!displayed))
            {
                if (VMESSAGEBOX_REPLY_OK==App::uiThread->messageBox_warning(App::mainWindow,strTranslate(IDSN_EXIT),strTranslate(IDS_INSTANCE_STILL_IN_EDIT_MODE_MESSAGE),VMESSAGEBOX_OK_CANCEL))
                {
                    if (App::mainWindow!=nullptr)
                        App::mainWindow->editModeContainer->processCommand(ANY_EDIT_MODE_FINISH_AND_CANCEL_CHANGES_EMCMD,nullptr);
                    ei=-1;
                }
                else
                    displayed=true;
            }
            if ((si==App::ct->getCurrentInstanceIndex())&&(!displayed))
            {
                if (VMESSAGEBOX_REPLY_OK==App::uiThread->messageBox_warning(App::mainWindow,strTranslate(IDSN_EXIT),strTranslate(IDS_SIMULATION_STILL_RUNNING_MESSAGE),VMESSAGEBOX_OK_CANCEL))
                    App::ct->simulatorMessageQueue->addCommand(sim_message_simulation_stop_request,0,0,0,0,nullptr,0);
                displayed=true;
            }
            if ((ci==App::ct->getCurrentInstanceIndex())&&(!displayed))
            {
                unsigned short action=VMESSAGEBOX_REPLY_NO;
                if (CFileOperationsBase::handleVerSpec_canSaveScene())
                    action=App::uiThread->messageBox_warning(App::mainWindow,strTranslate(IDSN_SAVE),strTranslate(IDS_WANNA_SAVE_THE_SCENE_WARNING),VMESSAGEBOX_YES_NO_CANCEL);
                if (action==VMESSAGEBOX_REPLY_YES)
                {
                    if (_saveSceneWithDialogAndEverything()) // will call save as if needed!
                        action=VMESSAGEBOX_REPLY_NO;
                }
                if (action==VMESSAGEBOX_REPLY_NO)
                {
                    App::ct->undoBufferContainer->clearSceneSaveMaybeNeededFlag();
                    ci=App::ct->getInstanceIndexOfASceneNotYetSaved(false);
                }
                else
                    displayed=true;
            }
            if ((ei!=-1)&&(!displayed))
            {
                App::uiThread->messageBox_warning(App::mainWindow,strTranslate(IDSN_EXIT),strTranslate(IDS_ANOTHER_INSTANCE_STILL_IN_EDIT_MODE_MESSAGE),VMESSAGEBOX_OKELI);
                App::ct->setInstanceIndex(ei);
                displayed=true;
            }
            if ((si!=-1)&&(!displayed))
            {
                App::uiThread->messageBox_warning(App::mainWindow,strTranslate(IDSN_EXIT),strTranslate(IDS_ANOTHER_SIMULATION_STILL_RUNNING_MESSAGE),VMESSAGEBOX_OKELI);
                App::ct->setInstanceIndex(si);
                displayed=true;
            }
            if ((ci!=-1)&&(!displayed))
            {
                if (VMESSAGEBOX_REPLY_CANCEL==App::uiThread->messageBox_warning(App::mainWindow,strTranslate(IDSN_SAVE),strTranslate(IDS_ANOTHER_INSTANCE_STILL_NOT_SAVED_WANNA_LEAVE_ANYWAY_MESSAGE),VMESSAGEBOX_OK_CANCEL))
                {
                    App::ct->setInstanceIndex(ci);
                    displayed=true;
                }
            }
#endif
            if (!displayed)
            {
                SSimulationThreadCommand cmd;
                cmd.cmdId=FINAL_EXIT_REQUEST_CMD;
                App::appendSimulationThreadCommand(cmd);
            }
        }
        else
            App::appendSimulationThreadCommand(cmd); // We are in the UI thread. Execute the command via the main thread

        return(true);
    }

    if (cmd.cmdId==FILE_OPERATION_OPEN_DRAG_AND_DROP_MODEL_FOCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            CFileOperations::loadModel(cmd.stringParams[0].c_str(),true,cmd.boolParams[0],false,nullptr,cmd.boolParams[1],nullptr,false,false);
        }
        else
            App::appendSimulationThreadCommand(cmd); // We are in the UI thread. Execute the command via the main thread:
        return(true);
    }
    return(false);
}


void CFileOperations::createNewScene(bool displayMessages,bool forceForNewInstance)
{
    FUNCTION_DEBUG;
    bool useNewInstance=false;
    useNewInstance=(App::ct->undoBufferContainer->isSceneSaveMaybeNeededFlagSet()||(App::ct->mainSettings->getScenePathAndName()!=""))&&(!App::ct->environment->getSceneCanBeDiscardedWhenNewSceneOpened());
    if (forceForNewInstance)
        useNewInstance=true;
    CFileOperationsBase::handleVerSpec_createNewScene();
    App::setDefaultMouseMode();
    if (useNewInstance)
        App::ct->createNewInstance();
    else
        App::ct->simulation->stopSimulation();
    App::ct->emptyScene(true);
    std::string fullPathAndFilename=App::directories->systemDirectory+VREP_SLASH;
    fullPathAndFilename+="dfltscn.";
    fullPathAndFilename+=VREP_SCENE_EXTENSION;
    loadScene(fullPathAndFilename.c_str(),false,false,false);
    App::ct->mainSettings->setScenePathAndName("");//savedLoc;
    App::ct->environment->generateNewUniquePersistentIdString();
    if (displayMessages)
        App::addStatusbarMessage(IDSNS_DEFAULT_SCENE_WAS_SET_UP);
    App::ct->undoBufferContainer->memorizeState(); // so that we can come back to the initial state!
    App::ct->undoBufferContainer->clearSceneSaveMaybeNeededFlag();
}

void CFileOperations::closeScene(bool displayMessages,bool displayDialogs)
{
    FUNCTION_DEBUG;
    unsigned short action=VMESSAGEBOX_REPLY_NO;
#ifdef SIM_WITH_GUI
    if (displayMessages&&(!App::ct->environment->getSceneCanBeDiscardedWhenNewSceneOpened()))
    {
        if (CFileOperationsBase::handleVerSpec_canSaveScene()&&(!App::ct->environment->getSceneLocked()))
        {
            if (displayDialogs&&App::ct->undoBufferContainer->isSceneSaveMaybeNeededFlagSet())
            {
                action=App::uiThread->messageBox_warning(App::mainWindow,strTranslate(IDSN_SAVE),strTranslate(IDS_WANNA_SAVE_THE_SCENE_WARNING),VMESSAGEBOX_YES_NO_CANCEL);
                if (action==VMESSAGEBOX_REPLY_YES)
                {
                    if (_saveSceneWithDialogAndEverything()) // will call save as if needed!
                        action=VMESSAGEBOX_REPLY_NO;
                }
            }
        }
    }
#endif
    if (action==VMESSAGEBOX_REPLY_NO)
    {
        App::ct->simulation->stopSimulation();
#ifdef SIM_WITH_GUI
        App::setDefaultMouseMode();
        if (App::mainWindow!=nullptr)
            App::mainWindow->editModeContainer->processCommand(ANY_EDIT_MODE_FINISH_AND_CANCEL_CHANGES_EMCMD,nullptr);
#endif
        if (displayDialogs)
            App::uiThread->showOrHideProgressBar(true,-1,"Closing scene...");
        App::ct->emptyScene(true);
        if (displayDialogs)
            App::uiThread->showOrHideProgressBar(false);
        if (App::ct->getInstanceCount()>1)
        { // remove this instance:
            App::ct->destroyCurrentInstance();
        }
        else
        { // simply set-up an empty (default) scene
            std::string savedLoc=App::ct->mainSettings->getScenePathAndName();
            std::string fullPathAndFilename=App::directories->systemDirectory+VREP_SLASH;
            fullPathAndFilename+="dfltscn.";
            fullPathAndFilename+=VREP_SCENE_EXTENSION;
            loadScene(fullPathAndFilename.c_str(),false,false,false);
            App::ct->mainSettings->setScenePathAndName(""); //savedLoc.c_str());
            App::ct->environment->generateNewUniquePersistentIdString();
            App::addStatusbarMessage(IDSNS_DEFAULT_SCENE_WAS_SET_UP);
            App::ct->undoBufferContainer->memorizeState(); // so that we can come back to the initial state!
            App::ct->undoBufferContainer->clearSceneSaveMaybeNeededFlag();
        }
    }
    App::setRebuildHierarchyFlag();
}

bool CFileOperations::_pathImportRoutine(const std::string& pathName,bool displayDialogs)
{ // Should only be called by the NON-UI thread
    bool retVal=false;
    if (VFile::doesFileExist(pathName))
    {
        if (displayDialogs)
            App::uiThread->showOrHideProgressBar(true,-1,"Importing path...");
        try
        {
            VFile file(pathName.c_str(),VFile::READ|VFile::SHARE_DENY_NONE);
            VArchive archive(&file,VArchive::LOAD);
            unsigned int currentPos=0;
            std::string line;
            std::vector<float> readData;
            bool automaticOrientation=true;
            const int ds=16;
            // We read:
            // (x,y,z), (a,b,g), velocityRatio, bezierPtCount,beforeRatio,afterRatio,virtualDistance,auxFlags,auxChannel1,auxChannel2,auxChannel3,auxChannel4
            while (archive.readSingleLine(currentPos,line,false))
            {
                float data[ds];
                int cnt=0;
                std::string word;
                float val;
                while (tt::extractCommaSeparatedWord(line,word))
                {
                    if (tt::getValidFloat(word,val))
                        data[cnt]=val;
                    else
                        break;
                    cnt++;
                    if (cnt>=ds)
                        break;
                }
                if (cnt>=3)
                { // We have a point!
                    int dataOff=(int)readData.size();
                    readData.push_back(0.0f); // x
                    readData.push_back(0.0f); // y
                    readData.push_back(0.0f); // z
                    readData.push_back(0.0f); // Euler 0
                    readData.push_back(0.0f); // Euler 1
                    readData.push_back(0.0f); // Euler 2
                    readData.push_back(1.0f); // Velocity ratio
                    readData.push_back(1.0f); // Bezier count
                    readData.push_back(0.5f); // Bezier 0
                    readData.push_back(0.5f); // Bezier 1
                    readData.push_back(0.0f); // virtual distance
                    readData.push_back(0.0f); // aux flags
                    readData.push_back(0.0f); // aux channel1
                    readData.push_back(0.0f); // aux channel2
                    readData.push_back(0.0f); // aux channel3
                    readData.push_back(0.0f); // aux channel4

                    readData[dataOff+0]=data[0]; // x
                    readData[dataOff+1]=data[1]; // y
                    readData[dataOff+2]=data[2]; // z
                    if (cnt>=6)
                    {
                        automaticOrientation=false;
                        readData[dataOff+3]=data[3]*degToRad_f; // Euler 0
                        readData[dataOff+4]=data[4]*degToRad_f; // Euler 1
                        readData[dataOff+5]=data[5]*degToRad_f; // Euler 2
                    }
                    if (cnt>=7)
                        readData[dataOff+6]=data[6]; // velocity ratio
                    if (cnt>=8)
                        readData[dataOff+7]=data[7]; // Bezier count
                    if (cnt>=10)
                    {
                        readData[dataOff+8]=data[8]; // Bezier 0
                        readData[dataOff+9]=data[9]; // Bezier 1
                    }
                    if (cnt>=11)
                        readData[dataOff+10]=data[10]; // Virtual distance
                    if (cnt>=12)
                        readData[dataOff+11]=data[11]; // aux flags
                    if (cnt>=13)
                        readData[dataOff+12]=data[12]; // aux channel1
                    if (cnt>=14)
                        readData[dataOff+13]=data[13]; // aux channel2
                    if (cnt>=15)
                        readData[dataOff+14]=data[14]; // aux channel3
                    if (cnt>=16)
                        readData[dataOff+15]=data[15]; // aux channel4
                }
            }
            if (readData.size()!=0)
            {
                CPath* newObject=new CPath();
                newObject->pathContainer->enableActualization(false);
                int attr=newObject->pathContainer->getAttributes()|sim_pathproperty_automatic_orientation;
                if (!automaticOrientation)
                    attr-=sim_pathproperty_automatic_orientation;
                newObject->pathContainer->setAttributes(attr);
                for (int i=0;i<int(readData.size())/ds;i++)
                {
                    CSimplePathPoint* it=new CSimplePathPoint();
                    C7Vector tr;
                    tr.X(0)=readData[ds*i+0];
                    tr.X(1)=readData[ds*i+1];
                    tr.X(2)=readData[ds*i+2];
                    tr.Q.setEulerAngles(readData[ds*i+3],readData[ds*i+4],readData[ds*i+5]);
                    it->setTransformation(tr,newObject->pathContainer->getAttributes());
                    it->setMaxRelAbsVelocity(readData[ds*i+6]);
                    it->setBezierPointCount(int(readData[ds*i+7]+0.5f));
                    it->setBezierFactors(readData[ds*i+8],readData[ds*i+9]);
                    it->setOnSpotDistance(readData[ds*i+10]);
                    it->setAuxFlags(int(readData[ds*i+11]+0.5f));
                    it->setAuxChannels(&readData[ds*i+12]);
                    newObject->pathContainer->addSimplePathPoint(it);
                }
                newObject->pathContainer->enableActualization(true);
                newObject->pathContainer->actualizePath();
                newObject->setObjectName_objectNotYetInScene(IDSOGL_IMPORTEDPATH);
                newObject->setObjectAltName_objectNotYetInScene(tt::getObjectAltNameFromObjectName(newObject->getObjectName()));

                App::ct->objCont->addObjectToScene(newObject,false,true);
                App::ct->objCont->selectObject(newObject->getObjectHandle());
            }
            archive.close();
            file.close();
            retVal=readData.size()!=0;
        }
        catch(VFILE_EXCEPTION_TYPE e)
        {
            VFile::reportAndHandleFileExceptionError(e);
            retVal=false;
        }
        if (displayDialogs)
            App::uiThread->showOrHideProgressBar(false);
    }
    return(retVal);
}

bool CFileOperations::_pathExportPoints(const std::string& pathName,int pathID,bool bezierPoints,bool displayDialogs)
{
    CPath* pathObject=App::ct->objCont->getPath(pathID);
    if (pathObject==nullptr)
        return(false);
    bool retVal=false;
    if (displayDialogs)
        App::uiThread->showOrHideProgressBar(true,-1,"Exporting path points...");
    try
    {
        VFile myFile(pathName.c_str(),VFile::CREATE_WRITE|VFile::SHARE_EXCLUSIVE);
        VArchive ar(&myFile,VArchive::STORE);

        CPathCont* it=pathObject->pathContainer;
        C7Vector pathTr(pathObject->getCumulativeTransformation());
        if (bezierPoints)
        {
            for (int i=0;i<it->getBezierPathPointCount();i++)
            {
                CBezierPathPoint* bp=it->getBezierPathPoint(i);
                C7Vector tr(pathTr*bp->getTransformation());
                C3Vector euler(tr.Q.getEulerAngles());
                std::string line(tt::FNb(tr.X(0))+',');
                line+=tt::FNb(tr.X(1))+',';
                line+=tt::FNb(tr.X(2))+',';
                line+=tt::FNb(euler(0)*radToDeg_f)+',';
                line+=tt::FNb(euler(1)*radToDeg_f)+',';
                line+=tt::FNb(euler(2)*radToDeg_f)+',';
                line+=tt::FNb(bp->getMaxRelAbsVelocity())+',';
                line+=tt::FNb(bp->getOnSpotDistance())+',';
                line+=tt::FNb(bp->getAuxFlags())+',';
                float auxChannels[4];
                bp->getAuxChannels(auxChannels);
                line+=tt::FNb(auxChannels[0])+',';
                line+=tt::FNb(auxChannels[1])+',';
                line+=tt::FNb(auxChannels[2])+',';
                line+=tt::FNb(auxChannels[3]);
                ar.writeLine(line);
            }
        }
        else
        {
            for (int i=0;i<it->getSimplePathPointCount();i++)
            {
                CSimplePathPoint* bp=it->getSimplePathPoint(i);
                C7Vector tr(pathTr*bp->getTransformation());
                C3Vector euler(tr.Q.getEulerAngles());
                float f0,f1;
                bp->getBezierFactors(f0,f1);
                std::string line(tt::FNb(tr.X(0))+',');
                line+=tt::FNb(tr.X(1))+',';
                line+=tt::FNb(tr.X(2))+',';
                line+=tt::FNb(euler(0)*radToDeg_f)+',';
                line+=tt::FNb(euler(1)*radToDeg_f)+',';
                line+=tt::FNb(euler(2)*radToDeg_f)+',';
                line+=tt::FNb(bp->getMaxRelAbsVelocity())+',';
                line+=tt::FNb(bp->getBezierPointCount())+',';
                line+=tt::FNb(f0)+',';
                line+=tt::FNb(f1)+',';
                line+=tt::FNb(bp->getOnSpotDistance())+',';
                line+=tt::FNb(bp->getAuxFlags())+',';
                float auxChannels[4];
                bp->getAuxChannels(auxChannels);
                line+=tt::FNb(auxChannels[0])+',';
                line+=tt::FNb(auxChannels[1])+',';
                line+=tt::FNb(auxChannels[2])+',';
                line+=tt::FNb(auxChannels[3]);
                ar.writeLine(line);
            }
        }
        ar.close();
        myFile.close();
        retVal=true;
    }
    catch(VFILE_EXCEPTION_TYPE e)
    {
        VFile::reportAndHandleFileExceptionError(e);
        retVal=false;
    }
    if (displayDialogs)
        App::uiThread->showOrHideProgressBar(false);
    return(retVal);
}

bool CFileOperations::loadScene(const char* pathAndFilename,bool displayMessages,bool displayDialogs,bool setCurrentDir)
{
    FUNCTION_DEBUG;
    if (App::isFullScreen()||App::userSettings->doNotShowAcknowledgmentMessages)
        displayDialogs=false;

    if (strlen(pathAndFilename)==0)
    {
        createNewScene(displayMessages,true);
        return(true);
    }

    App::setDefaultMouseMode();

    int result=-3;
    CFileOperationsBase::handleVerSpec_loadScene1();
    App::ct->objCont->deselectObjects();
    App::ct->simulation->stopSimulation(); // should be anyway stopped!
    if (VFile::doesFileExist(pathAndFilename))
    {
        App::ct->emptyScene(true);

        App::ct->mainSettings->setScenePathAndName(pathAndFilename);
        if (setCurrentDir)
            App::directories->sceneDirectory=App::ct->mainSettings->getScenePath();
        CFileOperationsBase::handleVerSpec_loadScene2();

        if (displayDialogs)
            App::uiThread->showOrHideProgressBar(true,-1,"Opening scene...");

        CSer* serObj;
        int serializationVersion;
        unsigned short vrepVersionThatWroteThis;
        int licenseTypeThatWroteThis;
        char revisionNumber;
        if (CSer::getFileTypeFromName(pathAndFilename)!=CSer::filetype_unspecified_file)
        {
            serObj=new CSer(pathAndFilename,CSer::getFileTypeFromName(pathAndFilename));
            result=serObj->readOpenBinary(serializationVersion,vrepVersionThatWroteThis,licenseTypeThatWroteThis,revisionNumber,false);
        }

        std::string infoPrintOut(tt::decorateString("",IDSNS_LOADING_SCENE," ("));
        infoPrintOut+=std::string(pathAndFilename)+"). ";
#ifdef SIM_WITH_GUI
        if ((result==-3)&&(App::mainWindow!=nullptr))
        {
            if (displayMessages)
                App::addStatusbarMessage("The file does not seem to be a valid scene file.");
            if (displayDialogs)
            {
                App::uiThread->showOrHideProgressBar(false);
                App::uiThread->messageBox_critical(App::mainWindow,IDSN_SERIALIZATION,"The file does not seem to be a valid scene file.",VMESSAGEBOX_OKELI);
                App::uiThread->showOrHideProgressBar(true);
            }
        }
        if ((result!=-3)&&displayMessages&&(App::mainWindow!=nullptr))
        {
            infoPrintOut+=" ";
            infoPrintOut+=IDSNS_SERIALIZATION_VERSION_IS;
            infoPrintOut+=" ";
            infoPrintOut+=boost::lexical_cast<std::string>(serializationVersion)+".";
            App::addStatusbarMessage(infoPrintOut.c_str());
            infoPrintOut=_getStringOfVersionAndLicenseThatTheFileWasWrittenWith(vrepVersionThatWroteThis,licenseTypeThatWroteThis,revisionNumber);
            if (infoPrintOut!="")
                App::addStatusbarMessage(infoPrintOut.c_str());
        }
        if ((result==-2)&&(App::mainWindow!=nullptr))
        {
            if (displayMessages)
                App::addStatusbarMessage(IDS_SERIALIZATION_VERSION_NOT_SUPPORTED_ANYMORE);
            if (displayDialogs)
            {
                App::uiThread->showOrHideProgressBar(false);
                App::uiThread->messageBox_critical(App::mainWindow,strTranslate(IDSN_SERIALIZATION),strTranslate(IDS_SERIALIZATION_VERSION_NOT_SUPPORTED_ANYMORE),VMESSAGEBOX_OKELI);
                App::uiThread->showOrHideProgressBar(true);
            }
        }
        if ((result==-1)&&(App::mainWindow!=nullptr))
        {
            if (displayMessages)
                App::addStatusbarMessage(IDS_SERIALIZATION_VERSION_TOO_RECENT);
            if (displayDialogs)
            {
                App::uiThread->showOrHideProgressBar(false);
                App::uiThread->messageBox_critical(App::mainWindow,strTranslate(IDSN_SERIALIZATION),strTranslate(IDS_SERIALIZATION_VERSION_TOO_RECENT),VMESSAGEBOX_OKELI);
                App::uiThread->showOrHideProgressBar(true);
            }
        }
        if ((result==0)&&(App::mainWindow!=nullptr))
        {
            if (displayMessages)
                App::addStatusbarMessage(IDS_COMPRESSION_SCHEME_NOT_SUPPORTED);
            if (displayDialogs)
            {
                App::uiThread->showOrHideProgressBar(false);
                App::uiThread->messageBox_critical(App::mainWindow,strTranslate(IDSN_SERIALIZATION),strTranslate(IDS_COMPRESSION_SCHEME_NOT_SUPPORTED),VMESSAGEBOX_OKELI);
                App::uiThread->showOrHideProgressBar(true);
            }
        }
#endif
        if (result==1)
        {
            App::ct->objCont->loadScene(serObj[0],false);
            serObj->readClose();
#ifdef SIM_WITH_GUI
            if (App::mainWindow!=nullptr)
                App::mainWindow->refreshDimensions(); // this is important so that the new pages and views are set to the correct dimensions
            if (displayMessages)
                App::addStatusbarMessage(IDSNS_SCENE_OPENED);
            if ((vrepVersionThatWroteThis>VREP_PROGRAM_VERSION_NB)&&displayDialogs&&(App::mainWindow!=nullptr))
                App::uiThread->messageBox_warning(App::mainWindow,strTranslate(IDSN_SCENE),strTranslate(IDS_SAVED_WITH_MORE_RECENT_VERSION_WARNING),VMESSAGEBOX_OKELI);
            std::string acknowledgement(App::ct->environment->getAcknowledgement());
            std::string tmp(acknowledgement);
            tt::removeSpacesAtBeginningAndEnd(tmp);
            if (displayDialogs&&(App::mainWindow!=nullptr))
            {
                if (tmp.length()!=0)
                {
                    SSimulationThreadCommand cmd;
                    cmd.cmdId=POSTPONE_PROCESSING_THIS_LOOP_CMD;
                    cmd.intParams.push_back(3);
                    App::appendSimulationThreadCommand(cmd);
                    cmd.cmdId=DISPLAY_ACKNOWLEDGMENT_MESSAGE_CMD;
                    cmd.stringParams.push_back(IDS_SCENE_CONTENT_ACKNOWLEDGMENTS);
                    cmd.stringParams.push_back(acknowledgement);
                    App::appendSimulationThreadCommand(cmd);
                }
            }
#endif
        }
        else
        {
            if (displayMessages)
                App::addStatusbarMessage(IDSNS_SCENE_COULD_NOT_BE_OPENED);
        }
        delete serObj;
        if (displayDialogs)
            App::uiThread->showOrHideProgressBar(false);
        App::ct->undoBufferContainer->memorizeState(); // so that we can come back to the initial state!
    }
    else
    {
        if (displayMessages)
            App::addStatusbarMessage(IDSNS_ABORTED_FILE_DOES_NOT_EXIST);
    }
    App::setRebuildHierarchyFlag();
    return(result==1);
}

bool CFileOperations::loadModel(const char* pathAndFilename,bool displayMessages,bool displayDialogs,bool setCurrentDir,std::string* acknowledgmentPointerInReturn,bool doUndoThingInHere,std::vector<char>* loadBuffer,bool onlyThumbnail,bool forceModelAsCopy)
{ // if acknowledgment is nullptr, then acknowledgments are directly displayed here!
    FUNCTION_DEBUG;
    if (App::isFullScreen()||App::userSettings->doNotShowAcknowledgmentMessages)
        displayDialogs=false;
    int result=-3;
    CFileOperationsBase::handleVerSpec_loadModel1();
    if ((pathAndFilename==nullptr)||VFile::doesFileExist(pathAndFilename))
    {
        std::string theAcknowledgement;
        App::ct->objCont->deselectObjects();

        if (setCurrentDir&&(pathAndFilename!=nullptr))
            App::directories->modelDirectory=App::directories->getPathFromFull(pathAndFilename);

        if (displayDialogs)
            App::uiThread->showOrHideProgressBar(true,-1,"Loading model...");

        if (pathAndFilename!=nullptr)
        { // loading from file...
            CSer* serObj;
            int serializationVersion;
            unsigned short vrepVersionThatWroteThis;
            int licenseTypeThatWroteThis;
            char revisionNumber;

            if (CSer::getFileTypeFromName(pathAndFilename)!=CSer::filetype_unspecified_file)
            {
                serObj=new CSer(pathAndFilename,CSer::getFileTypeFromName(pathAndFilename));
                result=serObj->readOpenBinary(serializationVersion,vrepVersionThatWroteThis,licenseTypeThatWroteThis,revisionNumber,false);
            }

            std::string infoPrintOut(tt::decorateString("",IDSNS_LOADING_MODEL," ("));
            infoPrintOut+=std::string(pathAndFilename)+"). ";
    #ifdef SIM_WITH_GUI
            if ((result==-3)&&(App::mainWindow!=nullptr))
            {
                if (displayMessages)
                    App::addStatusbarMessage("The file does not seem to be a valid model file.");
                if (displayDialogs)
                {
                    App::uiThread->showOrHideProgressBar(false);
                    App::uiThread->messageBox_critical(App::mainWindow,IDSN_SERIALIZATION,"The file does not seem to be a valid model file.",VMESSAGEBOX_OKELI);
                    App::uiThread->showOrHideProgressBar(true);
                }
            }
            if (((result!=-3)&&displayMessages)&&(App::mainWindow!=nullptr))
            {
                infoPrintOut+=" ";
                infoPrintOut+=IDSNS_SERIALIZATION_VERSION_IS;
                infoPrintOut+=" ";
                infoPrintOut+=boost::lexical_cast<std::string>(serializationVersion)+".";
                App::addStatusbarMessage(infoPrintOut.c_str());
                infoPrintOut=_getStringOfVersionAndLicenseThatTheFileWasWrittenWith(vrepVersionThatWroteThis,licenseTypeThatWroteThis,revisionNumber);
                if (infoPrintOut!="")
                    App::addStatusbarMessage(infoPrintOut.c_str());
            }
            if ((result==-2)&&(App::mainWindow!=nullptr))
            {
                if (displayMessages)
                    App::addStatusbarMessage(IDS_SERIALIZATION_VERSION_NOT_SUPPORTED_ANYMORE);
                if (displayDialogs)
                {
                    App::uiThread->showOrHideProgressBar(false);
                    App::uiThread->messageBox_critical(App::mainWindow,strTranslate(IDSN_SERIALIZATION),strTranslate(IDS_SERIALIZATION_VERSION_NOT_SUPPORTED_ANYMORE),VMESSAGEBOX_OKELI);
                    App::uiThread->showOrHideProgressBar(true);
                }
            }
            if ((result==-1)&&(App::mainWindow!=nullptr))
            {
                if (displayMessages)
                    App::addStatusbarMessage(IDS_SERIALIZATION_VERSION_TOO_RECENT);
                if (displayDialogs)
                {
                    App::uiThread->showOrHideProgressBar(false);
                    App::uiThread->messageBox_critical(App::mainWindow,strTranslate(IDSN_SERIALIZATION),strTranslate(IDS_SERIALIZATION_VERSION_TOO_RECENT),VMESSAGEBOX_OKELI);
                    App::uiThread->showOrHideProgressBar(true);
                }
            }
            if ((result==0)&&(App::mainWindow!=nullptr))
            {
                if (displayMessages)
                    App::addStatusbarMessage(IDS_COMPRESSION_SCHEME_NOT_SUPPORTED);
                if (displayDialogs)
                {
                    App::uiThread->showOrHideProgressBar(false);
                    App::uiThread->messageBox_critical(App::mainWindow,strTranslate(IDSN_SERIALIZATION),strTranslate(IDS_COMPRESSION_SCHEME_NOT_SUPPORTED),VMESSAGEBOX_OKELI);
                    App::uiThread->showOrHideProgressBar(true);
                }
            }
    #endif
            if (result==1)
            {
                App::ct->objCont->loadModel(serObj[0],onlyThumbnail,forceModelAsCopy,nullptr,nullptr,nullptr);
                serObj->readClose();
                if (displayMessages&&(!onlyThumbnail))
                    App::addStatusbarMessage(IDSNS_MODEL_LOADED);
    #ifdef SIM_WITH_GUI
                if ((vrepVersionThatWroteThis>VREP_PROGRAM_VERSION_NB)&&displayDialogs&&(App::mainWindow!=nullptr)&&(!onlyThumbnail))
                {
                    App::uiThread->showOrHideProgressBar(false);
                    App::uiThread->messageBox_warning(App::mainWindow,strTranslate(IDSN_MODEL),strTranslate(IDS_MODEL_SAVED_WITH_MORE_RECENT_VERSION_WARNING),VMESSAGEBOX_OKELI);
                    App::uiThread->showOrHideProgressBar(true);
                }
    #endif
                if (!onlyThumbnail)
                {
                    std::string acknowledgement;
                    std::string tmp;
                    // now we search for the model base that contains the acknowledgment:
                    std::vector<C3DObject*> loadedObjects;
                    App::ct->objCont->getSelectedObjects(loadedObjects);
                    for (int obba=0;obba<int(loadedObjects.size());obba++)
                    {
                        if (loadedObjects[obba]->getParentObject()==nullptr)
                        {
                            acknowledgement=loadedObjects[obba]->getModelAcknowledgement();
                            tmp=acknowledgement;
                            tt::removeSpacesAtBeginningAndEnd(tmp);
                            break;
                        }
                    }

                    if (tmp.length()!=0)
                    {
                        if (acknowledgmentPointerInReturn==nullptr)
                        {

                            if (displayMessages)
                                theAcknowledgement=acknowledgement;
                        }
                        else
                            acknowledgmentPointerInReturn[0]=acknowledgement;
                    }
                }
            }
            else
            {
                if (displayMessages)
                    App::addStatusbarMessage(IDSNS_MODEL_COULD_NOT_BE_LOADED);
            }
            delete serObj;
        }
        else
        { // loading from buffer...
            CSer serObj(loadBuffer[0],CSer::filetype_vrep_bin_model_buff);
            int serializationVersion;
            unsigned short vrepVersionThatWroteThis;
            int licenseTypeThatWroteThis;
            char revisionNumber;
            result=serObj.readOpenBinary(serializationVersion,vrepVersionThatWroteThis,licenseTypeThatWroteThis,revisionNumber,false);
            if (result==1)
            {
                App::ct->objCont->loadModel(serObj,onlyThumbnail,forceModelAsCopy,nullptr,nullptr,nullptr);
                serObj.readClose();
            }
        }

        if (displayDialogs)
            App::uiThread->showOrHideProgressBar(false);

        App::ct->objCont->removeFromSelectionAllExceptModelBase(false);
        App::setRebuildHierarchyFlag();
        if (doUndoThingInHere)
        {
            POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
        }
#ifdef SIM_WITH_GUI
        if ((theAcknowledgement.length()!=0)&&displayDialogs&&(App::mainWindow!=nullptr))
        {
            SSimulationThreadCommand cmd;
            cmd.cmdId=POSTPONE_PROCESSING_THIS_LOOP_CMD;
            cmd.intParams.push_back(3);
            App::appendSimulationThreadCommand(cmd);
            cmd.cmdId=DISPLAY_ACKNOWLEDGMENT_MESSAGE_CMD;
            cmd.stringParams.push_back(IDS_MODEL_CONTENT_ACKNOWLEDGMENTS);
            cmd.stringParams.push_back(theAcknowledgement);
            App::appendSimulationThreadCommand(cmd);
        }
#endif
    }
    else
    {
        if (displayMessages)
            App::addStatusbarMessage(IDSNS_ABORTED_FILE_DOES_NOT_EXIST);
    }
    return(result==1);
}

bool CFileOperations::saveScene(const char* pathAndFilename,bool displayMessages,bool displayDialogs,bool setCurrentDir,bool changeSceneUniqueId)
{ // There is a similar routine in CUndoBuffer!!
    bool retVal=false;
    if (CFileOperationsBase::handleVerSpec_canSaveScene())
    {
        if (App::isFullScreen())
            displayDialogs=false;

#ifdef SIM_WITH_GUI
        if (App::mainWindow!=nullptr)
            App::mainWindow->codeEditorContainer->saveOrCopyOperationAboutToHappen();
#endif

        void* returnVal=CPluginContainer::sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_scenesave,nullptr,nullptr,nullptr);
        delete[] (char*)returnVal;

        CSer* serObj;
            serObj=new CSer(pathAndFilename,CSer::getFileTypeFromName(pathAndFilename));
            retVal=serObj->writeOpenBinary(App::userSettings->compressFiles);

        if (retVal)
        {
            if (displayDialogs)
                App::uiThread->showOrHideProgressBar(true,-1,"Saving scene...");
            App::ct->mainSettings->setScenePathAndName(pathAndFilename);

            App::ct->luaScriptContainer->sceneOrModelAboutToBeSaved(-1);

            if (changeSceneUniqueId)
                App::ct->environment->generateNewUniquePersistentIdString();

            if (setCurrentDir)
                App::directories->sceneDirectory=App::ct->mainSettings->getScenePath();

            std::string infoPrintOut(IDSN_SAVING_SCENE);
            infoPrintOut+=" (";
            infoPrintOut+=std::string(pathAndFilename)+"). ";
            infoPrintOut+=IDSNS_SERIALIZATION_VERSION_IS;
            infoPrintOut+=" ";
                infoPrintOut+=boost::lexical_cast<std::string>(CSer::SER_SERIALIZATION_VERSION)+".";
            if (displayMessages)
                App::addStatusbarMessage(infoPrintOut.c_str());

            App::ct->objCont->saveScene(serObj[0]);
            serObj->writeClose();

            if (displayMessages)
                App::addStatusbarMessage(IDSNS_SCENE_WAS_SAVED);

            if (displayDialogs)
                App::uiThread->showOrHideProgressBar(false);
            App::setRebuildHierarchyFlag(); // we might have saved under a different name, we need to reflect it
        }
        else
        {
            #ifdef SIM_WITH_GUI
                if ((App::mainWindow!=nullptr)&&displayDialogs)
                { // to avoid an error when saving a file that was opened while still attached to an email for instance
                    App::uiThread->messageBox_critical(App::mainWindow,strTranslate(IDSN_FILE_ACCESS),strTranslate(IDSN_ACCESS_TO_FILE_WAS_DENIED),VMESSAGEBOX_OKELI);
                }
            #endif
        }
        delete serObj;
    }
    return(retVal);
}

bool CFileOperations::saveModel(int modelBaseDummyID,const char* pathAndFilename,bool displayMessages,bool displayDialogs,bool setCurrentDir,std::vector<char>* saveBuffer/*=nullptr*/)
{
    if ( CFileOperationsBase::handleVerSpec_canSaveModel()||(saveBuffer!=nullptr) )
    {
        App::ct->luaScriptContainer->sceneOrModelAboutToBeSaved(modelBaseDummyID);
        if (App::isFullScreen())
            displayDialogs=false;
        std::vector<int> sel;
        sel.push_back(modelBaseDummyID);

        C3DObject* modelBaseObject=App::ct->objCont->getObjectFromHandle(modelBaseDummyID);
        C3Vector minV,maxV;
        bool b=true;
        C7Vector modelTr(modelBaseObject->getCumulativeTransformationPart1());
        C3Vector modelBBSize;
        float modelNonDefaultTranslationStepSize=modelBaseObject->getNonDefaultTranslationStepSize();

        if (modelBaseObject->getGlobalMarkingBoundingBox(modelTr.getInverse(),minV,maxV,b,true,false))
        {
            modelBBSize=maxV-minV;
            modelTr.X+=modelTr.Q*((minV+maxV)*0.5);
        }
        else
            modelBBSize=C3Vector::zeroVector;

#ifdef SIM_WITH_GUI
        if (App::mainWindow!=nullptr)
            App::mainWindow->codeEditorContainer->saveOrCopyOperationAboutToHappen();
#endif

        if (sel.size()>0)
        {
            CSceneObjectOperations::addRootObjectChildrenToSelection(sel);
            void* plugRetVal=CPluginContainer::sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_modelsave,nullptr,nullptr,nullptr);
            delete[] (char*)plugRetVal;

            std::string infoPrintOut(IDSNS_SAVING_MODEL);
            if (pathAndFilename!=nullptr)
            {
                if (setCurrentDir)
                    App::directories->modelDirectory=App::directories->getPathFromFull(pathAndFilename);
                infoPrintOut+=" (";
                infoPrintOut+=std::string(pathAndFilename)+"). ";
            }

            if (pathAndFilename!=nullptr)
            { // saving to file...
                CSer* serObj;
                if (CSer::getFileTypeFromName(pathAndFilename)!=CSer::filetype_unspecified_file)
                {
                    serObj=new CSer(pathAndFilename,CSer::getFileTypeFromName(pathAndFilename));
                    serObj->writeOpenBinary(App::userSettings->compressFiles);
                }
                App::ct->copyBuffer->serializeCurrentSelection(serObj[0],&sel,modelTr,modelBBSize,modelNonDefaultTranslationStepSize);
                serObj->writeClose();
                delete serObj;
            }
            else
            { // saving to buffer...
                CSer serObj(saveBuffer[0],CSer::filetype_vrep_bin_model_buff);

                serObj.writeOpenBinary(App::userSettings->compressFiles);
                App::ct->copyBuffer->serializeCurrentSelection(serObj,&sel,modelTr,modelBBSize,modelNonDefaultTranslationStepSize);
                serObj.writeClose();
            }
            infoPrintOut+=IDSNS_SERIALIZATION_VERSION_IS;
            infoPrintOut+=" ";
            infoPrintOut+=boost::lexical_cast<std::string>(CSer::SER_SERIALIZATION_VERSION)+".";

            if (displayMessages)
                App::addStatusbarMessage(infoPrintOut.c_str());
            if (displayMessages)
                App::addStatusbarMessage(IDSNS_MODEL_WAS_SAVED);
            return(true);
        }
    }
    return(false);
}

std::string CFileOperations::_getStringOfVersionAndLicenseThatTheFileWasWrittenWith(unsigned short vrepVer,int licenseType,char revision)
{
    if (vrepVer==0)
        return("");
    std::string retStr;
    retStr=tt::decorateString("",IDSNS_FILE_WAS_PREVIOUSLY_WRITTEN_WITH_VREP_VERSION," ");
    int v=vrepVer;
    retStr+=char('0')+(unsigned char)(v/10000);
    retStr+='.';
    v=v-(v/10000)*10000;
    retStr+=char('0')+(unsigned char)(v/1000);
    v=v-(v/1000)*1000;
    retStr+=char('0')+(unsigned char)(v/100);
    v=v-(v/100)*100;
    retStr+='.';
    retStr+=char('0')+(unsigned char)(v/10);
    v=v-(v/10)*10;
    retStr+=char('0')+(unsigned char)v;

    retStr+=" (rev ";
    retStr+=tt::FNb(0,(int)revision);
    retStr+=')';

    if (licenseType!=-1)
    {
        licenseType=(licenseType|0x00040000)-0x00040000;
        if (licenseType==VREP_LICENSE_VREP_DEMO)
            retStr+=" (V-REP EVAL/STUDENT license)";
        if (licenseType==VREP_LICENSE_VREP_PRO_EDU)
            retStr+=" (V-REP PRO EDU license)";
        if ((licenseType==VREP_LICENSE_VREP_PRO)||(licenseType==VREP_LICENSE_VREP_PRO_P))
            retStr+=" (V-REP PRO license)";
        if (licenseType==VREP_LICENSE_VREP_SUBLICENSE)
            retStr+=" (custom V-REP license)";
        if (licenseType==VREP_LICENSE_VREP_PLAYER)
            retStr+=" (V-REP PLAYER license)";
        if (licenseType==VREP_LICENSE_VREP_BASIC)
            retStr+=" (V-REP license, custom compilation)";
        if (licenseType==VREP_LICENSE_BLUE_REALITY)
            retStr+=" (BlueReality license)";
    }
    return(retStr);
}

bool CFileOperations::heightfieldImportRoutine(const std::string& pathName)
{
    if (VFile::doesFileExist(pathName))
    {
        try
        {
            std::vector<std::vector<float>*> readData;
            // We read each line at a time, which gives rows:
            int minRow=-1;

            std::string ext(CTTUtil::getLowerCaseString(VVarious::splitPath_fileExtension(pathName).c_str()));
            if ((ext.compare("csv")!=0)&&(ext.compare("txt")!=0))
            { // from image file
                int resX,resY,n;
                unsigned char* data=CImageLoaderSaver::load(pathName.c_str(),&resX,&resY,&n,3);
                if (data!=nullptr)
                {
                    if ( (resX>1)&&(resY>1) )
                    {
                        int bytesPerPixel=0;
                        if (n==3)
                            bytesPerPixel=3;
                        if (n==4)
                            bytesPerPixel=4;
                        if (bytesPerPixel!=0)
                        {
                            for (int i=0;i<resY;i++)
                            {
                                std::vector<float>* lineVect=new std::vector<float>;
                                for (int j=0;j<resX;j++)
                                    lineVect->push_back(float(data[bytesPerPixel*(i*resX+j)+0]+data[bytesPerPixel*(i*resX+j)+1]+data[bytesPerPixel*(i*resX+j)+2])/768.0f);
                                readData.push_back(lineVect);
                            }
                            minRow=resX;
                        }
                    }
                    delete[] data;
                }
                else
                {
                    #ifdef SIM_WITH_GUI
                        App::uiThread->messageBox_critical(App::mainWindow,strTranslate(IDSN_IMPORT),strTranslate(IDS_TEXTURE_FILE_COULD_NOT_BE_LOADED),VMESSAGEBOX_OKELI);
                    #endif
                }
            }
            else
            { // from csv or txt file:
                VFile file(pathName.c_str(),VFile::READ|VFile::SHARE_DENY_NONE);
                VArchive archive(&file,VArchive::LOAD);
                unsigned int currentPos=0;
                std::string line;
                while (archive.readSingleLine(currentPos,line,false))
                {
                    std::string word;
                    std::vector<float>* lineVect=new std::vector<float>;
                    while (tt::extractCommaSeparatedWord(line,word))
                    {
                        tt::removeSpacesAtBeginningAndEnd(word);
                        float val;
                        if (tt::getValidFloat(word,val))
                            lineVect->push_back(val);
                        else
                            break;
                    }
                    if (lineVect->size()!=0)
                    {
                        if ( (minRow==-1)||(int(lineVect->size())<minRow) )
                            minRow=int(lineVect->size());
                        readData.push_back(lineVect);
                    }
                    else
                    {
                        delete lineVect;
                        break;
                    }
                }
                archive.close();
                file.close();
            }
            if ( (readData.size()>1)&&(minRow>1) )
            {
                int xSize=minRow;
                int ySize=int(readData.size());

                // Display the heightfield scaling dialog:
                SUIThreadCommand cmdIn;
                SUIThreadCommand cmdOut;
                cmdIn.cmdId=HEIGHTFIELD_DIMENSION_DLG_UITHREADCMD;
                cmdIn.floatParams.push_back(10.0f);
                cmdIn.floatParams.push_back(float(ySize-1)/float(xSize-1));
                App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
                if (cmdOut.floatParams.size()==0)
                {
                    cmdOut.floatParams.push_back(1.0f);
                    cmdOut.floatParams.push_back(1.0f);
                }
                float pointSpacing=cmdOut.floatParams[0]/float(xSize-1);
                for (int i=0;i<int(readData.size());i++)
                {
                    for (int j=0;j<int(readData[i]->size());j++)
                    {
                        readData[i]->at(j)*=cmdOut.floatParams[1];
                    }
                }
                int shapeHandle=apiAddHeightfieldToScene(xSize,pointSpacing,readData,0.0f,2);
                App::ct->objCont->deselectObjects();
                App::ct->objCont->addObjectToSelection(shapeHandle);

            }
            for (int i=0;i<int(readData.size());i++)
                delete readData[i];
            return(readData.size()!=0);
        }
        catch(VFILE_EXCEPTION_TYPE e)
        {
            VFile::reportAndHandleFileExceptionError(e);
            return(false);
        }
    }
    return(false);
}

int CFileOperations::apiAddHeightfieldToScene(int xSize,float pointSpacing,const std::vector<std::vector<float>*>& readData,float shadingAngle,int options)
{ // options bits:
    // 0 set --> backfaces are culled
    // 1 set --> edges are visible
    // 2 set --> a normal shape is created instead
    // 4 set --> non respondable
    int ySize=int(readData.size());
    std::vector<float> allHeights;
    float maxHeight=-99999999.0f;
    float minHeight=+99999999.0f;
    for (int i=ySize-1;i>=0;i--)
    {
        for (int j=0;j<xSize;j++)
        {
            allHeights.push_back(readData[i]->at(j));
            if (readData[i]->at(j)>maxHeight)
                maxHeight=readData[i]->at(j);
            if (readData[i]->at(j)<minHeight)
                minHeight=readData[i]->at(j);
        }
    }
    CGeomProxy* geom=new CGeomProxy(allHeights,xSize,ySize,pointSpacing,maxHeight-minHeight);
    CShape* shape=new CShape();
    shape->setLocalTransformation(geom->getCreationTransformation());
    geom->setCreationTransformation(C7Vector::identityTransformation);
    shape->geomData=geom;

    if (options&4)
        ((CGeometric*)geom->geomInfo)->setPurePrimitiveType(sim_pure_primitive_none,1.0f,1.0f,1.0f);

    App::ct->objCont->addObjectToScene(shape,false,true);
    shape->setCulling((options&1)!=0);
    shape->setVisibleEdges((options&2)!=0);
    ((CGeometric*)shape->geomData->geomInfo)->setGouraudShadingAngle(shadingAngle);
    ((CGeometric*)shape->geomData->geomInfo)->setEdgeThresholdAngle(shadingAngle);
    ((CGeometric*)shape->geomData->geomInfo)->color.colors[0]=0.68f;
    ((CGeometric*)shape->geomData->geomInfo)->color.colors[1]=0.56f;
    ((CGeometric*)shape->geomData->geomInfo)->color.colors[2]=0.36f;
    ((CGeometric*)shape->geomData->geomInfo)->color.colors[6]=0.25f;
    ((CGeometric*)shape->geomData->geomInfo)->color.colors[7]=0.25f;
    ((CGeometric*)shape->geomData->geomInfo)->color.colors[8]=0.25f;
    std::string tempName(IDSOGL_HEIGHTFIELD);
    while (App::ct->objCont->getObjectFromName(tempName.c_str())!=nullptr)
        tempName=tt::generateNewName_noDash(tempName);
    App::ct->objCont->renameObject(shape->getObjectHandle(),tempName.c_str());
    tempName=tt::getObjectAltNameFromObjectName(IDSOGL_HEIGHTFIELD);
    while (App::ct->objCont->getObjectFromAltName(tempName.c_str())!=nullptr)
        tempName=tt::generateNewName_noDash(tempName);
    App::ct->objCont->altRenameObject(shape->getObjectHandle(),tempName.c_str());

    shape->alignBoundingBoxWithWorld();

    int propToRemove=sim_objectspecialproperty_collidable|sim_objectspecialproperty_measurable;
    shape->setLocalObjectSpecialProperty((shape->getLocalObjectSpecialProperty()|propToRemove)-propToRemove);
    shape->setRespondable((options&8)==0);
    shape->setShapeIsDynamicallyStatic(true);

    return(shape->getObjectHandle());
}

void CFileOperations::addToRecentlyOpenedScenes(std::string filenameAndPath)
{
    CPersistentDataContainer cont(FILENAME_OF_USER_SETTINGS_IN_BINARY_FILE);
    std::string recentScenes[10];
    int sameIndex=-1;
    for (int i=0;i<10;i++)
    {
        std::string tmp("SIMSETTINGS_RECENTSCENE0");
        tmp[23]=48+i;
        cont.readData(tmp.c_str(),recentScenes[i]);
        if (recentScenes[i].compare(filenameAndPath)==0)
            sameIndex=i;
    }
    if (sameIndex==-1)
    {
        for (int i=8;i>=0;i--)
            recentScenes[i+1]=recentScenes[i];
    }
    else
    {
        for (int i=sameIndex;i>0;i--)
            recentScenes[i]=recentScenes[i-1];
    }
    recentScenes[0]=filenameAndPath;
    int cnt=0;
    for (int i=0;i<10;i++)
    {
        if (recentScenes[i].length()>3)
        {
            std::string tmp("SIMSETTINGS_RECENTSCENE0");
            tmp[23]=48+cnt;
            cont.writeData(tmp.c_str(),recentScenes[i],!App::userSettings->doNotWritePersistentData);
            cnt++;
        }
    }
}

void CFileOperations::_removeFromRecentlyOpenedScenes(std::string filenameAndPath)
{
    CPersistentDataContainer cont(FILENAME_OF_USER_SETTINGS_IN_BINARY_FILE);
    std::string recentScenes[10];
    int sameIndex=-1;
    for (int i=0;i<10;i++)
    {
        std::string tmp("SIMSETTINGS_RECENTSCENE0");
        tmp[23]=48+i;
        cont.readData(tmp.c_str(),recentScenes[i]);
        if (recentScenes[i].compare(filenameAndPath)==0)
            sameIndex=i;
    }
    if (sameIndex!=-1)
    {
        for (int i=sameIndex;i<9;i++)
            recentScenes[i]=recentScenes[i+1];
        recentScenes[9]="";
        int cnt=0;
        for (int i=0;i<10;i++)
        {
            if (recentScenes[i].length()>3)
            {
                std::string tmp("SIMSETTINGS_RECENTSCENE0");
                tmp[23]=48+cnt;
                cont.writeData(tmp.c_str(),recentScenes[i],!App::userSettings->doNotWritePersistentData);
                cnt++;
            }
        }
        for (int i=cnt;i<10;i++)
        {
            std::string tmp("SIMSETTINGS_RECENTSCENE0");
            tmp[23]=48+i;
            cont.writeData(tmp.c_str(),"",!App::userSettings->doNotWritePersistentData);
        }
    }
}

bool CFileOperations::apiExportIkContent(const char* pathAndName,bool displayDialogs)
{ // Call only from SIM thread
    CExtIkSer ar;
    App::ct->objCont->exportIkContent(ar);
    bool retVal=true;
    if (displayDialogs)
        App::uiThread->showOrHideProgressBar(true,-1,"Exporting IK content...");
    try
    {
        VFile myFile(pathAndName,VFile::CREATE_WRITE|VFile::SHARE_EXCLUSIVE);
        VArchive arch(&myFile,VArchive::STORE);

        int dataLength;
        unsigned char* data=ar.getBuffer(dataLength);
        for (int i=0;i<dataLength;i++)
            arch << data[i];
        arch.close();
        myFile.close();
    }
    catch(VFILE_EXCEPTION_TYPE e)
    {
        retVal=false;
    }
    if (displayDialogs)
        App::uiThread->showOrHideProgressBar(false);
    return(retVal);
}

#ifdef SIM_WITH_GUI
void CFileOperations::keyPress(int key)
{
    if (key==CTRL_S_KEY)
        processCommand(FILE_OPERATION_SAVE_SCENE_FOCMD);
    if (key==CTRL_O_KEY)
        processCommand(FILE_OPERATION_OPEN_SCENE_FOCMD);
    if (key==CTRL_W_KEY)
        processCommand(FILE_OPERATION_CLOSE_SCENE_FOCMD);
    if (key==CTRL_Q_KEY)
        processCommand(FILE_OPERATION_EXIT_SIMULATOR_FOCMD);
    if (key==CTRL_N_KEY)
        processCommand(FILE_OPERATION_NEW_SCENE_FOCMD);
}

void CFileOperations::addMenu(VMenu* menu)
{
    CFileOperationsBase::handleVerSpec_addMenu1(menu);
}

bool CFileOperations::_saveSceneWithDialogAndEverything()
{ // SHOULD ONLY BE CALLED BY THE MAIN SIMULATION THREAD!
    bool retVal=false;
    if (!App::ct->environment->getSceneLocked())
    {
        if (App::ct->mainSettings->getScenePathAndName()=="")
            retVal=_saveSceneAsWithDialogAndEverything(CFileOperationsBase::handleVerSpec_saveSceneAsWithDialogAndEverything1());
        else
        {
            if ( (!App::ct->environment->getRequestFinalSave())||(VMESSAGEBOX_REPLY_YES==App::uiThread->messageBox_warning(App::mainWindow,strTranslate(IDSN_SAVE),strTranslate(IDS_FINAL_SCENE_SAVE_WARNING),VMESSAGEBOX_YES_NO)) )
            {
                if (App::ct->environment->getRequestFinalSave())
                    App::ct->environment->setSceneLocked();
                std::string infoPrintOut(IDSN_SAVING_SCENE);
                infoPrintOut+="...";
                App::addStatusbarMessage(infoPrintOut.c_str());
                if (saveScene(App::ct->mainSettings->getScenePathAndName().c_str(),true,true,true,false))
                {
                    addToRecentlyOpenedScenes(App::ct->mainSettings->getScenePathAndName());
                    App::ct->undoBufferContainer->clearSceneSaveMaybeNeededFlag();
                }
            }
            retVal=true;
        }
    }
    else
        App::uiThread->messageBox_warning(App::mainWindow,strTranslate(IDSN_SCENE),strTranslate(IDS_SCENE_IS_LOCKED_WARNING),VMESSAGEBOX_OKELI);
    return(retVal);
}

bool CFileOperations::_saveSceneAsWithDialogAndEverything(int filetype)
{
    bool retVal=false;
    if (!App::ct->environment->getSceneLocked())
    {
        if ( ((!App::ct->environment->getRequestFinalSave())||(filetype==CSer::filetype_vrep_bin_scene_file)) ||(VMESSAGEBOX_REPLY_YES==App::uiThread->messageBox_warning(App::mainWindow,strTranslate(IDSN_SAVE),strTranslate(IDS_FINAL_SCENE_SAVE_WARNING),VMESSAGEBOX_YES_NO)) )
        {
            if (App::ct->environment->getRequestFinalSave()&&(filetype!=CSer::filetype_vrep_bin_scene_file))
                App::ct->environment->setSceneLocked();

            std::string infoPrintOut(IDSN_SAVING_SCENE);
            infoPrintOut+="...";
            App::addStatusbarMessage(infoPrintOut.c_str());
            std::string initPath;
            if (App::ct->mainSettings->getScenePathAndName().size()==0)
                initPath=App::directories->sceneDirectory;
            else
                initPath=App::ct->mainSettings->getScenePath();
            std::string filenameAndPath=CFileOperationsBase::handleVerSpec_saveSceneAsWithDialogAndEverything2(filetype,initPath);

            if (filenameAndPath.length()!=0)
            {
                if (saveScene(filenameAndPath.c_str(),true,true,true,true))
                {
                    addToRecentlyOpenedScenes(App::ct->mainSettings->getScenePathAndName());
                    App::ct->undoBufferContainer->clearSceneSaveMaybeNeededFlag();
                    retVal=true;
                }
            }
            else
                App::addStatusbarMessage(IDSNS_ABORTED);
        }
    }
    else
        App::uiThread->messageBox_warning(App::mainWindow,strTranslate(IDSN_SCENE),strTranslate(IDS_SCENE_IS_LOCKED_WARNING),VMESSAGEBOX_OKELI);
    return(retVal);
}
#endif
