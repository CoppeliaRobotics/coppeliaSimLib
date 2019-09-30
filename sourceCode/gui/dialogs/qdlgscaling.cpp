
#include "qdlgscaling.h"
#include "ui_qdlgscaling.h"
#include "app.h"
#include "sceneObjectOperations.h"
#include "tt.h"

CQDlgScaling::CQDlgScaling(QWidget *parent) :
    VDialog(parent,QT_MODAL_DLG_STYLE),
    ui(new Ui::CQDlgScaling)
{
    ui->setupUi(this);
    inPlace=false;
    factor=2.0f;
}

CQDlgScaling::~CQDlgScaling()
{
    delete ui;
}

void CQDlgScaling::cancelEvent()
{
    defaultModalDialogEndRoutine(false);
}

void CQDlgScaling::okEvent()
{
    _doTheScaling();
    defaultModalDialogEndRoutine(true);
}

void CQDlgScaling::refresh()
{
    ui->qqInPlace->setChecked(inPlace);
    ui->qqScalingFactor->setText(tt::getEString(false,factor,3).c_str());
}

void CQDlgScaling::_doTheScaling()
{
    std::vector<int> sel;
    for (int i=0;i<App::ct->objCont->getSelSize();i++)
        sel.push_back(App::ct->objCont->getSelID(i));
    SSimulationThreadCommand cmd;
    cmd.cmdId=SCALE_SCALINGGUITRIGGEREDCMD;
    cmd.intParams.assign(sel.begin(),sel.end());
    cmd.floatParams.push_back(factor);
    cmd.boolParams.push_back(inPlace);
    App::appendSimulationThreadCommand(cmd);
    App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
    App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
}

void CQDlgScaling::on_qqInPlace_clicked()
{
    inPlace=!inPlace;
    refresh();
}

void CQDlgScaling::on_qqScalingFactor_editingFinished()
{
    if (!ui->qqScalingFactor->isModified())
        return;
    bool ok;
    float newVal=ui->qqScalingFactor->text().toFloat(&ok);
    if (ok)
    {
        tt::limitValue(0.001f,1000.0f,newVal);
        factor=newVal;
    }
    refresh();
}

void CQDlgScaling::on_qqOkCancel_accepted()
{
    _doTheScaling();
    defaultModalDialogEndRoutine(true);
}

void CQDlgScaling::on_qqOkCancel_rejected()
{
    defaultModalDialogEndRoutine(false);
}
