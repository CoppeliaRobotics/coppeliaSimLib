#include "qdlgdynamics.h"
#include "ui_qdlgdynamics.h"
#include "tt.h"
#include "app.h"
#include "gV.h"
#include "simStrings.h"

CQDlgDynamics::CQDlgDynamics(QWidget *parent) :
    CDlgEx(parent),
    ui(new Ui::CQDlgDynamics)
{
    _dlgType=DYNAMICS_DLG;
    ui->setupUi(this);
    inMainRefreshRoutine=false;
}

CQDlgDynamics::~CQDlgDynamics()
{
    delete ui;
}

void CQDlgDynamics::cancelEvent()
{
    // we override this cancel event. The container window should close, not this one!!
    App::mainWindow->dlgCont->close(CALCULATION_DLG);
}

void CQDlgDynamics::refresh()
{
    inMainRefreshRoutine=true;
    QLineEdit* lineEditToSelect=getSelectedLineEdit();

    bool noEditModeNoSim=(App::getEditModeType()==NO_EDIT_MODE)&&App::currentWorld->simulation->isSimulationStopped();
    bool noEditMode=(App::getEditModeType()==NO_EDIT_MODE);
    bool en=App::currentWorld->dynamicsContainer->getDynamicsEnabled();

    ui->qqEngineCombo->setEnabled(noEditModeNoSim&&en);
    ui->qqContactPoints->setEnabled(noEditMode&&en);
    ui->qqAdjustEngine->setEnabled(noEditModeNoSim&&en);
    ui->qqTimestep->setEnabled(noEditModeNoSim&&en);
    ui->qqGravityX->setEnabled(noEditMode&&en);
    ui->qqGravityY->setEnabled(noEditMode&&en);
    ui->qqGravityZ->setEnabled(noEditMode&&en);

    if (App::currentWorld->dynamicsContainer->getSettingsAreDefault())
        ui->qqInfo->setText("");
    else
    {
        ui->qqInfo->setText("Detected non-default settings!");
        ui->qqInfo->setStyleSheet("QLabel { color : red; }");
    }

    ui->qqEnabled->setChecked(en);
    ui->qqEngineCombo->clear();
    ui->qqEngineCombo->addItem(IDS_BULLET_2_78,0);
    ui->qqEngineCombo->addItem(IDS_BULLET_2_83,1);
    ui->qqEngineCombo->addItem(IDS_ODE,2);
    ui->qqEngineCombo->addItem(IDS_VORTEX,3);
    ui->qqEngineCombo->addItem(IDS_NEWTON,4);
    ui->qqEngineCombo->addItem(IDS_MUJOCO,5);
    int ver;
    int eng=App::currentWorld->dynamicsContainer->getDynamicEngineType(&ver);
    if ( (eng==sim_physics_bullet)&&(ver==0) )
        ui->qqEngineCombo->setCurrentIndex(0);
    if ( (eng==sim_physics_bullet)&&(ver==283) )
        ui->qqEngineCombo->setCurrentIndex(1);
    if (eng==sim_physics_ode)
        ui->qqEngineCombo->setCurrentIndex(2);
    if (eng==sim_physics_vortex)
        ui->qqEngineCombo->setCurrentIndex(3);
    if (eng==sim_physics_newton)
        ui->qqEngineCombo->setCurrentIndex(4);
    if (eng==sim_physics_mujoco)
        ui->qqEngineCombo->setCurrentIndex(5);

    ui->qqTimestep->setText(tt::getEString(false,App::currentWorld->dynamicsContainer->getCurrentDynamicStepSize(),3).c_str());

    ui->qqContactPoints->setChecked(App::currentWorld->dynamicsContainer->getDisplayContactPoints());

    C3Vector accel(App::currentWorld->dynamicsContainer->getGravity());
    ui->qqGravityX->setText(tt::getEString(true,accel(0),2).c_str());
    ui->qqGravityY->setText(tt::getEString(true,accel(1),2).c_str());
    ui->qqGravityZ->setText(tt::getEString(true,accel(2),2).c_str());

    selectLineEdit(lineEditToSelect);
    inMainRefreshRoutine=false;
}

void CQDlgDynamics::on_qqEnabled_clicked()
{
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_DYNAMICS_DYNAMICSGUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgDynamics::on_qqEngineCombo_currentIndexChanged(int index)
{
    if (!inMainRefreshRoutine)
    {
        IF_UI_EVENT_CAN_READ_DATA
        {
            int index=ui->qqEngineCombo->itemData(ui->qqEngineCombo->currentIndex()).toInt();
            int ver=0;
            int eng=0;
            if ((index==0)||(index==1))
                eng=sim_physics_bullet;
            if (index==1)
                ver=283;
            if (index==2)
                eng=sim_physics_ode;
            if (index==3)
                eng=sim_physics_vortex;
            if (index==4)
                eng=sim_physics_newton;
            if (index==5)
                eng=sim_physics_mujoco;

            App::appendSimulationThreadCommand(SET_ENGINE_DYNAMICSGUITRIGGEREDCMD,eng,ver);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(REFRESH_TOOLBARS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgDynamics::on_qqContactPoints_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_DISPLAYCONTACTS_DYNAMICSGUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgDynamics::on_qqAdjustEngine_clicked()
{
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        SSimulationThreadCommand cmd;
        cmd.cmdId=SET_ENGINEPARAMS_DYNAMICSGUITRIGGEREDCMD;
        App::appendSimulationThreadCommand(cmd);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgDynamics::on_qqGravityX_editingFinished()
{
    if (!ui->qqGravityX->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        float newVal=ui->qqGravityX->text().toFloat(&ok);
        if (ok)
        {
            C3Vector vect=App::currentWorld->dynamicsContainer->getGravity();
            vect(0)=newVal;
            SSimulationThreadCommand cmd;
            cmd.cmdId=SET_GRAVITY_DYNAMICSGUITRIGGEREDCMD;
            cmd.posParams.push_back(vect);
            App::appendSimulationThreadCommand(cmd);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgDynamics::on_qqGravityY_editingFinished()
{
    if (!ui->qqGravityY->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        float newVal=ui->qqGravityY->text().toFloat(&ok);
        if (ok)
        {
            C3Vector vect=App::currentWorld->dynamicsContainer->getGravity();
            vect(1)=newVal;
            SSimulationThreadCommand cmd;
            cmd.cmdId=SET_GRAVITY_DYNAMICSGUITRIGGEREDCMD;
            cmd.posParams.push_back(vect);
            App::appendSimulationThreadCommand(cmd);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgDynamics::on_qqGravityZ_editingFinished()
{
    if (!ui->qqGravityZ->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        float newVal=ui->qqGravityZ->text().toFloat(&ok);
        if (ok)
        {
            C3Vector vect=App::currentWorld->dynamicsContainer->getGravity();
            vect(2)=newVal;
            SSimulationThreadCommand cmd;
            cmd.cmdId=SET_GRAVITY_DYNAMICSGUITRIGGEREDCMD;
            cmd.posParams.push_back(vect);
            App::appendSimulationThreadCommand(cmd);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgDynamics::on_qqTimestep_editingFinished()
{
    if (!ui->qqTimestep->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        float newVal=ui->qqTimestep->text().toFloat(&ok);
        if (ok)
        {
            SSimulationThreadCommand cmd;
            cmd.cmdId=SET_TIMESTEP_DYNAMICSGUITRIGGEREDCMD;
            cmd.floatParams.push_back(newVal);
            App::appendSimulationThreadCommand(cmd);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}
