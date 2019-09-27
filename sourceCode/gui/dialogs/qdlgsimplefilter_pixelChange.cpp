
#include "vrepMainHeader.h"
#include "qdlgsimplefilter_pixelChange.h"
#include "ui_qdlgsimplefilter_pixelChange.h"
#include "gV.h"
#include "tt.h"

CQDlgSimpleFilter_pixelChange::CQDlgSimpleFilter_pixelChange(QWidget *parent) :
    CQDlgSimpleFilter(parent),
    ui(new Ui::CQDlgSimpleFilter_pixelChange)
{
    ui->setupUi(this);
}

CQDlgSimpleFilter_pixelChange::~CQDlgSimpleFilter_pixelChange()
{
    delete ui;
}

void CQDlgSimpleFilter_pixelChange::refresh()
{
    ui->qqX->setText(tt::getFString(false,_floatParameters[0]*100.0f,2).c_str());
}

void CQDlgSimpleFilter_pixelChange::on_qqOkCancel_accepted()
{
    defaultModalDialogEndRoutine(true);
}

void CQDlgSimpleFilter_pixelChange::on_qqOkCancel_rejected()
{
    defaultModalDialogEndRoutine(false);
}

void CQDlgSimpleFilter_pixelChange::on_qqX_editingFinished()
{
    if (!ui->qqX->isModified())
        return;
    bool ok;
    float newVal=ui->qqX->text().toFloat(&ok);
    if (ok)
    {
        tt::limitValue(1.0f,100.0f,newVal);
        _floatParameters[0]=newVal/100.0f;
    }
    refresh();
}
