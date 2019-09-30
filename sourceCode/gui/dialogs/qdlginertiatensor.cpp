#include "qdlginertiatensor.h"
#include "ui_qdlginertiatensor.h"
#include "v_rep_internal.h"
#include "app.h"
#include "tt.h"
#include "gV.h"

CQDlgInertiaTensor::CQDlgInertiaTensor(QWidget *parent) :
    VDialog(parent,QT_MODAL_DLG_STYLE),
    ui(new Ui::CQDlgInertiaTensor)
{
    ui->setupUi(this);
}

CQDlgInertiaTensor::~CQDlgInertiaTensor()
{
    delete ui;
}

void CQDlgInertiaTensor::cancelEvent()
{
    defaultModalDialogEndRoutine(false);
}

void CQDlgInertiaTensor::okEvent()
{
//  defaultModalDialogEndRoutine(true);
}

void CQDlgInertiaTensor::refresh()
{
    bool severalShapes=App::ct->objCont->getShapeNumberInSelection()>1;
    if (!severalShapes)
        applyToSelection=false;
    ui->qqI11->setText(tt::getEString(true,tensor[0],3).c_str());
    ui->qqI21->setText(tt::getEString(true,tensor[1],3).c_str());
    ui->qqI31->setText(tt::getEString(true,tensor[2],3).c_str());
    ui->qqI12->setText(tt::getEString(true,tensor[3],3).c_str());
    ui->qqI22->setText(tt::getEString(true,tensor[4],3).c_str());
    ui->qqI32->setText(tt::getEString(true,tensor[5],3).c_str());
    ui->qqI13->setText(tt::getEString(true,tensor[6],3).c_str());
    ui->qqI23->setText(tt::getEString(true,tensor[7],3).c_str());
    ui->qqI33->setText(tt::getEString(true,tensor[8],3).c_str());

    ui->qqMx->setText(tt::getEString(true,com[0],3).c_str());
    ui->qqMy->setText(tt::getEString(true,com[1],3).c_str());
    ui->qqMz->setText(tt::getEString(true,com[2],3).c_str());

    ui->qqApplyToSelection->setChecked(applyToSelection);
    ui->qqApplyToSelection->setEnabled(severalShapes);
}

void CQDlgInertiaTensor::on_qqI11_editingFinished()
{
    if (!ui->qqI11->isModified())
        return;
    bool ok;
    float newVal=ui->qqI11->text().toFloat(&ok);
    if (ok)
        tensor[0]=newVal;
    refresh();
}

void CQDlgInertiaTensor::on_qqI21_editingFinished()
{
    if (!ui->qqI21->isModified())
        return;
    bool ok;
    float newVal=ui->qqI21->text().toFloat(&ok);
    if (ok)
    {
        tensor[1]=newVal;
        tensor[3]=newVal;
    }
    refresh();
}

void CQDlgInertiaTensor::on_qqI31_editingFinished()
{
    if (!ui->qqI31->isModified())
        return;
    bool ok;
    float newVal=ui->qqI31->text().toFloat(&ok);
    if (ok)
    {
        tensor[2]=newVal;
        tensor[6]=newVal;
    }
    refresh();
}

void CQDlgInertiaTensor::on_qqI12_editingFinished()
{
    if (!ui->qqI12->isModified())
        return;
    bool ok;
    float newVal=ui->qqI12->text().toFloat(&ok);
    if (ok)
    {
        tensor[3]=newVal;
        tensor[1]=newVal;
    }
    refresh();
}

void CQDlgInertiaTensor::on_qqI22_editingFinished()
{
    if (!ui->qqI22->isModified())
        return;
    bool ok;
    float newVal=ui->qqI22->text().toFloat(&ok);
    if (ok)
        tensor[4]=newVal;
    refresh();
}

void CQDlgInertiaTensor::on_qqI32_editingFinished()
{
    if (!ui->qqI32->isModified())
        return;
    bool ok;
    float newVal=ui->qqI32->text().toFloat(&ok);
    if (ok)
    {
        tensor[5]=newVal;
        tensor[7]=newVal;
    }
    refresh();
}

void CQDlgInertiaTensor::on_qqI13_editingFinished()
{
    if (!ui->qqI13->isModified())
        return;
    bool ok;
    float newVal=ui->qqI13->text().toFloat(&ok);
    if (ok)
    {
        tensor[6]=newVal;
        tensor[2]=newVal;
    }
    refresh();
}

void CQDlgInertiaTensor::on_qqI23_editingFinished()
{
    if (!ui->qqI23->isModified())
        return;
    bool ok;
    float newVal=ui->qqI23->text().toFloat(&ok);
    if (ok)
    {
        tensor[7]=newVal;
        tensor[5]=newVal;
    }
    refresh();
}

void CQDlgInertiaTensor::on_qqI33_editingFinished()
{
    if (!ui->qqI33->isModified())
        return;
    bool ok;
    float newVal=ui->qqI33->text().toFloat(&ok);
    if (ok)
        tensor[8]=newVal;
    refresh();
}

void CQDlgInertiaTensor::on_qqOkCancel_accepted()
{
    defaultModalDialogEndRoutine(true);
}

void CQDlgInertiaTensor::on_qqOkCancel_rejected()
{
    defaultModalDialogEndRoutine(false);
}

void CQDlgInertiaTensor::on_qqMx_editingFinished()
{
    if (!ui->qqMx->isModified())
        return;
    bool ok;
    float newVal=ui->qqMx->text().toFloat(&ok);
    if (ok)
        com[0]=newVal;
    refresh();
}

void CQDlgInertiaTensor::on_qqMy_editingFinished()
{
    if (!ui->qqMy->isModified())
        return;
    bool ok;
    float newVal=ui->qqMy->text().toFloat(&ok);
    if (ok)
        com[1]=newVal;
    refresh();
}

void CQDlgInertiaTensor::on_qqMz_editingFinished()
{
    if (!ui->qqMz->isModified())
        return;
    bool ok;
    float newVal=ui->qqMz->text().toFloat(&ok);
    if (ok)
        com[2]=newVal;
    refresh();
}

void CQDlgInertiaTensor::on_qqApplyToSelection_clicked(bool checked)
{
    applyToSelection=checked;
    refresh();
}
