
#include "vrepMainHeader.h"
#include "qdlgmultishapeedition.h"
#include "ui_qdlgmultishapeedition.h"
#include "tt.h"
#include "gV.h"
#include "geometric.h"
#include "qdlgtextures.h"
#include "app.h"
#include "v_repStrings.h"
#include "qdlgmaterial.h"

CGeometric* theGeom=nullptr;

CQDlgMultishapeEdition::CQDlgMultishapeEdition(QWidget *parent) :
    CDlgEx(parent),
    ui(new Ui::CQDlgMultishapeEdition)
{
    _dlgType=MULTISHAPE_EDITION_DLG;
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
    cmd.cmdId=ANY_EDIT_MODE_FINISH_AND_APPLY_CHANGES_EMCMD;
    App::appendSimulationThreadCommand(cmd);
}

void CQDlgMultishapeEdition::refresh()
{
    QLineEdit* lineEditToSelect=getSelectedLineEdit();
    theGeom=nullptr;
    if ( App::mainWindow->editModeContainer->getMultishapeEditMode()->isCurrentMultishapeGeometricComponentValid() )
        theGeom=App::mainWindow->editModeContainer->getMultishapeEditMode()->getCurrentMultishapeGeometricComponent();

    ui->qqShadingAngle->setEnabled(theGeom!=nullptr);
    ui->qqEdgesAngle->setEnabled(theGeom!=nullptr);
    ui->qqBackfaceCulling->setEnabled(theGeom!=nullptr);
    ui->qqWireframe->setEnabled(theGeom!=nullptr);
    ui->qqShowEdges->setEnabled(theGeom!=nullptr);
    ui->qqHiddenBorder->setEnabled(theGeom!=nullptr);
    ui->qqAdjustColor->setEnabled(theGeom!=nullptr);
    ui->qqTexture->setEnabled(theGeom!=nullptr);

    if (theGeom!=nullptr)
    {
        ui->qqShadingAngle->setText(tt::getAngleFString(false,theGeom->getGouraudShadingAngle(),1).c_str());
        ui->qqEdgesAngle->setText(tt::getAngleFString(false,theGeom->getEdgeThresholdAngle(),1).c_str());
        ui->qqBackfaceCulling->setChecked(theGeom->getCulling());
        ui->qqWireframe->setChecked(theGeom->getWireframe());
        ui->qqShowEdges->setChecked(theGeom->getVisibleEdges());
        ui->qqHiddenBorder->setChecked(theGeom->getHideEdgeBorders());
    }
    else
    {
        ui->qqShadingAngle->setText("");
        ui->qqEdgesAngle->setText("");
        ui->qqBackfaceCulling->setChecked(false);
        ui->qqWireframe->setChecked(false);
        ui->qqShowEdges->setChecked(false);
        ui->qqHiddenBorder->setChecked(false);
    }
    selectLineEdit(lineEditToSelect);
}

void CQDlgMultishapeEdition::on_qqBackfaceCulling_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (theGeom==nullptr)
            return;
        CShape* shape=App::mainWindow->editModeContainer->getMultishapeEditMode()->getEditModeMultishape();
        int index=App::mainWindow->editModeContainer->getMultishapeEditMode()->getMultishapeGeometricComponentIndex();
        if (shape!=nullptr)
        {
            App::appendSimulationThreadCommand(TOGGLE_BACKFACECULLING_MULTISHAPEEDITIONGUITRIGGEREDCMD,shape->getObjectHandle(),index);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgMultishapeEdition::on_qqWireframe_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (theGeom==nullptr)
            return;
        CShape* shape=App::mainWindow->editModeContainer->getMultishapeEditMode()->getEditModeMultishape();
        int index=App::mainWindow->editModeContainer->getMultishapeEditMode()->getMultishapeGeometricComponentIndex();
        if (shape!=nullptr)
        {
            App::appendSimulationThreadCommand(TOGGLE_WIREFRAME_MULTISHAPEEDITIONGUITRIGGEREDCMD,shape->getObjectHandle(),index);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgMultishapeEdition::on_qqShowEdges_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (theGeom==nullptr)
            return;
        CShape* shape=App::mainWindow->editModeContainer->getMultishapeEditMode()->getEditModeMultishape();
        int index=App::mainWindow->editModeContainer->getMultishapeEditMode()->getMultishapeGeometricComponentIndex();
        if (shape!=nullptr)
        {
            App::appendSimulationThreadCommand(TOGGLE_SHOWEDGES_MULTISHAPEEDITIONGUITRIGGEREDCMD,shape->getObjectHandle(),index);
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
        if (theGeom==nullptr)
            return;
        bool ok;
        float newVal=ui->qqShadingAngle->text().toFloat(&ok);
        CShape* shape=App::mainWindow->editModeContainer->getMultishapeEditMode()->getEditModeMultishape();
        int index=App::mainWindow->editModeContainer->getMultishapeEditMode()->getMultishapeGeometricComponentIndex();
        if ((shape!=nullptr)&&ok)
        {
            App::appendSimulationThreadCommand(SET_SHADINGANGLE_MULTISHAPEEDITIONGUITRIGGEREDCMD,shape->getObjectHandle(),index,gv::userToRad*newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgMultishapeEdition::on_qqAdjustColor_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (theGeom==nullptr)
            return;
        CQDlgMaterial::displayMaterialDlg(COLOR_ID_SHAPE_GEOMETRY,App::mainWindow->editModeContainer->getEditModeObjectID(),App::mainWindow->editModeContainer->getMultishapeEditMode()->getMultishapeGeometricComponentIndex(),App::mainWindow);
    }
}

void CQDlgMultishapeEdition::on_qqTexture_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (theGeom==nullptr)
            return;
        CQDlgTextures::displayDlg(TEXTURE_ID_COMPOUND_SHAPE,App::mainWindow->editModeContainer->getEditModeObjectID(),App::mainWindow->editModeContainer->getMultishapeEditMode()->getMultishapeGeometricComponentIndex(),App::mainWindow);
    }
}

void CQDlgMultishapeEdition::on_qqEdgesAngle_editingFinished()
{
    if (!ui->qqEdgesAngle->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (theGeom==nullptr)
            return;
        bool ok;
        float newVal=ui->qqEdgesAngle->text().toFloat(&ok);
        CShape* shape=App::mainWindow->editModeContainer->getMultishapeEditMode()->getEditModeMultishape();
        int index=App::mainWindow->editModeContainer->getMultishapeEditMode()->getMultishapeGeometricComponentIndex();
        if ((shape!=nullptr)&&ok)
        {
            App::appendSimulationThreadCommand(SET_EDGEANGLE_MULTISHAPEEDITIONGUITRIGGEREDCMD,shape->getObjectHandle(),index,gv::userToRad*newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgMultishapeEdition::on_qqHiddenBorder_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (theGeom==nullptr)
            return;
        CShape* shape=App::mainWindow->editModeContainer->getMultishapeEditMode()->getEditModeMultishape();
        int index=App::mainWindow->editModeContainer->getMultishapeEditMode()->getMultishapeGeometricComponentIndex();
        if (shape!=nullptr)
        {
            App::appendSimulationThreadCommand(TOGGLE_HIDDENBORDER_MULTISHAPEEDITIONGUITRIGGEREDCMD,shape->getObjectHandle(),index);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}
