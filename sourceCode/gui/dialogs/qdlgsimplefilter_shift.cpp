
#include "qdlgsimplefilter_shift.h"
#include "ui_qdlgsimplefilter_shift.h"
#include "gV.h"
#include "tt.h"

CQDlgSimpleFilter_shift::CQDlgSimpleFilter_shift(QWidget *parent) :
    CQDlgSimpleFilter(parent),
    ui(new Ui::CQDlgSimpleFilter_shift)
{
    ui->setupUi(this);
}

CQDlgSimpleFilter_shift::~CQDlgSimpleFilter_shift()
{
    delete ui;
}

void CQDlgSimpleFilter_shift::refresh()
{
    ui->qqWrap->setChecked((_byteParameters[0]&1)==0);
    ui->qqX->setText(tt::getFString(true,_floatParameters[0],3).c_str());
    ui->qqY->setText(tt::getFString(true,_floatParameters[1],3).c_str());
}

void CQDlgSimpleFilter_shift::on_qqOkCancel_accepted()
{
    defaultModalDialogEndRoutine(true);
}

void CQDlgSimpleFilter_shift::on_qqOkCancel_rejected()
{
    defaultModalDialogEndRoutine(false);
}

void CQDlgSimpleFilter_shift::on_qqX_editingFinished()
{
    if (!ui->qqX->isModified())
        return;
    bool ok;
    float newVal=ui->qqX->text().toFloat(&ok);
    if (ok)
    {
        tt::limitValue(-1.0f,1.0f,newVal);
        _floatParameters[0]=newVal;
    }
    refresh();
}

void CQDlgSimpleFilter_shift::on_qqY_editingFinished()
{
    if (!ui->qqY->isModified())
        return;
    bool ok;
    float newVal=ui->qqY->text().toFloat(&ok);
    if (ok)
    {
        tt::limitValue(-1.0f,1.0f,newVal);
        _floatParameters[1]=newVal;
    }
    refresh();
}

void CQDlgSimpleFilter_shift::on_qqWrap_clicked()
{
    _byteParameters[0]^=1;
    refresh();
}
