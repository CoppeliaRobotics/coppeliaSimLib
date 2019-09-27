
#include "vrepMainHeader.h"
#include "qdlgmills.h"
#include "ui_qdlgmills.h"
#include "tt.h"
#include "gV.h"
#include "qdlgmaterial.h"
#include "qdlgdetectionvolume.h"
#include "v_repStrings.h"
#include "app.h"
#include "vMessageBox.h"

CQDlgMills::CQDlgMills(QWidget *parent) :
    CDlgEx(parent),
    ui(new Ui::CQDlgMills)
{
    _dlgType=MILL_DLG;
    ui->setupUi(this);
    inMainRefreshRoutine=false;
}

CQDlgMills::~CQDlgMills()
{
    delete ui;
}

void CQDlgMills::cancelEvent()
{
    // we override this cancel event. The container window should close, not this one!!
    App::mainWindow->dlgCont->close(OBJECT_DLG);
}

void CQDlgMills::refresh()
{
    inMainRefreshRoutine=true;
    QLineEdit* lineEditToSelect=getSelectedLineEdit();

    CMill* it=App::ct->objCont->getLastSelection_mill();

    bool isMill=App::ct->objCont->isLastSelectionAMill();
    bool noEditMode=App::getEditModeType()==NO_EDIT_MODE;
    bool noEditModeAndNoSim=noEditMode&&App::ct->simulation->isSimulationStopped();

    ui->qqEnableAll->setEnabled(noEditMode);
    ui->qqEnableAll->setChecked(App::ct->mainSettings->millsEnabled);

    ui->qqExplicitHandling->setEnabled(isMill&&noEditModeAndNoSim);
    ui->qqBaseSize->setEnabled(isMill&&noEditModeAndNoSim);
    ui->qqAdjustVolume->setEnabled(noEditModeAndNoSim);
    ui->qqAdjustVolume->setChecked(CQDlgDetectionVolume::showVolumeWindow);


    ui->qqEntityToCut->setEnabled(isMill&&noEditModeAndNoSim);
    ui->qqActiveVolumeColor->setEnabled(isMill&&noEditModeAndNoSim);
    ui->qqPassiveVolumeColor->setEnabled(isMill&&noEditModeAndNoSim);

    ui->qqExplicitHandling->setChecked(isMill&&it->getExplicitHandling());
    ui->qqEntityToCut->clear();

    if (it!=nullptr)
    { // Last selection is a mill
        ui->qqBaseSize->setText(tt::getFString(false,it->getSize(),3).c_str());

        ui->qqEntityToCut->addItem(strTranslate(IDSN_ALL_CUTTABLE_OBJECTS_IN_SCENE),QVariant(-1));

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
            ui->qqEntityToCut->addItem(names[i].c_str(),QVariant(ids[i]));

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
            ui->qqEntityToCut->addItem(names[i].c_str(),QVariant(ids[i]));

        // Select current item:
        for (int i=0;i<ui->qqEntityToCut->count();i++)
        {
            if (ui->qqEntityToCut->itemData(i).toInt()==it->getMillableObject())
            {
                ui->qqEntityToCut->setCurrentIndex(i);
                break;
            }
        }
    }
    else
        ui->qqBaseSize->setText("");

    selectLineEdit(lineEditToSelect);
    inMainRefreshRoutine=false;
}

void CQDlgMills::on_qqEnableAll_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_ENABLEALL_MILLGUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgMills::on_qqExplicitHandling_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CMill* theMill=App::ct->objCont->getLastSelection_mill();
        if (theMill!=nullptr)
        {
            App::appendSimulationThreadCommand(TOGGLE_EXPLICITHANDLING_MILLGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID());
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgMills::on_qqBaseSize_editingFinished()
{
    if (!ui->qqBaseSize->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        CMill* it=App::ct->objCont->getLastSelection_mill();
        bool ok;
        float newVal=ui->qqBaseSize->text().toFloat(&ok);
        if (it!=nullptr)
        {
            if (ok)
            {
                App::appendSimulationThreadCommand(SET_SIZE_MILLGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),-1,newVal);
                App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            }
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgMills::on_qqAdjustVolume_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CQDlgDetectionVolume::showVolumeWindow=!CQDlgDetectionVolume::showVolumeWindow;
        if (App::mainWindow->dlgCont->isVisible(DETECTION_VOLUME_DLG)!=CQDlgDetectionVolume::showVolumeWindow)
            App::mainWindow->dlgCont->toggle(DETECTION_VOLUME_DLG);
    }
}

void CQDlgMills::on_qqEntityToCut_currentIndexChanged(int index)
{
    if (!inMainRefreshRoutine)
    {
        IF_UI_EVENT_CAN_READ_DATA
        {
            int objID=ui->qqEntityToCut->itemData(ui->qqEntityToCut->currentIndex()).toInt();
            CMill* it=App::ct->objCont->getLastSelection_mill();
            if (it!=nullptr)
            {
                if (objID!=-1)
                {
                    bool displayWarning=false;
                    if ((objID<SIM_IDSTART_COLLECTION)&&(objID>=0))
                    {
                        C3DObject* it2=App::ct->objCont->getObjectFromHandle(objID);
                        if (it2!=nullptr)
                            displayWarning|=((it2->getLocalObjectSpecialProperty()&sim_objectspecialproperty_cuttable)==0);
                    }
                    if (displayWarning)
                        App::uiThread->messageBox_warning(App::mainWindow,strTranslate("Scene object"),strTranslate(IDS_OBJECT_NOT_CUTTABLE_WARNING),VMESSAGEBOX_OKELI);
                }
                App::appendSimulationThreadCommand(SET_ENTITYTOCUT_MILLGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),objID);
                App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
                App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
            }
        }
    }
}

void CQDlgMills::on_qqPassiveVolumeColor_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CQDlgMaterial::displayMaterialDlg(COLOR_ID_MILL_PASSIVE,App::ct->objCont->getLastSelectionID(),-1,App::mainWindow);
    }
}

void CQDlgMills::on_qqActiveVolumeColor_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CQDlgMaterial::displayMaterialDlg(COLOR_ID_MILL_ACTIVE,App::ct->objCont->getLastSelectionID(),-1,App::mainWindow);
    }
}
