
#include "vrepMainHeader.h"
#include "qdlgvisionsensors.h"
#include "ui_qdlgvisionsensors.h"
#include "tt.h"
#include "gV.h"
#include "qdlgmaterial.h"
#include "qdlgimagecolor.h"
#include "v_repStrings.h"
#include "app.h"
#include "vMessageBox.h"

bool CQDlgVisionSensors::showFilterWindow=false;

CQDlgVisionSensors::CQDlgVisionSensors(QWidget *parent) :
    CDlgEx(parent),
    ui(new Ui::CQDlgVisionSensors)
{
    _dlgType=VISION_SENSOR_DLG;
    ui->setupUi(this);
    inMainRefreshRoutine=false;
}

CQDlgVisionSensors::~CQDlgVisionSensors()
{
    delete ui;
}

void CQDlgVisionSensors::cancelEvent()
{
    // we override this cancel event. The container window should close, not this one!!
    App::mainWindow->dlgCont->close(OBJECT_DLG);
}

void CQDlgVisionSensors::refresh()
{
    inMainRefreshRoutine=true;
    QLineEdit* lineEditToSelect=getSelectedLineEdit();

    CVisionSensor* it=App::ct->objCont->getLastSelection_visionSensor();

    bool isSensor=App::ct->objCont->isLastSelectionAVisionSensor();
    bool manySensors=App::ct->objCont->getVisionSensorNumberInSelection()>1;
    bool noEditModeAndNoSim=(App::getEditModeType()==NO_EDIT_MODE)&&App::ct->simulation->isSimulationStopped();

    ui->qqNearPlane->setEnabled(isSensor&&noEditModeAndNoSim);
    ui->qqFarPlane->setEnabled(isSensor&&noEditModeAndNoSim);

    ui->qqPerspectiveAngleOrOrthographicSize->setEnabled(isSensor&&noEditModeAndNoSim);

    ui->qqResX->setEnabled(isSensor&&noEditModeAndNoSim);
    ui->qqResY->setEnabled(isSensor&&noEditModeAndNoSim);

    ui->qqSizeX->setEnabled(isSensor&&noEditModeAndNoSim);
    ui->qqSizeY->setEnabled(isSensor&&noEditModeAndNoSim);
    ui->qqSizeZ->setEnabled(isSensor&&noEditModeAndNoSim);

    ui->qqPerspective->setEnabled(isSensor&&noEditModeAndNoSim);
    ui->qqExplicitHandling->setEnabled(isSensor&&noEditModeAndNoSim);
    ui->qqExternalInput->setEnabled(isSensor&&noEditModeAndNoSim);
    ui->qqLocalLights->setEnabled(isSensor&&noEditModeAndNoSim);

    ui->qqShowFog->setEnabled(isSensor&&noEditModeAndNoSim);

    ui->qqShowNotDetecting->setEnabled(isSensor&&noEditModeAndNoSim);
    ui->qqShowDetecting->setEnabled(isSensor&&noEditModeAndNoSim);

    ui->qqIgnoreRGB->setEnabled(isSensor&&noEditModeAndNoSim);
    ui->qqIgnoreDepth->setEnabled(isSensor&&noEditModeAndNoSim&&(it->getRenderMode()!=sim_rendermode_povray));
    ui->qqIgnorePacket1->setEnabled(isSensor&&noEditModeAndNoSim);

    ui->qqCasingColorPassive->setEnabled(isSensor&&noEditModeAndNoSim);
    ui->qqCasingColorActive->setEnabled(isSensor&&noEditModeAndNoSim);

    ui->qqEnabled->setChecked(App::ct->mainSettings->visionSensorsEnabled);

    ui->qqApplyMainProperties->setEnabled(isSensor&&manySensors&&noEditModeAndNoSim);
    ui->qqApplyColors->setEnabled(isSensor&&manySensors&&noEditModeAndNoSim);

    ui->qqAdjustImageColor->setEnabled(isSensor&&noEditModeAndNoSim);
    ui->qqAdjustFilters->setEnabled(noEditModeAndNoSim);
    ui->qqAdjustFilters->setChecked(showFilterWindow);

    ui->qqRenderModeCombo->setEnabled(isSensor&&noEditModeAndNoSim);
    ui->qqRenderModeCombo->clear();

    ui->qqEntityCombo->setEnabled(isSensor&&noEditModeAndNoSim);
    ui->qqEntityCombo->clear();

    if (isSensor)
    {
        CVisionSensor* s=App::ct->objCont->getLastSelection_visionSensor();

        ui->qqShowFog->setChecked(s->getShowFogIfAvailable());

        ui->qqNearPlane->setText(tt::getEString(false,s->getNearClippingPlane(),2).c_str());
        ui->qqFarPlane->setText(tt::getEString(false,s->getFarClippingPlane(),2).c_str());

        ui->qqPerspective->setChecked(s->getPerspectiveOperation());
        if (s->getPerspectiveOperation())
            ui->qqPerspectiveAngleOrOrthographicSize->setText(gv::getAngleStr(false,s->getViewAngle(),0).c_str());
        else
            ui->qqPerspectiveAngleOrOrthographicSize->setText(gv::getSizeStr(false,s->getOrthoViewSize()).c_str());

        int r[2];
        s->getDesiredResolution(r);
        ui->qqResX->setText(tt::getIString(false,r[0]).c_str());
        ui->qqResY->setText(tt::getIString(false,r[1]).c_str());
        C3Vector size(s->getSize());
        ui->qqSizeX->setText(tt::getFString(false,size(0),3).c_str());
        ui->qqSizeY->setText(tt::getFString(false,size(1),3).c_str());
        ui->qqSizeZ->setText(tt::getFString(false,size(2),3).c_str());

        ui->qqExplicitHandling->setChecked(s->getExplicitHandling());
        ui->qqExternalInput->setChecked(s->getUseExternalImage());
        ui->qqLocalLights->setChecked(s->getuseLocalLights());


        ui->qqShowNotDetecting->setChecked(s->getShowVolumeWhenNotDetecting());
        ui->qqShowDetecting->setChecked(s->getShowVolumeWhenDetecting());

        ui->qqIgnoreRGB->setChecked(s->getIgnoreRGBInfo());
        ui->qqIgnoreDepth->setChecked(s->getIgnoreDepthInfo());
        ui->qqIgnorePacket1->setChecked(!s->getComputeImageBasicStats());

        ui->qqRenderModeCombo->addItem(strTranslate(IDS_VISIBLE_COMPONENTS),QVariant(sim_rendermode_opengl));
        ui->qqRenderModeCombo->addItem(strTranslate(IDS_AUXILIARY_CHANNELS),QVariant(sim_rendermode_auxchannels));
        ui->qqRenderModeCombo->addItem(strTranslate(IDS_COLOR_CODED_IDS),QVariant(sim_rendermode_colorcoded));
        ui->qqRenderModeCombo->addItem(strTranslate(IDS_RAY_TRACING),QVariant(sim_rendermode_povray));
//        ui->qqRenderModeCombo->addItem(strTranslate(IDS_RAY_TRACING2),QVariant(4));
        ui->qqRenderModeCombo->addItem(strTranslate(IDS_EXTERNAL_RENDERER),QVariant(sim_rendermode_extrenderer));
        ui->qqRenderModeCombo->addItem(strTranslate(IDS_EXTERNAL_RENDERER_WINDOWED),QVariant(sim_rendermode_extrendererwindowed));
        ui->qqRenderModeCombo->addItem(strTranslate(IDS_OPENGL3),QVariant(sim_rendermode_opengl3));
        ui->qqRenderModeCombo->addItem(strTranslate(IDS_OPENGL3_WINDOWED),QVariant(sim_rendermode_opengl3windowed));

        // Select current item:
        for (int i=0;i<ui->qqRenderModeCombo->count();i++)
        {
            if (ui->qqRenderModeCombo->itemData(i).toInt()==it->getRenderMode())
            {
                ui->qqRenderModeCombo->setCurrentIndex(i);
                break;
            }
        }

        ui->qqEntityCombo->addItem(strTranslate(IDS_ALL_RENDERABLE_OBJECTS_IN_SCENE),QVariant(-1));

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
            ui->qqEntityCombo->addItem(names[i].c_str(),QVariant(ids[i]));

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
            ui->qqEntityCombo->addItem(names[i].c_str(),QVariant(ids[i]));

        names.clear();
        ids.clear();

        // Now paths:
        for (int i=0;i<int(App::ct->objCont->pathList.size());i++)
        {
            CPath* it=App::ct->objCont->getPath(App::ct->objCont->pathList[i]);
            std::string name(tt::decorateString("[",strTranslate(IDSN_PATH),"] "));
            name+=it->getObjectName();
            names.push_back(name);
            ids.push_back(it->getObjectHandle());
        }
        tt::orderStrings(names,ids);
        for (int i=0;i<int(names.size());i++)
            ui->qqEntityCombo->addItem(names[i].c_str(),QVariant(ids[i]));

        names.clear();
        ids.clear();

        // Now graphs:
        for (int i=0;i<int(App::ct->objCont->graphList.size());i++)
        {
            CGraph* it=App::ct->objCont->getGraph(App::ct->objCont->graphList[i]);
            std::string name(tt::decorateString("[",strTranslate(IDSN_GRAPH),"] "));
            name+=it->getObjectName();
            names.push_back(name);
            ids.push_back(it->getObjectHandle());
        }
        tt::orderStrings(names,ids);
        for (int i=0;i<int(names.size());i++)
            ui->qqEntityCombo->addItem(names[i].c_str(),QVariant(ids[i]));

        // Select current item:
        for (int i=0;i<ui->qqEntityCombo->count();i++)
        {
            if (ui->qqEntityCombo->itemData(i).toInt()==it->getDetectableEntityID())
            {
                ui->qqEntityCombo->setCurrentIndex(i);
                break;
            }
        }
    }
    else
    {
        ui->qqNearPlane->setText("");
        ui->qqFarPlane->setText("");

        ui->qqPerspectiveAngleOrOrthographicSize->setText("");

        ui->qqResX->setText("");
        ui->qqResY->setText("");

        ui->qqSizeX->setText("");
        ui->qqSizeY->setText("");
        ui->qqSizeZ->setText("");

        ui->qqPerspective->setChecked(false);
        ui->qqExplicitHandling->setChecked(false);
        ui->qqExternalInput->setChecked(false);
        ui->qqLocalLights->setChecked(false);

        ui->qqShowFog->setChecked(false);

        ui->qqShowNotDetecting->setChecked(false);
        ui->qqShowDetecting->setChecked(false);
        ui->qqIgnoreRGB->setChecked(false);
        ui->qqIgnoreDepth->setChecked(false);
        ui->qqIgnorePacket1->setChecked(false);
    }

    selectLineEdit(lineEditToSelect);
    inMainRefreshRoutine=false;
}

void CQDlgVisionSensors::on_qqEnabled_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_ENABLE_ALL_VISIONSENSORGUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgVisionSensors::on_qqExplicitHandling_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_EXPLICITHANDLING_VISIONSENSORGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgVisionSensors::on_qqExternalInput_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_EXTERNALINPUT_VISIONSENSORGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgVisionSensors::on_qqPerspective_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_PERSPECTIVE_VISIONSENSORGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgVisionSensors::on_qqLocalLights_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_LOCALLIGHTS_VISIONSENSORGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgVisionSensors::on_qqShowNotDetecting_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_SHOWVOLUME_WHEN_NOT_DETECTING_VISIONSENSORGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgVisionSensors::on_qqShowFog_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_SHOWFOG_VISIONSENSORGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgVisionSensors::on_qqShowDetecting_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_SHOWVOLUME_WHEN_DETECTING_VISIONSENSORGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgVisionSensors::on_qqNearPlane_editingFinished()
{
    if (!ui->qqNearPlane->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        float newVal=ui->qqNearPlane->text().toFloat(&ok);
        if (ok)
        {
            App::appendSimulationThreadCommand(SET_NEARCLIPPING_VISIONSENSORGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),-1,newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgVisionSensors::on_qqFarPlane_editingFinished()
{
    if (!ui->qqFarPlane->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        float newVal=ui->qqFarPlane->text().toFloat(&ok);
        if (ok)
        {
            App::appendSimulationThreadCommand(SET_FARCLIPPING_VISIONSENSORGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),-1,newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgVisionSensors::on_qqPerspectiveAngleOrOrthographicSize_editingFinished()
{
    if (!ui->qqPerspectiveAngleOrOrthographicSize->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        float newVal=ui->qqPerspectiveAngleOrOrthographicSize->text().toFloat(&ok);
        if (ok)
        {
            CVisionSensor* it=App::ct->objCont->getLastSelection_visionSensor();
            if ((it!=nullptr)&&it->getPerspectiveOperation())
                newVal*=gv::userToRad;
            App::appendSimulationThreadCommand(SET_PERSPECTANGLE_OR_ORTHOSIZE_VISIONSENSORGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),-1,newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgVisionSensors::on_qqResX_editingFinished()
{
    if (!ui->qqResX->isModified())
        return;
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        CVisionSensor* it=App::ct->objCont->getLastSelection_visionSensor();
        if (it!=nullptr)
        {
            bool ok;
            int newVal=ui->qqResX->text().toInt(&ok);
            if (ok)
            {
                int r[2];
                it->getDesiredResolution(r);
                if (newVal!=r[0])
                {
                    r[0]=newVal;
                    // Check if the resolution is a power of 2:
                    newVal&=(32768-1);
                    unsigned short tmp=32768;
                    while (tmp!=1)
                    {
                        if (newVal&tmp)
                            newVal&=tmp;
                        tmp/=2;
                    }
                    if (newVal!=r[0])
                        App::uiThread->messageBox_warning(App::mainWindow,strTranslate("Vision sensor"),strTranslate(IDS_VISION_SENSOR_RESOLUTION_NOT_POWER_OF_TWO_WARNING),VMESSAGEBOX_OKELI);
                    SSimulationThreadCommand cmd;
                    cmd.cmdId=SET_RESOLUTION_VISIONSENSORGUITRIGGEREDCMD;
                    cmd.intParams.push_back(it->getObjectHandle());
                    cmd.intParams.push_back(r[0]);
                    cmd.intParams.push_back(r[1]);
                    App::appendSimulationThreadCommand(cmd);
                    App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
                }
            }
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgVisionSensors::on_qqResY_editingFinished()
{
    if (!ui->qqResY->isModified())
        return;
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        CVisionSensor* it=App::ct->objCont->getLastSelection_visionSensor();
        if (it!=nullptr)
        {
            bool ok;
            int newVal=ui->qqResY->text().toInt(&ok);
            if (ok)
            {
                int r[2];
                it->getDesiredResolution(r);
                if (newVal!=r[1])
                {
                    r[1]=newVal;
                    // Check if the resolution is a power of 2:
                    newVal&=(32768-1);
                    unsigned short tmp=32768;
                    while (tmp!=1)
                    {
                        if (newVal&tmp)
                            newVal&=tmp;
                        tmp/=2;
                    }
                    if (newVal!=r[1])
                        App::uiThread->messageBox_warning(App::mainWindow,strTranslate("Vision sensor"),strTranslate(IDS_VISION_SENSOR_RESOLUTION_NOT_POWER_OF_TWO_WARNING),VMESSAGEBOX_OKELI);
                    SSimulationThreadCommand cmd;
                    cmd.cmdId=SET_RESOLUTION_VISIONSENSORGUITRIGGEREDCMD;
                    cmd.intParams.push_back(it->getObjectHandle());
                    cmd.intParams.push_back(r[0]);
                    cmd.intParams.push_back(r[1]);
                    App::appendSimulationThreadCommand(cmd);
                    App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
                }
            }
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgVisionSensors::on_qqSizeX_editingFinished()
{
    if (!ui->qqSizeX->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        CVisionSensor* it=App::ct->objCont->getLastSelection_visionSensor();
        if (it!=nullptr)
        {
            bool ok;
            float newVal=ui->qqSizeX->text().toFloat(&ok);
            if (ok)
            {
                C3Vector s(it->getSize());
                s(0)=newVal;
                SSimulationThreadCommand cmd;
                cmd.cmdId=SET_OBJECTSIZE_VISIONSENSORGUITRIGGEREDCMD;
                cmd.intParams.push_back(it->getObjectHandle());
                cmd.floatParams.push_back(s(0));
                cmd.floatParams.push_back(s(1));
                cmd.floatParams.push_back(s(2));
                App::appendSimulationThreadCommand(cmd);
                App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            }
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgVisionSensors::on_qqSizeY_editingFinished()
{
    if (!ui->qqSizeY->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        CVisionSensor* it=App::ct->objCont->getLastSelection_visionSensor();
        if (it!=nullptr)
        {
            bool ok;
            float newVal=ui->qqSizeY->text().toFloat(&ok);
            if (ok)
            {
                C3Vector s(it->getSize());
                s(1)=newVal;
                SSimulationThreadCommand cmd;
                cmd.cmdId=SET_OBJECTSIZE_VISIONSENSORGUITRIGGEREDCMD;
                cmd.intParams.push_back(it->getObjectHandle());
                cmd.floatParams.push_back(s(0));
                cmd.floatParams.push_back(s(1));
                cmd.floatParams.push_back(s(2));
                App::appendSimulationThreadCommand(cmd);
                App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            }
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgVisionSensors::on_qqSizeZ_editingFinished()
{
    if (!ui->qqSizeZ->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        CVisionSensor* it=App::ct->objCont->getLastSelection_visionSensor();
        if (it!=nullptr)
        {
            bool ok;
            float newVal=ui->qqSizeZ->text().toFloat(&ok);
            if (ok)
            {
                C3Vector s(it->getSize());
                s(2)=newVal;
                SSimulationThreadCommand cmd;
                cmd.cmdId=SET_OBJECTSIZE_VISIONSENSORGUITRIGGEREDCMD;
                cmd.intParams.push_back(it->getObjectHandle());
                cmd.floatParams.push_back(s(0));
                cmd.floatParams.push_back(s(1));
                cmd.floatParams.push_back(s(2));
                App::appendSimulationThreadCommand(cmd);
                App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            }
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgVisionSensors::on_qqAdjustImageColor_clicked()
{
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        CVisionSensor* it=App::ct->objCont->getLastSelection_visionSensor();
        if (it!=nullptr)
        {
            CQDlgImageColor theDialog(this);
            float col[3];
            it->getDefaultBufferValues(col);
            theDialog.sameAsFog=it->getUseEnvironmentBackgroundColor();
            theDialog.red=col[0];
            theDialog.green=col[1];
            theDialog.blue=col[2];
            theDialog.refresh();
            if (theDialog.makeDialogModal()!=VDIALOG_MODAL_RETURN_CANCEL)
            {
                col[0]=theDialog.red;
                col[1]=theDialog.green;
                col[2]=theDialog.blue;
                SSimulationThreadCommand cmd;
                cmd.cmdId=SET_DEFAULTIMGCOL_VISIONSENSORGUITRIGGEREDCMD;
                cmd.intParams.push_back(it->getObjectHandle());
                cmd.floatParams.push_back(col[0]);
                cmd.floatParams.push_back(col[1]);
                cmd.floatParams.push_back(col[2]);
                cmd.boolParams.push_back(theDialog.sameAsFog);
                App::appendSimulationThreadCommand(cmd);
                App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
                App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
            }
        }
    }
}

void CQDlgVisionSensors::on_qqApplyMainProperties_clicked()
{
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        CVisionSensor* last=App::ct->objCont->getLastSelection_visionSensor();
        if (last!=nullptr)
        {
            SSimulationThreadCommand cmd;
            cmd.cmdId=APPLY_MAINPROP_TO_SELECTION_VISIONSENSORGUITRIGGEREDCMD;
            cmd.intParams.push_back(last->getObjectHandle());
            for (int i=0;i<App::ct->objCont->getSelSize()-1;i++)
            {
                CVisionSensor* it=App::ct->objCont->getVisionSensor(App::ct->objCont->getSelID(i));
                if (it!=nullptr)
                    cmd.intParams.push_back(it->getObjectHandle());
            }
            App::appendSimulationThreadCommand(cmd);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgVisionSensors::on_qqAdjustFilters_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        showFilterWindow=!showFilterWindow;
        if (App::mainWindow->dlgCont->isVisible(VISION_SENSOR_FILTER_DLG)!=showFilterWindow)
            App::mainWindow->dlgCont->toggle(VISION_SENSOR_FILTER_DLG);
    }
}

void CQDlgVisionSensors::on_qqCasingColorPassive_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CQDlgMaterial::displayMaterialDlg(COLOR_ID_VISIONSENSOR_PASSIVE,App::ct->objCont->getLastSelectionID(),-1,App::mainWindow);
    }
}

void CQDlgVisionSensors::on_qqCasingColorActive_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CQDlgMaterial::displayMaterialDlg(COLOR_ID_VISIONSENSOR_ACTIVE,App::ct->objCont->getLastSelectionID(),-1,App::mainWindow);
    }
}

void CQDlgVisionSensors::on_qqApplyColors_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CVisionSensor* last=App::ct->objCont->getLastSelection_visionSensor();
        if (last!=nullptr)
        {
            SSimulationThreadCommand cmd;
            cmd.cmdId=APPLY_VISUALPROP_TO_SELECTION_VISIONSENSORGUITRIGGEREDCMD;
            cmd.intParams.push_back(last->getObjectHandle());
            for (int i=0;i<App::ct->objCont->getSelSize()-1;i++)
            {
                CVisionSensor* it=App::ct->objCont->getVisionSensor(App::ct->objCont->getSelID(i));
                if (it!=nullptr)
                    cmd.intParams.push_back(it->getObjectHandle());
            }
            App::appendSimulationThreadCommand(cmd);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgVisionSensors::on_qqEntityCombo_currentIndexChanged(int index)
{
    if (!inMainRefreshRoutine)
    {
        IF_UI_EVENT_CAN_WRITE_DATA
        {
            int objID=ui->qqEntityCombo->itemData(ui->qqEntityCombo->currentIndex()).toInt();
            CVisionSensor* it=App::ct->objCont->getLastSelection_visionSensor();
            if ( (it!=nullptr)&&(objID!=-1) )
            {
                bool displayWarning=false;
                if ((objID<SIM_IDSTART_COLLECTION)&&(objID>=0))
                {
                    C3DObject* it2=App::ct->objCont->getObjectFromHandle(objID);
                    if (it2!=nullptr)
                        displayWarning|=((it2->getLocalObjectSpecialProperty()&sim_objectspecialproperty_renderable)==0);
                }
                if (displayWarning)
                    App::uiThread->messageBox_warning(App::mainWindow,strTranslate("Scene object"),strTranslate(IDS_OBJECT_NOT_RENDERABLE_WARNING),VMESSAGEBOX_OKELI);
            }
            App::appendSimulationThreadCommand(SET_ENTITYTODETECT_VISIONSENSORGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),objID);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgVisionSensors::on_qqIgnoreRGB_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_IGNORERGB_VISIONSENSORGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgVisionSensors::on_qqIgnoreDepth_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_IGNOREDEPTH_VISIONSENSORGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgVisionSensors::on_qqIgnorePacket1_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_PACKET1BLANK_VISIONSENSORGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgVisionSensors::on_qqRenderModeCombo_currentIndexChanged(int index)
{
    if (!inMainRefreshRoutine)
    {
        IF_UI_EVENT_CAN_WRITE_DATA
        {
            int renderMode=ui->qqRenderModeCombo->itemData(ui->qqRenderModeCombo->currentIndex()).toInt();
            App::appendSimulationThreadCommand(SET_RENDERMODE_VISIONSENSORGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),renderMode);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}
