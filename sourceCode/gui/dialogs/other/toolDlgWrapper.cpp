
#include "vrepMainHeader.h"
#include "v_rep_internal.h"
#include "toolDlgWrapper.h"
#include "global.h"
#include "app.h"
#include "qdlgsettings.h"
#include "qdlglayers.h"
#include "qdlgavirecorder.h"
#include "qdlgselection.h"
#include "qdlgsimulation.h"
#include "qdlginteractiveik.h"
#include "qdlgcommonproperties.h"
#include "qdlgenvironment.h"
#include "qdlgcolor.h"
#include "qdlglightmaterial.h"
#include "qdlgmaterial.h"
#include "qdlgfog.h"
#include "qdlgcollections.h"
#include "qdlgcollisions.h"
#include "qdlgdistances.h"
#include "qdlgscripts.h"
#include "qdlgdynamics.h"
#include "qdlgcameras.h"
#include "qdlglights.h"
#include "qdlgdummies.h"
#include "qdlgforcesensors.h"
#include "qdlgconstraintsolver.h"
#include "qdlgvisionsensors.h"
#include "qdlgfilters.h"
#include "qdlgshapes.h"
#include "qdlgtextures.h"
#include "qdlggeometry.h"
#include "qdlgshapedyn.h"
#include "qdlgproximitysensors.h"
#include "qdlgdetectionvolume.h"
#include "qdlgmills.h"
#include "qdlgobjectdialogcontainer.h"
#include "qdlgcalcdialogcontainer.h"
#include "qdlgshapeeditioncontainer.h"
#include "qdlgjoints.h"
#include "qdlgjointdyn.h"
#include "qdlggraphs.h"
#include "qdlgik.h"
#include "qdlguidialogcontainer.h"
#include "qdlgpathshaping.h"
#include "qdlgpathedit.h"
#include "qdlgtranslationrotationcont.h"
#include "qdlgmultishapeedition.h"
#include "qdlg2d3dgraphproperties.h"
#include "qdlgikelements.h"

CToolDlgWrapper::CToolDlgWrapper(int dlgType,int resID)
{   // The dialog is not created when calling the constructor! 
    // It is created/destroyed in setVisible
    dialogType=dlgType;
    resourceID=resID;
    toolDialog=nullptr;
    position[0]=42000; // indicates: uninitialized
    position[1]=42000;
}

CToolDlgWrapper::~CToolDlgWrapper()
{
    if (toolDialog!=nullptr)
    {
        delete toolDialog;
    }
}

void CToolDlgWrapper::dialogCallbackFunc(const SUIThreadCommand* cmdIn,SUIThreadCommand* cmdOut)
{
    if (toolDialog!=nullptr)
        toolDialog->dialogCallbackFunc(cmdIn,cmdOut);
}

void CToolDlgWrapper::refresh()
{
    if (toolDialog!=nullptr)
        toolDialog->refresh();
}

void CToolDlgWrapper::showDialogButDontOpenIt()
{
    if (toolDialog!=nullptr)
    {
        toolDialog->showDialog(true);
#ifdef LIN_VREP
        toolDialog->bringDialogToTop(); // otherwise the dialog opens behind the last dialog on Linux
#endif
    }
}

void CToolDlgWrapper::hideDialogButDontCloseIt()
{
    if (toolDialog!=nullptr)
        toolDialog->showDialog(false);
}

void CToolDlgWrapper::visibleInstanceAboutToSwitch()
{
    if (toolDialog!=nullptr)
    {
        if (toolDialog->doesInstanceSwitchRequireDestruction())
            setVisible(false,nullptr);
    }
}
void CToolDlgWrapper::setVisible(bool visible,QWidget* parentWindow)
{
    if ( (toolDialog!=nullptr)&&(!visible) )
    {
        getPosition(nullptr); // To save the current position
        delete toolDialog;
        toolDialog=nullptr;
    }
    if (visible)
    {
        if (toolDialog!=nullptr)
            toolDialog->bringDialogToTop();
        else
        {
            if (dialogType==SETTINGS_DLG)
                toolDialog=new CQDlgSettings(App::mainWindow);
            if (dialogType==LAYERS_DLG)
                toolDialog=new CQDlgLayers(App::mainWindow);
            if (dialogType==AVI_RECORDER_DLG)
                toolDialog=new CQDlgAviRecorder(App::mainWindow);
            if (dialogType==SELECTION_DLG)
                toolDialog=new CQDlgSelection(App::mainWindow);
            if (dialogType==SIMULATION_DLG)
                toolDialog=new CQDlgSimulation(App::mainWindow);
            if (dialogType==INTERACTIVE_IK_DLG)
                toolDialog=new CQDlgInteractiveIk(App::mainWindow);
            if (dialogType==ENVIRONMENT_DLG)
                toolDialog=new CQDlgEnvironment(App::mainWindow);
            if (dialogType==COLOR_DLG)
                toolDialog=new CQDlgColor(App::mainWindow);
            if (dialogType==LIGHTMATERIAL_DLG)
                toolDialog=new CQDlgLightMaterial(App::mainWindow);
            if (dialogType==MATERIAL_DLG)
                toolDialog=new CQDlgMaterial(App::mainWindow);
            if (dialogType==FOG_DLG)
                toolDialog=new CQDlgFog(App::mainWindow);
            if (dialogType==COLLECTION_DLG)
                toolDialog=new CQDlgCollections(App::mainWindow);
            if (dialogType==LUA_SCRIPT_DLG)
                toolDialog=new CQDlgScripts(App::mainWindow);
            if (dialogType==VISION_SENSOR_FILTER_DLG)
                toolDialog=new CQDlgFilters(App::mainWindow);
            if (dialogType==TEXTURE_DLG)
                toolDialog=new CQDlgTextures(App::mainWindow);
            if (dialogType==GEOMETRY_DLG)
                toolDialog=new CQDlgGeometry(App::mainWindow);
            if (dialogType==GRAPH2DAND3DCURVES_DLG)
                toolDialog=new CQDlg2D3DGraphProperties(App::mainWindow);
            if (dialogType==IKELEMENT_DLG)
                toolDialog=new CQDlgIkElements(App::mainWindow);



            if (dialogType==SHAPE_DYN_DLG)
                toolDialog=new CQDlgShapeDyn(App::mainWindow);
            if (dialogType==DETECTION_VOLUME_DLG)
                toolDialog=new CQDlgDetectionVolume(App::mainWindow);
            if (dialogType==SHAPE_EDITION_DLG)
                toolDialog=new CQDlgShapeEditionContainer(App::mainWindow);
            if (dialogType==OBJECT_DLG)
                toolDialog=new CQDlgObjectDialogContainer(App::mainWindow);
            if (dialogType==CALCULATION_DLG)
                toolDialog=new CQDlgCalcDialogContainer(App::mainWindow);
            if (dialogType==JOINT_DYN_DLG)
                toolDialog=new CQDlgJointDyn(App::mainWindow);
            if (dialogType==CUSTOM_UI_DLG)
                toolDialog=new CQDlgUiDialogContainer(App::mainWindow);
            if (dialogType==TRANSLATION_ROTATION_DLG)
                toolDialog=new CQDlgTranslationRotationCont(App::mainWindow);
            if (dialogType==PATH_SHAPING_DLG)
                toolDialog=new CQDlgPathShaping(App::mainWindow);
            if (dialogType==PATH_EDITION_DLG)
                toolDialog=new CQDlgPathEdit(App::mainWindow);
            if (dialogType==MULTISHAPE_EDITION_DLG)
                toolDialog=new CQDlgMultishapeEdition(App::mainWindow);

            if (toolDialog!=nullptr)
            {
                if (position[0]==42000)
                { // first time we open that dialog
#ifdef WIN_VREP
                    position[0]=App::mainWindow->geometry().width()+App::mainWindow->geometry().x()-toolDialog->geometry().width()-10; // -10 because when in full-screen, the main window doesn't have a frame and things are a little bit different!!
                    position[1]=App::mainWindow->geometry().y()+100;
#endif
#ifdef MAC_VREP
                    position[0]=400; // doesn't work like under windows somehow...
                    position[1]=200;
#endif
#ifdef LIN_VREP
                    position[0]=App::mainWindow->geometry().width()+App::mainWindow->geometry().x()-toolDialog->geometry().width()-10; // -10 because when in full-screen, the main window doesn't have a frame and things are a little bit different!!
                    position[1]=App::mainWindow->geometry().y()+100;
#endif
                }
                setPosition(position[0],position[1]);
                toolDialog->refresh();
                toolDialog->showDialog(true);
#ifdef LIN_VREP
                toolDialog->bringDialogToTop(); // otherwise the dialog opens behind the last dialog on Linux
#endif
            }
        }
    }
}

void CToolDlgWrapper::destroyIfDestructionNeeded()
{
    if (toolDialog!=nullptr)
    {
        if (toolDialog->needsDestruction())
        {
            getPosition(nullptr); // To save the current position
            delete toolDialog;
            toolDialog=nullptr;
        }
    }
}

bool CToolDlgWrapper::getVisible()
{
    return(toolDialog!=nullptr);
}
void CToolDlgWrapper::getPosition(int pos[2])
{
    if (toolDialog!=nullptr)
        toolDialog->getDialogPositionAndSize(position,nullptr);
    if (pos!=nullptr)
    {
        pos[0]=position[0];
        pos[1]=position[1];
    }
}

void CToolDlgWrapper::setPosition(int x,int y)
{
    position[0]=x;
    position[1]=y;
    if (toolDialog!=nullptr)
        toolDialog->setDialogPositionAndSize(position,nullptr);
}

int CToolDlgWrapper::getDialogType()
{
    return(dialogType);
}

CDlgEx* CToolDlgWrapper::getDialog()
{
    return(toolDialog);
}
