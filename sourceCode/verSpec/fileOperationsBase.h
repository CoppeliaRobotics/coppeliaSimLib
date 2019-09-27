#pragma once

#include "app.h"
#ifdef SIM_WITH_GUI
    #include "vMenubar.h"
#endif

class CFileOperationsBase
{
public:
    static bool handleVerSpec_canSaveScene(){return(true);}
    static bool handleVerSpec_canSaveModel(){return(true);}
    static void handleVerSpec_createNewScene(){}
    static void handleVerSpec_loadScene1(){}
    static void handleVerSpec_loadScene2(){}
    static void handleVerSpec_loadModel1(){}

#ifdef SIM_WITH_GUI
    static bool handleVerSpec_saveSceneAsWithDialogAndEverything1(){return(CSer::CSer::filetype_vrep_bin_scene_file);}
    static std::string handleVerSpec_saveSceneAsWithDialogAndEverything2(int fileType,const std::string& initPath)
    {
        std::string retStr;
        std::string sceneName(App::ct->mainSettings->getScenePathAndName());
        sceneName=VVarious::splitPath_fileBaseAndExtension(sceneName);
        std::string ext=VVarious::splitPath_fileExtension(sceneName);
        std::transform(ext.begin(),ext.end(),ext.begin(),::tolower);
        if ( (fileType==CSer::filetype_vrep_bin_scene_file)&&(ext.compare(VREP_SCENE_EXTENSION)!=0) )
            sceneName="";

        if (fileType==CSer::filetype_vrep_bin_scene_file)
            retStr=App::uiThread->getSaveFileName(App::mainWindow,0,tt::decorateString("",IDSN_SAVING_SCENE,"..."),initPath,sceneName,false,"V-REP Scene",VREP_SCENE_EXTENSION);
        return(retStr);
    }
    static std::string handleVerSpec_openScenePhase2()
    {
        std::string tst(App::directories->sceneDirectory);
        std::string retStr=App::uiThread->getOpenFileName(App::mainWindow,0,IDSN_LOADING_SCENE,tst,"",false,"Scenes",VREP_SCENE_EXTENSION);
        return(retStr);
    }
    static void handleVerSpec_openRecentScene(){}
    static std::string handleVerSpec_loadModel2()
    {
        std::string tst(App::directories->modelDirectory);
        std::string retStr=App::uiThread->getOpenFileName(App::mainWindow,0,IDSN_LOADING_MODEL,tst,"",false,"Models",VREP_MODEL_EXTENSION);
        return(retStr);
    }
    static std::string handleVerSpec_saveModel(int what)
    {
        std::string retStr;
        std::string tst(App::directories->modelDirectory);
        if (what==0)
            retStr=App::uiThread->getSaveFileName(App::mainWindow,0,IDS_SAVING_MODEL___,tst,"",false,"V-REP Model",VREP_MODEL_EXTENSION);
        return(retStr);
    }
    static void handleVerSpec_addMenu1(VMenu* menu)
    {
        bool fileOpOk=(App::ct->simulation->isSimulationStopped())&&(App::getEditModeType()==NO_EDIT_MODE);
        bool simStoppedOrPausedNoEditMode=App::ct->simulation->isSimulationStopped()||App::ct->simulation->isSimulationPaused();
        bool fileOpOkAlsoDuringSimulation=(App::getEditModeType()==NO_EDIT_MODE);
        int selItems=App::ct->objCont->getSelSize();
        bool justModelSelected=false;
        if (selItems==1)
        {
            C3DObject* obj=App::ct->objCont->getObjectFromHandle(App::ct->objCont->getSelID(0));
            justModelSelected=(obj!=nullptr)&&(obj->getModelBase());
        }
        std::vector<int> sel;
        sel.reserve(App::ct->objCont->getSelSize());
        for (int i=0;i<App::ct->objCont->getSelSize();i++)
            sel.push_back(App::ct->objCont->getSelID(i));
        CSceneObjectOperations::addRootObjectChildrenToSelection(sel);
        int shapeNumber=App::ct->objCont->getShapeNumberInSelection(&sel);
        int pathNumber=App::ct->objCont->getPathNumberInSelection(&sel);
        int graphNumber=App::ct->objCont->getGraphNumberInSelection(&sel);

        menu->appendMenuItem(fileOpOk,false,FILE_OPERATION_NEW_SCENE_FOCMD,IDS_NEW_SCENE_MENU_ITEM);
        menu->appendMenuItem(fileOpOk,false,FILE_OPERATION_OPEN_SCENE_FOCMD,IDS_OPEN_SCENE___MENU_ITEM);

        // recent scene files:
        CPersistentDataContainer cont(FILENAME_OF_USER_SETTINGS_IN_BINARY_FILE);
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
                recentSceneMenu->appendMenuItem(fileOpOk,false,FILE_OPERATION_OPEN_RECENT_SCENE0_FOCMD+i,VVarious::splitPath_fileBaseAndExtension(recentScenes[i]).c_str());
        }
        menu->appendMenuAndDetach(recentSceneMenu,(recentScenesCnt>0)&&fileOpOk,IDS_OPEN_RECENT_SCENE_MENU_ITEM);

        menu->appendMenuItem(fileOpOkAlsoDuringSimulation,false,FILE_OPERATION_LOAD_MODEL_FOCMD,IDS_LOAD_MODEL___MENU_ITEM);
        menu->appendMenuSeparator();

        menu->appendMenuItem(fileOpOk,false,FILE_OPERATION_CLOSE_SCENE_FOCMD,IDS_CLOSE_SCENE_MENU_ITEM);
        menu->appendMenuSeparator();

        menu->appendMenuItem(fileOpOk,false,FILE_OPERATION_SAVE_SCENE_FOCMD,IDS_SAVE_SCENE_MENU_ITEM);
        VMenu* saveSceneMenu=new VMenu();
        saveSceneMenu->appendMenuItem(fileOpOk,false,FILE_OPERATION_SAVE_SCENE_AS_VREP_FOCMD,IDS_SCENE_AS_VREP___MENU_ITEM);
        menu->appendMenuAndDetach(saveSceneMenu,fileOpOk,IDS_SAVE_SCENE_AS_MENU_ITEM);
        VMenu* saveModelMenu=new VMenu();
        saveModelMenu->appendMenuItem(fileOpOk&&justModelSelected,false,FILE_OPERATION_SAVE_MODEL_AS_VREP_FOCMD,IDS_MODEL_AS_VREP___MENU_ITEM);
        menu->appendMenuAndDetach(saveModelMenu,fileOpOk&&justModelSelected,IDS_SAVE_MODEL_AS_MENU_ITEM);
        menu->appendMenuSeparator();
        VMenu* impMenu=new VMenu();
        impMenu->appendMenuItem(fileOpOk,false,FILE_OPERATION_IMPORT_MESH_FOCMD,IDS_IMPORT_MESH___MENU_ITEM);
        impMenu->appendMenuItem(fileOpOk,false,FILE_OPERATION_IMPORT_PATH_FOCMD,IDS_IMPORT_PATH___MENU_ITEM);
        impMenu->appendMenuItem(fileOpOk,false,FILE_OPERATION_IMPORT_HEIGHTFIELD_FOCMD,std::string(IDSN_IMPORT_HEIGHTFIELD)+"...");
        menu->appendMenuAndDetach(impMenu,true,IDSN_IMPORT_MENU_ITEM);

        VMenu* expMenu=new VMenu();
        expMenu->appendMenuItem(simStoppedOrPausedNoEditMode&&(shapeNumber>0),false,FILE_OPERATION_EXPORT_SHAPE_FOCMD,IDS_EXPORT_SHAPE_MENU_ITEM);
        expMenu->appendMenuItem(fileOpOk&&(graphNumber!=0),false,FILE_OPERATION_EXPORT_GRAPHS_FOCMD,IDS_EXPORT_SELECTED_GRAPHS_MENU_ITEM);
        expMenu->appendMenuItem(fileOpOk&&(pathNumber==1)&&(selItems==1),false,FILE_OPERATION_EXPORT_PATH_SIMPLE_POINTS_FOCMD,IDS_EXPORT_SELECTED_PATH_MENU_ITEM);
        expMenu->appendMenuItem(fileOpOk&&(pathNumber==1)&&(selItems==1),false,FILE_OPERATION_EXPORT_PATH_BEZIER_POINTS_FOCMD,IDS_EXPORT_SELECTED_PATH_BEZIER_CURVE_MENU_ITEM);
        expMenu->appendMenuItem(fileOpOk,false,FILE_OPERATION_EXPORT_IK_CONTENT_FOCMD,IDS_EXPORT_IK_CONTENT_MENU_ITEM);
        bool canExportDynamicContent=CPluginContainer::dyn_isDynamicContentAvailable()!=0;
        expMenu->appendMenuItem(canExportDynamicContent,false,FILE_OPERATION_EXPORT_DYNAMIC_CONTENT_FOCMD,IDSN_EXPORT_DYNAMIC_CONTENT);
        menu->appendMenuAndDetach(expMenu,true,IDSN_EXPORT_MENU_ITEM);
        menu->appendMenuSeparator();
        menu->appendMenuItem(true,false,FILE_OPERATION_EXIT_SIMULATOR_FOCMD,IDS_EXIT_MENU_ITEM);
    }
#endif
};
