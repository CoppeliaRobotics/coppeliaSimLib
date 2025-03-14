#include <qdlgdummies.h>
#include <ui_qdlgdummies.h>
#include <tt.h>
#include <utils.h>
#include <qdlgmaterial.h>
#include <app.h>
#include <simStrings.h>
#include <guiApp.h>

CQDlgDummies::CQDlgDummies(QWidget* parent)
    : CDlgEx(parent), ui(new Ui::CQDlgDummies)
{
    _dlgType = DUMMY_DLG;
    ui->setupUi(this);
    inMainRefreshRoutine = false;
}

CQDlgDummies::~CQDlgDummies()
{
    delete ui;
}

void CQDlgDummies::cancelEvent()
{
    // we override this cancel event. The container window should close, not this one!!
    GuiApp::mainWindow->dlgCont->close(OBJECT_DLG);
}

void CQDlgDummies::refresh()
{
    inMainRefreshRoutine = true;
    QLineEdit* lineEditToSelect = getSelectedLineEdit();
    bool noEditModeNoSim =
        (GuiApp::getEditModeType() == NO_EDIT_MODE) && App::currentWorld->simulation->isSimulationStopped();

    bool sel = App::currentWorld->sceneObjects->isLastSelectionOfType(sim_sceneobject_dummy);
    bool bigSel = (App::currentWorld->sceneObjects->getObjectCountInSelection(sim_sceneobject_dummy) > 1);
    CDummy* it = App::currentWorld->sceneObjects->getLastSelectionDummy();

    ui->qqSize->setEnabled(sel && noEditModeNoSim);
    ui->qqColor->setEnabled(sel && noEditModeNoSim);
    ui->qqApplyMainProperties->setEnabled(bigSel && noEditModeNoSim);

    ui->qqLinkedDummyCombo->setEnabled(sel && noEditModeNoSim);
    ui->qqLinkedDummyCombo->clear();
    ui->qqLinkTypeCombo->setEnabled(sel && noEditModeNoSim);
    ui->qqLinkTypeCombo->clear();
    ui->qqEditEngine->setEnabled(sel && noEditModeNoSim);

    ui->qqfollowParentOrientation->setEnabled(sel && noEditModeNoSim);
    ui->qqFollow->setEnabled(sel && noEditModeNoSim);

    ui->qqTrajectoryGroup->setVisible(App::userSettings->showOldDlgs);

    if (sel)
    {
        ui->qqSize->setText(utils::getSizeString(false, it->getDummySize()).c_str());

        int lt = it->getDummyType();
        int lts = lt;
        ui->qqLinkTypeCombo->addItem("Default", QVariant(sim_dummytype_default));
        ui->qqLinkTypeCombo->addItem("Assembly", QVariant(sim_dummytype_assembly));
        ui->qqLinkTypeCombo->addItem("Dynamic, overlap constraint", QVariant(sim_dummytype_dynloopclosure));
        ui->qqLinkTypeCombo->addItem("Dynamic, tendon constraint", QVariant(sim_dummytype_dyntendon));
        // Following for backward compatibility:
        if (it->getLinkedDummyHandle() != -1)
        {
            if ((it->getDummyType() == sim_dummy_linktype_ik_tip_target) || App::userSettings->showOldDlgs)
                ui->qqLinkTypeCombo->addItem("IK, tip-target (deprecated)", QVariant(sim_dummy_linktype_ik_tip_target));
            if (it->getDummyType() == sim_dummy_linktype_gcs_loop_closure)
                ui->qqLinkTypeCombo->addItem("GCS, overlap constraint (deprecated)",
                                             QVariant(sim_dummy_linktype_gcs_loop_closure));
            if (it->getDummyType() == sim_dummy_linktype_gcs_tip)
                ui->qqLinkTypeCombo->addItem("GCS, tip (deprecated)", QVariant(sim_dummy_linktype_gcs_tip));
            if (it->getDummyType() == sim_dummy_linktype_gcs_target)
                ui->qqLinkTypeCombo->addItem("GCS, target (deprecated)", QVariant(sim_dummy_linktype_gcs_target));
        }
        else
        { // if we have a deprecated dummy type, but no linked dummy, we fake a 'default' link type:
            if ((it->getDummyType() == sim_dummy_linktype_ik_tip_target) ||
                (it->getDummyType() == sim_dummy_linktype_gcs_loop_closure) ||
                (it->getDummyType() == sim_dummy_linktype_gcs_tip) ||
                (it->getDummyType() == sim_dummy_linktype_gcs_target))
                lts = sim_dummytype_default;
        }

        // Here we select the appropriate item:
        for (int i = 0; i < ui->qqLinkTypeCombo->count(); i++)
        {
            if (ui->qqLinkTypeCombo->itemData(i).toInt() == lts)
            {
                ui->qqLinkTypeCombo->setCurrentIndex(i);
                break;
            }
        }

        ui->qqLinkedDummyCombo->addItem(IDSN_NONE, QVariant(-1));
        if ((lt != sim_dummytype_default) && (lt != sim_dummytype_assembly))
        {
            std::vector<std::string> names;
            std::vector<int> ids;
            for (size_t i = 0; i < App::currentWorld->sceneObjects->getObjectCount(sim_sceneobject_dummy); i++)
            {
                CDummy* it2 = App::currentWorld->sceneObjects->getDummyFromIndex(i);
                if (it2 != it)
                {
                    names.push_back(it2->getObjectAlias_printPath());
                    ids.push_back(it2->getObjectHandle());
                }
            }
            tt::orderStrings(names, ids);
            for (int i = 0; i < int(names.size()); i++)
                ui->qqLinkedDummyCombo->addItem(names[i].c_str(), QVariant(ids[i]));
        }
        for (int i = 0; i < ui->qqLinkedDummyCombo->count(); i++)
        {
            if (ui->qqLinkedDummyCombo->itemData(i).toInt() == it->getLinkedDummyHandle())
            {
                ui->qqLinkedDummyCombo->setCurrentIndex(i);
                break;
            }
        }

        ui->qqfollowParentOrientation->setChecked(it->getAssignedToParentPathOrientation());
        ui->qqFollow->setChecked(it->getAssignedToParentPath());

        ui->qqFree->setEnabled(it->getAssignedToParentPath() && noEditModeNoSim);
        ui->qqFixed->setEnabled(it->getAssignedToParentPath() && noEditModeNoSim);
        ui->qqFree->setChecked(it->getAssignedToParentPath() && it->getFreeOnPathTrajectory());
        ui->qqFixed->setChecked(it->getAssignedToParentPath() && (!it->getFreeOnPathTrajectory()));

        ui->qqOffset->setEnabled(!it->getFreeOnPathTrajectory() && noEditModeNoSim);
        ui->qqIncrement->setEnabled(!it->getFreeOnPathTrajectory() && noEditModeNoSim);
        if (it->getAssignedToParentPath() && (!it->getFreeOnPathTrajectory()))
        {
            ui->qqOffset->setText(utils::getSizeString(true, it->getVirtualDistanceOffsetOnPath()).c_str());
            ui->qqIncrement->setText(
                utils::getSizeString(true, it->getVirtualDistanceOffsetOnPath_variationWhenCopy()).c_str());
        }
        else
        {
            ui->qqOffset->setText("");
            ui->qqIncrement->setText("");
        }
    }
    else
    {
        ui->qqSize->setText("");
        ui->qqfollowParentOrientation->setChecked(false);
        ui->qqFollow->setChecked(false);
        ui->qqFree->setEnabled(false);
        ui->qqFree->setChecked(false);
        ui->qqFixed->setEnabled(false);
        ui->qqFixed->setChecked(false);
        ui->qqOffset->setEnabled(false);
        ui->qqOffset->setText("");
        ui->qqIncrement->setEnabled(false);
        ui->qqIncrement->setText("");
    }

    selectLineEdit(lineEditToSelect);
    inMainRefreshRoutine = false;
}

void CQDlgDummies::on_qqSize_editingFinished()
{
    if (!ui->qqSize->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        double newVal = GuiApp::getEvalDouble(ui->qqSize->text().toStdString().c_str(), &ok);
        App::appendSimulationThreadCommand(SET_SIZE_DUMMYGUITRIGGEREDCMD,
                                           App::currentWorld->sceneObjects->getLastSelectionHandle(), -1, newVal);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgDummies::on_qqColor_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CQDlgMaterial::displayMaterialDlg(COLOR_ID_DUMMY, App::currentWorld->sceneObjects->getLastSelectionHandle(), -1,
                                          GuiApp::mainWindow);
    }
}

void CQDlgDummies::on_qqApplyMainProperties_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CDummy* it = App::currentWorld->sceneObjects->getLastSelectionDummy();
        if (it != nullptr)
        {
            SSimulationThreadCommand cmd;
            cmd.cmdId = APPLY_VISUALPROP_DUMMYGUITRIGGEREDCMD;
            cmd.intParams.push_back(App::currentWorld->sceneObjects->getLastSelectionHandle());
            for (size_t i = 0; i < App::currentWorld->sceneObjects->getSelectionCount() - 1; i++)
                cmd.intParams.push_back(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
            App::appendSimulationThreadCommand(cmd);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgDummies::on_qqLinkedDummyCombo_currentIndexChanged(int index)
{
    if (!inMainRefreshRoutine)
    {
        IF_UI_EVENT_CAN_WRITE_DATA
        {
            int objID = ui->qqLinkedDummyCombo->itemData(ui->qqLinkedDummyCombo->currentIndex()).toInt();
            App::appendSimulationThreadCommand(SET_LINKEDDUMMY_DUMMYGUITRIGGEREDCMD,
                                               App::currentWorld->sceneObjects->getLastSelectionHandle(), objID);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgDummies::on_qqLinkTypeCombo_currentIndexChanged(int index)
{
    if (!inMainRefreshRoutine)
    {
        IF_UI_EVENT_CAN_WRITE_DATA
        {
            int dataID = ui->qqLinkTypeCombo->itemData(ui->qqLinkTypeCombo->currentIndex()).toInt();
            App::appendSimulationThreadCommand(SET_LINKTYPE_DUMMYGUITRIGGEREDCMD,
                                               App::currentWorld->sceneObjects->getLastSelectionHandle(), dataID);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgDummies::on_qqfollowParentOrientation_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_FOLLOWORIENTATION_DUMMYGUITRIGGEREDCMD,
                                           App::currentWorld->sceneObjects->getLastSelectionHandle());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgDummies::on_qqFollow_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_FOLLOWPOSITION_DUMMYGUITRIGGEREDCMD,
                                           App::currentWorld->sceneObjects->getLastSelectionHandle());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgDummies::on_qqFree_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(SET_FREEORFIXEDONPATH_DUMMYGUITRIGGEREDCMD,
                                           App::currentWorld->sceneObjects->getLastSelectionHandle(), 1);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgDummies::on_qqFixed_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(SET_FREEORFIXEDONPATH_DUMMYGUITRIGGEREDCMD,
                                           App::currentWorld->sceneObjects->getLastSelectionHandle(), 0);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgDummies::on_qqOffset_editingFinished()
{
    if (!ui->qqOffset->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        double newVal = GuiApp::getEvalDouble(ui->qqOffset->text().toStdString().c_str(), &ok);
        App::appendSimulationThreadCommand(SET_OFFSET_DUMMYGUITRIGGEREDCMD,
                                           App::currentWorld->sceneObjects->getLastSelectionHandle(), -1, newVal);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgDummies::on_qqIncrement_editingFinished()
{
    if (!ui->qqIncrement->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        double newVal = GuiApp::getEvalDouble(ui->qqIncrement->text().toStdString().c_str(), &ok);
        App::appendSimulationThreadCommand(SET_COPYINCREMENT_DUMMYGUITRIGGEREDCMD,
                                           App::currentWorld->sceneObjects->getLastSelectionHandle(), -1, newVal);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgDummies::on_qqEditEngine_clicked()
{
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        CDummy* it = App::currentWorld->sceneObjects->getLastSelectionDummy();
        if (it != nullptr)
        {
            SSimulationThreadCommand cmd;
            cmd.cmdId = SET_ENGINEPARAMS_DUMMYGUITRIGGEREDCMD;
            cmd.intParams.push_back(App::currentWorld->sceneObjects->getLastSelectionHandle());
            App::appendSimulationThreadCommand(cmd);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}
