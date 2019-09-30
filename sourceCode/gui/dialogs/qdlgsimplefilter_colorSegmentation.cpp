
#include "qdlgsimplefilter_colorSegmentation.h"
#include "ui_qdlgsimplefilter_colorSegmentation.h"
#include "gV.h"
#include "tt.h"

CQDlgSimpleFilter_colorSegmentation::CQDlgSimpleFilter_colorSegmentation(QWidget *parent) :
    CQDlgSimpleFilter(parent),
    ui(new Ui::CQDlgSimpleFilter_colorSegmentation)
{
    ui->setupUi(this);
}

CQDlgSimpleFilter_colorSegmentation::~CQDlgSimpleFilter_colorSegmentation()
{
    delete ui;
}

void CQDlgSimpleFilter_colorSegmentation::refresh()
{
    ui->qqDist->setText(tt::getFString(false,_floatParameters[0],3).c_str());
}

void CQDlgSimpleFilter_colorSegmentation::on_qqOkCancel_accepted()
{
    defaultModalDialogEndRoutine(true);
}

void CQDlgSimpleFilter_colorSegmentation::on_qqOkCancel_rejected()
{
    defaultModalDialogEndRoutine(false);
}

void CQDlgSimpleFilter_colorSegmentation::on_qqDist_editingFinished()
{
    if (!ui->qqDist->isModified())
        return;
    bool ok;
    float newVal=ui->qqDist->text().toFloat(&ok);
    if (ok)
    {
        tt::limitValue(0.05f,0.5f,newVal);
        _floatParameters[0]=newVal;
    }
    refresh();
}
