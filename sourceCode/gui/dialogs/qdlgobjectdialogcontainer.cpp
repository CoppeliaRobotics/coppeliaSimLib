#include <qdlgobjectdialogcontainer.h>
#include <ui_qdlgobjectdialogcontainer.h>
#include <app.h>
#include <simStrings.h>
#include <qdlgmirrors.h>
#include <qdlgcameras.h>
#include <qdlglights.h>
#include <qdlgdummies.h>
#include <qdlgscripts.h>
#include <qdlgoctrees.h>
#include <qdlgpointclouds.h>
#include <qdlgforcesensors.h>
#include <qdlgvisionsensors.h>
#include <qdlgshapes.h>
#include <qdlgproximitysensors.h>
#include <qdlgjoints.h>
#include <qdlggraphs.h>
#include <qdlgdetectionvolume.h>
#include <qdlgshapedyn.h>
#include <qdlgpaths.h>
#include <qdlgpathshaping.h>
#include <qdlgcommonproperties.h>
#include <guiApp.h>

#define TOP_BORDER_WIDTH 30

CQDlgObjectDialogContainer::CQDlgObjectDialogContainer(QWidget* parent)
    : CDlgEx(parent), ui(new Ui::CQDlgObjectDialogContainer)
{
    _dlgType = OBJECT_DLG;
    ui->setupUi(this);
    pageDlgs[0] = new CQDlgDummies();
    originalHeights[0] = pageDlgs[0]->size().height();
    if (App::userSettings->showOldDlgs)
        originalHeights[0] = 352;

    ui->qqObjectProp->setText(IDSN_DUMMY);
    objTypeDlg = sim_sceneobject_dummy;

    pageDlgs[1] = new CQDlgCommonProperties();
    originalHeights[1] = pageDlgs[1]->size().height();

    currentPage = 0;
    desiredPage = 0;
    bl = new QVBoxLayout();
    bl->setContentsMargins(0, TOP_BORDER_WIDTH, 0, 0);
    setLayout(bl);
    bl->addWidget(pageDlgs[0]);
    bl->addWidget(pageDlgs[1]);
    pageDlgs[1]->setVisible(false);

    QSize s(pageDlgs[currentPage]->size());
    s.setHeight(originalHeights[currentPage] + TOP_BORDER_WIDTH);
    setFixedSize(s);
}

CQDlgObjectDialogContainer::~CQDlgObjectDialogContainer()
{
    if (GuiApp::mainWindow->dlgCont->isVisible(JOINT_DYN_DLG))
        GuiApp::mainWindow->dlgCont->toggle(JOINT_DYN_DLG);
    if (GuiApp::mainWindow->dlgCont->isVisible(DETECTION_VOLUME_DLG))
        GuiApp::mainWindow->dlgCont->toggle(DETECTION_VOLUME_DLG);
    if (GuiApp::mainWindow->dlgCont->isVisible(SHAPE_DYN_DLG))
        GuiApp::mainWindow->dlgCont->toggle(SHAPE_DYN_DLG);
    if (GuiApp::mainWindow->dlgCont->isVisible(PATH_SHAPING_DLG))
        GuiApp::mainWindow->dlgCont->toggle(PATH_SHAPING_DLG);
    delete ui;
}

void CQDlgObjectDialogContainer::dialogCallbackFunc(const SUIThreadCommand* cmdIn, SUIThreadCommand* cmdOut)
{
    pageDlgs[currentPage]->dialogCallbackFunc(cmdIn, cmdOut);
}

void CQDlgObjectDialogContainer::refresh()
{
    ui->qqObjectProp->setChecked(desiredPage == 0);
    ui->qqCommonProp->setChecked(desiredPage == 1);

    if (desiredPage != currentPage)
    {
        pageDlgs[currentPage]->setVisible(false);
        currentPage = desiredPage;
        pageDlgs[currentPage]->setVisible(true);

        QSize s(pageDlgs[currentPage]->size());
        s.setHeight(originalHeights[currentPage] + TOP_BORDER_WIDTH);

#ifdef MAC_SIM
        // Since Qt5, we have problems on Mac (resising-->ugly glitch)
        setVisible(false);
        setFixedSize(s);
        setVisible(true);
#else
        setFixedSize(s);
#endif
    }

    CSceneObject* sel = App::currentWorld->sceneObjects->getLastSelectionObject();
    int editMode = GuiApp::getEditModeType();
    if (sel != nullptr)
    {
        int t = sel->getObjectType();
        std::string title;
        if (t == sim_sceneobject_octree)
            title = IDSN_OCTREE;
        if (t == sim_sceneobject_pointcloud)
            title = IDSN_POINTCLOUD;
        if (t == sim_sceneobject_mirror)
            title = IDSN_MIRROR;
        if (t == sim_sceneobject_shape)
            title = IDSN_SHAPE;
        if (t == sim_sceneobject_joint)
            title = IDSN_JOINT;
        if (t == sim_sceneobject_camera)
            title = IDSN_CAMERA;
        if (t == sim_sceneobject_dummy)
            title = IDSN_DUMMY;
        if (t == sim_sceneobject_script)
            title = IDSN_SCRIPT;
        if (t == sim_sceneobject_proximitysensor)
            title = IDSN_PROXIMITY_SENSOR;
        if (t == sim_sceneobject_path)
            title = IDSN_PATH;
        if (t == sim_sceneobject_visionsensor)
            title = IDSN_VISION_SENSOR;
        if (t == sim_sceneobject_forcesensor)
            title = IDSN_FORCE_SENSOR;
        if (t == sim_sceneobject_light)
            title = IDSN_LIGHT;
        if (t == sim_sceneobject_graph)
            title = IDSN_GRAPH;
        ui->qqObjectProp->setText(title.c_str());
    }
    else
    {
        if (editMode == PATH_EDIT_MODE_OLD)
            ui->qqObjectProp->setText(IDSN_GRAPH);
        if (editMode & SHAPE_EDIT_MODE)
            ui->qqObjectProp->setText(IDSN_SHAPE);
    }

    if ((currentPage == 0) && ((sel != nullptr) || (editMode == PATH_EDIT_MODE_OLD) || (editMode & SHAPE_EDIT_MODE)))
    { // object properties
        int t = -1;
        if (sel != nullptr)
            t = sel->getObjectType();
        else
        {
            if (editMode == PATH_EDIT_MODE_OLD)
                t = sim_sceneobject_path;
            if (editMode & SHAPE_EDIT_MODE)
                t = sim_sceneobject_shape;
        }
        if ((t != objTypeDlg) && (t != -1))
        {
            objTypeDlg = t;
            bl->removeWidget(pageDlgs[currentPage]);
            delete pageDlgs[currentPage];
            if (objTypeDlg == sim_sceneobject_mirror)
                pageDlgs[currentPage] = new CQDlgMirrors();
            if (objTypeDlg == sim_sceneobject_octree)
                pageDlgs[currentPage] = new CQDlgOctrees();
            if (objTypeDlg == sim_sceneobject_pointcloud)
                pageDlgs[currentPage] = new CQDlgPointclouds();
            if (objTypeDlg == sim_sceneobject_shape)
                pageDlgs[currentPage] = new CQDlgShapes();
            if (objTypeDlg == sim_sceneobject_joint)
                pageDlgs[currentPage] = new CQDlgJoints();
            if (objTypeDlg == sim_sceneobject_camera)
                pageDlgs[currentPage] = new CQDlgCameras();
            if (objTypeDlg == sim_sceneobject_dummy)
                pageDlgs[currentPage] = new CQDlgDummies();
            if (objTypeDlg == sim_sceneobject_script)
                pageDlgs[currentPage] = new CQDlgScripts();
            if (objTypeDlg == sim_sceneobject_proximitysensor)
                pageDlgs[currentPage] = new CQDlgProximitySensors();
            if (objTypeDlg == sim_sceneobject_graph)
                pageDlgs[currentPage] = new CQDlgGraphs();
            if (objTypeDlg == sim_sceneobject_visionsensor)
                pageDlgs[currentPage] = new CQDlgVisionSensors();
            if (objTypeDlg == sim_sceneobject_path)
                pageDlgs[currentPage] = new CQDlgPaths();
            if (objTypeDlg == sim_sceneobject_forcesensor)
                pageDlgs[currentPage] = new CQDlgForceSensors();
            if (objTypeDlg == sim_sceneobject_light)
                pageDlgs[currentPage] = new CQDlgLights();

            originalHeights[currentPage] = pageDlgs[currentPage]->size().height();
            if ((App::userSettings->showOldDlgs) && (objTypeDlg == sim_sceneobject_dummy))
                originalHeights[0] = 352;
            if ((App::userSettings->showOldDlgs) && (objTypeDlg == sim_sceneobject_graph))
                originalHeights[0] = 501;

            bl->addWidget(pageDlgs[currentPage]);

            QSize s(pageDlgs[currentPage]->size());
            s.setHeight(originalHeights[currentPage] + TOP_BORDER_WIDTH);

#ifdef MAC_SIM
            // Since Qt5, we have problems on Mac (resising-->ugly glitch)
            setVisible(false);
            setFixedSize(s);
            setVisible(true);
#else
            setFixedSize(s);
#endif
        }
    }

    if (currentPage == 0)
    {
        if (objTypeDlg == sim_sceneobject_path)
        {
            if (CQDlgPathShaping::showWindow != GuiApp::mainWindow->dlgCont->isVisible(PATH_SHAPING_DLG))
                GuiApp::mainWindow->dlgCont->toggle(PATH_SHAPING_DLG);
        }
        else
        {
            if (GuiApp::mainWindow->dlgCont->isVisible(PATH_SHAPING_DLG))
                GuiApp::mainWindow->dlgCont->toggle(PATH_SHAPING_DLG);
        }
        if (objTypeDlg == sim_sceneobject_shape)
        {
            if (CQDlgShapeDyn::showDynamicWindow != GuiApp::mainWindow->dlgCont->isVisible(SHAPE_DYN_DLG))
                GuiApp::mainWindow->dlgCont->toggle(SHAPE_DYN_DLG);
        }
        else
        {
            if (GuiApp::mainWindow->dlgCont->isVisible(SHAPE_DYN_DLG))
                GuiApp::mainWindow->dlgCont->toggle(SHAPE_DYN_DLG);
        }
        if (objTypeDlg == sim_sceneobject_joint)
        {
            if (CQDlgJoints::showDynamicWindow != GuiApp::mainWindow->dlgCont->isVisible(JOINT_DYN_DLG))
                GuiApp::mainWindow->dlgCont->toggle(JOINT_DYN_DLG);
        }
        else
        {
            if (GuiApp::mainWindow->dlgCont->isVisible(JOINT_DYN_DLG))
                GuiApp::mainWindow->dlgCont->toggle(JOINT_DYN_DLG);
        }
        if ((objTypeDlg == sim_sceneobject_proximitysensor) || (objTypeDlg == sim_sceneobject_mill))
        {
            if (CQDlgDetectionVolume::showVolumeWindow != GuiApp::mainWindow->dlgCont->isVisible(DETECTION_VOLUME_DLG))
                GuiApp::mainWindow->dlgCont->toggle(DETECTION_VOLUME_DLG);
        }
        else
        {
            if (GuiApp::mainWindow->dlgCont->isVisible(DETECTION_VOLUME_DLG))
                GuiApp::mainWindow->dlgCont->toggle(DETECTION_VOLUME_DLG);
        }
    }
    else
    {
        if (GuiApp::mainWindow->dlgCont->isVisible(JOINT_DYN_DLG))
            GuiApp::mainWindow->dlgCont->toggle(JOINT_DYN_DLG);
        if (GuiApp::mainWindow->dlgCont->isVisible(DETECTION_VOLUME_DLG))
            GuiApp::mainWindow->dlgCont->toggle(DETECTION_VOLUME_DLG);
        if (GuiApp::mainWindow->dlgCont->isVisible(SHAPE_DYN_DLG))
            GuiApp::mainWindow->dlgCont->toggle(SHAPE_DYN_DLG);
        if (GuiApp::mainWindow->dlgCont->isVisible(PATH_SHAPING_DLG))
            GuiApp::mainWindow->dlgCont->toggle(PATH_SHAPING_DLG);
    }

    pageDlgs[currentPage]->refresh();
}

void CQDlgObjectDialogContainer::on_qqObjectProp_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        desiredPage = 0;
        refresh();
    }
}

void CQDlgObjectDialogContainer::on_qqCommonProp_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        desiredPage = 1;
        refresh();
    }
}
