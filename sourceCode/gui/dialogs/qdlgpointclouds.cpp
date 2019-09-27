
#include "vrepMainHeader.h"
#include "qdlgpointclouds.h"
#include "ui_qdlgpointclouds.h"
#include "tt.h"
#include "gV.h"
#include "qdlgmaterial.h"
#include "app.h"
#include "v_repStrings.h"
#include "sceneObjectOperations.h"

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
    App::mainWindow->dlgCont->close(OBJECT_DLG);
}

void CQDlgPointclouds::refresh()
{
    QLineEdit* lineEditToSelect=getSelectedLineEdit();
    bool noEditMode=App::getEditModeType()==NO_EDIT_MODE;
    bool noEditModeAndNoSim=noEditMode&&App::ct->simulation->isSimulationStopped();

    bool sel=App::ct->objCont->isLastSelectionAPointCloud();
    int objCnt=App::ct->objCont->getSelSize();
    bool octreeStruct=false;
    CPointCloud* it=App::ct->objCont->getLastSelection_pointCloud();
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
        ui->qqMaxCellSize->setText(tt::getFString(false,it->getCellSize(),4).c_str());
        ui->qqMaxPointCount->setText(tt::getIString(false,it->getMaxPointCountPerCell()).c_str());
        ui->qqShowOctree->setChecked(it->getShowOctree());
        ui->qqRandomColors->setChecked(it->getUseRandomColors());
        ui->qqPointSize->setText(tt::getIString(false,it->getPointSize()).c_str());
        ui->qqPointCount->setText(tt::getIString(false,(int)it->getPoints()->size()/3).c_str());
        if (it->getAveragePointCountInCell()<0.0)
            ui->qqAveragePoints->setText("-"); // empty point cloud or point cloud doesn't use octree struct.
        else
            ui->qqAveragePoints->setText(tt::getFString(false,it->getAveragePointCountInCell(),1).c_str());
        ui->qqBuildResolution->setText(tt::getFString(false,it->getBuildResolution(),4).c_str());
        ui->qqInsertionTolerance->setText(tt::getFString(false,it->getInsertionDistanceTolerance(),4).c_str());
        ui->qqSubtractionTolerance->setText(tt::getFString(false,it->getRemovalDistanceTolerance(),4).c_str());
        ui->qqEmissiveColor->setChecked(it->getColorIsEmissive());
        ui->qqDisplayRatio->setText(tt::getFString(false,it->getPointDisplayRatio(),2).c_str());
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
        float newVal=ui->qqMaxCellSize->text().toFloat(&ok);
        if (ok)
        {
            App::appendSimulationThreadCommand(SET_MAXVOXELSIZE_PTCLOUDGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),-1,newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgPointclouds::on_qqColor_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CQDlgColor::displayDlg(COLOR_ID_POINTCLOUD,App::ct->objCont->getLastSelectionID(),-1,0,App::mainWindow);
    }
}

void CQDlgPointclouds::on_qqShowOctree_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_SHOWOCTREE_PTCLOUDGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgPointclouds::on_qqRandomColors_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_RANDOMCOLORS_PTCLOUDGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
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
            App::appendSimulationThreadCommand(SET_PTSIZE_PTCLOUDGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgPointclouds::on_qqClear_clicked()
{
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        App::appendSimulationThreadCommand(CLEAR_PTCLOUDGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgPointclouds::on_qqInsert_clicked()
{
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        CPointCloud* it=App::ct->objCont->getLastSelection_pointCloud();
        if (it!=nullptr)
        {
            std::vector<int> sel;
            for (int i=0;i<App::ct->objCont->getSelSize()-1;i++)
                sel.push_back(App::ct->objCont->getSelID(i));
            CSceneObjectOperations::addRootObjectChildrenToSelection(sel);

            // Now keep only visible objects:
            std::vector<int> sel2;
            for (size_t i=0;i<sel.size();i++)
            {
                C3DObject* it=App::ct->objCont->getObjectFromHandle(sel[i]);
                if ( (!it->isObjectPartOfInvisibleModel())&&(App::ct->mainSettings->getActiveLayers()&it->layer) )
                    sel2.push_back(sel[i]);
            }
            App::appendSimulationThreadCommand(SHOW_PROGRESSDLGGUITRIGGEREDCMD,-1,-1,0.0,0.0,"Inserting object(s) into point cloud...");
            SSimulationThreadCommand cmd;
            cmd.cmdId=INSERT_OBJECTS_PTCLOUDGUITRIGGEREDCMD;
            cmd.intParams.push_back(App::ct->objCont->getLastSelectionID());
            for (size_t i=0;i<sel2.size();i++)
                cmd.intParams.push_back(sel2[i]);
            App::appendSimulationThreadCommand(cmd);
            App::appendSimulationThreadCommand(HIDE_PROGRESSDLGGUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(SET_OBJECT_SELECTION_GUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID());
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
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
            App::appendSimulationThreadCommand(SET_MAXPTCNT_PTCLOUDGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgPointclouds::on_qqBuildResolution_editingFinished()
{
    if (!ui->qqBuildResolution->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        float newVal=ui->qqBuildResolution->text().toFloat(&ok);
        if (ok)
        {
            App::appendSimulationThreadCommand(SET_BUILDRESOLUTION_PTCLOUDGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),-1,newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgPointclouds::on_qqNoOctreeStructure_clicked()
{
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        CPointCloud* it=App::ct->objCont->getLastSelection_pointCloud();
        if (it!=nullptr)
        {
            if (!it->getDoNotUseCalculationStructure())
                App::uiThread->messageBox_warning(App::mainWindow,"Point cloud octree calculation structure","Be aware that when disabling the octree calculation structure, your point cloud will not be collidable, measurable nor detectable anymore. Also, some functionality might be limited in that case.",VMESSAGEBOX_OKELI);
            App::appendSimulationThreadCommand(TOGGLE_USEOCTREE_PTCLOUDGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID());
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgPointclouds::on_qqEmissiveColor_clicked()
{
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_EMISSIVECOLOR_PTCLOUDGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgPointclouds::on_qqDisplayRatio_editingFinished()
{
    if (!ui->qqDisplayRatio->isModified())
        return;
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        bool ok;
        float newVal=ui->qqDisplayRatio->text().toFloat(&ok);
        if (ok)
        {
            App::appendSimulationThreadCommand(SET_DISPLAYRATIO_PTCLOUDGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),-1,newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgPointclouds::on_qqSubtract_clicked()
{
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        CPointCloud* it=App::ct->objCont->getLastSelection_pointCloud();
        if (it!=nullptr)
        {
            std::vector<int> sel;
            for (int i=0;i<App::ct->objCont->getSelSize()-1;i++)
                sel.push_back(App::ct->objCont->getSelID(i));
            CSceneObjectOperations::addRootObjectChildrenToSelection(sel);

            // Now keep only visible objects:
            std::vector<int> sel2;
            for (size_t i=0;i<sel.size();i++)
            {
                C3DObject* it=App::ct->objCont->getObjectFromHandle(sel[i]);
                if ( (!it->isObjectPartOfInvisibleModel())&&(App::ct->mainSettings->getActiveLayers()&it->layer) )
                    sel2.push_back(sel[i]);
            }
            App::appendSimulationThreadCommand(SHOW_PROGRESSDLGGUITRIGGEREDCMD,-1,-1,0.0,0.0,"Subtracting object(s) from point cloud...");
            SSimulationThreadCommand cmd;
            cmd.cmdId=SUBTRACT_OBJECTS_PTCLOUDGUITRIGGEREDCMD;
            cmd.intParams.push_back(App::ct->objCont->getLastSelectionID());
            for (size_t i=0;i<sel2.size();i++)
                cmd.intParams.push_back(sel2[i]);
            App::appendSimulationThreadCommand(cmd);
            App::appendSimulationThreadCommand(HIDE_PROGRESSDLGGUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(SET_OBJECT_SELECTION_GUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID());
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
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
        float newVal=ui->qqSubtractionTolerance->text().toFloat(&ok);
        if (ok)
        {
            App::appendSimulationThreadCommand(SET_SUBTRACTTOLERANCE_PTCLOUDGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),-1,newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgPointclouds::on_qqInsertionTolerance_editingFinished()
{
    if (!ui->qqInsertionTolerance->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        float newVal=ui->qqInsertionTolerance->text().toFloat(&ok);
        if (ok)
        {
            App::appendSimulationThreadCommand(SET_INSERTTOLERANCE_PTCLOUDGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),-1,newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}
