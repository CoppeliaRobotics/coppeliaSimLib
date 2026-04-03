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
    bool noEditModeNoSim = (GuiApp::getEditModeType() == NO_EDIT_MODE) && App::currentScene->simulation->isSimulationStopped();

    bool sel = App::currentScene->sceneObjects->isLastSelectionOfType(sim_sceneobject_marker);
    bool bigSel = (App::currentScene->sceneObjects->getObjectCountInSelection(sim_sceneobject_marker) > 1);
    CMarker* it = App::currentScene->sceneObjects->getLastSelectionMarker();

    if (sel)
    {
    }
    else
    {
    }

    selectLineEdit(lineEditToSelect);
    inMainRefreshRoutine = false;
}
