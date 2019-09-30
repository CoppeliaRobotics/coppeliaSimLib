
#include "qdlgsimplefilter_circularCut.h"
#include "ui_qdlgsimplefilter_circularCut.h"
#include "gV.h"
#include "tt.h"

CQDlgSimpleFilter_circularCut::CQDlgSimpleFilter_circularCut(QWidget *parent) :
    CQDlgSimpleFilter(parent),
    ui(new Ui::CQDlgSimpleFilter_circularCut)
{
    ui->setupUi(this);
}

CQDlgSimpleFilter_circularCut::~CQDlgSimpleFilter_circularCut()
{
    delete ui;
}

void CQDlgSimpleFilter_circularCut::refresh()
{
    ui->qqRadius->setText(tt::getFString(false,_floatParameters[0],3).c_str());
    ui->qqInverse->setChecked((_byteParameters[0]&1)!=0);
}

void CQDlgSimpleFilter_circularCut::on_qqOkCancel_accepted()
{
    defaultModalDialogEndRoutine(true);
}

void CQDlgSimpleFilter_circularCut::on_qqOkCancel_rejected()
{
    defaultModalDialogEndRoutine(false);
}

void CQDlgSimpleFilter_circularCut::on_qqRadius_editingFinished()
{
    if (!ui->qqRadius->isModified())
        return;
    bool ok;
    float newVal=ui->qqRadius->text().toFloat(&ok);
    if (ok)
    {
        tt::limitValue(0.01f,100.0f,newVal);
        _floatParameters[0]=newVal;
    }
    refresh();
}

void CQDlgSimpleFilter_circularCut::on_qqInverse_clicked()
{
    _byteParameters[0]^=1;
}
