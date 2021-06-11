#include "qdlgvisionsensors.h"
#include "ui_qdlgvisionsensors.h"
#include "tt.h"
#include "gV.h"
#include "qdlgmaterial.h"
#include "qdlgimagecolor.h"
#include "simStrings.h"
#include "app.h"
#include "vMessageBox.h"

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

    CVisionSensor* it=App::currentWorld->sceneObjects->getLastSelectionVisionSensor();

    bool isSensor=App::currentWorld->sceneObjects->isLastSelectionAVisionSensor();
    bool manySensors=App::currentWorld->sceneObjects->getVisionSensorCountInSelection()>1;
    bool noEditModeAndNoSim=(App::getEditModeType()==NO_EDIT_MODE)&&App::currentWorld->simulation->isSimulationStopped();

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
    ui->qqLocalLights->setVisible(App::userSettings->showOldDlgs);

    ui->qqShowFog->setEnabled(isSensor&&noEditModeAndNoSim);

    ui->qqShowNotDetecting->setEnabled(isSensor&&noEditModeAndNoSim);
    ui->qqShowDetecting->setEnabled(isSensor&&noEditModeAndNoSim);

    ui->qqIgnoreRGB->setEnabled(isSensor&&noEditModeAndNoSim);
    ui->qqIgnoreDepth->setEnabled(isSensor&&noEditModeAndNoSim&&(it->getRenderMode()!=sim_rendermode_povray));
    ui->qqIgnorePacket1->setEnabled(isSensor&&noEditModeAndNoSim);

    ui->qqCasingColorPassive->setEnabled(isSensor&&noEditModeAndNoSim);
    ui->qqCasingColorActive->setEnabled(isSensor&&noEditModeAndNoSim);

    ui->qqEnabled->setChecked(App::currentWorld->mainSettings->visionSensorsEnabled);

    ui->qqApplyMainProperties->setEnabled(isSensor&&manySensors&&noEditModeAndNoSim);
    ui->qqApplyColors->setEnabled(isSensor&&manySensors&&noEditModeAndNoSim);

    ui->qqAdjustImageColor->setEnabled(isSensor&&noEditModeAndNoSim);

    ui->qqRenderModeCombo->setEnabled(isSensor&&noEditModeAndNoSim);
    ui->qqRenderModeCombo->clear();

    ui->qqEntityCombo->setEnabled(isSensor&&noEditModeAndNoSim);
    ui->qqEntityCombo->clear();

    if (isSensor)
    {
        CVisionSensor* s=App::currentWorld->sceneObjects->getLastSelectionVisionSensor();

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

        ui->qqRenderModeCombo->addItem(IDS_VISIBLE_COMPONENTS,QVariant(sim_rendermode_opengl));
        ui->qqRenderModeCombo->addItem(IDS_AUXILIARY_CHANNELS,QVariant(sim_rendermode_auxchannels));
        ui->qqRenderModeCombo->addItem(IDS_COLOR_CODED_IDS,QVariant(sim_rendermode_colorcoded));
        ui->qqRenderModeCombo->addItem(IDS_RAY_TRACING,QVariant(sim_rendermode_povray));
//        ui->qqRenderModeCombo->addItem(IDS_RAY_TRACING2,QVariant(4));
        ui->qqRenderModeCombo->addItem(IDS_EXTERNAL_RENDERER,QVariant(sim_rendermode_extrenderer));
        ui->qqRenderModeCombo->addItem(IDS_EXTERNAL_RENDERER_WINDOWED,QVariant(sim_rendermode_extrendererwindowed));
        ui->qqRenderModeCombo->addItem(IDS_OPENGL3,QVariant(sim_rendermode_opengl3));
        ui->qqRenderModeCombo->addItem(IDS_OPENGL3_WINDOWED,QVariant(sim_rendermode_opengl3windowed));

        // Select current item:
        for (int i=0;i<ui->qqRenderModeCombo->count();i++)
        {
            if (ui->qqRenderModeCombo->itemData(i).toInt()==it->getRenderMode())
            {
                ui->qqRenderModeCombo->setCurrentIndex(i);
                break;
            }
        }

        ui->qqEntityCombo->addItem(IDS_ALL_RENDERABLE_OBJECTS_IN_SCENE,QVariant(-1));

        std::vector<std::string> names;
        std::vector<int> ids;

        // Now collections:
        for (size_t i=0;i<App::currentWorld->collections->getObjectCount();i++)
        {
            CCollection* it=App::currentWorld->collections->getObjectFromIndex(i);
            std::string name(tt::decorateString("[",IDSN_COLLECTION,"] "));
            name+=it->getCollectionName();
            names.push_back(name);
            ids.push_back(it->getCollectionHandle());
        }
        tt::orderStrings(names,ids);
        for (size_t i=0;i<names.size();i++)
            ui->qqEntityCombo->addItem(names[i].c_str(),QVariant(ids[i]));

        names.clear();
        ids.clear();

        // Now shapes:
        for (size_t i=0;i<App::currentWorld->sceneObjects->getShapeCount();i++)
        {
            CShape* it=App::currentWorld->sceneObjects->getShapeFromIndex(i);
            std::string name(tt::decorateString("[",IDSN_SHAPE,"] "));
            name+=it->getObjectAlias_shortPath();
            names.push_back(name);
            ids.push_back(it->getObjectHandle());
        }
        tt::orderStrings(names,ids);
        for (int i=0;i<int(names.size());i++)
            ui->qqEntityCombo->addItem(names[i].c_str(),QVariant(ids[i]));

        names.clear();
        ids.clear();

        // Now paths:
        for (size_t i=0;i<App::currentWorld->sceneObjects->getPathCount();i++)
        {
            CPath_old* it=App::currentWorld->sceneObjects->getPathFromIndex(i);
            std::string name(tt::decorateString("[",IDSN_PATH,"] "));
            name+=it->getObjectAlias_shortPath();
            names.push_back(name);
            ids.push_back(it->getObjectHandle());
        }
        tt::orderStrings(names,ids);
        for (int i=0;i<int(names.size());i++)
            ui->qqEntityCombo->addItem(names[i].c_str(),QVariant(ids[i]));

        names.clear();
        ids.clear();

        // Now graphs:
        for (size_t i=0;i<App::currentWorld->sceneObjects->getGraphCount();i++)
        {
            CGraph* it=App::currentWorld->sceneObjects->getGraphFromIndex(i);
            std::string name(tt::decorateString("[",IDSN_GRAPH,"] "));
            name+=it->getObjectAlias_shortPath();
            names.push_back(name);
            ids.push_back(it->getObjectHandle());
        }
        tt::orderStrings(names,ids);
        for (int i=0;i<int(names.size());i++)
            ui->qqEntityCombo->addItem(names[i].c_str(),QVariant(ids[i]));

        // Select current item:
        for (int i=0;i<ui->qqEntityCombo->count();i++)
        {
            if (ui->qqEntityCombo->itemData(i).toInt()==it->getDetectableEntityHandle())
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
        App::appendSimulationThreadCommand(TOGGLE_EXPLICITHANDLING_VISIONSENSORGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgVisionSensors::on_qqExternalInput_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_EXTERNALINPUT_VISIONSENSORGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgVisionSensors::on_qqPerspective_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_PERSPECTIVE_VISIONSENSORGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgVisionSensors::on_qqLocalLights_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_LOCALLIGHTS_VISIONSENSORGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgVisionSensors::on_qqShowNotDetecting_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_SHOWVOLUME_WHEN_NOT_DETECTING_VISIONSENSORGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgVisionSensors::on_qqShowFog_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_SHOWFOG_VISIONSENSORGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgVisionSensors::on_qqShowDetecting_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_SHOWVOLUME_WHEN_DETECTING_VISIONSENSORGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle());
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
            App::appendSimulationThreadCommand(SET_NEARCLIPPING_VISIONSENSORGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle(),-1,newVal);
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
            App::appendSimulationThreadCommand(SET_FARCLIPPING_VISIONSENSORGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle(),-1,newVal);
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
            CVisionSensor* it=App::currentWorld->sceneObjects->getLastSelectionVisionSensor();
            if ((it!=nullptr)&&it->getPerspectiveOperation())
                newVal*=gv::userToRad;
            App::appendSimulationThreadCommand(SET_PERSPECTANGLE_OR_ORTHOSIZE_VISIONSENSORGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle(),-1,newVal);
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
        CVisionSensor* it=App::currentWorld->sceneObjects->getLastSelectionVisionSensor();
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
                        App::uiThread->messageBox_warning(App::mainWindow,"Vision sensor",IDS_VISION_SENSOR_RESOLUTION_NOT_POWER_OF_TWO_WARNING,VMESSAGEBOX_OKELI,VMESSAGEBOX_REPLY_OK);
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
        CVisionSensor* it=App::currentWorld->sceneObjects->getLastSelectionVisionSensor();
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
                        App::uiThread->messageBox_warning(App::mainWindow,"Vision sensor",IDS_VISION_SENSOR_RESOLUTION_NOT_POWER_OF_TWO_WARNING,VMESSAGEBOX_OKELI,VMESSAGEBOX_REPLY_OK);
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
        CVisionSensor* it=App::currentWorld->sceneObjects->getLastSelectionVisionSensor();
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
        CVisionSensor* it=App::currentWorld->sceneObjects->getLastSelectionVisionSensor();
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
        CVisionSensor* it=App::currentWorld->sceneObjects->getLastSelectionVisionSensor();
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
        CVisionSensor* it=App::currentWorld->sceneObjects->getLastSelectionVisionSensor();
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
        CVisionSensor* last=App::currentWorld->sceneObjects->getLastSelectionVisionSensor();
        if (last!=nullptr)
        {
            SSimulationThreadCommand cmd;
            cmd.cmdId=APPLY_MAINPROP_TO_SELECTION_VISIONSENSORGUITRIGGEREDCMD;
            cmd.intParams.push_back(last->getObjectHandle());
            for (size_t i=0;i<App::currentWorld->sceneObjects->getSelectionCount()-1;i++)
            {
                CVisionSensor* it=App::currentWorld->sceneObjects->getVisionSensorFromHandle(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
                if (it!=nullptr)
                    cmd.intParams.push_back(it->getObjectHandle());
            }
            App::appendSimulationThreadCommand(cmd);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgVisionSensors::on_qqCasingColorPassive_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CQDlgMaterial::displayMaterialDlg(COLOR_ID_VISIONSENSOR_PASSIVE,App::currentWorld->sceneObjects->getLastSelectionHandle(),-1,App::mainWindow);
    }
}

void CQDlgVisionSensors::on_qqCasingColorActive_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CQDlgMaterial::displayMaterialDlg(COLOR_ID_VISIONSENSOR_ACTIVE,App::currentWorld->sceneObjects->getLastSelectionHandle(),-1,App::mainWindow);
    }
}

void CQDlgVisionSensors::on_qqApplyColors_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CVisionSensor* last=App::currentWorld->sceneObjects->getLastSelectionVisionSensor();
        if (last!=nullptr)
        {
            SSimulationThreadCommand cmd;
            cmd.cmdId=APPLY_VISUALPROP_TO_SELECTION_VISIONSENSORGUITRIGGEREDCMD;
            cmd.intParams.push_back(last->getObjectHandle());
            for (size_t i=0;i<App::currentWorld->sceneObjects->getSelectionCount()-1;i++)
            {
                CVisionSensor* it=App::currentWorld->sceneObjects->getVisionSensorFromHandle(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
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
            CVisionSensor* it=App::currentWorld->sceneObjects->getLastSelectionVisionSensor();
            if ( (it!=nullptr)&&(objID!=-1) )
            {
                bool displayWarning=false;
                if ((objID<SIM_IDSTART_COLLECTION)&&(objID>=0))
                {
                    CSceneObject* it2=App::currentWorld->sceneObjects->getObjectFromHandle(objID);
                    if (it2!=nullptr)
                        displayWarning|=((it2->getLocalObjectSpecialProperty()&sim_objectspecialproperty_renderable)==0);
                }
                if (displayWarning)
                    App::uiThread->messageBox_warning(App::mainWindow,"Scene object",IDS_OBJECT_NOT_RENDERABLE_WARNING,VMESSAGEBOX_OKELI,VMESSAGEBOX_REPLY_OK);
            }
            App::appendSimulationThreadCommand(SET_ENTITYTODETECT_VISIONSENSORGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle(),objID);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgVisionSensors::on_qqIgnoreRGB_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_IGNORERGB_VISIONSENSORGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgVisionSensors::on_qqIgnoreDepth_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_IGNOREDEPTH_VISIONSENSORGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgVisionSensors::on_qqIgnorePacket1_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_PACKET1BLANK_VISIONSENSORGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle());
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
            App::appendSimulationThreadCommand(SET_RENDERMODE_VISIONSENSORGUITRIGGEREDCMD,App::currentWorld->sceneObjects->getLastSelectionHandle(),renderMode);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}
