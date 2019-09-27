
#include "vrepMainHeader.h"
#include "qdlgsimulation.h"
#include "ui_qdlgsimulation.h"
#include "app.h"
#include "tt.h"
#include "gV.h"
#include "v_repStrings.h"

CQDlgSimulation::CQDlgSimulation(QWidget *parent) :
    CDlgEx(parent),
    ui(new Ui::CQDlgSimulation)
{
    _dlgType=SIMULATION_DLG;
    ui->setupUi(this);
    inMainRefreshRoutine=false;
}

CQDlgSimulation::~CQDlgSimulation()
{
    delete ui;
}

void CQDlgSimulation::refresh()
{
    inMainRefreshRoutine=true;
    QLineEdit* lineEditToSelect=getSelectedLineEdit();
    bool noEditMode=App::getEditModeType()==NO_EDIT_MODE;
    bool noEditModeNoSim=noEditMode&&App::ct->simulation->isSimulationStopped();
    bool custom=(App::ct->simulation->getDefaultSimulationParameterIndex()==5);

    ui->qqConfigCombo->setEnabled(noEditModeNoSim);
    ui->qqTimeStep->setEnabled(custom&&noEditModeNoSim);
    ui->qqFullscreen->setEnabled(noEditModeNoSim);
    ui->qqScriptExecutionPasses->setEnabled(custom&&noEditModeNoSim);
    ui->qqRealTime->setEnabled(noEditModeNoSim);
    ui->qqCatchUp->setEnabled(noEditModeNoSim&&App::ct->simulation->getRealTimeSimulation());
    ui->qqMultiplicationCoefficient->setEnabled(noEditModeNoSim&&App::ct->simulation->getRealTimeSimulation());
    ui->qqPauseTime->setEnabled(noEditModeNoSim&&App::ct->simulation->getPauseAtSpecificTime());
    ui->qqPauseWhenTimeHigher->setEnabled(noEditModeNoSim);
    ui->qqPauseOnScriptError->setEnabled(noEditModeNoSim);
    ui->qqResetScene->setEnabled(noEditMode);
    ui->qqRemoveNewObjects->setEnabled(noEditMode);

    ui->qqConfigCombo->clear();
    ui->qqConfigCombo->addItem(strTranslate(IDSN_TIME_STEP_CONFIG_200),QVariant(0));
    ui->qqConfigCombo->addItem(strTranslate(IDSN_TIME_STEP_CONFIG_100),QVariant(1));
    ui->qqConfigCombo->addItem(strTranslate(IDSN_TIME_STEP_CONFIG_50),QVariant(2));
    ui->qqConfigCombo->addItem(strTranslate(IDSN_TIME_STEP_CONFIG_25),QVariant(3));
    ui->qqConfigCombo->addItem(strTranslate(IDSN_TIME_STEP_CONFIG_10),QVariant(4));

    float dt=float(App::ct->simulation->getSimulationTimeStep_raw_ns(5))/1000.0f;
    std::string txt("dt=");
    txt+=tt::FNb(0,dt,1,false);
    txt+=IDSN_TIME_STEP_CONFIG_CUSTOM;
    ui->qqConfigCombo->addItem(txt.c_str(),QVariant(5));

    for (int i=0;i<ui->qqConfigCombo->count();i++)
    {
        if (ui->qqConfigCombo->itemData(i).toInt()==App::ct->simulation->getDefaultSimulationParameterIndex())
        {
            ui->qqConfigCombo->setCurrentIndex(i);
            break;
        }
    }

    ui->qqRealTime->setChecked(App::ct->simulation->getRealTimeSimulation());
    ui->qqCatchUp->setChecked(App::ct->simulation->getCatchUpIfLate());
    ui->qqTimeStep->setText(tt::getDString(false,double(App::ct->simulation->getSimulationTimeStep_speedModified_ns())/1000000.0,4).c_str());
    ui->qqFullscreen->setChecked(App::ct->simulation->getFullscreenAtSimulationStart());

    ui->qqScriptExecutionPasses->setText(tt::getIString(false,App::ct->simulation->getSimulationPassesPerRendering_speedModified()).c_str());
    ui->qqMultiplicationCoefficient->setText(tt::getDString(false,App::ct->simulation->getRealTimeCoefficient_speedModified(),2).c_str());

    ui->qqResetScene->setChecked(App::ct->simulation->getResetSceneAtSimulationEnd());
    ui->qqRemoveNewObjects->setChecked(App::ct->simulation->getRemoveNewObjectsAtSimulationEnd());

    ui->qqPauseTime->setText(tt::getDString(false,double(App::ct->simulation->getPauseTime_ns())/1000000.0,2).c_str());
    ui->qqPauseWhenTimeHigher->setChecked(App::ct->simulation->getPauseAtSpecificTime());
    ui->qqPauseOnScriptError->setChecked(App::ct->simulation->getPauseAtError());

    selectLineEdit(lineEditToSelect);
    inMainRefreshRoutine=false;
}

void CQDlgSimulation::on_qqTimeStep_editingFinished()
{
    if (!ui->qqTimeStep->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (App::ct->simulation->isSimulationStopped())
        {
            bool ok;
            double newVal=ui->qqTimeStep->text().toDouble(&ok);
            if (ok)
            {
                SSimulationThreadCommand cmd;
                cmd.cmdId=SET_TIMESTEP_SIMULATIONGUITRIGGEREDCMD;
                cmd.uint64Params.push_back(quint64(newVal*1000000.0));
                App::appendSimulationThreadCommand(cmd);
                App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            }
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgSimulation::on_qqScriptExecutionPasses_editingFinished()
{
    if (!ui->qqScriptExecutionPasses->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (App::ct->simulation->isSimulationStopped())
        {
            bool ok;
            int newVal=ui->qqScriptExecutionPasses->text().toInt(&ok);
            if (ok)
            {
                App::appendSimulationThreadCommand(SET_PPF_SIMULATIONGUITRIGGEREDCMD,newVal);
                App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            }
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgSimulation::on_qqRealTime_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (App::ct->simulation->isSimulationStopped())
        {
            App::appendSimulationThreadCommand(TOGGLE_REALTIME_SIMULATIONGUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgSimulation::on_qqMultiplicationCoefficient_editingFinished()
{
    if (!ui->qqMultiplicationCoefficient->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (App::ct->simulation->isSimulationStopped())
        {
            bool ok;
            double newVal=ui->qqMultiplicationCoefficient->text().toDouble(&ok);
            if (ok)
            {
                App::appendSimulationThreadCommand(SET_REALTIMEFACTOR_SIMULATIONGUITRIGGEREDCMD,-1,-1,newVal);
                App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            }
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgSimulation::on_qqCatchUp_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (App::ct->simulation->isSimulationStopped())
        {
            App::appendSimulationThreadCommand(TOGGLE_TRYCATCHINGUP_SIMULATIONGUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgSimulation::on_qqPauseWhenTimeHigher_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_PAUSEATTIME_SIMULATIONGUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgSimulation::on_qqPauseTime_editingFinished()
{
    if (!ui->qqPauseTime->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (App::ct->simulation->isSimulationStopped())
        {
            bool ok;
            double newVal=ui->qqPauseTime->text().toDouble(&ok);
            if (ok)
            {
                SSimulationThreadCommand cmd;
                cmd.cmdId=SET_PAUSETIME_SIMULATIONGUITRIGGEREDCMD;
                cmd.uint64Params.push_back(quint64(newVal*1000000.0));
                App::appendSimulationThreadCommand(cmd);
                App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            }
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgSimulation::on_qqPauseOnScriptError_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_PAUSEATERROR_SIMULATIONGUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgSimulation::on_qqResetScene_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_RESETSCENETOINITIAL_SIMULATIONGUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgSimulation::on_qqRemoveNewObjects_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_REMOVENEWOBJECTS_SIMULATIONGUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgSimulation::on_qqConfigCombo_currentIndexChanged(int index)
{
    if (!inMainRefreshRoutine)
    {
        IF_UI_EVENT_CAN_READ_DATA
        {
            if (App::ct->simulation->isSimulationStopped())
            {
                int config=ui->qqConfigCombo->itemData(ui->qqConfigCombo->currentIndex()).toInt();
                App::appendSimulationThreadCommand(SET_TIMESTEPSCHEME_SIMULATIONGUITRIGGEREDCMD,config);
                App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
                App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
            }
        }
    }
}

void CQDlgSimulation::on_qqFullscreen_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_FULLSCREENATSTART_SIMULATIONGUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}
