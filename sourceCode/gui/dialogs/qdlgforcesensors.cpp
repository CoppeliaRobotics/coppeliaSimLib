
#include "vrepMainHeader.h"
#include "qdlgforcesensors.h"
#include "ui_qdlgforcesensors.h"
#include "tt.h"
#include "gV.h"
#include "qdlgmaterial.h"
#include "app.h"

CQDlgForceSensors::CQDlgForceSensors(QWidget *parent) :
    CDlgEx(parent),
    ui(new Ui::CQDlgForceSensors)
{
    _dlgType=FORCE_SENSOR_DLG;
    ui->setupUi(this);
}

CQDlgForceSensors::~CQDlgForceSensors()
{
    delete ui;
}

void CQDlgForceSensors::cancelEvent()
{
    // we override this cancel event. The container window should close, not this one!!
    App::mainWindow->dlgCont->close(OBJECT_DLG);
}

void CQDlgForceSensors::refresh()
{
    QLineEdit* lineEditToSelect=getSelectedLineEdit();
    bool noEditModeNoSim=(App::getEditModeType()==NO_EDIT_MODE)&&App::ct->simulation->isSimulationStopped();
    bool sel=App::ct->objCont->isLastSelectionAForceSensor();
    bool bigSel=(App::ct->objCont->getForceSensorNumberInSelection()>1);
    CForceSensor* it=App::ct->objCont->getLastSelection_forceSensor();
    bool breakingEnabled=false;
    if ( (it!=nullptr)&&(it->getEnableForceThreshold()||it->getEnableTorqueThreshold()) )
        breakingEnabled=true;

    ui->qqSize->setEnabled(sel&&noEditModeNoSim);
    ui->qqColorA->setEnabled(sel&&noEditModeNoSim);
    ui->qqColorB->setEnabled(sel&&noEditModeNoSim);
    ui->qqApplyMain->setEnabled(bigSel&&noEditModeNoSim);

    ui->qqSampleSize->setEnabled(sel&&noEditModeNoSim);
    ui->qqAverage->setEnabled(sel&&noEditModeNoSim);
    ui->qqMedian->setEnabled(sel&&noEditModeNoSim);
    ui->qqApplyFilter->setEnabled(bigSel&&noEditModeNoSim);

    ui->qqForceThresholdEnable->setEnabled(sel&&noEditModeNoSim);
    ui->qqTorqueThresholdEnable->setEnabled(sel&&noEditModeNoSim);
    ui->qqForceThreshold->setEnabled(sel&&it->getEnableForceThreshold()&&noEditModeNoSim);
    ui->qqTorqueThreshold->setEnabled(sel&&it->getEnableTorqueThreshold()&&noEditModeNoSim);
    ui->qqViolationCount->setEnabled(breakingEnabled&&noEditModeNoSim);
    ui->qqApplyBreaking->setEnabled(bigSel&&noEditModeNoSim);

    if (sel)
    {
        ui->qqSize->setText(tt::getFString(false,it->getSize(),3).c_str());

        ui->qqSampleSize->setText(tt::getIString(false,it->getValueCountForFilter()).c_str());
        ui->qqAverage->setChecked(it->getFilterType()==0);
        ui->qqMedian->setChecked(it->getFilterType()==1);

        ui->qqForceThresholdEnable->setChecked(it->getEnableForceThreshold());
        ui->qqForceThreshold->setText(tt::getEString(false,it->getForceThreshold(),2).c_str());
        ui->qqTorqueThresholdEnable->setChecked(it->getEnableTorqueThreshold());
        ui->qqTorqueThreshold->setText(tt::getEString(false,it->getTorqueThreshold(),2).c_str());
        ui->qqViolationCount->setText(tt::getIString(false,it->getConsecutiveThresholdViolationsForBreaking()).c_str());
    }
    else
    {
        ui->qqSize->setText("");

        ui->qqSampleSize->setText("");
        ui->qqAverage->setChecked(false);
        ui->qqMedian->setChecked(false);

        ui->qqForceThresholdEnable->setChecked(false);
        ui->qqForceThreshold->setText("");
        ui->qqTorqueThresholdEnable->setChecked(false);
        ui->qqTorqueThreshold->setText("");
        ui->qqViolationCount->setText("");
    }
    selectLineEdit(lineEditToSelect);
}

void CQDlgForceSensors::on_qqSize_editingFinished()
{
    if (!ui->qqSize->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        float newVal=ui->qqSize->text().toFloat(&ok);
        if (ok)
        {
            App::appendSimulationThreadCommand(SET_SIZE_FORCESENSORGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),-1,newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgForceSensors::on_qqColorA_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CQDlgMaterial::displayMaterialDlg(COLOR_ID_FORCESENSOR_A,App::ct->objCont->getLastSelectionID(),-1,App::mainWindow);
    }
}

void CQDlgForceSensors::on_qqColorB_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CQDlgMaterial::displayMaterialDlg(COLOR_ID_FORCESENSOR_B,App::ct->objCont->getLastSelectionID(),-1,App::mainWindow);
    }
}

void CQDlgForceSensors::on_qqApplyMain_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        SSimulationThreadCommand cmd;
        cmd.cmdId=APPLY_VISUALPROP_FORCESENSORGUITRIGGEREDCMD;
        cmd.intParams.push_back(App::ct->objCont->getLastSelectionID());
        for (int i=0;i<App::ct->objCont->getSelSize()-1;i++)
            cmd.intParams.push_back(App::ct->objCont->getSelID(i));
        App::appendSimulationThreadCommand(cmd);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgForceSensors::on_qqSampleSize_editingFinished()
{
    if (!ui->qqSampleSize->isModified())
        return;
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        bool ok;
        int newVal=ui->qqSampleSize->text().toInt(&ok);
        if (ok)
        {
            App::appendSimulationThreadCommand(SET_SAMPLESIZE_FORCESENSORGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgForceSensors::on_qqAverage_clicked()
{
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        App::appendSimulationThreadCommand(SET_AVERAGEVALUE_FORCESENSORGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgForceSensors::on_qqMedian_clicked()
{
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        App::appendSimulationThreadCommand(SET_MEDIANVALUE_FORCESENSORGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgForceSensors::on_qqApplyFilter_clicked()
{
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        SSimulationThreadCommand cmd;
        cmd.cmdId=APPLY_FILER_FORCESENSORGUITRIGGEREDCMD;
        cmd.intParams.push_back(App::ct->objCont->getLastSelectionID());
        for (int i=0;i<App::ct->objCont->getSelSize()-1;i++)
            cmd.intParams.push_back(App::ct->objCont->getSelID(i));
        App::appendSimulationThreadCommand(cmd);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgForceSensors::on_qqForceThresholdEnable_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_FORCETHRESHOLDENABLE_FORCESENSORGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgForceSensors::on_qqForceThreshold_editingFinished()
{
    if (!ui->qqForceThreshold->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        float newVal=ui->qqForceThreshold->text().toFloat(&ok);
        if (ok)
        {
            App::appendSimulationThreadCommand(SET_FORCETHRESHOLD_FORCESENSORGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),-1,newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgForceSensors::on_qqTorqueThresholdEnable_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_TORQUETHRESHOLDENABLE_FORCESENSORGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgForceSensors::on_qqTorqueThreshold_editingFinished()
{
    if (!ui->qqTorqueThreshold->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        float newVal=ui->qqTorqueThreshold->text().toFloat(&ok);
        if (ok)
        {
            App::appendSimulationThreadCommand(SET_TORQUETHRESHOLD_FORCESENSORGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),-1,newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgForceSensors::on_qqViolationCount_editingFinished()
{
    if (!ui->qqViolationCount->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        int newVal=ui->qqViolationCount->text().toInt(&ok);
        if (ok)
        {
            App::appendSimulationThreadCommand(SET_CONSECTHRESHOLDVIOLATIONS_FORCESENSORGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgForceSensors::on_qqApplyBreaking_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        SSimulationThreadCommand cmd;
        cmd.cmdId=APPLY_BREAKING_FORCESENSORGUITRIGGEREDCMD;
        cmd.intParams.push_back(App::ct->objCont->getLastSelectionID());
        for (int i=0;i<App::ct->objCont->getSelSize()-1;i++)
            cmd.intParams.push_back(App::ct->objCont->getSelID(i));
        App::appendSimulationThreadCommand(cmd);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}
