
#include "qdlgsimplefilter_blob.h"
#include "ui_qdlgsimplefilter_blob.h"
#include "gV.h"
#include "tt.h"

CQDlgSimpleFilter_blob::CQDlgSimpleFilter_blob(QWidget *parent) :
    CQDlgSimpleFilter(parent),
    ui(new Ui::CQDlgSimpleFilter_blob)
{
    ui->setupUi(this);
}

CQDlgSimpleFilter_blob::~CQDlgSimpleFilter_blob()
{
    delete ui;
}

void CQDlgSimpleFilter_blob::refresh()
{
    ui->qqThreshold->setText(tt::getFString(false,_floatParameters[0],3).c_str());
    ui->qqMinSize->setText(tt::getFString(false,_floatParameters[1],3).c_str());
    ui->qqDiffColor->setChecked(_byteParameters[0]&1);
}

void CQDlgSimpleFilter_blob::on_qqOkCancel_accepted()
{
    defaultModalDialogEndRoutine(true);
}

void CQDlgSimpleFilter_blob::on_qqOkCancel_rejected()
{
    defaultModalDialogEndRoutine(false);
}

void CQDlgSimpleFilter_blob::on_qqThreshold_editingFinished()
{
    if (!ui->qqThreshold->isModified())
        return;
    bool ok;
    float newVal=ui->qqThreshold->text().toFloat(&ok);
    if (ok)
    {
        tt::limitValue(0.01f,1.0f,newVal);
        _floatParameters[0]=newVal;
    }
    refresh();
}

void CQDlgSimpleFilter_blob::on_qqDiffColor_clicked()
{
    if (_byteParameters[0]&1)
        _byteParameters[0]-=1;
    else
        _byteParameters[0]|=1;
    refresh();
}

void CQDlgSimpleFilter_blob::on_qqMinSize_editingFinished()
{
    if (!ui->qqMinSize->isModified())
        return;
    bool ok;
    float newVal=ui->qqMinSize->text().toFloat(&ok);
    if (ok)
    {
        tt::limitValue(0.0f,1.0f,newVal);
        _floatParameters[1]=newVal;
    }
    refresh();
}
