#include <qdlgscripts.h>
#include <ui_qdlgscripts.h>
#include <qdlgmaterial.h>
#include <utils.h>
#include <simStrings.h>
#include <app.h>
#include <guiApp.h>

CQDlgScripts::CQDlgScripts(QWidget* parent)
    : CDlgEx(parent), ui(new Ui::CQDlgScripts)
{
    _dlgType = SCRIPT_DLG;
    ui->setupUi(this);
    inMainRefreshRoutine = false;
}

CQDlgScripts::~CQDlgScripts()
{
    delete ui;
}

void CQDlgScripts::cancelEvent()
{
    // we override this cancel event. The container window should close, not this one!!
    GuiApp::mainWindow->dlgCont->close(OBJECT_DLG);
}

void CQDlgScripts::refresh()
{
    inMainRefreshRoutine = true;
    QLineEdit* lineEditToSelect = getSelectedLineEdit();
    bool noEditModeNoSim = (GuiApp::getEditModeType() == NO_EDIT_MODE) && App::currentScene->simulation->isSimulationStopped();

    bool sel = App::currentScene->sceneObjects->isLastSelectionOfType(sim_sceneobject_script);
    bool bigSel = (App::currentScene->sceneObjects->getObjectCountInSelection(sim_sceneobject_script) > 1);
    CScript* it = App::currentScene->sceneObjects->getLastSelectionScript();

    ui->qqSize->setEnabled(sel && noEditModeNoSim);
    ui->qqColor->setEnabled(sel && noEditModeNoSim);
    ui->qqApplyMainProperties->setEnabled(bigSel && noEditModeNoSim);

    ui->qqExecutionOrder->clear();
    ui->qqExecutionOrder->setEnabled(sel && noEditModeNoSim);
    ui->qqEnabled->setEnabled(sel && noEditModeNoSim);
    ui->qqResetAfterSimError->setEnabled(sel && noEditModeNoSim && (it->detachedScript->getScriptType() == sim_scripttype_customization));
    ui->qqParentProxy->setVisible(sel && it->detachedScript->getParentIsProxy());
    ui->qqParentProxy->setEnabled(sel && noEditModeNoSim);

    if (sel)
    {
        std::string lang(it->detachedScript->getLang());
        if (((lang != "lua") && (lang != "python")) || (it->detachedScript->getScriptType() == sim_scripttype_passive))
        {
            ui->qqEnabled->setEnabled(false);
            ui->qqExecutionOrder->setEnabled(false);
        }
        lang = "Language: " + lang;
        ui->qqLang->setText(lang.c_str());
        ui->qqSize->setText(utils::getSizeString(false, it->getScriptSize()).c_str());

        ui->qqExecutionOrder->addItem(IDSN_FIRST, QVariant(sim_scriptexecorder_first));
        ui->qqExecutionOrder->addItem(IDSN_NORMAL, QVariant(sim_scriptexecorder_normal));
        ui->qqExecutionOrder->addItem(IDSN_LAST, QVariant(sim_scriptexecorder_last));
        ui->qqExecutionOrder->setCurrentIndex(it->detachedScript->getScriptExecPriority());

        ui->qqEnabled->setChecked(!it->detachedScript->getScriptIsDisabled());
        ui->qqResetAfterSimError->setChecked(it->getResetAfterSimError());
        ui->qqParentProxy->setChecked(it->detachedScript->getParentIsProxy());
    }
    else
    {
        ui->qqLang->setText("Language: -");
        ui->qqSize->setText("");

        ui->qqEnabled->setChecked(false);
        ui->qqResetAfterSimError->setChecked(false);
        ui->qqParentProxy->setChecked(false);
    }

    selectLineEdit(lineEditToSelect);
    inMainRefreshRoutine = false;
}

void CQDlgScripts::on_qqSize_editingFinished()
{
    if (!ui->qqSize->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        double newVal = GuiApp::getEvalDouble(ui->qqSize->text().toStdString().c_str(), &ok);
        App::appendSimulationThreadCommand(SET_SIZE_SCRIPTGUITRIGGEREDCMD,
                                           App::currentScene->sceneObjects->getLastSelectionHandle(), -1, newVal);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgScripts::on_qqColor_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CQDlgMaterial::displayMaterialDlg(COLOR_ID_SCRIPT, App::currentScene->sceneObjects->getLastSelectionHandle(), -1,
                                          GuiApp::mainWindow);
    }
}

void CQDlgScripts::on_qqApplyMainProperties_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CScript* it = App::currentScene->sceneObjects->getLastSelectionScript();
        if (it != nullptr)
        {
            SSimulationThreadCommand cmd;
            cmd.cmdId = APPLY_VISUALPROP_SCRIPTGUITRIGGEREDCMD;
            cmd.intParams.push_back(App::currentScene->sceneObjects->getLastSelectionHandle());
            for (size_t i = 0; i < App::currentScene->sceneObjects->getSelectionCount() - 1; i++)
                cmd.intParams.push_back(App::currentScene->sceneObjects->getObjectHandleFromSelectionIndex(i));
            App::appendSimulationThreadCommand(cmd);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgScripts::on_qqEnabled_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CScript* it = App::currentScene->sceneObjects->getLastSelectionScript();
        if (it != nullptr)
        {
            App::appendSimulationThreadCommand(TOGGLE_ENABLED_SCRIPTGUITRIGGEREDCMD, it->getObjectHandle());
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_HIERARCHY_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgScripts::on_qqParentProxy_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CScript* it = App::currentScene->sceneObjects->getLastSelectionScript();
        if (it != nullptr)
        {
            App::appendSimulationThreadCommand(PARENTPROXY_OFF_SCRIPTGUITRIGGEREDCMD, it->getObjectHandle());
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
            CScript* it = App::currentScene->sceneObjects->getLastSelectionScript();
            if (it != nullptr)
            {
                int executionOrder = ui->qqExecutionOrder->itemData(ui->qqExecutionOrder->currentIndex()).toInt();
                App::appendSimulationThreadCommand(SET_EXECORDER_SCRIPTGUITRIGGEREDCMD, it->getObjectHandle(), executionOrder);
                App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
                App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
            }
        }
    }
}

void CQDlgScripts::on_qqResetAfterSimError_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CScript* it = App::currentScene->sceneObjects->getLastSelectionScript();
        if (it != nullptr)
        {
            App::appendSimulationThreadCommand(TOGGLE_RESETAFTERSIMERROR_SCRIPTGUITRIGGEREDCMD, it->getObjectHandle());
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_HIERARCHY_GUITRIGGEREDCMD);
        }
    }
}
