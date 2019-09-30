
#include "qdlgsimplefilter_correlation.h"
#include "ui_qdlgsimplefilter_correlation.h"
#include "gV.h"
#include "tt.h"

CQDlgSimpleFilter_correlation::CQDlgSimpleFilter_correlation(QWidget *parent) :
    CQDlgSimpleFilter(parent),
    ui(new Ui::CQDlgSimpleFilter_correlation)
{
    ui->setupUi(this);
}

CQDlgSimpleFilter_correlation::~CQDlgSimpleFilter_correlation()
{
    delete ui;
}

void CQDlgSimpleFilter_correlation::refresh()
{
    ui->qqGreyScale->setChecked((_byteParameters[0]&16)!=0);
    ui->qqVisualizeCenter->setChecked((_byteParameters[0]&8)!=0);
    ui->qqTemplateX->setText(tt::getFString(false,_floatParameters[0],3).c_str());
    ui->qqTemplateY->setText(tt::getFString(false,_floatParameters[1],3).c_str());
    ui->qqFrameX->setText(tt::getFString(false,_floatParameters[2],3).c_str());
    ui->qqFrameY->setText(tt::getFString(false,_floatParameters[3],3).c_str());
}

void CQDlgSimpleFilter_correlation::on_qqOkCancel_accepted()
{
    defaultModalDialogEndRoutine(true);
}

void CQDlgSimpleFilter_correlation::on_qqOkCancel_rejected()
{
    defaultModalDialogEndRoutine(false);
}

void CQDlgSimpleFilter_correlation::on_qqGreyScale_clicked()
{
    _byteParameters[0]^=16;
    refresh();
}

void CQDlgSimpleFilter_correlation::on_qqVisualizeCenter_clicked()
{
    _byteParameters[0]^=8;
    refresh();
}

void CQDlgSimpleFilter_correlation::on_qqTemplateX_editingFinished()
{
    if (!ui->qqTemplateX->isModified())
        return;
    bool ok;
    float newVal=ui->qqTemplateX->text().toFloat(&ok);
    if (ok)
    {
        tt::limitValue(0.0f,1.0f,newVal);
        _floatParameters[0]=newVal;
        tt::limitValue(newVal,1.0f,_floatParameters[2]); // make sure the frame is not smaller!
    }
    refresh();
}

void CQDlgSimpleFilter_correlation::on_qqTemplateY_editingFinished()
{
    if (!ui->qqTemplateY->isModified())
        return;
    bool ok;
    float newVal=ui->qqTemplateY->text().toFloat(&ok);
    if (ok)
    {
        tt::limitValue(0.0f,1.0f,newVal);
        _floatParameters[1]=newVal;
        tt::limitValue(newVal,1.0f,_floatParameters[3]); // make sure the frame is not smaller!
    }
    refresh();
}

void CQDlgSimpleFilter_correlation::on_qqFrameX_editingFinished()
{
    if (!ui->qqFrameX->isModified())
        return;
    bool ok;
    float newVal=ui->qqFrameX->text().toFloat(&ok);
    if (ok)
    {
        tt::limitValue(0.0f,1.0f,newVal);
        _floatParameters[2]=newVal;
        tt::limitValue(0.0f,newVal,_floatParameters[0]); // make sure the template is not bigger!
    }
    refresh();
}

void CQDlgSimpleFilter_correlation::on_qqFrameY_editingFinished()
{
    if (!ui->qqFrameY->isModified())
        return;
    bool ok;
    float newVal=ui->qqFrameY->text().toFloat(&ok);
    if (ok)
    {
        tt::limitValue(0.0f,1.0f,newVal);
        _floatParameters[3]=newVal;
        tt::limitValue(0.0f,newVal,_floatParameters[1]); // make sure the template is not bigger!
    }
    refresh();
}
