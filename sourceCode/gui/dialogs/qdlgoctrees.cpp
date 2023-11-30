#include <qdlgoctrees.h>
#include <ui_qdlgoctrees.h>
#include <tt.h>
#include <utils.h>
#include <qdlgmaterial.h>
#include <app.h>
#include <simStrings.h>
#include <sceneObjectOperations.h>
#include <guiApp.h>

CQDlgOctrees::CQDlgOctrees(QWidget *parent) : CDlgEx(parent), ui(new Ui::CQDlgOctrees)
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
    GuiApp::mainWindow->dlgCont->close(OBJECT_DLG);
}

void CQDlgOctrees::refresh()
{
    QLineEdit *lineEditToSelect = getSelectedLineEdit();
    bool noEditMode = GuiApp::getEditModeType() == NO_EDIT_MODE;
    bool noEditModeAndNoSim = noEditMode && App::currentWorld->simulation->isSimulationStopped();

    bool sel = App::currentWorld->sceneObjects->isLastSelectionAnOctree();
    size_t objCnt = App::currentWorld->sceneObjects->getSelectionCount();
    COcTree *it = App::currentWorld->sceneObjects->getLastSelectionOctree();

    ui->qqSize->setEnabled(sel && noEditModeAndNoSim);
    ui->qqColor->setEnabled(sel && noEditModeAndNoSim);
    ui->qqShowOctree->setEnabled(sel && noEditModeAndNoSim);
    ui->qqRandomColors->setEnabled(sel && noEditModeAndNoSim);
    ui->qqUsePoints->setEnabled(sel && noEditModeAndNoSim);
    ui->qqPointSize->setEnabled(sel && noEditModeAndNoSim && it->getUsePointsInsteadOfCubes());
    ui->qqClear->setEnabled(sel && noEditModeAndNoSim);
    ui->qqInsert->setEnabled(sel && noEditModeAndNoSim && (objCnt > 1));
    ui->qqSubtract->setEnabled(sel && noEditModeAndNoSim && (objCnt > 1));
    ui->qqEmissiveColor->setEnabled(sel && noEditModeAndNoSim);

    if (sel)
    {
        ui->qqSize->setText(utils::getSizeString(false, it->getCellSize()).c_str());
        ui->qqShowOctree->setChecked(it->getShowOctree());
        ui->qqRandomColors->setChecked(it->getUseRandomColors());
        ui->qqCellCount->setText(utils::getIntString(false, (int)it->getCubePositions()->size() / 3).c_str());
        ui->qqUsePoints->setChecked(it->getUsePointsInsteadOfCubes());
        ui->qqPointSize->setText(utils::getIntString(false, it->getPointSize()).c_str());
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
        double newVal = GuiApp::getEvalDouble(ui->qqSize->text().toStdString().c_str(), &ok);
        if (ok)
        {
            App::appendSimulationThreadCommand(SET_VOXELSIZE_OCTREEGUITRIGGEREDCMD,
                                               App::currentWorld->sceneObjects->getLastSelectionHandle(), -1, newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgOctrees::on_qqColor_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CQDlgColor::displayDlg(COLOR_ID_OCTREE, App::currentWorld->sceneObjects->getLastSelectionHandle(), -1, 0,
                               GuiApp::mainWindow);
    }
}

void CQDlgOctrees::on_qqShowOctree_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_SHOWSTRUCTURE_OCTREEGUITRIGGEREDCMD,
                                           App::currentWorld->sceneObjects->getLastSelectionHandle());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgOctrees::on_qqRandomColors_clicked()
{
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_RANDOMCOLORS_OCTREEGUITRIGGEREDCMD,
                                           App::currentWorld->sceneObjects->getLastSelectionHandle());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgOctrees::on_qqUsePoints_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_SHOWPOINTS_OCTREEGUITRIGGEREDCMD,
                                           App::currentWorld->sceneObjects->getLastSelectionHandle());
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
        int newVal = (int)GuiApp::getEvalInt(ui->qqPointSize->text().toStdString().c_str(), &ok);
        if (ok)
        {
            App::appendSimulationThreadCommand(SET_POINTSIZE_OCTREEGUITRIGGEREDCMD,
                                               App::currentWorld->sceneObjects->getLastSelectionHandle(), newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgOctrees::on_qqClear_clicked()
{
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        App::appendSimulationThreadCommand(CLEAR_OCTREEGUITRIGGEREDCMD,
                                           App::currentWorld->sceneObjects->getLastSelectionHandle());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgOctrees::on_qqInsert_clicked()
{
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        COcTree *it = App::currentWorld->sceneObjects->getLastSelectionOctree();
        if (it != nullptr)
        {
            SSimulationThreadCommand cmd;
            cmd.cmdId = INSERT_SELECTEDVISIBLEOBJECTS_OCTREEGUITRIGGEREDCMD;
            App::appendSimulationThreadCommand(cmd);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgOctrees::on_qqSubtract_clicked()
{
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        COcTree *it = App::currentWorld->sceneObjects->getLastSelectionOctree();
        if (it != nullptr)
        {
            SSimulationThreadCommand cmd;
            cmd.cmdId = SUBTRACT_SELECTEDVISIBLEOBJECTS_OCTREEGUITRIGGEREDCMD;
            App::appendSimulationThreadCommand(cmd);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgOctrees::on_qqEmissiveColor_clicked()
{
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_COLOREMISSIVE_OCTREEGUITRIGGEREDCMD,
                                           App::currentWorld->sceneObjects->getLastSelectionHandle());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}
