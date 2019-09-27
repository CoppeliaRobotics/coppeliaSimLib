
#include "vrepMainHeader.h"
#include "qdlgoctrees.h"
#include "ui_qdlgoctrees.h"
#include "tt.h"
#include "gV.h"
#include "qdlgmaterial.h"
#include "app.h"
#include "v_repStrings.h"
#include "sceneObjectOperations.h"

CQDlgOctrees::CQDlgOctrees(QWidget *parent) :
    CDlgEx(parent),
    ui(new Ui::CQDlgOctrees)
{
    ui->setupUi(this);
}

CQDlgOctrees::~CQDlgOctrees()
{
    delete ui;
}

void CQDlgOctrees::cancelEvent()
{
    // we override this cancel event. The container window should close, not this one!!
    App::mainWindow->dlgCont->close(OBJECT_DLG);
}

void CQDlgOctrees::refresh()
{
    QLineEdit* lineEditToSelect=getSelectedLineEdit();
    bool noEditMode=App::getEditModeType()==NO_EDIT_MODE;
    bool noEditModeAndNoSim=noEditMode&&App::ct->simulation->isSimulationStopped();

    bool sel=App::ct->objCont->isLastSelectionAnOctree();
    int objCnt=App::ct->objCont->getSelSize();
    COctree* it=App::ct->objCont->getLastSelection_octree();

    ui->qqSize->setEnabled(sel&&noEditModeAndNoSim);
    ui->qqColor->setEnabled(sel&&noEditModeAndNoSim);
    ui->qqShowOctree->setEnabled(sel&&noEditModeAndNoSim);
    ui->qqRandomColors->setEnabled(sel&&noEditModeAndNoSim);
    ui->qqUsePoints->setEnabled(sel&&noEditModeAndNoSim);
    ui->qqPointSize->setEnabled(sel&&noEditModeAndNoSim&&it->getUsePointsInsteadOfCubes());
    ui->qqClear->setEnabled(sel&&noEditModeAndNoSim);
    ui->qqInsert->setEnabled(sel&&noEditModeAndNoSim&&(objCnt>1));
    ui->qqSubtract->setEnabled(sel&&noEditModeAndNoSim&&(objCnt>1));
    ui->qqEmissiveColor->setEnabled(sel&&noEditModeAndNoSim);

    if (sel)
    {
        ui->qqSize->setText(tt::getFString(false,it->getCellSize(),4).c_str());
        ui->qqShowOctree->setChecked(it->getShowOctree());
        ui->qqRandomColors->setChecked(it->getUseRandomColors());
        ui->qqCellCount->setText(tt::getIString(false,(int)it->getCubePositions()->size()/3).c_str());
        ui->qqUsePoints->setChecked(it->getUsePointsInsteadOfCubes());
        ui->qqPointSize->setText(tt::getIString(false,it->getPointSize()).c_str());
        ui->qqEmissiveColor->setChecked(it->getColorIsEmissive());
    }
    else
    {
        ui->qqSize->setText("");
        ui->qqShowOctree->setChecked(false);
        ui->qqRandomColors->setChecked(false);
        ui->qqCellCount->setText("");
        ui->qqUsePoints->setChecked(false);
        ui->qqPointSize->setText("");
        ui->qqEmissiveColor->setChecked(false);
    }
    selectLineEdit(lineEditToSelect);
}

void CQDlgOctrees::on_qqSize_editingFinished()
{
    if (!ui->qqSize->isModified())
        return;
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        bool ok;
        float newVal=ui->qqSize->text().toFloat(&ok);
        if (ok)
        {
            App::appendSimulationThreadCommand(SET_VOXELSIZE_OCTREEGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),-1,newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgOctrees::on_qqColor_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CQDlgColor::displayDlg(COLOR_ID_OCTREE,App::ct->objCont->getLastSelectionID(),-1,0,App::mainWindow);
    }
}

void CQDlgOctrees::on_qqShowOctree_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_SHOWSTRUCTURE_OCTREEGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgOctrees::on_qqRandomColors_clicked()
{
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_RANDOMCOLORS_OCTREEGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgOctrees::on_qqUsePoints_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_SHOWPOINTS_OCTREEGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgOctrees::on_qqPointSize_editingFinished()
{
    if (!ui->qqPointSize->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        int newVal=ui->qqPointSize->text().toInt(&ok);
        if (ok)
        {
            App::appendSimulationThreadCommand(SET_POINTSIZE_OCTREEGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgOctrees::on_qqClear_clicked()
{
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        App::appendSimulationThreadCommand(CLEAR_OCTREEGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgOctrees::on_qqInsert_clicked()
{
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        COctree* it=App::ct->objCont->getLastSelection_octree();
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
            App::appendSimulationThreadCommand(SHOW_PROGRESSDLGGUITRIGGEREDCMD,-1,-1,0.0,0.0,"Inserting object(s) into octree...");
            SSimulationThreadCommand cmd;
            cmd.cmdId=INSERT_SELECTEDVISIBLEOBJECTS_OCTREEGUITRIGGEREDCMD;
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

void CQDlgOctrees::on_qqSubtract_clicked()
{
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        COctree* it=App::ct->objCont->getLastSelection_octree();
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
            App::appendSimulationThreadCommand(SHOW_PROGRESSDLGGUITRIGGEREDCMD,-1,-1,0.0,0.0,"Subtracting object(s) from octree...");
            SSimulationThreadCommand cmd;
            cmd.cmdId=SUBTRACT_SELECTEDVISIBLEOBJECTS_OCTREEGUITRIGGEREDCMD;
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

void CQDlgOctrees::on_qqEmissiveColor_clicked()
{
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_COLOREMISSIVE_OCTREEGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}
