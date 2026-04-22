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
    _dlgType = MARKER_DLG;
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

    if (sel)
    {
    }
    else
    {
    }

    selectLineEdit(lineEditToSelect);
    inMainRefreshRoutine = false;
}
