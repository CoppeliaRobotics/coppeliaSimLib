
#include "vrepMainHeader.h"
#include "qdlgsimplefilter_resize.h"
#include "ui_qdlgsimplefilter_resize.h"
#include "gV.h"
#include "tt.h"

CQDlgSimpleFilter_resize::CQDlgSimpleFilter_resize(QWidget *parent) :
    CQDlgSimpleFilter(parent),
    ui(new Ui::CQDlgSimpleFilter_resize)
{
    ui->setupUi(this);
}

CQDlgSimpleFilter_resize::~CQDlgSimpleFilter_resize()
{
    delete ui;
}

void CQDlgSimpleFilter_resize::refresh()
{
    ui->qqX->setText(tt::getFString(false,_floatParameters[0],3).c_str());
    ui->qqY->setText(tt::getFString(false,_floatParameters[1],3).c_str());
}

void CQDlgSimpleFilter_resize::on_qqOkCancel_accepted()
{
    defaultModalDialogEndRoutine(true);
}

void CQDlgSimpleFilter_resize::on_qqOkCancel_rejected()
{
    defaultModalDialogEndRoutine(false);
}

void CQDlgSimpleFilter_resize::on_qqX_editingFinished()
{
    if (!ui->qqX->isModified())
        return;
    bool ok;
    float newVal=ui->qqX->text().toFloat(&ok);
    if (ok)
    {
        tt::limitValue(0.01f,100.0f,newVal);
        _floatParameters[0]=newVal;
    }
    refresh();
}

void CQDlgSimpleFilter_resize::on_qqY_editingFinished()
{
    if (!ui->qqY->isModified())
        return;
    bool ok;
    float newVal=ui->qqY->text().toFloat(&ok);
    if (ok)
    {
        tt::limitValue(0.01f,100.0f,newVal);
        _floatParameters[1]=newVal;
    }
    refresh();
}
