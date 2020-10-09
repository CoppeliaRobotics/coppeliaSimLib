#include "qdlgselection.h"
#include "ui_qdlgselection.h"
#include "app.h"
#include "simStrings.h"
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
    tmp=std::string(IDS_SIMPLE_SHAPES)+": "+boost::lexical_cast<std::string>(App::currentWorld->sceneObjects->getSimpleShapeCountInSelection())+"/"+
        boost::lexical_cast<std::string>(App::currentWorld->sceneObjects->getSimpleShapeCount());
    ui->simpleShapesButton->setText(tmp.c_str());
    tmp=std::string(IDS_COMPOUND_SHAPES)+": "+boost::lexical_cast<std::string>(App::currentWorld->sceneObjects->getCompoundCountInSelection())+"/"+
        boost::lexical_cast<std::string>(App::currentWorld->sceneObjects->getCompoundShapeCount());
    ui->groupedShapesButton->setText(tmp.c_str());
    tmp=std::string(IDS_JOINTS)+": "+boost::lexical_cast<std::string>(App::currentWorld->sceneObjects->getJointCountInSelection())+"/"+
        boost::lexical_cast<std::string>(App::currentWorld->sceneObjects->getJointCount());
    ui->jointsButton->setText(tmp.c_str());
    tmp=std::string(IDS_CAMERAS)+": "+boost::lexical_cast<std::string>(App::currentWorld->sceneObjects->getCameraCountInSelection())+"/"+
        boost::lexical_cast<std::string>(App::currentWorld->sceneObjects->getCameraCount());
    ui->camerasButton->setText(tmp.c_str());
    tmp=std::string(IDS_LIGHTS)+": "+boost::lexical_cast<std::string>(App::currentWorld->sceneObjects->getLightCountInSelection())+"/"+
        boost::lexical_cast<std::string>(App::currentWorld->sceneObjects->getLightCount());
    ui->lightsButton->setText(tmp.c_str());
    tmp=std::string(IDS_PROXSENSORS)+": "+boost::lexical_cast<std::string>(App::currentWorld->sceneObjects->getProxSensorCountInSelection())+"/"+
        boost::lexical_cast<std::string>(App::currentWorld->sceneObjects->getProximitySensorCount());
    ui->proximitySensorsButton->setText(tmp.c_str());
    tmp=std::string(IDS_VISION_SENSORS)+": "+boost::lexical_cast<std::string>(App::currentWorld->sceneObjects->getVisionSensorCountInSelection())+"/"+
        boost::lexical_cast<std::string>(App::currentWorld->sceneObjects->getVisionSensorCount());
    ui->renderingSensorsButton->setText(tmp.c_str());
    tmp=std::string(IDS_FORCE_SENSORS)+": "+boost::lexical_cast<std::string>(App::currentWorld->sceneObjects->getForceSensorCountInSelection())+"/"+
        boost::lexical_cast<std::string>(App::currentWorld->sceneObjects->getForceSensorCount());
    ui->forceSensorsButton->setText(tmp.c_str());
    tmp=std::string(IDS_DUMMIES)+": "+boost::lexical_cast<std::string>(App::currentWorld->sceneObjects->getDummyCountInSelection())+"/"+
        boost::lexical_cast<std::string>(App::currentWorld->sceneObjects->getDummyCount());
    ui->dummiesButton->setText(tmp.c_str());
    tmp=std::string(IDS_GRAPHS)+": "+boost::lexical_cast<std::string>(App::currentWorld->sceneObjects->getGraphCountInSelection())+"/"+
        boost::lexical_cast<std::string>(App::currentWorld->sceneObjects->getGraphCount());
    ui->graphsButton->setText(tmp.c_str());
    tmp=std::string(IDS_PATHS)+": "+boost::lexical_cast<std::string>(App::currentWorld->sceneObjects->getPathCountInSelection())+"/"+
        boost::lexical_cast<std::string>(App::currentWorld->sceneObjects->getPathCount());
    ui->pathsButton->setText(tmp.c_str());
    ui->pathsButton->setVisible(App::userSettings->showOldDlgs);
    tmp=std::string(IDS_OCTREES)+": "+boost::lexical_cast<std::string>(App::currentWorld->sceneObjects->getOctreeCountInSelection())+"/"+
        boost::lexical_cast<std::string>(App::currentWorld->sceneObjects->getOctreeCount());
    ui->octreesButton->setText(tmp.c_str());
    tmp=std::string(IDS_POINTCLOUDS)+": "+boost::lexical_cast<std::string>(App::currentWorld->sceneObjects->getPointCloudCountInSelection())+"/"+
        boost::lexical_cast<std::string>(App::currentWorld->sceneObjects->getPointCloudCount());
    ui->pointCloudsButton->setText(tmp.c_str());

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
    ui->dummiesButton->setEnabled(objEnabled);
    ui->graphsButton->setEnabled(objEnabled);
    ui->pathsButton->setEnabled(objEnabled);
    ui->octreesButton->setEnabled(objEnabled);
    ui->pointCloudsButton->setEnabled(objEnabled);


    if ((App::getEditModeType()&SHAPE_OR_PATH_EDIT_MODE)==0)
    {
        tmp=std::string(IDS_TOTAL_SELECTED_OBJECTS)+": "+boost::lexical_cast<std::string>(App::currentWorld->sceneObjects->getSelectionCount())+"/"+
            boost::lexical_cast<std::string>(App::currentWorld->sceneObjects->getObjectCount());
    }
    ui->selectionInfo->setText(tmp.c_str());
}

void CQDlgSelection::on_simpleShapesButton_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        list.clear();
        for (size_t i=0;i<App::currentWorld->sceneObjects->getShapeCount();i++)
        {
            CShape* it=App::currentWorld->sceneObjects->getShapeFromIndex(i);
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
        for (size_t i=0;i<App::currentWorld->sceneObjects->getShapeCount();i++)
        {
            CShape* it=App::currentWorld->sceneObjects->getShapeFromIndex(i);
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
        for (size_t i=0;i<App::currentWorld->sceneObjects->getDummyCount();i++)
            list.push_back(App::currentWorld->sceneObjects->getDummyFromIndex(i)->getObjectHandle());
        processIt();
    }
}

void CQDlgSelection::on_camerasButton_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        list.clear();
        for (size_t i=0;i<App::currentWorld->sceneObjects->getCameraCount();i++)
            list.push_back(App::currentWorld->sceneObjects->getCameraFromIndex(i)->getObjectHandle());
        processIt();
    }
}

void CQDlgSelection::on_lightsButton_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        list.clear();
        for (size_t i=0;i<App::currentWorld->sceneObjects->getLightCount();i++)
            list.push_back(App::currentWorld->sceneObjects->getLightFromIndex(i)->getObjectHandle());
        processIt();
    }
}

void CQDlgSelection::on_proximitySensorsButton_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        list.clear();
        for (size_t i=0;i<App::currentWorld->sceneObjects->getProximitySensorCount();i++)
            list.push_back(App::currentWorld->sceneObjects->getProximitySensorFromIndex(i)->getObjectHandle());
        processIt();
    }
}

void CQDlgSelection::on_renderingSensorsButton_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        list.clear();
        for (size_t i=0;i<App::currentWorld->sceneObjects->getVisionSensorCount();i++)
            list.push_back(App::currentWorld->sceneObjects->getVisionSensorFromIndex(i)->getObjectHandle());
        processIt();
    }
}

void CQDlgSelection::on_forceSensorsButton_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        list.clear();
        for (size_t i=0;i<App::currentWorld->sceneObjects->getForceSensorCount();i++)
            list.push_back(App::currentWorld->sceneObjects->getForceSensorFromIndex(i)->getObjectHandle());
        processIt();
    }
}

void CQDlgSelection::on_jointsButton_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        list.clear();
        for (size_t i=0;i<App::currentWorld->sceneObjects->getJointCount();i++)
            list.push_back(App::currentWorld->sceneObjects->getJointFromIndex(i)->getObjectHandle());
        processIt();
    }
}

void CQDlgSelection::on_graphsButton_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        list.clear();
        for (size_t i=0;i<App::currentWorld->sceneObjects->getGraphCount();i++)
            list.push_back(App::currentWorld->sceneObjects->getGraphFromIndex(i)->getObjectHandle());
        processIt();
    }
}

void CQDlgSelection::on_pathsButton_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        list.clear();
        for (size_t i=0;i<App::currentWorld->sceneObjects->getPathCount();i++)
            list.push_back(App::currentWorld->sceneObjects->getPathFromIndex(i)->getObjectHandle());
        processIt();
    }
}

void CQDlgSelection::on_octreesButton_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        list.clear();
        for (size_t i=0;i<App::currentWorld->sceneObjects->getOctreeCount();i++)
            list.push_back(App::currentWorld->sceneObjects->getOctreeFromIndex(i)->getObjectHandle());
        processIt();
    }
}

void CQDlgSelection::on_pointCloudsButton_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        list.clear();
        for (size_t i=0;i<App::currentWorld->sceneObjects->getPointCloudCount();i++)
            list.push_back(App::currentWorld->sceneObjects->getPointCloudFromIndex(i)->getObjectHandle());
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

