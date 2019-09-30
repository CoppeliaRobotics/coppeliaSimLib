#include "qdlgsimplefilter_uniformImage.h"
#include "ui_qdlgsimplefilter_uniformImage.h"
#include "gV.h"
#include "tt.h"

CQDlgSimpleFilter_uniformImage::CQDlgSimpleFilter_uniformImage(QWidget *parent) :
    CQDlgSimpleFilter(parent),
    ui(new Ui::CQDlgSimpleFilter_uniformImage)
{
    ui->setupUi(this);
}

CQDlgSimpleFilter_uniformImage::~CQDlgSimpleFilter_uniformImage()
{
    delete ui;
}

void CQDlgSimpleFilter_uniformImage::refresh()
{
    ui->qqRed->setText(tt::getFString(false,_floatParameters[0],3).c_str());
    ui->qqGreen->setText(tt::getFString(false,_floatParameters[1],3).c_str());
    ui->qqBlue->setText(tt::getFString(false,_floatParameters[2],3).c_str());
}

void CQDlgSimpleFilter_uniformImage::on_qqOkCancel_accepted()
{
    defaultModalDialogEndRoutine(true);
}

void CQDlgSimpleFilter_uniformImage::on_qqOkCancel_rejected()
{
    defaultModalDialogEndRoutine(false);
}

void CQDlgSimpleFilter_uniformImage::on_qqRed_editingFinished()
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

void CQDlgSimpleFilter_uniformImage::on_qqGreen_editingFinished()
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

void CQDlgSimpleFilter_uniformImage::on_qqBlue_editingFinished()
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
