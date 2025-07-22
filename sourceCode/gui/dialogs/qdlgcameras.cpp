#include <qdlgcameras.h>
#include <ui_qdlgcameras.h>
#include <tt.h>
#include <utils.h>
#include <qdlgmaterial.h>
#include <app.h>
#include <simStrings.h>
#include <guiApp.h>

CQDlgCameras::CQDlgCameras(QWidget* parent)
    : CDlgEx(parent), ui(new Ui::CQDlgCameras)
{
    _dlgType = CAMERA_DLG;
    ui->setupUi(this);
    inMainRefreshRoutine = false;
}

CQDlgCameras::~CQDlgCameras()
{
    delete ui;
}

void CQDlgCameras::cancelEvent()
{
    // we override this cancel event. The container window should close, not this one!!
    GuiApp::mainWindow->dlgCont->close(OBJECT_DLG);
}

void CQDlgCameras::refresh()
{
    inMainRefreshRoutine = true;
    QLineEdit* lineEditToSelect = getSelectedLineEdit();

    bool noEditModeNoSim =
        (GuiApp::getEditModeType() == NO_EDIT_MODE) && App::currentWorld->simulation->isSimulationStopped();
    CCamera* it = App::currentWorld->sceneObjects->getLastSelectionCamera();

    ui->qqAllowRotation->setEnabled((it != nullptr) && noEditModeNoSim);
    ui->qqColorA->setEnabled((it != nullptr) && noEditModeNoSim);
    ui->qqFarClipping->setEnabled((it != nullptr) && noEditModeNoSim);
    ui->qqNearClipping->setEnabled((it != nullptr) && noEditModeNoSim);
    ui->qqAllowRotation->setEnabled((it != nullptr) && noEditModeNoSim);
    ui->qqShowVolume->setEnabled((it != nullptr) && noEditModeNoSim);
    ui->qqLocalLights->setEnabled((it != nullptr) && noEditModeNoSim);
    ui->qqLocalLights->setVisible(App::userSettings->showOldDlgs);
    ui->qqAllowPicking->setEnabled((it != nullptr) && noEditModeNoSim);
    ui->qqAllowPicking->setVisible(App::userSettings->showOldDlgs);
    ui->qqManipProxy->setEnabled((it != nullptr) && noEditModeNoSim);
    ui->qqPerspectiveProjectionAngle->setEnabled((it != nullptr) && noEditModeNoSim);
    ui->qqOrthographicProjectionSize->setEnabled((it != nullptr) && noEditModeNoSim);
    ui->qqShowFog->setEnabled((it != nullptr) && noEditModeNoSim);
    ui->qqShowFog->setVisible(App::userSettings->showOldDlgs);
    ui->qqSize->setEnabled((it != nullptr) && noEditModeNoSim);
    ui->qqTrackedCombo->setEnabled((it != nullptr) && noEditModeNoSim);
    ui->qqRenderModeCombo->setEnabled((it != nullptr) && noEditModeNoSim);

    ui->qqTrackedCombo->clear();
    ui->qqRenderModeCombo->clear();
    ui->qqCameraType->setText("");

    if (it != nullptr)
    {
        if (it->getPerspectiveOperation_old() == 0)
            ui->qqCameraType->setText("Orthographic projection mode camera");
        if (it->getPerspectiveOperation_old() == 1)
            ui->qqCameraType->setText("Perspective projection mode camera");
        ui->qqManipProxy->setChecked(it->getUseParentObjectAsManipulationProxy());
        ui->qqShowVolume->setChecked(it->getShowVolume());
        ui->qqAllowRotation->setChecked(it->getAllowRotation());
        ui->qqAllowTranslation->setChecked(it->getAllowTranslation());
        ui->qqPerspectiveProjectionAngle->setText(utils::getAngleString(false, it->getViewAngle()).c_str());
        ui->qqOrthographicProjectionSize->setText(utils::getSizeString(false, it->getOrthoViewSize()).c_str());
        ui->qqSize->setText(utils::getSizeString(false, it->getCameraSize()).c_str());
        double np, fp;
        it->getClippingPlanes(np, fp);
        ui->qqNearClipping->setText(utils::getSizeString(false, np).c_str());
        ui->qqFarClipping->setText(utils::getSizeString(false, fp).c_str());
        ui->qqShowFog->setChecked(!it->getHideFog());
        ui->qqLocalLights->setChecked(it->getuseLocalLights());
        ui->qqAllowPicking->setChecked(it->getAllowPicking());

        ui->qqTrackedCombo->addItem(IDSN_NONE, QVariant(-1));
        std::vector<std::string> names;
        std::vector<int> ids;
        for (size_t i = 0; i < App::currentWorld->sceneObjects->getObjectCount(); i++)
        {
            CSceneObject* it2 = App::currentWorld->sceneObjects->getObjectFromIndex(i);
            if (it2 != it)
            {
                names.push_back(it2->getObjectAlias_printPath());
                ids.push_back(it2->getObjectHandle());
            }
        }
        tt::orderStrings(names, ids);
        for (int i = 0; i < int(names.size()); i++)
            ui->qqTrackedCombo->addItem(names[i].c_str(), QVariant(ids[i]));
        for (int i = 0; i < ui->qqTrackedCombo->count(); i++)
        {
            if (ui->qqTrackedCombo->itemData(i).toInt() == it->getTrackedObjectHandle())
            {
                ui->qqTrackedCombo->setCurrentIndex(i);
                break;
            }
        }

        bool duringSim, duringRec;
        int renderMode = it->getRenderMode(&duringSim, &duringRec);
        ui->qqRenderModeCombo->addItem("Legacy OpenGL", QVariant(0));
        if (renderMode == sim_rendermode_extrenderer)
        {
            ui->qqRenderModeCombo->addItem("External renderer (deprecated)", QVariant(1));
            ui->qqRenderModeCombo->addItem("External renderer during simulation (deprecated)", QVariant(2));
            ui->qqRenderModeCombo->addItem("External renderer during simulation and recording (deprecated)",
                                           QVariant(3));
        }
        ui->qqRenderModeCombo->addItem("OpenGL 3", QVariant(4));
        ui->qqRenderModeCombo->addItem("OpenGL 3 (simulation)", QVariant(5));
        ui->qqRenderModeCombo->addItem("OpenGL 3 (simulation & video recording)", QVariant(6));
        ui->qqRenderModeCombo->addItem("POV-Ray (simulation)", QVariant(7));
        ui->qqRenderModeCombo->addItem("POV-Ray (simulation & video recording)", QVariant(8));

        // Select current item:
        for (int i = 0; i < ui->qqRenderModeCombo->count(); i++)
        {
            bool ok = false;
            if (renderMode == sim_rendermode_opengl)
                ok = (ui->qqRenderModeCombo->itemData(i).toInt() == 0);
            if ((renderMode == sim_rendermode_extrenderer) && (!duringSim) && (!duringRec))
                ok = (ui->qqRenderModeCombo->itemData(i).toInt() == 1);
            if ((renderMode == sim_rendermode_extrenderer) && duringSim && (!duringRec))
                ok = (ui->qqRenderModeCombo->itemData(i).toInt() == 2);
            if ((renderMode == sim_rendermode_extrenderer) && duringSim && duringRec)
                ok = (ui->qqRenderModeCombo->itemData(i).toInt() == 3);
            if ((renderMode == sim_rendermode_opengl3) && (!duringSim) && (!duringRec))
                ok = (ui->qqRenderModeCombo->itemData(i).toInt() == 4);
            if ((renderMode == sim_rendermode_opengl3) && duringSim && (!duringRec))
                ok = (ui->qqRenderModeCombo->itemData(i).toInt() == 5);
            if ((renderMode == sim_rendermode_opengl3) && duringSim && duringRec)
                ok = (ui->qqRenderModeCombo->itemData(i).toInt() == 6);
            if ((renderMode == sim_rendermode_povray) && duringSim && (!duringRec))
                ok = (ui->qqRenderModeCombo->itemData(i).toInt() == 7);
            if ((renderMode == sim_rendermode_povray) && duringSim && duringRec)
                ok = (ui->qqRenderModeCombo->itemData(i).toInt() == 8);
            if (ok)
            {
                ui->qqRenderModeCombo->setCurrentIndex(i);
                break;
            }
        }
    }
    else
    {
        ui->qqAllowTranslation->setChecked(false);
        ui->qqAllowRotation->setChecked(false);
        ui->qqShowVolume->setChecked(false);
        ui->qqLocalLights->setChecked(false);
        ui->qqManipProxy->setChecked(false);
        ui->qqShowFog->setChecked(false);
        ui->qqAllowPicking->setChecked(false);
        ui->qqFarClipping->setText("");
        ui->qqNearClipping->setText("");
        ui->qqPerspectiveProjectionAngle->setText("");
        ui->qqOrthographicProjectionSize->setText("");
        ui->qqSize->setText("");
    }

    selectLineEdit(lineEditToSelect);
    inMainRefreshRoutine = false;
}

void CQDlgCameras::on_qqPerspectiveProjectionAngle_editingFinished()
{
    if (!ui->qqPerspectiveProjectionAngle->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        double newVal = GuiApp::getEvalDouble(ui->qqPerspectiveProjectionAngle->text().toStdString().c_str(), &ok);
        if (ok)
        {
            double v = newVal * degToRad;
            App::appendSimulationThreadCommand(SET_VIEW_ANGLE_CAMERAGUITRIGGEREDCMD,
                                               App::currentWorld->sceneObjects->getLastSelectionHandle(), -1, v);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgCameras::on_qqOrthographicProjectionSize_editingFinished()
{
    if (!ui->qqOrthographicProjectionSize->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        double newVal = GuiApp::getEvalDouble(ui->qqOrthographicProjectionSize->text().toStdString().c_str(), &ok);
        if (ok)
        {
            App::appendSimulationThreadCommand(SET_ORTHO_VIEW_SIZE_CAMERAGUITRIGGEREDCMD,
                                               App::currentWorld->sceneObjects->getLastSelectionHandle(), -1, newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgCameras::on_qqNearClipping_editingFinished()
{
    if (!ui->qqNearClipping->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        double newVal = GuiApp::getEvalDouble(ui->qqNearClipping->text().toStdString().c_str(), &ok);
        if (ok)
        {
            App::appendSimulationThreadCommand(SET_NEAR_CLIPPING_CAMERAGUITRIGGEREDCMD,
                                               App::currentWorld->sceneObjects->getLastSelectionHandle(), -1, newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgCameras::on_qqFarClipping_editingFinished()
{
    if (!ui->qqFarClipping->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        double newVal = GuiApp::getEvalDouble(ui->qqFarClipping->text().toStdString().c_str(), &ok);
        if (ok)
        {
            App::appendSimulationThreadCommand(SET_FAR_CLIPPING_CAMERAGUITRIGGEREDCMD,
                                               App::currentWorld->sceneObjects->getLastSelectionHandle(), -1, newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgCameras::on_qqTrackedCombo_currentIndexChanged(int index)
{
    if (!inMainRefreshRoutine)
    {
        IF_UI_EVENT_CAN_WRITE_DATA
        {
            int objID = ui->qqTrackedCombo->itemData(ui->qqTrackedCombo->currentIndex()).toInt();
            App::appendSimulationThreadCommand(SET_TRACKED_OBJECT_CAMERAGUITRIGGEREDCMD,
                                               App::currentWorld->sceneObjects->getLastSelectionHandle(), objID);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgCameras::on_qqShowFog_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_SHOWFOG_CAMERAGUITRIGGEREDCMD,
                                           App::currentWorld->sceneObjects->getLastSelectionHandle());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgCameras::on_qqManipProxy_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_USEPARENTASMANIPPROXY_CAMERAGUITRIGGEREDCMD,
                                           App::currentWorld->sceneObjects->getLastSelectionHandle());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgCameras::on_qqAllowTranslation_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_ALLOWTRANSLATION_CAMERAGUITRIGGEREDCMD,
                                           App::currentWorld->sceneObjects->getLastSelectionHandle());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgCameras::on_qqAllowRotation_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_ALLOWROTATION_CAMERAGUITRIGGEREDCMD,
                                           App::currentWorld->sceneObjects->getLastSelectionHandle());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgCameras::on_qqColorA_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CQDlgMaterial::displayMaterialDlg(COLOR_ID_CAMERA_A, App::currentWorld->sceneObjects->getLastSelectionHandle(),
                                          -1, GuiApp::mainWindow);
    }
}

void CQDlgCameras::on_qqSize_editingFinished()
{
    if (!ui->qqSize->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        double newVal = GuiApp::getEvalDouble(ui->qqSize->text().toStdString().c_str(), &ok);
        if (ok)
        {
            App::appendSimulationThreadCommand(SET_SIZE_CAMERAGUITRIGGEREDCMD,
                                               App::currentWorld->sceneObjects->getLastSelectionHandle(), -1, newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgCameras::on_qqLocalLights_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_LOCALLIGHTS_CAMERAGUITRIGGEREDCMD,
                                           App::currentWorld->sceneObjects->getLastSelectionHandle());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgCameras::on_qqAllowPicking_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_ALLOWPICKING_CAMERAGUITRIGGEREDCMD,
                                           App::currentWorld->sceneObjects->getLastSelectionHandle());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgCameras::on_qqRenderModeCombo_currentIndexChanged(int index)
{
    if (!inMainRefreshRoutine)
    {
        IF_UI_EVENT_CAN_WRITE_DATA
        {
            int index = ui->qqRenderModeCombo->itemData(ui->qqRenderModeCombo->currentIndex()).toInt();

            bool duringSim = false;
            bool duringRec = false;
            int renderMode = sim_rendermode_opengl;
            if ((index >= 1) && (index <= 3))
            {
                renderMode = sim_rendermode_extrenderer;
                duringSim = (index >= 2);
                duringRec = (index >= 3);
            }
            if ((index >= 4) && (index <= 6))
            {
                renderMode = sim_rendermode_opengl3;
                duringSim = (index >= 5);
                duringRec = (index >= 6);
            }
            if ((index >= 7) && (index <= 8))
            {
                renderMode = sim_rendermode_povray;
                duringSim = (index >= 7);
                duringRec = (index >= 8);
            }
            SSimulationThreadCommand cmd;
            cmd.cmdId = SET_RENDERMODE_CAMERAGUITRIGGEREDCMD;
            cmd.intParams.push_back(App::currentWorld->sceneObjects->getLastSelectionHandle());
            cmd.intParams.push_back(renderMode);
            cmd.boolParams.push_back(duringSim);
            cmd.boolParams.push_back(duringRec);
            App::appendSimulationThreadCommand(cmd);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgCameras::on_qqShowVolume_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_SHOWVOLUME_CAMERAGUITRIGGEREDCMD,
                                           App::currentWorld->sceneObjects->getLastSelectionHandle());
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}
