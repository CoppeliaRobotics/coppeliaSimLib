
#include "vrepMainHeader.h"
#include "qdlgsimplefilter_keepRemoveCols.h"
#include "ui_qdlgsimplefilter_keepRemoveCols.h"
#include "gV.h"
#include "tt.h"

CQDlgSimpleFilter_keepRemoveCols::CQDlgSimpleFilter_keepRemoveCols(QWidget *parent) :
    CQDlgSimpleFilter(parent),
    ui(new Ui::CQDlgSimpleFilter_keepRemoveCols)
{
    ui->setupUi(this);
}

CQDlgSimpleFilter_keepRemoveCols::~CQDlgSimpleFilter_keepRemoveCols()
{
    delete ui;
}

void CQDlgSimpleFilter_keepRemoveCols::refresh()
{
    ui->qqKeep->setChecked((_byteParameters[0]&1)!=0);
    ui->qqRemove->setChecked((_byteParameters[0]&1)==0);
    ui->qqRgb->setChecked((_byteParameters[0]&2)!=0);
    ui->qqHsl->setChecked((_byteParameters[0]&2)==0);
    ui->qqBuffer1->setChecked((_byteParameters[0]&4)!=0);
    ui->qqRed->setText(tt::getFString(false,_floatParameters[0],3).c_str());
    ui->qqGreen->setText(tt::getFString(false,_floatParameters[1],3).c_str());
    ui->qqBlue->setText(tt::getFString(false,_floatParameters[2],3).c_str());
    ui->qqRedTol->setText(tt::getFString(false,_floatParameters[3],3).c_str());
    ui->qqGreenTol->setText(tt::getFString(false,_floatParameters[4],3).c_str());
    ui->qqBlueTol->setText(tt::getFString(false,_floatParameters[5],3).c_str());
}

void CQDlgSimpleFilter_keepRemoveCols::on_qqOkCancel_accepted()
{
    defaultModalDialogEndRoutine(true);
}

void CQDlgSimpleFilter_keepRemoveCols::on_qqOkCancel_rejected()
{
    defaultModalDialogEndRoutine(false);
}

void CQDlgSimpleFilter_keepRemoveCols::on_qqRemove_clicked()
{
    _byteParameters[0]&=255-1;
    refresh();
}

void CQDlgSimpleFilter_keepRemoveCols::on_qqKeep_clicked()
{
    _byteParameters[0]|=1;
    refresh();
}

void CQDlgSimpleFilter_keepRemoveCols::on_qqBuffer1_clicked()
{
    _byteParameters[0]^=4;
    refresh();
}

void CQDlgSimpleFilter_keepRemoveCols::on_qqRgb_clicked()
{
    _byteParameters[0]|=2;
    refresh();
}

void CQDlgSimpleFilter_keepRemoveCols::on_qqHsl_clicked()
{
    _byteParameters[0]&=255-2;
    refresh();
}

void CQDlgSimpleFilter_keepRemoveCols::on_qqRed_editingFinished()
{
    if (!ui->qqRed->isModified())
        return;
    bool ok;
    float newVal=ui->qqRed->text().toFloat(&ok);
    if (ok)
    {
        tt::limitValue(0.0f,1.0f,newVal);
        _floatParameters[0]=newVal;
    }
    refresh();
}

void CQDlgSimpleFilter_keepRemoveCols::on_qqGreen_editingFinished()
{
    if (!ui->qqGreen->isModified())
        return;
    bool ok;
    float newVal=ui->qqGreen->text().toFloat(&ok);
    if (ok)
    {
        tt::limitValue(0.0f,1.0f,newVal);
        _floatParameters[1]=newVal;
    }
    refresh();
}

void CQDlgSimpleFilter_keepRemoveCols::on_qqBlue_editingFinished()
{
    if (!ui->qqBlue->isModified())
        return;
    bool ok;
    float newVal=ui->qqBlue->text().toFloat(&ok);
    if (ok)
    {
        tt::limitValue(0.0f,1.0f,newVal);
        _floatParameters[2]=newVal;
    }
    refresh();
}

void CQDlgSimpleFilter_keepRemoveCols::on_qqRedTol_editingFinished()
{
    if (!ui->qqRedTol->isModified())
        return;
    bool ok;
    float newVal=ui->qqRedTol->text().toFloat(&ok);
    if (ok)
    {
        tt::limitValue(0.0f,1.0f,newVal);
        _floatParameters[3]=newVal;
    }
    refresh();
}

void CQDlgSimpleFilter_keepRemoveCols::on_qqGreenTol_editingFinished()
{
    if (!ui->qqGreenTol->isModified())
        return;
    bool ok;
    float newVal=ui->qqGreenTol->text().toFloat(&ok);
    if (ok)
    {
        tt::limitValue(0.0f,1.0f,newVal);
        _floatParameters[4]=newVal;
    }
    refresh();
}

void CQDlgSimpleFilter_keepRemoveCols::on_qqBlueTol_editingFinished()
{
    if (!ui->qqBlueTol->isModified())
        return;
    bool ok;
    float newVal=ui->qqBlueTol->text().toFloat(&ok);
    if (ok)
    {
        tt::limitValue(0.0f,1.0f,newVal);
        _floatParameters[5]=newVal;
    }
    refresh();
}
