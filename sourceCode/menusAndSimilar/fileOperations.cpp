#include "simInternal.h"
#include "fileOperations.h"
#include "simulation.h"
#include "tt.h"
#include "persistentDataContainer.h"
#include "sceneObjectOperations.h"
#include "algos.h"
#include "app.h"
#include "pluginContainer.h"
#include "meshManip.h"
#include "mesh.h"
#include "simStrings.h"
#include <boost/lexical_cast.hpp>
#include "imgLoaderSaver.h"
#include "vVarious.h"
#include "vDateTime.h"
#include "ttUtil.h"
#include "simFlavor.h"
#include <boost/algorithm/string/predicate.hpp>
#ifdef SIM_WITH_GUI
    #include "vFileDialog.h"
    #include "vMessageBox.h"
#endif
#include "vFileFinder.h"

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
    { // Cannot undo this command
        if ( App::currentWorld->simulation->isSimulationStopped()&&(App::getEditModeType()==NO_EDIT_MODE) )
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
        if (App::currentWorld->simulation->isSimulationStopped()&&(App::getEditModeType()==NO_EDIT_MODE) )
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
        if ( App::currentWorld->simulation->isSimulationStopped()&&(App::getEditModeType()==NO_EDIT_MODE) )
        { // execute the command only when simulation is not running and not in an edit mode
            if (!VThread::isCurrentThreadTheUiThread())
            { // we are NOT in the UI thread. We execute the command now:
                App::logMsg(sim_verbosity_msgs,tt::decorateString("",IDSNS_LOADING_SCENE,"...").c_str());
                CSimFlavor::run(2);
                std::string ext[4];
                for (int i=0;i<4;i++)
                    ext[i]=CSimFlavor::getStringVal_int(1,i);
                std::string filenameAndPath=App::uiThread->getOpenFileName(App::mainWindow,0,IDSN_LOADING_SCENE,App::folders->getScenesPath().c_str(),"",false,"Scenes",ext[0].c_str(),ext[1].c_str(),ext[2].c_str(),ext[3].c_str());

                if (filenameAndPath.length()!=0)
                {
                    App::setRebuildHierarchyFlag();
                    App::setDefaultMouseMode();
                    App::worldContainer->createNewWorld();
                    createNewScene(true,false);
                    if (loadScene(filenameAndPath.c_str(),true,true,true))
                        addToRecentlyOpenedScenes(filenameAndPath);
                    else
                        _removeFromRecentlyOpenedScenes(filenameAndPath);
                }
                else
                    App::logMsg(sim_verbosity_msgs,IDSNS_ABORTED);
                App::currentWorld->undoBufferContainer->clearSceneSaveMaybeNeededFlag();
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
        App::appendSimulationThreadCommand(cmd2);
        return(true);
    }
    if ((cmd.cmdId>=FILE_OPERATION_OPEN_RECENT_SCENE0_PHASE2_FOCMD)&&(cmd.cmdId<=FILE_OPERATION_OPEN_RECENT_SCENE9_PHASE2_FOCMD))
    {
        if (App::currentWorld->simulation->isSimulationStopped()&&(App::getEditModeType()==NO_EDIT_MODE) )
        { // execute the command only when simulation is not running and not in an edit mode
            if (!VThread::isCurrentThreadTheUiThread())
            { // we are NOT in the UI thread. We execute the command now:
                CSimFlavor::run(2);
                CPersistentDataContainer cont(SIM_FILENAME_OF_USER_SETTINGS_IN_BINARY_FILE);
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

                if (VFile::doesFileExist(filenameAndPath.c_str()))
                {
                    App::setDefaultMouseMode();
                    App::worldContainer->createNewWorld();
                    CFileOperations::createNewScene(true,false);

                    if (loadScene(filenameAndPath.c_str(),true,true,true))
                        addToRecentlyOpenedScenes(filenameAndPath);
                    else
                        _removeFromRecentlyOpenedScenes(filenameAndPath);
                    App::currentWorld->undoBufferContainer->clearSceneSaveMaybeNeededFlag();
                }
                else
                { // file does not exist anymore
                    App::uiThread->messageBox_information(App::mainWindow,"Open Recent Scene","File does not exist anymore.",VMESSAGEBOX_OKELI,VMESSAGEBOX_REPLY_OK);
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
            CSimFlavor::run(2);
            std::string ext[4];
            for (int i=0;i<4;i++)
                ext[i]=CSimFlavor::getStringVal_int(2,i);
            std::string filenameAndPath=App::uiThread->getOpenFileName(App::mainWindow,0,IDSN_LOADING_MODEL,App::folders->getModelsPath().c_str(),"",false,"Models",ext[0].c_str(),ext[1].c_str(),ext[2].c_str(),ext[3].c_str());

            if (filenameAndPath.length()!=0)
                loadModel(filenameAndPath.c_str(),true,true,true,true,nullptr,false,false); // Undo things is in here.
            else
                App::logMsg(sim_verbosity_msgs,IDSNS_ABORTED);
        }
        else
            App::appendSimulationThreadCommand(cmd); // We are in the UI thread. Execute the command via the main thread
        return(true);
    }
    if (cmd.cmdId==FILE_OPERATION_SAVE_SCENE_FOCMD)
    {
        if (App::currentWorld->simulation->isSimulationStopped()&&(App::getEditModeType()==NO_EDIT_MODE) )
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
            if (!App::currentWorld->environment->getSceneLocked())
            {
                App::logMsg(sim_verbosity_msgs,IDSNS_EXPORTING_IK_CONTENT);
                std::string tst(App::folders->getOtherFilesPath());
                std::string filenameAndPath=App::uiThread->getSaveFileName(App::mainWindow,0,IDS_EXPORTING_IK_CONTENT___,tst.c_str(),"",false,"Coppelia Kinematics Content Files","ik");
                if (filenameAndPath.length()!=0)
                {
                    App::folders->setOtherFilesPath(App::folders->getPathFromFull(filenameAndPath.c_str()).c_str());
                    if (apiExportIkContent(filenameAndPath.c_str(),true))
                        App::logMsg(sim_verbosity_msgs,IDSNS_DONE);
                    else
                        App::logMsg(sim_verbosity_errors,IDSNS_EXPORT_OPERATION_FAILED);
                }
                else
                    App::logMsg(sim_verbosity_msgs,IDSNS_ABORTED);
            }
            else
                App::uiThread->messageBox_warning(App::mainWindow,IDSN_EXPORT,IDS_SCENE_IS_LOCKED_WARNING,VMESSAGEBOX_OKELI,VMESSAGEBOX_REPLY_OK);
        }
        else
            App::appendSimulationThreadCommand(cmd); // We are in the UI thread. Execute the command via the main thread
        return(true);
    }
    if ( (cmd.cmdId==FILE_OPERATION_SAVE_SCENE_AS_CSIM_FOCMD)||(cmd.cmdId==FILE_OPERATION_SAVE_SCENE_AS_EXXML_FOCMD)||(cmd.cmdId==FILE_OPERATION_SAVE_SCENE_AS_SIMPLEXML_FOCMD)||(cmd.cmdId==FILE_OPERATION_SAVE_SCENE_AS_XR_FOCMD) )
    {
        if (App::currentWorld->simulation->isSimulationStopped()&&(App::getEditModeType()==NO_EDIT_MODE) )
        { // execute the command only when simulation is not running and not in an edit mode
            if (!VThread::isCurrentThreadTheUiThread())
            { // we are NOT in the UI thread. We execute the command now:
                int filetype=CSer::filetype_unspecified_file;
                if (cmd.cmdId==FILE_OPERATION_SAVE_SCENE_AS_CSIM_FOCMD)
                    filetype=CSer::filetype_csim_bin_scene_file;
                if (cmd.cmdId==FILE_OPERATION_SAVE_SCENE_AS_EXXML_FOCMD)
                    filetype=CSer::filetype_csim_xml_xscene_file;
                if (cmd.cmdId==FILE_OPERATION_SAVE_SCENE_AS_SIMPLEXML_FOCMD)
                    filetype=CSer::filetype_csim_xml_simplescene_file;
                if (cmd.cmdId==FILE_OPERATION_SAVE_SCENE_AS_XR_FOCMD)
                    filetype=CSer::filetype_xr_bin_scene_file;
                _saveSceneAsWithDialogAndEverything(filetype);
            }
            else
                App::appendSimulationThreadCommand(cmd); // We are in the UI thread. Execute the command via the main thread
        }
        return(true);
    }
    if ( (cmd.cmdId==FILE_OPERATION_SAVE_MODEL_AS_CSIM_FOCMD)||(cmd.cmdId==FILE_OPERATION_SAVE_MODEL_AS_XR_FOCMD)||(cmd.cmdId==FILE_OPERATION_SAVE_MODEL_AS_EXXML_FOCMD) )
    {
        if (App::currentWorld->simulation->isSimulationStopped()&&(App::getEditModeType()==NO_EDIT_MODE) )
        { // execute the command only when simulation is not running and not in an edit mode
            if (!VThread::isCurrentThreadTheUiThread())
            { // we are NOT in the UI thread. We execute the command now:
                std::vector<int> sel;
                for (size_t i=0;i<App::currentWorld->sceneObjects->getSelectionCount();i++)
                    sel.push_back(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
                if (!App::currentWorld->environment->getSceneLocked())
                {
                    std::string infoM(IDSNS_SAVING_MODEL);
                    infoM+="...";
                    App::logMsg(sim_verbosity_msgs,infoM.c_str());
                    if (sel.size()!=0)
                    {
                        int modelBase=App::currentWorld->sceneObjects->getLastSelectionHandle();

                        // Display a warning if needed
                        CPersistentDataContainer cont(SIM_FILENAME_OF_USER_SETTINGS_IN_BINARY_FILE);
                        std::string val;
                        cont.readData("SIMSETTINGS_MODEL_SAVE_OFFSET_WARNING",val);
                        int intVal=0;
                        tt::getValidInt(val.c_str(),intVal);
                        if (intVal<1)
                        {
                            if (App::uiThread->messageBox_checkbox(App::mainWindow,IDSN_MODEL,IDSN_MODEL_SAVE_POSITION_OFFSET_INFO,IDSN_DO_NOT_SHOW_THIS_MESSAGE_AGAIN,false))
                            {
                                intVal++;
                                val=tt::FNb(intVal);
                                cont.writeData("SIMSETTINGS_MODEL_SAVE_OFFSET_WARNING",val,!App::userSettings->doNotWritePersistentData);
                            }
                        }

                        bool keepCurrentThumbnail=false;
                        bool operationCancelled=false;
                        int ft=-1;
                        if (cmd.cmdId==FILE_OPERATION_SAVE_MODEL_AS_CSIM_FOCMD)
                            ft=0;
                        if (cmd.cmdId==FILE_OPERATION_SAVE_MODEL_AS_XR_FOCMD)
                            ft=1;
                        if (cmd.cmdId==FILE_OPERATION_SAVE_MODEL_AS_EXXML_FOCMD)
                            ft=2;
                        while (true)
                        {
                            if (App::currentWorld->environment->modelThumbnail_notSerializedHere.hasImage())
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
                            std::string filenameAndPath;
                            if (ft==0)
                                filenameAndPath=App::uiThread->getSaveFileName(App::mainWindow,0,IDS_SAVING_MODEL___,App::folders->getModelsPath().c_str(),"",false,"CoppeliaSim Model",SIM_MODEL_EXTENSION);
                            if (ft==1)
                                filenameAndPath=App::uiThread->getSaveFileName(App::mainWindow,0,IDS_SAVING_MODEL___,App::folders->getModelsPath().c_str(),"",false,"XReality Model",SIM_XR_MODEL_EXTENSION);
                            if (ft==2)
                                filenameAndPath=App::uiThread->getSaveFileName(App::mainWindow,0,IDS_SAVING_MODEL___,App::folders->getModelsPath().c_str(),"",false,"CoppeliaSim XML Model (exhaustive)",SIM_XML_MODEL_EXTENSION);
                            if (ft==3)
                                filenameAndPath=App::uiThread->getSaveFileName(App::mainWindow,0,IDS_SAVING_MODEL___,App::folders->getModelsPath().c_str(),"",false,"CoppeliaSim XML Model (simple)",SIM_XML_MODEL_EXTENSION);

                            if (filenameAndPath.length()!=0)
                            {


                                bool abort=false;
                                if (!App::userSettings->suppressXmlOverwriteMsg)
                                {
                                    if ( (CSer::getFileTypeFromName(filenameAndPath.c_str())==CSer::filetype_csim_xml_xmodel_file)&&(App::userSettings->xmlExportSplitSize!=0) )
                                    {
                                        std::string prefix(VVarious::splitPath_fileBase(filenameAndPath.c_str())+"_");
                                        int cnt0=VFileFinder::countFiles(VVarious::splitPath_path(filenameAndPath.c_str()).c_str());
                                        int cnt1=VFileFinder::countFolders(VVarious::splitPath_path(filenameAndPath.c_str()).c_str());
                                        int cnt2=VFileFinder::countFilesWithPrefix(VVarious::splitPath_path(filenameAndPath.c_str()).c_str(),prefix.c_str());
                                        int cnt3=VFileFinder::countFilesWithPrefix(VVarious::splitPath_path(filenameAndPath.c_str()).c_str(),(VVarious::splitPath_fileBase(filenameAndPath.c_str())+".").c_str());
                                        if ( (cnt2+cnt3!=cnt0)||(cnt1!=0) )
                                        {
                                            std::string msg("The scene/model will possibly be saved as several separate files, all with the '");
                                            msg+=prefix;
                                            msg+="' prefix. Existing files with the same prefix will be erased or overwritten. To avoid this, it is recommended to either save XML scenes/models in individual folders, or to set the 'xmlExportSplitSize' variable in 'system/usrset.txt' to 0 to generate a single file.\n(this warning can be disabled via the 'suppressXmlOverwriteMsg' variable in 'system/usrset.txt')\n\nProceed anyway?";
                                            abort=(VMESSAGEBOX_REPLY_NO==App::uiThread->messageBox_warning(App::mainWindow,IDSN_SAVE,msg.c_str(),VMESSAGEBOX_YES_NO,VMESSAGEBOX_REPLY_YES));
                                        }
                                    }
                                    if (CSer::getFileTypeFromName(filenameAndPath.c_str())==CSer::filetype_csim_xml_simplemodel_file)
                                    {
                                        std::string prefix(VVarious::splitPath_fileBase(filenameAndPath.c_str())+"_");
                                        int cnt0=VFileFinder::countFiles(VVarious::splitPath_path(filenameAndPath.c_str()).c_str());
                                        int cnt1=VFileFinder::countFolders(VVarious::splitPath_path(filenameAndPath.c_str()).c_str());
                                        int cnt2=VFileFinder::countFilesWithPrefix(VVarious::splitPath_path(filenameAndPath.c_str()).c_str(),prefix.c_str());
                                        int cnt3=VFileFinder::countFilesWithPrefix(VVarious::splitPath_path(filenameAndPath.c_str()).c_str(),(VVarious::splitPath_fileBase(filenameAndPath.c_str())+".").c_str());
                                        if ( (cnt2+cnt3!=cnt0)||(cnt1!=0) )
                                        {
                                            std::string msg("The scene/model will possibly be saved as several separate files, all with the '");
                                            msg+=prefix;
                                            msg+="' prefix. Existing files with the same prefix will be erased or overwritten. To avoid this, it is recommended to save XML scenes/models in individual folders.\n(this warning can be disabled via the 'suppressXmlOverwriteMsg' variable in 'system/usrset.txt')\n\nProceed anyway?";
                                            abort=(VMESSAGEBOX_REPLY_NO==App::uiThread->messageBox_warning(App::mainWindow,IDSN_SAVE,msg.c_str(),VMESSAGEBOX_YES_NO,VMESSAGEBOX_REPLY_YES));
                                        }
                                    }
                                }
                                if (!abort)
                                {
                                    saveModel(modelBase,filenameAndPath.c_str(),true,true,true);
                                    App::currentWorld->sceneObjects->deselectObjects();
                                }
                                else
                                    App::logMsg(sim_verbosity_msgs,IDSNS_ABORTED);
                            }
                            else
                                App::logMsg(sim_verbosity_msgs,IDSNS_ABORTED);
                        }
                        else
                            App::logMsg(sim_verbosity_msgs,IDSNS_ABORTED);
                    }
                    else
                        App::logMsg(sim_verbosity_errors,IDSNS_CANNOT_PROCEED_SELECTION_IS_EMPTY);
                }
                else
                    App::uiThread->messageBox_warning(App::mainWindow,IDSN_MODEL,IDS_SCENE_IS_LOCKED_WARNING,VMESSAGEBOX_OKELI,VMESSAGEBOX_REPLY_OK);
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
                App::logMsg(sim_verbosity_msgs,IDS_IMPORTING_MESH___);
                App::currentWorld->sceneObjects->deselectObjects();
                std::string tst(App::folders->getCadFilesPath());

                std::vector<std::string> filenamesAndPaths;
                if (App::uiThread->getOpenFileNames(filenamesAndPaths,App::mainWindow,0,IDS_IMPORTING_MESH___,tst.c_str(),"",false,"Mesh files","obj","dxf","ply","stl","dae"))
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
                    App::worldContainer->sandboxScript->callScriptFunctionEx("simAssimp.importShapesDlg",&stack);
                }
                else
                    App::logMsg(sim_verbosity_msgs,IDSNS_ABORTED);
            }
            else
                App::uiThread->messageBox_critical(App::mainWindow,IDSN_EXPORT,"Assimp plugin was not found, cannot import",VMESSAGEBOX_OKELI,VMESSAGEBOX_REPLY_OK);
        }
        else
            App::appendSimulationThreadCommand(cmd); // We are in the UI thread. Execute the command via the main thread
        return(true);
    }
    if (cmd.cmdId==FILE_OPERATION_IMPORT_PATH_FOCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            App::logMsg(sim_verbosity_msgs,IDSNS_IMPORTING_PATH_FROM_CSV_FILE);
            App::currentWorld->sceneObjects->deselectObjects();
            std::string tst(App::folders->getCadFilesPath());
            std::string filenameAndPath=App::uiThread->getOpenFileName(App::mainWindow,0,IDS_IMPORTING_PATH_FROM_CSV_FILE,tst.c_str(),"",false,"CSV Files","csv");
            if (filenameAndPath.length()!=0)
            {
                if (VFile::doesFileExist(filenameAndPath.c_str()))
                {
                    if (_pathImportRoutine(filenameAndPath.c_str(),true))
                        App::logMsg(sim_verbosity_msgs,IDSNS_DONE);
                    else
                        App::logMsg(sim_verbosity_errors,IDSNS_AN_ERROR_OCCURRED_DURING_THE_IMPORT_OPERATION);
                    POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
                }
                else
                    App::logMsg(sim_verbosity_errors,IDSNS_AN_ERROR_OCCURRED_DURING_THE_IMPORT_OPERATION);
            }
            else
                App::logMsg(sim_verbosity_msgs,IDSNS_ABORTED);
        }
        else
            App::appendSimulationThreadCommand(cmd); // We are in the UI thread. Execute the command via the main thread
        return(true);
    }

    if (cmd.cmdId==FILE_OPERATION_IMPORT_HEIGHTFIELD_FOCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            App::logMsg(sim_verbosity_msgs,IDSNS_IMPORTING_HEIGHTFIELD_SHAPE);
            App::currentWorld->sceneObjects->deselectObjects();
            std::string tst(App::folders->getCadFilesPath());
            std::string filenameAndPath=App::uiThread->getOpenFileName(App::mainWindow,0,IDS_IMPORTING_HEIGHTFIELD___,tst.c_str(),"",true,"Image, CSV and TXT files","tga","jpg","jpeg","png","gif","bmp","tiff","csv","txt");

            if (filenameAndPath.length()!=0)
            {
                if (VFile::doesFileExist(filenameAndPath.c_str()))
                {
                    if (heightfieldImportRoutine(filenameAndPath.c_str()))
                        App::logMsg(sim_verbosity_msgs,IDSNS_DONE);
                    else
                        App::logMsg(sim_verbosity_errors,IDSNS_AN_ERROR_OCCURRED_DURING_THE_IMPORT_OPERATION);
                    POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
                }
                else
                    App::logMsg(sim_verbosity_errors,IDSNS_ABORTED_FILE_DOES_NOT_EXIST);
            }
            else
                App::logMsg(sim_verbosity_msgs,IDSNS_ABORTED);
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
                for (size_t i=0;i<App::currentWorld->sceneObjects->getSelectionCount();i++)
                    sel.push_back(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
                if (!App::currentWorld->environment->getSceneLocked())
                {
                    App::logMsg(sim_verbosity_msgs,IDSNS_EXPORTING_SHAPES);
                    CSceneObjectOperations::addRootObjectChildrenToSelection(sel);
                    if (0==App::currentWorld->sceneObjects->getShapeCountInSelection(&sel))
                        return(true); // Selection contains nothing that can be exported!
                    std::string tst(App::folders->getCadFilesPath());
                    std::string filenameAndPath=App::uiThread->getSaveFileName(App::mainWindow,0,IDSNS_EXPORTING_SHAPES,tst.c_str(),"",false,"Mesh files","obj","ply","stl","dae");
                    if (filenameAndPath.length()!=0)
                    {
                        App::folders->setCadFilesPath(App::folders->getPathFromFull(filenameAndPath.c_str()).c_str());

                        CInterfaceStack stack;
                        stack.pushStringOntoStack(filenameAndPath.c_str(),0);
                        stack.pushTableOntoStack();
                        for (size_t i=0;i<sel.size();i++)
                        {
                            stack.pushNumberOntoStack(double(i+1)); // key or index
                            stack.pushNumberOntoStack(sel[i]);
                            stack.insertDataIntoStackTable();
                        }
                        App::worldContainer->sandboxScript->callScriptFunctionEx("simAssimp.exportShapesDlg",&stack);
                    }
                    else
                        App::logMsg(sim_verbosity_msgs,IDSNS_ABORTED);
                }
                else
                    App::uiThread->messageBox_warning(App::mainWindow,IDSN_EXPORT,IDS_SCENE_IS_LOCKED_WARNING,VMESSAGEBOX_OKELI,VMESSAGEBOX_REPLY_OK);
            }
            else
                App::uiThread->messageBox_critical(App::mainWindow,IDSN_EXPORT,"Assimp plugin was not found, cannot export",VMESSAGEBOX_OKELI,VMESSAGEBOX_REPLY_OK);
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
            for (size_t i=0;i<App::currentWorld->sceneObjects->getSelectionCount();i++)
                sel.push_back(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
            App::logMsg(sim_verbosity_msgs,IDSNS_EXPORTING_GRAPH_DATA);
            App::currentWorld->simulation->stopSimulation();
            if (App::currentWorld->sceneObjects->getGraphCountInSelection(&sel)!=0)
            {
                std::string tst(App::folders->getOtherFilesPath());
                std::string filenameAndPath=App::uiThread->getSaveFileName(App::mainWindow,0,IDS_SAVING_GRAPHS___,tst.c_str(),"",false,"CSV Files","csv");
                if (filenameAndPath.length()!=0)
                {
                    VFile myFile(filenameAndPath.c_str(),VFile::CREATE_WRITE|VFile::SHARE_EXCLUSIVE);
                    VArchive ar(&myFile,VArchive::STORE);
                    App::folders->setOtherFilesPath(App::folders->getPathFromFull(filenameAndPath.c_str()).c_str());
                    for (int i=0;i<int(sel.size());i++)
                    {
                        CGraph* it=App::currentWorld->sceneObjects->getGraphFromHandle(sel[i]);
                        if (it!=nullptr)
                            it->exportGraphData(ar);
                    }
                    ar.close();
                    myFile.close();
                    App::logMsg(sim_verbosity_msgs,IDSNS_DONE);
                }
                else
                    App::logMsg(sim_verbosity_msgs,IDSNS_ABORTED);
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
            for (size_t i=0;i<App::currentWorld->sceneObjects->getSelectionCount();i++)
                sel.push_back(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
            if (App::currentWorld->sceneObjects->isLastSelectionAPath(&sel))
            {
                CPath* it=(CPath*)App::currentWorld->sceneObjects->getLastSelectionObject(&sel);
                if (it->pathContainer->getSimplePathPointCount()!=0)
                {
                    if (cmd.cmdId==FILE_OPERATION_EXPORT_PATH_SIMPLE_POINTS_FOCMD)
                        App::logMsg(sim_verbosity_msgs,IDSNS_EXPORTING_PATH);
                    else
                        App::logMsg(sim_verbosity_msgs,IDSNS_EXPORTING_PATHS_BEZIER_CURVE);
                    App::currentWorld->simulation->stopSimulation();
                    std::string titleString;
                    if (cmd.cmdId==FILE_OPERATION_EXPORT_PATH_SIMPLE_POINTS_FOCMD)
                        titleString=IDS_EXPORTING_PATH___;
                    else
                        titleString=IDS_EXPORTING_PATH_BEZIER_CURVE___;
                    std::string filenameAndPath=App::uiThread->getSaveFileName(App::mainWindow,0,titleString.c_str(),App::folders->getExecutablePath().c_str(),"",false,"CSV Files","csv");
                    if (filenameAndPath.length()!=0)
                    {
                        _pathExportPoints(filenameAndPath.c_str(),it->getObjectHandle(),cmd.cmdId==FILE_OPERATION_EXPORT_PATH_BEZIER_POINTS_FOCMD,true);
                        App::logMsg(sim_verbosity_msgs,IDSNS_DONE);
                    }
                    else
                        App::logMsg(sim_verbosity_msgs,IDSNS_ABORTED);
                }
                else
                    App::logMsg(sim_verbosity_errors,IDSNS_CANNOT_EXPORT_AN_EMPTY_PATH);
            }
            else
                App::logMsg(sim_verbosity_errors,IDSNS_LAST_SELECTION_IS_NOT_A_PATH);
            App::currentWorld->sceneObjects->deselectObjects();
        }
        else
            App::appendSimulationThreadCommand(cmd); // We are in the UI thread. Execute the command via the main thread
        return(true);
    }
    if (cmd.cmdId==FILE_OPERATION_EXPORT_DYNAMIC_CONTENT_FOCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            if (!App::currentWorld->environment->getSceneLocked())
            {
                App::logMsg(sim_verbosity_msgs,IDSNS_EXPORTING_DYNAMIC_CONTENT);
                if (CPluginContainer::dyn_isDynamicContentAvailable()!=0)
                {
                    int eng=App::currentWorld->dynamicsContainer->getDynamicEngineType(nullptr);
                    if (eng==sim_physics_ode)
                    {
                        std::string tst(App::folders->getOtherFilesPath());
                        std::string filenameAndPath=App::uiThread->getSaveFileName(App::mainWindow,0,IDS_EXPORTING_DYNAMIC_CONTENT___,tst.c_str(),"",false,"ODE Dynamics World Files","ode");
                        if (filenameAndPath.length()!=0)
                        {
                            App::folders->setOtherFilesPath(App::folders->getPathFromFull(filenameAndPath.c_str()).c_str());
                            CPluginContainer::dyn_serializeDynamicContent(filenameAndPath.c_str(),0);
                            App::logMsg(sim_verbosity_msgs,IDSNS_DONE);
                        }
                        else
                            App::logMsg(sim_verbosity_msgs,IDSNS_ABORTED);
                    }
                    if (eng==sim_physics_bullet)
                    {
                        std::string tst(App::folders->getOtherFilesPath());
                        std::string filenameAndPath=App::uiThread->getSaveFileName(App::mainWindow,0,IDS_EXPORTING_DYNAMIC_CONTENT___,tst.c_str(),"",false,"Bullet Dynamics World Files","bullet");
                        if (filenameAndPath.length()!=0)
                        {
                            App::folders->setOtherFilesPath(App::folders->getPathFromFull(filenameAndPath.c_str()).c_str());
                            CPluginContainer::dyn_serializeDynamicContent(filenameAndPath.c_str(),App::userSettings->bulletSerializationBuffer);
                            App::logMsg(sim_verbosity_msgs,IDSNS_DONE);
                        }
                        else
                            App::logMsg(sim_verbosity_msgs,IDSNS_ABORTED);
                    }
                    if (eng==sim_physics_vortex)
                    {
                        std::string tst(App::folders->getOtherFilesPath());
                        std::string filenameAndPath=App::uiThread->getSaveFileName(App::mainWindow,0,IDS_EXPORTING_DYNAMIC_CONTENT___,tst.c_str(),"",false,"Vortex Dynamics World Files","vortex");
                        if (filenameAndPath.length()!=0)
                        {
                            App::folders->setOtherFilesPath(App::folders->getPathFromFull(filenameAndPath.c_str()).c_str());
                            CPluginContainer::dyn_serializeDynamicContent(filenameAndPath.c_str(),App::userSettings->bulletSerializationBuffer);
                            App::logMsg(sim_verbosity_msgs,IDSNS_DONE);
                        }
                        else
                            App::logMsg(sim_verbosity_msgs,IDSNS_ABORTED);
                    }
                    if (eng==sim_physics_newton)
                    {
                        std::string tst(App::folders->getOtherFilesPath());
                        std::string filenameAndPath=App::uiThread->getSaveFileName(App::mainWindow,0,IDS_EXPORTING_DYNAMIC_CONTENT___,tst.c_str(),"",false,"Newton Dynamics World Files","newton");
                        if (filenameAndPath.length()!=0)
                        {
                            App::folders->setOtherFilesPath(App::folders->getPathFromFull(filenameAndPath.c_str()).c_str());
                            CPluginContainer::dyn_serializeDynamicContent(filenameAndPath.c_str(),App::userSettings->bulletSerializationBuffer);
                            App::logMsg(sim_verbosity_msgs,IDSNS_DONE);
                        }
                        else
                            App::logMsg(sim_verbosity_msgs,IDSNS_ABORTED);
                    }
                }
                else
                    App::logMsg(sim_verbosity_errors,IDSNS_CANNOT_PROCEED_NO_DYNAMIC_CONTENT_AVAILABLE);
            }
            else
                App::uiThread->messageBox_warning(App::mainWindow,IDSN_EXPORT,IDS_SCENE_IS_LOCKED_WARNING,VMESSAGEBOX_OKELI,VMESSAGEBOX_REPLY_OK);
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

            ci=App::worldContainer->getInstanceIndexOfASceneNotYetSaved(App::currentWorld->environment->getSceneLocked());
            if (!App::currentWorld->simulation->isSimulationStopped())
                si=App::worldContainer->getCurrentWorldIndex();
            if (App::getEditModeType()!=NO_EDIT_MODE)
                ei=App::worldContainer->getCurrentWorldIndex();

            if (!App::currentWorld->environment->getSceneLocked())
            {
                if (App::currentWorld->undoBufferContainer->isSceneSaveMaybeNeededFlagSet())
                    ci=App::worldContainer->getCurrentWorldIndex();
            }
            if ((ei==App::worldContainer->getCurrentWorldIndex())&&(!displayed))
            {
                if (VMESSAGEBOX_REPLY_OK==App::uiThread->messageBox_warning(App::mainWindow,IDSN_EXIT,IDS_INSTANCE_STILL_IN_EDIT_MODE_MESSAGE,VMESSAGEBOX_OK_CANCEL,VMESSAGEBOX_REPLY_OK))
                {
                    if (App::mainWindow!=nullptr)
                        App::mainWindow->editModeContainer->processCommand(ANY_EDIT_MODE_FINISH_AND_CANCEL_CHANGES_EMCMD,nullptr);
                    ei=-1;
                }
                else
                    displayed=true;
            }
            if ((si==App::worldContainer->getCurrentWorldIndex())&&(!displayed))
            {
                if (VMESSAGEBOX_REPLY_OK==App::uiThread->messageBox_warning(App::mainWindow,IDSN_EXIT,IDS_SIMULATION_STILL_RUNNING_MESSAGE,VMESSAGEBOX_OK_CANCEL,VMESSAGEBOX_REPLY_OK))
                    App::worldContainer->simulatorMessageQueue->addCommand(sim_message_simulation_stop_request,0,0,0,0,nullptr,0);
                displayed=true;
            }
            if ((ci==App::worldContainer->getCurrentWorldIndex())&&(!displayed))
            {
                unsigned short action=VMESSAGEBOX_REPLY_NO;
                if (CSimFlavor::getBoolVal(16))
                    action=App::uiThread->messageBox_warning(App::mainWindow,IDSN_SAVE,IDS_WANNA_SAVE_THE_SCENE_WARNING,VMESSAGEBOX_YES_NO_CANCEL,VMESSAGEBOX_REPLY_NO);
                if (action==VMESSAGEBOX_REPLY_YES)
                {
                    if (_saveSceneWithDialogAndEverything()) // will call save as if needed!
                        action=VMESSAGEBOX_REPLY_NO;
                }
                if (action==VMESSAGEBOX_REPLY_NO)
                {
                    App::currentWorld->undoBufferContainer->clearSceneSaveMaybeNeededFlag();
                    ci=App::worldContainer->getInstanceIndexOfASceneNotYetSaved(false);
                }
                else
                    displayed=true;
            }
            if ((ei!=-1)&&(!displayed))
            {
                App::uiThread->messageBox_warning(App::mainWindow,IDSN_EXIT,IDS_ANOTHER_INSTANCE_STILL_IN_EDIT_MODE_MESSAGE,VMESSAGEBOX_OKELI,VMESSAGEBOX_REPLY_OK);
                App::worldContainer->switchToWorld(ei);
                displayed=true;
            }
            if ((si!=-1)&&(!displayed))
            {
                App::uiThread->messageBox_warning(App::mainWindow,IDSN_EXIT,IDS_ANOTHER_SIMULATION_STILL_RUNNING_MESSAGE,VMESSAGEBOX_OKELI,VMESSAGEBOX_REPLY_OK);
                App::worldContainer->switchToWorld(si);
                displayed=true;
            }
            if ((ci!=-1)&&(!displayed))
            {
                if (VMESSAGEBOX_REPLY_CANCEL==App::uiThread->messageBox_warning(App::mainWindow,IDSN_SAVE,IDS_ANOTHER_INSTANCE_STILL_NOT_SAVED_WANNA_LEAVE_ANYWAY_MESSAGE,VMESSAGEBOX_OK_CANCEL,VMESSAGEBOX_REPLY_OK))
                {
                    App::worldContainer->switchToWorld(ci);
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
            CFileOperations::loadModel(cmd.stringParams[0].c_str(),true,cmd.boolParams[0],false,cmd.boolParams[1],nullptr,false,false);
        }
        else
            App::appendSimulationThreadCommand(cmd); // We are in the UI thread. Execute the command via the main thread:
        return(true);
    }
    return(false);
}


void CFileOperations::createNewScene(bool displayMessages,bool forceForNewInstance)
{
    TRACE_INTERNAL;
    bool useNewInstance=false;
    useNewInstance=(App::currentWorld->undoBufferContainer->isSceneSaveMaybeNeededFlagSet()||(App::currentWorld->mainSettings->getScenePathAndName()!=""))&&(!App::currentWorld->environment->getSceneCanBeDiscardedWhenNewSceneOpened());
    if (forceForNewInstance)
        useNewInstance=true;
    CSimFlavor::run(2);
    App::setDefaultMouseMode();
    if (useNewInstance)
        App::worldContainer->createNewWorld();
    else
        App::currentWorld->simulation->stopSimulation();
    App::currentWorld->clearScene(true);
    std::string fullPathAndFilename=App::folders->getSystemPath()+"/";
    fullPathAndFilename+=CSimFlavor::getStringVal(16);
    loadScene(fullPathAndFilename.c_str(),false,false,false);
    App::currentWorld->mainSettings->setScenePathAndName("");//savedLoc;
    App::currentWorld->environment->generateNewUniquePersistentIdString();
    if (displayMessages)
        App::logMsg(sim_verbosity_msgs,IDSNS_DEFAULT_SCENE_WAS_SET_UP);
    App::currentWorld->undoBufferContainer->memorizeState(); // so that we can come back to the initial state!
    App::currentWorld->undoBufferContainer->clearSceneSaveMaybeNeededFlag();
}

void CFileOperations::closeScene(bool displayMessages,bool displayDialogs)
{
    TRACE_INTERNAL;
    unsigned short action=VMESSAGEBOX_REPLY_NO;
#ifdef SIM_WITH_GUI
    if (displayMessages&&(!App::currentWorld->environment->getSceneCanBeDiscardedWhenNewSceneOpened()))
    {
        if (CSimFlavor::getBoolVal(16)&&(!App::currentWorld->environment->getSceneLocked()))
        {
            if (displayDialogs&&App::currentWorld->undoBufferContainer->isSceneSaveMaybeNeededFlagSet())
            {
                action=App::uiThread->messageBox_warning(App::mainWindow,IDSN_SAVE,IDS_WANNA_SAVE_THE_SCENE_WARNING,VMESSAGEBOX_YES_NO_CANCEL,VMESSAGEBOX_REPLY_NO);
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
        App::currentWorld->simulation->stopSimulation();
#ifdef SIM_WITH_GUI
        App::setDefaultMouseMode();
        if (App::mainWindow!=nullptr)
            App::mainWindow->editModeContainer->processCommand(ANY_EDIT_MODE_FINISH_AND_CANCEL_CHANGES_EMCMD,nullptr);
#endif
        if (displayDialogs)
            App::uiThread->showOrHideProgressBar(true,-1,"Closing scene...");
        App::currentWorld->clearScene(true);
        if (displayDialogs)
            App::uiThread->showOrHideProgressBar(false);
        if (App::worldContainer->getWorldCount()>1)
        { // remove this instance:
            App::worldContainer->destroyCurrentWorld();
        }
        else
        { // simply set-up an empty (default) scene
            std::string savedLoc=App::currentWorld->mainSettings->getScenePathAndName();
            std::string fullPathAndFilename=App::folders->getSystemPath()+"/";
            fullPathAndFilename+="dfltscn.";
            fullPathAndFilename+=SIM_SCENE_EXTENSION;
            loadScene(fullPathAndFilename.c_str(),false,false,false);
            App::currentWorld->mainSettings->setScenePathAndName(""); //savedLoc.c_str());
            App::currentWorld->environment->generateNewUniquePersistentIdString();
            App::logMsg(sim_verbosity_msgs,IDSNS_DEFAULT_SCENE_WAS_SET_UP);
            App::currentWorld->undoBufferContainer->memorizeState(); // so that we can come back to the initial state!
            App::currentWorld->undoBufferContainer->clearSceneSaveMaybeNeededFlag();
        }
    }
    App::setRebuildHierarchyFlag();
}

bool CFileOperations::_pathImportRoutine(const char* pathName,bool displayDialogs)
{ // Should only be called by the NON-UI thread
    bool retVal=false;
    if (VFile::doesFileExist(pathName))
    {
        if (displayDialogs)
            App::uiThread->showOrHideProgressBar(true,-1,"Importing path...");
        try
        {
            VFile file(pathName,VFile::READ|VFile::SHARE_DENY_NONE);
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
                    if (tt::getValidFloat(word.c_str(),val))
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
                newObject->setObjectName(IDSOGL_IMPORTEDPATH,true);
                newObject->setObjectAltName(tt::getObjectAltNameFromObjectName(newObject->getObjectName().c_str()).c_str(),true);

                App::currentWorld->sceneObjects->addObjectToScene(newObject,false,true);
                App::currentWorld->sceneObjects->selectObject(newObject->getObjectHandle());
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

bool CFileOperations::_pathExportPoints(const char* pathName,int pathID,bool bezierPoints,bool displayDialogs)
{
    CPath* pathObject=App::currentWorld->sceneObjects->getPathFromHandle(pathID);
    if (pathObject==nullptr)
        return(false);
    bool retVal=false;
    if (displayDialogs)
        App::uiThread->showOrHideProgressBar(true,-1,"Exporting path points...");
    try
    {
        VFile myFile(pathName,VFile::CREATE_WRITE|VFile::SHARE_EXCLUSIVE);
        VArchive ar(&myFile,VArchive::STORE);

        CPathCont* it=pathObject->pathContainer;
        C7Vector pathTr(pathObject->getFullCumulativeTransformation());
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
    TRACE_INTERNAL;
    if (App::isFullScreen()||App::userSettings->doNotShowAcknowledgmentMessages||(App::getDlgVerbosity()<sim_verbosity_infos))
        displayDialogs=false;

    if (strlen(pathAndFilename)==0)
    {
        createNewScene(displayMessages,true);
        return(true);
    }

    App::setDefaultMouseMode();

    int result=-3;
    CSimFlavor::run(2);
    App::currentWorld->sceneObjects->deselectObjects();
    App::currentWorld->simulation->stopSimulation(); // should be anyway stopped!
    if (VFile::doesFileExist(pathAndFilename))
    {
        App::currentWorld->clearScene(true);

        App::currentWorld->mainSettings->setScenePathAndName(pathAndFilename);
        if (setCurrentDir)
            App::folders->setScenesPath(App::currentWorld->mainSettings->getScenePath().c_str());
        if (CSimFlavor::getBoolVal_str(1,App::currentWorld->mainSettings->getScenePathAndName().c_str()))
            App::currentWorld->mainSettings->setScenePathAndName("");

        if (displayDialogs)
            App::uiThread->showOrHideProgressBar(true,-1,"Opening scene...");

        CSer* serObj;
        int serializationVersion;
        unsigned short csimVersionThatWroteThis;
        int licenseTypeThatWroteThis;
        char revisionNumber;
        if ( (CSer::getFileTypeFromName(pathAndFilename)==CSer::filetype_csim_xml_simplescene_file) )
        {
            serObj=new CSer(pathAndFilename,CSer::getFileTypeFromName(pathAndFilename));
            result=serObj->readOpenXml(serializationVersion,csimVersionThatWroteThis,licenseTypeThatWroteThis,revisionNumber,false);
            if (serObj->getFileType()==CSer::filetype_csim_xml_simplescene_file) // final file type is set in readOpenXml (whether exhaustive or simple scene)
                App::currentWorld->mainSettings->setScenePathAndName(""); // since lossy format
        }
        else
        {
            serObj=new CSer(pathAndFilename,CSer::getFileTypeFromName(pathAndFilename));
            result=serObj->readOpenBinary(serializationVersion,csimVersionThatWroteThis,licenseTypeThatWroteThis,revisionNumber,false);
        }

        std::string infoPrintOut(tt::decorateString("",IDSNS_LOADING_SCENE," ("));
        infoPrintOut+=std::string(pathAndFilename)+"). ";
#ifdef SIM_WITH_GUI
        if ((result==-3)&&(App::mainWindow!=nullptr))
        {
            if (displayMessages)
                App::logMsg(sim_verbosity_errors,"The file does not seem to be a valid scene file.");
            if (displayDialogs)
            {
                App::uiThread->showOrHideProgressBar(false);
                App::uiThread->messageBox_critical(App::mainWindow,IDSN_SERIALIZATION,"The file does not seem to be a valid scene file.",VMESSAGEBOX_OKELI,VMESSAGEBOX_REPLY_OK);
                App::uiThread->showOrHideProgressBar(true);
            }
        }
        if ((result!=-3)&&displayMessages&&(App::mainWindow!=nullptr))
        {
            infoPrintOut+=" ";
            infoPrintOut+=IDSNS_SERIALIZATION_VERSION_IS;
            infoPrintOut+=" ";
            infoPrintOut+=boost::lexical_cast<std::string>(serializationVersion)+".";
            App::logMsg(sim_verbosity_msgs,infoPrintOut.c_str());
            infoPrintOut=_getStringOfVersionAndLicenseThatTheFileWasWrittenWith(csimVersionThatWroteThis,licenseTypeThatWroteThis,revisionNumber);
            if (infoPrintOut!="")
                App::logMsg(sim_verbosity_msgs,infoPrintOut.c_str());
        }
        if ((result==-2)&&(App::mainWindow!=nullptr))
        {
            if (displayMessages)
                App::logMsg(sim_verbosity_errors,IDS_SERIALIZATION_VERSION_NOT_SUPPORTED_ANYMORE);
            if (displayDialogs)
            {
                App::uiThread->showOrHideProgressBar(false);
                App::uiThread->messageBox_critical(App::mainWindow,IDSN_SERIALIZATION,IDS_SERIALIZATION_VERSION_NOT_SUPPORTED_ANYMORE,VMESSAGEBOX_OKELI,VMESSAGEBOX_REPLY_OK);
                App::uiThread->showOrHideProgressBar(true);
            }
        }
        if ((result==-1)&&(App::mainWindow!=nullptr))
        {
            if (displayMessages)
                App::logMsg(sim_verbosity_errors,IDS_SERIALIZATION_VERSION_TOO_RECENT);
            if (displayDialogs)
            {
                App::uiThread->showOrHideProgressBar(false);
                App::uiThread->messageBox_critical(App::mainWindow,IDSN_SERIALIZATION,IDS_SERIALIZATION_VERSION_TOO_RECENT,VMESSAGEBOX_OKELI,VMESSAGEBOX_REPLY_OK);
                App::uiThread->showOrHideProgressBar(true);
            }
        }
        if ((result==0)&&(App::mainWindow!=nullptr))
        {
            if (displayMessages)
                App::logMsg(sim_verbosity_errors,IDS_COMPRESSION_SCHEME_NOT_SUPPORTED);
            if (displayDialogs)
            {
                App::uiThread->showOrHideProgressBar(false);
                App::uiThread->messageBox_critical(App::mainWindow,IDSN_SERIALIZATION,IDS_COMPRESSION_SCHEME_NOT_SUPPORTED,VMESSAGEBOX_OKELI,VMESSAGEBOX_REPLY_OK);
                App::uiThread->showOrHideProgressBar(true);
            }
        }
#endif
        if (result==1)
        {
            App::currentWorld->loadScene(serObj[0],false);
            serObj->readClose();
#ifdef SIM_WITH_GUI
            if (App::mainWindow!=nullptr)
                App::mainWindow->refreshDimensions(); // this is important so that the new pages and views are set to the correct dimensions
            if (displayMessages)
                App::logMsg(sim_verbosity_msgs,IDSNS_SCENE_OPENED);
            if ((csimVersionThatWroteThis>SIM_PROGRAM_VERSION_NB)&&displayDialogs&&(App::mainWindow!=nullptr))
                App::uiThread->messageBox_warning(App::mainWindow,IDSN_SCENE,IDS_SAVED_WITH_MORE_RECENT_VERSION_WARNING,VMESSAGEBOX_OKELI,VMESSAGEBOX_REPLY_OK);
            if (displayMessages)
            {
                std::string acknowledgement(App::currentWorld->environment->getAcknowledgement());
                tt::removeSpacesAtBeginningAndEnd(acknowledgement);
                if (acknowledgement.length()!=0)
                {
                    acknowledgement="Scene content acknowledgement / infos:\n"+acknowledgement;
                    App::logMsg(sim_verbosity_scriptinfos,acknowledgement.c_str());
                }
            }
#endif
        }
        else
        {
            if (displayMessages)
                App::logMsg(sim_verbosity_errors,IDSNS_SCENE_COULD_NOT_BE_OPENED);
        }
        delete serObj;
        if (displayDialogs)
            App::uiThread->showOrHideProgressBar(false);
        App::currentWorld->undoBufferContainer->memorizeState(); // so that we can come back to the initial state!
    }
    else
    {
        if (displayMessages)
            App::logMsg(sim_verbosity_errors,IDSNS_ABORTED_FILE_DOES_NOT_EXIST);
    }
    App::setRebuildHierarchyFlag();
    return(result==1);
}

bool CFileOperations::loadModel(const char* pathAndFilename,bool displayMessages,bool displayDialogs,bool setCurrentDir,bool doUndoThingInHere,std::vector<char>* loadBuffer,bool onlyThumbnail,bool forceModelAsCopy)
{
    TRACE_INTERNAL;
    if (App::isFullScreen()||App::userSettings->doNotShowAcknowledgmentMessages||(App::getDlgVerbosity()<sim_verbosity_infos))
        displayDialogs=false;
    int result=-3;
    CSimFlavor::run(2);
    if ((pathAndFilename==nullptr)||VFile::doesFileExist(pathAndFilename))
    {
        App::currentWorld->sceneObjects->deselectObjects();

        if (setCurrentDir&&(pathAndFilename!=nullptr))
            App::folders->setModelsPath(App::folders->getPathFromFull(pathAndFilename).c_str());

        if (displayDialogs)
            App::uiThread->showOrHideProgressBar(true,-1,"Loading model...");

        if (pathAndFilename!=nullptr)
        { // loading from file...
            CSer* serObj;
            int serializationVersion;
            unsigned short csimVersionThatWroteThis;
            int licenseTypeThatWroteThis;
            char revisionNumber;

            if (CSer::getFileTypeFromName(pathAndFilename)==CSer::filetype_csim_xml_simplemodel_file)
            {
                serObj=new CSer(pathAndFilename,CSer::getFileTypeFromName(pathAndFilename));
                result=serObj->readOpenXml(serializationVersion,csimVersionThatWroteThis,licenseTypeThatWroteThis,revisionNumber,false);
            }
            else
            {
                serObj=new CSer(pathAndFilename,CSer::getFileTypeFromName(pathAndFilename));
                result=serObj->readOpenBinary(serializationVersion,csimVersionThatWroteThis,licenseTypeThatWroteThis,revisionNumber,false);
            }

            std::string infoPrintOut(tt::decorateString("",IDSNS_LOADING_MODEL," ("));
            infoPrintOut+=std::string(pathAndFilename)+"). ";
    #ifdef SIM_WITH_GUI
            if ((result==-3)&&(App::mainWindow!=nullptr))
            {
                if (displayMessages)
                    App::logMsg(sim_verbosity_errors,"The file does not seem to be a valid model file.");
                if (displayDialogs)
                {
                    App::uiThread->showOrHideProgressBar(false);
                    App::uiThread->messageBox_critical(App::mainWindow,IDSN_SERIALIZATION,"The file does not seem to be a valid model file.",VMESSAGEBOX_OKELI,VMESSAGEBOX_REPLY_OK);
                    App::uiThread->showOrHideProgressBar(true);
                }
            }
            if (((result!=-3)&&displayMessages)&&(App::mainWindow!=nullptr))
            {
                infoPrintOut+=" ";
                infoPrintOut+=IDSNS_SERIALIZATION_VERSION_IS;
                infoPrintOut+=" ";
                infoPrintOut+=boost::lexical_cast<std::string>(serializationVersion)+".";
                App::logMsg(sim_verbosity_msgs,infoPrintOut.c_str());
                infoPrintOut=_getStringOfVersionAndLicenseThatTheFileWasWrittenWith(csimVersionThatWroteThis,licenseTypeThatWroteThis,revisionNumber);
                if (infoPrintOut!="")
                    App::logMsg(sim_verbosity_msgs,infoPrintOut.c_str());
            }
            if ((result==-2)&&(App::mainWindow!=nullptr))
            {
                if (displayMessages)
                    App::logMsg(sim_verbosity_errors,IDS_SERIALIZATION_VERSION_NOT_SUPPORTED_ANYMORE);
                if (displayDialogs)
                {
                    App::uiThread->showOrHideProgressBar(false);
                    App::uiThread->messageBox_critical(App::mainWindow,IDSN_SERIALIZATION,IDS_SERIALIZATION_VERSION_NOT_SUPPORTED_ANYMORE,VMESSAGEBOX_OKELI,VMESSAGEBOX_REPLY_OK);
                    App::uiThread->showOrHideProgressBar(true);
                }
            }
            if ((result==-1)&&(App::mainWindow!=nullptr))
            {
                if (displayMessages)
                    App::logMsg(sim_verbosity_errors,IDS_SERIALIZATION_VERSION_TOO_RECENT);
                if (displayDialogs)
                {
                    App::uiThread->showOrHideProgressBar(false);
                    App::uiThread->messageBox_critical(App::mainWindow,IDSN_SERIALIZATION,IDS_SERIALIZATION_VERSION_TOO_RECENT,VMESSAGEBOX_OKELI,VMESSAGEBOX_REPLY_OK);
                    App::uiThread->showOrHideProgressBar(true);
                }
            }
            if ((result==0)&&(App::mainWindow!=nullptr))
            {
                if (displayMessages)
                    App::logMsg(sim_verbosity_errors,IDS_COMPRESSION_SCHEME_NOT_SUPPORTED);
                if (displayDialogs)
                {
                    App::uiThread->showOrHideProgressBar(false);
                    App::uiThread->messageBox_critical(App::mainWindow,IDSN_SERIALIZATION,IDS_COMPRESSION_SCHEME_NOT_SUPPORTED,VMESSAGEBOX_OKELI,VMESSAGEBOX_REPLY_OK);
                    App::uiThread->showOrHideProgressBar(true);
                }
            }
    #endif
            if (result==1)
            {
                App::currentWorld->loadModel(serObj[0],onlyThumbnail,forceModelAsCopy,nullptr,nullptr,nullptr);
                serObj->readClose();
                if (displayMessages&&(!onlyThumbnail))
                    App::logMsg(sim_verbosity_msgs,IDSNS_MODEL_LOADED);
    #ifdef SIM_WITH_GUI
                if ((csimVersionThatWroteThis>SIM_PROGRAM_VERSION_NB)&&displayDialogs&&(App::mainWindow!=nullptr)&&(!onlyThumbnail))
                {
                    App::uiThread->showOrHideProgressBar(false);
                    App::uiThread->messageBox_warning(App::mainWindow,IDSN_MODEL,IDS_MODEL_SAVED_WITH_MORE_RECENT_VERSION_WARNING,VMESSAGEBOX_OKELI,VMESSAGEBOX_REPLY_OK);
                    App::uiThread->showOrHideProgressBar(true);
                }
    #endif
                if (!onlyThumbnail)
                {
                    std::string acknowledgement;
                    // now we search for the model base that contains the acknowledgment:
                    std::vector<CSceneObject*> loadedObjects;
                    App::currentWorld->sceneObjects->getSelectedObjects(loadedObjects);
                    for (size_t obba=0;obba<loadedObjects.size();obba++)
                    {
                        if (loadedObjects[obba]->getParent()==nullptr)
                        {
                            acknowledgement=loadedObjects[obba]->getModelAcknowledgement();
                            tt::removeSpacesAtBeginningAndEnd(acknowledgement);
                            break;
                        }
                    }
                    if (displayMessages&&(acknowledgement.length()!=0))
                    {
                        acknowledgement="Model acknowledgement / infos:\n"+acknowledgement;
                        App::logMsg(sim_verbosity_scriptinfos,acknowledgement.c_str());
                    }
                }
            }
            else
            {
                if (displayMessages)
                    App::logMsg(sim_verbosity_errors,IDSNS_MODEL_COULD_NOT_BE_LOADED);
            }
            delete serObj;
        }
        else
        { // loading from buffer...
            CSer serObj(loadBuffer[0],CSer::filetype_csim_bin_model_buff);
            int serializationVersion;
            unsigned short csimVersionThatWroteThis;
            int licenseTypeThatWroteThis;
            char revisionNumber;
            result=serObj.readOpenBinary(serializationVersion,csimVersionThatWroteThis,licenseTypeThatWroteThis,revisionNumber,false);
            if (result==1)
            {
                App::currentWorld->loadModel(serObj,onlyThumbnail,forceModelAsCopy,nullptr,nullptr,nullptr);
                serObj.readClose();
            }
        }

        if (displayDialogs)
            App::uiThread->showOrHideProgressBar(false);

        App::currentWorld->sceneObjects->removeFromSelectionAllExceptModelBase(false);
        App::setRebuildHierarchyFlag();
        if (doUndoThingInHere)
        {
            POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
        }
    }
    else
    {
        if (displayMessages)
            App::logMsg(sim_verbosity_errors,IDSNS_ABORTED_FILE_DOES_NOT_EXIST);
    }
    return(result==1);
}

bool CFileOperations::saveScene(const char* pathAndFilename,bool displayMessages,bool displayDialogs,bool setCurrentDir,bool changeSceneUniqueId)
{ // There is a similar routine in CUndoBuffer!!
    bool retVal=false;
    if (CSimFlavor::getBoolVal(16))
    {
        if (App::isFullScreen())
            displayDialogs=false;

#ifdef SIM_WITH_GUI
        if (App::mainWindow!=nullptr)
            App::mainWindow->codeEditorContainer->saveOrCopyOperationAboutToHappen();
#endif

        void* returnVal=CPluginContainer::sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_scenesave,nullptr,nullptr,nullptr);
        delete[] (char*)returnVal;

        std::string _pathAndFilename(pathAndFilename);
        size_t simpleXmlPos=_pathAndFilename.find("@simpleXml");
        bool simpleXml=(simpleXmlPos!=std::string::npos);
        if (simpleXml)
            _pathAndFilename.erase(_pathAndFilename.begin()+simpleXmlPos,_pathAndFilename.end());
        CSer* serObj;
        if ( (CSer::getFileTypeFromName(_pathAndFilename.c_str())==CSer::filetype_csim_xml_simplescene_file) )
        {
            VFile::eraseFilesWithPrefix(VVarious::splitPath_path(_pathAndFilename.c_str()).c_str(),(VVarious::splitPath_fileBase(_pathAndFilename.c_str())+"_").c_str());
            if (!simpleXml)
                serObj=new CSer(_pathAndFilename.c_str(),CSer::filetype_csim_xml_xscene_file);
            else
                serObj=new CSer(_pathAndFilename.c_str(),CSer::filetype_csim_xml_simplescene_file);
            retVal=serObj->writeOpenXml(App::userSettings->xmlExportSplitSize,App::userSettings->xmlExportKnownFormats);
        }
        else
        {
            serObj=new CSer(_pathAndFilename.c_str(),CSer::getFileTypeFromName(_pathAndFilename.c_str()));
            retVal=serObj->writeOpenBinary(App::userSettings->compressFiles);
        }

        if (retVal)
        {
            if (displayDialogs)
                App::uiThread->showOrHideProgressBar(true,-1,"Saving scene...");
            if (!simpleXml) // because lossy
                App::currentWorld->mainSettings->setScenePathAndName(_pathAndFilename.c_str());

            App::currentWorld->embeddedScriptContainer->sceneOrModelAboutToBeSaved(-1);

            if (changeSceneUniqueId)
                App::currentWorld->environment->generateNewUniquePersistentIdString();

            if (setCurrentDir)
                App::folders->setScenesPath(App::currentWorld->mainSettings->getScenePath().c_str());

            std::string infoPrintOut(IDSN_SAVING_SCENE);
            infoPrintOut+=" (";
            infoPrintOut+=std::string(_pathAndFilename.c_str())+"). ";
            infoPrintOut+=IDSNS_SERIALIZATION_VERSION_IS;
            infoPrintOut+=" ";
            if ( (serObj->getFileType()==CSer::filetype_csim_xml_xscene_file)||(serObj->getFileType()==CSer::filetype_csim_xml_simplescene_file) )
                infoPrintOut+=boost::lexical_cast<std::string>(CSer::XML_XSERIALIZATION_VERSION)+".";
            else
                infoPrintOut+=boost::lexical_cast<std::string>(CSer::SER_SERIALIZATION_VERSION)+".";
            if (displayMessages)
                App::logMsg(sim_verbosity_msgs,infoPrintOut.c_str());

            App::currentWorld->saveScene(serObj[0]);
            serObj->writeClose();

            if (displayMessages)
                App::logMsg(sim_verbosity_msgs,IDSNS_SCENE_WAS_SAVED);

            if (displayDialogs)
                App::uiThread->showOrHideProgressBar(false);
            App::setRebuildHierarchyFlag(); // we might have saved under a different name, we need to reflect it
        }
        else
        {
            #ifdef SIM_WITH_GUI
                if ((App::mainWindow!=nullptr)&&displayDialogs)
                { // to avoid an error when saving a file that was opened while still attached to an email for instance
                    App::uiThread->messageBox_critical(App::mainWindow,IDSN_FILE_ACCESS,IDSN_ACCESS_TO_FILE_WAS_DENIED,VMESSAGEBOX_OKELI,VMESSAGEBOX_REPLY_OK);
                }
            #endif
        }
        delete serObj;
    }
    return(retVal);
}

bool CFileOperations::saveModel(int modelBaseDummyID,const char* pathAndFilename,bool displayMessages,bool displayDialogs,bool setCurrentDir,std::vector<char>* saveBuffer/*=nullptr*/)
{
    if ( CSimFlavor::getBoolVal(16)||(saveBuffer!=nullptr) )
    {
        App::currentWorld->embeddedScriptContainer->sceneOrModelAboutToBeSaved(modelBaseDummyID);
        if (App::isFullScreen())
            displayDialogs=false;
        std::vector<int> sel;
        sel.push_back(modelBaseDummyID);

        CSceneObject* modelBaseObject=App::currentWorld->sceneObjects->getObjectFromHandle(modelBaseDummyID);
        C3Vector minV,maxV;
        bool b=true;
        C7Vector modelTr(modelBaseObject->getCumulativeTransformation());
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
                    App::folders->setModelsPath(App::folders->getPathFromFull(pathAndFilename).c_str());
                infoPrintOut+=" (";
                infoPrintOut+=std::string(pathAndFilename)+"). ";
            }

            if (pathAndFilename!=nullptr)
            { // saving to file...
                CSer* serObj;
                if (CSer::getFileTypeFromName(pathAndFilename)==CSer::filetype_csim_xml_simplemodel_file)
                {
                    VFile::eraseFilesWithPrefix(VVarious::splitPath_path(pathAndFilename).c_str(),(VVarious::splitPath_fileBase(pathAndFilename)+"_").c_str());
                    serObj=new CSer(pathAndFilename,CSer::filetype_csim_xml_xmodel_file); // we can only save exhaustive models
                    serObj->writeOpenXml(App::userSettings->xmlExportSplitSize,App::userSettings->xmlExportKnownFormats);
                }
                else
                {
                    serObj=new CSer(pathAndFilename,CSer::getFileTypeFromName(pathAndFilename));
                    serObj->writeOpenBinary(App::userSettings->compressFiles);
                }
                App::worldContainer->copyBuffer->serializeCurrentSelection(serObj[0],&sel,modelTr,modelBBSize,modelNonDefaultTranslationStepSize);
                serObj->writeClose();
                delete serObj;
            }
            else
            { // saving to buffer...
                CSer serObj(saveBuffer[0],CSer::filetype_csim_bin_model_buff);

                serObj.writeOpenBinary(App::userSettings->compressFiles);
                App::worldContainer->copyBuffer->serializeCurrentSelection(serObj,&sel,modelTr,modelBBSize,modelNonDefaultTranslationStepSize);
                serObj.writeClose();
            }
            infoPrintOut+=IDSNS_SERIALIZATION_VERSION_IS;
            infoPrintOut+=" ";
            infoPrintOut+=boost::lexical_cast<std::string>(CSer::SER_SERIALIZATION_VERSION)+".";

            if (displayMessages)
                App::logMsg(sim_verbosity_msgs,infoPrintOut.c_str());
            if (displayMessages)
                App::logMsg(sim_verbosity_msgs,IDSNS_MODEL_WAS_SAVED);
            return(true);
        }
    }
    return(false);
}

std::string CFileOperations::_getStringOfVersionAndLicenseThatTheFileWasWrittenWith(unsigned short coppeliaSimVer,int licenseType,char revision)
{
    if (coppeliaSimVer==0)
        return("");
    std::string retStr;
    retStr=tt::decorateString("",IDSNS_FILE_WAS_PREVIOUSLY_WRITTEN_WITH_CSIM_VERSION," ");
    int v=coppeliaSimVer;
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
        if (licenseType==0x00001000)
            retStr+=" (CoppeliaSim Edu license)";
        if (licenseType==0x00002000)
            retStr+=" (CoppeliaSim Pro license)";
        if (licenseType==0x00005000)
            retStr+=" (CoppeliaSim Player license)";
        if (licenseType==0x00006000)
            retStr+=" (custom compilation)";
        if (licenseType==0x00007000)
            retStr+=" (XReality license)";
    }
    return(retStr);
}

bool CFileOperations::heightfieldImportRoutine(const char* pathName)
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
                unsigned char* data=CImageLoaderSaver::load(pathName,&resX,&resY,&n,3);
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
                        App::uiThread->messageBox_critical(App::mainWindow,IDSN_IMPORT,IDS_TEXTURE_FILE_COULD_NOT_BE_LOADED,VMESSAGEBOX_OKELI,VMESSAGEBOX_REPLY_OK);
                    #endif
                }
            }
            else
            { // from csv or txt file:
                VFile file(pathName,VFile::READ|VFile::SHARE_DENY_NONE);
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
                        if (tt::getValidFloat(word.c_str(),val))
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
                App::currentWorld->sceneObjects->deselectObjects();
                App::currentWorld->sceneObjects->addObjectToSelection(shapeHandle);

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
    CShape* shape=new CShape(allHeights,xSize,ySize,pointSpacing,maxHeight-minHeight);

    if (options&4)
        shape->getSingleMesh()->setPurePrimitiveType(sim_pure_primitive_none,1.0f,1.0f,1.0f);

    App::currentWorld->sceneObjects->addObjectToScene(shape,false,true);
    shape->setCulling((options&1)!=0);
    shape->setVisibleEdges((options&2)!=0);
    shape->getSingleMesh()->setGouraudShadingAngle(shadingAngle);
    shape->getSingleMesh()->setEdgeThresholdAngle(shadingAngle);
    shape->getSingleMesh()->color.setColor(0.68f,0.56f,0.36f,sim_colorcomponent_ambient_diffuse);
    shape->getSingleMesh()->color.setColor(0.25f,0.25f,0.25f,sim_colorcomponent_specular);
    std::string tempName(IDSOGL_HEIGHTFIELD);
    while (App::currentWorld->sceneObjects->getObjectFromName(tempName.c_str())!=nullptr)
        tempName=tt::generateNewName_noHash(tempName.c_str());
    shape->setObjectName(tempName.c_str(),true);
    //App::currentWorld->sceneObjects->renameObject(shape->getObjectHandle(),tempName.c_str());
    tempName=tt::getObjectAltNameFromObjectName(IDSOGL_HEIGHTFIELD);
    while (App::currentWorld->sceneObjects->getObjectFromAltName(tempName.c_str())!=nullptr)
        tempName=tt::generateNewName_noHash(tempName.c_str());
    shape->setObjectAltName(tempName.c_str(),true);
    //App::currentWorld->sceneObjects->altRenameObject(shape->getObjectHandle(),tempName.c_str());

    shape->alignBoundingBoxWithWorld();

    int propToRemove=sim_objectspecialproperty_collidable|sim_objectspecialproperty_measurable;
    shape->setLocalObjectSpecialProperty((shape->getLocalObjectSpecialProperty()|propToRemove)-propToRemove);
    shape->setRespondable((options&8)==0);
    shape->setShapeIsDynamicallyStatic(true);

    return(shape->getObjectHandle());
}

void CFileOperations::addToRecentlyOpenedScenes(std::string filenameAndPath)
{
    CPersistentDataContainer cont(SIM_FILENAME_OF_USER_SETTINGS_IN_BINARY_FILE);
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
    CPersistentDataContainer cont(SIM_FILENAME_OF_USER_SETTINGS_IN_BINARY_FILE);
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
    App::currentWorld->exportIkContent(ar);
    bool retVal=true;
    if (displayDialogs)
        App::uiThread->showOrHideProgressBar(true,-1,"Exporting kinematics content...");
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
    bool fileOpOk=(App::currentWorld->simulation->isSimulationStopped())&&(App::getEditModeType()==NO_EDIT_MODE);
    bool simStoppedOrPausedNoEditMode=App::currentWorld->simulation->isSimulationStopped()||App::currentWorld->simulation->isSimulationPaused();
    bool fileOpOkAlsoDuringSimulation=(App::getEditModeType()==NO_EDIT_MODE);
    size_t selItems=App::currentWorld->sceneObjects->getSelectionCount();
    bool justModelSelected=false;
    if (selItems==1)
    {
        CSceneObject* obj=App::currentWorld->sceneObjects->getObjectFromHandle(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(0));
        justModelSelected=(obj!=nullptr)&&(obj->getModelBase());
    }
    std::vector<int> sel;
    for (size_t i=0;i<App::currentWorld->sceneObjects->getSelectionCount();i++)
        sel.push_back(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
    CSceneObjectOperations::addRootObjectChildrenToSelection(sel);
    int shapeNumber=App::currentWorld->sceneObjects->getShapeCountInSelection(&sel);
    int pathNumber=App::currentWorld->sceneObjects->getPathCountInSelection(&sel);
    int graphNumber=App::currentWorld->sceneObjects->getGraphCountInSelection(&sel);

    menu->appendMenuItem(fileOpOk,false,FILE_OPERATION_NEW_SCENE_FOCMD,IDS_NEW_SCENE_MENU_ITEM);

    menu->appendMenuItem(fileOpOk,false,FILE_OPERATION_OPEN_SCENE_FOCMD,IDS_OPEN_SCENE___MENU_ITEM);

    // recent scene files:
    CPersistentDataContainer cont(SIM_FILENAME_OF_USER_SETTINGS_IN_BINARY_FILE);
    std::string recentScenes[10];
    int recentScenesCnt=0;
    for (int i=0;i<10;i++)
    {
        std::string tmp("SIMSETTINGS_RECENTSCENE0");
        tmp[23]=48+i;
        cont.readData(tmp.c_str(),recentScenes[i]);
        if (recentScenes[i].length()>3)
            recentScenesCnt++;
    }
    VMenu* recentSceneMenu=new VMenu();
    for (int i=0;i<10;i++)
    {
        if (recentScenes[i].length()>3)
            recentSceneMenu->appendMenuItem(fileOpOk,false,FILE_OPERATION_OPEN_RECENT_SCENE0_FOCMD+i,VVarious::splitPath_fileBaseAndExtension(recentScenes[i].c_str()).c_str());
    }
    menu->appendMenuAndDetach(recentSceneMenu,(recentScenesCnt>0)&&fileOpOk,IDS_OPEN_RECENT_SCENE_MENU_ITEM);

    menu->appendMenuItem(fileOpOkAlsoDuringSimulation,false,FILE_OPERATION_LOAD_MODEL_FOCMD,IDS_LOAD_MODEL___MENU_ITEM);

    menu->appendMenuSeparator();

    menu->appendMenuItem(fileOpOk,false,FILE_OPERATION_CLOSE_SCENE_FOCMD,IDS_CLOSE_SCENE_MENU_ITEM);

    if ( (CSimFlavor::getIntVal(2)==-1)||(CSimFlavor::getIntVal(2)==1) )
    {
        menu->appendMenuSeparator();
        menu->appendMenuItem(fileOpOk,false,FILE_OPERATION_SAVE_SCENE_FOCMD,IDS_SAVE_SCENE_MENU_ITEM);
        VMenu* saveSceneMenu=new VMenu();
        saveSceneMenu->appendMenuItem(fileOpOk,false,FILE_OPERATION_SAVE_SCENE_AS_CSIM_FOCMD,IDS_SCENE_AS_CSIM___MENU_ITEM);
        VMenu* saveSceneAsXmlMenu=new VMenu();
        saveSceneAsXmlMenu->appendMenuItem(fileOpOk,false,FILE_OPERATION_SAVE_SCENE_AS_EXXML_FOCMD,IDS_SCENE_AS_XML___MENU_ITEM);
        saveSceneAsXmlMenu->appendMenuItem(fileOpOk,false,FILE_OPERATION_SAVE_SCENE_AS_SIMPLEXML_FOCMD,IDS_SCENE_AS_SIMPLEXML___MENU_ITEM);
        saveSceneMenu->appendMenuAndDetach(saveSceneAsXmlMenu,fileOpOk,IDS_SAVE_SCENE_AS_XML_MENU_ITEM);
        menu->appendMenuAndDetach(saveSceneMenu,fileOpOk,IDS_SAVE_SCENE_AS_MENU_ITEM);
        VMenu* saveModelMenu=new VMenu();
        saveModelMenu->appendMenuItem(fileOpOk&&justModelSelected,false,FILE_OPERATION_SAVE_MODEL_AS_CSIM_FOCMD,IDS_MODEL_AS_CSIM___MENU_ITEM);
        saveModelMenu->appendMenuItem(fileOpOk&&justModelSelected,false,FILE_OPERATION_SAVE_MODEL_AS_EXXML_FOCMD,IDS_MODEL_AS_XML___MENU_ITEM);
        menu->appendMenuAndDetach(saveModelMenu,fileOpOk&&justModelSelected,IDS_SAVE_MODEL_AS_MENU_ITEM);
    }

    if (CSimFlavor::getIntVal(2)==2)
    {
        menu->appendMenuSeparator();
        if (CSimFlavor::getBoolVal(14))
        {
            menu->appendMenuItem(fileOpOk,false,FILE_OPERATION_SAVE_SCENE_FOCMD,IDS_SAVE_SCENE_MENU_ITEM);
            VMenu* saveSceneMenu=new VMenu();
            saveSceneMenu->appendMenuItem(fileOpOk,false,FILE_OPERATION_SAVE_SCENE_AS_CSIM_FOCMD,IDS_SCENE_AS_CSIM___MENU_ITEM);
            saveSceneMenu->appendMenuItem(fileOpOk,false,FILE_OPERATION_SAVE_SCENE_AS_XR_FOCMD,IDS_SCENE_AS_XR___MENU_ITEM);
            VMenu* saveSceneAsXmlMenu=new VMenu();
            saveSceneAsXmlMenu->appendMenuItem(fileOpOk,false,FILE_OPERATION_SAVE_SCENE_AS_EXXML_FOCMD,IDS_SCENE_AS_XML___MENU_ITEM);
            saveSceneAsXmlMenu->appendMenuItem(fileOpOk,false,FILE_OPERATION_SAVE_SCENE_AS_SIMPLEXML_FOCMD,IDS_SCENE_AS_SIMPLEXML___MENU_ITEM);
            saveSceneMenu->appendMenuAndDetach(saveSceneAsXmlMenu,fileOpOk,IDS_SAVE_SCENE_AS_XML_MENU_ITEM);
            menu->appendMenuAndDetach(saveSceneMenu,fileOpOk,IDS_SAVE_SCENE_AS_MENU_ITEM);

            VMenu* saveModelMenu=new VMenu();
            saveModelMenu->appendMenuItem(fileOpOk&&justModelSelected,false,FILE_OPERATION_SAVE_MODEL_AS_CSIM_FOCMD,IDS_MODEL_AS_CSIM___MENU_ITEM);
            saveModelMenu->appendMenuItem(fileOpOk&&justModelSelected,false,FILE_OPERATION_SAVE_MODEL_AS_XR_FOCMD,IDS_MODEL_AS_XR___MENU_ITEM);
            saveModelMenu->appendMenuItem(fileOpOk&&justModelSelected,false,FILE_OPERATION_SAVE_MODEL_AS_EXXML_FOCMD,IDS_MODEL_AS_XML___MENU_ITEM);
            menu->appendMenuAndDetach(saveModelMenu,fileOpOk&&justModelSelected,IDS_SAVE_MODEL_AS_MENU_ITEM);
        }
        else
            menu->appendMenuItem(false,false,0,CSimFlavor::getStringVal(10).c_str());
    }

    if (CSimFlavor::getIntVal(2)==3)
    {
        menu->appendMenuSeparator();
        if (CSimFlavor::getBoolVal(14))
        {
            menu->appendMenuItem(fileOpOk,false,FILE_OPERATION_SAVE_SCENE_FOCMD,IDS_SAVE_SCENE_MENU_ITEM);
            menu->appendMenuItem(fileOpOk,false,FILE_OPERATION_SAVE_SCENE_AS_XR_FOCMD,IDS_SAVE_SCENE_AS___MENU_ITEM);
        }
        else
            menu->appendMenuItem(false,false,0,CSimFlavor::getStringVal(10).c_str());
    }

    if ( (CSimFlavor::getIntVal(2)==-1)||(CSimFlavor::getIntVal(2)==1)||(CSimFlavor::getIntVal(2)==2) )
    {
        menu->appendMenuSeparator();
        VMenu* impMenu=new VMenu();
        impMenu->appendMenuItem(fileOpOk,false,FILE_OPERATION_IMPORT_MESH_FOCMD,IDS_IMPORT_MESH___MENU_ITEM);
        impMenu->appendMenuItem(fileOpOk,false,FILE_OPERATION_IMPORT_PATH_FOCMD,IDS_IMPORT_PATH___MENU_ITEM);
        impMenu->appendMenuItem(fileOpOk,false,FILE_OPERATION_IMPORT_HEIGHTFIELD_FOCMD,(std::string(IDSN_IMPORT_HEIGHTFIELD)+"...").c_str());
        menu->appendMenuAndDetach(impMenu,true,IDSN_IMPORT_MENU_ITEM);

        VMenu* expMenu=new VMenu();
        expMenu->appendMenuItem(simStoppedOrPausedNoEditMode&&(shapeNumber>0),false,FILE_OPERATION_EXPORT_SHAPE_FOCMD,IDS_EXPORT_SHAPE_MENU_ITEM);
        expMenu->appendMenuItem(fileOpOk&&(graphNumber!=0),false,FILE_OPERATION_EXPORT_GRAPHS_FOCMD,IDS_EXPORT_SELECTED_GRAPHS_MENU_ITEM);
        expMenu->appendMenuItem(fileOpOk&&(pathNumber==1)&&(selItems==1),false,FILE_OPERATION_EXPORT_PATH_SIMPLE_POINTS_FOCMD,IDS_EXPORT_SELECTED_PATH_MENU_ITEM);
        expMenu->appendMenuItem(fileOpOk&&(pathNumber==1)&&(selItems==1),false,FILE_OPERATION_EXPORT_PATH_BEZIER_POINTS_FOCMD,IDS_EXPORT_SELECTED_PATH_BEZIER_CURVE_MENU_ITEM);
        if (App::userSettings->showOldDlgs)
            expMenu->appendMenuItem(fileOpOk,false,FILE_OPERATION_EXPORT_IK_CONTENT_FOCMD,IDS_EXPORT_IK_CONTENT_MENU_ITEM);
        bool canExportDynamicContent=CPluginContainer::dyn_isDynamicContentAvailable()!=0;
        expMenu->appendMenuItem(canExportDynamicContent,false,FILE_OPERATION_EXPORT_DYNAMIC_CONTENT_FOCMD,IDSN_EXPORT_DYNAMIC_CONTENT);
        menu->appendMenuAndDetach(expMenu,true,IDSN_EXPORT_MENU_ITEM);
    }

    menu->appendMenuSeparator();
    menu->appendMenuItem(true,false,FILE_OPERATION_EXIT_SIMULATOR_FOCMD,IDS_EXIT_MENU_ITEM);
}

bool CFileOperations::_saveSceneWithDialogAndEverything()
{ // SHOULD ONLY BE CALLED BY THE MAIN SIMULATION THREAD!
    bool retVal=false;
    if (!App::currentWorld->environment->getSceneLocked())
    {
        if (App::currentWorld->mainSettings->getScenePathAndName()=="")
            retVal=_saveSceneAsWithDialogAndEverything(CSimFlavor::getIntVal(1));
        else
        {
            if ( (!App::currentWorld->environment->getRequestFinalSave())||(VMESSAGEBOX_REPLY_YES==App::uiThread->messageBox_warning(App::mainWindow,IDSN_SAVE,IDS_FINAL_SCENE_SAVE_WARNING,VMESSAGEBOX_YES_NO,VMESSAGEBOX_REPLY_YES)) )
            {
                if (App::currentWorld->environment->getRequestFinalSave())
                    App::currentWorld->environment->setSceneLocked();
                std::string infoPrintOut(IDSN_SAVING_SCENE);
                infoPrintOut+="...";
                App::logMsg(sim_verbosity_msgs,infoPrintOut.c_str());
                if (saveScene(App::currentWorld->mainSettings->getScenePathAndName().c_str(),true,true,true,false))
                {
                    addToRecentlyOpenedScenes(App::currentWorld->mainSettings->getScenePathAndName());
                    App::currentWorld->undoBufferContainer->clearSceneSaveMaybeNeededFlag();
                }
            }
            retVal=true;
        }
    }
    else
        App::uiThread->messageBox_warning(App::mainWindow,IDSN_SCENE,IDS_SCENE_IS_LOCKED_WARNING,VMESSAGEBOX_OKELI,VMESSAGEBOX_REPLY_OK);
    return(retVal);
}

bool CFileOperations::_saveSceneAsWithDialogAndEverything(int filetype)
{
    bool retVal=false;
    if (!App::currentWorld->environment->getSceneLocked())
    {
        if ( ((!App::currentWorld->environment->getRequestFinalSave())||(filetype==CSer::filetype_csim_bin_scene_file)) ||(VMESSAGEBOX_REPLY_YES==App::uiThread->messageBox_warning(App::mainWindow,IDSN_SAVE,IDS_FINAL_SCENE_SAVE_WARNING,VMESSAGEBOX_YES_NO,VMESSAGEBOX_REPLY_YES)) )
        {
            if (App::currentWorld->environment->getRequestFinalSave()&&(filetype!=CSer::filetype_csim_bin_scene_file))
                App::currentWorld->environment->setSceneLocked();

            std::string infoPrintOut(IDSN_SAVING_SCENE);
            infoPrintOut+="...";
            App::logMsg(sim_verbosity_msgs,infoPrintOut.c_str());
            std::string initPath;
            if (App::currentWorld->mainSettings->getScenePathAndName().size()==0)
                initPath=App::folders->getScenesPath();
            else
                initPath=App::currentWorld->mainSettings->getScenePath();
            std::string filenameAndPath;
            std::string sceneName(App::currentWorld->mainSettings->getScenePathAndName());
            sceneName=VVarious::splitPath_fileBaseAndExtension(sceneName.c_str());
            std::string ext=VVarious::splitPath_fileExtension(sceneName.c_str());
            std::transform(ext.begin(),ext.end(),ext.begin(),::tolower);
            if ( (filetype==CSer::filetype_csim_bin_scene_file)&&(ext.compare(SIM_SCENE_EXTENSION)!=0) )
                sceneName="";
            if ( (filetype==CSer::filetype_csim_xml_xscene_file)&&(ext.compare(SIM_XML_SCENE_EXTENSION)!=0) )
                sceneName="";
            if ( (filetype==CSer::filetype_csim_xml_simplescene_file)&&(ext.compare(SIM_XML_SCENE_EXTENSION)!=0) )
                sceneName="";
            if ( (filetype==CSer::filetype_xr_bin_scene_file)&&(ext.compare(SIM_XR_SCENE_EXTENSION)!=0) )
                sceneName="";

            if (filetype==CSer::filetype_csim_bin_scene_file)
                filenameAndPath=App::uiThread->getSaveFileName(App::mainWindow,0,tt::decorateString("",IDSN_SAVING_SCENE,"...").c_str(),initPath.c_str(),sceneName.c_str(),false,"CoppeliaSim Scene",SIM_SCENE_EXTENSION);
            if (filetype==CSer::filetype_csim_xml_xscene_file)
                filenameAndPath=App::uiThread->getSaveFileName(App::mainWindow,0,tt::decorateString("",IDSN_SAVING_SCENE,"...").c_str(),initPath.c_str(),sceneName.c_str(),false,"CoppeliaSim XML Scene (exhaustive)",SIM_XML_SCENE_EXTENSION);
            if (filetype==CSer::filetype_csim_xml_simplescene_file)
                filenameAndPath=App::uiThread->getSaveFileName(App::mainWindow,0,tt::decorateString("",IDSN_SAVING_SCENE,"...").c_str(),initPath.c_str(),sceneName.c_str(),false,"CoppeliaSim XML Scene (simple)",SIM_XML_SCENE_EXTENSION);
            if (filetype==CSer::filetype_xr_bin_scene_file)
                filenameAndPath=App::uiThread->getSaveFileName(App::mainWindow,0,tt::decorateString("",IDSN_SAVING_SCENE,"...").c_str(),initPath.c_str(),sceneName.c_str(),false,"XReality Scene",SIM_XR_SCENE_EXTENSION);

            if (filenameAndPath.length()!=0)
            {
                bool abort=false;
                if (!App::userSettings->suppressXmlOverwriteMsg)
                {
                    if ( (filetype==CSer::filetype_csim_xml_xscene_file)&&(App::userSettings->xmlExportSplitSize!=0) )
                    {
                        std::string prefix(VVarious::splitPath_fileBase(filenameAndPath.c_str())+"_");
                        int cnt0=VFileFinder::countFiles(VVarious::splitPath_path(filenameAndPath.c_str()).c_str());
                        int cnt1=VFileFinder::countFolders(VVarious::splitPath_path(filenameAndPath.c_str()).c_str());
                        int cnt2=VFileFinder::countFilesWithPrefix(VVarious::splitPath_path(filenameAndPath.c_str()).c_str(),prefix.c_str());
                        int cnt3=VFileFinder::countFilesWithPrefix(VVarious::splitPath_path(filenameAndPath.c_str()).c_str(),(VVarious::splitPath_fileBase(filenameAndPath.c_str())+".").c_str());
                        if ( (cnt2+cnt3!=cnt0)||(cnt1!=0) )
                        {
                            std::string msg("The scene/model will possibly be saved as several separate files, all with the '");
                            msg+=prefix;
                            msg+="' prefix. Existing files with the same prefix will be erased or overwritten. To avoid this, it is recommended to either save XML scenes/models in individual folders, or to set the 'xmlExportSplitSize' variable in 'system/usrset.txt' to 0 to generate a single file.\n(this warning can be disabled via the 'suppressXmlOverwriteMsg' variable in 'system/usrset.txt')\n\nProceed anyway?";
                            abort=(VMESSAGEBOX_REPLY_NO==App::uiThread->messageBox_warning(App::mainWindow,IDSN_SAVE,msg.c_str(),VMESSAGEBOX_YES_NO,VMESSAGEBOX_REPLY_YES));
                        }
                    }
                    if (filetype==CSer::filetype_csim_xml_simplescene_file)
                    {
                        std::string prefix(VVarious::splitPath_fileBase(filenameAndPath.c_str())+"_");
                        int cnt0=VFileFinder::countFiles(VVarious::splitPath_path(filenameAndPath.c_str()).c_str());
                        int cnt1=VFileFinder::countFolders(VVarious::splitPath_path(filenameAndPath.c_str()).c_str());
                        int cnt2=VFileFinder::countFilesWithPrefix(VVarious::splitPath_path(filenameAndPath.c_str()).c_str(),prefix.c_str());
                        int cnt3=VFileFinder::countFilesWithPrefix(VVarious::splitPath_path(filenameAndPath.c_str()).c_str(),(VVarious::splitPath_fileBase(filenameAndPath.c_str())+".").c_str());
                        if ( (cnt2+cnt3!=cnt0)||(cnt1!=0) )
                        {
                            std::string msg("The scene/model will possibly be saved as several separate files, all with the '");
                            msg+=prefix;
                            msg+="' prefix. Existing files with the same prefix will be erased or overwritten. To avoid this, it is recommended to save XML scenes/models in individual folders.\n(this warning can be disabled via the 'suppressXmlOverwriteMsg' variable in 'system/usrset.txt')\n\nProceed anyway?";
                            abort=(VMESSAGEBOX_REPLY_NO==App::uiThread->messageBox_warning(App::mainWindow,IDSN_SAVE,msg.c_str(),VMESSAGEBOX_YES_NO,VMESSAGEBOX_REPLY_YES));
                        }
                    }
                }
                if (!abort)
                {
                    if (filetype==CSer::filetype_csim_xml_simplescene_file)
                        filenameAndPath+="@simpleXml";
                    if (saveScene(filenameAndPath.c_str(),true,true,true,true))
                    {
                        addToRecentlyOpenedScenes(App::currentWorld->mainSettings->getScenePathAndName());
                        App::currentWorld->undoBufferContainer->clearSceneSaveMaybeNeededFlag();
                        retVal=true;
                    }
                }
                else
                    App::logMsg(sim_verbosity_msgs,IDSNS_ABORTED);
            }
            else
                App::logMsg(sim_verbosity_msgs,IDSNS_ABORTED);
        }
    }
    else
        App::uiThread->messageBox_warning(App::mainWindow,IDSN_SCENE,IDS_SCENE_IS_LOCKED_WARNING,VMESSAGEBOX_OKELI,VMESSAGEBOX_REPLY_OK);
    return(retVal);
}
#endif
