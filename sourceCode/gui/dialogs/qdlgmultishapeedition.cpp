#include <qdlgmultishapeedition.h>
#include <ui_qdlgmultishapeedition.h>
#include <tt.h>
#include <utils.h>
#include <mesh.h>
#include <qdlgtextures.h>
#include <app.h>
#include <simStrings.h>
#include <qdlgmaterial.h>
#include <guiApp.h>

CMesh* theGeom = nullptr;

CQDlgMultishapeEdition::CQDlgMultishapeEdition(QWidget* parent)
    : CDlgEx(parent), ui(new Ui::CQDlgMultishapeEdition)
{
    _dlgType = MULTISHAPE_EDITION_DLG;
    ui->setupUi(this);
}

CQDlgMultishapeEdition::~CQDlgMultishapeEdition()
{
    delete ui;
}

void CQDlgMultishapeEdition::cancelEvent()
{
    // we override this cancel event.
    SSimulationThreadCommand cmd;
    cmd.cmdId = ANY_EDIT_MODE_FINISH_AND_APPLY_CHANGES_EMCMD;
    App::appendSimulationThreadCommand(cmd);
}

void CQDlgMultishapeEdition::refresh()
{
    QLineEdit* lineEditToSelect = getSelectedLineEdit();
    theGeom = nullptr;
    if (GuiApp::mainWindow->editModeContainer->getMultishapeEditMode()->isCurrentMultishapeGeometricComponentValid())
        theGeom =
            GuiApp::mainWindow->editModeContainer->getMultishapeEditMode()->getCurrentMultishapeGeometricComponent();

    ui->qqShadingAngle->setEnabled(theGeom != nullptr);
    ui->qqBackfaceCulling->setEnabled(theGeom != nullptr);
    ui->qqShowEdges->setEnabled(theGeom != nullptr);
    ui->qqAdjustColor->setEnabled(theGeom != nullptr);
    ui->qqTexture->setEnabled(theGeom != nullptr);

    if (theGeom != nullptr)
    {
        ui->qqShadingAngle->setText(utils::getAngleString(false, theGeom->getShadingAngle()).c_str());
        ui->qqBackfaceCulling->setChecked(theGeom->getCulling());
        ui->qqShowEdges->setChecked(theGeom->getVisibleEdges());
    }
    else
    {
        ui->qqShadingAngle->setText("");
        ui->qqBackfaceCulling->setChecked(false);
        ui->qqShowEdges->setChecked(false);
    }
    selectLineEdit(lineEditToSelect);
}

void CQDlgMultishapeEdition::on_qqBackfaceCulling_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (theGeom == nullptr)
            return;
        CShape* shape = GuiApp::mainWindow->editModeContainer->getMultishapeEditMode()->getEditModeMultishape();
        int index =
            GuiApp::mainWindow->editModeContainer->getMultishapeEditMode()->getMultishapeGeometricComponentIndex();
        if (shape != nullptr)
        {
            App::appendSimulationThreadCommand(TOGGLE_BACKFACECULLING_MULTISHAPEEDITIONGUITRIGGEREDCMD,
                                               shape->getObjectHandle(), index);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgMultishapeEdition::on_qqShowEdges_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (theGeom == nullptr)
            return;
        CShape* shape = GuiApp::mainWindow->editModeContainer->getMultishapeEditMode()->getEditModeMultishape();
        int index =
            GuiApp::mainWindow->editModeContainer->getMultishapeEditMode()->getMultishapeGeometricComponentIndex();
        if (shape != nullptr)
        {
            App::appendSimulationThreadCommand(TOGGLE_SHOWEDGES_MULTISHAPEEDITIONGUITRIGGEREDCMD,
                                               shape->getObjectHandle(), index);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgMultishapeEdition::on_qqShadingAngle_editingFinished()
{
    if (!ui->qqShadingAngle->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (theGeom == nullptr)
            return;
        bool ok;
        double newVal = GuiApp::getEvalDouble(ui->qqShadingAngle->text().toStdString().c_str(), &ok);
        CShape* shape = GuiApp::mainWindow->editModeContainer->getMultishapeEditMode()->getEditModeMultishape();
        int index =
            GuiApp::mainWindow->editModeContainer->getMultishapeEditMode()->getMultishapeGeometricComponentIndex();
        if ((shape != nullptr) && ok)
        {
            App::appendSimulationThreadCommand(SET_SHADINGANGLE_MULTISHAPEEDITIONGUITRIGGEREDCMD,
                                               shape->getObjectHandle(), index, degToRad * newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgMultishapeEdition::on_qqAdjustColor_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (theGeom == nullptr)
            return;
        CQDlgMaterial::displayMaterialDlg(
            COLOR_ID_SHAPE_GEOMETRY, GuiApp::mainWindow->editModeContainer->getEditModeObjectID(),
            GuiApp::mainWindow->editModeContainer->getMultishapeEditMode()->getMultishapeGeometricComponentIndex(),
            GuiApp::mainWindow);
    }
}

void CQDlgMultishapeEdition::on_qqTexture_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (theGeom == nullptr)
            return;
        CQDlgTextures::displayDlg(
            TEXTURE_ID_COMPOUND_SHAPE, GuiApp::mainWindow->editModeContainer->getEditModeObjectID(),
            GuiApp::mainWindow->editModeContainer->getMultishapeEditMode()->getMultishapeGeometricComponentIndex(),
            GuiApp::mainWindow);
    }
}
