
#include "qdlgsimplefilter_binary.h"
#include "ui_qdlgsimplefilter_binary.h"
#include "gV.h"
#include "tt.h"
#include "mathDefines.h"

CQDlgSimpleFilter_binary::CQDlgSimpleFilter_binary(QWidget *parent) :
    CQDlgSimpleFilter(parent),
    ui(new Ui::CQDlgSimpleFilter_binary)
{
    ui->setupUi(this);
}

CQDlgSimpleFilter_binary::~CQDlgSimpleFilter_binary()
{
    delete ui;
}

void CQDlgSimpleFilter_binary::refresh()
{
    ui->qqTriggerEnabled->setChecked((_byteParameters[0]&4)!=0);
    ui->qqVisualize->setChecked((_byteParameters[0]&8)!=0);
    ui->qqThreshold->setText(tt::getFString(false,_floatParameters[0],3).c_str());
    ui->qqProportion->setText(tt::getFString(false,_floatParameters[4],3).c_str());
    ui->qqProportionTol->setText(tt::getFString(false,_floatParameters[5],3).c_str());
    ui->qqPosX->setText(tt::getFString(false,_floatParameters[6],3).c_str());
    ui->qqPosXTol->setText(tt::getFString(false,_floatParameters[7],3).c_str());
    ui->qqPosY->setText(tt::getFString(false,_floatParameters[8],3).c_str());
    ui->qqPosYTol->setText(tt::getFString(false,_floatParameters[9],3).c_str());
    ui->qqAngle->setText(tt::getAngleFString(true,_floatParameters[10],2).c_str());
    ui->qqAngleTol->setText(tt::getAngleFString(false,_floatParameters[11],2).c_str());
    ui->qqRoundness->setText(tt::getFString(false,_floatParameters[12],3).c_str());
}

void CQDlgSimpleFilter_binary::on_qqOkCancel_accepted()
{
    defaultModalDialogEndRoutine(true);
}

void CQDlgSimpleFilter_binary::on_qqOkCancel_rejected()
{
    defaultModalDialogEndRoutine(false);
}

void CQDlgSimpleFilter_binary::on_qqThreshold_editingFinished()
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

void CQDlgSimpleFilter_binary::on_qqTriggerEnabled_clicked()
{
    _byteParameters[0]^=4;
    refresh();
}

void CQDlgSimpleFilter_binary::on_qqVisualize_clicked()
{
    _byteParameters[0]^=8;
    refresh();
}

void CQDlgSimpleFilter_binary::on_qqProportion_editingFinished()
{
    if (!ui->qqProportion->isModified())
        return;
    bool ok;
    float newVal=ui->qqProportion->text().toFloat(&ok);
    if (ok)
    {
        tt::limitValue(0.0f,1.0f,newVal);
        _floatParameters[4]=newVal;
    }
    refresh();
}

void CQDlgSimpleFilter_binary::on_qqProportionTol_editingFinished()
{
    if (!ui->qqProportionTol->isModified())
        return;
    bool ok;
    float newVal=ui->qqProportionTol->text().toFloat(&ok);
    if (ok)
    {
        tt::limitValue(0.0f,1.0f,newVal);
        _floatParameters[5]=newVal;
    }
    refresh();
}

void CQDlgSimpleFilter_binary::on_qqPosX_editingFinished()
{
    if (!ui->qqPosX->isModified())
        return;
    bool ok;
    float newVal=ui->qqPosX->text().toFloat(&ok);
    if (ok)
    {
        tt::limitValue(0.0f,1.0f,newVal);
        _floatParameters[6]=newVal;
    }
    refresh();
}

void CQDlgSimpleFilter_binary::on_qqPosXTol_editingFinished()
{
    if (!ui->qqPosXTol->isModified())
        return;
    bool ok;
    float newVal=ui->qqPosXTol->text().toFloat(&ok);
    if (ok)
    {
        tt::limitValue(0.0f,1.0f,newVal);
        _floatParameters[7]=newVal;
    }
    refresh();
}

void CQDlgSimpleFilter_binary::on_qqPosY_editingFinished()
{
    if (!ui->qqPosY->isModified())
        return;
    bool ok;
    float newVal=ui->qqPosY->text().toFloat(&ok);
    if (ok)
    {
        tt::limitValue(0.0f,1.0f,newVal);
        _floatParameters[8]=newVal;
    }
    refresh();
}

void CQDlgSimpleFilter_binary::on_qqPosYTol_editingFinished()
{
    if (!ui->qqPosYTol->isModified())
        return;
    bool ok;
    float newVal=ui->qqPosYTol->text().toFloat(&ok);
    if (ok)
    {
        tt::limitValue(0.0f,1.0f,newVal);
        _floatParameters[9]=newVal;
    }
    refresh();
}

void CQDlgSimpleFilter_binary::on_qqAngle_editingFinished()
{
    if (!ui->qqAngle->isModified())
        return;
    bool ok;
    float newVal=ui->qqAngle->text().toFloat(&ok);
    if (ok)
    {
        newVal*=gv::userToRad;
        tt::limitValue(-piValD2_f,piValD2_f,newVal);
        _floatParameters[10]=newVal;
    }
    refresh();
}

void CQDlgSimpleFilter_binary::on_qqAngleTol_editingFinished()
{
    if (!ui->qqAngleTol->isModified())
        return;
    bool ok;
    float newVal=ui->qqAngleTol->text().toFloat(&ok);
    if (ok)
    {
        newVal*=gv::userToRad;
        tt::limitValue(0.0f,piValue_f*0.501f,newVal);
        _floatParameters[11]=newVal;
    }
    refresh();
}

void CQDlgSimpleFilter_binary::on_qqRoundness_editingFinished()
{
    if (!ui->qqRoundness->isModified())
        return;
    bool ok;
    float newVal=ui->qqRoundness->text().toFloat(&ok);
    if (ok)
    {
        tt::limitValue(0.0f,1.0f,newVal);
        _floatParameters[12]=newVal;
    }
    refresh();
}
