#include <qdlgheightfielddimension.h>
#include <ui_qdlgheightfielddimension.h>
#include <tt.h>
#include <utils.h>
#include <app.h>

CQDlgHeightfieldDimension::CQDlgHeightfieldDimension(QWidget *parent) :
    VDialog(parent,QT_MODAL_DLG_STYLE),
    ui(new Ui::CQDlgHeightfieldDimension)
{
    ui->setupUi(this);
    xSize=1.0;
    ySize=1.0;
    zScaling=1.0;
    xSizeTimesThisGivesYSize=1.0;
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
    ui->qqSizeX->setText(utils::getSizeString(false,xSize).c_str());
    ui->qqSizeY->setText(utils::getSizeString(false,ySize).c_str());
    ui->qqScalingZ->setText(utils::getMultString(false,zScaling).c_str());
}

void CQDlgHeightfieldDimension::on_qqSizeX_editingFinished()
{
    if (!ui->qqSizeX->isModified())
        return;
    bool ok;
    double newVal=ui->qqSizeX->text().toDouble(&ok);
    if (ok)
    {
        tt::limitValue(0.01,10000.0,newVal);
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
    double newVal=ui->qqSizeY->text().toDouble(&ok);
    if (ok)
    {
        tt::limitValue(0.01,10000.0,newVal);
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
    double newVal=ui->qqScalingZ->text().toDouble(&ok);
    if (ok)
    {
        tt::limitValue(0.001,1000.0,newVal);
        zScaling=newVal;
    }
    refresh();
}

void CQDlgHeightfieldDimension::on_qqOk_accepted()
{
    defaultModalDialogEndRoutine(true);
}
