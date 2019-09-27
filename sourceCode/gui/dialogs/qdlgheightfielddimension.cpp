
#include "vrepMainHeader.h"
#include "qdlgheightfielddimension.h"
#include "ui_qdlgheightfielddimension.h"
#include "gV.h"
#include "tt.h"
#include "app.h"

CQDlgHeightfieldDimension::CQDlgHeightfieldDimension(QWidget *parent) :
    VDialog(parent,QT_MODAL_DLG_STYLE),
    ui(new Ui::CQDlgHeightfieldDimension)
{
    ui->setupUi(this);
    xSize=1.0f;
    ySize=1.0f;
    zScaling=1.0f;
    xSizeTimesThisGivesYSize=1.0f;
}

CQDlgHeightfieldDimension::~CQDlgHeightfieldDimension()
{
    delete ui;
}

void CQDlgHeightfieldDimension::cancelEvent()
{
//  defaultModalDialogEndRoutine(false);
}

void CQDlgHeightfieldDimension::okEvent()
{
//  defaultModalDialogEndRoutine(true);
}

void CQDlgHeightfieldDimension::refresh()
{
    ui->qqSizeX->setText(tt::getFString(false,xSize,2).c_str());
    ui->qqSizeY->setText(tt::getFString(false,ySize,2).c_str());
    ui->qqScalingZ->setText(tt::getFString(false,zScaling,3).c_str());
}

void CQDlgHeightfieldDimension::on_qqSizeX_editingFinished()
{
    if (!ui->qqSizeX->isModified())
        return;
    bool ok;
    float newVal=ui->qqSizeX->text().toFloat(&ok);
    if (ok)
    {
        tt::limitValue(0.01f,10000.0f,newVal);
        xSize=newVal;
        ySize=newVal*xSizeTimesThisGivesYSize;
    }
    refresh();
}

void CQDlgHeightfieldDimension::on_qqSizeY_editingFinished()
{
    if (!ui->qqSizeY->isModified())
        return;
    bool ok;
    float newVal=ui->qqSizeY->text().toFloat(&ok);
    if (ok)
    {
        tt::limitValue(0.01f,10000.0f,newVal);
        ySize=newVal;
        xSize=newVal/xSizeTimesThisGivesYSize;
    }
    refresh();
}

void CQDlgHeightfieldDimension::on_qqScalingZ_editingFinished()
{
    if (!ui->qqScalingZ->isModified())
        return;
    bool ok;
    float newVal=ui->qqScalingZ->text().toFloat(&ok);
    if (ok)
    {
        tt::limitValue(0.001f,1000.0f,newVal);
        zScaling=newVal;
    }
    refresh();
}

void CQDlgHeightfieldDimension::on_qqOk_accepted()
{
    defaultModalDialogEndRoutine(true);
}
