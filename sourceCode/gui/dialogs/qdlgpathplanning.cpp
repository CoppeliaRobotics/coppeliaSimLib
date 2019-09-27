
#include "vrepMainHeader.h"
#include "qdlgpathplanning.h"
#include "ui_qdlgpathplanning.h"
#include "tt.h"
#include "gV.h"
#include "app.h"
#include "editboxdelegate.h"
#include "qdlgpathplanningaddnew.h"
#include "qdlgpathplanningparams.h"
#include "v_repStrings.h"

CQDlgPathPlanning::CQDlgPathPlanning(QWidget *parent) :
    CDlgEx(parent),
    ui(new Ui::CQDlgPathPlanning)
{
    _dlgType=PATH_PLANNING_DLG;
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

CQDlgPathPlanning::~CQDlgPathPlanning()
{
    delete ui;
}

void CQDlgPathPlanning::cancelEvent()
{
    // we override this cancel event. The container window should close, not this one!!
    App::mainWindow->dlgCont->close(CALCULATION_DLG);
}

void CQDlgPathPlanning::dialogCallbackFunc(const SUIThreadCommand* cmdIn,SUIThreadCommand* cmdOut)
{
    if ( (cmdIn!=nullptr)&&(cmdIn->intParams[0]==_dlgType) )
    {
        if (cmdIn->intParams[1]==0)
            selectObjectInList(cmdIn->intParams[2]);
    }
}

void CQDlgPathPlanning::refresh()
{
    inMainRefreshRoutine=true;
    QLineEdit* lineEditToSelect=getSelectedLineEdit();
    bool noEditModeNoSim=(App::getEditModeType()==NO_EDIT_MODE)&&App::ct->simulation->isSimulationStopped();

    int taskID=getSelectedObjectID();
    CPathPlanningTask* task=App::ct->pathPlanning->getObject(taskID);
    ui->qqPathPlanningGroup->setEnabled(noEditModeNoSim);

    if (!inListSelectionRoutine)
    {
        updateObjectsInList();
        selectObjectInList(taskID);
    }

    ui->qqAddNew->setEnabled(noEditModeNoSim);

    ui->qqGoalCombo->setEnabled(noEditModeNoSim&&(task!=nullptr));
    ui->qqPathCombo->setEnabled(noEditModeNoSim&&(task!=nullptr));
    ui->qqCheckTypeCombo->setEnabled(noEditModeNoSim&&(task!=nullptr));
    ui->qqRobotCombo->setEnabled(noEditModeNoSim&&(task!=nullptr));
    ui->qqObstacleCombo->setEnabled(noEditModeNoSim&&(task!=nullptr));
    ui->qqMinDist->setEnabled(noEditModeNoSim&&(task!=nullptr)&&(!task->getCollisionDetection()));
    ui->qqMaxDist->setEnabled(noEditModeNoSim&&(task!=nullptr)&&(!task->getCollisionDetection())&&task->getObstacleMaxDistanceEnabled());
    ui->qqAdjustSearchParameters->setEnabled(noEditModeNoSim&&(task!=nullptr));
    ui->qqUsePartialPath->setEnabled(noEditModeNoSim&&(task!=nullptr));
    ui->qqShowNodes->setEnabled(noEditModeNoSim&&(task!=nullptr));
    ui->qqMaxCalcTime->setEnabled(noEditModeNoSim&&(task!=nullptr));
    ui->qqPostProcessingPasses->setEnabled(noEditModeNoSim&&(task!=nullptr));

    ui->qqComputePath->setEnabled(noEditModeNoSim&&(task!=nullptr)&&task->isTaskValid());


    ui->qqGoalCombo->clear();
    ui->qqPathCombo->clear();
    ui->qqCheckTypeCombo->clear();
    ui->qqRobotCombo->clear();
    ui->qqObstacleCombo->clear();
    ui->qqUsePartialPath->setChecked((task!=nullptr)&&task->getPartialPathIsOk());
    ui->qqShowNodes->setChecked((task!=nullptr)&&task->getShowSearchTrees());

    if (task!=nullptr)
    {
        if (task->getCollisionDetection())
        {
            ui->qqMinDist->setText("");
            ui->qqMaxDist->setText("");
        }
        else
        {
            ui->qqMinDist->setText(tt::getFString(false,task->getObstacleClearance(),3).c_str());
            if (task->getObstacleMaxDistanceEnabled())
                ui->qqMaxDist->setText(tt::getFString(false,task->getObstacleMaxDistance(),3).c_str());
            else
                ui->qqMaxDist->setText("");
        }
        ui->qqMaxCalcTime->setText(tt::getIString(false,(int)task->getMaxTime()).c_str());
        ui->qqPostProcessingPasses->setText(tt::getIString(false,task->getPostProcessingPassCount()).c_str());

        ui->qqCheckTypeCombo->addItem(strTranslate(IDSN_CHECK_ROBOT_OBSTACLE_COLLISION),QVariant(0));
        ui->qqCheckTypeCombo->addItem(strTranslate(IDSN_CHECK_ROBOT_OBSTACLE_MINIMUM_DISTANCE),QVariant(1));
        ui->qqCheckTypeCombo->addItem(strTranslate(IDSN_CHECK_ROBOT_OBSTACLE_MINIMUM_AND_MAXIMUM_DISTANCE),QVariant(2));
        if (task->getCollisionDetection())
            ui->qqCheckTypeCombo->setCurrentIndex(0);
        else
        {
            if (task->getObstacleMaxDistanceEnabled())
                ui->qqCheckTypeCombo->setCurrentIndex(2);
            else
                ui->qqCheckTypeCombo->setCurrentIndex(1);
        }

        std::vector<std::string> names;
        std::vector<int> ids;

        // Goal dummy
        names.clear();
        ids.clear();
        ui->qqGoalCombo->addItem(strTranslate(IDSN_NONE),QVariant(-1));
        for (size_t i=0;i<App::ct->objCont->dummyList.size();i++)
        {
            CDummy* it2=App::ct->objCont->getDummy(App::ct->objCont->dummyList[i]);
            if (it2->getObjectHandle()!=task->getStartDummyID())
            {
                names.push_back(it2->getObjectName());
                ids.push_back(it2->getObjectHandle());
            }
        }
        tt::orderStrings(names,ids);
        for (size_t i=0;i<names.size();i++)
            ui->qqGoalCombo->addItem(names[i].c_str(),QVariant(ids[i]));
        for (int i=0;i<ui->qqGoalCombo->count();i++)
        {
            if (ui->qqGoalCombo->itemData(i).toInt()==task->getGoalDummyID())
            {
                ui->qqGoalCombo->setCurrentIndex(i);
                break;
            }
        }

        // Path object
        names.clear();
        ids.clear();
        ui->qqPathCombo->addItem(strTranslate(IDSN_NONE),QVariant(-1));
        for (size_t i=0;i<App::ct->objCont->pathList.size();i++)
        {
            CPath* it2=App::ct->objCont->getPath(App::ct->objCont->pathList[i]);
            names.push_back(it2->getObjectName());
            ids.push_back(it2->getObjectHandle());
        }
        tt::orderStrings(names,ids);
        for (size_t i=0;i<names.size();i++)
            ui->qqPathCombo->addItem(names[i].c_str(),QVariant(ids[i]));
        for (int i=0;i<ui->qqPathCombo->count();i++)
        {
            if (ui->qqPathCombo->itemData(i).toInt()==task->getPathID())
            {
                ui->qqPathCombo->setCurrentIndex(i);
                break;
            }
        }

        // robot:
        fillComboWithCollisionOrDistanceEntities(ui->qqRobotCombo,task->getCollisionDetection(),false,true,-1);
        if (!selectItemInCombo(ui->qqRobotCombo,task->getRobotEntityID()))
        { // should not happen
            task->setRobotEntityID(-1); // none
            ui->qqRobotCombo->setCurrentIndex(0);
        }

        // obstacles:
        fillComboWithCollisionOrDistanceEntities(ui->qqObstacleCombo,task->getCollisionDetection(),true,false,task->getRobotEntityID());
        if (!selectItemInCombo(ui->qqObstacleCombo,task->getObstacleEntityID()))
        { // should not happen
            task->setObstacleEntityID(-1); // all
            ui->qqObstacleCombo->setCurrentIndex(0);
        }

    }
    else
    {
        ui->qqMinDist->setText("");
        ui->qqMaxDist->setText("");
        ui->qqMaxCalcTime->setText("");
        ui->qqPostProcessingPasses->setText("");
    }

    selectLineEdit(lineEditToSelect);
    inMainRefreshRoutine=false;
}

void CQDlgPathPlanning::fillComboWithCollisionOrDistanceEntities(QComboBox* combo,bool collisionEntities,bool insertAllAtBeginning,bool insertNoneAtBeginning,int idToExclude)
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
    for (size_t i=0;i<App::ct->collections->allCollections.size();i++)
    {
        CRegCollection* it=App::ct->collections->allCollections[i];
        std::string name(tt::decorateString("[",strTranslate(IDSN_COLLECTION),"] "));
        name+=it->getCollectionName();
        names.push_back(name);
        ids.push_back(it->getCollectionID());
    }
    tt::orderStrings(names,ids);
    for (size_t i=0;i<names.size();i++)
        combo->addItem(names[i].c_str(),QVariant(ids[i]));

    names.clear();
    ids.clear();

    // Now shapes:
    for (size_t i=0;i<App::ct->objCont->shapeList.size();i++)
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
    for (size_t i=0;i<names.size();i++)
        combo->addItem(names[i].c_str(),QVariant(ids[i]));

    if (!collisionEntities)
    {
        names.clear();
        ids.clear();

        // Now dummies:
        for (size_t i=0;i<App::ct->objCont->dummyList.size();i++)
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
        for (size_t i=0;i<names.size();i++)
            combo->addItem(names[i].c_str(),QVariant(ids[i]));
    }
}

bool CQDlgPathPlanning::selectItemInCombo(QComboBox* combo,int itemID)
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

void CQDlgPathPlanning::updateObjectsInList()
{
    noListSelectionAllowed=true;
    ui->qqList->clear();

    for (size_t i=0;i<App::ct->pathPlanning->allObjects.size();i++)
    {
        CPathPlanningTask* it=App::ct->pathPlanning->allObjects[i];
        std::string tmp=it->getObjectName();
        if (it->getPathPlanningType()==sim_pathplanning_holonomic)
        { // We have a holonomic path planning here:
            CDummy* startDummy=App::ct->objCont->getDummy(it->getStartDummyID());
            tmp=tmp.append(" [");
            tmp=tmp.append(startDummy->getObjectName().c_str());
            tmp=tmp.append("] (");
            tmp=tmp.append(IDSN_HOLONOMIC);
            tmp=tmp.append(")");
        }
        if (it->getPathPlanningType()==sim_pathplanning_nonholonomic)
        { // We have a holonomic path planning here:
            CDummy* startDummy=App::ct->objCont->getDummy(it->getStartDummyID());
            tmp=tmp.append(" [");
            tmp=tmp.append(startDummy->getObjectName().c_str());
            tmp=tmp.append("] (");
            tmp=tmp.append(IDSN_NON_HOLONOMIC);
            tmp=tmp.append(")");
        }
        QListWidgetItem* itm=new QListWidgetItem(tmp.c_str());
        itm->setData(Qt::UserRole,QVariant(it->getObjectID()));
        itm->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable|Qt::ItemIsEnabled);
        ui->qqList->addItem(itm);
    }

    noListSelectionAllowed=false;
}

int CQDlgPathPlanning::getSelectedObjectID()
{
    QList<QListWidgetItem*> sel=ui->qqList->selectedItems();
    if (sel.size()>0)
        return(sel.at(0)->data(Qt::UserRole).toInt());
    return(-1);
}

void CQDlgPathPlanning::selectObjectInList(int objectID)
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

void CQDlgPathPlanning::onDeletePressed()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (focusWidget()==ui->qqList)
        {
            App::appendSimulationThreadCommand(REMOVE_OBJECT_PATHPLANNINGGUITRIGGEREDCMD,getSelectedObjectID());
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}


void CQDlgPathPlanning::on_qqList_itemChanged(QListWidgetItem *item)
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (item!=nullptr)
        {
            App::appendSimulationThreadCommand(RENAME_OBJECT_PATHPLANNINGGUITRIGGEREDCMD,item->data(Qt::UserRole).toInt(),-1,0.0,0.0,item->text().toStdString().c_str());
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgPathPlanning::on_qqList_itemSelectionChanged()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (!noListSelectionAllowed)
        {
            int objID=getSelectedObjectID();
            CPathPlanningTask* it=App::ct->pathPlanning->getObject(objID);
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

void CQDlgPathPlanning::on_qqAddNew_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CQDlgPathPlanningAddNew theDialog(this);
        delKeyShortcut->setEnabled(false);
        backspaceKeyShortcut->setEnabled(false);
        theDialog.refresh();
        if (theDialog.makeDialogModal()!=VDIALOG_MODAL_RETURN_CANCEL)
        {
            App::appendSimulationThreadCommand(ADD_OBJECT_PATHPLANNINGGUITRIGGEREDCMD,theDialog.selectedTaskType,theDialog.selectedDummy);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
        backspaceKeyShortcut->setEnabled(true);
        delKeyShortcut->setEnabled(true);
    }
}

void CQDlgPathPlanning::on_qqGoalCombo_currentIndexChanged(int index)
{
    if (!inMainRefreshRoutine)
    {
        IF_UI_EVENT_CAN_READ_DATA
        {
            App::appendSimulationThreadCommand(SET_GOALDUMMY_PATHPLANNINGGUITRIGGEREDCMD,getSelectedObjectID(),ui->qqGoalCombo->itemData(ui->qqGoalCombo->currentIndex()).toInt());
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgPathPlanning::on_qqPathCombo_currentIndexChanged(int index)
{
    if (!inMainRefreshRoutine)
    {
        IF_UI_EVENT_CAN_READ_DATA
        {
            App::appendSimulationThreadCommand(SET_PATHOBJECT_PATHPLANNINGGUITRIGGEREDCMD,getSelectedObjectID(),ui->qqPathCombo->itemData(ui->qqPathCombo->currentIndex()).toInt());
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgPathPlanning::on_qqCheckTypeCombo_currentIndexChanged(int index)
{
    if (!inMainRefreshRoutine)
    {
        IF_UI_EVENT_CAN_READ_DATA
        {
            App::appendSimulationThreadCommand(SET_CHECKTYPE_PATHPLANNINGGUITRIGGEREDCMD,getSelectedObjectID(),ui->qqCheckTypeCombo->itemData(ui->qqCheckTypeCombo->currentIndex()).toInt());
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgPathPlanning::on_qqRobotCombo_currentIndexChanged(int index)
{
    if (!inMainRefreshRoutine)
    {
        IF_UI_EVENT_CAN_READ_DATA
        {
            App::appendSimulationThreadCommand(SET_ROBOT_PATHPLANNINGGUITRIGGEREDCMD,getSelectedObjectID(),ui->qqRobotCombo->itemData(ui->qqRobotCombo->currentIndex()).toInt());
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgPathPlanning::on_qqObstacleCombo_currentIndexChanged(int index)
{
    if (!inMainRefreshRoutine)
    {
        IF_UI_EVENT_CAN_READ_DATA
        {
            App::appendSimulationThreadCommand(SET_OBSTACLE_PATHPLANNINGGUITRIGGEREDCMD,getSelectedObjectID(),ui->qqObstacleCombo->itemData(ui->qqObstacleCombo->currentIndex()).toInt());
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgPathPlanning::on_qqMinDist_editingFinished()
{
    if (!ui->qqMinDist->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        float newVal=ui->qqMinDist->text().toFloat(&ok);
        if (ok)
        {
            App::appendSimulationThreadCommand(SET_DISTANCES_PATHPLANNINGGUITRIGGEREDCMD,getSelectedObjectID(),0,newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgPathPlanning::on_qqMaxDist_editingFinished()
{
    if (!ui->qqMaxDist->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        float newVal=ui->qqMaxDist->text().toFloat(&ok);
        if (ok)
        {
            App::appendSimulationThreadCommand(SET_DISTANCES_PATHPLANNINGGUITRIGGEREDCMD,getSelectedObjectID(),1,newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgPathPlanning::on_qqAdjustSearchParameters_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CPathPlanningTask* it=App::ct->pathPlanning->getObject(getSelectedObjectID());
        if (it!=nullptr)
        {
            CQDlgPathPlanningParams theDialog(this);
            delKeyShortcut->setEnabled(false);
            backspaceKeyShortcut->setEnabled(false);

            theDialog.planningType=it->getPathPlanningType();
            theDialog.holonomicType=it->getHolonomicTaskType();
            theDialog.visualizeArea=it->getVisualizeSearchArea();
            it->getSearchRange(theDialog.searchRangeMin,theDialog.searchRangeMax);
            it->getSearchDirection(theDialog.searchDir);
            theDialog.stepSize=it->getStepSize();
            theDialog.angularStepSize=it->getAngularStepSize();
            theDialog.gammaAxis=it->getGammaAxis();
            theDialog.minTurningCircleDiameter=it->getMinTurningCircleDiameter();

            theDialog.refresh();
            theDialog.makeDialogModal();
            backspaceKeyShortcut->setEnabled(true);
            delKeyShortcut->setEnabled(true);

            SSimulationThreadCommand cmd;
            cmd.cmdId=SET_VARIOUSPARAMS_PATHPLANNINGGUITRIGGEREDCMD;
            cmd.intParams.push_back(getSelectedObjectID());
            cmd.intParams.push_back(theDialog.planningType);
            cmd.intParams.push_back(theDialog.holonomicType);
            cmd.boolParams.push_back(theDialog.visualizeArea);
            cmd.floatParams.push_back(theDialog.stepSize);
            cmd.floatParams.push_back(theDialog.angularStepSize);
            cmd.floatParams.push_back(theDialog.minTurningCircleDiameter);
            cmd.posParams.push_back(theDialog.gammaAxis);
            cmd.floatVectorParams.push_back(std::vector<float>());
            cmd.floatVectorParams.push_back(std::vector<float>());
            cmd.intVectorParams.push_back(std::vector<int>());
            for (int i=0;i<4;i++)
            {
                cmd.floatVectorParams[0].push_back(theDialog.searchRangeMin[i]);
                cmd.floatVectorParams[1].push_back(theDialog.searchRangeMax[i]);
                cmd.intVectorParams[0].push_back(theDialog.searchDir[i]);
            }
            App::appendSimulationThreadCommand(cmd);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgPathPlanning::on_qqUsePartialPath_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_USEPARTIALPATH_PATHPLANNINGGUITRIGGEREDCMD,getSelectedObjectID());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgPathPlanning::on_qqShowNodes_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_SHOWSEARCHEDNODES_PATHPLANNINGGUITRIGGEREDCMD,getSelectedObjectID());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgPathPlanning::on_qqMaxCalcTime_editingFinished()
{
    if (!ui->qqMaxCalcTime->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        int newVal=ui->qqMaxCalcTime->text().toInt(&ok);
        if (ok)
        {
            App::appendSimulationThreadCommand(SET_CALCTIME_PATHPLANNINGGUITRIGGEREDCMD,getSelectedObjectID(),newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgPathPlanning::on_qqPostProcessingPasses_editingFinished()
{
    if (!ui->qqPostProcessingPasses->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        int newVal=ui->qqPostProcessingPasses->text().toInt(&ok);
        if (ok)
        {
            App::appendSimulationThreadCommand(SET_POSTPROCESSINGPASSES_PATHPLANNINGGUITRIGGEREDCMD,getSelectedObjectID(),newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgPathPlanning::on_qqComputePath_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(COMPUTE_PATHPLANNINGGUITRIGGEREDCMD,getSelectedObjectID());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}
