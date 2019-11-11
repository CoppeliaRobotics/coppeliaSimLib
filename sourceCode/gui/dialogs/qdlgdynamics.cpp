#include "qdlgdynamics.h"
#include "ui_qdlgdynamics.h"
#include "tt.h"
#include "app.h"
#include "gV.h"
#include "propBrowser_engineProp_general.h"
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

    bool noEditModeNoSim=(App::getEditModeType()==NO_EDIT_MODE)&&App::ct->simulation->isSimulationStopped();
    bool noEditMode=(App::getEditModeType()==NO_EDIT_MODE);
    bool en=App::ct->dynamicsContainer->getDynamicsEnabled();

    ui->qqEngineCombo->setEnabled(noEditModeNoSim&&en);
    ui->qqContactPoints->setEnabled(noEditMode&&en);
    ui->qqAdjustEngine->setEnabled(noEditModeNoSim&&en);
    ui->qqGravityX->setEnabled(noEditMode&&en);
    ui->qqGravityY->setEnabled(noEditMode&&en);
    ui->qqGravityZ->setEnabled(noEditMode&&en);

    ui->qqEnabled->setChecked(en);
    ui->qqEngineCombo->clear();
    ui->qqEngineCombo->addItem(strTranslate(IDS_BULLET_2_78),0);
    ui->qqEngineCombo->addItem(strTranslate(IDS_BULLET_2_83),1);
    ui->qqEngineCombo->addItem(strTranslate(IDS_ODE),2);
    ui->qqEngineCombo->addItem(strTranslate(IDS_VORTEX),3);
    ui->qqEngineCombo->addItem(strTranslate(IDS_NEWTON),4);
    int ver;
    int eng=App::ct->dynamicsContainer->getDynamicEngineType(&ver);
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

    ui->qqContactPoints->setChecked(App::ct->dynamicsContainer->getDisplayContactPoints());

    C3Vector accel(App::ct->dynamicsContainer->getGravity());
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
        CPropBrowserEngineGeneral dlg(this);
        dlg.setModal(true);
        dlg.exec(); // items are set in here
        SSimulationThreadCommand cmd;
        cmd.cmdId=SET_ALLGLOBALPARAMS_DYNAMICSGUITRIGGEREDCMD;
        cmd.intParams.push_back(App::ct->dynamicsContainer->getUseDynamicDefaultCalculationParameters());
        std::vector<int> iParams;
        std::vector<float> fParams;
        App::ct->dynamicsContainer->getBulletIntParams(iParams);
        App::ct->dynamicsContainer->getBulletFloatParams(fParams);
        cmd.intVectorParams.push_back(iParams);
        cmd.floatVectorParams.push_back(fParams);
        iParams.clear();
        fParams.clear();
        App::ct->dynamicsContainer->getOdeIntParams(iParams);
        App::ct->dynamicsContainer->getOdeFloatParams(fParams);
        cmd.intVectorParams.push_back(iParams);
        cmd.floatVectorParams.push_back(fParams);
        iParams.clear();
        fParams.clear();
        App::ct->dynamicsContainer->getVortexIntParams(iParams);
        App::ct->dynamicsContainer->getVortexFloatParams(fParams);
        cmd.intVectorParams.push_back(iParams);
        cmd.floatVectorParams.push_back(fParams);
        iParams.clear();
        fParams.clear();
        App::ct->dynamicsContainer->getNewtonIntParams(iParams);
        App::ct->dynamicsContainer->getNewtonFloatParams(fParams);
        cmd.intVectorParams.push_back(iParams);
        cmd.floatVectorParams.push_back(fParams);
        iParams.clear();
        fParams.clear();
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
            C3Vector vect=App::ct->dynamicsContainer->getGravity();
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
            C3Vector vect=App::ct->dynamicsContainer->getGravity();
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
            C3Vector vect=App::ct->dynamicsContainer->getGravity();
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
