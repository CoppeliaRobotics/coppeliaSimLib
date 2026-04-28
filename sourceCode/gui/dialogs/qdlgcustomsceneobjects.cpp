#include <qdlgcustomsceneobjects.h>
#include <ui_qdlgcustomsceneobjects.h>
#include <qdlgmaterial.h>
#include <utils.h>
#include <simStrings.h>
#include <app.h>
#include <guiApp.h>

CQDlgCustomSceneObjects::CQDlgCustomSceneObjects(QWidget* parent)
    : CDlgEx(parent), ui(new Ui::CQDlgCustomSceneObjects)
{
    _dlgType = CUSTOMSCENEOBJECT_DLG;
    ui->setupUi(this);
    inMainRefreshRoutine = false;
}

CQDlgCustomSceneObjects::~CQDlgCustomSceneObjects()
{
    delete ui;
}

void CQDlgCustomSceneObjects::cancelEvent()
{
    // we override this cancel event. The container window should close, not this one!!
    GuiApp::mainWindow->dlgCont->close(OBJECT_DLG);
}

void CQDlgCustomSceneObjects::refresh()
{
    inMainRefreshRoutine = true;
    QLineEdit* lineEditToSelect = getSelectedLineEdit();
    bool noEditModeNoSim = (GuiApp::getEditModeType() == NO_EDIT_MODE) && App::scene->simulation->isSimulationStopped();

    bool sel = App::scene->sceneObjects->isLastSelectionOfType(sim_sceneobject_customsceneobject);
    bool bigSel = (App::scene->sceneObjects->getObjectCountInSelection(sim_sceneobject_customsceneobject) > 1);
    CCustomSceneObject* it = App::scene->sceneObjects->getLastSelectionCustomSceneObject();

    ui->qqSize->setEnabled(sel && noEditModeNoSim);
    ui->qqColor->setEnabled(sel && noEditModeNoSim);
    ui->qqApplyMainProperties->setEnabled(bigSel && noEditModeNoSim);

    if (sel)
        ui->qqSize->setText(utils::getSizeString(false, it->getObjectSize()).c_str());
    else
        ui->qqSize->setText("");

    selectLineEdit(lineEditToSelect);
    inMainRefreshRoutine = false;
}

void CQDlgCustomSceneObjects::on_qqSize_editingFinished()
{
    if (!ui->qqSize->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        double newVal = GuiApp::getEvalDouble(ui->qqSize->text().toStdString().c_str(), &ok);
        App::appendSimulationThreadCommand(SET_SIZE_CUSTOMSCENEOBJECTGUITRIGGEREDCMD, App::scene->sceneObjects->getLastSelectionHandle(), -1, newVal);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgCustomSceneObjects::on_qqColor_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CQDlgMaterial::displayMaterialDlg(COLOR_ID_CUSTOM_SCENE_OBJECT, App::scene->sceneObjects->getLastSelectionHandle(), -1, GuiApp::mainWindow);
    }
}

void CQDlgCustomSceneObjects::on_qqApplyMainProperties_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CCustomSceneObject* it = App::scene->sceneObjects->getLastSelectionCustomSceneObject();
        if (it != nullptr)
        {
            SSimulationThreadCommand cmd;
            cmd.cmdId = APPLY_VISUALPROP_CUSTOMSCENEOBJECTGUITRIGGEREDCMD;
            cmd.intParams.push_back(App::scene->sceneObjects->getLastSelectionHandle());
            for (size_t i = 0; i < App::scene->sceneObjects->getSelectionCount() - 1; i++)
                cmd.intParams.push_back(App::scene->sceneObjects->getObjectHandleFromSelectionIndex(i));
            App::appendSimulationThreadCommand(cmd);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}
