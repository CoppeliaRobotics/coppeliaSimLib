#include <qdlgpointclouds.h>
#include <ui_qdlgpointclouds.h>
#include <tt.h>
#include <utils.h>
#include <qdlgmaterial.h>
#include <app.h>
#include <simStrings.h>
#include <sceneObjectOperations.h>
#include <guiApp.h>

CQDlgPointclouds::CQDlgPointclouds(QWidget *parent) :
    CDlgEx(parent),
    ui(new Ui::CQDlgPointclouds)
{
    ui->setupUi(this);
}

CQDlgPointclouds::~CQDlgPointclouds()
{
    delete ui;
}

void CQDlgPointclouds::cancelEvent()
{
    // we override this cancel event. The container window should close, not this one!!
    GuiApp::mainWindow->dlgCont->close(OBJECT_DLG);
}

void CQDlgPointclouds::refresh()
{
    QLineEdit* lineEditToSelect=getSelectedLineEdit();
    bool noEditMode=GuiApp::getEditModeType()==NO_EDIT_MODE;
    bool noEditModeAndNoSim=noEditMode&&App::currentWorld->simulation->isSimulationStopped();

    bool sel=App::currentWorld->sceneObjects->isLastSelectionAPointCloud();
    size_t objCnt=App::currentWorld->sceneObjects->getSelectionCount();
    bool octreeStruct=false;
    CPointCloud* it=App::currentWorld->sceneObjects->getLastSelectionPointCloud();
    if (sel)
        octreeStruct=!it->getDoNotUseCalculationStructure();

    ui->qqNoOctreeStructure->setEnabled(sel&&noEditModeAndNoSim);
    ui->qqMaxCellSize->setEnabled(sel&&noEditModeAndNoSim&&octreeStruct);
    ui->qqMaxPointCount->setEnabled(sel&&noEditModeAndNoSim&&octreeStruct);
    ui->qqColor->setEnabled(sel&&noEditModeAndNoSim);
    ui->qqShowOctree->setEnabled(sel&&noEditModeAndNoSim&&octreeStruct);
    ui->qqRandomColors->setEnabled(sel&&noEditModeAndNoSim);
    ui->qqPointSize->setEnabled(sel&&noEditModeAndNoSim);
    ui->qqClear->setEnabled(sel&&noEditModeAndNoSim);
    ui->qqInsert->setEnabled(sel&&noEditModeAndNoSim&&(objCnt>1));
    ui->qqSubtract->setEnabled(sel&&noEditModeAndNoSim&&octreeStruct&&(objCnt>1));
    ui->qqBuildResolution->setEnabled(sel&&noEditModeAndNoSim);
    ui->qqSubtractionTolerance->setEnabled(sel&&noEditModeAndNoSim&&octreeStruct);
    ui->qqInsertionTolerance->setEnabled(sel&&noEditModeAndNoSim&&octreeStruct);
    ui->qqEmissiveColor->setEnabled(sel&&noEditModeAndNoSim);
    ui->qqDisplayRatio->setEnabled(sel&&noEditModeAndNoSim&&octreeStruct);

    if (sel)
    {
        ui->qqNoOctreeStructure->setChecked(!octreeStruct);
        ui->qqMaxCellSize->setText(utils::getSizeString(false,it->getCellSize()).c_str());
        ui->qqMaxPointCount->setText(utils::getIntString(false,it->getMaxPointCountPerCell()).c_str());
        ui->qqShowOctree->setChecked(it->getShowOctree());
        ui->qqRandomColors->setChecked(it->getUseRandomColors());
        ui->qqPointSize->setText(utils::getIntString(false,it->getPointSize()).c_str());
        ui->qqPointCount->setText(utils::getIntString(false,(int)it->getPoints()->size()/3).c_str());
        if (it->getAveragePointCountInCell()<0.0)
            ui->qqAveragePoints->setText("-"); // empty point cloud or point cloud doesn't use octree struct.
        else
            ui->qqAveragePoints->setText(utils::get0To1String(false,it->getAveragePointCountInCell()).c_str());
        ui->qqBuildResolution->setText(utils::getSizeString(false,it->getBuildResolution()).c_str());
        ui->qqInsertionTolerance->setText(utils::getSizeString(false,it->getInsertionDistanceTolerance()).c_str());
        ui->qqSubtractionTolerance->setText(utils::getSizeString(false,it->getRemovalDistanceTolerance()).c_str());
        ui->qqEmissiveColor->setChecked(it->getColorIsEmissive());
        ui->qqDisplayRatio->setText(utils::get0To1String(false,it->getPointDisplayRatio()).c_str());
    }
    else
    {
        ui->qqNoOctreeStructure->setChecked(false);
        ui->qqMaxCellSize->setText("");
        ui->qqMaxPointCount->setText("");
        ui->qqShowOctree->setChecked(false);
        ui->qqRandomColors->setChecked(false);
        ui->qqPointSize->setText("");
        ui->qqPointCount->setText("");
        ui->qqAveragePoints->setText("");
        ui->qqBuildResolution->setText("");
        ui->qqInsertionTolerance->setText("");
        ui->qqSubtractionTolerance->setText("");
        ui->qqEmissiveColor->setChecked(false);
        ui->qqDisplayRatio->setText("");
    }
    selectLineEdit(lineEditToSelect);
}

void CQDlgPointclouds::on_qqMaxCellSize_editingFinished()
{
    if (!ui->qqMaxCellSize->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        double newVal=ui->qqMaxCellSize->text().toDouble(&ok);
        if (ok)
        {
            GuiApp::appendSimulationThreadCommand(SET_MAXVOXELSIZE_PTCLOUDGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle(),-1,newVal);
            GuiApp::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        GuiApp::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgPointclouds::on_qqColor_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CQDlgColor::displayDlg(COLOR_ID_POINTCLOUD,App::currentWorld->sceneObjects->getLastSelectionHandle(),-1,0,GuiApp::mainWindow);
    }
}

void CQDlgPointclouds::on_qqShowOctree_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        GuiApp::appendSimulationThreadCommand(TOGGLE_SHOWOCTREE_PTCLOUDGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle());
        GuiApp::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        GuiApp::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgPointclouds::on_qqRandomColors_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        GuiApp::appendSimulationThreadCommand(TOGGLE_RANDOMCOLORS_PTCLOUDGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle());
        GuiApp::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        GuiApp::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgPointclouds::on_qqPointSize_editingFinished()
{
    if (!ui->qqPointSize->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        int newVal=ui->qqPointSize->text().toInt(&ok);
        if (ok)
        {
            GuiApp::appendSimulationThreadCommand(SET_PTSIZE_PTCLOUDGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle(),newVal);
            GuiApp::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        GuiApp::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgPointclouds::on_qqClear_clicked()
{
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        GuiApp::appendSimulationThreadCommand(CLEAR_PTCLOUDGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle());
        GuiApp::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        GuiApp::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgPointclouds::on_qqInsert_clicked()
{
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        CPointCloud* it=App::currentWorld->sceneObjects->getLastSelectionPointCloud();
        if (it!=nullptr)
        {
            SSimulationThreadCommand cmd;
            cmd.cmdId=INSERT_OBJECTS_PTCLOUDGUITRIGGEREDCMD;
            GuiApp::appendSimulationThreadCommand(cmd);
            GuiApp::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgPointclouds::on_qqMaxPointCount_editingFinished()
{
    if (!ui->qqMaxPointCount->isModified())
        return;
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        bool ok;
        int newVal=ui->qqMaxPointCount->text().toInt(&ok);
        if (ok)
        {
            GuiApp::appendSimulationThreadCommand(SET_MAXPTCNT_PTCLOUDGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle(),newVal);
            GuiApp::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        GuiApp::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgPointclouds::on_qqBuildResolution_editingFinished()
{
    if (!ui->qqBuildResolution->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        double newVal=ui->qqBuildResolution->text().toDouble(&ok);
        if (ok)
        {
            GuiApp::appendSimulationThreadCommand(SET_BUILDRESOLUTION_PTCLOUDGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle(),-1,newVal);
            GuiApp::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        GuiApp::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgPointclouds::on_qqNoOctreeStructure_clicked()
{
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        CPointCloud* it=App::currentWorld->sceneObjects->getLastSelectionPointCloud();
        if (it!=nullptr)
        {
            if (!it->getDoNotUseCalculationStructure())
                GuiApp::uiThread->messageBox_warning(GuiApp::mainWindow,"Point cloud octree calculation structure","Be aware that when disabling the octree calculation structure, your point cloud will not be collidable, measurable nor detectable anymore. Also, some functionality might be limited in that case.",VMESSAGEBOX_OKELI,VMESSAGEBOX_REPLY_OK);
            GuiApp::appendSimulationThreadCommand(TOGGLE_USEOCTREE_PTCLOUDGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle());
            GuiApp::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            GuiApp::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgPointclouds::on_qqEmissiveColor_clicked()
{
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        GuiApp::appendSimulationThreadCommand(TOGGLE_EMISSIVECOLOR_PTCLOUDGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle());
        GuiApp::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        GuiApp::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgPointclouds::on_qqDisplayRatio_editingFinished()
{
    if (!ui->qqDisplayRatio->isModified())
        return;
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        bool ok;
        double newVal=ui->qqDisplayRatio->text().toDouble(&ok);
        if (ok)
        {
            GuiApp::appendSimulationThreadCommand(SET_DISPLAYRATIO_PTCLOUDGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle(),-1,newVal);
            GuiApp::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        GuiApp::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgPointclouds::on_qqSubtract_clicked()
{
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        CPointCloud* it=App::currentWorld->sceneObjects->getLastSelectionPointCloud();
        if (it!=nullptr)
        {
            SSimulationThreadCommand cmd;
            cmd.cmdId=SUBTRACT_OBJECTS_PTCLOUDGUITRIGGEREDCMD;
            GuiApp::appendSimulationThreadCommand(cmd);
            GuiApp::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgPointclouds::on_qqSubtractionTolerance_editingFinished()
{
    if (!ui->qqSubtractionTolerance->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        double newVal=ui->qqSubtractionTolerance->text().toDouble(&ok);
        if (ok)
        {
            GuiApp::appendSimulationThreadCommand(SET_SUBTRACTTOLERANCE_PTCLOUDGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle(),-1,newVal);
            GuiApp::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        GuiApp::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgPointclouds::on_qqInsertionTolerance_editingFinished()
{
    if (!ui->qqInsertionTolerance->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        double newVal=ui->qqInsertionTolerance->text().toDouble(&ok);
        if (ok)
        {
            GuiApp::appendSimulationThreadCommand(SET_INSERTTOLERANCE_PTCLOUDGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle(),-1,newVal);
            GuiApp::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        GuiApp::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}
