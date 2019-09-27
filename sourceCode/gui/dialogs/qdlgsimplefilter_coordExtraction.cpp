
#include "vrepMainHeader.h"
#include "qdlgsimplefilter_coordExtraction.h"
#include "ui_qdlgsimplefilter_coordExtraction.h"
#include "gV.h"
#include "tt.h"

CQDlgSimpleFilter_coordExtraction::CQDlgSimpleFilter_coordExtraction(QWidget *parent) :
    CQDlgSimpleFilter(parent),
    ui(new Ui::CQDlgSimpleFilter_coordExtraction)
{
    ui->setupUi(this);
}

CQDlgSimpleFilter_coordExtraction::~CQDlgSimpleFilter_coordExtraction()
{
    delete ui;
}

void CQDlgSimpleFilter_coordExtraction::refresh()
{
    ui->qqX->setText(tt::getIString(false,_intParameters[0]).c_str());
    ui->qqY->setText(tt::getIString(false,_intParameters[1]).c_str());
    ui->qqAngular->setChecked((_byteParameters[0]&1)==0);
    ui->qqNonAngular->setChecked((_byteParameters[0]&1)!=0);
}

void CQDlgSimpleFilter_coordExtraction::on_qqOkCancel_accepted()
{
    defaultModalDialogEndRoutine(true);
}

void CQDlgSimpleFilter_coordExtraction::on_qqOkCancel_rejected()
{
    defaultModalDialogEndRoutine(false);
}

void CQDlgSimpleFilter_coordExtraction::on_qqX_editingFinished()
{
    if (!ui->qqX->isModified())
        return;
    bool ok;
    int newVal=ui->qqX->text().toInt(&ok);
    if (ok)
    {
        tt::limitValue(1,2048,newVal);
        _intParameters[0]=newVal;
    }
    refresh();
}

void CQDlgSimpleFilter_coordExtraction::on_qqY_editingFinished()
{
    if (!ui->qqY->isModified())
        return;
    bool ok;
    int newVal=ui->qqY->text().toInt(&ok);
    if (ok)
    {
        tt::limitValue(1,2048,newVal);
        _intParameters[1]=newVal;
    }
    refresh();
}


void CQDlgSimpleFilter_coordExtraction::on_qqAngular_clicked()
{
    _byteParameters[0]&=255-1;
    refresh();
}

void CQDlgSimpleFilter_coordExtraction::on_qqNonAngular_clicked()
{
    _byteParameters[0]|=1;
    refresh();
}
