#include <qdlgfog.h>
#include <ui_qdlgfog.h>
#include <tt.h>
#include <utils.h>
#include <qdlgcolor.h>
#include <app.h>
#include <guiApp.h>

CQDlgFog::CQDlgFog(QWidget* parent)
    : CDlgEx(parent), ui(new Ui::CQDlgFog)
{
    _dlgType = FOG_DLG;
    ui->setupUi(this);
}

CQDlgFog::~CQDlgFog()
{
    delete ui;
}

void CQDlgFog::refresh()
{
    QLineEdit* lineEditToSelect = getSelectedLineEdit();
    bool noEditModeNoSim =
        (GuiApp::getEditModeType() == NO_EDIT_MODE) && App::currentWorld->simulation->isSimulationStopped();

    ui->qqFogEnabled->setEnabled(noEditModeNoSim);
    ui->qqFogEnabled->setChecked(App::currentWorld->environment->getFogEnabled());

    ui->qqLinear->setEnabled(App::currentWorld->environment->getFogEnabled() && noEditModeNoSim);
    ui->qqExp->setEnabled(App::currentWorld->environment->getFogEnabled() && noEditModeNoSim);
    ui->qqExp2->setEnabled(App::currentWorld->environment->getFogEnabled() && noEditModeNoSim);
    ui->qqStartDistance->setEnabled(App::currentWorld->environment->getFogEnabled() &&
                                    (App::currentWorld->environment->getFogType() == 0) && noEditModeNoSim);
    ui->qqEndDistance->setEnabled(App::currentWorld->environment->getFogEnabled() &&
                                  (App::currentWorld->environment->getFogType() == 0) && noEditModeNoSim);
    ui->qqDensity->setEnabled(App::currentWorld->environment->getFogEnabled() &&
                              (App::currentWorld->environment->getFogType() != 0) && noEditModeNoSim);
    ui->qqAdjustColor->setEnabled(App::currentWorld->environment->getFogEnabled() && noEditModeNoSim);

    ui->qqLinear->setChecked(App::currentWorld->environment->getFogType() == 0);
    ui->qqExp->setChecked(App::currentWorld->environment->getFogType() == 1);
    ui->qqExp2->setChecked(App::currentWorld->environment->getFogType() == 2);
    if (App::currentWorld->environment->getFogType() != 0)
    {
        ui->qqStartDistance->setText("");
        ui->qqEndDistance->setText("");
        ui->qqDensity->setText(utils::get0To1String(false, App::currentWorld->environment->getFogDensity()).c_str());
    }
    else
    {
        ui->qqStartDistance->setText(
            utils::getSizeString(false, App::currentWorld->environment->getFogStart()).c_str());
        ui->qqEndDistance->setText(utils::getSizeString(false, App::currentWorld->environment->getFogEnd()).c_str());
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
        App::appendSimulationThreadCommand(SET_TYPE_FOGGUITRIGGEREDCMD, 0);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgFog::on_qqExp_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(SET_TYPE_FOGGUITRIGGEREDCMD, 1);
        App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgFog::on_qqExp2_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::appendSimulationThreadCommand(SET_TYPE_FOGGUITRIGGEREDCMD, 2);
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
        double newVal = GuiApp::getEvalDouble(ui->qqStartDistance->text().toStdString().c_str(), &ok);
        if (ok)
        {
            App::appendSimulationThreadCommand(SET_START_FOGGUITRIGGEREDCMD, -1, -1, newVal);
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
        double newVal = GuiApp::getEvalDouble(ui->qqEndDistance->text().toStdString().c_str(), &ok);
        if (ok)
        {
            App::appendSimulationThreadCommand(SET_END_FOGGUITRIGGEREDCMD, -1, -1, newVal);
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
        double newVal = GuiApp::getEvalDouble(ui->qqDensity->text().toStdString().c_str(), &ok);
        if (ok)
        {
            App::appendSimulationThreadCommand(SET_DENSITY_FOGGUITRIGGEREDCMD, -1, -1, newVal);
            App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
        }
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgFog::on_qqAdjustColor_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        CQDlgColor::displayDlg(COLOR_ID_FOG, -1, -1, 0, GuiApp::mainWindow);
    }
}
