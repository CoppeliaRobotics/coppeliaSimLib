
#include "vrepMainHeader.h"
#include "qdlgsimplefilter_5x5.h"
#include "ui_qdlgsimplefilter_5x5.h"
#include "gV.h"
#include "tt.h"

CQDlgSimpleFilter_5x5::CQDlgSimpleFilter_5x5(QWidget *parent) :
    CQDlgSimpleFilter(parent),
    ui(new Ui::CQDlgSimpleFilter_5x5)
{
    ui->setupUi(this);
}

CQDlgSimpleFilter_5x5::~CQDlgSimpleFilter_5x5()
{
    delete ui;
}

void CQDlgSimpleFilter_5x5::refresh()
{
    ui->qqPasses->setText(tt::getIString(false,_intParameters[0]).c_str());
    ui->qqMultiplier->setText(tt::getEString(false,_floatParameters[25],2).c_str());
    ui->qq0->setText(tt::getEString(true,_floatParameters[0],2).c_str());
    ui->qq1->setText(tt::getEString(true,_floatParameters[1],2).c_str());
    ui->qq2->setText(tt::getEString(true,_floatParameters[2],2).c_str());
    ui->qq3->setText(tt::getEString(true,_floatParameters[3],2).c_str());
    ui->qq4->setText(tt::getEString(true,_floatParameters[4],2).c_str());
    ui->qq5->setText(tt::getEString(true,_floatParameters[5],2).c_str());
    ui->qq6->setText(tt::getEString(true,_floatParameters[6],2).c_str());
    ui->qq7->setText(tt::getEString(true,_floatParameters[7],2).c_str());
    ui->qq8->setText(tt::getEString(true,_floatParameters[8],2).c_str());
    ui->qq9->setText(tt::getEString(true,_floatParameters[9],2).c_str());
    ui->qq10->setText(tt::getEString(true,_floatParameters[10],2).c_str());
    ui->qq11->setText(tt::getEString(true,_floatParameters[11],2).c_str());
    ui->qq12->setText(tt::getEString(true,_floatParameters[12],2).c_str());
    ui->qq13->setText(tt::getEString(true,_floatParameters[13],2).c_str());
    ui->qq14->setText(tt::getEString(true,_floatParameters[14],2).c_str());
    ui->qq15->setText(tt::getEString(true,_floatParameters[15],2).c_str());
    ui->qq16->setText(tt::getEString(true,_floatParameters[16],2).c_str());
    ui->qq17->setText(tt::getEString(true,_floatParameters[17],2).c_str());
    ui->qq18->setText(tt::getEString(true,_floatParameters[18],2).c_str());
    ui->qq19->setText(tt::getEString(true,_floatParameters[19],2).c_str());
    ui->qq20->setText(tt::getEString(true,_floatParameters[20],2).c_str());
    ui->qq21->setText(tt::getEString(true,_floatParameters[21],2).c_str());
    ui->qq22->setText(tt::getEString(true,_floatParameters[22],2).c_str());
    ui->qq23->setText(tt::getEString(true,_floatParameters[23],2).c_str());
    ui->qq24->setText(tt::getEString(true,_floatParameters[24],2).c_str());
}

void CQDlgSimpleFilter_5x5::on_qqOkCancel_accepted()
{
    defaultModalDialogEndRoutine(true);
}

void CQDlgSimpleFilter_5x5::on_qqOkCancel_rejected()
{
    defaultModalDialogEndRoutine(false);
}

void CQDlgSimpleFilter_5x5::on_qqPasses_editingFinished()
{
    if (!ui->qqPasses->isModified())
        return;
    bool ok;
    int newVal=ui->qqPasses->text().toInt(&ok);
    if (ok)
    {
        tt::limitValue(1,10,newVal);
        _intParameters[0]=newVal;
    }
    refresh();
}

void CQDlgSimpleFilter_5x5::on_qqMultiplier_editingFinished()
{
    if (!ui->qqMultiplier->isModified())
        return;
    bool ok;
    float newVal=ui->qqMultiplier->text().toFloat(&ok);
    if (ok)
    {
        tt::limitValue(-100.0f,100.0f,newVal);
        _floatParameters[25]=newVal;
    }
    refresh();
}

void CQDlgSimpleFilter_5x5::on_qq0_editingFinished()
{
    if (!ui->qq0->isModified())
        return;
    bool ok;
    float newVal=ui->qq0->text().toFloat(&ok);
    if (ok)
    {
        tt::limitValue(-100.0f,100.0f,newVal);
        _floatParameters[0]=newVal;
    }
    refresh();
}

void CQDlgSimpleFilter_5x5::on_qq1_editingFinished()
{
    if (!ui->qq1->isModified())
        return;
    bool ok;
    float newVal=ui->qq1->text().toFloat(&ok);
    if (ok)
    {
        tt::limitValue(-100.0f,100.0f,newVal);
        _floatParameters[1]=newVal;
    }
    refresh();
}

void CQDlgSimpleFilter_5x5::on_qq2_editingFinished()
{
    if (!ui->qq2->isModified())
        return;
    bool ok;
    float newVal=ui->qq2->text().toFloat(&ok);
    if (ok)
    {
        tt::limitValue(-100.0f,100.0f,newVal);
        _floatParameters[2]=newVal;
    }
    refresh();
}

void CQDlgSimpleFilter_5x5::on_qq3_editingFinished()
{
    if (!ui->qq3->isModified())
        return;
    bool ok;
    float newVal=ui->qq3->text().toFloat(&ok);
    if (ok)
    {
        tt::limitValue(-100.0f,100.0f,newVal);
        _floatParameters[3]=newVal;
    }
    refresh();
}

void CQDlgSimpleFilter_5x5::on_qq4_editingFinished()
{
    if (!ui->qq4->isModified())
        return;
    bool ok;
    float newVal=ui->qq4->text().toFloat(&ok);
    if (ok)
    {
        tt::limitValue(-100.0f,100.0f,newVal);
        _floatParameters[4]=newVal;
    }
    refresh();
}

void CQDlgSimpleFilter_5x5::on_qq5_editingFinished()
{
    if (!ui->qq5->isModified())
        return;
    bool ok;
    float newVal=ui->qq5->text().toFloat(&ok);
    if (ok)
    {
        tt::limitValue(-100.0f,100.0f,newVal);
        _floatParameters[5]=newVal;
    }
    refresh();
}

void CQDlgSimpleFilter_5x5::on_qq6_editingFinished()
{
    if (!ui->qq6->isModified())
        return;
    bool ok;
    float newVal=ui->qq6->text().toFloat(&ok);
    if (ok)
    {
        tt::limitValue(-100.0f,100.0f,newVal);
        _floatParameters[6]=newVal;
    }
    refresh();
}

void CQDlgSimpleFilter_5x5::on_qq7_editingFinished()
{
    if (!ui->qq7->isModified())
        return;
    bool ok;
    float newVal=ui->qq7->text().toFloat(&ok);
    if (ok)
    {
        tt::limitValue(-100.0f,100.0f,newVal);
        _floatParameters[7]=newVal;
    }
    refresh();
}

void CQDlgSimpleFilter_5x5::on_qq8_editingFinished()
{
    if (!ui->qq8->isModified())
        return;
    bool ok;
    float newVal=ui->qq8->text().toFloat(&ok);
    if (ok)
    {
        tt::limitValue(-100.0f,100.0f,newVal);
        _floatParameters[8]=newVal;
    }
    refresh();
}

void CQDlgSimpleFilter_5x5::on_qq9_editingFinished()
{
    if (!ui->qq9->isModified())
        return;
    bool ok;
    float newVal=ui->qq9->text().toFloat(&ok);
    if (ok)
    {
        tt::limitValue(-100.0f,100.0f,newVal);
        _floatParameters[9]=newVal;
    }
    refresh();
}

void CQDlgSimpleFilter_5x5::on_qq10_editingFinished()
{
    if (!ui->qq10->isModified())
        return;
    bool ok;
    float newVal=ui->qq10->text().toFloat(&ok);
    if (ok)
    {
        tt::limitValue(-100.0f,100.0f,newVal);
        _floatParameters[10]=newVal;
    }
    refresh();
}

void CQDlgSimpleFilter_5x5::on_qq11_editingFinished()
{
    if (!ui->qq11->isModified())
        return;
    bool ok;
    float newVal=ui->qq11->text().toFloat(&ok);
    if (ok)
    {
        tt::limitValue(-100.0f,100.0f,newVal);
        _floatParameters[11]=newVal;
    }
    refresh();
}

void CQDlgSimpleFilter_5x5::on_qq12_editingFinished()
{
    if (!ui->qq12->isModified())
        return;
    bool ok;
    float newVal=ui->qq12->text().toFloat(&ok);
    if (ok)
    {
        tt::limitValue(-100.0f,100.0f,newVal);
        _floatParameters[12]=newVal;
    }
    refresh();
}

void CQDlgSimpleFilter_5x5::on_qq13_editingFinished()
{
    if (!ui->qq13->isModified())
        return;
    bool ok;
    float newVal=ui->qq13->text().toFloat(&ok);
    if (ok)
    {
        tt::limitValue(-100.0f,100.0f,newVal);
        _floatParameters[13]=newVal;
    }
    refresh();
}

void CQDlgSimpleFilter_5x5::on_qq14_editingFinished()
{
    if (!ui->qq14->isModified())
        return;
    bool ok;
    float newVal=ui->qq14->text().toFloat(&ok);
    if (ok)
    {
        tt::limitValue(-100.0f,100.0f,newVal);
        _floatParameters[14]=newVal;
    }
    refresh();
}

void CQDlgSimpleFilter_5x5::on_qq15_editingFinished()
{
    if (!ui->qq15->isModified())
        return;
    bool ok;
    float newVal=ui->qq15->text().toFloat(&ok);
    if (ok)
    {
        tt::limitValue(-100.0f,100.0f,newVal);
        _floatParameters[15]=newVal;
    }
    refresh();
}

void CQDlgSimpleFilter_5x5::on_qq16_editingFinished()
{
    if (!ui->qq16->isModified())
        return;
    bool ok;
    float newVal=ui->qq16->text().toFloat(&ok);
    if (ok)
    {
        tt::limitValue(-100.0f,100.0f,newVal);
        _floatParameters[16]=newVal;
    }
    refresh();
}

void CQDlgSimpleFilter_5x5::on_qq17_editingFinished()
{
    if (!ui->qq17->isModified())
        return;
    bool ok;
    float newVal=ui->qq17->text().toFloat(&ok);
    if (ok)
    {
        tt::limitValue(-100.0f,100.0f,newVal);
        _floatParameters[17]=newVal;
    }
    refresh();
}

void CQDlgSimpleFilter_5x5::on_qq18_editingFinished()
{
    if (!ui->qq18->isModified())
        return;
    bool ok;
    float newVal=ui->qq18->text().toFloat(&ok);
    if (ok)
    {
        tt::limitValue(-100.0f,100.0f,newVal);
        _floatParameters[18]=newVal;
    }
    refresh();
}

void CQDlgSimpleFilter_5x5::on_qq19_editingFinished()
{
    if (!ui->qq19->isModified())
        return;
    bool ok;
    float newVal=ui->qq19->text().toFloat(&ok);
    if (ok)
    {
        tt::limitValue(-100.0f,100.0f,newVal);
        _floatParameters[19]=newVal;
    }
    refresh();
}

void CQDlgSimpleFilter_5x5::on_qq20_editingFinished()
{
    if (!ui->qq20->isModified())
        return;
    bool ok;
    float newVal=ui->qq20->text().toFloat(&ok);
    if (ok)
    {
        tt::limitValue(-100.0f,100.0f,newVal);
        _floatParameters[20]=newVal;
    }
    refresh();
}

void CQDlgSimpleFilter_5x5::on_qq21_editingFinished()
{
    if (!ui->qq21->isModified())
        return;
    bool ok;
    float newVal=ui->qq21->text().toFloat(&ok);
    if (ok)
    {
        tt::limitValue(-100.0f,100.0f,newVal);
        _floatParameters[21]=newVal;
    }
    refresh();
}

void CQDlgSimpleFilter_5x5::on_qq22_editingFinished()
{
    if (!ui->qq22->isModified())
        return;
    bool ok;
    float newVal=ui->qq22->text().toFloat(&ok);
    if (ok)
    {
        tt::limitValue(-100.0f,100.0f,newVal);
        _floatParameters[22]=newVal;
    }
    refresh();
}

void CQDlgSimpleFilter_5x5::on_qq23_editingFinished()
{
    if (!ui->qq23->isModified())
        return;
    bool ok;
    float newVal=ui->qq23->text().toFloat(&ok);
    if (ok)
    {
        tt::limitValue(-100.0f,100.0f,newVal);
        _floatParameters[23]=newVal;
    }
    refresh();
}

void CQDlgSimpleFilter_5x5::on_qq24_editingFinished()
{
    if (!ui->qq24->isModified())
        return;
    bool ok;
    float newVal=ui->qq24->text().toFloat(&ok);
    if (ok)
    {
        tt::limitValue(-100.0f,100.0f,newVal);
        _floatParameters[24]=newVal;
    }
    refresh();
}
