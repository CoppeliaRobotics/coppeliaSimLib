
#pragma once

#include <QByteArray>
#include "vMenubar.h"

class CMainWindowBase
{
public:
    CMainWindowBase(){}
    virtual ~CMainWindowBase(){}

    bool handleVerSpec_vrepMessageHandler1(int id){return(false);}
    bool handleVerSpec_onKeyPress1(const QByteArray& ba){return(false);}
    bool handleVerSpec_showAddMenubarItem(){return(true);}
    bool handleVerSpec_showPluginMenubarItem(){return(true);}
    bool handleVerSpec_showAddOnMenubarItem(){return(true);}
    bool handleVerSpec_showLayoutMenubarItem(){return(false);}
    bool handleVerSpec_showJobMenubarItem(){return(false);}
    bool handleVerSpec_showSMenubarItem(std::string& str){return(false);}
    bool handleVerSpec_showCameraAngleButton(){return(true);}
    bool handleVerSpec_showClickSelectionButton(){return(true);}
    bool handleVerSpec_showTransferDnaButton(){return(true);}
    bool handleVerSpec_showDynContentVisualizationButton(){return(true);}
    bool handleVerSpec_showEnginePrecisionCombo(){return(true);}
    bool handleVerSpec_showTimeStepConfigCombo(){return(true);}
    bool handleVerSpec_showOnlineButton(){return(false);}
    bool handleVerSpec_showVerifyButton(){return(false);}
    bool handleVerSpec_showRealTimeButton(){return(true);}
    bool handleVerSpec_showReduceSpeedButton(){return(true);}
    bool handleVerSpec_showIncreaseSpeedButton(){return(true);}
    bool handleVerSpec_showThreadedRenderingButton(){return(true);}
    bool handleVerSpec_showSimulationSettingsButton(){return(true);}
    bool handleVerSpec_showObjectPropertiesButton(){return(true);}
    bool handleVerSpec_showCalculationModulesButton(){return(true);}
    bool handleVerSpec_showCollectionsButton(){return(true);}
    bool handleVerSpec_showScriptsButton(){return(true);}
    bool handleVerSpec_showShapeEditionButton(){return(true);}
    bool handleVerSpec_showPathEditionButton(){return(true);}
    bool handleVerSpec_showSelectionButton(){return(true);}
    bool handleVerSpec_showSceneHierarchyButton(){return(true);}
    bool handleVerSpec_showLayersButton(){return(true);}
    bool handleVerSpec_userSettingsButtonCheckable(){return(true);}
    int handleVerSpec_checkSceneExt(const std::string& extension){return(0);}
    int handleVerSpec_checkModelExt(const std::string& extension){return(0);}
    void handleVerSpec_aboutToShowSSystemMenu(VMenu* m){}
};
