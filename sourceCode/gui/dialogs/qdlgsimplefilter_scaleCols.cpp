
#include "vrepMainHeader.h"
#include "qdlgsimplefilter_scaleCols.h"
#include "ui_qdlgsimplefilter_scaleCols.h"
#include "gV.h"
#include "tt.h"

CQDlgSimpleFilter_scaleCols::CQDlgSimpleFilter_scaleCols(QWidget *parent) :
    CQDlgSimpleFilter(parent),
    ui(new Ui::CQDlgSimpleFilter_scaleCols)
{
    ui->setupUi(this);
}

CQDlgSimpleFilter_scaleCols::~CQDlgSimpleFilter_scaleCols()
{
    delete ui;
}

void CQDlgSimpleFilter_scaleCols::refresh()
{
    ui->qqRgb->setChecked((_byteParameters[0]&2)!=0);
    ui->qqHsl->setChecked((_byteParameters[0]&2)==0);
    ui->qqRed1->setText(tt::getFString(true,_floatParameters[0],3).c_str());
    ui->qqGreen1->setText(tt::getFString(true,_floatParameters[1],3).c_str());
    ui->qqBlue1->setText(tt::getFString(true,_floatParameters[2],3).c_str());
    ui->qqRed2->setText(tt::getFString(true,_floatParameters[3],3).c_str());
    ui->qqGreen2->setText(tt::getFString(true,_floatParameters[4],3).c_str());
    ui->qqBlue2->setText(tt::getFString(true,_floatParameters[5],3).c_str());
    ui->qqRed3->setText(tt::getFString(true,_floatParameters[6],3).c_str());
    ui->qqGreen3->setText(tt::getFString(true,_floatParameters[7],3).c_str());
    ui->qqBlue3->setText(tt::getFString(true,_floatParameters[8],3).c_str());
}

void CQDlgSimpleFilter_scaleCols::on_qqOkCancel_accepted()
{
    defaultModalDialogEndRoutine(true);
}

void CQDlgSimpleFilter_scaleCols::on_qqOkCancel_rejected()
{
    defaultModalDialogEndRoutine(false);
}

void CQDlgSimpleFilter_scaleCols::on_qqRgb_clicked()
{
    _byteParameters[0]|=2;
    refresh();
}

void CQDlgSimpleFilter_scaleCols::on_qqHsl_clicked()
{
    _byteParameters[0]&=255-2;
    refresh();
}

void CQDlgSimpleFilter_scaleCols::on_qqRed1_editingFinished()
{
    if (!ui->qqRed1->isModified())
        return;
    bool ok;
    float newVal=ui->qqRed1->text().toFloat(&ok);
    if (ok)
    {
        tt::limitValue(-1.0f,1.0f,newVal);
        _floatParameters[0]=newVal;
    }
    refresh();
}

void CQDlgSimpleFilter_scaleCols::on_qqGreen1_editingFinished()
{
    if (!ui->qqGreen1->isModified())
        return;
    bool ok;
    float newVal=ui->qqGreen1->text().toFloat(&ok);
    if (ok)
    {
        tt::limitValue(-1.0f,1.0f,newVal);
        _floatParameters[1]=newVal;
    }
    refresh();
}

void CQDlgSimpleFilter_scaleCols::on_qqBlue1_editingFinished()
{
    if (!ui->qqBlue1->isModified())
        return;
    bool ok;
    float newVal=ui->qqBlue1->text().toFloat(&ok);
    if (ok)
    {
        tt::limitValue(-1.0f,1.0f,newVal);
        _floatParameters[2]=newVal;
    }
    refresh();
}

void CQDlgSimpleFilter_scaleCols::on_qqRed2_editingFinished()
{
    if (!ui->qqRed2->isModified())
        return;
    bool ok;
    float newVal=ui->qqRed2->text().toFloat(&ok);
    if (ok)
    {
        tt::limitValue(-100.0f,100.0f,newVal);
        _floatParameters[3]=newVal;
    }
    refresh();
}

void CQDlgSimpleFilter_scaleCols::on_qqGreen2_editingFinished()
{
    if (!ui->qqGreen2->isModified())
        return;
    bool ok;
    float newVal=ui->qqGreen2->text().toFloat(&ok);
    if (ok)
    {
        tt::limitValue(-100.0f,100.0f,newVal);
        _floatParameters[4]=newVal;
    }
    refresh();
}

void CQDlgSimpleFilter_scaleCols::on_qqBlue2_editingFinished()
{
    if (!ui->qqBlue2->isModified())
        return;
    bool ok;
    float newVal=ui->qqBlue2->text().toFloat(&ok);
    if (ok)
    {
        tt::limitValue(-100.0f,100.0f,newVal);
        _floatParameters[5]=newVal;
    }
    refresh();
}

void CQDlgSimpleFilter_scaleCols::on_qqRed3_editingFinished()
{
    if (!ui->qqRed3->isModified())
        return;
    bool ok;
    float newVal=ui->qqRed3->text().toFloat(&ok);
    if (ok)
    {
        tt::limitValue(-1.0f,1.0f,newVal);
        _floatParameters[6]=newVal;
    }
    refresh();
}

void CQDlgSimpleFilter_scaleCols::on_qqGreen3_editingFinished()
{
    if (!ui->qqGreen3->isModified())
        return;
    bool ok;
    float newVal=ui->qqGreen3->text().toFloat(&ok);
    if (ok)
    {
        tt::limitValue(-1.0f,1.0f,newVal);
        _floatParameters[7]=newVal;
    }
    refresh();
}

void CQDlgSimpleFilter_scaleCols::on_qqBlue3_editingFinished()
{
    if (!ui->qqBlue3->isModified())
        return;
    bool ok;
    float newVal=ui->qqBlue3->text().toFloat(&ok);
    if (ok)
    {
        tt::limitValue(-1.0f,1.0f,newVal);
        _floatParameters[8]=newVal;
    }
    refresh();
}
