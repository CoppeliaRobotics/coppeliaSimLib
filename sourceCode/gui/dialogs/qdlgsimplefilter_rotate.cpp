
#include "vrepMainHeader.h"
#include "qdlgsimplefilter_rotate.h"
#include "ui_qdlgsimplefilter_rotate.h"
#include "gV.h"
#include "tt.h"
#include "mathDefines.h"

CQDlgSimpleFilter_rotate::CQDlgSimpleFilter_rotate(QWidget *parent) :
    CQDlgSimpleFilter(parent),
    ui(new Ui::CQDlgSimpleFilter_rotate)
{
    ui->setupUi(this);
}

CQDlgSimpleFilter_rotate::~CQDlgSimpleFilter_rotate()
{
    delete ui;
}

void CQDlgSimpleFilter_rotate::refresh()
{
    ui->qqAngle->setText(tt::getAngleFString(true,_floatParameters[0],2).c_str());
}

void CQDlgSimpleFilter_rotate::on_qqOkCancel_accepted()
{
    defaultModalDialogEndRoutine(true);
}

void CQDlgSimpleFilter_rotate::on_qqOkCancel_rejected()
{
    defaultModalDialogEndRoutine(false);
}

void CQDlgSimpleFilter_rotate::on_qqAngle_editingFinished()
{
    if (!ui->qqAngle->isModified())
        return;
    bool ok;
    float newVal=ui->qqAngle->text().toFloat(&ok);
    if (ok)
    {
        newVal*=gv::userToRad;
        tt::limitValue(-piValue_f,piValue_f,newVal);
        _floatParameters[0]=newVal;
    }
    refresh();
}
