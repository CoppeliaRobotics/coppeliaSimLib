
#include "vrepMainHeader.h"
#include "qdlgsimplefilter_edge.h"
#include "ui_qdlgsimplefilter_edge.h"
#include "gV.h"
#include "tt.h"

CQDlgSimpleFilter_edge::CQDlgSimpleFilter_edge(QWidget *parent) :
    CQDlgSimpleFilter(parent),
    ui(new Ui::CQDlgSimpleFilter_edge)
{
    ui->setupUi(this);
}

CQDlgSimpleFilter_edge::~CQDlgSimpleFilter_edge()
{
    delete ui;
}

void CQDlgSimpleFilter_edge::refresh()
{
    ui->qqThreshold->setText(tt::getFString(false,_floatParameters[0],3).c_str());
}

void CQDlgSimpleFilter_edge::on_qqOkCancel_accepted()
{
    defaultModalDialogEndRoutine(true);
}

void CQDlgSimpleFilter_edge::on_qqOkCancel_rejected()
{
    defaultModalDialogEndRoutine(false);
}

void CQDlgSimpleFilter_edge::on_qqThreshold_editingFinished()
{
    if (!ui->qqThreshold->isModified())
        return;
    bool ok;
    float newVal=ui->qqThreshold->text().toFloat(&ok);
    if (ok)
    {
        tt::limitValue(0.0f,1.0f,newVal);
        _floatParameters[0]=newVal;
    }
    refresh();
}
