#include "qdlgworkspacemetric.h"
#include "ui_qdlgworkspacemetric.h"
#include "gV.h"
#include "tt.h"
#include "app.h"
#include "simStringTable.h"

CQDlgWorkSpaceMetric::CQDlgWorkSpaceMetric(QWidget *parent) :
    VDialog(parent,QT_MODAL_DLG_STYLE),
    ui(new Ui::CQDlgWorkSpaceMetric)
{
    ui->setupUi(this);
}

CQDlgWorkSpaceMetric::~CQDlgWorkSpaceMetric()
{
    delete ui;
}

void CQDlgWorkSpaceMetric::cancelEvent()
{
//  defaultModalDialogEndRoutine(false);
}

void CQDlgWorkSpaceMetric::okEvent()
{
//  defaultModalDialogEndRoutine(true);
}

void CQDlgWorkSpaceMetric::refresh()
{
    ui->qqX->setText(tt::getFString(false,data[0],3).c_str());
    ui->qqY->setText(tt::getFString(false,data[1],3).c_str());
    ui->qqZ->setText(tt::getFString(false,data[2],3).c_str());
    ui->qqAbg->setText(tt::getFString(false,data[3],3).c_str());
}

void CQDlgWorkSpaceMetric::on_qqClose_clicked(QAbstractButton *button)
{
    defaultModalDialogEndRoutine(true);
}

void CQDlgWorkSpaceMetric::on_qqX_editingFinished()
{
    if (!ui->qqX->isModified())
        return;
    bool ok;
    float newVal=ui->qqX->text().toFloat(&ok);
    if (ok)
        data[0]=tt::getLimitedFloat(0.0f,1000.0f,newVal);
    refresh();
}

void CQDlgWorkSpaceMetric::on_qqY_editingFinished()
{
    if (!ui->qqY->isModified())
        return;
    bool ok;
    float newVal=ui->qqY->text().toFloat(&ok);
    if (ok)
        data[1]=tt::getLimitedFloat(0.0f,1000.0f,newVal);
    refresh();
}

void CQDlgWorkSpaceMetric::on_qqZ_editingFinished()
{
    if (!ui->qqZ->isModified())
        return;
    bool ok;
    float newVal=ui->qqZ->text().toFloat(&ok);
    if (ok)
        data[2]=tt::getLimitedFloat(0.0f,1000.0f,newVal);
    refresh();
}

void CQDlgWorkSpaceMetric::on_qqAbg_editingFinished()
{
    if (!ui->qqAbg->isModified())
        return;
    bool ok;
    float newVal=ui->qqAbg->text().toFloat(&ok);
    if (ok)
        data[3]=tt::getLimitedFloat(0.0f,1000.0f,newVal);
    refresh();
}
