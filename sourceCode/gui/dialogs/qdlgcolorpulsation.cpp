#include "qdlgcolorpulsation.h"
#include "ui_qdlgcolorpulsation.h"
#include "gV.h"
#include "tt.h"
#include "app.h"

CQDlgColorPulsation::CQDlgColorPulsation(QWidget *parent) :
    VDialog(parent,QT_MODAL_DLG_STYLE),
    ui(new Ui::CQDlgColorPulsation)
{
    ui->setupUi(this);
}

CQDlgColorPulsation::~CQDlgColorPulsation()
{
    delete ui;
}

void CQDlgColorPulsation::cancelEvent()
{
    defaultModalDialogEndRoutine(false);
}

void CQDlgColorPulsation::okEvent()
{
    defaultModalDialogEndRoutine(true);
}

void CQDlgColorPulsation::refresh()
{
    ui->qqEnablePulsation->setChecked(pulsationEnabled);
    ui->qqRealTime->setEnabled(pulsationEnabled);
    ui->qqSimulationTime->setEnabled(pulsationEnabled);
    ui->qqFrequency->setEnabled(pulsationEnabled);
    ui->qqPhase->setEnabled(pulsationEnabled);
    ui->qqRatio->setEnabled(pulsationEnabled);

    ui->qqRealTime->setChecked(pulsationRealTime);
    ui->qqSimulationTime->setChecked(!pulsationRealTime);
    ui->qqFrequency->setText(tt::getFString(false,pulsationFrequency,3).c_str());
    ui->qqPhase->setText(tt::getFString(false,pulsationPhase,3).c_str());
    ui->qqRatio->setText(tt::getFString(false,pulsationRatio,3).c_str());
}

void CQDlgColorPulsation::on_qqEnablePulsation_clicked()
{
    pulsationEnabled=!pulsationEnabled;
    refresh();
}

void CQDlgColorPulsation::on_qqRealTime_clicked()
{
    pulsationRealTime=true;
    refresh();
}

void CQDlgColorPulsation::on_qqSimulationTime_clicked()
{
    pulsationRealTime=false;
    refresh();
}

void CQDlgColorPulsation::on_qqFrequency_editingFinished()
{
    if (!ui->qqFrequency->isModified())
        return;
    bool ok;
    float newVal=ui->qqFrequency->text().toFloat(&ok);
    if (ok)
    {
        pulsationFrequency=newVal;
        pulsationFrequency=tt::getLimitedFloat(0.001,1000.0,pulsationFrequency);
    }
    refresh();
}

void CQDlgColorPulsation::on_qqPhase_editingFinished()
{
    if (!ui->qqPhase->isModified())
        return;
    bool ok;
    float newVal=ui->qqPhase->text().toFloat(&ok);
    if (ok)
    {
        pulsationPhase=newVal;
        pulsationPhase=tt::getLimitedFloat(0.0,0.999,pulsationPhase);
    }
    refresh();
}

void CQDlgColorPulsation::on_qqRatio_editingFinished()
{
    if (!ui->qqRatio->isModified())
        return;
    bool ok;
    float newVal=ui->qqRatio->text().toFloat(&ok);
    if (ok)
    {
        pulsationRatio=newVal;
        pulsationRatio=tt::getLimitedFloat(0.01,1.0,pulsationRatio);
    }
    refresh();
}

void CQDlgColorPulsation::on_qqOkCancel_accepted()
{
    defaultModalDialogEndRoutine(true);
}

void CQDlgColorPulsation::on_qqOkCancel_rejected()
{
    defaultModalDialogEndRoutine(false);
}
