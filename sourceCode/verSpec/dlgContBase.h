#pragma once

#include "vMenubar.h"
#include "app.h"

class CDlgContBase
{
public:
    static bool handleVerSpec_modalSettingsDlg(){return(false);}
    static void handleVerSpec_addMenu(VMenu* menu,bool noShapePathEditModeNoSelector)
    {
        menu->appendMenuItem(App::mainWindow->getObjPropToggleViaGuiEnabled()&&noShapePathEditModeNoSelector,App::mainWindow->dlgCont->isVisible(OBJECT_DLG),TOGGLE_OBJECT_DLG_CMD,IDSN_OBJECT_PROPERTIES_MENU_ITEM,true);
        menu->appendMenuItem(App::mainWindow->getCalcModulesToggleViaGuiEnabled()&&noShapePathEditModeNoSelector,App::mainWindow->dlgCont->isVisible(CALCULATION_DLG),TOGGLE_CALCULATION_DLG_CMD,IDSN_CALCULATION_MODULE_PROPERTIES_MENU_ITEM,true);
        menu->appendMenuSeparator();
        #ifdef NEWIKFUNC
            menu->appendMenuItem(noShapePathEditModeNoSelector,App::mainWindow->dlgCont->isVisible(INTERACTIVE_IK_DLG),TOGGLE_INTERACTIVE_IK_DLG_CMD,IDSN_INTERACTIVE_IK_DLG,true);
        #endif
        menu->appendMenuItem(noShapePathEditModeNoSelector,App::mainWindow->dlgCont->isVisible(LUA_SCRIPT_DLG),TOGGLE_LUA_SCRIPT_DLG_CMD,IDSN_SCRIPTS,true);
        menu->appendMenuItem(noShapePathEditModeNoSelector,App::mainWindow->dlgCont->isVisible(COLLECTION_DLG),TOGGLE_COLLECTION_DLG_CMD,IDSN_COLLECTIONS,true);
        menu->appendMenuItem(noShapePathEditModeNoSelector,App::mainWindow->dlgCont->isVisible(SELECTION_DLG),TOGGLE_SELECTION_DLG_CMD,IDSN_SELECTION,true);
        menu->appendMenuItem(noShapePathEditModeNoSelector,App::mainWindow->dlgCont->isVisible(ENVIRONMENT_DLG),TOGGLE_ENVIRONMENT_DLG_CMD,IDSN_ENVIRONMENT,true);
        menu->appendMenuItem(noShapePathEditModeNoSelector&&App::mainWindow->getBrowserToggleViaGuiEnabled(),App::getBrowserEnabled(),TOGGLE_BROWSER_DLG_CMD,IDSN_MODEL_BROWSER,true);
        menu->appendMenuItem(App::mainWindow->getHierarchyToggleViaGuiEnabled(),App::mainWindow->oglSurface->isHierarchyEnabled(),TOGGLE_HIERARCHY_DLG_CMD,IDSN_SCENE_HIERARCHY,true);
        menu->appendMenuItem(true,App::mainWindow->dlgCont->isVisible(LAYERS_DLG),TOGGLE_LAYERS_DLG_CMD,IDS_LAYERS,true);
        menu->appendMenuItem(CAuxLibVideo::video_recorderGetEncoderString!=nullptr,App::mainWindow->dlgCont->isVisible(AVI_RECORDER_DLG),TOGGLE_AVI_RECORDER_DLG_CMD,IDSN_AVI_RECORDER,true);
        menu->appendMenuItem(noShapePathEditModeNoSelector,App::mainWindow->dlgCont->isVisible(SETTINGS_DLG),TOGGLE_SETTINGS_DLG_CMD,IDSN_USER_SETTINGS,true);
    }
};
