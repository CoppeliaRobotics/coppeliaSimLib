#include "qdlgjoints.h"
#include "ui_qdlgjoints.h"
#include "tt.h"
#include "gV.h"
#include "qdlgmaterial.h"
#include "app.h"
#include "qdlgdependencyequation.h"
#include "qdlgjointdyn.h"
#include "simStrings.h"

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
    bool noEditModeNoSim=(App::getEditModeType()==NO_EDIT_MODE)&&App::currentWorld->simulation->isSimulationStopped();
    bool sel=App::currentWorld->sceneObjects->isLastSelectionAJoint();
    bool bigSel=(App::currentWorld->sceneObjects->isLastSelectionAJoint()&&(App::currentWorld->sceneObjects->getJointCountInSelection()>1));
    bool revolute=false;
    bool prismatic=false;
    bool spherical=false;
    bool dynamic=false;
    CJoint* it=App::currentWorld->sceneObjects->getLastSelectionJoint();
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
    }
    else
    {
        ui->qqPosMinLabel->setText("Pos. min. [deg]");
        ui->qqPosRangeLabel->setText("Pos. range [deg]");
        ui->qqPosLabel->setText("Position [deg]");
    }

    ui->qqCyclic->setEnabled(sel&&revolute&&noEditModeNoSim);
    ui->qqPitch->setEnabled(sel&&revolute&&noEditModeNoSim&&(!dynamic)&&(!it->getPositionIsCyclic()));
    ui->qqMinimum->setEnabled(sel&&(!spherical)&&(!it->getPositionIsCyclic())&&noEditModeNoSim);
    if (spherical)
        ui->qqRange->setEnabled(sel&&(!dynamic)&&noEditModeNoSim);
    else
        ui->qqRange->setEnabled(sel&&(!it->getPositionIsCyclic())&&noEditModeNoSim);
    ui->qqPosition->setEnabled(sel&&(!spherical));
    ui->qqApplyConfig->setEnabled(sel&&bigSel&&noEditModeNoSim);

    ui->qqJointModeCombo->setEnabled(sel&&noEditModeNoSim);
    ui->qqJointModeCombo->clear();
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
        }


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

        ui->qqDiameter->setText(tt::getFString(false,it->getDiameter(),3).c_str());

        ui->qqJointModeCombo->addItem(IDSN_JOINT_IS_IN_PASSIVE_MODE,QVariant(sim_jointmode_passive));
        if ( ( (it->getJointMode()==sim_jointmode_passive)&&(it->getHybridFunctionality()) )||App::userSettings->showOldCalcModuleDlgs )
            ui->qqJointModeCombo->addItem(IDSN_JOINT_IS_IN_HYBRID_PASSIVE_MODE,QVariant(sim_jointmode_passive|sim_jointmode_hybrid_deprecated));

        if (!spherical)
        {
            ui->qqJointModeCombo->addItem(IDSN_JOINT_IS_IN_DEPENDENT_MODE,QVariant(sim_jointmode_dependent));
            if ( ( (it->getJointMode()==sim_jointmode_dependent)&&(it->getHybridFunctionality()) )||App::userSettings->showOldCalcModuleDlgs )
                ui->qqJointModeCombo->addItem(IDSN_JOINT_IS_IN_HYBRID_DEPENDENT_MODE,QVariant(sim_jointmode_dependent|sim_jointmode_hybrid_deprecated));
        }

        if ( (it->getJointMode()==sim_jointmode_ik_deprecated)||App::userSettings->showOldCalcModuleDlgs )
        {
            if (App::userSettings->showOldCalcModuleDlgs)
            {
                ui->qqJointModeCombo->addItem(IDSN_JOINT_IS_IN_IK_MODE,QVariant(sim_jointmode_ik_deprecated));
                ui->qqJointModeCombo->addItem(IDSN_JOINT_IS_IN_HYBRID_IK_MODE,QVariant(sim_jointmode_ik_deprecated|sim_jointmode_hybrid_deprecated));
            }
            else
            {
                if (it->getHybridFunctionality())
                    ui->qqJointModeCombo->addItem(IDSN_JOINT_IS_IN_HYBRID_IK_MODE,QVariant(sim_jointmode_ik_deprecated|sim_jointmode_hybrid_deprecated));
                else
                    ui->qqJointModeCombo->addItem(IDSN_JOINT_IS_IN_IK_MODE,QVariant(sim_jointmode_ik_deprecated));
            }
        }

        ui->qqJointModeCombo->addItem(IDSN_JOINT_IS_IN_TORQUE_FORCE_MODE,QVariant(sim_jointmode_force));

        if ( (!spherical)&&(it->getJointMode()==sim_jointmode_motion_deprecated) )
        {
            if (it->getHybridFunctionality())
                ui->qqJointModeCombo->addItem(IDSN_JOINT_IS_IN_HYBRID_MOTION_MODE,QVariant(sim_jointmode_motion_deprecated|sim_jointmode_hybrid_deprecated));
            else
                ui->qqJointModeCombo->addItem(IDSN_JOINT_IS_IN_MOTION_MODE,QVariant(sim_jointmode_motion_deprecated));
        }
/*
        ui->qqJointModeCombo->addItem(IDSN_JOINT_IS_IN_PASSIVE_MODE,QVariant(sim_jointmode_passive));
        ui->qqJointModeCombo->addItem(IDSN_JOINT_IS_IN_IK_MODE,QVariant(sim_jointmode_ik_deprecated));
        if (!spherical)
        {
            ui->qqJointModeCombo->addItem(IDSN_JOINT_IS_IN_DEPENDENT_MODE,QVariant(sim_jointmode_dependent));
            ui->qqJointModeCombo->addItem(IDSN_JOINT_IS_IN_MOTION_MODE,QVariant(sim_jointmode_motion_deprecated));
        }
        ui->qqJointModeCombo->addItem(IDSN_JOINT_IS_IN_TORQUE_FORCE_MODE,QVariant(sim_jointmode_force));
        */
        for (int i=0;i<ui->qqJointModeCombo->count();i++)
        {
            int val=0;
            if (it->getHybridFunctionality())
                val=sim_jointmode_hybrid_deprecated;
            if (ui->qqJointModeCombo->itemData(i).toInt()==(it->getJointMode()|val))
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
        App::appendSimulationThreadCommand(TOGGLE_CYCLIC_JOINTGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle());
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
            App::appendSimulationThreadCommand(SET_PITCH_JOINTGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle(),-1,newVal/gv::userToRad);
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
        CJoint* it=App::currentWorld->sceneObjects->getLastSelectionJoint();
        if (ok&&(it!=nullptr))
        {
            if (it->getJointType()!=sim_joint_prismatic_subtype)
                newVal*=gv::userToRad;
            App::appendSimulationThreadCommand(SET_MINPOS_JOINTGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle(),-1,newVal);
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
        CJoint* it=App::currentWorld->sceneObjects->getLastSelectionJoint();
        if (ok&&(it!=nullptr))
        {
            if (it->getJointType()!=sim_joint_prismatic_subtype)
                newVal*=gv::userToRad;
            App::appendSimulationThreadCommand(SET_RANGE_JOINTGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle(),-1,newVal);
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
        CJoint* it=App::currentWorld->sceneObjects->getLastSelectionJoint();
        if (ok&&(it!=nullptr))
        {
            if (it->getJointType()!=sim_joint_prismatic_subtype)
                newVal*=gv::userToRad;
            App::appendSimulationThreadCommand(SET_POS_JOINTGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle(),-1,newVal);
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
        cmd.intParams.push_back(App::currentWorld->sceneObjects->getLastSelectionHandle());
        for (size_t i=0;i<App::currentWorld->sceneObjects->getSelectionCount()-1;i++)
            cmd.intParams.push_back(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
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
            App::appendSimulationThreadCommand(SET_MODE_JOINTGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle(),mode);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgJoints::on_qqApplyMode_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        SSimulationThreadCommand cmd;
        cmd.cmdId=APPLY_MODEPARAMS_JOINTGUITRIGGEREDCMD;
        cmd.intParams.push_back(App::currentWorld->sceneObjects->getLastSelectionHandle());
        for (size_t i=0;i<App::currentWorld->sceneObjects->getSelectionCount()-1;i++)
            cmd.intParams.push_back(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
        App::appendSimulationThreadCommand(cmd);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgJoints::on_qqAdjustDependency_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (App::currentWorld->sceneObjects->isLastSelectionAJoint())
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
            App::appendSimulationThreadCommand(SET_LENGTH_JOINTGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle(),-1,newVal);
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
            App::appendSimulationThreadCommand(SET_DIAMETER_JOINTGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle(),-1,newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgJoints::on_qqAdjustColorA_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CQDlgMaterial::displayMaterialDlg(COLOR_ID_JOINT_A,App::currentWorld->sceneObjects->getLastSelectionHandle(),-1,App::mainWindow);
    }
}

void CQDlgJoints::on_qqAdjustColorB_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CQDlgMaterial::displayMaterialDlg(COLOR_ID_JOINT_B,App::currentWorld->sceneObjects->getLastSelectionHandle(),-1,App::mainWindow);
    }
}

void CQDlgJoints::on_qqApplyAppearance_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        SSimulationThreadCommand cmd;
        cmd.cmdId=APPLY_VISUALPARAMS_JOINTGUITRIGGEREDCMD;
        cmd.intParams.push_back(App::currentWorld->sceneObjects->getLastSelectionHandle());
        for (size_t i=0;i<App::currentWorld->sceneObjects->getSelectionCount()-1;i++)
            cmd.intParams.push_back(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
        App::appendSimulationThreadCommand(cmd);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}
