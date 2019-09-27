
#include "vrepMainHeader.h"
#include "qdlgselection.h"
#include "ui_qdlgselection.h"
#include "app.h"
#include "v_repStrings.h"
#include <boost/lexical_cast.hpp>

CQDlgSelection::CQDlgSelection(QWidget *parent) :
      CDlgEx(parent),
      ui(new Ui::CQDlgSelection)
{
    _dlgType=SELECTION_DLG;
    ui->setupUi(this);
}

CQDlgSelection::~CQDlgSelection()
{
    delete ui;
}

void CQDlgSelection::refresh()
{
    std::string tmp;
    tmp=std::string(IDS_SIMPLE_SHAPES)+": "+boost::lexical_cast<std::string>(App::ct->objCont->getSimpleShapeNumberInSelection())+"/"+
        boost::lexical_cast<std::string>(App::ct->objCont->getSimpleShapeNumberInSelection(&App::ct->objCont->objectList));
    ui->simpleShapesButton->setText(tmp.c_str());
    tmp=std::string(IDS_COMPOUND_SHAPES)+": "+boost::lexical_cast<std::string>(App::ct->objCont->getCompoundNumberInSelection())+"/"+
        boost::lexical_cast<std::string>(App::ct->objCont->getCompoundNumberInSelection(&App::ct->objCont->objectList));
    ui->groupedShapesButton->setText(tmp.c_str());
    tmp=std::string(IDS_JOINTS)+": "+boost::lexical_cast<std::string>(App::ct->objCont->getJointNumberInSelection())+"/"+
        boost::lexical_cast<std::string>(App::ct->objCont->getJointNumberInSelection(&App::ct->objCont->objectList));
    ui->jointsButton->setText(tmp.c_str());
    tmp=std::string(IDS_CAMERAS)+": "+boost::lexical_cast<std::string>(App::ct->objCont->getCameraNumberInSelection())+"/"+
        boost::lexical_cast<std::string>(App::ct->objCont->getCameraNumberInSelection(&App::ct->objCont->objectList));
    ui->camerasButton->setText(tmp.c_str());
    tmp=std::string(IDS_LIGHTS)+": "+boost::lexical_cast<std::string>(App::ct->objCont->getLightNumberInSelection())+"/"+
        boost::lexical_cast<std::string>(App::ct->objCont->getLightNumberInSelection(&App::ct->objCont->objectList));
    ui->lightsButton->setText(tmp.c_str());
    tmp=std::string(IDS_PROXSENSORS)+": "+boost::lexical_cast<std::string>(App::ct->objCont->getProxSensorNumberInSelection())+"/"+
        boost::lexical_cast<std::string>(App::ct->objCont->getProxSensorNumberInSelection(&App::ct->objCont->objectList));
    ui->proximitySensorsButton->setText(tmp.c_str());
    tmp=std::string(IDS_VISION_SENSORS)+": "+boost::lexical_cast<std::string>(App::ct->objCont->getVisionSensorNumberInSelection())+"/"+
        boost::lexical_cast<std::string>(App::ct->objCont->getVisionSensorNumberInSelection(&App::ct->objCont->objectList));
    ui->renderingSensorsButton->setText(tmp.c_str());
    tmp=std::string(IDS_FORCE_SENSORS)+": "+boost::lexical_cast<std::string>(App::ct->objCont->getForceSensorNumberInSelection())+"/"+
        boost::lexical_cast<std::string>(App::ct->objCont->getForceSensorNumberInSelection(&App::ct->objCont->objectList));
    ui->forceSensorsButton->setText(tmp.c_str());
    tmp=std::string(IDS_DUMMIES)+": "+boost::lexical_cast<std::string>(App::ct->objCont->getDummyNumberInSelection())+"/"+
        boost::lexical_cast<std::string>(App::ct->objCont->getDummyNumberInSelection(&App::ct->objCont->objectList));
    ui->dummiesButton->setText(tmp.c_str());
    tmp=std::string(IDS_GRAPHS)+": "+boost::lexical_cast<std::string>(App::ct->objCont->getGraphNumberInSelection())+"/"+
        boost::lexical_cast<std::string>(App::ct->objCont->getGraphNumberInSelection(&App::ct->objCont->objectList));
    ui->graphsButton->setText(tmp.c_str());
    tmp=std::string(IDS_MILLS)+": "+boost::lexical_cast<std::string>(App::ct->objCont->getMillNumberInSelection())+"/"+
        boost::lexical_cast<std::string>(App::ct->objCont->getMillNumberInSelection(&App::ct->objCont->objectList));
    ui->millsButton->setText(tmp.c_str());
    tmp=std::string(IDS_PATHS)+": "+boost::lexical_cast<std::string>(App::ct->objCont->getPathNumberInSelection())+"/"+
        boost::lexical_cast<std::string>(App::ct->objCont->getPathNumberInSelection(&App::ct->objCont->objectList));
    ui->pathsButton->setText(tmp.c_str());
    tmp=std::string(IDS_OCTREES)+": "+boost::lexical_cast<std::string>(App::ct->objCont->getOctreeNumberInSelection())+"/"+
        boost::lexical_cast<std::string>(App::ct->objCont->getOctreeNumberInSelection(&App::ct->objCont->objectList));
    ui->octreesButton->setText(tmp.c_str());
    tmp=std::string(IDS_POINTCLOUDS)+": "+boost::lexical_cast<std::string>(App::ct->objCont->getPointCloudNumberInSelection())+"/"+
        boost::lexical_cast<std::string>(App::ct->objCont->getPointCloudNumberInSelection(&App::ct->objCont->objectList));
    ui->pointCloudsButton->setText(tmp.c_str());
    tmp=std::string(IDS_MIRRORS)+": "+boost::lexical_cast<std::string>(App::ct->objCont->getMirrorNumberInSelection())+"/"+
        boost::lexical_cast<std::string>(App::ct->objCont->getMirrorNumberInSelection(&App::ct->objCont->objectList));
    ui->mirrorsButton->setText(tmp.c_str());

    tmp.clear();

    bool objEnabled=App::getEditModeType()==NO_EDIT_MODE;

    ui->clearSelectionButton->setEnabled(objEnabled);
    ui->invertSelectionButton->setEnabled(objEnabled);

    ui->simpleShapesButton->setEnabled(objEnabled);
    ui->jointsButton->setEnabled(objEnabled);
    ui->groupedShapesButton->setEnabled(objEnabled);
    ui->camerasButton->setEnabled(objEnabled);
    ui->lightsButton->setEnabled(objEnabled);
    ui->proximitySensorsButton->setEnabled(objEnabled);
    ui->renderingSensorsButton->setEnabled(objEnabled);
    ui->forceSensorsButton->setEnabled(objEnabled);
    ui->millsButton->setEnabled(objEnabled);
    ui->dummiesButton->setEnabled(objEnabled);
    ui->graphsButton->setEnabled(objEnabled);
    ui->pathsButton->setEnabled(objEnabled);
    ui->mirrorsButton->setEnabled(objEnabled);
    ui->octreesButton->setEnabled(objEnabled);
    ui->pointCloudsButton->setEnabled(objEnabled);


    if ((App::getEditModeType()&SHAPE_OR_PATH_EDIT_MODE)==0)
    {
        tmp=std::string(IDS_TOTAL_SELECTED_OBJECTS)+": "+boost::lexical_cast<std::string>(App::ct->objCont->getSelSize())+"/"+
            boost::lexical_cast<std::string>(App::ct->objCont->objectList.size());
    }
    ui->selectionInfo->setText(tmp.c_str());
}

void CQDlgSelection::on_simpleShapesButton_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        list.clear();
        for (size_t i=0;i<App::ct->objCont->shapeList.size();i++)
        {
            CShape* it=App::ct->objCont->getShape(App::ct->objCont->shapeList[i]);
            if (!it->isCompound())
                list.push_back(it->getObjectHandle());
        }
        processIt();
    }
}

void CQDlgSelection::on_groupedShapesButton_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        list.clear();
        for (size_t i=0;i<App::ct->objCont->shapeList.size();i++)
        {
            CShape* it=App::ct->objCont->getShape(App::ct->objCont->shapeList[i]);
            if (it->isCompound())
                list.push_back(it->getObjectHandle());
        }
        processIt();
    }
}

void CQDlgSelection::on_dummiesButton_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        list.clear();
        for (size_t i=0;i<App::ct->objCont->dummyList.size();i++)
            list.push_back(App::ct->objCont->dummyList[i]);
        processIt();
    }
}

void CQDlgSelection::on_millsButton_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        list.clear();
        for (size_t i=0;i<App::ct->objCont->millList.size();i++)
            list.push_back(App::ct->objCont->millList[i]);
        processIt();
    }
}

void CQDlgSelection::on_camerasButton_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        list.clear();
        for (size_t i=0;i<App::ct->objCont->cameraList.size();i++)
            list.push_back(App::ct->objCont->cameraList[i]);
        processIt();
    }
}

void CQDlgSelection::on_lightsButton_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        list.clear();
        for (size_t i=0;i<App::ct->objCont->lightList.size();i++)
            list.push_back(App::ct->objCont->lightList[i]);
        processIt();
    }
}

void CQDlgSelection::on_proximitySensorsButton_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        list.clear();
        for (size_t i=0;i<App::ct->objCont->proximitySensorList.size();i++)
            list.push_back(App::ct->objCont->proximitySensorList[i]);
        processIt();
    }
}

void CQDlgSelection::on_renderingSensorsButton_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        list.clear();
        for (size_t i=0;i<App::ct->objCont->visionSensorList.size();i++)
            list.push_back(App::ct->objCont->visionSensorList[i]);
        processIt();
    }
}

void CQDlgSelection::on_forceSensorsButton_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        list.clear();
        for (size_t i=0;i<App::ct->objCont->forceSensorList.size();i++)
            list.push_back(App::ct->objCont->forceSensorList[i]);
        processIt();
    }
}

void CQDlgSelection::on_jointsButton_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        list.clear();
        for (size_t i=0;i<App::ct->objCont->jointList.size();i++)
            list.push_back(App::ct->objCont->jointList[i]);
        processIt();
    }
}

void CQDlgSelection::on_graphsButton_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        list.clear();
        for (size_t i=0;i<App::ct->objCont->graphList.size();i++)
            list.push_back(App::ct->objCont->graphList[i]);
        processIt();
    }
}

void CQDlgSelection::on_pathsButton_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        list.clear();
        for (size_t i=0;i<App::ct->objCont->pathList.size();i++)
            list.push_back(App::ct->objCont->pathList[i]);
        processIt();
    }
}

void CQDlgSelection::on_mirrorsButton_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        list.clear();
        for (size_t i=0;i<App::ct->objCont->mirrorList.size();i++)
            list.push_back(App::ct->objCont->mirrorList[i]);
        processIt();
    }
}

void CQDlgSelection::on_octreesButton_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        list.clear();
        for (size_t i=0;i<App::ct->objCont->octreeList.size();i++)
            list.push_back(App::ct->objCont->octreeList[i]);
        processIt();
    }
}

void CQDlgSelection::on_pointCloudsButton_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        list.clear();
        for (size_t i=0;i<App::ct->objCont->pointCloudList.size();i++)
            list.push_back(App::ct->objCont->pointCloudList[i]);
        processIt();
    }
}

void CQDlgSelection::processIt()
{
    SSimulationThreadCommand cmd;
    cmd.cmdId=ADD_OBJECTS_TO_SELECTION_GUITRIGGEREDCMD;
    cmd.intParams.assign(list.begin(),list.end());
    App::appendSimulationThreadCommand(cmd);
    App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
}

void CQDlgSelection::on_clearSelectionButton_clicked()
{
    App::appendSimulationThreadCommand(SET_OBJECT_SELECTION_GUITRIGGEREDCMD);
    App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
}

void CQDlgSelection::on_invertSelectionButton_clicked()
{
    if ((App::getEditModeType()&SHAPE_OR_PATH_EDIT_MODE)==0)
    {
        App::appendSimulationThreadCommand(INVERT_SELECTION_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

