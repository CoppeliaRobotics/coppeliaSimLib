#include <qdlgsettings.h>
#include <ui_qdlgsettings.h>
#include <tt.h>
#include <utils.h>
#include <simStrings.h>
#include <vFileDialog.h>
#include <qdlgopenglsettings.h>
#include <app.h>
#include <vMessageBox.h>
#ifdef SIM_WITH_GUI
#include <guiApp.h>
#endif

CQDlgSettings::CQDlgSettings(QWidget* parent)
    : CDlgEx(parent), ui(new Ui::CQDlgSettings)
{
    _dlgType = SETTINGS_DLG;
    ui->setupUi(this);
}

CQDlgSettings::~CQDlgSettings()
{
    delete ui;
}

void CQDlgSettings::refresh()
{
    bool noEditMode = (GuiApp::getEditModeType() == NO_EDIT_MODE);
    bool noSim = App::currentWorld->simulation->isSimulationStopped();
    bool noEditModeAndNoSim = noEditMode && noSim;

#ifndef WIN_SIM
    ui->hideConsole->setVisible(false);
#endif

    ui->qqHideHierarchy->setEnabled(noSim);
    ui->undoRedo->setEnabled(noEditModeAndNoSim);
    ui->qqAutoSave->setEnabled(noEditModeAndNoSim);
    ui->qqAdjustOpenGl->setEnabled(noEditModeAndNoSim);

    ui->translationStepSize->clear();
    ui->translationStepSize->addItem(utils::getSizeString(false, 0.001).c_str(), QVariant(1));
    ui->translationStepSize->addItem(utils::getSizeString(false, 0.002).c_str(), QVariant(2));
    ui->translationStepSize->addItem(utils::getSizeString(false, 0.005).c_str(), QVariant(5));
    ui->translationStepSize->addItem(utils::getSizeString(false, 0.01).c_str(), QVariant(10));
    ui->translationStepSize->addItem(utils::getSizeString(false, 0.025).c_str(), QVariant(25));
    ui->translationStepSize->addItem(utils::getSizeString(false, 0.05).c_str(), QVariant(50));
    ui->translationStepSize->addItem(utils::getSizeString(false, 0.1).c_str(), QVariant(100));
    ui->translationStepSize->addItem(utils::getSizeString(false, 0.25).c_str(), QVariant(250));
    ui->translationStepSize->addItem(utils::getSizeString(false, 0.5).c_str(), QVariant(500));
    _selectItemOfCombobox(ui->translationStepSize,
                          int((App::userSettings->getTranslationStepSize() + 0.0005) * 1000.0));

    ui->rotationStepSize->clear();
    ui->rotationStepSize->addItem(utils::getAngleString(false, 1.0 * degToRad).c_str(), QVariant(1));
    ui->rotationStepSize->addItem(utils::getAngleString(false, 2.0 * degToRad).c_str(), QVariant(2));
    ui->rotationStepSize->addItem(utils::getAngleString(false, 5.0 * degToRad).c_str(), QVariant(5));
    ui->rotationStepSize->addItem(utils::getAngleString(false, 10.0 * degToRad).c_str(), QVariant(10));
    ui->rotationStepSize->addItem(utils::getAngleString(false, 15.0 * degToRad).c_str(), QVariant(15));
    ui->rotationStepSize->addItem(utils::getAngleString(false, 30.0 * degToRad).c_str(), QVariant(30));
    ui->rotationStepSize->addItem(utils::getAngleString(false, 45.0 * degToRad).c_str(), QVariant(45));
    _selectItemOfCombobox(ui->rotationStepSize, int(App::userSettings->getRotationStepSize() * radToDeg + 0.5));

    ui->qqHideHierarchy->setChecked(App::userSettings->sceneHierarchyHiddenDuringSimulation);
    ui->worldReference->setChecked(App::userSettings->displayWorldReference);
    ui->undoRedo->setChecked(App::userSettings->getUndoRedoEnabled());
    ui->hideConsole->setChecked(!App::userSettings->alwaysShowConsole);
    ui->qqAutoSave->setChecked(App::userSettings->autoSaveDelay != 0);

    ui->qqAdjustOpenGl->setEnabled(App::currentWorld->simulation->isSimulationStopped());
}

void CQDlgSettings::on_translationStepSize_activated(int index)
{
    App::appendSimulationThreadCommand(SET_TRANSLATIONSTEPSIZE_USERSETTINGSGUITRIGGEREDCMD, -1, -1,
                                       double(ui->translationStepSize->itemData(index).toInt()) / 1000.0);
    App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
}

void CQDlgSettings::on_rotationStepSize_activated(int index)
{
    App::appendSimulationThreadCommand(SET_ROTATIONSTEPSIZE_USERSETTINGSGUITRIGGEREDCMD, -1, -1,
                                       double(ui->rotationStepSize->itemData(index).toInt()) * degToRad);
    App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
}

void CQDlgSettings::on_worldReference_clicked()
{
    App::appendSimulationThreadCommand(TOGGLE_SHOWWORLDREF_USERSETTINGSGUITRIGGEREDCMD);
    App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
}

void CQDlgSettings::on_undoRedo_clicked()
{
    App::appendSimulationThreadCommand(TOGGLE_UNDOREDO_USERSETTINGSGUITRIGGEREDCMD);
    App::appendSimulationThreadCommand(REFRESH_TOOLBARS_GUITRIGGEREDCMD);
    App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
}

void CQDlgSettings::on_hideConsole_clicked()
{
    App::appendSimulationThreadCommand(TOGGLE_HIDECONSOLE_USERSETTINGSGUITRIGGEREDCMD);
    App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
}

void CQDlgSettings::on_qqAutoSave_clicked()
{
    App::appendSimulationThreadCommand(TOGGLE_AUTOSAVE_USERSETTINGSGUITRIGGEREDCMD);
    App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
}

void CQDlgSettings::on_qqAdjustOpenGl_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CQDlgOpenGlSettings theDialog(this);

        theDialog.offscreenContextType = App::userSettings->offscreenContextType;
        theDialog.fboType = App::userSettings->fboType;
        theDialog.vboOperation = App::userSettings->vboOperation;
        //        theDialog.exclusiveGuiRendering=(App::userSettings->visionSensorsUseGuiThread_windowed!=0);
        //        theDialog.compatibilityTweak1=App::userSettings->oglCompatibilityTweak1;
        //        theDialog.glFinish_normal=App::userSettings->useGlFinish;
        //        theDialog.glFinish_visionSensors=App::userSettings->useGlFinish_visionSensors;
        theDialog.idleFps = App::userSettings->getIdleFps();
        //        theDialog.forceExt=App::userSettings->forceFboViaExt;
        //        theDialog.glVersionMajor=App::userSettings->desiredOpenGlMajor;
        //        theDialog.glVersionMinor=App::userSettings->desiredOpenGlMinor;

        theDialog.refresh();
        if (theDialog.makeDialogModal() != VDIALOG_MODAL_RETURN_CANCEL)
        {
            SSimulationThreadCommand cmd;
            cmd.cmdId = SET_OPENGLSETTINGS_USERSETTINGSGUITRIGGEREDCMD;
            cmd.intParams.push_back(theDialog.offscreenContextType);
            cmd.intParams.push_back(theDialog.fboType);
            cmd.intParams.push_back(theDialog.vboOperation);

            cmd.intParams.push_back(false);
            cmd.boolParams.push_back(App::userSettings->oglCompatibilityTweak1);
            cmd.boolParams.push_back(App::userSettings->useGlFinish);
            cmd.boolParams.push_back(App::userSettings->useGlFinish_visionSensors);
            cmd.intParams.push_back(theDialog.idleFps);
            cmd.boolParams.push_back(App::userSettings->forceFboViaExt);

            //            int tmp=-1;
            //            if (!theDialog.exclusiveGuiRendering)
            //                tmp=0;
            //            cmd.intParams.push_back(tmp);
            //            cmd.boolParams.push_back(theDialog.compatibilityTweak1);
            //            cmd.boolParams.push_back(theDialog.glFinish_normal);
            //            cmd.boolParams.push_back(theDialog.glFinish_visionSensors);
            //            cmd.intParams.push_back(theDialog.idleFps);
            //            cmd.boolParams.push_back(theDialog.forceExt);
            //            cmd.intParams.push_back(theDialog.glVersionMajor);
            //            cmd.intParams.push_back(theDialog.glVersionMinor);
            App::appendSimulationThreadCommand(cmd);
        }
    }
}

void CQDlgSettings::_selectItemOfCombobox(QComboBox* theBox, int itemData)
{
    theBox->setCurrentIndex(_getIndexOfComboboxItemWithData(theBox, itemData));
}

int CQDlgSettings::_getIndexOfComboboxItemWithData(QComboBox* theBox, int itemData)
{
    for (int i = 0; i < theBox->count(); i++)
    {
        if (theBox->itemData(i).toInt() == itemData)
            return (i);
    }
    return (-1);
}

void CQDlgSettings::on_qqHideHierarchy_clicked()
{
    App::appendSimulationThreadCommand(TOGGLE_HIDEHIERARCHY_USERSETTINGSGUITRIGGEREDCMD);
    App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
}
