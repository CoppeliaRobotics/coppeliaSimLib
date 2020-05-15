#include "qdlgfog.h"
#include "ui_qdlgfog.h"
#include "gV.h"
#include "tt.h"
#include "qdlgcolor.h"
#include "app.h"

CQDlgFog::CQDlgFog(QWidget *parent) :
    CDlgEx(parent),
    ui(new Ui::CQDlgFog)
{
    _dlgType=FOG_DLG;
    ui->setupUi(this);
}

CQDlgFog::~CQDlgFog()
{
    delete ui;
}

void CQDlgFog::refresh()
{
    QLineEdit* lineEditToSelect=getSelectedLineEdit();
    bool noEditModeNoSim=(App::getEditModeType()==NO_EDIT_MODE)&&App::currentWorld->simulation->isSimulationStopped();

    ui->qqFogEnabled->setEnabled(noEditModeNoSim);
    ui->qqFogEnabled->setChecked(App::currentWorld->environment->getFogEnabled());

    ui->qqLinear->setEnabled(App::currentWorld->environment->getFogEnabled()&&noEditModeNoSim);
    ui->qqExp->setEnabled(App::currentWorld->environment->getFogEnabled()&&noEditModeNoSim);
    ui->qqExp2->setEnabled(App::currentWorld->environment->getFogEnabled()&&noEditModeNoSim);
    ui->qqStartDistance->setEnabled(App::currentWorld->environment->getFogEnabled()&&(App::currentWorld->environment->getFogType()==0)&&noEditModeNoSim);
    ui->qqEndDistance->setEnabled(App::currentWorld->environment->getFogEnabled()&&(App::currentWorld->environment->getFogType()==0)&&noEditModeNoSim);
    ui->qqDensity->setEnabled(App::currentWorld->environment->getFogEnabled()&&(App::currentWorld->environment->getFogType()!=0)&&noEditModeNoSim);
    ui->qqAdjustColor->setEnabled(App::currentWorld->environment->getFogEnabled()&&noEditModeNoSim);

    ui->qqLinear->setChecked(App::currentWorld->environment->getFogType()==0);
    ui->qqExp->setChecked(App::currentWorld->environment->getFogType()==1);
    ui->qqExp2->setChecked(App::currentWorld->environment->getFogType()==2);
    if (App::currentWorld->environment->getFogType()!=0)
    {
        ui->qqStartDistance->setText("");
        ui->qqEndDistance->setText("");
        ui->qqDensity->setText(tt::getFString(false,App::currentWorld->environment->getFogDensity(),2).c_str());
    }
    else
    {
        ui->qqStartDistance->setText(tt::getFString(false,App::currentWorld->environment->getFogStart(),2).c_str());
        ui->qqEndDistance->setText(tt::getFString(false,App::currentWorld->environment->getFogEnd(),2).c_str());
        ui->qqDensity->setText("");
    }

    selectLineEdit(lineEditToSelect);
}

void CQDlgFog::on_qqFogEnabled_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(TOGGLE_ENABLED_FOGGUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgFog::on_qqLinear_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(SET_TYPE_FOGGUITRIGGEREDCMD,0);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgFog::on_qqExp_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(SET_TYPE_FOGGUITRIGGEREDCMD,1);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgFog::on_qqExp2_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(SET_TYPE_FOGGUITRIGGEREDCMD,2);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgFog::on_qqStartDistance_editingFinished()
{
    if (!ui->qqStartDistance->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        float newVal=ui->qqStartDistance->text().toFloat(&ok);
        if (ok)
        {
            App::appendSimulationThreadCommand(SET_START_FOGGUITRIGGEREDCMD,-1,-1,newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgFog::on_qqEndDistance_editingFinished()
{
    if (!ui->qqEndDistance->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        float newVal=ui->qqEndDistance->text().toFloat(&ok);
        if (ok)
        {
            App::appendSimulationThreadCommand(SET_END_FOGGUITRIGGEREDCMD,-1,-1,newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgFog::on_qqDensity_editingFinished()
{
    if (!ui->qqDensity->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        float newVal=ui->qqDensity->text().toFloat(&ok);
        if (ok)
        {
            App::appendSimulationThreadCommand(SET_DENSITY_FOGGUITRIGGEREDCMD,-1,-1,newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgFog::on_qqAdjustColor_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CQDlgColor::displayDlg(COLOR_ID_FOG,-1,-1,0,App::mainWindow);
    }
}
