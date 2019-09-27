
#include "vrepMainHeader.h"
#include "qdlgproximitysensors.h"
#include "ui_qdlgproximitysensors.h"
#include "tt.h"
#include "gV.h"
#include "qdlgmaterial.h"
#include "qdlgproxsensdetectionparam.h"
#include "qdlgdetectionvolume.h"
#include "v_repStrings.h"
#include "app.h"
#include "vMessageBox.h"

CQDlgProximitySensors::CQDlgProximitySensors(QWidget *parent) :
    CDlgEx(parent),
    ui(new Ui::CQDlgProximitySensors)
{
    _dlgType=PROXIMITY_SENSOR_DLG;
    ui->setupUi(this);
    inMainRefreshRoutine=false;
}

CQDlgProximitySensors::~CQDlgProximitySensors()
{
    delete ui;
}

void CQDlgProximitySensors::cancelEvent()
{
    // we override this cancel event. The container window should close, not this one!!
    App::mainWindow->dlgCont->close(OBJECT_DLG);
}

void CQDlgProximitySensors::refresh()
{
    inMainRefreshRoutine=true;
    QLineEdit* lineEditToSelect=getSelectedLineEdit();

    CProxSensor* it=App::ct->objCont->getLastSelection_proxSensor();

    bool isSensor=App::ct->objCont->isLastSelectionAProxSensor();
    bool manySensors=App::ct->objCont->getProxSensorNumberInSelection()>1;
    bool noEditMode=App::getEditModeType()==NO_EDIT_MODE;
    bool noEditModeAndNoSim=noEditMode&&App::ct->simulation->isSimulationStopped();
//  bool noSim=App::ct->simulation->isSimulationStopped();

    ui->qqEnableAll->setEnabled(noEditMode);
    ui->qqEnableAll->setChecked(App::ct->mainSettings->proximitySensorsEnabled);

    ui->qqExplicitHandling->setEnabled(isSensor&&noEditModeAndNoSim);
    ui->qqShowDetecting->setEnabled(isSensor&&noEditModeAndNoSim);
    ui->qqShowNotDetecting->setEnabled(isSensor&&noEditModeAndNoSim);
    ui->qqPointSize->setEnabled(isSensor&&noEditModeAndNoSim);
    ui->qqSensorTypeCombo->setEnabled(isSensor&&noEditModeAndNoSim);
    ui->qqSensorTypeCombo->clear();
    ui->qqApplyMain->setEnabled(isSensor&&manySensors&&noEditModeAndNoSim);

    ui->qqAdjustVolume->setEnabled(noEditModeAndNoSim);
    ui->qqAdjustVolume->setChecked(CQDlgDetectionVolume::showVolumeWindow);

    ui->qqEntityToDetect->setEnabled(isSensor&&noEditModeAndNoSim);
    ui->qqEntityToDetect->clear();

    ui->qqAdjustDetectionParams->setEnabled(isSensor&&noEditModeAndNoSim);

    ui->qqPassiveVolumeColor->setEnabled(isSensor&&noEditModeAndNoSim);
    ui->qqActiveVolumeColor->setEnabled(isSensor&&noEditModeAndNoSim);
    ui->qqMinDistColor->setEnabled(isSensor&&noEditModeAndNoSim);
    ui->qqRayColor->setEnabled(isSensor&&noEditModeAndNoSim);
    ui->qqApplyColors->setEnabled(isSensor&&manySensors&&noEditModeAndNoSim);

    ui->qqShowDetecting->setChecked(isSensor&&it->getShowVolumeWhenDetecting());
    ui->qqShowNotDetecting->setChecked(isSensor&&it->getShowVolumeWhenNotDetecting());
    ui->qqExplicitHandling->setChecked(isSensor&&it->getExplicitHandling());

    if (isSensor)
    {
        ui->qqPointSize->setText(tt::getFString(false,it->getSize(),3).c_str());

        ui->qqSensorTypeCombo->addItem(strTranslate(IDS_DETECTABLE_ULTRASONIC),QVariant(sim_objectspecialproperty_detectable_ultrasonic));
        ui->qqSensorTypeCombo->addItem(strTranslate(IDS_DETECTABLE_INFRARED),QVariant(sim_objectspecialproperty_detectable_infrared));
        ui->qqSensorTypeCombo->addItem(strTranslate(IDS_DETECTABLE_LASER),QVariant(sim_objectspecialproperty_detectable_laser));
        ui->qqSensorTypeCombo->addItem(strTranslate(IDS_DETECTABLE_INDUCTIVE),QVariant(sim_objectspecialproperty_detectable_inductive));
        ui->qqSensorTypeCombo->addItem(strTranslate(IDS_DETECTABLE_CAPACITIVE),QVariant(sim_objectspecialproperty_detectable_capacitive));
        for (int i=0;i<ui->qqSensorTypeCombo->count();i++)
        {
            if (ui->qqSensorTypeCombo->itemData(i).toInt()==it->getSensableType())
            {
                ui->qqSensorTypeCombo->setCurrentIndex(i);
                break;
            }
        }


        ui->qqEntityToDetect->addItem(strTranslate(IDSN_ALL_DETECTABLE_OBJECTS_IN_SCENE),QVariant(-1));
        std::vector<std::string> names;
        std::vector<int> ids;
        // Now collections:
        for (int i=0;i<int(App::ct->collections->allCollections.size());i++)
        {
            CRegCollection* it=App::ct->collections->allCollections[i];
            std::string name(tt::decorateString("[",strTranslate(IDSN_COLLECTION),"] "));
            name+=it->getCollectionName();
            names.push_back(name);
            ids.push_back(it->getCollectionID());
        }
        tt::orderStrings(names,ids);
        for (int i=0;i<int(names.size());i++)
            ui->qqEntityToDetect->addItem(names[i].c_str(),QVariant(ids[i]));

        names.clear();
        ids.clear();

        // Now shapes:
        for (int i=0;i<int(App::ct->objCont->shapeList.size());i++)
        {
            CShape* it=App::ct->objCont->getShape(App::ct->objCont->shapeList[i]);
            std::string name(tt::decorateString("[",strTranslate(IDSN_SHAPE),"] "));
            name+=it->getObjectName();
            names.push_back(name);
            ids.push_back(it->getObjectHandle());
        }
        tt::orderStrings(names,ids);
        for (int i=0;i<int(names.size());i++)
            ui->qqEntityToDetect->addItem(names[i].c_str(),QVariant(ids[i]));

        names.clear();
        ids.clear();

        // Now dummies:
        for (int i=0;i<int(App::ct->objCont->dummyList.size());i++)
        {
            CDummy* it=App::ct->objCont->getDummy(App::ct->objCont->dummyList[i]);
            std::string name(tt::decorateString("[",strTranslate(IDSN_DUMMY),"] "));
            name+=it->getObjectName();
            names.push_back(name);
            ids.push_back(it->getObjectHandle());
        }
        tt::orderStrings(names,ids);
        for (int i=0;i<int(names.size());i++)
            ui->qqEntityToDetect->addItem(names[i].c_str(),QVariant(ids[i]));

        // Select current item:
        for (int i=0;i<ui->qqEntityToDetect->count();i++)
        {
            if (ui->qqEntityToDetect->itemData(i).toInt()==it->getSensableObject())
            {
                ui->qqEntityToDetect->setCurrentIndex(i);
                break;
            }
        }
    }
    else
        ui->qqPointSize->setText("");

    selectLineEdit(lineEditToSelect);
    inMainRefreshRoutine=false;
}

void CQDlgProximitySensors::on_qqEnableAll_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_ENABLEALL_PROXSENSORGUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgProximitySensors::on_qqExplicitHandling_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
            App::appendSimulationThreadCommand(TOGGLE_EXPLICITHANDLING_PROXSENSORGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID());
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
            int objID=ui->qqSensorTypeCombo->itemData(ui->qqSensorTypeCombo->currentIndex()).toInt();
            App::appendSimulationThreadCommand(SET_SENSORSUBTYPE_PROXSENSORGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),objID);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgProximitySensors::on_qqShowDetecting_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_SHOWVOLWHENDETECTING_PROXSENSORGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgProximitySensors::on_qqPointSize_editingFinished()
{
    if (!ui->qqPointSize->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        float newVal=ui->qqPointSize->text().toFloat(&ok);
        if (ok)
        {
            App::appendSimulationThreadCommand(SET_POINTSIZE_PROXSENSORGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),-1,newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgProximitySensors::on_qqShowNotDetecting_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_SHOWVOLWHENNOTDETECTING_PROXSENSORGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgProximitySensors::on_qqApplyMain_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CProxSensor* last=App::ct->objCont->getLastSelection_proxSensor();
        if ((last!=nullptr)&&(App::ct->objCont->getProxSensorNumberInSelection()>=2))
        {
            SSimulationThreadCommand cmd;
            cmd.cmdId=APPLY_DETECTIONVOLUMEGUITRIGGEREDCMD;
            cmd.intParams.push_back(last->getObjectHandle());
            for (int i=0;i<App::ct->objCont->getSelSize()-1;i++)
                cmd.intParams.push_back(App::ct->objCont->getSelID(i));
            App::appendSimulationThreadCommand(cmd);
            cmd.cmdId=APPLY_MAINPROP_PROXSENSORGUITRIGGEREDCMD;
            App::appendSimulationThreadCommand(cmd);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgProximitySensors::on_qqAdjustVolume_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CQDlgDetectionVolume::showVolumeWindow=!CQDlgDetectionVolume::showVolumeWindow;
        if (App::mainWindow->dlgCont->isVisible(DETECTION_VOLUME_DLG)!=CQDlgDetectionVolume::showVolumeWindow)
            App::mainWindow->dlgCont->toggle(DETECTION_VOLUME_DLG);
    }
}

void CQDlgProximitySensors::on_qqEntityToDetect_currentIndexChanged(int index)
{
    if (!inMainRefreshRoutine)
    {
        IF_UI_EVENT_CAN_READ_DATA
        {
            CProxSensor* it=App::ct->objCont->getLastSelection_proxSensor();
            int objID=ui->qqEntityToDetect->itemData(ui->qqEntityToDetect->currentIndex()).toInt();
            if ((objID!=-1)&&(it!=nullptr))
            {
                bool displayWarning=false;
                if ((objID<SIM_IDSTART_COLLECTION)&&(objID>=0))
                {
                    C3DObject* it2=App::ct->objCont->getObjectFromHandle(objID);
                    if (it2!=nullptr)
                        displayWarning|=((it2->getLocalObjectSpecialProperty()&sim_objectspecialproperty_detectable_all)==0);
                }
                if (displayWarning)
                    App::uiThread->messageBox_warning(App::mainWindow,strTranslate("Scene object"),strTranslate(IDS_OBJECT_NOT_DETECTABLE_WARNING),VMESSAGEBOX_OKELI);
            }
            App::appendSimulationThreadCommand(SET_ENTITYTODETECT_PROXSENSORGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),objID);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgProximitySensors::on_qqAdjustDetectionParams_clicked()
{
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        CProxSensor* it=App::ct->objCont->getLastSelection_proxSensor();
        if (it!=nullptr)
        {
            CQDlgProxSensDetectionParam theDialog(this);
            theDialog.frontFace=it->getFrontFaceDetection();
            theDialog.backFace=it->getBackFaceDetection();
            theDialog.fast=!it->getClosestObjectMode();
            theDialog.limitedAngle=it->getNormalCheck();
            theDialog.angle=it->getAllowedNormal();
//            theDialog.occlusionCheck=it->getCheckOcclusions();
            theDialog.distanceContraint=it->convexVolume->getSmallestDistanceEnabled();
            theDialog.minimumDistance=it->convexVolume->getSmallestDistanceAllowed();
            theDialog.randomizedDetection=it->getRandomizedDetection();
            theDialog.rayCount=it->getRandomizedDetectionSampleCount();
            theDialog.rayDetectionCount=it->getRandomizedDetectionCountForDetection();
            theDialog.refresh();

            if (theDialog.makeDialogModal()!=VDIALOG_MODAL_RETURN_CANCEL)
            {
                SSimulationThreadCommand cmd;
                cmd.cmdId=SET_DETECTIONPARAMS_PROXSENSORGUITRIGGEREDCMD;
                cmd.intParams.push_back(it->getObjectHandle());
                cmd.boolParams.push_back(theDialog.frontFace);
                cmd.boolParams.push_back(theDialog.backFace);
                cmd.boolParams.push_back(!theDialog.fast);
                cmd.boolParams.push_back(theDialog.limitedAngle);
                cmd.floatParams.push_back(theDialog.angle);
//                cmd.boolParams.push_back(theDialog.occlusionCheck);
                cmd.boolParams.push_back(theDialog.distanceContraint);
                cmd.floatParams.push_back(theDialog.minimumDistance);
                cmd.intParams.push_back(theDialog.rayCount);
                cmd.intParams.push_back(theDialog.rayDetectionCount);
                App::appendSimulationThreadCommand(cmd);
                App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            }
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgProximitySensors::on_qqPassiveVolumeColor_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CQDlgMaterial::displayMaterialDlg(COLOR_ID_PROXSENSOR_PASSIVE,App::ct->objCont->getLastSelectionID(),-1,App::mainWindow);
    }
}

void CQDlgProximitySensors::on_qqActiveVolumeColor_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CQDlgMaterial::displayMaterialDlg(COLOR_ID_PROXSENSOR_ACTIVE,App::ct->objCont->getLastSelectionID(),-1,App::mainWindow);
    }
}

void CQDlgProximitySensors::on_qqRayColor_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CQDlgMaterial::displayMaterialDlg(COLOR_ID_PROXSENSOR_RAY,App::ct->objCont->getLastSelectionID(),-1,App::mainWindow);
    }
}

void CQDlgProximitySensors::on_qqMinDistColor_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CQDlgMaterial::displayMaterialDlg(COLOR_ID_PROXSENSOR_MINDIST,App::ct->objCont->getLastSelectionID(),-1,App::mainWindow);
    }
}

void CQDlgProximitySensors::on_qqApplyColors_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        //CProxSensor* it=App::ct->objCont->getLastSelection_proxSensor();
        SSimulationThreadCommand cmd;
        cmd.cmdId=APPLY_VISUALPROP_PROXSENSORGUITRIGGEREDCMD;
        cmd.intParams.push_back(App::ct->objCont->getLastSelectionID());
        for (int i=0;i<App::ct->objCont->getSelSize();i++)
            cmd.intParams.push_back(App::ct->objCont->getSelID(i));
        App::appendSimulationThreadCommand(cmd);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
    }
}
