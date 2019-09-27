#include "vrepMainHeader.h"
#include "qdlgscripts.h"
#include "ui_qdlgscripts.h"
#include "tt.h"
#include <QShortcut>
#include "qdlginsertscript.h"
#include "app.h"
#include "v_repStrings.h"
#include "jointObject.h"
#include "vMessageBox.h"

int CQDlgScripts::scriptViewMode=0;

CQDlgScripts::CQDlgScripts(QWidget *parent) :
    CDlgEx(parent),
    ui(new Ui::CQDlgScripts)
{
    _dlgType=LUA_SCRIPT_DLG;
    ui->setupUi(this);
    inSelectionRoutine=false;
    rebuildingRoutine=false;
    inMainRefreshRoutine=false;
    QShortcut* shortcut = new QShortcut(QKeySequence(Qt::Key_Delete), this);
    connect(shortcut,SIGNAL(activated()), this, SLOT(onDeletePressed()));
    QShortcut* shortcut2 = new QShortcut(QKeySequence(Qt::Key_Backspace), this);
    connect(shortcut2,SIGNAL(activated()), this, SLOT(onDeletePressed()));
}

CQDlgScripts::~CQDlgScripts()
{
    delete ui;
}

void CQDlgScripts::dialogCallbackFunc(const SUIThreadCommand* cmdIn,SUIThreadCommand* cmdOut)
{
    if ( (cmdIn!=nullptr)&&(cmdIn->intParams[0]==_dlgType) )
    {
        if (cmdIn->intParams[1]==0)
            selectObjectInList(cmdIn->intParams[2]);
    }
}

void CQDlgScripts::refresh()
{
    inMainRefreshRoutine=true;
    bool noEditMode=(App::getEditModeType()==NO_EDIT_MODE);
    bool noEditModeAndNoSim=noEditMode&&App::ct->simulation->isSimulationStopped();

    ui->qqCombo->setEnabled(noEditMode);
    ui->qqCombo->clear();
    ui->qqCombo->addItem(strTranslate(IDS_SIMULATION_SCRIPTS),QVariant(0));
    ui->qqCombo->addItem(strTranslate(IDS_CUSTOMIZATION_SCRIPTS),QVariant(1));

    ui->qqCombo->setCurrentIndex(scriptViewMode);

    int selectedObjectID=getSelectedObjectID();
    updateObjectsInList();
    selectObjectInList(selectedObjectID);

    ui->qqAddNewScript->setEnabled(noEditModeAndNoSim);
    ui->qqScriptList->setEnabled(noEditMode);

    ui->qqExecutionOrder->clear();
    ui->qqTreeTraversalDirection->clear();
    ui->qqDebugMode->clear();
    ui->qqAssociatedObjectCombo->clear();

    CLuaScriptObject* theScript=App::ct->luaScriptContainer->getScriptFromID_alsoAddOnsAndSandbox(getSelectedObjectID());
    ui->qqExecutionOrder->setEnabled((theScript!=nullptr)&&noEditModeAndNoSim&&( (theScript->getScriptType()==sim_scripttype_childscript)||(theScript->getScriptType()==sim_scripttype_customizationscript) ));
    ui->qqTreeTraversalDirection->setEnabled((theScript!=nullptr)&&noEditModeAndNoSim&&( (theScript->getScriptType()==sim_scripttype_childscript)||(theScript->getScriptType()==sim_scripttype_customizationscript) ));
    ui->qqDebugMode->setEnabled((theScript!=nullptr)&&noEditModeAndNoSim&&( (theScript->getScriptType()==sim_scripttype_childscript)||(theScript->getScriptType()==sim_scripttype_customizationscript)||(theScript->getScriptType()==sim_scripttype_mainscript) ));
    ui->qqAssociatedObjectCombo->setEnabled((theScript!=nullptr)&&noEditModeAndNoSim&&( (theScript->getScriptType()==sim_scripttype_childscript)||(theScript->getScriptType()==sim_scripttype_customizationscript) ));
    ui->qqDisabled->setEnabled((theScript!=nullptr)&&noEditMode&&( (theScript->getScriptType()==sim_scripttype_childscript)||(theScript->getScriptType()==sim_scripttype_customizationscript) ));
    ui->qqExecuteOnce->setEnabled((theScript!=nullptr)&&noEditModeAndNoSim&&(theScript->getScriptType()==sim_scripttype_childscript)&&theScript->getThreadedExecution());

    if (theScript!=nullptr)
    {
        if ( (theScript->getScriptType()==sim_scripttype_childscript)||(theScript->getScriptType()==sim_scripttype_customizationscript) )
        {
            ui->qqExecutionOrder->addItem(strTranslate(IDSN_FIRST),QVariant(sim_scriptexecorder_first));
            ui->qqExecutionOrder->addItem(strTranslate(IDSN_NORMAL),QVariant(sim_scriptexecorder_normal));
            ui->qqExecutionOrder->addItem(strTranslate(IDSN_LAST),QVariant(sim_scriptexecorder_last));
            ui->qqExecutionOrder->setCurrentIndex(theScript->getExecutionOrder());

            ui->qqTreeTraversalDirection->addItem(strTranslate(IDSN_REVERSE_TRAVERSAL),QVariant(sim_scripttreetraversal_reverse));
            ui->qqTreeTraversalDirection->addItem(strTranslate(IDSN_FORWARD_TRAVERSAL),QVariant(sim_scripttreetraversal_forward));
            ui->qqTreeTraversalDirection->addItem(strTranslate(IDSN_PARENT_TRAVERSAL),QVariant(sim_scripttreetraversal_parent));
            ui->qqTreeTraversalDirection->setCurrentIndex(theScript->getTreeTraversalDirection());

            ui->qqDebugMode->addItem(strTranslate(IDSN_SCRIPTDEBUG_NONE),QVariant(sim_scriptdebug_none));
            ui->qqDebugMode->addItem(strTranslate(IDSN_SCRIPTDEBUG_SYSCALLS),QVariant(sim_scriptdebug_syscalls));
            ui->qqDebugMode->addItem(strTranslate(IDSN_SCRIPTDEBUG_VARS_1SEC),QVariant(sim_scriptdebug_vars_interval));
            ui->qqDebugMode->addItem(strTranslate(IDSN_SCRIPTDEBUG_ALLCALLS),QVariant(sim_scriptdebug_allcalls));
            ui->qqDebugMode->addItem(strTranslate(IDSN_SCRIPTDEBUG_VARS),QVariant(sim_scriptdebug_vars));
            ui->qqDebugMode->addItem(strTranslate(IDSN_SCRIPTDEBUG_FULL),QVariant(sim_scriptdebug_callsandvars));
            ui->qqDebugMode->setCurrentIndex(theScript->getDebugLevel());

            ui->qqAssociatedObjectCombo->addItem(strTranslate(IDSN_NONE),QVariant(-1));
            std::vector<std::string> names;
            std::vector<int> ids;
            for (int i=0;i<int(App::ct->objCont->objectList.size());i++)
            {
                C3DObject* it=App::ct->objCont->getObjectFromHandle(App::ct->objCont->objectList[i]);
                CLuaScriptObject* so=nullptr;
                if (theScript->getScriptType()==sim_scripttype_childscript)
                    so=App::ct->luaScriptContainer->getScriptFromObjectAttachedTo_child(it->getObjectHandle());
                if (theScript->getScriptType()==sim_scripttype_customizationscript)
                    so=App::ct->luaScriptContainer->getScriptFromObjectAttachedTo_customization(it->getObjectHandle());
                if ( (so==nullptr)||(so==theScript) )
                {
                    names.push_back(it->getObjectName());
                    ids.push_back(it->getObjectHandle());
                }
            }
            tt::orderStrings(names,ids);
            for (int i=0;i<int(names.size());i++)
                ui->qqAssociatedObjectCombo->addItem(names[i].c_str(),QVariant(ids[i]));
            int objIdAttached=-1;
            if (theScript->getScriptType()==sim_scripttype_childscript)
                objIdAttached=theScript->getObjectIDThatScriptIsAttachedTo_child();
            if (theScript->getScriptType()==sim_scripttype_customizationscript)
                objIdAttached=theScript->getObjectIDThatScriptIsAttachedTo_customization();
            for (int i=0;i<ui->qqAssociatedObjectCombo->count();i++)
            {
                if (ui->qqAssociatedObjectCombo->itemData(i).toInt()==objIdAttached)
                {
                    ui->qqAssociatedObjectCombo->setCurrentIndex(i);
                    break;
                }
            }
        }

        if (theScript->getScriptType()==sim_scripttype_customizationscript)
            ui->qqDisabled->setChecked(theScript->getScriptIsDisabled()||theScript->getCustomizationScriptIsTemporarilyDisabled());
        else
            ui->qqDisabled->setChecked(theScript->getScriptIsDisabled());

        ui->qqDisableWithError->setEnabled(theScript->getScriptType()==sim_scripttype_customizationscript);
        if (theScript->getScriptType()==sim_scripttype_customizationscript)
            ui->qqDisableWithError->setChecked(theScript->getDisableCustomizationScriptWithError());
        else
            ui->qqDisableWithError->setChecked(false);

        ui->qqExecuteOnce->setChecked(theScript->getExecuteJustOnce());
    }
    else
    {
        ui->qqDisableWithError->setEnabled(false);
        ui->qqDisableWithError->setChecked(false);
        ui->qqDisabled->setChecked(false);
        ui->qqExecuteOnce->setChecked(false);
    }
    inMainRefreshRoutine=false;
}

void CQDlgScripts::updateObjectsInList()
{
    rebuildingRoutine=true;
    ui->qqScriptList->clear();

    if (scriptViewMode==0)
    { // Main and child scripts
        CLuaScriptObject* it=App::ct->luaScriptContainer->getMainScript();
        if (it!=nullptr)
        {
            std::string tmp=it->getDescriptiveName();
            int id=it->getScriptID();
            QListWidgetItem* itm=new QListWidgetItem(tmp.c_str());
            itm->setData(Qt::UserRole,QVariant(id));
            itm->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
            itm->setForeground(QColor(255,128,128)); // RED
            ui->qqScriptList->addItem(itm);
        }
        for (int i=0;i<int(App::ct->luaScriptContainer->allScripts.size());i++)
        {
            it=App::ct->luaScriptContainer->allScripts[i];
            int t=it->getScriptType();
            if (t==sim_scripttype_childscript)
            {
                std::string tmp=it->getDescriptiveName();
                int id=it->getScriptID();
                QListWidgetItem* itm=new QListWidgetItem(tmp.c_str());
                itm->setData(Qt::UserRole,QVariant(id));
                itm->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
                if (it->getThreadedExecution())
                    itm->setForeground(QColor(128,205,205)); // CYAN
                else
                    itm->setForeground(QColor(128,128,128)); // GREY
                ui->qqScriptList->addItem(itm);
            }
        }
    }

    if (scriptViewMode==1)
    { // Customization scripts
        for (int i=0;i<int(App::ct->luaScriptContainer->allScripts.size());i++)
        {
            CLuaScriptObject* it=App::ct->luaScriptContainer->allScripts[i];
            int t=it->getScriptType();
            if (t==sim_scripttype_customizationscript)
            {
                std::string tmp=it->getDescriptiveName();
                int id=it->getScriptID();
                QListWidgetItem* itm=new QListWidgetItem(tmp.c_str());
                itm->setData(Qt::UserRole,QVariant(id));
                itm->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
                itm->setForeground(QColor(128,128,128)); // GREY
                ui->qqScriptList->addItem(itm);
            }
        }
    }

    rebuildingRoutine=false;
}

int CQDlgScripts::getSelectedObjectID()
{
    QList<QListWidgetItem*> sel=ui->qqScriptList->selectedItems();
    if (sel.size()>0)
        return(sel.at(0)->data(Qt::UserRole).toInt());
    return(-1);
}

void CQDlgScripts::selectObjectInList(int objectID)
{
    for (int i=0;i<ui->qqScriptList->count();i++)
    {
        QListWidgetItem* it=ui->qqScriptList->item(i);
        if (it!=nullptr)
        {
            if (it->data(Qt::UserRole).toInt()==objectID)
            {
                inSelectionRoutine=true;
                it->setSelected(true);
                inSelectionRoutine=false;
                break;
            }
        }
    }
}

void CQDlgScripts::onDeletePressed()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if ( (focusWidget()==ui->qqScriptList)&&App::ct->simulation->isSimulationStopped() )
        {
            int scriptID=getSelectedObjectID();
            App::appendSimulationThreadCommand(DELETE_SCRIPT_SCRIPTGUITRIGGEREDCMD,scriptID);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}


void CQDlgScripts::on_qqAddNewScript_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CQDlgInsertScript theDialog(this);
        theDialog.initialize();
        if (theDialog.makeDialogModal()!=VDIALOG_MODAL_RETURN_CANCEL)
        {
            if (theDialog.scriptType==sim_scripttype_mainscript)
            {
                scriptViewMode=0;
                CLuaScriptObject* it=App::ct->luaScriptContainer->getMainScript();
                if (it!=nullptr)
                {
                    if (VMESSAGEBOX_REPLY_YES==App::uiThread->messageBox_warning(App::mainWindow,strTranslate(IDS_MAIN_SCRIPT),strTranslate(IDS_INFO_NO_MORE_THAN_ONE_MAIN_SCRIPT),VMESSAGEBOX_YES_NO))
                    {
                        App::appendSimulationThreadCommand(DELETE_SCRIPT_SCRIPTGUITRIGGEREDCMD,it->getScriptID());
                        App::appendSimulationThreadCommand(INSERT_SCRIPT_SCRIPTGUITRIGGEREDCMD,sim_scripttype_mainscript,0);
                    }
                }
                else
                    App::appendSimulationThreadCommand(INSERT_SCRIPT_SCRIPTGUITRIGGEREDCMD,sim_scripttype_mainscript,0);
            }
            if (theDialog.scriptType==sim_scripttype_childscript)
            {
                scriptViewMode=0;
                App::appendSimulationThreadCommand(INSERT_SCRIPT_SCRIPTGUITRIGGEREDCMD,sim_scripttype_childscript,0);
            }
            if (theDialog.scriptType==(sim_scripttype_childscript|sim_scripttype_threaded))
            {
                scriptViewMode=0;
                App::appendSimulationThreadCommand(INSERT_SCRIPT_SCRIPTGUITRIGGEREDCMD,sim_scripttype_childscript,1);
            }
            if (theDialog.scriptType==sim_scripttype_customizationscript)
            {
                scriptViewMode=1;
                App::appendSimulationThreadCommand(INSERT_SCRIPT_SCRIPTGUITRIGGEREDCMD,sim_scripttype_customizationscript,0);
            }

            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
//      selectObjectInList(newScriptID);
    }
}

void CQDlgScripts::on_qqScriptList_itemSelectionChanged()
{ // react only if the user clicked there!
    IF_UI_EVENT_CAN_READ_DATA
    {
        if ((!inSelectionRoutine)&&(!rebuildingRoutine))
            refresh();
    }
}

void CQDlgScripts::on_qqScriptList_itemDoubleClicked(QListWidgetItem *item)
{
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        if ( (item!=nullptr)&&App::ct->simulation->isSimulationStopped() )
        {
            CLuaScriptObject* it=App::ct->luaScriptContainer->getScriptFromID_alsoAddOnsAndSandbox(item->data(Qt::UserRole).toInt());
            if (it!=nullptr)
            {
                // Process the command via the simulation thread (delayed):
                SSimulationThreadCommand cmd;
                cmd.cmdId=OPEN_SCRIPT_EDITOR_CMD;
                cmd.intParams.push_back(it->getScriptID());
                App::appendSimulationThreadCommand(cmd);
            }
        }
    }
}

void CQDlgScripts::on_qqDisabled_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        int scriptID=getSelectedObjectID();
        App::appendSimulationThreadCommand(TOGGLE_DISABLED_SCRIPTGUITRIGGEREDCMD,scriptID);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_HIERARCHY_GUITRIGGEREDCMD);
    }
}

void CQDlgScripts::on_qqExecuteOnce_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        int scriptID=getSelectedObjectID();
        App::appendSimulationThreadCommand(TOGGLE_EXECUTEONCE_SCRIPTGUITRIGGEREDCMD,scriptID);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgScripts::on_qqAssociatedObjectCombo_currentIndexChanged(int index)
{
    if (!inMainRefreshRoutine)
    {
        IF_UI_EVENT_CAN_READ_DATA
        {
            int scriptID=getSelectedObjectID();
            int objID=ui->qqAssociatedObjectCombo->itemData(ui->qqAssociatedObjectCombo->currentIndex()).toInt();
            App::appendSimulationThreadCommand(SET_ASSOCIATEDOBJECT_SCRIPTGUITRIGGEREDCMD,scriptID,objID);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_HIERARCHY_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgScripts::on_qqExecutionOrder_currentIndexChanged(int index)
{
    if (!inMainRefreshRoutine)
    {
        IF_UI_EVENT_CAN_READ_DATA
        {
            int scriptID=getSelectedObjectID();
            int executionOrder=ui->qqExecutionOrder->itemData(ui->qqExecutionOrder->currentIndex()).toInt();
            App::appendSimulationThreadCommand(SET_EXECORDER_SCRIPTGUITRIGGEREDCMD,scriptID,executionOrder);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgScripts::on_qqCombo_currentIndexChanged(int index)
{
    if (!inMainRefreshRoutine)
    {
        IF_UI_EVENT_CAN_READ_DATA
        {
            scriptViewMode=ui->qqCombo->itemData(ui->qqCombo->currentIndex()).toInt();
            refresh();
        }
    }
}

void CQDlgScripts::on_qqDisableWithError_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        int scriptID=getSelectedObjectID();
        App::appendSimulationThreadCommand(TOGGLE_DISABLE_CUSTOM_SCRIPT_WITH_ERROR_SCRIPTGUITRIGGEREDCMD,scriptID);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_HIERARCHY_GUITRIGGEREDCMD);
    }
}

void CQDlgScripts::on_qqTreeTraversalDirection_currentIndexChanged(int index)
{
    if (!inMainRefreshRoutine)
    {
        IF_UI_EVENT_CAN_READ_DATA
        {
            int scriptID=getSelectedObjectID();
            int treeTraversalDirection=ui->qqTreeTraversalDirection->itemData(ui->qqTreeTraversalDirection->currentIndex()).toInt();
            App::appendSimulationThreadCommand(SET_TREETRAVERSALDIR_SCRIPTGUITRIGGEREDCMD,scriptID,treeTraversalDirection);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgScripts::on_qqDebugMode_currentIndexChanged(int index)
{
    if (!inMainRefreshRoutine)
    {
        IF_UI_EVENT_CAN_READ_DATA
        {
            int scriptID=getSelectedObjectID();
            int debugLevel=ui->qqDebugMode->itemData(ui->qqDebugMode->currentIndex()).toInt();
            App::appendSimulationThreadCommand(SET_DEBUGMODE_SCRIPTGUITRIGGEREDCMD,scriptID,debugLevel);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}
