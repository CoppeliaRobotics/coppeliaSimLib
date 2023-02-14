#include <qdlginertiatensor.h>
#include <ui_qdlginertiatensor.h>
#include <simInternal.h>
#include <app.h>
#include <tt.h>
#include <utils.h>

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
    bool severalShapes=App::currentWorld->sceneObjects->getShapeCountInSelection()>1;
    if (!severalShapes)
        applyToSelection=false;
    ui->qqI11->setText(utils::getMasslessTensorString(true,tensor[0]).c_str());
    ui->qqI21->setText(utils::getMasslessTensorString(true,tensor[1]).c_str());
    ui->qqI31->setText(utils::getMasslessTensorString(true,tensor[2]).c_str());
    ui->qqI12->setText(utils::getMasslessTensorString(true,tensor[3]).c_str());
    ui->qqI22->setText(utils::getMasslessTensorString(true,tensor[4]).c_str());
    ui->qqI32->setText(utils::getMasslessTensorString(true,tensor[5]).c_str());
    ui->qqI13->setText(utils::getMasslessTensorString(true,tensor[6]).c_str());
    ui->qqI23->setText(utils::getMasslessTensorString(true,tensor[7]).c_str());
    ui->qqI33->setText(utils::getMasslessTensorString(true,tensor[8]).c_str());

    ui->qqMx->setText(utils::getPosString(true,com[0]).c_str());
    ui->qqMy->setText(utils::getPosString(true,com[1]).c_str());
    ui->qqMz->setText(utils::getPosString(true,com[2]).c_str());

    ui->qqApplyToSelection->setChecked(applyToSelection);
    ui->qqApplyToSelection->setEnabled(severalShapes);
}

void CQDlgInertiaTensor::on_qqI11_editingFinished()
{
    if (!ui->qqI11->isModified())
        return;
    bool ok;
    double newVal=ui->qqI11->text().toDouble(&ok);
    if (ok)
        tensor[0]=newVal;
    refresh();
}

void CQDlgInertiaTensor::on_qqI21_editingFinished()
{
    if (!ui->qqI21->isModified())
        return;
    bool ok;
    double newVal=ui->qqI21->text().toDouble(&ok);
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
    double newVal=ui->qqI31->text().toDouble(&ok);
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
    double newVal=ui->qqI12->text().toDouble(&ok);
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
    double newVal=ui->qqI22->text().toDouble(&ok);
    if (ok)
        tensor[4]=newVal;
    refresh();
}

void CQDlgInertiaTensor::on_qqI32_editingFinished()
{
    if (!ui->qqI32->isModified())
        return;
    bool ok;
    double newVal=ui->qqI32->text().toDouble(&ok);
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
    double newVal=ui->qqI13->text().toDouble(&ok);
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
    double newVal=ui->qqI23->text().toDouble(&ok);
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
    double newVal=ui->qqI33->text().toDouble(&ok);
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
    double newVal=ui->qqMx->text().toDouble(&ok);
    if (ok)
        com[0]=newVal;
    refresh();
}

void CQDlgInertiaTensor::on_qqMy_editingFinished()
{
    if (!ui->qqMy->isModified())
        return;
    bool ok;
    double newVal=ui->qqMy->text().toDouble(&ok);
    if (ok)
        com[1]=newVal;
    refresh();
}

void CQDlgInertiaTensor::on_qqMz_editingFinished()
{
    if (!ui->qqMz->isModified())
        return;
    bool ok;
    double newVal=ui->qqMz->text().toDouble(&ok);
    if (ok)
        com[2]=newVal;
    refresh();
}

void CQDlgInertiaTensor::on_qqApplyToSelection_clicked(bool checked)
{
    applyToSelection=checked;
    refresh();
}
