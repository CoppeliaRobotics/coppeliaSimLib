#include "vrepMainHeader.h"
#include "qdlgobjectdialogcontainer.h"
#include "ui_qdlgobjectdialogcontainer.h"
#include "app.h"
#include "v_repStrings.h"
#include "qdlgmirrors.h"
#include "qdlgcameras.h"
#include "qdlglights.h"
#include "qdlgdummies.h"
#include "qdlgoctrees.h"
#include "qdlgpointclouds.h"
#include "qdlgforcesensors.h"
#include "qdlgvisionsensors.h"
#include "qdlgshapes.h"
#include "qdlgproximitysensors.h"
#include "qdlgmills.h"
#include "qdlgjoints.h"
#include "qdlggraphs.h"
#include "qdlgdetectionvolume.h"
#include "qdlgshapedyn.h"
#include "qdlgpaths.h"
#include "qdlgpathshaping.h"
#include "qdlgcommonproperties.h"

#define TOP_BORDER_WIDTH 30

CQDlgObjectDialogContainer::CQDlgObjectDialogContainer(QWidget *parent) :
    CDlgEx(parent),
    ui(new Ui::CQDlgObjectDialogContainer)
{
    _dlgType=OBJECT_DLG;
    ui->setupUi(this);
    pageDlgs[0]=new CQDlgDummies();
    originalHeights[0]=pageDlgs[0]->size().height();
    ui->qqObjectProp->setText(strTranslate(IDSN_DUMMY));
    objTypeDlg=sim_object_dummy_type;

    pageDlgs[1]=new CQDlgCommonProperties();
    originalHeights[1]=pageDlgs[1]->size().height();

    currentPage=0;
    desiredPage=0;
    bl=new QVBoxLayout();
    bl->setContentsMargins(0,TOP_BORDER_WIDTH,0,0);
    setLayout(bl);
    bl->addWidget(pageDlgs[0]);
    bl->addWidget(pageDlgs[1]);
    pageDlgs[1]->setVisible(false);

    QSize s(pageDlgs[currentPage]->size());
    s.setHeight(originalHeights[currentPage]+TOP_BORDER_WIDTH);
    setFixedSize(s);
}

CQDlgObjectDialogContainer::~CQDlgObjectDialogContainer()
{
    if (App::mainWindow->dlgCont->isVisible(JOINT_DYN_DLG))
        App::mainWindow->dlgCont->toggle(JOINT_DYN_DLG);
    if (App::mainWindow->dlgCont->isVisible(VISION_SENSOR_FILTER_DLG))
        App::mainWindow->dlgCont->toggle(VISION_SENSOR_FILTER_DLG);
    if (App::mainWindow->dlgCont->isVisible(DETECTION_VOLUME_DLG))
        App::mainWindow->dlgCont->toggle(DETECTION_VOLUME_DLG);
    if (App::mainWindow->dlgCont->isVisible(SHAPE_DYN_DLG))
        App::mainWindow->dlgCont->toggle(SHAPE_DYN_DLG);
    if (App::mainWindow->dlgCont->isVisible(PATH_SHAPING_DLG))
        App::mainWindow->dlgCont->toggle(PATH_SHAPING_DLG);
    delete ui;
}

void CQDlgObjectDialogContainer::dialogCallbackFunc(const SUIThreadCommand* cmdIn,SUIThreadCommand* cmdOut)
{
    pageDlgs[currentPage]->dialogCallbackFunc(cmdIn,cmdOut);
}

void CQDlgObjectDialogContainer::refresh()
{
    ui->qqObjectProp->setChecked(desiredPage==0);
    ui->qqCommonProp->setChecked(desiredPage==1);

    if (desiredPage!=currentPage)
    {
        pageDlgs[currentPage]->setVisible(false);
        currentPage=desiredPage;
        pageDlgs[currentPage]->setVisible(true);

        QSize s(pageDlgs[currentPage]->size());
        s.setHeight(originalHeights[currentPage]+TOP_BORDER_WIDTH);

#ifdef MAC_VREP
        // Since Qt5, we have problems on Mac (resising-->ugly glitch)
        setVisible(false);
        setFixedSize(s);
        setVisible(true);
#else
        setFixedSize(s);
#endif

    }

    C3DObject* sel=App::ct->objCont->getLastSelection_object();
    int editMode=App::getEditModeType();
    if (sel!=nullptr)
    {
        int t=sel->getObjectType();
        std::string title;
        if (t==sim_object_octree_type)
            title=strTranslate(IDSN_OCTREE);
        if (t==sim_object_pointcloud_type)
            title=strTranslate(IDSN_POINTCLOUD);
        if (t==sim_object_mirror_type)
            title=strTranslate(IDSN_MIRROR);
        if (t==sim_object_shape_type)
            title=strTranslate(IDSN_SHAPE);
        if (t==sim_object_joint_type)
            title=strTranslate(IDSN_JOINT);
        if (t==sim_object_camera_type)
            title=strTranslate(IDSN_CAMERA);
        if (t==sim_object_dummy_type)
            title=strTranslate(IDSN_DUMMY);
        if (t==sim_object_proximitysensor_type)
            title=strTranslate(IDSN_PROXIMITY_SENSOR);
        if (t==sim_object_path_type)
            title=strTranslate(IDSN_PATH);
        if (t==sim_object_visionsensor_type)
            title=strTranslate(IDSN_VISION_SENSOR);
        if (t==sim_object_mill_type)
            title=strTranslate(IDSN_MILL);
        if (t==sim_object_forcesensor_type)
            title=strTranslate(IDSN_FORCE_SENSOR);
        if (t==sim_object_light_type)
            title=strTranslate(IDSN_LIGHT);
        if (t==sim_object_graph_type)
            title=strTranslate(IDSN_GRAPH);
        ui->qqObjectProp->setText(title.c_str());
    }
    else
    {
        if (editMode==PATH_EDIT_MODE)
            ui->qqObjectProp->setText(strTranslate(IDSN_GRAPH));
        if (editMode&SHAPE_EDIT_MODE)
            ui->qqObjectProp->setText(strTranslate(IDSN_SHAPE));
    }

    if ((currentPage==0)&&((sel!=nullptr)||(editMode==PATH_EDIT_MODE)||(editMode&SHAPE_EDIT_MODE)))
    { // object properties
        int t=-1;
        if (sel!=nullptr)
            t=sel->getObjectType();
        else
        {
            if (editMode==PATH_EDIT_MODE)
                t=sim_object_path_type;
            if (editMode&SHAPE_EDIT_MODE)
                t=sim_object_shape_type;
        }
        if ((t!=objTypeDlg)&&(t!=-1))
        {
            objTypeDlg=t;
            bl->removeWidget(pageDlgs[currentPage]);
            delete pageDlgs[currentPage];
            if (objTypeDlg==sim_object_mirror_type)
                pageDlgs[currentPage]=new CQDlgMirrors();
            if (objTypeDlg==sim_object_octree_type)
                pageDlgs[currentPage]=new CQDlgOctrees();
            if (objTypeDlg==sim_object_pointcloud_type)
                pageDlgs[currentPage]=new CQDlgPointclouds();
            if (objTypeDlg==sim_object_shape_type)
                pageDlgs[currentPage]=new CQDlgShapes();
            if (objTypeDlg==sim_object_joint_type)
                pageDlgs[currentPage]=new CQDlgJoints();
            if (objTypeDlg==sim_object_camera_type)
                pageDlgs[currentPage]=new CQDlgCameras();
            if (objTypeDlg==sim_object_dummy_type)
                pageDlgs[currentPage]=new CQDlgDummies();
            if (objTypeDlg==sim_object_proximitysensor_type)
                pageDlgs[currentPage]=new CQDlgProximitySensors();
            if (objTypeDlg==sim_object_graph_type)
                pageDlgs[currentPage]=new CQDlgGraphs();
            if (objTypeDlg==sim_object_visionsensor_type)
                pageDlgs[currentPage]=new CQDlgVisionSensors();
            if (objTypeDlg==sim_object_path_type)
                pageDlgs[currentPage]=new CQDlgPaths();
            if (objTypeDlg==sim_object_mill_type)
                pageDlgs[currentPage]=new CQDlgMills();
            if (objTypeDlg==sim_object_forcesensor_type)
                pageDlgs[currentPage]=new CQDlgForceSensors();
            if (objTypeDlg==sim_object_light_type)
                pageDlgs[currentPage]=new CQDlgLights();

            originalHeights[currentPage]=pageDlgs[currentPage]->size().height();

            bl->addWidget(pageDlgs[currentPage]);

            QSize s(pageDlgs[currentPage]->size());
            s.setHeight(originalHeights[currentPage]+TOP_BORDER_WIDTH);

#ifdef MAC_VREP
            // Since Qt5, we have problems on Mac (resising-->ugly glitch)
            setVisible(false);
            setFixedSize(s);
            setVisible(true);
#else
            setFixedSize(s);
#endif
        }
    }

    if (currentPage==0)
    {
        if (objTypeDlg==sim_object_path_type)
        {
            if (CQDlgPathShaping::showWindow!=App::mainWindow->dlgCont->isVisible(PATH_SHAPING_DLG))
                App::mainWindow->dlgCont->toggle(PATH_SHAPING_DLG);
        }
        else
        {
            if (App::mainWindow->dlgCont->isVisible(PATH_SHAPING_DLG))
                App::mainWindow->dlgCont->toggle(PATH_SHAPING_DLG);
        }
        if (objTypeDlg==sim_object_shape_type)
        {
            if (CQDlgShapeDyn::showDynamicWindow!=App::mainWindow->dlgCont->isVisible(SHAPE_DYN_DLG))
                App::mainWindow->dlgCont->toggle(SHAPE_DYN_DLG);
        }
        else
        {
            if (App::mainWindow->dlgCont->isVisible(SHAPE_DYN_DLG))
                App::mainWindow->dlgCont->toggle(SHAPE_DYN_DLG);
        }
        if (objTypeDlg==sim_object_joint_type)
        {
            if (CQDlgJoints::showDynamicWindow!=App::mainWindow->dlgCont->isVisible(JOINT_DYN_DLG))
                App::mainWindow->dlgCont->toggle(JOINT_DYN_DLG);
        }
        else
        {
            if (App::mainWindow->dlgCont->isVisible(JOINT_DYN_DLG))
                App::mainWindow->dlgCont->toggle(JOINT_DYN_DLG);
        }
        if (objTypeDlg==sim_object_visionsensor_type)
        {
            if (CQDlgVisionSensors::showFilterWindow!=App::mainWindow->dlgCont->isVisible(VISION_SENSOR_FILTER_DLG))
                App::mainWindow->dlgCont->toggle(VISION_SENSOR_FILTER_DLG);
        }
        else
        {
            if (App::mainWindow->dlgCont->isVisible(VISION_SENSOR_FILTER_DLG))
                App::mainWindow->dlgCont->toggle(VISION_SENSOR_FILTER_DLG);
        }
        if ((objTypeDlg==sim_object_proximitysensor_type)||(objTypeDlg==sim_object_mill_type))
        {
            if (CQDlgDetectionVolume::showVolumeWindow!=App::mainWindow->dlgCont->isVisible(DETECTION_VOLUME_DLG))
                App::mainWindow->dlgCont->toggle(DETECTION_VOLUME_DLG);
        }
        else
        {
            if (App::mainWindow->dlgCont->isVisible(DETECTION_VOLUME_DLG))
                App::mainWindow->dlgCont->toggle(DETECTION_VOLUME_DLG);
        }
    }
    else
    {
        if (App::mainWindow->dlgCont->isVisible(JOINT_DYN_DLG))
            App::mainWindow->dlgCont->toggle(JOINT_DYN_DLG);
        if (App::mainWindow->dlgCont->isVisible(VISION_SENSOR_FILTER_DLG))
            App::mainWindow->dlgCont->toggle(VISION_SENSOR_FILTER_DLG);
        if (App::mainWindow->dlgCont->isVisible(DETECTION_VOLUME_DLG))
            App::mainWindow->dlgCont->toggle(DETECTION_VOLUME_DLG);
        if (App::mainWindow->dlgCont->isVisible(SHAPE_DYN_DLG))
            App::mainWindow->dlgCont->toggle(SHAPE_DYN_DLG);
        if (App::mainWindow->dlgCont->isVisible(PATH_SHAPING_DLG))
            App::mainWindow->dlgCont->toggle(PATH_SHAPING_DLG);
    }


    pageDlgs[currentPage]->refresh();
}

void CQDlgObjectDialogContainer::on_qqObjectProp_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        desiredPage=0;
        refresh();
    }
}

void CQDlgObjectDialogContainer::on_qqCommonProp_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        desiredPage=1;
        refresh();
    }
}
