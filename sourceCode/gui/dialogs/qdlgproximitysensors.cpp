#include <qdlgproximitysensors.h>
#include <ui_qdlgproximitysensors.h>
#include <tt.h>
#include <utils.h>
#include <qdlgmaterial.h>
#include <qdlgproxsensdetectionparam.h>
#include <qdlgdetectionvolume.h>
#include <simStrings.h>
#include <app.h>
#include <vMessageBox.h>
#include <guiApp.h>

CQDlgProximitySensors::CQDlgProximitySensors(QWidget* parent)
    : CDlgEx(parent), ui(new Ui::CQDlgProximitySensors)
{
    _dlgType = PROXIMITY_SENSOR_DLG;
    ui->setupUi(this);
    inMainRefreshRoutine = false;
}

CQDlgProximitySensors::~CQDlgProximitySensors()
{
    delete ui;
}

void CQDlgProximitySensors::cancelEvent()
{
    // we override this cancel event. The container window should close, not this one!!
    GuiApp::mainWindow->dlgCont->close(OBJECT_DLG);
}

void CQDlgProximitySensors::refresh()
{
    inMainRefreshRoutine = true;
    QLineEdit* lineEditToSelect = getSelectedLineEdit();

    CProxSensor* it = App::currentWorld->sceneObjects->getLastSelectionProxSensor();

    bool isSensor = App::currentWorld->sceneObjects->isLastSelectionOfType(sim_sceneobject_proximitysensor);
    bool manySensors = App::currentWorld->sceneObjects->getObjectCountInSelection(sim_sceneobject_proximitysensor) > 1;
    bool noEditMode = GuiApp::getEditModeType() == NO_EDIT_MODE;
    bool noEditModeAndNoSim = noEditMode && App::currentWorld->simulation->isSimulationStopped();
    //  bool noSim=App::currentWorld->simulation->isSimulationStopped();

    ui->qqExplicitHandling->setEnabled(isSensor && noEditModeAndNoSim);
    ui->qqShowVolume->setEnabled(isSensor && noEditModeAndNoSim);
    ui->qqPointSize->setEnabled(isSensor && noEditModeAndNoSim);
    ui->qqSensorTypeCombo->setEnabled(isSensor && noEditModeAndNoSim);
    ui->qqSensorTypeCombo->clear();
    ui->qqApplyMain->setEnabled(isSensor && manySensors && noEditModeAndNoSim);

    ui->qqAdjustVolume->setEnabled(noEditModeAndNoSim);
    ui->qqAdjustVolume->setChecked(CQDlgDetectionVolume::showVolumeWindow);

    ui->qqAdjustDetectionParams->setEnabled(isSensor && noEditModeAndNoSim);

    ui->qqVolumeColor->setEnabled(isSensor && noEditModeAndNoSim);
    ui->qqRayColor->setEnabled(isSensor && noEditModeAndNoSim);
    ui->qqApplyColors->setEnabled(isSensor && manySensors && noEditModeAndNoSim);

    ui->qqShowVolume->setChecked(isSensor && it->getShowVolume());
    ui->qqExplicitHandling->setChecked(isSensor && it->getExplicitHandling());

    ui->qqSensorTypeCombo->setVisible(App::userSettings->showOldDlgs);
    ui->qqSubtype->setVisible(App::userSettings->showOldDlgs);
    if (isSensor)
    {
        ui->qqPointSize->setText(utils::getSizeString(false, it->getProxSensorSize()).c_str());

        ui->qqSensorTypeCombo->addItem(IDS_DETECTABLE_ULTRASONIC,
                                       QVariant(sim_objectspecialproperty_detectable_ultrasonic));
        ui->qqSensorTypeCombo->addItem(IDS_DETECTABLE_INFRARED,
                                       QVariant(sim_objectspecialproperty_detectable_infrared));
        ui->qqSensorTypeCombo->addItem(IDS_DETECTABLE_LASER, QVariant(sim_objectspecialproperty_detectable_laser));
        ui->qqSensorTypeCombo->addItem(IDS_DETECTABLE_INDUCTIVE,
                                       QVariant(sim_objectspecialproperty_detectable_inductive));
        ui->qqSensorTypeCombo->addItem(IDS_DETECTABLE_CAPACITIVE,
                                       QVariant(sim_objectspecialproperty_detectable_capacitive));
        for (int i = 0; i < ui->qqSensorTypeCombo->count(); i++)
        {
            if (ui->qqSensorTypeCombo->itemData(i).toInt() == it->getSensableType())
            {
                ui->qqSensorTypeCombo->setCurrentIndex(i);
                break;
            }
        }
    }
    else
        ui->qqPointSize->setText("");

    selectLineEdit(lineEditToSelect);
    inMainRefreshRoutine = false;
}

void CQDlgProximitySensors::on_qqExplicitHandling_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_EXPLICITHANDLING_PROXSENSORGUITRIGGEREDCMD,
                                           App::currentWorld->sceneObjects->getLastSelectionHandle());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgProximitySensors::on_qqSensorTypeCombo_currentIndexChanged(int index)
{
    if (!inMainRefreshRoutine)
    {
        IF_UI_EVENT_CAN_READ_DATA
        {
            int objID = ui->qqSensorTypeCombo->itemData(ui->qqSensorTypeCombo->currentIndex()).toInt();
            App::appendSimulationThreadCommand(SET_SENSORSUBTYPE_PROXSENSORGUITRIGGEREDCMD,
                                               App::currentWorld->sceneObjects->getLastSelectionHandle(), objID);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgProximitySensors::on_qqPointSize_editingFinished()
{
    if (!ui->qqPointSize->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        double newVal = GuiApp::getEvalDouble(ui->qqPointSize->text().toStdString().c_str(), &ok);
        if (ok)
        {
            App::appendSimulationThreadCommand(SET_POINTSIZE_PROXSENSORGUITRIGGEREDCMD,
                                               App::currentWorld->sceneObjects->getLastSelectionHandle(), -1, newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgProximitySensors::on_qqShowVolume_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_SHOWVOLUME_PROXSENSORGUITRIGGEREDCMD,
                                           App::currentWorld->sceneObjects->getLastSelectionHandle());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgProximitySensors::on_qqApplyMain_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CProxSensor* last = App::currentWorld->sceneObjects->getLastSelectionProxSensor();
        if ((last != nullptr) && (App::currentWorld->sceneObjects->getObjectCountInSelection(sim_sceneobject_proximitysensor) >= 2))
        {
            SSimulationThreadCommand cmd;
            cmd.cmdId = APPLY_DETECTIONVOLUMEGUITRIGGEREDCMD;
            cmd.intParams.push_back(last->getObjectHandle());
            for (size_t i = 0; i < App::currentWorld->sceneObjects->getSelectionCount() - 1; i++)
                cmd.intParams.push_back(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
            App::appendSimulationThreadCommand(cmd);
            cmd.cmdId = APPLY_MAINPROP_PROXSENSORGUITRIGGEREDCMD;
            App::appendSimulationThreadCommand(cmd);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgProximitySensors::on_qqAdjustVolume_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CQDlgDetectionVolume::showVolumeWindow = !CQDlgDetectionVolume::showVolumeWindow;
        if (GuiApp::mainWindow->dlgCont->isVisible(DETECTION_VOLUME_DLG) != CQDlgDetectionVolume::showVolumeWindow)
            GuiApp::mainWindow->dlgCont->toggle(DETECTION_VOLUME_DLG);
    }
}

void CQDlgProximitySensors::on_qqAdjustDetectionParams_clicked()
{
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        CProxSensor* it = App::currentWorld->sceneObjects->getLastSelectionProxSensor();
        if (it != nullptr)
        {
            CQDlgProxSensDetectionParam theDialog(this);
            theDialog.frontFace = it->getFrontFaceDetection();
            theDialog.backFace = it->getBackFaceDetection();
            theDialog.fast = !it->getExactMode();
            theDialog.limitedAngle = it->getAllowedNormal() > 0.0;
            theDialog.angle = it->getAllowedNormal();
            theDialog.distanceContraint = it->convexVolume->getSmallestDistanceAllowed() > 0.0;
            theDialog.minimumDistance = it->convexVolume->getSmallestDistanceAllowed();
            theDialog.randomizedDetection = it->getRandomizedDetection();
            theDialog.rayCount = it->getRandomizedDetectionSampleCount();
            theDialog.rayDetectionCount = it->getRandomizedDetectionCountForDetection();
            theDialog.refresh();

            if (theDialog.makeDialogModal() != VDIALOG_MODAL_RETURN_CANCEL)
            {
                SSimulationThreadCommand cmd;
                cmd.cmdId = SET_DETECTIONPARAMS_PROXSENSORGUITRIGGEREDCMD;
                cmd.intParams.push_back(it->getObjectHandle());
                cmd.boolParams.push_back(theDialog.frontFace);
                cmd.boolParams.push_back(theDialog.backFace);
                cmd.boolParams.push_back(!theDialog.fast);
                cmd.boolParams.push_back(theDialog.limitedAngle);
                cmd.boolParams.push_back(theDialog.distanceContraint);
                cmd.doubleParams.push_back(theDialog.angle);
                cmd.doubleParams.push_back(theDialog.minimumDistance);
                cmd.intParams.push_back(theDialog.rayCount);
                cmd.intParams.push_back(theDialog.rayDetectionCount);
                App::appendSimulationThreadCommand(cmd);
                App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            }
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgProximitySensors::on_qqVolumeColor_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CQDlgMaterial::displayMaterialDlg(COLOR_ID_PROXSENSOR_VOLUME,
                                          App::currentWorld->sceneObjects->getLastSelectionHandle(), -1,
                                          GuiApp::mainWindow);
    }
}

void CQDlgProximitySensors::on_qqRayColor_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CQDlgMaterial::displayMaterialDlg(
            COLOR_ID_PROXSENSOR_RAY, App::currentWorld->sceneObjects->getLastSelectionHandle(), -1, GuiApp::mainWindow);
    }
}

void CQDlgProximitySensors::on_qqApplyColors_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        // CProxSensor* it=App::currentWorld->objCont->getLastSelection_proxSensor();
        SSimulationThreadCommand cmd;
        cmd.cmdId = APPLY_VISUALPROP_PROXSENSORGUITRIGGEREDCMD;
        cmd.intParams.push_back(App::currentWorld->sceneObjects->getLastSelectionHandle());
        for (size_t i = 0; i < App::currentWorld->sceneObjects->getSelectionCount(); i++)
            cmd.intParams.push_back(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
        App::appendSimulationThreadCommand(cmd);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
    }
}
