
#include "vrepMainHeader.h"
#include "qdlgsimplefilter_intensityScale.h"
#include "ui_qdlgsimplefilter_intensityScale.h"
#include "gV.h"
#include "tt.h"

CQDlgSimpleFilter_intensityScale::CQDlgSimpleFilter_intensityScale(QWidget *parent) :
    CQDlgSimpleFilter(parent),
    ui(new Ui::CQDlgSimpleFilter_intensityScale)
{
    ui->setupUi(this);
}

CQDlgSimpleFilter_intensityScale::~CQDlgSimpleFilter_intensityScale()
{
    delete ui;
}

void CQDlgSimpleFilter_intensityScale::refresh()
{
    ui->qqGreyScale->setChecked(_intParameters[0]==0);
    ui->qqIntensityScale->setChecked(_intParameters[0]==1);
    ui->qqStart->setText(tt::getFString(false,_floatParameters[0],3).c_str());
    ui->qqEnd->setText(tt::getFString(false,_floatParameters[1],3).c_str());
}

void CQDlgSimpleFilter_intensityScale::on_qqOkCancel_accepted()
{
    defaultModalDialogEndRoutine(true);
}

void CQDlgSimpleFilter_intensityScale::on_qqOkCancel_rejected()
{
    defaultModalDialogEndRoutine(false);
}

void CQDlgSimpleFilter_intensityScale::on_qqGreyScale_clicked()
{
    _intParameters[0]=0;
    refresh();
}

void CQDlgSimpleFilter_intensityScale::on_qqIntensityScale_clicked()
{
    _intParameters[0]=1;
    refresh();
}

void CQDlgSimpleFilter_intensityScale::on_qqStart_editingFinished()
{
    if (!ui->qqStart->isModified())
        return;
    bool ok;
    float newVal=ui->qqStart->text().toFloat(&ok);
    if (ok)
    {
        tt::limitValue(0.0f,1.0f,newVal);
        _floatParameters[0]=newVal;
    }
    refresh();

}

void CQDlgSimpleFilter_intensityScale::on_qqEnd_editingFinished()
{
    if (!ui->qqEnd->isModified())
        return;
    bool ok;
    float newVal=ui->qqEnd->text().toFloat(&ok);
    if (ok)
    {
        tt::limitValue(0.0f,1.0f,newVal);
        _floatParameters[1]=newVal;
    }
    refresh();
}
