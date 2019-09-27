
#include "vrepMainHeader.h"
#include "qdlgsimplefilter_rectangularCut.h"
#include "ui_qdlgsimplefilter_rectangularCut.h"
#include "gV.h"
#include "tt.h"

CQDlgSimpleFilter_rectangularCut::CQDlgSimpleFilter_rectangularCut(QWidget *parent) :
    CQDlgSimpleFilter(parent),
    ui(new Ui::CQDlgSimpleFilter_rectangularCut)
{
    ui->setupUi(this);
}

CQDlgSimpleFilter_rectangularCut::~CQDlgSimpleFilter_rectangularCut()
{
    delete ui;
}

void CQDlgSimpleFilter_rectangularCut::refresh()
{
    ui->qqX->setText(tt::getFString(false,_floatParameters[0],3).c_str());
    ui->qqY->setText(tt::getFString(false,_floatParameters[1],3).c_str());
    ui->qqInverse->setChecked(_byteParameters[0]&1);
}

void CQDlgSimpleFilter_rectangularCut::on_qqOkCancel_accepted()
{
    defaultModalDialogEndRoutine(true);
}

void CQDlgSimpleFilter_rectangularCut::on_qqOkCancel_rejected()
{
    defaultModalDialogEndRoutine(false);
}

void CQDlgSimpleFilter_rectangularCut::on_qqX_editingFinished()
{
    if (!ui->qqX->isModified())
        return;
    bool ok;
    float newVal=ui->qqX->text().toFloat(&ok);
    if (ok)
    {
        tt::limitValue(0.01f,0.99f,newVal);
        _floatParameters[0]=newVal;
    }
    refresh();
}

void CQDlgSimpleFilter_rectangularCut::on_qqY_editingFinished()
{
    if (!ui->qqY->isModified())
        return;
    bool ok;
    float newVal=ui->qqY->text().toFloat(&ok);
    if (ok)
    {
        tt::limitValue(0.01f,0.99f,newVal);
        _floatParameters[1]=newVal;
    }
    refresh();
}

void CQDlgSimpleFilter_rectangularCut::on_qqInverse_clicked()
{
    _byteParameters[0]^=1;
}
