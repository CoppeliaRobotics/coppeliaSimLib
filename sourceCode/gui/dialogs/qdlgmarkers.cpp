#include <qdlgmarkers.h>
#include <ui_qdlgmarkers.h>
#include <qdlgmaterial.h>
#include <utils.h>
#include <simStrings.h>
#include <app.h>
#include <guiApp.h>

CQDlgMarkers::CQDlgMarkers(QWidget* parent)
    : CDlgEx(parent), ui(new Ui::CQDlgMarkers)
{
    _dlgType = MARKER_DLG;
    ui->setupUi(this);
    inMainRefreshRoutine = false;
}

CQDlgMarkers::~CQDlgMarkers()
{
    delete ui;
}

void CQDlgMarkers::cancelEvent()
{
    // we override this cancel event. The container window should close, not this one!!
    GuiApp::mainWindow->dlgCont->close(OBJECT_DLG);
}

void CQDlgMarkers::refresh()
{
    inMainRefreshRoutine = true;
    QLineEdit* lineEditToSelect = getSelectedLineEdit();
    bool noEditModeNoSim = (GuiApp::getEditModeType() == NO_EDIT_MODE) && App::currentWorld->simulation->isSimulationStopped();

    bool sel = App::currentWorld->sceneObjects->isLastSelectionOfType(sim_sceneobject_marker);
    bool bigSel = (App::currentWorld->sceneObjects->getObjectCountInSelection(sim_sceneobject_marker) > 1);
    CMarker* it = App::currentWorld->sceneObjects->getLastSelectionMarker();

    ui->qqSize->setEnabled(sel && noEditModeNoSim);
    ui->qqColor->setEnabled(sel && noEditModeNoSim);
    ui->qqApplyMainProperties->setEnabled(bigSel && noEditModeNoSim);

    if (sel)
    {
        ui->qqSize->setText(utils::getSizeString(false, it->getMarkerSize()).c_str());
    }
    else
    {
        ui->qqSize->setText("");
    }

    selectLineEdit(lineEditToSelect);
    inMainRefreshRoutine = false;
}

void CQDlgMarkers::on_qqSize_editingFinished()
{
    if (!ui->qqSize->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        double newVal = GuiApp::getEvalDouble(ui->qqSize->text().toStdString().c_str(), &ok);
        App::appendSimulationThreadCommand(SET_SIZE_MARKERGUITRIGGEREDCMD, App::currentWorld->sceneObjects->getLastSelectionHandle(), -1, newVal);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgMarkers::on_qqColor_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CQDlgMaterial::displayMaterialDlg(COLOR_ID_MARKER, App::currentWorld->sceneObjects->getLastSelectionHandle(), -1, GuiApp::mainWindow);
    }
}

void CQDlgMarkers::on_qqApplyMainProperties_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CMarker* it = App::currentWorld->sceneObjects->getLastSelectionMarker();
        if (it != nullptr)
        {
            SSimulationThreadCommand cmd;
            cmd.cmdId = APPLY_VISUALPROP_MARKERGUITRIGGEREDCMD;
            cmd.intParams.push_back(App::currentWorld->sceneObjects->getLastSelectionHandle());
            for (size_t i = 0; i < App::currentWorld->sceneObjects->getSelectionCount() - 1; i++)
                cmd.intParams.push_back(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
            App::appendSimulationThreadCommand(cmd);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}
