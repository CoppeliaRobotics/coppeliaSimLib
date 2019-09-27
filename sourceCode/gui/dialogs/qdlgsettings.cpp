
#include "vrepMainHeader.h"
#include "qdlgsettings.h"
#include "ui_qdlgsettings.h"
#include "tt.h"
#include "gV.h"
#include "v_repStrings.h"
#include "vFileDialog.h"
#include "qdlgopenglsettings.h"
#include "app.h"
#include "vMessageBox.h"

CQDlgSettings::CQDlgSettings(QWidget *parent) :
    CDlgEx(parent),
    ui(new Ui::CQDlgSettings)
{
    _dlgType=SETTINGS_DLG;
    ui->setupUi(this);
}

CQDlgSettings::~CQDlgSettings()
{
    delete ui;
}

void CQDlgSettings::refresh()
{
    bool noEditMode=(App::getEditModeType()==NO_EDIT_MODE);
    bool noSim=App::ct->simulation->isSimulationStopped();
    bool noEditModeAndNoSim=noEditMode&&noSim;

#ifndef WIN_VREP
    ui->hideConsole->setVisible(false);
#endif

    ui->removeIdenticalVerticesCheckbox->setEnabled(noSim);
    ui->removeIdenticalVerticesTolerance->setEnabled(noSim);
    ui->removeIdenticalTriangles->setEnabled(noSim);
    ui->ignoreTriangleWinding->setEnabled(App::userSettings->identicalTrianglesCheck&&noSim);
    ui->qqHideHierarchy->setEnabled(noSim);
    ui->undoRedo->setEnabled(noEditModeAndNoSim);
    ui->qqAutoSave->setEnabled(noEditModeAndNoSim);
    ui->qqAdjustOpenGl->setEnabled(noEditModeAndNoSim);

    ui->translationStepSize->clear();
    ui->translationStepSize->addItem(tt::getFString(false,0.001f,3).c_str(),QVariant(1));
    ui->translationStepSize->addItem(tt::getFString(false,0.002f,3).c_str(),QVariant(2));
    ui->translationStepSize->addItem(tt::getFString(false,0.005f,3).c_str(),QVariant(5));
    ui->translationStepSize->addItem(tt::getFString(false,0.01f,3).c_str(),QVariant(10));
    ui->translationStepSize->addItem(tt::getFString(false,0.025f,3).c_str(),QVariant(25));
    ui->translationStepSize->addItem(tt::getFString(false,0.05f,3).c_str(),QVariant(50));
    ui->translationStepSize->addItem(tt::getFString(false,0.1f,3).c_str(),QVariant(100));
    ui->translationStepSize->addItem(tt::getFString(false,0.25f,3).c_str(),QVariant(250));
    ui->translationStepSize->addItem(tt::getFString(false,0.5f,3).c_str(),QVariant(500));
    _selectItemOfCombobox(ui->translationStepSize,int((App::userSettings->getTranslationStepSize()+0.0005f)*1000.0f));

    ui->rotationStepSize->clear();
    ui->rotationStepSize->addItem(tt::getAngleFString(false,1.0f*degToRad_f,1).c_str(),QVariant(1));
    ui->rotationStepSize->addItem(tt::getAngleFString(false,2.0f*degToRad_f,1).c_str(),QVariant(2));
    ui->rotationStepSize->addItem(tt::getAngleFString(false,5.0f*degToRad_f,1).c_str(),QVariant(5));
    ui->rotationStepSize->addItem(tt::getAngleFString(false,10.0f*degToRad_f,1).c_str(),QVariant(10));
    ui->rotationStepSize->addItem(tt::getAngleFString(false,15.0f*degToRad_f,1).c_str(),QVariant(15));
    ui->rotationStepSize->addItem(tt::getAngleFString(false,30.0f*degToRad_f,1).c_str(),QVariant(30));
    ui->rotationStepSize->addItem(tt::getAngleFString(false,45.0f*degToRad_f,1).c_str(),QVariant(45));
    _selectItemOfCombobox(ui->rotationStepSize,int(App::userSettings->getRotationStepSize()*radToDeg_f+0.5f));

    ui->removeIdenticalVerticesCheckbox->setChecked(App::userSettings->identicalVerticesCheck);
    ui->removeIdenticalVerticesTolerance->setText(tt::getEString(false,App::userSettings->identicalVerticesTolerance,2).c_str());

    ui->removeIdenticalTriangles->setChecked(App::userSettings->identicalTrianglesCheck);
    ui->ignoreTriangleWinding->setChecked(App::userSettings->identicalTrianglesWindingCheck);



    ui->qqHideHierarchy->setChecked(App::userSettings->sceneHierarchyHiddenDuringSimulation);
    ui->worldReference->setChecked(App::userSettings->displayWorldReference);
    ui->boundingBoxDisplay->setChecked(App::userSettings->displayBoundingBoxeWhenObjectSelected);
    ui->undoRedo->setChecked(App::userSettings->getUndoRedoEnabled());
    ui->hideConsole->setChecked(!App::userSettings->alwaysShowConsole);
    ui->qqAutoSave->setChecked(App::userSettings->autoSaveDelay!=0);

    ui->qqAdjustOpenGl->setEnabled(App::ct->simulation->isSimulationStopped());
}

void CQDlgSettings::on_translationStepSize_activated(int index)
{
    App::appendSimulationThreadCommand(SET_TRANSLATIONSTEPSIZE_USERSETTINGSGUITRIGGEREDCMD,-1,-1,float(ui->translationStepSize->itemData(index).toInt())/1000.0f);
    App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
}

void CQDlgSettings::on_rotationStepSize_activated(int index)
{
    App::appendSimulationThreadCommand(SET_ROTATIONSTEPSIZE_USERSETTINGSGUITRIGGEREDCMD,-1,-1,float(ui->rotationStepSize->itemData(index).toInt())*degToRad_f);
    App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
}

void CQDlgSettings::on_removeIdenticalVerticesCheckbox_clicked()
{
    App::appendSimulationThreadCommand(TOGGLE_REMOVEIDENTICALVERTICES_USERSETTINGSGUITRIGGEREDCMD);
    App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
}

void CQDlgSettings::on_removeIdenticalVerticesTolerance_editingFinished()
{
    if (!ui->removeIdenticalVerticesTolerance->isModified())
        return;
    float newVal;
    bool ok;
    newVal=ui->removeIdenticalVerticesTolerance->text().toFloat(&ok);
    if (ok)
    {
        if (newVal>=0.0f)
        {
            if (newVal>0.5f)
                newVal=0.5f;
            App::appendSimulationThreadCommand(SET_IDENTICALVERTICESTOLERANCE_USERSETTINGSGUITRIGGEREDCMD,-1,-1,newVal);
        }
    }
    App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
}

void CQDlgSettings::on_removeIdenticalTriangles_clicked()
{
    App::appendSimulationThreadCommand(TOGGLE_REMOVEIDENTICALTRIANGLES_USERSETTINGSGUITRIGGEREDCMD);
    App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
}

void CQDlgSettings::on_ignoreTriangleWinding_clicked()
{
    App::appendSimulationThreadCommand(TOGGLE_IGNORETRIANGLEWINDING_USERSETTINGSGUITRIGGEREDCMD);
    App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
}


void CQDlgSettings::on_worldReference_clicked()
{
    App::appendSimulationThreadCommand(TOGGLE_SHOWWORLDREF_USERSETTINGSGUITRIGGEREDCMD);
    App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
}

void CQDlgSettings::on_boundingBoxDisplay_clicked()
{
    App::appendSimulationThreadCommand(TOGGLE_SHOWBOUNDINGBOX_USERSETTINGSGUITRIGGEREDCMD);
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

        theDialog.offscreenContextType=App::userSettings->offscreenContextType;
        theDialog.fboType=App::userSettings->fboType;
        theDialog.vboOperation=App::userSettings->vboOperation;
//        theDialog.exclusiveGuiRendering=(App::userSettings->visionSensorsUseGuiThread_windowed!=0);
//        theDialog.compatibilityTweak1=App::userSettings->oglCompatibilityTweak1;
//        theDialog.glFinish_normal=App::userSettings->useGlFinish;
//        theDialog.glFinish_visionSensors=App::userSettings->useGlFinish_visionSensors;
        theDialog.idleFps=App::userSettings->getIdleFps();
//        theDialog.forceExt=App::userSettings->forceFboViaExt;
//        theDialog.glVersionMajor=App::userSettings->desiredOpenGlMajor;
//        theDialog.glVersionMinor=App::userSettings->desiredOpenGlMinor;

        theDialog.refresh();
        if (theDialog.makeDialogModal()!=VDIALOG_MODAL_RETURN_CANCEL)
        {
            SSimulationThreadCommand cmd;
            cmd.cmdId=SET_OPENGLSETTINGS_USERSETTINGSGUITRIGGEREDCMD;
            cmd.intParams.push_back(theDialog.offscreenContextType);
            cmd.intParams.push_back(theDialog.fboType);
            cmd.intParams.push_back(theDialog.vboOperation);

            cmd.intParams.push_back(App::userSettings->visionSensorsUseGuiThread_windowed);
            cmd.boolParams.push_back(App::userSettings->oglCompatibilityTweak1);
            cmd.boolParams.push_back(App::userSettings->useGlFinish);
            cmd.boolParams.push_back(App::userSettings->useGlFinish_visionSensors);
            cmd.intParams.push_back(theDialog.idleFps);
            cmd.boolParams.push_back(App::userSettings->forceFboViaExt);
            cmd.intParams.push_back(App::userSettings->desiredOpenGlMajor);
            cmd.intParams.push_back(App::userSettings->desiredOpenGlMinor);

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

void CQDlgSettings::_selectItemOfCombobox(QComboBox* theBox,int itemData)
{
    theBox->setCurrentIndex(_getIndexOfComboboxItemWithData(theBox,itemData));
}

int CQDlgSettings::_getIndexOfComboboxItemWithData(QComboBox* theBox,int itemData)
{
    for (int i=0;i<theBox->count();i++)
    {
        if (theBox->itemData(i).toInt()==itemData)
            return(i);
    }
    return(-1);
}


void CQDlgSettings::on_qqHideHierarchy_clicked()
{
    App::appendSimulationThreadCommand(TOGGLE_HIDEHIERARCHY_USERSETTINGSGUITRIGGEREDCMD);
    App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
}
