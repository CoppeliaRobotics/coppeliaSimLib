#include "vrepMainHeader.h"
#include "qdlgjointdyn.h"
#include "ui_qdlgjointdyn.h"
#include "tt.h"
#include "gV.h"
#include "propBrowser_engineProp_joint.h"
#include "qdlgjoints.h"
#include "app.h"
#include "v_repStringTable.h"
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
    bool noEditModeNoSim=(App::getEditModeType()==NO_EDIT_MODE)&&App::ct->simulation->isSimulationStopped();

    bool sel=App::ct->objCont->isLastSelectionAJoint();
    bool bigSel=(App::ct->objCont->isLastSelectionAJoint()&&(App::ct->objCont->getJointNumberInSelection()>1));
    bool revolute=false;
    bool prismatic=false;
    bool spherical=false;
    bool dynamic=false;
    bool dynamicMotControlAllowed=false;
    bool motorEnabled=false;

    bool ctrlEnabled=false;
    bool pidCtrlEnabled=true;
    bool springCtrlEnabled=false;
    CJoint* it=nullptr;
    if (sel)
    {
        it=App::ct->objCont->getLastSelection_joint();
        revolute=(it->getJointType()==sim_joint_revolute_subtype);
        prismatic=(it->getJointType()==sim_joint_prismatic_subtype);
        spherical=(it->getJointType()==sim_joint_spherical_subtype);
        dynamic=((it->getJointMode()==sim_jointmode_force)||it->getHybridFunctionality() );
        dynamicMotControlAllowed=(dynamic&&(!spherical));
        motorEnabled=dynamic&&it->getEnableDynamicMotor();
        ctrlEnabled=motorEnabled&&it->getEnableDynamicMotorControlLoop();
        pidCtrlEnabled=ctrlEnabled&&(!it->getEnableTorqueModulation());
        springCtrlEnabled=ctrlEnabled&&it->getEnableTorqueModulation();
    }

    if (sel&&prismatic)
    {
        ui->qqTargetVelocityLabel->setText("Target velocity [m/s]");
        ui->qqMaxForceLabel->setText("Maximum force [N]");
        ui->qqTargetPositionLabel->setText("Target position [m]");
        ui->qqUpperVelocityLabel->setText("Upper velocity limit [m/s]");
        ui->qqKLabel->setText("Spring constant K [N/m]");
        ui->qqCLabel->setText("Damping coefficient C [N*s/m]");
    }
    else
    {
        ui->qqTargetVelocityLabel->setText("Target velocity [deg/s]");
        ui->qqMaxForceLabel->setText("Maximum torque [N*m]");
        ui->qqTargetPositionLabel->setText("Target position [deg]");
        ui->qqUpperVelocityLabel->setText("Upper velocity limit [deg/s]");
        ui->qqKLabel->setText("Spring constant K [N]");
        ui->qqCLabel->setText("Damping coefficient C [N*s]");
    }

    ui->qqMotorEnabled->setEnabled(dynamicMotControlAllowed&&(!it->getHybridFunctionality()));
    ui->qqTargetVelocity->setEnabled(dynamicMotControlAllowed&&motorEnabled&&((!ctrlEnabled)||(!pidCtrlEnabled)));
    ui->qqMotorLockEnabled->setEnabled(dynamicMotControlAllowed&&motorEnabled&&(!ctrlEnabled));
    ui->qqMaxForce->setEnabled(dynamicMotControlAllowed&&motorEnabled);
    ui->qqAdjustEngineProperties->setEnabled(dynamic&&noEditModeNoSim);
    ui->qqApplyDynamicProperties->setEnabled(dynamic&&bigSel&&noEditModeNoSim);

    ui->qqControlEnabled->setEnabled(dynamicMotControlAllowed&&motorEnabled&&(!it->getHybridFunctionality()));
    ui->qqPositionControl->setEnabled(dynamicMotControlAllowed&&ctrlEnabled&&noEditModeNoSim);
    ui->qqSpringControl->setEnabled(dynamicMotControlAllowed&&ctrlEnabled&&noEditModeNoSim);

    ui->qqVelocityUpperLimit->setEnabled(dynamicMotControlAllowed&&ctrlEnabled&&noEditModeNoSim&&(!springCtrlEnabled));
    ui->qqTargetPosition->setEnabled(dynamicMotControlAllowed&&ctrlEnabled&&(!it->getHybridFunctionality()));
    ui->qqP->setEnabled(dynamicMotControlAllowed&&ctrlEnabled&&pidCtrlEnabled);
    ui->qqI->setEnabled(dynamicMotControlAllowed&&ctrlEnabled&&pidCtrlEnabled);
    ui->qqD->setEnabled(dynamicMotControlAllowed&&ctrlEnabled&&pidCtrlEnabled);

    ui->qqK->setEnabled(dynamicMotControlAllowed&&ctrlEnabled&&springCtrlEnabled);
    ui->qqC->setEnabled(dynamicMotControlAllowed&&ctrlEnabled&&springCtrlEnabled);

    ui->qqApplyControlParameters->setEnabled(dynamicMotControlAllowed&&bigSel&&noEditModeNoSim);

    ui->qqMotorEnabled->setChecked(dynamic&&it->getEnableDynamicMotor());
    ui->qqControlEnabled->setChecked(dynamic&&it->getEnableDynamicMotorControlLoop());

    if (dynamicMotControlAllowed)
    {
        if (ctrlEnabled)
        {
            if (!pidCtrlEnabled)
            {
                if (it->getJointType()==sim_joint_revolute_subtype)
                    ui->qqTargetVelocity->setText(tt::getAngleEString(true,it->getDynamicMotorTargetVelocity(),4).c_str());
                else
                    ui->qqTargetVelocity->setText(tt::getEString(true,it->getDynamicMotorTargetVelocity(),4).c_str());
            }
            else
                ui->qqTargetVelocity->setText("");
            ui->qqMotorLockEnabled->setChecked(false);
        }
        else
        {
            if (it->getJointType()==sim_joint_revolute_subtype)
                ui->qqTargetVelocity->setText(tt::getAngleEString(true,it->getDynamicMotorTargetVelocity(),4).c_str());
            else
                ui->qqTargetVelocity->setText(tt::getEString(true,it->getDynamicMotorTargetVelocity(),4).c_str());
            ui->qqMotorLockEnabled->setChecked(it->getDynamicMotorLockModeWhenInVelocityControl());
        }

        if (it->getJointType()==sim_joint_revolute_subtype)
        {
            ui->qqMaxForce->setText(tt::getEString(false,it->getDynamicMotorMaximumForce(),4).c_str());

            if (ctrlEnabled&&(!springCtrlEnabled))
                ui->qqVelocityUpperLimit->setText(tt::getAngleEString(false,it->getDynamicMotorUpperLimitVelocity(),4).c_str());
            else
                ui->qqVelocityUpperLimit->setText("");
            if (it->getHybridFunctionality())
                ui->qqTargetPosition->setText("");
            else
                ui->qqTargetPosition->setText(tt::getAngleEString(true,it->getDynamicMotorPositionControlTargetPosition(),4).c_str());
        }
        else
        {
            ui->qqMaxForce->setText(tt::getEString(false,it->getDynamicMotorMaximumForce(),4).c_str());
            if (ctrlEnabled&&(!springCtrlEnabled))
                ui->qqVelocityUpperLimit->setText(tt::getEString(false,it->getDynamicMotorUpperLimitVelocity(),4).c_str());
            else
                ui->qqVelocityUpperLimit->setText("");
            if (it->getHybridFunctionality())
                ui->qqTargetPosition->setText("");
            else
                ui->qqTargetPosition->setText(tt::getEString(true,it->getDynamicMotorPositionControlTargetPosition(),4).c_str());
        }

        float pp,ip,dp;
        it->getDynamicMotorPositionControlParameters(pp,ip,dp);
        ui->qqP->setText(tt::getFString(false,pp,3).c_str());
        ui->qqI->setText(tt::getFString(false,ip,3).c_str());
        ui->qqD->setText(tt::getFString(false,dp,3).c_str());

        float kp,cp;
        it->getDynamicMotorSpringControlParameters(kp,cp);
        ui->qqK->setText(tt::getEString(false,kp,3).c_str());
        ui->qqC->setText(tt::getEString(false,cp,3).c_str());


        ui->qqPositionControl->setChecked(pidCtrlEnabled);
        ui->qqSpringControl->setChecked(springCtrlEnabled);
    }
    else
    {
        ui->qqTargetVelocity->setText("");
        ui->qqMotorLockEnabled->setChecked(false);
        ui->qqMaxForce->setText("");
        ui->qqVelocityUpperLimit->setText("");
        ui->qqTargetPosition->setText("");
        ui->qqP->setText("");
        ui->qqI->setText("");
        ui->qqD->setText("");
        ui->qqK->setText("");
        ui->qqC->setText("");
    }
    selectLineEdit(lineEditToSelect);
}

void CQDlgJointDyn::on_qqMotorEnabled_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_MOTORENABLED_JOINTDYNGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgJointDyn::on_qqTargetVelocity_editingFinished()
{
    if (!ui->qqTargetVelocity->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        CJoint* it=App::ct->objCont->getLastSelection_joint();
        bool ok;
        float newVal=ui->qqTargetVelocity->text().toFloat(&ok);
        if (ok&&(it!=nullptr))
        {
            if (it->getJointType()!=sim_joint_prismatic_subtype)
                newVal*=gv::userToAngularVel;
            App::appendSimulationThreadCommand(SET_TARGETVELOCITY_JOINTDYNGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),-1,newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgJointDyn::on_qqAdjustEngineProperties_clicked()
{
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        CJoint* it=App::ct->objCont->getLastSelection_joint();
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

void CQDlgJointDyn::on_qqMaxForce_editingFinished()
{
    if (!ui->qqMaxForce->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        float newVal=ui->qqMaxForce->text().toFloat(&ok);
        if (ok)
        {
            App::appendSimulationThreadCommand(SET_MAXFORCE_JOINTDYNGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),-1,newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgJointDyn::on_qqApplyDynamicProperties_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        SSimulationThreadCommand cmd;
        cmd.cmdId=APPLY_MOTORPARAMS_JOINTDYNGUITRIGGEREDCMD;
        cmd.intParams.push_back(App::ct->objCont->getLastSelectionID());
        for (int i=0;i<App::ct->objCont->getSelSize()-1;i++)
            cmd.intParams.push_back(App::ct->objCont->getSelID(i));
        App::appendSimulationThreadCommand(cmd);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgJointDyn::on_qqControlEnabled_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_CTRLLOOP_JOINTDYNGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgJointDyn::on_qqVelocityUpperLimit_editingFinished()
{
    if (!ui->qqVelocityUpperLimit->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        CJoint* it=App::ct->objCont->getLastSelection_joint();
        bool ok;
        float newVal=ui->qqVelocityUpperLimit->text().toFloat(&ok);
        if (ok&&(it!=nullptr))
        {
            if (it->getJointType()!=sim_joint_prismatic_subtype)
                newVal*=gv::userToRad;
            App::appendSimulationThreadCommand(SET_UPPERVELLIMIT_JOINTDYNGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),-1,newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgJointDyn::on_qqTargetPosition_editingFinished()
{
    if (!ui->qqTargetPosition->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        CJoint* it=App::ct->objCont->getLastSelection_joint();
        bool ok;
        float newVal=ui->qqTargetPosition->text().toFloat(&ok);
        if (ok&&(it!=nullptr))
        {
            if (it->getJointType()!=sim_joint_prismatic_subtype)
                newVal*=gv::userToRad;
            App::appendSimulationThreadCommand(SET_TARGETPOSITION_JOINTDYNGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),-1,newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
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
        CJoint* it=App::ct->objCont->getLastSelection_joint();
        if (ok&&(it!=nullptr))
        {
            float pp,ip,dp;
            it->getDynamicMotorPositionControlParameters(pp,ip,dp);
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
        CJoint* it=App::ct->objCont->getLastSelection_joint();
        if (ok&&(it!=nullptr))
        {
            float pp,ip,dp;
            it->getDynamicMotorPositionControlParameters(pp,ip,dp);
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
        CJoint* it=App::ct->objCont->getLastSelection_joint();
        if (ok&&(it!=nullptr))
        {
            float pp,ip,dp;
            it->getDynamicMotorPositionControlParameters(pp,ip,dp);
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

void CQDlgJointDyn::on_qqApplyControlParameters_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        SSimulationThreadCommand cmd;
        cmd.cmdId=APPLY_CTRLPARAMS_JOINTDYNGUITRIGGEREDCMD;
        cmd.intParams.push_back(App::ct->objCont->getLastSelectionID());
        for (int i=0;i<App::ct->objCont->getSelSize()-1;i++)
            cmd.intParams.push_back(App::ct->objCont->getSelID(i));
        App::appendSimulationThreadCommand(cmd);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgJointDyn::on_qqPositionControl_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CJoint* it=App::ct->objCont->getLastSelection_joint();
        if (it!=nullptr)
        {
            App::appendSimulationThreadCommand(SELECT_PIDCTRL_JOINTDYNGUITRIGGEREDCMD,it->getObjectHandle());
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgJointDyn::on_qqSpringControl_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CJoint* it=App::ct->objCont->getLastSelection_joint();
        if (it!=nullptr)
        {
            App::appendSimulationThreadCommand(SELECT_SPRINGDAMPERCTRL_JOINTDYNGUITRIGGEREDCMD,it->getObjectHandle());
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
        CJoint* it=App::ct->objCont->getLastSelection_joint();
        bool ok;
        float newVal=ui->qqK->text().toFloat(&ok);
        if (ok&&(it!=nullptr))
        {
            float kp,cp;
            it->getDynamicMotorSpringControlParameters(kp,cp);
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
        CJoint* it=App::ct->objCont->getLastSelection_joint();
        bool ok;
        float newVal=ui->qqC->text().toFloat(&ok);
        if (ok&&(it!=nullptr))
        {
            float kp,cp;
            it->getDynamicMotorSpringControlParameters(kp,cp);
            App::appendSimulationThreadCommand(SET_KCVALUES_JOINTDYNGUITRIGGEREDCMD,it->getObjectHandle(),-1,kp,newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgJointDyn::on_qqMotorLockEnabled_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_LOCKMOTOR_JOINTDYNGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}
