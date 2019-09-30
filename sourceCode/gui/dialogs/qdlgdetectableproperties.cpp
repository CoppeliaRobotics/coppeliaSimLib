#include "qdlgdetectableproperties.h"
#include "ui_qdlgdetectableproperties.h"
#include "v_rep_internal.h"
#include "app.h"

CQDlgDetectableProperties::CQDlgDetectableProperties(QWidget *parent) :
    VDialog(parent,QT_MODAL_DLG_STYLE),
    ui(new Ui::CQDlgDetectableProperties)
{
    ui->setupUi(this);
    initializationEvent();
}

CQDlgDetectableProperties::~CQDlgDetectableProperties()
{
    delete ui;
}

void CQDlgDetectableProperties::cancelEvent()
{
    defaultModalDialogEndRoutine(false);
}

void CQDlgDetectableProperties::okEvent()
{
    defaultModalDialogEndRoutine(true);
}

void CQDlgDetectableProperties::initializationEvent()
{
    defaultDialogInitializationRoutine();
}

void CQDlgDetectableProperties::refresh()
{
    ui->qqCapacitive->setChecked((objectProperties&sim_objectspecialproperty_detectable_capacitive)!=0);
    ui->qqInductive->setChecked((objectProperties&sim_objectspecialproperty_detectable_inductive)!=0);
    ui->qqInfrared->setChecked((objectProperties&sim_objectspecialproperty_detectable_infrared)!=0);
    ui->qqLaser->setChecked((objectProperties&sim_objectspecialproperty_detectable_laser)!=0);
    ui->qqUltrasonic->setChecked((objectProperties&sim_objectspecialproperty_detectable_ultrasonic)!=0);
}

void CQDlgDetectableProperties::on_qqUltrasonic_clicked()
{
    objectProperties^=sim_objectspecialproperty_detectable_ultrasonic;
    refresh();
}

void CQDlgDetectableProperties::on_qqInductive_clicked()
{
    objectProperties^=sim_objectspecialproperty_detectable_inductive;
    refresh();
}

void CQDlgDetectableProperties::on_qqInfrared_clicked()
{
    objectProperties^=sim_objectspecialproperty_detectable_infrared;
    refresh();
}

void CQDlgDetectableProperties::on_qqCapacitive_clicked()
{
    objectProperties^=sim_objectspecialproperty_detectable_capacitive;
    refresh();
}

void CQDlgDetectableProperties::on_qqLaser_clicked()
{
    objectProperties^=sim_objectspecialproperty_detectable_laser;
    refresh();
}

void CQDlgDetectableProperties::on_qqOkCancel_accepted()
{
    defaultModalDialogEndRoutine(true);
}

void CQDlgDetectableProperties::on_qqOkCancel_rejected()
{
    defaultModalDialogEndRoutine(false);
}
