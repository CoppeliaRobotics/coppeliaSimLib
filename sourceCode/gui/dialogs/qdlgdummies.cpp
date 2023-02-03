#include <qdlgdummies.h>
#include <ui_qdlgdummies.h>
#include <tt.h>
#include <gV.h>
#include <qdlgmaterial.h>
#include <app.h>
#include <simStrings.h>

CQDlgDummies::CQDlgDummies(QWidget *parent) :
    CDlgEx(parent),
    ui(new Ui::CQDlgDummies)
{
    _dlgType=DUMMY_DLG;
    ui->setupUi(this);
    inMainRefreshRoutine=false;
}

CQDlgDummies::~CQDlgDummies()
{
    delete ui;
}

void CQDlgDummies::cancelEvent()
{
    // we override this cancel event. The container window should close, not this one!!
    App::mainWindow->dlgCont->close(OBJECT_DLG);
}

void CQDlgDummies::refresh()
{
    inMainRefreshRoutine=true;
    QLineEdit* lineEditToSelect=getSelectedLineEdit();
    bool noEditModeNoSim=(App::getEditModeType()==NO_EDIT_MODE)&&App::currentWorld->simulation->isSimulationStopped();

    bool sel=App::currentWorld->sceneObjects->isLastSelectionADummy();
    bool bigSel=(App::currentWorld->sceneObjects->getDummyCountInSelection()>1);
    CDummy* it=App::currentWorld->sceneObjects->getLastSelectionDummy();

    ui->qqSize->setEnabled(sel&&noEditModeNoSim);
    ui->qqColor->setEnabled(sel&&noEditModeNoSim);
    ui->qqApplyMainProperties->setEnabled(bigSel&&noEditModeNoSim);

    ui->qqLinkedDummyCombo->setEnabled(sel&&noEditModeNoSim);
    ui->qqLinkedDummyCombo->clear();
    ui->qqLinkTypeCombo->setEnabled(sel&&(it->getLinkedDummyHandle()!=-1)&&noEditModeNoSim);
    ui->qqLinkTypeCombo->clear();
    ui->qqEditEngine->setEnabled(sel&&noEditModeNoSim);

    ui->qqfollowParentOrientation->setEnabled(sel&&noEditModeNoSim);
    ui->qqFollow->setEnabled(sel&&noEditModeNoSim);

    ui->qqTrajectoryGroup->setVisible(App::userSettings->showOldDlgs);


    if (sel)
    {
        ui->qqSize->setText(tt::getFString(false,it->getDummySize(),4).c_str());

        ui->qqLinkedDummyCombo->addItem(IDSN_NONE,QVariant(-1));
        std::vector<std::string> names;
        std::vector<int> ids;
        for (size_t i=0;i<App::currentWorld->sceneObjects->getDummyCount();i++)
        {
            CDummy* it2=App::currentWorld->sceneObjects->getDummyFromIndex(i);
            if (it2!=it)
            {
                names.push_back(it2->getObjectAlias_printPath());
                ids.push_back(it2->getObjectHandle());
            }
        }
        tt::orderStrings(names,ids);
        for (int i=0;i<int(names.size());i++)
            ui->qqLinkedDummyCombo->addItem(names[i].c_str(),QVariant(ids[i]));
        for (int i=0;i<ui->qqLinkedDummyCombo->count();i++)
        {
            if (ui->qqLinkedDummyCombo->itemData(i).toInt()==it->getLinkedDummyHandle())
            {
                ui->qqLinkedDummyCombo->setCurrentIndex(i);
                break;
            }
        }

        if (it->getLinkedDummyHandle()!=-1)
        {
            ui->qqLinkTypeCombo->addItem("Dynamics, overlap constraint",QVariant(sim_dummylink_dynloopclosure));
            ui->qqLinkTypeCombo->addItem("Dynamics, tendon constraint",QVariant(sim_dummylink_dyntendon));
            // Following for backward compatibility:
            if ( (it->getLinkType()==sim_dummy_linktype_ik_tip_target)||App::userSettings->showOldDlgs )
                ui->qqLinkTypeCombo->addItem("IK, tip-target (deprecated)",QVariant(sim_dummy_linktype_ik_tip_target));
            if (it->getLinkType()==sim_dummy_linktype_gcs_loop_closure)
                ui->qqLinkTypeCombo->addItem("GCS, overlap constraint (deprecated)",QVariant(sim_dummy_linktype_gcs_loop_closure));
            if (it->getLinkType()==sim_dummy_linktype_gcs_tip)
                ui->qqLinkTypeCombo->addItem("GCS, tip (deprecated)",QVariant(sim_dummy_linktype_gcs_tip));
            if (it->getLinkType()==sim_dummy_linktype_gcs_target)
                ui->qqLinkTypeCombo->addItem("GCS, target (deprecated)",QVariant(sim_dummy_linktype_gcs_target));

            // Here we select the appropriate item:
            for (int i=0;i<ui->qqLinkTypeCombo->count();i++)
            {
                if (ui->qqLinkTypeCombo->itemData(i).toInt()==it->getLinkType())
                {
                    ui->qqLinkTypeCombo->setCurrentIndex(i);
                    break;
                }
            }
        }

        ui->qqfollowParentOrientation->setChecked(it->getAssignedToParentPathOrientation());
        ui->qqFollow->setChecked(it->getAssignedToParentPath());

        ui->qqFree->setEnabled(it->getAssignedToParentPath()&&noEditModeNoSim);
        ui->qqFixed->setEnabled(it->getAssignedToParentPath()&&noEditModeNoSim);
        ui->qqFree->setChecked(it->getAssignedToParentPath()&&it->getFreeOnPathTrajectory());
        ui->qqFixed->setChecked(it->getAssignedToParentPath()&&(!it->getFreeOnPathTrajectory()));


        ui->qqOffset->setEnabled(!it->getFreeOnPathTrajectory()&&noEditModeNoSim);
        ui->qqIncrement->setEnabled(!it->getFreeOnPathTrajectory()&&noEditModeNoSim);
        if (it->getAssignedToParentPath()&&(!it->getFreeOnPathTrajectory()))
        {
            ui->qqOffset->setText(tt::getEString(true,it->getVirtualDistanceOffsetOnPath(),3).c_str());
            ui->qqIncrement->setText(tt::getEString(true,it->getVirtualDistanceOffsetOnPath_variationWhenCopy(),3).c_str());
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
    inMainRefreshRoutine=false;
}

void CQDlgDummies::on_qqSize_editingFinished()
{
    if (!ui->qqSize->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        double newVal=ui->qqSize->text().toFloat(&ok);
        App::appendSimulationThreadCommand(SET_SIZE_DUMMYGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle(),-1,newVal);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgDummies::on_qqColor_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CQDlgMaterial::displayMaterialDlg(COLOR_ID_DUMMY,App::currentWorld->sceneObjects->getLastSelectionHandle(),-1,App::mainWindow);
    }
}

void CQDlgDummies::on_qqApplyMainProperties_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CDummy* it=App::currentWorld->sceneObjects->getLastSelectionDummy();
        if (it!=nullptr)
        {
            SSimulationThreadCommand cmd;
            cmd.cmdId=APPLY_VISUALPROP_DUMMYGUITRIGGEREDCMD;
            cmd.intParams.push_back(App::currentWorld->sceneObjects->getLastSelectionHandle());
            for (size_t i=0;i<App::currentWorld->sceneObjects->getSelectionCount()-1;i++)
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
            int objID=ui->qqLinkedDummyCombo->itemData(ui->qqLinkedDummyCombo->currentIndex()).toInt();
            App::appendSimulationThreadCommand(SET_LINKEDDUMMY_DUMMYGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle(),objID);
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
            int dataID=ui->qqLinkTypeCombo->itemData(ui->qqLinkTypeCombo->currentIndex()).toInt();
            App::appendSimulationThreadCommand(SET_LINKTYPE_DUMMYGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle(),dataID);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgDummies::on_qqfollowParentOrientation_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_FOLLOWORIENTATION_DUMMYGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgDummies::on_qqFollow_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_FOLLOWPOSITION_DUMMYGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgDummies::on_qqFree_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(SET_FREEORFIXEDONPATH_DUMMYGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle(),1);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgDummies::on_qqFixed_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(SET_FREEORFIXEDONPATH_DUMMYGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle(),0);
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
        double newVal=ui->qqOffset->text().toFloat(&ok);
        App::appendSimulationThreadCommand(SET_OFFSET_DUMMYGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle(),-1,newVal);
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
        double newVal=ui->qqIncrement->text().toFloat(&ok);
        App::appendSimulationThreadCommand(SET_COPYINCREMENT_DUMMYGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle(),-1,newVal);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgDummies::on_qqEditEngine_clicked()
{
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        CDummy* it=App::currentWorld->sceneObjects->getLastSelectionDummy();
        if (it!=nullptr)
        {
            SSimulationThreadCommand cmd;
            cmd.cmdId=SET_ENGINEPARAMS_DUMMYGUITRIGGEREDCMD;
            cmd.intParams.push_back(App::currentWorld->sceneObjects->getLastSelectionHandle());
            App::appendSimulationThreadCommand(cmd);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}
