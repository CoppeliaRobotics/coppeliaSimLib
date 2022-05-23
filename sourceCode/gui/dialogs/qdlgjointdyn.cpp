#include "qdlgjointdyn.h"
#include "ui_qdlgjointdyn.h"
#include "tt.h"
#include "gV.h"
#include "propBrowser_engineProp_joint.h"
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

    if (sel&&prismatic)
    {
        ui->qqCtrlMode_force->setText("Force control");
        ui->qqForceMode_forceLabel->setText("Force [N]");

        ui->qqVelocityMode_velocityLabel->setText("Target velocity [m/s]");
        ui->qqVelocityMode_forceLabel->setText("Force [N]");

        ui->qqPositionMode_positionLabel->setText("Target position [m]");
        ui->qqPositionMode_forceLabel->setText("Force [N]");
        ui->qqPositionMode_upperVel->setText("Upper velocity limit [m/s]");

        ui->qqSpringMode_positionLabel->setText("Zero displacement pos. [m]");
        ui->qqSpringMode_forceLabel->setText("Force [N]");
        //ui->qqKLabel->setText("K [N/m]");
        //ui->qqCLabel->setText("C [N*s/m]");
    }
    else
    {
        ui->qqCtrlMode_force->setText("Torque control");
        ui->qqForceMode_forceLabel->setText("Torque [N*m]");

        ui->qqVelocityMode_velocityLabel->setText("Target velocity [deg/s]");
        ui->qqVelocityMode_forceLabel->setText("Torque [N*m]");

        ui->qqPositionMode_positionLabel->setText("Target angle [deg]");
        ui->qqPositionMode_forceLabel->setText("Torque [N*m]");
        ui->qqPositionMode_upperVelLabel->setText("Upper velocity limit [deg/s]");

        ui->qqSpringMode_positionLabel->setText("Zero displacement angle [deg]");
        ui->qqSpringMode_forceLabel->setText("Torque [N*m]");
        //ui->qqKLabel->setText("Spring constant K [N*m/deg]");
        //ui->qqCLabel->setText("Damping coefficient C [N*s*m/deg]");
    }

    ui->qqAdjustEngineProperties->setEnabled(dynamic&&noEditModeNoSim);
    ui->qqApplyParams->setEnabled(dynamic&&bigSel&&noEditModeNoSim);

    ui->qqCtrlMode_free->setEnabled(canBeMotorized&&(!it->getHybridFunctionality_old()));
    ui->qqCtrlMode_free->setChecked(dynamic&&(ctrlMode==sim_jointdynctrl_free));


    ui->qqCtrlMode_force->setEnabled(canBeMotorized&&(!it->getHybridFunctionality_old()));
    ui->qqForceMode_force->setEnabled(dynamic&&(ctrlMode==sim_jointdynctrl_force));
    ui->qqCtrlMode_force->setChecked(dynamic&&(ctrlMode==sim_jointdynctrl_force));


    ui->qqCtrlMode_velocity->setEnabled(canBeMotorized&&(!it->getHybridFunctionality_old()));
    ui->qqVelocityMode_velocity->setEnabled(dynamic&&(ctrlMode==sim_jointdynctrl_velocity));
    ui->qqVelocityMode_force->setEnabled(dynamic&&(ctrlMode==sim_jointdynctrl_velocity));
    ui->qqVelocityMode_motorLock->setEnabled(dynamic&&(ctrlMode==sim_jointdynctrl_velocity));
    ui->qqCtrlMode_velocity->setChecked(dynamic&&(ctrlMode==sim_jointdynctrl_velocity));


    ui->qqCtrlMode_position->setEnabled(canBeMotorized&&(!it->getHybridFunctionality_old()));
    ui->qqPositionMode_position->setEnabled(dynamic&&(ctrlMode==sim_jointdynctrl_position));
    ui->qqPositionMode_force->setEnabled(dynamic&&(ctrlMode==sim_jointdynctrl_position));
    ui->qqPositionMode_upperVel->setEnabled(dynamic&&(ctrlMode==sim_jointdynctrl_position));
    ui->qqP->setEnabled(dynamic&&(ctrlMode==sim_jointdynctrl_position));
    ui->qqI->setEnabled(dynamic&&(ctrlMode==sim_jointdynctrl_position));
    ui->qqD->setEnabled(dynamic&&(ctrlMode==sim_jointdynctrl_position));
    ui->qqPositionMode_cb->setVisible(canBeMotorized&&(it->getDynCtrlMode()==sim_jointdynctrl_positioncb));
    ui->qqPositionMode_cb->setEnabled(dynamic&&(ctrlMode==sim_jointdynctrl_position));
    ui->qqCtrlMode_position->setChecked(dynamic&&(ctrlMode==sim_jointdynctrl_position));

    ui->qqCtrlMode_spring->setEnabled(canBeMotorized&&(!it->getHybridFunctionality_old()));
    ui->qqSpringMode_position->setEnabled(dynamic&&(ctrlMode==sim_jointdynctrl_spring));
    ui->qqSpringMode_force->setEnabled(dynamic&&(ctrlMode==sim_jointdynctrl_spring));
    ui->qqK->setEnabled(dynamic&&(ctrlMode==sim_jointdynctrl_spring));
    ui->qqC->setEnabled(dynamic&&(ctrlMode==sim_jointdynctrl_spring));
    ui->qqSpringMode_cb->setVisible(canBeMotorized&&(it->getDynCtrlMode()==sim_jointdynctrl_springcb));
    ui->qqSpringMode_cb->setEnabled(dynamic&&(ctrlMode==sim_jointdynctrl_spring));
    ui->qqCtrlMode_spring->setChecked(dynamic&&(ctrlMode==sim_jointdynctrl_spring));

    ui->qqCtrlMode_custom->setEnabled(canBeMotorized&&(!it->getHybridFunctionality_old()));
    ui->qqCtrlMode_custom->setChecked(dynamic&&(ctrlMode==sim_jointdynctrl_callback));

    if (dynamic&&(ctrlMode==sim_jointdynctrl_force))
        ui->qqForceMode_force->setText(tt::getEString(false,it->getTargetForce(true),4).c_str());
    else
        ui->qqForceMode_force->setText("");

    if (dynamic&&(ctrlMode==sim_jointdynctrl_velocity))
    {
        if (it->getJointType()==sim_joint_revolute_subtype)
            ui->qqVelocityMode_velocity->setText(tt::getAngleEString(true,it->getTargetVelocity(),4).c_str());
        else
            ui->qqVelocityMode_velocity->setText(tt::getEString(true,it->getTargetVelocity(),4).c_str());
        ui->qqVelocityMode_force->setText(tt::getEString(false,it->getTargetForce(true),4).c_str());
        ui->qqVelocityMode_motorLock->setChecked(it->getMotorLock());
    }
    else
    {
        ui->qqVelocityMode_velocity->setText("");
        ui->qqVelocityMode_force->setText("");
        ui->qqVelocityMode_motorLock->setChecked(false);
    }

    if (dynamic&&(ctrlMode==sim_jointdynctrl_position))
    {
        if (it->getJointType()==sim_joint_revolute_subtype)
            ui->qqPositionMode_position->setText(tt::getAngleEString(true,it->getTargetPosition(),4).c_str());
        else
            ui->qqPositionMode_position->setText(tt::getEString(true,it->getTargetPosition(),4).c_str());
        ui->qqPositionMode_force->setText(tt::getEString(false,it->getTargetForce(true),4).c_str());
        float maxVelAccelJerk[3];
        it->getMaxVelAccelJerk(maxVelAccelJerk);
        if (it->getJointType()==sim_joint_revolute_subtype)
            ui->qqPositionMode_upperVel->setText(tt::getAngleEString(false,maxVelAccelJerk[0],4).c_str());
        else
            ui->qqPositionMode_upperVel->setText(tt::getEString(false,maxVelAccelJerk[0],4).c_str());
        float pp,ip,dp;
        it->getPid(pp,ip,dp);
        ui->qqP->setText(tt::getFString(false,pp,3).c_str());
        ui->qqI->setText(tt::getFString(false,ip,3).c_str());
        ui->qqD->setText(tt::getFString(false,dp,3).c_str());
        ui->qqPositionMode_cb->setChecked(it->getDynCtrlMode()==sim_jointdynctrl_positioncb);
    }
    else
    {
        ui->qqPositionMode_position->setText("");
        ui->qqPositionMode_force->setText("");
        ui->qqPositionMode_upperVel->setText("");
        ui->qqP->setText("");
        ui->qqI->setText("");
        ui->qqD->setText("");
    }

    if (dynamic&&(ctrlMode==sim_jointdynctrl_spring))
    {
        if (it->getJointType()==sim_joint_revolute_subtype)
            ui->qqSpringMode_position->setText(tt::getAngleEString(true,it->getTargetPosition(),4).c_str());
        else
            ui->qqSpringMode_position->setText(tt::getEString(true,it->getTargetPosition(),4).c_str());
        ui->qqSpringMode_force->setText(tt::getEString(false,it->getTargetForce(true),4).c_str());
        float kp,cp;
        it->getKc(kp,cp);
        ui->qqK->setText(tt::getFString(false,kp,3).c_str());
        ui->qqC->setText(tt::getFString(false,cp,3).c_str());
        ui->qqSpringMode_cb->setChecked(it->getDynCtrlMode()==sim_jointdynctrl_springcb);
    }
    else
    {
        ui->qqSpringMode_position->setText("");
        ui->qqSpringMode_force->setText("");
        ui->qqK->setText("");
        ui->qqC->setText("");
    }
    selectLineEdit(lineEditToSelect);
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

void CQDlgJointDyn::on_qqCtrlMode_free_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(SET_JOINTCTRLMODE_JOINTDYNGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle(),sim_jointdynctrl_free);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgJointDyn::on_qqCtrlMode_force_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(SET_JOINTCTRLMODE_JOINTDYNGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle(),sim_jointdynctrl_force);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgJointDyn::on_qqCtrlMode_velocity_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(SET_JOINTCTRLMODE_JOINTDYNGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle(),sim_jointdynctrl_velocity);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgJointDyn::on_qqCtrlMode_position_clicked()
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

void CQDlgJointDyn::on_qqCtrlMode_spring_clicked()
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

void CQDlgJointDyn::on_qqCtrlMode_custom_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(SET_JOINTCTRLMODE_JOINTDYNGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle(),sim_jointdynctrl_callback);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgJointDyn::on_qqP_editingFinished()
{
    if (!ui->qqP->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        float newVal=ui->qqP->text().toFloat(&ok);
        CJoint* it=App::currentWorld->sceneObjects->getLastSelectionJoint();
        if (ok&&(it!=nullptr))
        {
            float pp,ip,dp;
            it->getPid(pp,ip,dp);
            SSimulationThreadCommand cmd;
            cmd.cmdId=SET_PIDVALUES_JOINTDYNGUITRIGGEREDCMD;
            cmd.intParams.push_back(it->getObjectHandle());
            cmd.floatParams.push_back(newVal);
            cmd.floatParams.push_back(ip);
            cmd.floatParams.push_back(dp);
            App::appendSimulationThreadCommand(cmd);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgJointDyn::on_qqI_editingFinished()
{
    if (!ui->qqI->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        float newVal=ui->qqI->text().toFloat(&ok);
        CJoint* it=App::currentWorld->sceneObjects->getLastSelectionJoint();
        if (ok&&(it!=nullptr))
        {
            float pp,ip,dp;
            it->getPid(pp,ip,dp);
            SSimulationThreadCommand cmd;
            cmd.cmdId=SET_PIDVALUES_JOINTDYNGUITRIGGEREDCMD;
            cmd.intParams.push_back(it->getObjectHandle());
            cmd.floatParams.push_back(pp);
            cmd.floatParams.push_back(newVal);
            cmd.floatParams.push_back(dp);
            App::appendSimulationThreadCommand(cmd);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgJointDyn::on_qqD_editingFinished()
{
    if (!ui->qqD->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        float newVal=ui->qqD->text().toFloat(&ok);
        CJoint* it=App::currentWorld->sceneObjects->getLastSelectionJoint();
        if (ok&&(it!=nullptr))
        {
            float pp,ip,dp;
            it->getPid(pp,ip,dp);
            SSimulationThreadCommand cmd;
            cmd.cmdId=SET_PIDVALUES_JOINTDYNGUITRIGGEREDCMD;
            cmd.intParams.push_back(it->getObjectHandle());
            cmd.floatParams.push_back(pp);
            cmd.floatParams.push_back(ip);
            cmd.floatParams.push_back(newVal);
            App::appendSimulationThreadCommand(cmd);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
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
        float newVal=ui->qqK->text().toFloat(&ok);
        if (ok&&(it!=nullptr))
        {
            float kp,cp;
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
        float newVal=ui->qqC->text().toFloat(&ok);
        if (ok&&(it!=nullptr))
        {
            float kp,cp;
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
        float newVal=ui->qqForceMode_force->text().toFloat(&ok);
        if (ok)
        {
            App::appendSimulationThreadCommand(SET_MAXFORCE_JOINTDYNGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle(),-1,newVal);
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
        float newVal=ui->qqVelocityMode_force->text().toFloat(&ok);
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
        float newVal=ui->qqVelocityMode_velocity->text().toFloat(&ok);
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
        float newVal=ui->qqPositionMode_position->text().toFloat(&ok);
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
        float newVal=ui->qqPositionMode_force->text().toFloat(&ok);
        if (ok)
        {
            App::appendSimulationThreadCommand(SET_MAXFORCE_JOINTDYNGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle(),-1,newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgJointDyn::on_qqPositionMode_upperVel_editingFinished()
{
    if (!ui->qqPositionMode_upperVel->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        CJoint* it=App::currentWorld->sceneObjects->getLastSelectionJoint();
        bool ok;
        float newVal=ui->qqPositionMode_upperVel->text().toFloat(&ok);
        if (ok&&(it!=nullptr))
        {
            if (it->getJointType()!=sim_joint_prismatic_subtype)
                newVal*=gv::userToRad;
            App::appendSimulationThreadCommand(SET_UPPERVELLIMIT_JOINTDYNGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle(),-1,newVal);
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
        float newVal=ui->qqSpringMode_position->text().toFloat(&ok);
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

void CQDlgJointDyn::on_qqSpringMode_force_editingFinished()
{
    if (!ui->qqSpringMode_force->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        float newVal=ui->qqSpringMode_force->text().toFloat(&ok);
        if (ok)
        {
            App::appendSimulationThreadCommand(SET_MAXFORCE_JOINTDYNGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle(),-1,newVal);
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
            CPropBrowserEngineJoint dlg(this);// App::mainWindow);
            dlg.setModal(true);
            dlg.exec(); // items are set in here
            // We however still need to modify the sim thread resources:
            SSimulationThreadCommand cmd;
            cmd.cmdId=SET_ALLENGINEPARAMS_JOINTDYNGUITRIGGEREDCMD;
            cmd.intParams.push_back(it->getObjectHandle());
            std::vector<int> iParams;
            std::vector<float> fParams;
            it->getBulletIntParams(iParams);
            it->getBulletFloatParams(fParams);
            cmd.intVectorParams.push_back(iParams);
            cmd.floatVectorParams.push_back(fParams);
            iParams.clear();
            fParams.clear();
            it->getOdeIntParams(iParams);
            it->getOdeFloatParams(fParams);
            cmd.intVectorParams.push_back(iParams);
            cmd.floatVectorParams.push_back(fParams);
            iParams.clear();
            fParams.clear();
            it->getVortexIntParams(iParams);
            it->getVortexFloatParams(fParams);
            cmd.intVectorParams.push_back(iParams);
            cmd.floatVectorParams.push_back(fParams);
            iParams.clear();
            fParams.clear();
            it->getNewtonIntParams(iParams);
            it->getNewtonFloatParams(fParams);
            cmd.intVectorParams.push_back(iParams);
            cmd.floatVectorParams.push_back(fParams);
            iParams.clear();
            fParams.clear();
            App::appendSimulationThreadCommand(cmd);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

