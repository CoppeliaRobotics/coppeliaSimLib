#include "qdlgimagecolor.h"
#include "ui_qdlgimagecolor.h"
#include "tt.h"
#include "app.h"

CQDlgImageColor::CQDlgImageColor(QWidget *parent) :
    VDialog(parent,QT_MODAL_DLG_STYLE),
    ui(new Ui::CQDlgImageColor)
{
    ui->setupUi(this);
}

CQDlgImageColor::~CQDlgImageColor()
{
    delete ui;
}

void CQDlgImageColor::cancelEvent()
{
    defaultModalDialogEndRoutine(false);
}

void CQDlgImageColor::okEvent()
{
    defaultModalDialogEndRoutine(true);
}

void CQDlgImageColor::refresh()
{
    ui->qqFromFogColor->setChecked(sameAsFog);
    ui->qqRed->setEnabled(!sameAsFog);
    ui->qqGreen->setEnabled(!sameAsFog);
    ui->qqBlue->setEnabled(!sameAsFog);
    ui->qqRed->setText(tt::getFString(false,red,2).c_str());
    ui->qqGreen->setText(tt::getFString(false,green,2).c_str());
    ui->qqBlue->setText(tt::getFString(false,blue,2).c_str());
}

void CQDlgImageColor::on_qqFromFogColor_clicked()
{
    sameAsFog=!sameAsFog;
    refresh();
}

void CQDlgImageColor::on_qqRed_editingFinished()
{
    if (!ui->qqRed->isModified())
        return;
    bool ok;
    double newVal=ui->qqRed->text().toFloat(&ok);
    if (ok)
        red=tt::getLimitedFloat(0.0,1.0,newVal);
    refresh();
}

void CQDlgImageColor::on_qqGreen_editingFinished()
{
    if (!ui->qqGreen->isModified())
        return;
    bool ok;
    double newVal=ui->qqGreen->text().toFloat(&ok);
    if (ok)
        green=tt::getLimitedFloat(0.0,1.0,newVal);
    refresh();
}

void CQDlgImageColor::on_qqBlue_editingFinished()
{
    if (!ui->qqBlue->isModified())
        return;
    bool ok;
    double newVal=ui->qqBlue->text().toFloat(&ok);
    if (ok)
        blue=tt::getLimitedFloat(0.0,1.0,newVal);
    refresh();
}

void CQDlgImageColor::on_qqOkCancel_accepted()
{
    defaultModalDialogEndRoutine(true);
}

void CQDlgImageColor::on_qqOkCancel_rejected()
{
    defaultModalDialogEndRoutine(false);
}
