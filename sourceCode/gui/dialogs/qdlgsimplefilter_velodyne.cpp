
#include "vrepMainHeader.h"
#include "qdlgsimplefilter_velodyne.h"
#include "ui_qdlgsimplefilter_velodyne.h"
#include "gV.h"
#include "tt.h"

CQDlgSimpleFilter_velodyne::CQDlgSimpleFilter_velodyne(QWidget *parent) :
    CQDlgSimpleFilter(parent),
    ui(new Ui::CQDlgSimpleFilter_velodyne)
{
    ui->setupUi(this);
}

CQDlgSimpleFilter_velodyne::~CQDlgSimpleFilter_velodyne()
{
    delete ui;
}

void CQDlgSimpleFilter_velodyne::refresh()
{
    ui->qqX->setText(tt::getIString(false,_intParameters[0]).c_str());
    ui->qqY->setText(tt::getIString(false,_intParameters[1]).c_str());
    ui->qqV->setText(tt::getAngleFString(false,_floatParameters[0],2).c_str());
}

void CQDlgSimpleFilter_velodyne::on_qqOkCancel_accepted()
{
    defaultModalDialogEndRoutine(true);
}

void CQDlgSimpleFilter_velodyne::on_qqOkCancel_rejected()
{
    defaultModalDialogEndRoutine(false);
}

void CQDlgSimpleFilter_velodyne::on_qqX_editingFinished()
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

void CQDlgSimpleFilter_velodyne::on_qqY_editingFinished()
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


void CQDlgSimpleFilter_velodyne::on_qqV_editingFinished()
{
    if (!ui->qqV->isModified())
        return;
    bool ok;
    float newVal=ui->qqV->text().toFloat(&ok);
    if (ok)
    {
        tt::limitValue(1.0f,120.0f,newVal);
        _floatParameters[0]=newVal*gv::userToRad;
    }
    refresh();
}
