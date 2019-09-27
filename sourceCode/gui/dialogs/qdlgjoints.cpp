
#include "vrepMainHeader.h"
#include "qdlgjoints.h"
#include "ui_qdlgjoints.h"
#include "tt.h"
#include "gV.h"
#include "qdlgmaterial.h"
#include "app.h"
#include "qdlgdependencyequation.h"
#include "qdlgjointdyn.h"
#include "v_repStrings.h"

bool CQDlgJoints::showDynamicWindow=false;

CQDlgJoints::CQDlgJoints(QWidget *parent) :
    CDlgEx(parent),
    ui(new Ui::CQDlgJoints)
{
    _dlgType=JOINT_DLG;
    ui->setupUi(this);
    inMainRefreshRoutine=false;
}

CQDlgJoints::~CQDlgJoints()
{
    delete ui;
}

void CQDlgJoints::cancelEvent()
{
    // we override this cancel event. The container window should close, not this one!!
    App::mainWindow->dlgCont->close(OBJECT_DLG);
}

void CQDlgJoints::refresh()
{
    inMainRefreshRoutine=true;
    QLineEdit* lineEditToSelect=getSelectedLineEdit();
    bool noEditModeNoSim=(App::getEditModeType()==NO_EDIT_MODE)&&App::ct->simulation->isSimulationStopped();
    bool sel=App::ct->objCont->isLastSelectionAJoint();
    bool bigSel=(App::ct->objCont->isLastSelectionAJoint()&&(App::ct->objCont->getJointNumberInSelection()>1));
    bool revolute=false;
    bool prismatic=false;
    bool spherical=false;
    bool dynamic=false;
    CJoint* it=App::ct->objCont->getLastSelection_joint();
    if (sel)
    {
        revolute=(it->getJointType()==sim_joint_revolute_subtype);
        prismatic=(it->getJointType()==sim_joint_prismatic_subtype);
        spherical=(it->getJointType()==sim_joint_spherical_subtype);
        dynamic=((it->getJointMode()==sim_jointmode_force)||it->getHybridFunctionality() );
    }

    if (sel&&prismatic)
    {
        ui->qqPosMinLabel->setText("Pos. min. [m]");
        ui->qqPosRangeLabel->setText("Pos. range [m]");
        ui->qqPosLabel->setText("Position [m]");
        ui->qqStepSizeLabel->setText("Max. step size [m]");
    }
    else
    {
        ui->qqPosMinLabel->setText("Pos. min. [deg]");
        ui->qqPosRangeLabel->setText("Pos. range [deg]");
        ui->qqPosLabel->setText("Position [deg]");
        ui->qqStepSizeLabel->setText("Max. step size [deg]");
    }

    ui->qqCyclic->setEnabled(sel&&revolute&&noEditModeNoSim);
    ui->qqPitch->setEnabled(sel&&revolute&&noEditModeNoSim&&(!dynamic)&&(!it->getPositionIsCyclic()));
    ui->qqMinimum->setEnabled(sel&&(!spherical)&&(!it->getPositionIsCyclic())&&noEditModeNoSim);
    if (spherical)
        ui->qqRange->setEnabled(sel&&(!dynamic)&&noEditModeNoSim);
    else
        ui->qqRange->setEnabled(sel&&(!it->getPositionIsCyclic())&&noEditModeNoSim);
    ui->qqPosition->setEnabled(sel&&(!spherical));
    ui->qqIkWeight->setEnabled(sel&&noEditModeNoSim);
    ui->qqMaxStepSize->setEnabled(sel&&noEditModeNoSim);
    ui->qqApplyConfig->setEnabled(sel&&bigSel&&noEditModeNoSim);

    ui->qqJointModeCombo->setEnabled(sel&&noEditModeNoSim);
    ui->qqJointModeCombo->clear();
    ui->qqHybrid->setEnabled(sel&&noEditModeNoSim&&(it->getJointMode()!=sim_jointmode_force)&&(!spherical));
    ui->qqApplyMode->setEnabled(sel&&bigSel&&noEditModeNoSim);

    ui->qqAdjustDependency->setEnabled(sel&&noEditModeNoSim&&((it->getJointMode()==sim_jointmode_dependent)||(it->getJointMode()==sim_jointmode_reserved_previously_ikdependent)));
    ui->qqAdjustDynamicParameters->setEnabled(true); 
    ui->qqAdjustDynamicParameters->setChecked(showDynamicWindow);

    ui->qqLength->setEnabled(sel&&(!spherical)&&noEditModeNoSim);
    ui->qqDiameter->setEnabled(sel&&noEditModeNoSim);
    ui->qqAdjustColorA->setEnabled(sel&&noEditModeNoSim);
    ui->qqAdjustColorB->setEnabled(sel&&noEditModeNoSim);
    ui->qqApplyAppearance->setEnabled(sel&&bigSel&&noEditModeNoSim);

    ui->qqCyclic->setChecked(sel&&revolute&&it->getPositionIsCyclic());
    ui->qqHybrid->setChecked(sel&&it->getHybridFunctionality());

    if (sel)
    {
        if (revolute)
        {
            ui->qqPitch->setText(tt::getEString(true,it->getScrewPitch()*degToRad_f,2).c_str());
            if (it->getPositionIsCyclic())
            {
                ui->qqMinimum->setText("");
                ui->qqRange->setText("");
            }
            else
            {
                ui->qqMinimum->setText(tt::getAngleEString(true,it->getPositionIntervalMin(),3).c_str());
                ui->qqRange->setText(tt::getAngleEString(false,it->getPositionIntervalRange(),3).c_str());
            }
            ui->qqPosition->setText(tt::getAngleEString(true,it->getPosition(),3).c_str());
        }
        else
            ui->qqPitch->setText("");

        if (prismatic)
        {
            ui->qqMinimum->setText(tt::getEString(true,it->getPositionIntervalMin(),3).c_str());
            ui->qqRange->setText(tt::getEString(false,it->getPositionIntervalRange(),3).c_str());
            ui->qqPosition->setText(tt::getEString(true,it->getPosition(),3).c_str());
            ui->qqMaxStepSize->setText(tt::getEString(false,it->getMaxStepSize(),2).c_str());
        }
        else
            ui->qqMaxStepSize->setText(tt::getAngleEString(false,it->getMaxStepSize(),2).c_str());


        if (spherical)
        {
            ui->qqMinimum->setText("");
            if (dynamic)
                ui->qqRange->setText("");
            else
                ui->qqRange->setText(tt::getAngleEString(false,it->getPositionIntervalRange(),3).c_str());
            ui->qqPosition->setText("");
            C3Vector euler(it->getSphericalTransformation().getEulerAngles());
            ui->qqLength->setText("");
        }
        else
        {
            ui->qqLength->setText(tt::getFString(false,it->getLength(),3).c_str());
        }

        ui->qqIkWeight->setText(tt::getFString(false,it->getIKWeight(),2).c_str());
        ui->qqDiameter->setText(tt::getFString(false,it->getDiameter(),3).c_str());


        ui->qqJointModeCombo->addItem(strTranslate(IDSN_JOINT_IS_IN_PASSIVE_MODE),QVariant(sim_jointmode_passive));
        ui->qqJointModeCombo->addItem(strTranslate(IDSN_JOINT_IS_IN_IK_MODE),QVariant(sim_jointmode_ik));
        if (!spherical)
        {
            ui->qqJointModeCombo->addItem(strTranslate(IDSN_JOINT_IS_IN_DEPENDENT_MODE),QVariant(sim_jointmode_dependent));
            ui->qqJointModeCombo->addItem(strTranslate(IDSN_JOINT_IS_IN_MOTION_MODE),QVariant(sim_jointmode_motion_deprecated));
        }
        ui->qqJointModeCombo->addItem(strTranslate(IDSN_JOINT_IS_IN_TORQUE_FORCE_MODE),QVariant(sim_jointmode_force));
        for (int i=0;i<ui->qqJointModeCombo->count();i++)
        {
            if (ui->qqJointModeCombo->itemData(i).toInt()==it->getJointMode())
            {
                ui->qqJointModeCombo->setCurrentIndex(i);
                break;
            }
        }

    }
    else
    {
        ui->qqPitch->setText("");
        ui->qqMinimum->setText("");
        ui->qqRange->setText("");
        ui->qqPosition->setText("");
        ui->qqIkWeight->setText("");
        ui->qqMaxStepSize->setText("");
        ui->qqLength->setText("");
        ui->qqDiameter->setText("");
    }

    VDialog* dlg=App::mainWindow->dlgCont->getDialog(JOINT_DYN_DLG);
    if (dlg!=nullptr)
        ((CQDlgJointDyn*)dlg)->refresh();

    selectLineEdit(lineEditToSelect);
    inMainRefreshRoutine=false;
}

void CQDlgJoints::on_qqCyclic_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_CYCLIC_JOINTGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgJoints::on_qqPitch_editingFinished()
{
    if (!ui->qqPitch->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        float newVal=ui->qqPitch->text().toFloat(&ok);
        if (ok)
        {
            App::appendSimulationThreadCommand(SET_PITCH_JOINTGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),-1,newVal/gv::userToRad);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgJoints::on_qqMinimum_editingFinished()
{
    if (!ui->qqMinimum->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        float newVal=ui->qqMinimum->text().toFloat(&ok);
        CJoint* it=App::ct->objCont->getLastSelection_joint();
        if (ok&&(it!=nullptr))
        {
            if (it->getJointType()!=sim_joint_prismatic_subtype)
                newVal*=gv::userToRad;
            App::appendSimulationThreadCommand(SET_MINPOS_JOINTGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),-1,newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgJoints::on_qqRange_editingFinished()
{
    if (!ui->qqRange->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        float newVal=ui->qqRange->text().toFloat(&ok);
        CJoint* it=App::ct->objCont->getLastSelection_joint();
        if (ok&&(it!=nullptr))
        {
            if (it->getJointType()!=sim_joint_prismatic_subtype)
                newVal*=gv::userToRad;
            App::appendSimulationThreadCommand(SET_RANGE_JOINTGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),-1,newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgJoints::on_qqPosition_editingFinished()
{
    if (!ui->qqPosition->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        float newVal=ui->qqPosition->text().toFloat(&ok);
        CJoint* it=App::ct->objCont->getLastSelection_joint();
        if (ok&&(it!=nullptr))
        {
            if (it->getJointType()!=sim_joint_prismatic_subtype)
                newVal*=gv::userToRad;
            App::appendSimulationThreadCommand(SET_POS_JOINTGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),-1,newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgJoints::on_qqIkWeight_editingFinished()
{
    if (!ui->qqIkWeight->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        float newVal=ui->qqIkWeight->text().toFloat(&ok);
        if (ok)
        {
            App::appendSimulationThreadCommand(SET_IKWEIGHT_JOINTGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),-1,newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgJoints::on_qqMaxStepSize_editingFinished()
{
    if (!ui->qqMaxStepSize->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        float newVal=ui->qqMaxStepSize->text().toFloat(&ok);
        CJoint* it=App::ct->objCont->getLastSelection_joint();
        if (ok&&(it!=nullptr))
        {
            if (it->getJointType()!=sim_joint_prismatic_subtype)
                newVal*=gv::userToRad;
            App::appendSimulationThreadCommand(SET_MAXSTEPSIZE_JOINTGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),-1,newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgJoints::on_qqApplyConfig_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        SSimulationThreadCommand cmd;
        cmd.cmdId=APPLY_CONFIGPARAMS_JOINTGUITRIGGEREDCMD;
        cmd.intParams.push_back(App::ct->objCont->getLastSelectionID());
        for (int i=0;i<App::ct->objCont->getSelSize()-1;i++)
            cmd.intParams.push_back(App::ct->objCont->getSelID(i));
        App::appendSimulationThreadCommand(cmd);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgJoints::on_qqJointModeCombo_currentIndexChanged(int index)
{
    if (!inMainRefreshRoutine)
    {
        IF_UI_EVENT_CAN_READ_DATA
        {
            int mode=ui->qqJointModeCombo->itemData(ui->qqJointModeCombo->currentIndex()).toInt();
            App::appendSimulationThreadCommand(SET_MODE_JOINTGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),mode);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgJoints::on_qqHybrid_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_HYBRID_JOINTGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgJoints::on_qqApplyMode_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        SSimulationThreadCommand cmd;
        cmd.cmdId=APPLY_MODEPARAMS_JOINTGUITRIGGEREDCMD;
        cmd.intParams.push_back(App::ct->objCont->getLastSelectionID());
        for (int i=0;i<App::ct->objCont->getSelSize()-1;i++)
            cmd.intParams.push_back(App::ct->objCont->getSelID(i));
        App::appendSimulationThreadCommand(cmd);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgJoints::on_qqAdjustDependency_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (App::ct->objCont->isLastSelectionAJoint())
        {
            CQDlgDependencyEquation theDialog(this);
            theDialog.refresh();
            theDialog.makeDialogModal(); // modifications made directly!
        }
    }
}

void CQDlgJoints::on_qqAdjustDynamicParameters_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        showDynamicWindow=!showDynamicWindow;
        if (App::mainWindow->dlgCont->isVisible(JOINT_DYN_DLG)!=showDynamicWindow)
            App::mainWindow->dlgCont->toggle(JOINT_DYN_DLG);
    }
}

void CQDlgJoints::on_qqLength_editingFinished()
{
    if (!ui->qqLength->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        float newVal=ui->qqLength->text().toFloat(&ok);
        if (ok)
        {
            App::appendSimulationThreadCommand(SET_LENGTH_JOINTGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),-1,newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgJoints::on_qqDiameter_editingFinished()
{
    if (!ui->qqDiameter->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        float newVal=ui->qqDiameter->text().toFloat(&ok);
        if (ok)
        {
            App::appendSimulationThreadCommand(SET_DIAMETER_JOINTGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),-1,newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgJoints::on_qqAdjustColorA_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CQDlgMaterial::displayMaterialDlg(COLOR_ID_JOINT_A,App::ct->objCont->getLastSelectionID(),-1,App::mainWindow);
    }
}

void CQDlgJoints::on_qqAdjustColorB_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CQDlgMaterial::displayMaterialDlg(COLOR_ID_JOINT_B,App::ct->objCont->getLastSelectionID(),-1,App::mainWindow);
    }
}

void CQDlgJoints::on_qqApplyAppearance_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        SSimulationThreadCommand cmd;
        cmd.cmdId=APPLY_VISUALPARAMS_JOINTGUITRIGGEREDCMD;
        cmd.intParams.push_back(App::ct->objCont->getLastSelectionID());
        for (int i=0;i<App::ct->objCont->getSelSize()-1;i++)
            cmd.intParams.push_back(App::ct->objCont->getSelID(i));
        App::appendSimulationThreadCommand(cmd);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}
