
#include "vrepMainHeader.h"
#include "qdlglights.h"
#include "ui_qdlglights.h"
#include "tt.h"
#include "gV.h"
#include "qdlgmaterial.h"
#include "qdlglightmaterial.h"
#include "app.h"

CQDlgLights::CQDlgLights(QWidget *parent) :
    CDlgEx(parent),
    ui(new Ui::CQDlgLights)
{
    _dlgType=LIGHT_DLG;
    ui->setupUi(this);
}

CQDlgLights::~CQDlgLights()
{
    delete ui;
}

void CQDlgLights::cancelEvent()
{
    // we override this cancel event. The container window should close, not this one!!
    App::mainWindow->dlgCont->close(OBJECT_DLG);
}

void CQDlgLights::refresh()
{
    QLineEdit* lineEditToSelect=getSelectedLineEdit();
    bool noEditModeNoSim=(App::getEditModeType()==NO_EDIT_MODE)&&App::ct->simulation->isSimulationStopped();

    CLight* it=App::ct->objCont->getLastSelection_light();

    ui->qqEnabled->setEnabled((it!=nullptr)&&noEditModeNoSim);
    ui->qqLocal->setEnabled((it!=nullptr)&&noEditModeNoSim);
    ui->qqSize->setEnabled((it!=nullptr)&&noEditModeNoSim);
    ui->qqLightColor->setEnabled((it!=nullptr)&&noEditModeNoSim);

    if (it!=nullptr)
    {
        int lt=it->getLightType();
        ui->qqEnabled->setChecked(it->getLightActive()&&noEditModeNoSim);
        ui->qqLocal->setChecked(it->getLightIsLocal());
        ui->qqCasingColor->setEnabled((lt!=sim_light_omnidirectional_subtype)&&noEditModeNoSim);

        ui->qqSize->setText(tt::getFString(false,it->getLightSize(),3).c_str());

        ui->qqConstantFactor->setEnabled((lt!=sim_light_directional_subtype)&&noEditModeNoSim);
        ui->qqLinearFactor->setEnabled((lt!=sim_light_directional_subtype)&&noEditModeNoSim);
        ui->qqQuadraticFactor->setEnabled((lt!=sim_light_directional_subtype)&&noEditModeNoSim);
        ui->qqSpotExponent->setEnabled((lt==sim_light_spot_subtype)&&noEditModeNoSim);
        ui->qqSpotCutoff->setEnabled((lt==sim_light_spot_subtype)&&noEditModeNoSim);

        if (lt==sim_light_spot_subtype)
        {
            ui->qqSpotExponent->setText(tt::getIString(false,it->getSpotExponent()).c_str());
            ui->qqSpotCutoff->setText(tt::getAngleFString(false,it->getSpotCutoffAngle(),2).c_str());
        }
        else
        {
            ui->qqSpotExponent->setText("");
            ui->qqSpotCutoff->setText("");
        }
        if (lt!=sim_light_directional_subtype)
        {
            ui->qqConstantFactor->setText(tt::getFString(false,it->getAttenuationFactor(CONSTANT_ATTENUATION),3).c_str());
            ui->qqLinearFactor->setText(tt::getFString(false,it->getAttenuationFactor(LINEAR_ATTENUATION),3).c_str());
            ui->qqQuadraticFactor->setText(tt::getFString(false,it->getAttenuationFactor(QUADRATIC_ATTENUATION),3).c_str());
        }
        else
        {
            ui->qqConstantFactor->setText("");
            ui->qqLinearFactor->setText("");
            ui->qqQuadraticFactor->setText("");
        }
    }
    else
    {
        ui->qqConstantFactor->setEnabled(false);
        ui->qqLinearFactor->setEnabled(false);
        ui->qqQuadraticFactor->setEnabled(false);
        ui->qqSpotCutoff->setEnabled(false);
        ui->qqSpotExponent->setEnabled(false);
        ui->qqCasingColor->setEnabled(false);
        ui->qqEnabled->setChecked(false);
        ui->qqLocal->setChecked(false);
        ui->qqSize->setText("");
        ui->qqConstantFactor->setText("");
        ui->qqLinearFactor->setText("");
        ui->qqQuadraticFactor->setText("");
        ui->qqSpotCutoff->setText("");
        ui->qqSpotExponent->setText("");
    }
    selectLineEdit(lineEditToSelect);
}

void CQDlgLights::on_qqEnabled_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_ENABLED_LIGHTGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgLights::on_qqLocal_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_LIGHTISLOCAL_LIGHTGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgLights::on_qqSize_editingFinished()
{
    if (!ui->qqSize->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        float newVal=ui->qqSize->text().toFloat(&ok);
        if (ok)
        {
            App::appendSimulationThreadCommand(SET_SIZE_LIGHTGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),-1,newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgLights::on_qqSpotCutoff_editingFinished()
{
    if (!ui->qqSpotCutoff->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        float newVal=ui->qqSpotCutoff->text().toFloat(&ok);
        if (ok)
        {
            App::appendSimulationThreadCommand(SET_SPOTCUTOFF_LIGHTGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),-1,newVal*gv::userToRad);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgLights::on_qqSpotExponent_editingFinished()
{
    if (!ui->qqSpotExponent->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        int newVal=ui->qqSpotExponent->text().toInt(&ok);
        if (ok)
        {
            App::appendSimulationThreadCommand(SET_SPOTEXPONENT_LIGHTGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgLights::on_qqConstantFactor_editingFinished()
{
    if (!ui->qqConstantFactor->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        float newVal=ui->qqConstantFactor->text().toFloat(&ok);
        if (ok)
        {
            App::appendSimulationThreadCommand(SET_CONSTATTENUATION_LIGHTGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),-1,newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgLights::on_qqLinearFactor_editingFinished()
{
    if (!ui->qqLinearFactor->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        float newVal=ui->qqLinearFactor->text().toFloat(&ok);
        if (ok)
        {
            App::appendSimulationThreadCommand(SET_LINATTENUATION_LIGHTGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),-1,newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgLights::on_qqQuadraticFactor_editingFinished()
{
    if (!ui->qqQuadraticFactor->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        float newVal=ui->qqQuadraticFactor->text().toFloat(&ok);
        if (ok)
        {
            App::appendSimulationThreadCommand(SET_QUADATTENUATION_LIGHTGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),-1,newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgLights::on_qqLightColor_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CQDlgLightMaterial::displayMaterialDlg(COLOR_ID_LIGHT_LIGHT,App::ct->objCont->getLastSelectionID(),-1,App::mainWindow);
    }
}

void CQDlgLights::on_qqCasingColor_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CQDlgMaterial::displayMaterialDlg(COLOR_ID_LIGHT_CASING,App::ct->objCont->getLastSelectionID(),-1,App::mainWindow);
    }
}
