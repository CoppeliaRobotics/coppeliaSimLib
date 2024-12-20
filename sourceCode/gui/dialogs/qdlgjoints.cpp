#include <qdlgjoints.h>
#include <ui_qdlgjoints.h>
#include <tt.h>
#include <utils.h>
#include <qdlgmaterial.h>
#include <app.h>
#include <qdlgdependencyequation.h>
#include <qdlgjointdyn.h>
#include <simStrings.h>
#include <guiApp.h>

bool CQDlgJoints::showDynamicWindow = false;

CQDlgJoints::CQDlgJoints(QWidget* parent)
    : CDlgEx(parent), ui(new Ui::CQDlgJoints)
{
    _dlgType = JOINT_DLG;
    ui->setupUi(this);
    inMainRefreshRoutine = false;
}

CQDlgJoints::~CQDlgJoints()
{
    delete ui;
}

void CQDlgJoints::cancelEvent()
{
    // we override this cancel event. The container window should close, not this one!!
    GuiApp::mainWindow->dlgCont->close(OBJECT_DLG);
}

void CQDlgJoints::refresh()
{
    inMainRefreshRoutine = true;
    QLineEdit* lineEditToSelect = getSelectedLineEdit();
    bool noEditModeNoSim =
        (GuiApp::getEditModeType() == NO_EDIT_MODE) && App::currentWorld->simulation->isSimulationStopped();
    bool sel = App::currentWorld->sceneObjects->isLastSelectionOfType(sim_sceneobject_joint);
    bool bigSel = (App::currentWorld->sceneObjects->isLastSelectionOfType(sim_sceneobject_joint) &&
                   (App::currentWorld->sceneObjects->getObjectCountInSelection(sim_sceneobject_joint) > 1));
    bool revolute = false;
    bool prismatic = false;
    bool spherical = false;
    bool dynamic = false;
    CJoint* it = App::currentWorld->sceneObjects->getLastSelectionJoint();
    if (sel)
    {
        revolute = (it->getJointType() == sim_joint_revolute);
        prismatic = (it->getJointType() == sim_joint_prismatic);
        spherical = (it->getJointType() == sim_joint_spherical);
        dynamic = ((it->getJointMode() == sim_jointmode_dynamic) || it->getHybridFunctionality_old());
    }

    if (sel && prismatic)
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

    ui->qqCyclic->setEnabled(sel && revolute && noEditModeNoSim);
    ui->qqLead->setEnabled(sel && revolute && noEditModeNoSim && (!dynamic) && (!it->getIsCyclic()));
    ui->qqMinimum->setEnabled(sel && (!spherical) && (!it->getIsCyclic()) && noEditModeNoSim);
    ui->qqRange->setEnabled((!spherical) && sel && (!it->getIsCyclic()) && noEditModeNoSim);
    ui->qqPosition->setEnabled(sel && (!spherical));
    ui->qqApplyConfig->setEnabled(sel && bigSel && noEditModeNoSim);

    ui->qqJointModeCombo->setEnabled(sel && noEditModeNoSim);
    ui->qqJointModeCombo->clear();
    ui->qqApplyMode->setEnabled(sel && bigSel && noEditModeNoSim);

    ui->qqAdjustDependency->setEnabled(sel && noEditModeNoSim &&
                                       ((it->getJointMode() == sim_jointmode_dependent) ||
                                        (it->getJointMode() == sim_jointmode_reserved_previously_ikdependent)));
    ui->qqAdjustDynamicParameters->setEnabled(true);
    ui->qqAdjustDynamicParameters->setChecked(showDynamicWindow);

    ui->qqLength->setEnabled(sel && (!spherical) && noEditModeNoSim);
    ui->qqDiameter->setEnabled(sel && noEditModeNoSim);
    ui->qqAdjustColorA->setEnabled(sel && noEditModeNoSim);
    ui->qqApplyAppearance->setEnabled(sel && bigSel && noEditModeNoSim);

    ui->qqCyclic->setChecked(sel && revolute && it->getIsCyclic());

    if (sel)
    {
        double minP, maxP;
        it->getInterval(minP, maxP);
        if (revolute)
        {
            ui->qqLead->setText(utils::getPosString(true, it->getScrewLead()).c_str());
            if (it->getIsCyclic())
            {
                ui->qqMinimum->setText("");
                ui->qqRange->setText("");
            }
            else
            {
                ui->qqMinimum->setText(utils::getAngleString(true, minP).c_str());
                ui->qqRange->setText(utils::getAngleString(false, maxP - minP).c_str());
            }
            ui->qqPosition->setText(utils::getAngleString(true, it->getPosition()).c_str());
        }
        else
            ui->qqLead->setText("");

        if (prismatic)
        {
            ui->qqMinimum->setText(utils::getPosString(true, minP).c_str());
            ui->qqRange->setText(utils::getPosString(false, maxP - minP).c_str());
            ui->qqPosition->setText(utils::getPosString(true, it->getPosition()).c_str());
        }

        if (spherical)
        {
            ui->qqMinimum->setText("");
            ui->qqRange->setText("");
            ui->qqPosition->setText("");
            C3Vector euler(it->getSphericalTransformation().getEulerAngles());
            ui->qqLength->setText("");
        }
        else
        {
            ui->qqLength->setText(utils::getSizeString(false, it->getLength()).c_str());
        }

        ui->qqDiameter->setText(utils::getSizeString(false, it->getDiameter()).c_str());

        ui->qqJointModeCombo->addItem(IDSN_JOINT_IS_IN_KINEMATIC_MODE, QVariant(sim_jointmode_kinematic));
        if (((it->getJointMode() == sim_jointmode_kinematic) && (it->getHybridFunctionality_old())) ||
            App::userSettings->showOldDlgs)
            ui->qqJointModeCombo->addItem(IDSN_JOINT_IS_IN_HYBRID_PASSIVE_MODE,
                                          QVariant(sim_jointmode_kinematic | sim_jointmode_hybrid_deprecated));

        if (!spherical)
        {
            ui->qqJointModeCombo->addItem(IDSN_JOINT_IS_IN_DEPENDENT_MODE, QVariant(sim_jointmode_dependent));
            if (((it->getJointMode() == sim_jointmode_dependent) && (it->getHybridFunctionality_old())) ||
                App::userSettings->showOldDlgs)
                ui->qqJointModeCombo->addItem(IDSN_JOINT_IS_IN_HYBRID_DEPENDENT_MODE,
                                              QVariant(sim_jointmode_dependent | sim_jointmode_hybrid_deprecated));
        }

        if ((it->getJointMode() == sim_jointmode_ik_deprecated) || App::userSettings->showOldDlgs)
        {
            if (App::userSettings->showOldDlgs)
            {
                ui->qqJointModeCombo->addItem(IDSN_JOINT_IS_IN_IK_MODE, QVariant(sim_jointmode_ik_deprecated));
                ui->qqJointModeCombo->addItem(IDSN_JOINT_IS_IN_HYBRID_IK_MODE,
                                              QVariant(sim_jointmode_ik_deprecated | sim_jointmode_hybrid_deprecated));
            }
            else
            {
                if (it->getHybridFunctionality_old())
                    ui->qqJointModeCombo->addItem(
                        IDSN_JOINT_IS_IN_HYBRID_IK_MODE,
                        QVariant(sim_jointmode_ik_deprecated | sim_jointmode_hybrid_deprecated));
                else
                    ui->qqJointModeCombo->addItem(IDSN_JOINT_IS_IN_IK_MODE, QVariant(sim_jointmode_ik_deprecated));
            }
        }

        ui->qqJointModeCombo->addItem(IDSN_JOINT_IS_IN_DYNAMIC_MODE, QVariant(sim_jointmode_dynamic));

        if ((!spherical) && (it->getJointMode() == sim_jointmode_motion_deprecated))
        {
            if (it->getHybridFunctionality_old())
                ui->qqJointModeCombo->addItem(
                    IDSN_JOINT_IS_IN_HYBRID_MOTION_MODE,
                    QVariant(sim_jointmode_motion_deprecated | sim_jointmode_hybrid_deprecated));
            else
                ui->qqJointModeCombo->addItem(IDSN_JOINT_IS_IN_MOTION_MODE, QVariant(sim_jointmode_motion_deprecated));
        }
        int val = 0;
        if (it->getHybridFunctionality_old())
            val = sim_jointmode_hybrid_deprecated;
        for (int i = 0; i < ui->qqJointModeCombo->count(); i++)
        {
            if (ui->qqJointModeCombo->itemData(i).toInt() == (it->getJointMode() | val))
            {
                ui->qqJointModeCombo->setCurrentIndex(i);
                break;
            }
        }
    }
    else
    {
        ui->qqLead->setText("");
        ui->qqMinimum->setText("");
        ui->qqRange->setText("");
        ui->qqPosition->setText("");
        ui->qqLength->setText("");
        ui->qqDiameter->setText("");
    }

    VDialog* dlg = GuiApp::mainWindow->dlgCont->getDialog(JOINT_DYN_DLG);
    if (dlg != nullptr)
        ((CQDlgJointDyn*)dlg)->refresh();

    selectLineEdit(lineEditToSelect);
    inMainRefreshRoutine = false;
}

void CQDlgJoints::on_qqCyclic_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_CYCLIC_JOINTGUITRIGGEREDCMD,
                                           App::currentWorld->sceneObjects->getLastSelectionHandle());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgJoints::on_qqLead_editingFinished()
{
    if (!ui->qqLead->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        double newVal = GuiApp::getEvalDouble(ui->qqLead->text().toStdString().c_str(), &ok);
        if (ok)
        {
            App::appendSimulationThreadCommand(SET_LEAD_JOINTGUITRIGGEREDCMD,
                                               App::currentWorld->sceneObjects->getLastSelectionHandle(), -1, newVal);
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
        double newVal = GuiApp::getEvalDouble(ui->qqMinimum->text().toStdString().c_str(), &ok);
        CJoint* it = App::currentWorld->sceneObjects->getLastSelectionJoint();
        if (ok && (it != nullptr))
        {
            if (it->getJointType() != sim_joint_prismatic)
                newVal *= degToRad;
            App::appendSimulationThreadCommand(SET_MINPOS_JOINTGUITRIGGEREDCMD,
                                               App::currentWorld->sceneObjects->getLastSelectionHandle(), -1, newVal);
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
        double newVal = GuiApp::getEvalDouble(ui->qqRange->text().toStdString().c_str(), &ok);
        CJoint* it = App::currentWorld->sceneObjects->getLastSelectionJoint();
        if (ok && (it != nullptr))
        {
            if (it->getJointType() != sim_joint_prismatic)
                newVal *= degToRad;
            App::appendSimulationThreadCommand(SET_RANGE_JOINTGUITRIGGEREDCMD,
                                               App::currentWorld->sceneObjects->getLastSelectionHandle(), -1, newVal);
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
        double newVal = GuiApp::getEvalDouble(ui->qqPosition->text().toStdString().c_str(), &ok);
        CJoint* it = App::currentWorld->sceneObjects->getLastSelectionJoint();
        if (ok && (it != nullptr))
        {
            if (it->getJointType() != sim_joint_prismatic)
                newVal *= degToRad;
            App::appendSimulationThreadCommand(SET_POS_JOINTGUITRIGGEREDCMD,
                                               App::currentWorld->sceneObjects->getLastSelectionHandle(), -1, newVal);
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
        cmd.cmdId = APPLY_CONFIGPARAMS_JOINTGUITRIGGEREDCMD;
        cmd.intParams.push_back(App::currentWorld->sceneObjects->getLastSelectionHandle());
        for (size_t i = 0; i < App::currentWorld->sceneObjects->getSelectionCount() - 1; i++)
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
            int mode = ui->qqJointModeCombo->itemData(ui->qqJointModeCombo->currentIndex()).toInt();
            App::appendSimulationThreadCommand(SET_MODE_JOINTGUITRIGGEREDCMD,
                                               App::currentWorld->sceneObjects->getLastSelectionHandle(), mode);
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
        cmd.cmdId = APPLY_MODEPARAMS_JOINTGUITRIGGEREDCMD;
        cmd.intParams.push_back(App::currentWorld->sceneObjects->getLastSelectionHandle());
        for (size_t i = 0; i < App::currentWorld->sceneObjects->getSelectionCount() - 1; i++)
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
        if (App::currentWorld->sceneObjects->isLastSelectionOfType(sim_sceneobject_joint))
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
        showDynamicWindow = !showDynamicWindow;
        if (GuiApp::mainWindow->dlgCont->isVisible(JOINT_DYN_DLG) != showDynamicWindow)
            GuiApp::mainWindow->dlgCont->toggle(JOINT_DYN_DLG);
    }
}

void CQDlgJoints::on_qqLength_editingFinished()
{
    if (!ui->qqLength->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        double newVal = GuiApp::getEvalDouble(ui->qqLength->text().toStdString().c_str(), &ok);
        if (ok)
        {
            App::appendSimulationThreadCommand(SET_LENGTH_JOINTGUITRIGGEREDCMD,
                                               App::currentWorld->sceneObjects->getLastSelectionHandle(), -1, newVal);
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
        double newVal = GuiApp::getEvalDouble(ui->qqDiameter->text().toStdString().c_str(), &ok);
        if (ok)
        {
            App::appendSimulationThreadCommand(SET_DIAMETER_JOINTGUITRIGGEREDCMD,
                                               App::currentWorld->sceneObjects->getLastSelectionHandle(), -1, newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgJoints::on_qqAdjustColorA_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CQDlgMaterial::displayMaterialDlg(COLOR_ID_JOINT_A, App::currentWorld->sceneObjects->getLastSelectionHandle(),
                                          -1, GuiApp::mainWindow);
    }
}

void CQDlgJoints::on_qqApplyAppearance_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        SSimulationThreadCommand cmd;
        cmd.cmdId = APPLY_VISUALPARAMS_JOINTGUITRIGGEREDCMD;
        cmd.intParams.push_back(App::currentWorld->sceneObjects->getLastSelectionHandle());
        for (size_t i = 0; i < App::currentWorld->sceneObjects->getSelectionCount() - 1; i++)
            cmd.intParams.push_back(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
        App::appendSimulationThreadCommand(cmd);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}
