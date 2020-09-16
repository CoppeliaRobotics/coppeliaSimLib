#include "qdlgik.h"
#include "ui_qdlgik.h"
#include "tt.h"
#include "gV.h"
#include "app.h"
#include "editboxdelegate.h"
#include "qdlgikelements.h"
#include "qdlgikconditional.h"
#include "simStrings.h"

CQDlgIk::CQDlgIk(QWidget *parent) :
    CDlgEx(parent),
    ui(new Ui::CQDlgIk)
{
    _dlgType=IK_DLG;
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

CQDlgIk::~CQDlgIk()
{
    CQDlgIkElements::_invalid=true;
    App::mainWindow->dlgCont->close(IKELEMENT_DLG);
    delete ui;
}

void CQDlgIk::cancelEvent()
{
    // we override this cancel event. The container window should close, not this one!!
    App::mainWindow->dlgCont->close(CALCULATION_DLG);
    App::mainWindow->dlgCont->close(IKELEMENT_DLG);
}

void CQDlgIk::dialogCallbackFunc(const SUIThreadCommand* cmdIn,SUIThreadCommand* cmdOut)
{
    if ( (cmdIn!=nullptr)&&(cmdIn->intParams[0]==_dlgType) )
    {
        if (cmdIn->intParams[1]==0)
            selectObjectInList(cmdIn->intParams[2]);
    }
}

void CQDlgIk::refresh()
{
    inMainRefreshRoutine=true;
    QLineEdit* lineEditToSelect=getSelectedLineEdit();
    bool noEditModeNoSim=(App::getEditModeType()==NO_EDIT_MODE)&&App::currentWorld->simulation->isSimulationStopped();

    if (!App::currentWorld->mainSettings->ikCalculationEnabled)
        noEditModeNoSim=false;

    int groupID=getSelectedObjectID();
    CIkGroup* it=App::currentWorld->ikGroups->getObjectFromHandle(groupID);

    if (!inListSelectionRoutine)
    {
        updateObjectsInList();
        selectObjectInList(groupID);
    }

    ui->qqAddNewGroup->setEnabled(noEditModeNoSim);
    ui->qqList->setEnabled(noEditModeNoSim);
    ui->qqUp->setEnabled((it!=nullptr)&&noEditModeNoSim);
    ui->qqDown->setEnabled((it!=nullptr)&&noEditModeNoSim);
    ui->qqGroupIsActive->setEnabled((it!=nullptr)&&noEditModeNoSim);

    ui->qqExplicitHandling->setEnabled((it!=nullptr)&&noEditModeNoSim&&it->getEnabled());
    ui->qqIgnoreMaxStepSizes->setEnabled((it!=nullptr)&&noEditModeNoSim&&it->getEnabled());
    ui->qqCalcMethodCombo->setEnabled((it!=nullptr)&&noEditModeNoSim&&it->getEnabled());
    ui->qqDamping->setEnabled(((it!=nullptr)&&noEditModeNoSim)&&(it->getCalculationMethod()==sim_ik_damped_least_squares_method)&&it->getEnabled());
    ui->qqMaxIterations->setEnabled((it!=nullptr)&&noEditModeNoSim&&it->getEnabled());
    ui->qqEditConditional->setEnabled((it!=nullptr)&&noEditModeNoSim&&it->getEnabled());
    ui->qqEditIkElements->setEnabled((it!=nullptr)&&noEditModeNoSim);

    ui->qqIkEnabled->setChecked(App::currentWorld->mainSettings->ikCalculationEnabled);

    ui->qqExplicitHandling->setChecked((it!=nullptr)&&it->getExplicitHandling());
    ui->qqGroupIsActive->setChecked((it!=nullptr)&&it->getEnabled());
    ui->qqIgnoreMaxStepSizes->setChecked((it!=nullptr)&&it->getIgnoreMaxStepSizes());

    ui->qqCalcMethodCombo->clear();

    if (it!=nullptr)
    {
        ui->qqCalcMethodCombo->addItem(IDS_PSEUDO_INVERSE,QVariant(sim_ik_pseudo_inverse_method));
        ui->qqCalcMethodCombo->addItem(IDS_DLS,QVariant(sim_ik_damped_least_squares_method));
        ui->qqCalcMethodCombo->addItem(IDS_JACOBIAN_TRANSPOSE,QVariant(sim_ik_jacobian_transpose_method));
        ui->qqCalcMethodCombo->addItem(IDS_UNDAMPED_PSEUDO_INVERSE,QVariant(sim_ik_undamped_pseudo_inverse_method));
        for (int i=0;i<ui->qqCalcMethodCombo->count();i++)
        {
            if (ui->qqCalcMethodCombo->itemData(i).toInt()==it->getCalculationMethod())
            {
                ui->qqCalcMethodCombo->setCurrentIndex(i);
                break;
            }
        }

        ui->qqDamping->setText(tt::getFString(false,it->getDampingFactor(),4).c_str());
        ui->qqMaxIterations->setText(tt::getIString(false,it->getMaxIterations()).c_str());
    }
    else
    {
        ui->qqDamping->setText("");
        ui->qqMaxIterations->setText("");
    }

    selectLineEdit(lineEditToSelect);
    inMainRefreshRoutine=false;
}

void CQDlgIk::updateObjectsInList()
{
    noListSelectionAllowed=true;
    ui->qqList->clear();

    for (size_t i=0;i<App::currentWorld->ikGroups->getObjectCount();i++)
    {
        CIkGroup* ikg=App::currentWorld->ikGroups->getObjectFromIndex(i);
        std::string txt=ikg->getObjectName();
        txt+=" [containing ";
        txt+=tt::FNb(0,int(ikg->getIkElementCount())).c_str();
        txt+=" ik element(s)]";
        int objID=ikg->getObjectHandle();
        QListWidgetItem* itm=new QListWidgetItem(txt.c_str());
        itm->setData(Qt::UserRole,QVariant(objID));
        itm->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable|Qt::ItemIsEnabled);
        ui->qqList->addItem(itm);
    }

    noListSelectionAllowed=false;
}

int CQDlgIk::getSelectedObjectID()
{
    QList<QListWidgetItem*> sel=ui->qqList->selectedItems();
    if (sel.size()>0)
        return(sel.at(0)->data(Qt::UserRole).toInt());
    return(-1);
}

void CQDlgIk::selectObjectInList(int objectID)
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

void CQDlgIk::onDeletePressed()
{
    if (focusWidget()==ui->qqList)
    {
        IF_UI_EVENT_CAN_READ_DATA
        {
            CQDlgIkElements::_invalid=true;
            App::mainWindow->dlgCont->close(IKELEMENT_DLG);
            App::appendSimulationThreadCommand(REMOVE_IKGROUP_IKGROUPGUITRIGGEREDCMD,getSelectedObjectID());
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgIk::on_qqIkEnabled_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_ALLENABLED_IKGROUPGUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgIk::on_qqAddNewGroup_clicked()
{
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        CQDlgIkElements::_invalid=true;
        App::mainWindow->dlgCont->close(IKELEMENT_DLG);
        App::appendSimulationThreadCommand(ADD_IKGROUP_IKGROUPGUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgIk::on_qqList_itemChanged(QListWidgetItem *item)
{
    if (item!=nullptr)
    {
        IF_UI_EVENT_CAN_WRITE_DATA
        {
            CQDlgIkElements::_invalid=true;
            App::mainWindow->dlgCont->close(IKELEMENT_DLG);
            App::appendSimulationThreadCommand(RENAME_IKGROUP_IKGROUPGUITRIGGEREDCMD,item->data(Qt::UserRole).toInt(),-1,0.0,0.0,item->text().toStdString().c_str());
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgIk::on_qqList_itemSelectionChanged()
{
    if (!noListSelectionAllowed)
    {
        IF_UI_EVENT_CAN_WRITE_DATA
        {
            CQDlgIkElements::_invalid=true;
            App::mainWindow->dlgCont->close(IKELEMENT_DLG);
            int objID=getSelectedObjectID();
            CIkGroup* it=App::currentWorld->ikGroups->getObjectFromHandle(objID);
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

void CQDlgIk::on_qqUp_clicked()
{
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        App::appendSimulationThreadCommand(SHIFT_IKGROUP_IKGROUPGUITRIGGEREDCMD,getSelectedObjectID(),1);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgIk::on_qqDown_clicked()
{
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        App::appendSimulationThreadCommand(SHIFT_IKGROUP_IKGROUPGUITRIGGEREDCMD,getSelectedObjectID(),0);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgIk::on_qqExplicitHandling_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_EXPLICITHANDLING_IKGROUPGUITRIGGEREDCMD,getSelectedObjectID());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgIk::on_qqGroupIsActive_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_ACTIVE_IKGROUPGUITRIGGEREDCMD,getSelectedObjectID());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgIk::on_qqIgnoreMaxStepSizes_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_IGNOREMAXSTEPSIZES_IKGROUPGUITRIGGEREDCMD,getSelectedObjectID());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgIk::on_qqCalcMethodCombo_currentIndexChanged(int index)
{
    if (!inMainRefreshRoutine)
    {
        IF_UI_EVENT_CAN_READ_DATA
        {
            App::appendSimulationThreadCommand(SET_CALCMETHOD_IKGROUPGUITRIGGEREDCMD,getSelectedObjectID(),ui->qqCalcMethodCombo->itemData(ui->qqCalcMethodCombo->currentIndex()).toInt());
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgIk::on_qqDamping_editingFinished()
{
    if (!ui->qqDamping->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        float newVal=ui->qqDamping->text().toFloat(&ok);
        if (ok)
        {
            App::appendSimulationThreadCommand(SET_DAMPING_IKGROUPGUITRIGGEREDCMD,getSelectedObjectID(),-1,newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgIk::on_qqMaxIterations_editingFinished()
{
    if (!ui->qqMaxIterations->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        int newVal=ui->qqMaxIterations->text().toInt(&ok);
        if (ok)
        {
            App::appendSimulationThreadCommand(SET_ITERATIONS_IKGROUPGUITRIGGEREDCMD,getSelectedObjectID(),newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgIk::on_qqEditConditional_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CIkGroup* it=App::currentWorld->ikGroups->getObjectFromHandle(getSelectedObjectID());
        if (it!=nullptr)
        {
            CQDlgIkConditional theDialog(this);
            theDialog.ikGroup=it;
            theDialog.refresh();
//          delKeyShortcut->setEnabled(false);
//          backspaceKeyShortcut->setEnabled(false);
            theDialog.makeDialogModal();
            SSimulationThreadCommand cmd;
            cmd.cmdId=SET_CONDITIONALPARAMS_IKGROUPGUITRIGGEREDCMD;
            cmd.intParams.push_back(getSelectedObjectID());
            cmd.intParams.push_back(theDialog.doOnFailOrSuccessOf);
            cmd.intParams.push_back(theDialog.ikResult);
            cmd.boolParams.push_back(theDialog.restoreIfPosNotReached);
            cmd.boolParams.push_back(theDialog.restoreIfOrNotReached);
            App::appendSimulationThreadCommand(cmd);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgIk::on_qqEditIkElements_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CIkGroup* it=App::currentWorld->ikGroups->getObjectFromHandle(getSelectedObjectID());
        if (it!=nullptr)
            CQDlgIkElements::display(it->getObjectHandle(),App::mainWindow);
    }
}
