
#include "vrepMainHeader.h"
#include "qdlgpaths.h"
#include "ui_qdlgpaths.h"
#include "tt.h"
#include "gV.h"
#include "qdlgmaterial.h"
#include "app.h"
#include "qdlgpathshaping.h"
#include "v_repStrings.h"

CQDlgPaths::CQDlgPaths(QWidget *parent) :
    CDlgEx(parent),
    ui(new Ui::CQDlgPaths)
{
    _dlgType=PATH_DLG;
    ui->setupUi(this);
    inMainRefreshRoutine=false;
}

CQDlgPaths::~CQDlgPaths()
{
    delete ui;
}

void CQDlgPaths::cancelEvent()
{
    // we override this cancel event. The container window should close, not this one!!
    App::mainWindow->dlgCont->close(OBJECT_DLG);
}

void CQDlgPaths::refresh()
{
    inMainRefreshRoutine=true;
    QLineEdit* lineEditToSelect=getSelectedLineEdit();
    bool noEditModeNoSim=(App::getEditModeType()==NO_EDIT_MODE)&&App::ct->simulation->isSimulationStopped();
    bool sel=App::ct->objCont->isLastSelectionAPath();

    CPath* path=App::ct->objCont->getLastSelection_path();
    CPathCont* pathCont=nullptr;
    if (path!=nullptr)
        pathCont=path->pathContainer;

    ui->qqShowOrientation->setEnabled(sel&&noEditModeNoSim);
    ui->qqShowPathLine->setEnabled(sel&&noEditModeNoSim);
    ui->qqShowPosition->setEnabled(sel&&noEditModeNoSim);
    ui->qqAdjustColor->setEnabled(sel&&noEditModeNoSim);
    ui->qqLineSize->setEnabled(sel&&noEditModeNoSim);
    ui->qqControlPointSize->setEnabled(sel&&noEditModeNoSim);
    ui->qqDistanceCombo->setEnabled(sel&&noEditModeNoSim);

    ui->qqShowShapingDialog->setEnabled(sel&&noEditModeNoSim);
    ui->qqShowShapingDialog->setChecked(CQDlgPathShaping::showWindow);

    ui->qqShowOrientation->setChecked((pathCont!=nullptr)&&((pathCont->getAttributes()&sim_pathproperty_show_orientation)!=0));
    ui->qqShowPathLine->setChecked((pathCont!=nullptr)&&((pathCont->getAttributes()&sim_pathproperty_show_line)!=0));
    ui->qqShowPosition->setChecked((pathCont!=nullptr)&&((pathCont->getAttributes()&sim_pathproperty_show_position)!=0));
    ui->qqDistanceCombo->clear();

    if (pathCont!=nullptr)
    {
        ui->qqLineSize->setText(tt::getIString(false,pathCont->getLineSize()).c_str());
        ui->qqControlPointSize->setText(tt::getFString(false,pathCont->getSquareSize(),3).c_str());

        ui->qqDistanceCombo->addItem(strTranslate(IDS_PATH_LENGTH_CALC_DL),QVariant(sim_distcalcmethod_dl));
        ui->qqDistanceCombo->addItem(strTranslate(IDS_PATH_LENGTH_CALC_DAC),QVariant(sim_distcalcmethod_dac));
        ui->qqDistanceCombo->addItem(strTranslate(IDS_PATH_LENGTH_CALC_MAX_DL_DAC),QVariant(sim_distcalcmethod_max_dl_dac));
        ui->qqDistanceCombo->addItem(strTranslate(IDS_PATH_LENGTH_CALC_DL_AND_DAC),QVariant(sim_distcalcmethod_dl_and_dac));
        ui->qqDistanceCombo->addItem(strTranslate(IDS_PATH_LENGTH_CALC_SQRT_DL2_AND_DAC2),QVariant(sim_distcalcmethod_sqrt_dl2_and_dac2));
        ui->qqDistanceCombo->addItem(strTranslate(IDS_PATH_LENGTH_CALC_DL_IF_NONZERO),QVariant(sim_distcalcmethod_dl_if_nonzero));
        ui->qqDistanceCombo->addItem(strTranslate(IDS_PATH_LENGTH_CALC_DAC_IF_NONZERO),QVariant(sim_distcalcmethod_dac_if_nonzero));
        for (int i=0;i<ui->qqDistanceCombo->count();i++)
        {
            if (ui->qqDistanceCombo->itemData(i).toInt()==pathCont->getPathLengthCalculationMethod())
            {
                ui->qqDistanceCombo->setCurrentIndex(i);
                break;
            }
        }
    }
    else
    {
        ui->qqLineSize->setText("");
        ui->qqControlPointSize->setText("");
    }

    selectLineEdit(lineEditToSelect);
    inMainRefreshRoutine=false;
}

CPathCont* CQDlgPaths::getPathCont()
{
    CPath* path=App::ct->objCont->getLastSelection_path();
    if (path!=nullptr)
        return(path->pathContainer);
    return(nullptr);
}

CPath* CQDlgPaths::getPath()
{
    return(App::ct->objCont->getLastSelection_path());
}


void CQDlgPaths::on_qqShowOrientation_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CPathCont* pathCont=getPathCont();
        if (pathCont!=nullptr)
        {
            int attr=pathCont->getAttributes()^sim_pathproperty_show_orientation;
            App::appendSimulationThreadCommand(SET_ATTRIBUTES_PATHGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),attr);
            pathCont->setAttributes(attr); // we also modify the UI thread resources
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgPaths::on_qqShowPathLine_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CPathCont* pathCont=getPathCont();
        if (pathCont!=nullptr)
        {
            int attr=pathCont->getAttributes()^sim_pathproperty_show_line;
            App::appendSimulationThreadCommand(SET_ATTRIBUTES_PATHGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),attr);
            pathCont->setAttributes(attr); // we also modify the UI thread resources
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgPaths::on_qqShowPosition_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CPathCont* pathCont=getPathCont();
        if (pathCont!=nullptr)
        {
            int attr=pathCont->getAttributes()^sim_pathproperty_show_position;
            App::appendSimulationThreadCommand(SET_ATTRIBUTES_PATHGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),attr);
            pathCont->setAttributes(attr); // we also modify the UI thread resources
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgPaths::on_qqAdjustColor_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CQDlgMaterial::displayMaterialDlg(COLOR_ID_PATH,App::ct->objCont->getLastSelectionID(),-1,App::mainWindow);
    }
}

void CQDlgPaths::on_qqLineSize_editingFinished()
{
    if (!ui->qqLineSize->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        CPathCont* pathCont=getPathCont();
        bool ok;
        int newVal=ui->qqLineSize->text().toInt(&ok);
        if (ok&&(pathCont!=nullptr))
        {
            App::appendSimulationThreadCommand(SET_LINESIZE_PATHGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgPaths::on_qqControlPointSize_editingFinished()
{
    if (!ui->qqControlPointSize->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        CPathCont* pathCont=getPathCont();
        bool ok;
        float newVal=ui->qqControlPointSize->text().toFloat(&ok);
        if (ok&&(pathCont!=nullptr))
        {
            App::appendSimulationThreadCommand(SET_CTRLPTSIZE_PATHGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),-1,newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgPaths::on_qqDistanceCombo_currentIndexChanged(int index)
{
    if (!inMainRefreshRoutine)
    {
        IF_UI_EVENT_CAN_READ_DATA
        {
            App::appendSimulationThreadCommand(SET_DISTANCEUNIT_PATHGUITRIGGEREDCMD,App::ct->objCont->getLastSelectionID(),ui->qqDistanceCombo->itemData(ui->qqDistanceCombo->currentIndex()).toInt());
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
            App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
        }
    }
}

void CQDlgPaths::on_qqShowShapingDialog_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CQDlgPathShaping::showWindow=!CQDlgPathShaping::showWindow;
        if (App::mainWindow->dlgCont->isVisible(PATH_SHAPING_DLG)!=CQDlgPathShaping::showWindow)
            App::mainWindow->dlgCont->toggle(PATH_SHAPING_DLG);
    }
}
