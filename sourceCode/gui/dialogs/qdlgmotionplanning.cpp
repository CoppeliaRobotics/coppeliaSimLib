
#include "vrepMainHeader.h"
#include "qdlgmotionplanning.h"
#include "ui_qdlgmotionplanning.h"
#include "tt.h"
#include "gV.h"
#include "editboxdelegate.h"
#include "v_repStrings.h"
#include "app.h"
#include <boost/lexical_cast.hpp>
#include "qdlgmotionplanningjoints.h"
#include "qdlgworkspacemetric.h"
#include "vMessageBox.h"

CQDlgMotionPlanning::CQDlgMotionPlanning(QWidget *parent) :
    CDlgEx(parent),
    ui(new Ui::CQDlgMotionPlanning)
{
    _dlgType=MOTION_PLANNING_DLG;
    ui->setupUi(this);
    inMainRefreshRoutine=false;
    inListSelectionRoutine=false;
    noListSelectionAllowed=false;
    delKeyShortcut = new QShortcut(QKeySequence(Qt::Key_Delete), this);
    connect(delKeyShortcut,SIGNAL(activated()), this, SLOT(onDeletePressed()));
    backspaceKeyShortcut = new QShortcut(QKeySequence(Qt::Key_Backspace), this);
    connect(backspaceKeyShortcut,SIGNAL(activated()), this, SLOT(onDeletePressed()));
    CEditBoxDelegate* delegate=new CEditBoxDelegate();
    ui->qqList->setItemDelegate(delegate);
}

CQDlgMotionPlanning::~CQDlgMotionPlanning()
{
    delete ui;
}

void CQDlgMotionPlanning::cancelEvent()
{
    // we override this cancel event. The container window should close, not this one!!
    App::mainWindow->dlgCont->close(CALCULATION_DLG);
}

void CQDlgMotionPlanning::dialogCallbackFunc(const SUIThreadCommand* cmdIn,SUIThreadCommand* cmdOut)
{
    if ( (cmdIn!=nullptr)&&(cmdIn->intParams[0]==_dlgType) )
    {
        if (cmdIn->intParams[1]==0)
            selectObjectInList(cmdIn->intParams[2]);
    }
}

void CQDlgMotionPlanning::refresh()
{
    inMainRefreshRoutine=true;
    QLineEdit* lineEditToSelect=getSelectedLineEdit();
    bool noEditModeNoSim=(App::getEditModeType()==NO_EDIT_MODE)&&App::ct->simulation->isSimulationStopped();

    int taskID=getSelectedObjectID();
    CMotionPlanningTask* task=App::ct->motionPlanning->getObject(taskID);

    ui->qqMotionPlanningGroup->setEnabled(noEditModeNoSim);

    if (!inListSelectionRoutine)
    {
        updateObjectsInList();
        selectObjectInList(taskID);
    }

    ui->qqAddNew->setEnabled(noEditModeNoSim);

    ui->qqIkGroupCombo->setEnabled(noEditModeNoSim&&(task!=nullptr));
    ui->qqEntity1Combo->setEnabled(noEditModeNoSim&&(task!=nullptr));
    ui->qqEntity2Combo->setEnabled(noEditModeNoSim&&(task!=nullptr)&&(task->getRobotSelfCollEntity1()!=-1));
    ui->qqSelfColl->setEnabled(noEditModeNoSim&&(task!=nullptr)&&(task->getRobotSelfCollEntity1()!=-1));
    ui->qqSelfDist->setEnabled(noEditModeNoSim&&(task!=nullptr)&&(task->getRobotSelfCollEntity1()!=-1));
    ui->qqSelfDistDist->setEnabled(noEditModeNoSim&&(task!=nullptr)&&(task->getSelfCollDistanceThreshold()!=0.0f)&&(task->getRobotSelfCollEntity1()!=-1));
    ui->qqRobotCombo->setEnabled(noEditModeNoSim&&(task!=nullptr));
    ui->qqObstacleCombo->setEnabled(noEditModeNoSim&&(task!=nullptr)&&(task->getRobotEntity()!=-1));
    ui->qqColl->setEnabled(noEditModeNoSim&&(task!=nullptr)&&(task->getRobotEntity()!=-1));
    ui->qqDist->setEnabled(noEditModeNoSim&&(task!=nullptr)&&(task->getRobotEntity()!=-1));
    ui->qqDistDist->setEnabled(noEditModeNoSim&&(task!=nullptr)&&(task->getDistanceThreshold()!=0.0f)&&(task->getRobotEntity()!=-1));

    ui->qqIkGroupCombo->clear();
    ui->qqEntity1Combo->clear();
    ui->qqEntity2Combo->clear();
    ui->qqRobotCombo->clear();
    ui->qqObstacleCombo->clear();

    ui->qqEditJointProp->setEnabled(noEditModeNoSim&&(task!=nullptr));
    ui->qqEditWorkspaceMetric->setEnabled(noEditModeNoSim&&(task!=nullptr));
    ui->qqComputeNodes->setEnabled(noEditModeNoSim&&(task!=nullptr));
    ui->showPhase1Nodes->setEnabled(noEditModeNoSim&&(task!=nullptr));

    if (task!=nullptr)
    {
        // IK group:
        fillComboWithIkGroups(ui->qqIkGroupCombo);
        if (!selectItemInCombo(ui->qqIkGroupCombo,task->getIkGroup()))
        { // should not happen
            task->setIkGroup(-1); // none
            ui->qqIkGroupCombo->setCurrentIndex(0);
        }

        if (task->getIkGroup()!=-1)
        {
            std::string warningText;

            CikGroup* ikGroup=App::ct->ikGroups->getIkGroup(task->getIkGroup());
            if (ikGroup->ikElements.size()!=1)
            {
                if (warningText.length()==0)
                    warningText="Warning: IK group should contain exactly 1 IK element";
            }

            C3DObject* obj=task->getBaseObject();
            if (obj==nullptr)
            {
                ui->qqBase->setText("world");
                if (warningText.length()==0)
                    warningText="Warning: base should not be 'world'";
            }
            else
                ui->qqBase->setText(obj->getObjectName().c_str());

            obj=task->getTipObject();
            if (obj==nullptr)
            {
                ui->qqTip->setText("-");
                if (warningText.length()==0)
                    warningText="Warning: tip is not defined";
            }
            else
                ui->qqTip->setText(obj->getObjectName().c_str());

            obj=task->getTargetObject();
            if (obj==nullptr)
            {
                ui->qqTarget->setText("-");
                if (warningText.length()==0)
                    warningText="Warning: target is not defined";
            }
            else
                ui->qqTarget->setText(obj->getObjectName().c_str());

            if (warningText.length()==0)
            {
                if (ikGroup->ikElements.size()==1)
                {
                    if (!ikGroup->ikElements[0]->getActive())
                    {
                        if (warningText.length()==0)
                            warningText="Warning: IK element is not active";
                    }
                    if (ikGroup->getCalculationMethod()!=sim_ik_pseudo_inverse_method)
                    {
                        if (warningText.length()==0)
                            warningText="Warning: IK resolution method is not the pseudo inverse";
                    }
                    if (!ikGroup->getExplicitHandling())
                    {
                        if (warningText.length()==0)
                            warningText="Warning: IK group should be flagged as 'explicit handling'";
                    }
                }
            }

            ui->qqWarning->setText(warningText.c_str());
        }
        else
        {
            ui->qqBase->setText("-");
            ui->qqTip->setText("-");
            ui->qqTarget->setText("-");
            ui->qqWarning->setText("Warning: no IK group selected");
        }

        // self-collision entity1:
        fillComboWithCollisionOrDistanceEntities(ui->qqEntity1Combo,task->getSelfCollDistanceThreshold()==0.0f,false,true,-1);
        if (!selectItemInCombo(ui->qqEntity1Combo,task->getRobotSelfCollEntity1()))
        { // should not happen
            task->setRobotSelfCollEntity1(-1); // none
            ui->qqEntity1Combo->setCurrentIndex(0);
        }

        // self-collentity2:
        fillComboWithCollisionOrDistanceEntities(ui->qqEntity2Combo,task->getSelfCollDistanceThreshold()==0.0f,true,false,task->getRobotSelfCollEntity1());
        if (!selectItemInCombo(ui->qqEntity2Combo,task->getRobotSelfCollEntity2()))
        { // should not happen
            task->setRobotSelfCollEntity2(-1); // all
            ui->qqEntity2Combo->setCurrentIndex(0);
        }

        ui->qqSelfColl->setChecked(task->getSelfCollDistanceThreshold()==0.0f);
        ui->qqSelfDist->setChecked(task->getSelfCollDistanceThreshold()!=0.0f);

        if (task->getSelfCollDistanceThreshold()!=0.0f)
            ui->qqSelfDistDist->setText(tt::getFString(false,task->getSelfCollDistanceThreshold(),3).c_str());
        else
            ui->qqSelfDistDist->setText("");


        // robot:
        fillComboWithCollisionOrDistanceEntities(ui->qqRobotCombo,task->getDistanceThreshold()==0.0f,false,true,-1);
        if (!selectItemInCombo(ui->qqRobotCombo,task->getRobotEntity()))
        { // should not happen
            task->setRobotEntity(-1); // none
            ui->qqRobotCombo->setCurrentIndex(0);
        }

        // obstacles:
        fillComboWithCollisionOrDistanceEntities(ui->qqObstacleCombo,task->getDistanceThreshold()==0.0f,true,false,task->getRobotEntity());
        if (!selectItemInCombo(ui->qqObstacleCombo,task->getObstacleEntity()))
        { // should not happen
            task->setObstacleEntity(-1); // all
            ui->qqObstacleCombo->setCurrentIndex(0);
        }

        ui->qqColl->setChecked(task->getDistanceThreshold()==0.0f);
        ui->qqDist->setChecked(task->getDistanceThreshold()!=0.0f);

        if (task->getDistanceThreshold()!=0.0f)
            ui->qqDistDist->setText(tt::getFString(false,task->getDistanceThreshold(),3).c_str());
        else
            ui->qqDistDist->setText("");

        ui->showPhase1Nodes->setChecked(task->getPhase1NodeVisualization());
    }
    else
    {
        ui->qqBase->setText("-");
        ui->qqTip->setText("-");
        ui->qqTarget->setText("-");
        ui->qqWarning->setText("");
        ui->qqSelfColl->setChecked(false);
        ui->qqSelfDist->setChecked(false);
        ui->qqSelfDistDist->setText("");
        ui->qqColl->setChecked(false);
        ui->qqDist->setChecked(false);
        ui->qqDistDist->setText("");
        ui->showPhase1Nodes->setChecked(false);
    }

    selectLineEdit(lineEditToSelect);
    inMainRefreshRoutine=false;
}

void CQDlgMotionPlanning::fillComboWithCollisionOrDistanceEntities(QComboBox* combo,bool collisionEntities,bool insertAllAtBeginning,bool insertNoneAtBeginning,int idToExclude)
{
    combo->clear();
    if (insertAllAtBeginning)
    {
        if (collisionEntities)
            combo->addItem(strTranslate(IDS_ALL_OTHER_COLLIDABLE_OBJECTS_IN_SCENE),QVariant(-1));
        else
            combo->addItem(strTranslate(IDS_ALL_OTHER_MEASURABLE_OBJECTS_IN_SCENE),QVariant(-1));
    }
    if (insertNoneAtBeginning)
        combo->addItem(strTranslate(IDSN_NONE),QVariant(-1));

    std::vector<std::string> names;
    std::vector<int> ids;

    // Now collections:
    for (int i=0;i<int(App::ct->collections->allCollections.size());i++)
    {
        CRegCollection* it=App::ct->collections->allCollections[i];
        std::string name(tt::decorateString("[",strTranslate(IDSN_COLLECTION),"] "));
        name+=it->getCollectionName();
        names.push_back(name);
        ids.push_back(it->getCollectionID());
    }
    tt::orderStrings(names,ids);
    for (int i=0;i<int(names.size());i++)
        combo->addItem(names[i].c_str(),QVariant(ids[i]));

    names.clear();
    ids.clear();

    // Now shapes:
    for (int i=0;i<int(App::ct->objCont->shapeList.size());i++)
    {
        CShape* it=App::ct->objCont->getShape(App::ct->objCont->shapeList[i]);
        if (it->getObjectHandle()!=idToExclude)
        {
            std::string name(tt::decorateString("[",strTranslate(IDSN_SHAPE),"] "));
            name+=it->getObjectName();
            names.push_back(name);
            ids.push_back(it->getObjectHandle());
        }
    }
    tt::orderStrings(names,ids);
    for (int i=0;i<int(names.size());i++)
        combo->addItem(names[i].c_str(),QVariant(ids[i]));

    if (!collisionEntities)
    {
        names.clear();
        ids.clear();

        // Now dummies:
        for (int i=0;i<int(App::ct->objCont->dummyList.size());i++)
        {
            CDummy* it=App::ct->objCont->getDummy(App::ct->objCont->dummyList[i]);
            if (it->getObjectHandle()!=idToExclude)
            {
                std::string name(tt::decorateString("[",strTranslate(IDSN_DUMMY),"] "));
                name+=it->getObjectName();
                names.push_back(name);
                ids.push_back(it->getObjectHandle());
            }
        }
        tt::orderStrings(names,ids);
        for (int i=0;i<int(names.size());i++)
            combo->addItem(names[i].c_str(),QVariant(ids[i]));
    }
}

void CQDlgMotionPlanning::fillComboWithIkGroups(QComboBox* combo)
{
    combo->clear();
    combo->addItem(strTranslate(IDSN_NONE),QVariant(-1));

    std::vector<std::string> names;
    std::vector<int> ids;

    // Now collections:
    for (int i=0;i<int(App::ct->ikGroups->ikGroups.size());i++)
    {
        CikGroup* it=App::ct->ikGroups->ikGroups[i];
        std::string name=it->getObjectName();
        names.push_back(name);
        ids.push_back(it->getObjectID());
    }
    tt::orderStrings(names,ids);
    for (int i=0;i<int(names.size());i++)
        combo->addItem(names[i].c_str(),QVariant(ids[i]));
}

bool CQDlgMotionPlanning::selectItemInCombo(QComboBox* combo,int itemID)
{
    for (int i=0;i<combo->count();i++)
    {
        if (combo->itemData(i).toInt()==itemID)
        {
            combo->setCurrentIndex(i);
            return(true);
        }
    }
    return(false);
}

void CQDlgMotionPlanning::updateObjectsInList()
{
    noListSelectionAllowed=true;
    ui->qqList->clear();

    for (int i=0;i<int(App::ct->motionPlanning->allObjects.size());i++)
    {
        CMotionPlanningTask* it=App::ct->motionPlanning->allObjects[i];
        std::string tmp=it->getObjectName();
        tmp+=" (";
        tmp+=boost::lexical_cast<std::string>(it->getJointCount());
        tmp+=" joints)";
        QListWidgetItem* itm=new QListWidgetItem(tmp.c_str());
        itm->setData(Qt::UserRole,QVariant(it->getObjectID()));
        itm->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable|Qt::ItemIsEnabled);
        ui->qqList->addItem(itm);
    }

    noListSelectionAllowed=false;
}

int CQDlgMotionPlanning::getSelectedObjectID()
{
    QList<QListWidgetItem*> sel=ui->qqList->selectedItems();
    if (sel.size()>0)
        return(sel.at(0)->data(Qt::UserRole).toInt());
    return(-1);
}

void CQDlgMotionPlanning::selectObjectInList(int objectID)
{
    noListSelectionAllowed=true;
    for (int i=0;i<ui->qqList->count();i++)
    {
        QListWidgetItem* it=ui->qqList->item(i);
        if (it!=nullptr)
        {
            if (it->data(Qt::UserRole).toInt()==objectID)
            {
                it->setSelected(true);
                break;
            }
        }
    }
    noListSelectionAllowed=false;
}

void CQDlgMotionPlanning::onDeletePressed()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (focusWidget()==ui->qqList)
        {
            App::appendSimulationThreadCommand(REMOVE_OBJECT_MOTIONPLANNINGGUITRIGGEREDCMD,getSelectedObjectID());
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}


void CQDlgMotionPlanning::on_qqList_itemChanged(QListWidgetItem *item)
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (item!=nullptr)
        {
            App::appendSimulationThreadCommand(RENAME_OBJECT_MOTIONPLANNINGGUITRIGGEREDCMD,item->data(Qt::UserRole).toInt(),-1,0.0,0.0,item->text().toStdString().c_str());
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgMotionPlanning::on_qqList_itemSelectionChanged()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (!noListSelectionAllowed)
        {
            int objID=getSelectedObjectID();
            CMotionPlanningTask* it=App::ct->motionPlanning->getObject(objID);
            if (it!=nullptr)
                ((CEditBoxDelegate*)ui->qqList->itemDelegate())->initialText=it->getObjectName();
            else
                ((CEditBoxDelegate*)ui->qqList->itemDelegate())->initialText="";
            inListSelectionRoutine=true;
            refresh();
            inListSelectionRoutine=false;
        }
    }
}

void CQDlgMotionPlanning::on_qqAddNew_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        SSimulationThreadCommand cmd;
        cmd.cmdId=ADD_OBJECT_MOTIONPLANNINGGUITRIGGEREDCMD;
        App::ct->objCont->getSelectedObjects(cmd.intParams);
        bool ok=(cmd.intParams.size()>0);
        for (size_t i=0;i<cmd.intParams.size();i++)
        {
            CJoint* joint=App::ct->objCont->getJoint(cmd.intParams[i]);
            if (joint!=nullptr)
            {
                if (joint->getJointType()==sim_joint_spherical_subtype)
                    ok=false;
            }
            else
                ok=false;
        }
        if (ok)
        {
            App::appendSimulationThreadCommand(CLEAR_OBJECT_SELECTION_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(cmd);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
        else
            App::uiThread->messageBox_information(App::mainWindow,strTranslate(IDSN_MOTION_PLANNING),strTranslate(IDSN_MOTION_PLANNING_SELECTION_INVALID),VMESSAGEBOX_OKELI);
    }
}

void CQDlgMotionPlanning::on_qqEditJointProp_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CMotionPlanningTask* task=App::ct->motionPlanning->getObject(getSelectedObjectID());
        if (task!=nullptr)
        {
            CQDlgMotionPlanningJoints theDialog(this);
            delKeyShortcut->setEnabled(false);
            backspaceKeyShortcut->setEnabled(false);
            std::vector<int> handles;
            task->getJointHandles(handles);
            for (size_t i=0;i<handles.size();i++)
            {
                CJoint* jn=App::ct->objCont->getJoint(handles[i]);
                theDialog.jointNames.push_back(jn->getObjectName());
                float n=task->getRobotMetric(handles[i]);
                theDialog.jointMetricWeight.push_back(n);
                int m=task->getJointStepCount(handles[i]);
                theDialog.jointSubdiv.push_back(m);
            }
            theDialog.refresh();
            theDialog.makeDialogModal();
            backspaceKeyShortcut->setEnabled(true);
            delKeyShortcut->setEnabled(true);
            SSimulationThreadCommand cmd;
            cmd.cmdId=SET_JOINTPROPERTIES_MOTIONPLANNINGGUITRIGGEREDCMD;
            cmd.intParams.push_back(getSelectedObjectID());
            cmd.intVectorParams.push_back(handles);
            cmd.intVectorParams.push_back(theDialog.jointSubdiv);
            cmd.floatVectorParams.push_back(theDialog.jointMetricWeight);
            App::appendSimulationThreadCommand(cmd);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgMotionPlanning::on_qqIkGroupCombo_currentIndexChanged(int index)
{
    if (!inMainRefreshRoutine)
    {
        IF_UI_EVENT_CAN_READ_DATA
        {
            App::appendSimulationThreadCommand(SET_ASSOCIATEDIKGROUP_MOTIONPLANNINGGUITRIGGEREDCMD,getSelectedObjectID(),ui->qqIkGroupCombo->itemData(ui->qqIkGroupCombo->currentIndex()).toInt());
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgMotionPlanning::on_qqEntity1Combo_currentIndexChanged(int index)
{
    if (!inMainRefreshRoutine)
    {
        IF_UI_EVENT_CAN_READ_DATA
        {
            SSimulationThreadCommand cmd;
            cmd.cmdId=SET_ENTITY1_MOTIONPLANNINGGUITRIGGEREDCMD;
            cmd.intParams.push_back(getSelectedObjectID());
            cmd.intParams.push_back(ui->qqEntity1Combo->itemData(ui->qqEntity1Combo->currentIndex()).toInt());
            cmd.boolParams.push_back(true); // self-collision
            App::appendSimulationThreadCommand(cmd);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgMotionPlanning::on_qqEntity2Combo_currentIndexChanged(int index)
{
    if (!inMainRefreshRoutine)
    {
        IF_UI_EVENT_CAN_READ_DATA
        {
            SSimulationThreadCommand cmd;
            cmd.cmdId=SET_ENTITY2_MOTIONPLANNINGGUITRIGGEREDCMD;
            cmd.intParams.push_back(getSelectedObjectID());
            cmd.intParams.push_back(ui->qqEntity2Combo->itemData(ui->qqEntity2Combo->currentIndex()).toInt());
            cmd.boolParams.push_back(true); // self-collision
            App::appendSimulationThreadCommand(cmd);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgMotionPlanning::on_qqSelfColl_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        SSimulationThreadCommand cmd;
        cmd.cmdId=SET_COLLDIST_MOTIONPLANNINGGUITRIGGEREDCMD;
        cmd.intParams.push_back(getSelectedObjectID());
        cmd.floatParams.push_back(0.0);
        cmd.boolParams.push_back(true); // self-collision
        App::appendSimulationThreadCommand(cmd);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgMotionPlanning::on_qqSelfDist_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        SSimulationThreadCommand cmd;
        cmd.cmdId=SET_COLLDIST_MOTIONPLANNINGGUITRIGGEREDCMD;
        cmd.intParams.push_back(getSelectedObjectID());
        cmd.floatParams.push_back(0.02f);
        cmd.boolParams.push_back(true); // self-collision
        App::appendSimulationThreadCommand(cmd);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgMotionPlanning::on_qqSelfDistDist_editingFinished()
{
    if (!ui->qqSelfDistDist->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        float newVal=ui->qqSelfDistDist->text().toFloat(&ok);
        if (ok)
        {
            SSimulationThreadCommand cmd;
            cmd.cmdId=SET_COLLDIST_MOTIONPLANNINGGUITRIGGEREDCMD;
            cmd.intParams.push_back(getSelectedObjectID());
            cmd.floatParams.push_back(newVal);
            cmd.boolParams.push_back(true); // self-collision
            App::appendSimulationThreadCommand(cmd);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgMotionPlanning::on_qqRobotCombo_currentIndexChanged(int index)
{
    if (!inMainRefreshRoutine)
    {
        IF_UI_EVENT_CAN_READ_DATA
        {
            SSimulationThreadCommand cmd;
            cmd.cmdId=SET_ENTITY1_MOTIONPLANNINGGUITRIGGEREDCMD;
            cmd.intParams.push_back(getSelectedObjectID());
            cmd.intParams.push_back(ui->qqRobotCombo->itemData(ui->qqRobotCombo->currentIndex()).toInt());
            cmd.boolParams.push_back(false); // NOT self-collision
            App::appendSimulationThreadCommand(cmd);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgMotionPlanning::on_qqObstacleCombo_currentIndexChanged(int index)
{
    if (!inMainRefreshRoutine)
    {
        IF_UI_EVENT_CAN_READ_DATA
        {
            SSimulationThreadCommand cmd;
            cmd.cmdId=SET_ENTITY2_MOTIONPLANNINGGUITRIGGEREDCMD;
            cmd.intParams.push_back(getSelectedObjectID());
            cmd.intParams.push_back(ui->qqObstacleCombo->itemData(ui->qqObstacleCombo->currentIndex()).toInt());
            cmd.boolParams.push_back(false); // NOT self-collision
            App::appendSimulationThreadCommand(cmd);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgMotionPlanning::on_qqColl_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        SSimulationThreadCommand cmd;
        cmd.cmdId=SET_COLLDIST_MOTIONPLANNINGGUITRIGGEREDCMD;
        cmd.intParams.push_back(getSelectedObjectID());
        cmd.floatParams.push_back(0.0);
        cmd.boolParams.push_back(false); // NOT self-collision
        App::appendSimulationThreadCommand(cmd);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgMotionPlanning::on_qqDist_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        SSimulationThreadCommand cmd;
        cmd.cmdId=SET_COLLDIST_MOTIONPLANNINGGUITRIGGEREDCMD;
        cmd.intParams.push_back(getSelectedObjectID());
        cmd.floatParams.push_back(0.02f);
        cmd.boolParams.push_back(false); // NOT self-collision
        App::appendSimulationThreadCommand(cmd);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgMotionPlanning::on_qqDistDist_editingFinished()
{
    if (!ui->qqDistDist->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        float newVal=ui->qqDistDist->text().toFloat(&ok);
        if (ok)
        {
            SSimulationThreadCommand cmd;
            cmd.cmdId=SET_COLLDIST_MOTIONPLANNINGGUITRIGGEREDCMD;
            cmd.intParams.push_back(getSelectedObjectID());
            cmd.floatParams.push_back(newVal);
            cmd.boolParams.push_back(false); // NOT self-collision
            App::appendSimulationThreadCommand(cmd);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgMotionPlanning::on_qqEditWorkspaceMetric_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CMotionPlanningTask* task=App::ct->motionPlanning->getObject(getSelectedObjectID());
        if (task!=nullptr)
        {
            CQDlgWorkSpaceMetric theDialog(this);
            delKeyShortcut->setEnabled(false);
            backspaceKeyShortcut->setEnabled(false);
            theDialog.data[0]=task->getTipMetric(0);
            theDialog.data[1]=task->getTipMetric(1);
            theDialog.data[2]=task->getTipMetric(2);
            theDialog.data[3]=task->getTipMetric(3);
            theDialog.refresh();
            theDialog.makeDialogModal();
            SSimulationThreadCommand cmd;
            cmd.cmdId=SET_WORKSPACEMETRIC_MOTIONPLANNINGGUITRIGGEREDCMD;
            cmd.intParams.push_back(getSelectedObjectID());
            cmd.floatParams.push_back(theDialog.data[0]);
            cmd.floatParams.push_back(theDialog.data[1]);
            cmd.floatParams.push_back(theDialog.data[2]);
            cmd.floatParams.push_back(theDialog.data[3]);
            App::appendSimulationThreadCommand(cmd);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
            backspaceKeyShortcut->setEnabled(true);
            delKeyShortcut->setEnabled(true);
        }
    }
}

void CQDlgMotionPlanning::on_qqComputeNodes_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(COMPUTE_PHASE1NODES_MOTIONPLANNINGGUITRIGGEREDCMD,getSelectedObjectID());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgMotionPlanning::on_showPhase1Nodes_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_PHASE1NODES_MOTIONPLANNINGGUITRIGGEREDCMD,getSelectedObjectID());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}
