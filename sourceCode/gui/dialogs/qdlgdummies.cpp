#include "vrepMainHeader.h"
#include "qdlgdummies.h"
#include "ui_qdlgdummies.h"
#include "tt.h"
#include "gV.h"
#include "qdlgmaterial.h"
#include "app.h"
#include "v_repStrings.h"

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
    bool noEditModeNoSim=(App::getEditModeType()==NO_EDIT_MODE)&&App::ct->simulation->isSimulationStopped();

    bool sel=App::ct->objCont->isLastSelectionADummy();
    bool bigSel=(App::ct->objCont->getDummyNumberInSelection()>1);
    CDummy* it=App::ct->objCont->getLastSelection_dummy();

    ui->qqSize->setEnabled(sel&&noEditModeNoSim);
    ui->qqColor->setEnabled(sel&&noEditModeNoSim);
    ui->qqApplyMainProperties->setEnabled(bigSel&&noEditModeNoSim);

    ui->qqLinkedDummyCombo->setEnabled(sel&&noEditModeNoSim);
    ui->qqLinkedDummyCombo->clear();
    ui->qqLinkTypeCombo->setEnabled(sel&&(it->getLinkedDummyID()!=-1)&&noEditModeNoSim);
    ui->qqLinkTypeCombo->clear();

    ui->qqfollowParentOrientation->setEnabled(sel&&noEditModeNoSim);
    ui->qqFollow->setEnabled(sel&&noEditModeNoSim);

    if (sel)
    {
        ui->qqSize->setText(tt::getFString(false,it->getSize(),4).c_str());

        ui->qqLinkedDummyCombo->addItem(strTranslate(IDSN_NONE),QVariant(-1));
        std::vector<std::string> names;
        std::vector<int> ids;
        for (int i=0;i<int(App::ct->objCont->dummyList.size());i++)
        {
            CDummy* it2=App::ct->objCont->getDummy(App::ct->objCont->dummyList[i]);
            if (it2!=it)
            {
                names.push_back(it2->getObjectName());
                ids.push_back(it2->getObjectHandle());
            }
        }
        tt::orderStrings(names,ids);
        for (int i=0;i<int(names.size());i++)
            ui->qqLinkedDummyCombo->addItem(names[i].c_str(),QVariant(ids[i]));
        for (int i=0;i<ui->qqLinkedDummyCombo->count();i++)
        {
            if (ui->qqLinkedDummyCombo->itemData(i).toInt()==it->getLinkedDummyID())
            {
                ui->qqLinkedDummyCombo->setCurrentIndex(i);
                break;
            }
        }

        if (it->getLinkedDummyID()!=-1)
        {
            ui->qqLinkTypeCombo->addItem(strTranslate(IDS_DUMMY_LINK_TYPE_IK_TIP_TARGET),QVariant(sim_dummy_linktype_ik_tip_target));
            ui->qqLinkTypeCombo->addItem(strTranslate(IDS_DUMMY_LINK_TYPE_GCS_LOOP_CLOSURE),QVariant(sim_dummy_linktype_gcs_loop_closure));
            ui->qqLinkTypeCombo->addItem(strTranslate(IDS_DUMMY_LINK_TYPE_GCS_TIP),QVariant(sim_dummy_linktype_gcs_tip));
            ui->qqLinkTypeCombo->addItem(strTranslate(IDS_DUMMY_LINK_TYPE_GCS_TARGET),QVariant(sim_dummy_linktype_gcs_target));
            ui->qqLinkTypeCombo->addItem(strTranslate(IDS_DUMMY_LINK_TYPE_DYNAMICS_LOOP_CLOSURE),QVariant(sim_dummy_linktype_dynamics_loop_closure));
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
        float newVal=ui->qqSize->text().toFloat(&ok);
        App::appendSimulationThreadCommand(SET_SIZE_DUMMYGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),-1,newVal);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgDummies::on_qqColor_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CQDlgMaterial::displayMaterialDlg(COLOR_ID_DUMMY,App::ct->objCont->getLastSelectionID(),-1,App::mainWindow);
    }
}

void CQDlgDummies::on_qqApplyMainProperties_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CDummy* it=App::ct->objCont->getLastSelection_dummy();
        if (it!=nullptr)
        {
            SSimulationThreadCommand cmd;
            cmd.cmdId=APPLY_VISUALPROP_DUMMYGUITRIGGEREDCMD;
            cmd.intParams.push_back(App::ct->objCont->getLastSelectionID());
            for (int i=0;i<App::ct->objCont->getSelSize()-1;i++)
                cmd.intParams.push_back(App::ct->objCont->getSelID(i));
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
            App::appendSimulationThreadCommand(SET_LINKEDDUMMY_DUMMYGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),objID);
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
            App::appendSimulationThreadCommand(SET_LINKTYPE_DUMMYGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),dataID);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgDummies::on_qqfollowParentOrientation_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_FOLLOWORIENTATION_DUMMYGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgDummies::on_qqFollow_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_FOLLOWPOSITION_DUMMYGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgDummies::on_qqFree_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(SET_FREEORFIXEDONPATH_DUMMYGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),1);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgDummies::on_qqFixed_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(SET_FREEORFIXEDONPATH_DUMMYGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),0);
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
        float newVal=ui->qqOffset->text().toFloat(&ok);
        App::appendSimulationThreadCommand(SET_OFFSET_DUMMYGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),-1,newVal);
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
        float newVal=ui->qqIncrement->text().toFloat(&ok);
        App::appendSimulationThreadCommand(SET_COPYINCREMENT_DUMMYGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),-1,newVal);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}
