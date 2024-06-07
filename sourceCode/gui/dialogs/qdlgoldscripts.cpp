#include <qdlgoldscripts.h>
#include <ui_qdlgoldscripts.h>
#include <tt.h>
#include <QShortcut>
#include <app.h>
#include <simStrings.h>
#include <jointObject.h>
#include <vMessageBox.h>
#include <guiApp.h>

int CQDlgOldScripts::scriptViewMode = 0;

CQDlgOldScripts::CQDlgOldScripts(QWidget *parent) : CDlgEx(parent), ui(new Ui::CQDlgOldScripts)
{
    _dlgType = OLD_LUA_SCRIPT_DLG;
    ui->setupUi(this);
    inSelectionRoutine = false;
    rebuildingRoutine = false;
    inMainRefreshRoutine = false;
    QShortcut *shortcut = new QShortcut(QKeySequence(Qt::Key_Delete), this);
    connect(shortcut, SIGNAL(activated()), this, SLOT(onDeletePressed()));
    QShortcut *shortcut2 = new QShortcut(QKeySequence(Qt::Key_Backspace), this);
    connect(shortcut2, SIGNAL(activated()), this, SLOT(onDeletePressed()));
}

CQDlgOldScripts::~CQDlgOldScripts()
{
    delete ui;
}

void CQDlgOldScripts::dialogCallbackFunc(const SUIThreadCommand *cmdIn, SUIThreadCommand *cmdOut)
{
    if ((cmdIn != nullptr) && (cmdIn->intParams[0] == _dlgType))
    {
        if (cmdIn->intParams[1] == 0)
            selectObjectInList(cmdIn->intParams[2]);
    }
}

void CQDlgOldScripts::refresh()
{
    inMainRefreshRoutine = true;
    bool noEditMode = (GuiApp::getEditModeType() == NO_EDIT_MODE);
    bool noEditModeAndNoSim = noEditMode && App::currentWorld->simulation->isSimulationStopped();

    ui->qqCombo->setEnabled(noEditMode);
    ui->qqCombo->clear();
    ui->qqCombo->addItem("Simulation scripts", QVariant(0));
    ui->qqCombo->addItem("Customization scripts", QVariant(1));

    ui->qqCombo->setCurrentIndex(scriptViewMode);

    int selectedObjectID = getSelectedObjectID();
    updateObjectsInList();
    selectObjectInList(selectedObjectID);

    ui->qqScriptList->setEnabled(noEditMode);

    ui->qqExecutionOrder->clear();

    CScriptObject *theScript = App::worldContainer->getScriptObjectFromHandle(getSelectedObjectID());
    CSceneObject *associatedObject = nullptr;
    if (theScript != nullptr)
        associatedObject = App::currentWorld->sceneObjects->getObjectFromHandle(
            App::currentWorld->sceneObjects->embeddedScriptContainer->getObjectHandleFromScriptHandle(theScript->getScriptHandle()));
    ui->qqExecutionOrder->setEnabled((associatedObject != nullptr) && noEditModeAndNoSim);
    ui->qqDisabled->setEnabled((theScript != nullptr) && noEditMode &&
                               ((theScript->getScriptType() == sim_scripttype_simulation) ||
                                (theScript->getScriptType() == sim_scripttype_customization)));
    ui->qqExecuteOnce->setEnabled((theScript != nullptr) && noEditModeAndNoSim &&
                                  (theScript->getScriptType() == sim_scripttype_simulation) &&
                                  theScript->getThreadedExecution_oldThreads());
    ui->qqExecuteOnce->setVisible(App::userSettings->keepOldThreadedScripts);

    if (theScript != nullptr)
    {
        if (associatedObject != nullptr)
        {
            ui->qqExecutionOrder->addItem(IDSN_FIRST, QVariant(sim_scriptexecorder_first));
            ui->qqExecutionOrder->addItem(IDSN_NORMAL, QVariant(sim_scriptexecorder_normal));
            ui->qqExecutionOrder->addItem(IDSN_LAST, QVariant(sim_scriptexecorder_last));
            ui->qqExecutionOrder->setCurrentIndex(associatedObject->getScriptExecPriority());
        }

        ui->qqDisabled->setChecked(theScript->getScriptIsDisabled());

        ui->qqExecuteOnce->setChecked(theScript->getExecuteJustOnce_oldThreads());
    }
    else
    {
        ui->qqDisabled->setChecked(false);
        ui->qqExecuteOnce->setChecked(false);
    }
    inMainRefreshRoutine = false;
}

void CQDlgOldScripts::updateObjectsInList()
{
    rebuildingRoutine = true;
    ui->qqScriptList->clear();

    if (scriptViewMode == 0)
    { // Main and simulation scripts
        CScriptObject *it = App::currentWorld->sceneObjects->embeddedScriptContainer->getMainScript();
        if (it != nullptr)
        {
            std::string tmp = it->getDescriptiveName();
            int id = it->getScriptHandle();
            QListWidgetItem *itm = new QListWidgetItem(tmp.c_str());
            itm->setData(Qt::UserRole, QVariant(id));
            itm->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            itm->setForeground(QColor(255, 128, 128)); // RED
            ui->qqScriptList->addItem(itm);
        }
        for (int i = 0; i < int(App::currentWorld->sceneObjects->embeddedScriptContainer->allScripts.size()); i++)
        {
            it = App::currentWorld->sceneObjects->embeddedScriptContainer->allScripts[i];
            int t = it->getScriptType();
            if (t == sim_scripttype_simulation)
            {
                std::string tmp = it->getDescriptiveName();
                int id = it->getScriptHandle();
                QListWidgetItem *itm = new QListWidgetItem(tmp.c_str());
                itm->setData(Qt::UserRole, QVariant(id));
                itm->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
                if (it->getThreadedExecution_oldThreads())
                    itm->setForeground(QColor(128, 205, 205)); // CYAN
                else
                    itm->setForeground(QColor(128, 128, 128)); // GREY
                ui->qqScriptList->addItem(itm);
            }
        }
    }

    if (scriptViewMode == 1)
    { // Customization scripts
        for (int i = 0; i < int(App::currentWorld->sceneObjects->embeddedScriptContainer->allScripts.size()); i++)
        {
            CScriptObject *it = App::currentWorld->sceneObjects->embeddedScriptContainer->allScripts[i];
            int t = it->getScriptType();
            if (t == sim_scripttype_customization)
            {
                std::string tmp = it->getDescriptiveName();
                int id = it->getScriptHandle();
                QListWidgetItem *itm = new QListWidgetItem(tmp.c_str());
                itm->setData(Qt::UserRole, QVariant(id));
                itm->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
                itm->setForeground(QColor(128, 128, 128)); // GREY
                ui->qqScriptList->addItem(itm);
            }
        }
    }

    rebuildingRoutine = false;
}

int CQDlgOldScripts::getSelectedObjectID()
{
    QList<QListWidgetItem *> sel = ui->qqScriptList->selectedItems();
    if (sel.size() > 0)
        return (sel.at(0)->data(Qt::UserRole).toInt());
    return (-1);
}

void CQDlgOldScripts::selectObjectInList(int objectID)
{
    for (int i = 0; i < ui->qqScriptList->count(); i++)
    {
        QListWidgetItem *it = ui->qqScriptList->item(i);
        if (it != nullptr)
        {
            if (it->data(Qt::UserRole).toInt() == objectID)
            {
                inSelectionRoutine = true;
                it->setSelected(true);
                inSelectionRoutine = false;
                break;
            }
        }
    }
}

void CQDlgOldScripts::onDeletePressed()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if ((focusWidget() == ui->qqScriptList) && App::currentWorld->simulation->isSimulationStopped())
        {
            int scriptID = getSelectedObjectID();
            App::appendSimulationThreadCommand(DELETE_SCRIPT_SCRIPTGUITRIGGEREDCMD, scriptID);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgOldScripts::on_qqScriptList_itemSelectionChanged()
{ // react only if the user clicked there!
    IF_UI_EVENT_CAN_READ_DATA
    {
        if ((!inSelectionRoutine) && (!rebuildingRoutine))
            refresh();
    }
}

void CQDlgOldScripts::on_qqScriptList_itemDoubleClicked(QListWidgetItem *item)
{
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        if ((item != nullptr) && App::currentWorld->simulation->isSimulationStopped())
        {
            CScriptObject *it = App::worldContainer->getScriptObjectFromHandle(item->data(Qt::UserRole).toInt());
            if (it != nullptr)
            {
                // Process the command via the simulation thread (delayed):
                SSimulationThreadCommand cmd;
                cmd.cmdId = OPEN_SCRIPT_EDITOR_CMD;
                cmd.intParams.push_back(it->getScriptHandle());
                App::appendSimulationThreadCommand(cmd);
            }
        }
    }
}

void CQDlgOldScripts::on_qqDisabled_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        int scriptID = getSelectedObjectID();
        App::appendSimulationThreadCommand(TOGGLE_ENABLED_SCRIPTGUITRIGGEREDCMD, scriptID);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_HIERARCHY_GUITRIGGEREDCMD);
    }
}

void CQDlgOldScripts::on_qqExecuteOnce_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        int scriptID = getSelectedObjectID();
        App::appendSimulationThreadCommand(TOGGLE_EXECUTEONCE_SCRIPTGUITRIGGEREDCMD, scriptID);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgOldScripts::on_qqExecutionOrder_currentIndexChanged(int index)
{
    if (!inMainRefreshRoutine)
    {
        IF_UI_EVENT_CAN_READ_DATA
        {
            int scriptID = getSelectedObjectID();
            int objectId = App::currentWorld->sceneObjects->embeddedScriptContainer->getObjectHandleFromScriptHandle(scriptID);
            if (objectId != -1)
            {
                int executionOrder = ui->qqExecutionOrder->itemData(ui->qqExecutionOrder->currentIndex()).toInt();
                App::appendSimulationThreadCommand(SET_EXECORDER_SCRIPTGUITRIGGEREDCMD, objectId, executionOrder);
                App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
                App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
            }
        }
    }
}

void CQDlgOldScripts::on_qqCombo_currentIndexChanged(int index)
{
    if (!inMainRefreshRoutine)
    {
        IF_UI_EVENT_CAN_READ_DATA
        {
            scriptViewMode = ui->qqCombo->itemData(ui->qqCombo->currentIndex()).toInt();
            refresh();
        }
    }
}
