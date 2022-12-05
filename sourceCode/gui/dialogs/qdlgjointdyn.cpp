#include "qdlgjointdyn.h"
#include "ui_qdlgjointdyn.h"
#include "tt.h"
#include "gV.h"
#include "qdlgjoints.h"
#include "app.h"
#include "simStringTable.h"
#include "vMessageBox.h"

CQDlgJointDyn::CQDlgJointDyn(QWidget *parent) :
    CDlgEx(parent),
    ui(new Ui::CQDlgJointDyn)
{
    _dlgType=JOINT_DYN_DLG;
    ui->setupUi(this);
    inMainRefreshRoutine=false;
}

CQDlgJointDyn::~CQDlgJointDyn()
{
    delete ui;
}

void CQDlgJointDyn::cancelEvent()
{ // no cancel event allowed
    CQDlgJoints::showDynamicWindow=false;
    CDlgEx::cancelEvent();
    App::setFullDialogRefreshFlag();
}

void CQDlgJointDyn::refresh()
{
    inMainRefreshRoutine=true;
    QLineEdit* lineEditToSelect=getSelectedLineEdit();
    bool noEditModeNoSim=(App::getEditModeType()==NO_EDIT_MODE)&&App::currentWorld->simulation->isSimulationStopped();

    bool sel=App::currentWorld->sceneObjects->isLastSelectionAJoint();
    bool bigSel=(App::currentWorld->sceneObjects->isLastSelectionAJoint()&&(App::currentWorld->sceneObjects->getJointCountInSelection()>1));
    bool revolute=false;
    bool prismatic=false;
    bool spherical=false;
    bool dynamic=false;
    bool canBeMotorized=false;
    int ctrlMode=-1;
    CJoint* it=nullptr;

    if (sel)
    {
        it=App::currentWorld->sceneObjects->getLastSelectionJoint();
        revolute=(it->getJointType()==sim_joint_revolute_subtype);
        prismatic=(it->getJointType()==sim_joint_prismatic_subtype);
        spherical=(it->getJointType()==sim_joint_spherical_subtype);
        dynamic=((it->getJointMode()==sim_jointmode_dynamic)||it->getHybridFunctionality_old() );
        canBeMotorized=(dynamic&&(!spherical));
        ctrlMode=it->getDynCtrlMode();
        if (ctrlMode==sim_jointdynctrl_positioncb)
            ctrlMode=sim_jointdynctrl_position;
        if (ctrlMode==sim_jointdynctrl_springcb)
            ctrlMode=sim_jointdynctrl_spring;
    }

    ui->qqModeLabel->setEnabled(noEditModeNoSim&&canBeMotorized&&(!it->getHybridFunctionality_old()));
    ui->qqCombo->setEnabled(noEditModeNoSim&&canBeMotorized&&(!it->getHybridFunctionality_old()));
    ui->qqCombo->clear();
    ui->qqCombo->addItem("Free",QVariant(sim_jointdynctrl_free));
    ui->qqCombo->addItem("Force",QVariant(sim_jointdynctrl_force));
    ui->qqCombo->addItem("Velocity",QVariant(sim_jointdynctrl_velocity));
    ui->qqCombo->addItem("Position",QVariant(sim_jointdynctrl_position));
    ui->qqCombo->addItem("Spring",QVariant(sim_jointdynctrl_spring));
    ui->qqCombo->addItem("Custom",QVariant(sim_jointdynctrl_callback));
    for (int i=0;i<ui->qqCombo->count();i++)
    {
        if (ui->qqCombo->itemData(i).toInt()==ctrlMode)
        {
            ui->qqCombo->setCurrentIndex(i);
            break;
        }
    }
    ui->qqStack->setCurrentIndex(ui->qqCombo->currentIndex());
    ui->qqStack->setEnabled(noEditModeNoSim&&canBeMotorized);

    if (sel&&prismatic)
    {
        ui->qqForceMode_forceLabel->setText("Constant force [N]");

        ui->qqVelocityMode_velocityLabel->setText("Target velocity [m/s]");
        ui->qqVelocityMode_forceLabel->setText("Max. force [N]");
        ui->qqVelocityMode_maxAccelLabel->setText("Max. acceleration [m/s^2]");
        ui->qqVelocityMode_maxJerkLabel->setText("Max. jerk [m/s^3]");

        ui->qqPositionMode_positionLabel->setText("Target position [m]");
        ui->qqPositionMode_forceLabel->setText("Max. force [N]");
        ui->qqPositionMode_maxVelLabel->setText("Max. velocity [m/s]");
        ui->qqPositionMode_maxAccelLabel->setText("Max. acceleration [m/s^2]");
        ui->qqPositionMode_maxJerkLabel->setText("Max. jerk [m/s^3]");


        ui->qqSpringMode_positionLabel->setText("Zero displacement pos. [m]");
        //ui->qqKLabel->setText("K [N/m]");
        //ui->qqCLabel->setText("C [N*s/m]");
    }
    else
    {
        ui->qqForceMode_forceLabel->setText("Constant torque [N*m]");

        ui->qqVelocityMode_velocityLabel->setText("Target velocity [deg/s]");
        ui->qqVelocityMode_forceLabel->setText("Max. torque [N*m]");
        ui->qqVelocityMode_maxAccelLabel->setText("Max. acceleration [deg/s^2]");
        ui->qqVelocityMode_maxJerkLabel->setText("Max. jerk [deg/s^3]");

        ui->qqPositionMode_positionLabel->setText("Target angle [deg]");
        ui->qqPositionMode_forceLabel->setText("Max. torque [N*m]");
        ui->qqPositionMode_maxVelLabel->setText("Max. velocity [deg/s]");
        ui->qqPositionMode_maxAccelLabel->setText("Max. acceleration [deg/s^2]");
        ui->qqPositionMode_maxJerkLabel->setText("Max. jerk [deg/s^3]");

        ui->qqSpringMode_positionLabel->setText("Zero displacement angle [deg]");
        //ui->qqKLabel->setText("Spring constant K [N*m/deg]");
        //ui->qqCLabel->setText("Damping coefficient C [N*s*m/deg]");
    }

    ui->qqAdjustEngineProperties->setEnabled(dynamic&&noEditModeNoSim);
    ui->qqApplyParams->setEnabled(dynamic&&bigSel&&noEditModeNoSim);


    ui->qqPositionMode_cb->setVisible(canBeMotorized&&(it->getDynCtrlMode()==sim_jointdynctrl_positioncb));
    ui->qqSpringMode_cb->setVisible(canBeMotorized&&(it->getDynCtrlMode()==sim_jointdynctrl_springcb));

    if (dynamic&&(ctrlMode==sim_jointdynctrl_force))
        ui->qqForceMode_force->setText(tt::getEString(false,it->getTargetForce(true),4).c_str());
    else
        ui->qqForceMode_force->setText("");

    if (dynamic&&(ctrlMode==sim_jointdynctrl_velocity))
    {
        double maxVelAccelJerk[3];
        it->getMaxVelAccelJerk(maxVelAccelJerk);
        if (it->getJointType()==sim_joint_revolute_subtype)
        {
            ui->qqVelocityMode_velocity->setText(tt::getAngleEString(true,it->getTargetVelocity(),4).c_str());
            ui->qqVelocityMode_maxAccel->setText(tt::getAngleEString(false,maxVelAccelJerk[1],3).c_str());
            ui->qqVelocityMode_maxJerk->setText(tt::getAngleEString(false,maxVelAccelJerk[2],3).c_str());
        }
        else
        {
            ui->qqVelocityMode_velocity->setText(tt::getEString(true,it->getTargetVelocity(),4).c_str());
            ui->qqVelocityMode_maxAccel->setText(tt::getEString(false,maxVelAccelJerk[1],3).c_str());
            ui->qqVelocityMode_maxJerk->setText(tt::getEString(false,maxVelAccelJerk[2],3).c_str());
        }
        ui->qqVelocityMode_force->setText(tt::getEString(false,it->getTargetForce(false),4).c_str());
        ui->qqVelocityMode_motorLock->setVisible(it->getMotorLock());
        ui->qqVelocityMode_motorLock->setChecked(it->getMotorLock());
        ui->qqVelocityMode_ruckig->setChecked(it->getDynVelCtrlType()==1);
        ui->qqVelocityMode_maxAccel->setEnabled(it->getDynVelCtrlType()==1);
        ui->qqVelocityMode_maxJerk->setEnabled(it->getDynVelCtrlType()==1);
    }
    else
    {
        ui->qqVelocityMode_velocity->setText("");
        ui->qqVelocityMode_force->setText("");
        ui->qqVelocityMode_motorLock->setVisible(false);
        ui->qqVelocityMode_motorLock->setChecked(false);
        ui->qqVelocityMode_ruckig->setChecked(false);
        ui->qqVelocityMode_maxAccel->setText("");
        ui->qqVelocityMode_maxJerk->setText("");
    }

    if (dynamic&&(ctrlMode==sim_jointdynctrl_position))
    {
        ui->qqPositionMode_ruckig->setChecked(it->getDynPosCtrlType()==1);

        if (it->getJointType()==sim_joint_revolute_subtype)
            ui->qqPositionMode_position->setText(tt::getAngleEString(true,it->getTargetPosition(),4).c_str());
        else
            ui->qqPositionMode_position->setText(tt::getEString(true,it->getTargetPosition(),4).c_str());
        ui->qqPositionMode_force->setText(tt::getEString(false,it->getTargetForce(false),4).c_str());
        double maxVelAccelJerk[3];
        it->getMaxVelAccelJerk(maxVelAccelJerk);
        if (it->getJointType()==sim_joint_revolute_subtype)
        {
            ui->qqPositionMode_maxVel->setText(tt::getAngleEString(false,maxVelAccelJerk[0],3).c_str());
            ui->qqPositionMode_maxAccel->setText(tt::getAngleEString(false,maxVelAccelJerk[1],3).c_str());
            ui->qqPositionMode_maxJerk->setText(tt::getAngleEString(false,maxVelAccelJerk[2],3).c_str());
        }
        else
        {
            ui->qqPositionMode_maxVel->setText(tt::getEString(false,maxVelAccelJerk[0],3).c_str());
            ui->qqPositionMode_maxAccel->setText(tt::getEString(false,maxVelAccelJerk[1],3).c_str());
            ui->qqPositionMode_maxJerk->setText(tt::getEString(false,maxVelAccelJerk[2],3).c_str());
        }

        ui->qqPositionMode_cb->setEnabled(it->getDynPosCtrlType()==0);

        // Always enabled   ui->qqPositionMode_maxVel->setEnabled(it->getDynPosCtrlType()==1);
        ui->qqPositionMode_maxAccel->setEnabled(it->getDynPosCtrlType()==1);
        ui->qqPositionMode_maxJerk->setEnabled(it->getDynPosCtrlType()==1);
        ui->qqPositionMode_cb->setChecked(it->getDynCtrlMode()==sim_jointdynctrl_positioncb);
    }
    else
    {
        ui->qqPositionMode_position->setText("");
        ui->qqPositionMode_force->setText("");
        ui->qqPositionMode_maxVel->setText("");
        ui->qqPositionMode_maxAccel->setText("");
        ui->qqPositionMode_maxJerk->setText("");
    }

    if (dynamic&&(ctrlMode==sim_jointdynctrl_spring))
    {
        if (it->getJointType()==sim_joint_revolute_subtype)
            ui->qqSpringMode_position->setText(tt::getAngleEString(true,it->getTargetPosition(),4).c_str());
        else
            ui->qqSpringMode_position->setText(tt::getEString(true,it->getTargetPosition(),4).c_str());
        double kp,cp;
        it->getKc(kp,cp);
        ui->qqK->setText(tt::getFString(false,kp,3).c_str());
        ui->qqC->setText(tt::getFString(false,cp,3).c_str());
        ui->qqSpringMode_cb->setChecked(it->getDynCtrlMode()==sim_jointdynctrl_springcb);
    }
    else
    {
        ui->qqSpringMode_position->setText("");
        ui->qqK->setText("");
        ui->qqC->setText("");
    }
    selectLineEdit(lineEditToSelect);
    inMainRefreshRoutine=false;
}

void CQDlgJointDyn::on_qqApplyParams_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        SSimulationThreadCommand cmd;
        cmd.cmdId=APPLY_PARAMS_JOINTDYNGUITRIGGEREDCMD;
        cmd.intParams.push_back(App::currentWorld->sceneObjects->getLastSelectionHandle());
        for (size_t i=0;i<App::currentWorld->sceneObjects->getSelectionCount()-1;i++)
            cmd.intParams.push_back(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
        App::appendSimulationThreadCommand(cmd);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgJointDyn::on_qqK_editingFinished()
{
    if (!ui->qqK->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        CJoint* it=App::currentWorld->sceneObjects->getLastSelectionJoint();
        bool ok;
        double newVal=ui->qqK->text().toFloat(&ok);
        if (ok&&(it!=nullptr))
        {
            double kp,cp;
            it->getKc(kp,cp);
            App::appendSimulationThreadCommand(SET_KCVALUES_JOINTDYNGUITRIGGEREDCMD,it->getObjectHandle(),-1,newVal,cp);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgJointDyn::on_qqC_editingFinished()
{
    if (!ui->qqC->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        CJoint* it=App::currentWorld->sceneObjects->getLastSelectionJoint();
        bool ok;
        double newVal=ui->qqC->text().toFloat(&ok);
        if (ok&&(it!=nullptr))
        {
            double kp,cp;
            it->getKc(kp,cp);
            App::appendSimulationThreadCommand(SET_KCVALUES_JOINTDYNGUITRIGGEREDCMD,it->getObjectHandle(),-1,kp,newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgJointDyn::on_qqForceMode_force_editingFinished()
{
    if (!ui->qqForceMode_force->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        double newVal=ui->qqForceMode_force->text().toFloat(&ok);
        if (ok)
        {
            App::appendSimulationThreadCommand(SET_FORCE_JOINTDYNGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle(),-1,newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgJointDyn::on_qqVelocityMode_force_editingFinished()
{
    if (!ui->qqVelocityMode_force->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        double newVal=ui->qqVelocityMode_force->text().toFloat(&ok);
        if (ok)
        {
            App::appendSimulationThreadCommand(SET_MAXFORCE_JOINTDYNGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle(),-1,newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgJointDyn::on_qqVelocityMode_velocity_editingFinished()
{
    if (!ui->qqVelocityMode_velocity->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        CJoint* it=App::currentWorld->sceneObjects->getLastSelectionJoint();
        bool ok;
        double newVal=ui->qqVelocityMode_velocity->text().toFloat(&ok);
        if (ok&&(it!=nullptr))
        {
            if (it->getJointType()!=sim_joint_prismatic_subtype)
                newVal*=gv::userToAngularVel;
            App::appendSimulationThreadCommand(SET_TARGETVELOCITY_JOINTDYNGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle(),-1,newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgJointDyn::on_qqVelocityMode_motorLock_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_LOCKMOTOR_JOINTDYNGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgJointDyn::on_qqPositionMode_position_editingFinished()
{
    if (!ui->qqPositionMode_position->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        CJoint* it=App::currentWorld->sceneObjects->getLastSelectionJoint();
        bool ok;
        double newVal=ui->qqPositionMode_position->text().toFloat(&ok);
        if (ok&&(it!=nullptr))
        {
            if (it->getJointType()!=sim_joint_prismatic_subtype)
                newVal*=gv::userToRad;
            App::appendSimulationThreadCommand(SET_TARGETPOSITION_JOINTDYNGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle(),-1,newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgJointDyn::on_qqPositionMode_force_editingFinished()
{
    if (!ui->qqPositionMode_force->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        double newVal=ui->qqPositionMode_force->text().toFloat(&ok);
        if (ok)
        {
            App::appendSimulationThreadCommand(SET_MAXFORCE_JOINTDYNGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle(),-1,newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgJointDyn::on_qqPositionMode_cb_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        SSimulationThreadCommand cmd;
        cmd.cmdId=SET_JOINTCTRLMODE_JOINTDYNGUITRIGGEREDCMD;
        cmd.intParams.push_back(App::currentWorld->sceneObjects->getLastSelectionHandle());
        cmd.intParams.push_back(sim_jointdynctrl_position);
        App::appendSimulationThreadCommand(cmd);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgJointDyn::on_qqSpringMode_cb_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        SSimulationThreadCommand cmd;
        cmd.cmdId=SET_JOINTCTRLMODE_JOINTDYNGUITRIGGEREDCMD;
        cmd.intParams.push_back(App::currentWorld->sceneObjects->getLastSelectionHandle());
        cmd.intParams.push_back(sim_jointdynctrl_spring);
        App::appendSimulationThreadCommand(cmd);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgJointDyn::on_qqSpringMode_position_editingFinished()
{
    if (!ui->qqSpringMode_position->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        CJoint* it=App::currentWorld->sceneObjects->getLastSelectionJoint();
        bool ok;
        double newVal=ui->qqSpringMode_position->text().toFloat(&ok);
        if (ok&&(it!=nullptr))
        {
            if (it->getJointType()!=sim_joint_prismatic_subtype)
                newVal*=gv::userToRad;
            App::appendSimulationThreadCommand(SET_TARGETPOSITION_JOINTDYNGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle(),-1,newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgJointDyn::on_qqAdjustEngineProperties_clicked()
{
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        CJoint* it=App::currentWorld->sceneObjects->getLastSelectionJoint();
        if (it!=nullptr)
        {
            SSimulationThreadCommand cmd;
            cmd.cmdId=SET_ENGINEPARAMS_JOINTDYNGUITRIGGEREDCMD;
            cmd.intParams.push_back(App::currentWorld->sceneObjects->getLastSelectionHandle());
            App::appendSimulationThreadCommand(cmd);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}


void CQDlgJointDyn::on_qqCombo_currentIndexChanged(int index)
{
    if (!inMainRefreshRoutine)
    {
        IF_UI_EVENT_CAN_READ_DATA
        {
            int mode=ui->qqCombo->itemData(ui->qqCombo->currentIndex()).toInt();
            App::appendSimulationThreadCommand(SET_JOINTCTRLMODE_JOINTDYNGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle(),mode);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgJointDyn::on_qqPositionMode_ruckig_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(SET_JOINTCTRLMODE_JOINTDYNGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle(),sim_jointdynctrl_position); // make sure it is not sim_jointdynctrl_positioncb
        App::appendSimulationThreadCommand(SET_JOINTPOSCTRLMODETOGGLE_JOINTDYNGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgJointDyn::on_qqPositionMode_maxVel_editingFinished()
{
    if (!ui->qqPositionMode_maxVel->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        CJoint* it=App::currentWorld->sceneObjects->getLastSelectionJoint();
        bool ok;
        double newVal=ui->qqPositionMode_maxVel->text().toFloat(&ok);
        if (ok&&(it!=nullptr))
        {
            if (it->getJointType()!=sim_joint_prismatic_subtype)
                newVal*=gv::userToRad;
            App::appendSimulationThreadCommand(SET_MOTIONPROFILEVALS_JOINTDYNGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle(),0,newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgJointDyn::on_qqPositionMode_maxAccel_editingFinished()
{
    if (!ui->qqPositionMode_maxAccel->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        CJoint* it=App::currentWorld->sceneObjects->getLastSelectionJoint();
        bool ok;
        double newVal=ui->qqPositionMode_maxAccel->text().toFloat(&ok);
        if (ok&&(it!=nullptr))
        {
            if (it->getJointType()!=sim_joint_prismatic_subtype)
                newVal*=gv::userToRad;
            App::appendSimulationThreadCommand(SET_MOTIONPROFILEVALS_JOINTDYNGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle(),1,newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgJointDyn::on_qqPositionMode_maxJerk_editingFinished()
{
    if (!ui->qqPositionMode_maxJerk->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        CJoint* it=App::currentWorld->sceneObjects->getLastSelectionJoint();
        bool ok;
        double newVal=ui->qqPositionMode_maxJerk->text().toFloat(&ok);
        if (ok&&(it!=nullptr))
        {
            if (it->getJointType()!=sim_joint_prismatic_subtype)
                newVal*=gv::userToRad;
            App::appendSimulationThreadCommand(SET_MOTIONPROFILEVALS_JOINTDYNGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle(),2,newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgJointDyn::on_qqVelocityMode_ruckig_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_JOINTVELCTRLMODETYPE_JOINTDYNGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgJointDyn::on_qqVelocityMode_maxAccel_editingFinished()
{
    if (!ui->qqVelocityMode_maxAccel->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        CJoint* it=App::currentWorld->sceneObjects->getLastSelectionJoint();
        bool ok;
        double newVal=ui->qqVelocityMode_maxAccel->text().toFloat(&ok);
        if (ok&&(it!=nullptr))
        {
            if (it->getJointType()!=sim_joint_prismatic_subtype)
                newVal*=gv::userToRad;
            App::appendSimulationThreadCommand(SET_MOTIONPROFILEVALS_JOINTDYNGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle(),1,newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgJointDyn::on_qqVelocityMode_maxJerk_editingFinished()
{
    if (!ui->qqVelocityMode_maxJerk->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        CJoint* it=App::currentWorld->sceneObjects->getLastSelectionJoint();
        bool ok;
        double newVal=ui->qqVelocityMode_maxJerk->text().toFloat(&ok);
        if (ok&&(it!=nullptr))
        {
            if (it->getJointType()!=sim_joint_prismatic_subtype)
                newVal*=gv::userToRad;
            App::appendSimulationThreadCommand(SET_MOTIONPROFILEVALS_JOINTDYNGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle(),2,newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}
