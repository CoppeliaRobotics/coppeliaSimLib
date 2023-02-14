#include <qdlgproxsensdetectionparam.h>
#include <ui_qdlgproxsensdetectionparam.h>
#include <tt.h>
#include <utils.h>
#include <simMath/mathDefines.h>
#include <app.h>

CQDlgProxSensDetectionParam::CQDlgProxSensDetectionParam(QWidget *parent) :
    VDialog(parent,QT_MODAL_DLG_STYLE),
    ui(new Ui::CQDlgProxSensDetectionParam)
{
    ui->setupUi(this);
}

CQDlgProxSensDetectionParam::~CQDlgProxSensDetectionParam()
{
    delete ui;
}

void CQDlgProxSensDetectionParam::cancelEvent()
{
    defaultModalDialogEndRoutine(false);
}

void CQDlgProxSensDetectionParam::okEvent()
{
    defaultModalDialogEndRoutine(true);
}

void CQDlgProxSensDetectionParam::refresh()
{
    ui->qqFrontFace->setChecked(frontFace);
    ui->qqBackFace->setChecked(backFace);
    ui->qqFast->setChecked(fast);
    ui->qqLimitedAngle->setChecked(limitedAngle);
    ui->qqAngle->setEnabled(limitedAngle);
    ui->qqAngle->setText(utils::getAngleString(false,angle).c_str());
    ui->qqMinDistEnabled->setChecked(distanceContraint);
    ui->qqMinDist->setEnabled(distanceContraint);
    ui->qqMinDist->setText(utils::getSizeString(false,minimumDistance).c_str());
    ui->qqRayCount->setEnabled(randomizedDetection);
    ui->qqRandomizedDetectionCount->setEnabled(randomizedDetection);
    if (randomizedDetection)
    {
        ui->qqRayCount->setText(utils::getIntString(false,rayCount).c_str());
        ui->qqRandomizedDetectionCount->setText(utils::getIntString(false,rayDetectionCount).c_str());
    }
    else
    {
        ui->qqRayCount->setText("");
        ui->qqRandomizedDetectionCount->setText("");
    }
}

void CQDlgProxSensDetectionParam::on_qqFrontFace_clicked()
{
    frontFace=!frontFace;
    if (!frontFace)
        backFace=true;
    refresh();
}

void CQDlgProxSensDetectionParam::on_qqBackFace_clicked()
{
    backFace=!backFace;
    if (!backFace)
        frontFace=true;
    refresh();
}

void CQDlgProxSensDetectionParam::on_qqFast_clicked()
{
    fast=!fast;
    refresh();
}

void CQDlgProxSensDetectionParam::on_qqLimitedAngle_clicked()
{
    limitedAngle=!limitedAngle;
    refresh();
}

void CQDlgProxSensDetectionParam::on_qqAngle_editingFinished()
{
    bool ok;
    double newVal=ui->qqAngle->text().toDouble(&ok);
    if (ok)
    {
        angle=(newVal*degToRad);
        tt::limitValue(0.1*degToRad,90.0*degToRad,angle);
    }
    refresh();
}
/*
void CQDlgProxSensDetectionParam::on_qqOcclusionCheck_clicked()
{
    occlusionCheck=!occlusionCheck;
    refresh();
}
*/
void CQDlgProxSensDetectionParam::on_qqMinDistEnabled_clicked()
{
    distanceContraint=!distanceContraint;
    refresh();
}

void CQDlgProxSensDetectionParam::on_qqMinDist_editingFinished()
{
    bool ok;
    double newVal=ui->qqMinDist->text().toDouble(&ok);
    if (ok)
        minimumDistance=(newVal);
    refresh();
}

void CQDlgProxSensDetectionParam::on_qqRayCount_editingFinished()
{
    bool ok;
    int newVal=ui->qqRayCount->text().toInt(&ok);
    if (ok)
    {
        rayCount=tt::getLimitedInt(1,5000,newVal);
        rayDetectionCount=tt::getLimitedInt(1,rayCount,rayDetectionCount);
    }
    refresh();
}

void CQDlgProxSensDetectionParam::on_qqRandomizedDetectionCount_editingFinished()
{
    bool ok;
    int newVal=ui->qqRandomizedDetectionCount->text().toInt(&ok);
    if (ok)
        rayDetectionCount=tt::getLimitedInt(1,rayCount,newVal);
    refresh();
}

void CQDlgProxSensDetectionParam::on_qqOkCancel_accepted()
{
    defaultModalDialogEndRoutine(true);
}

void CQDlgProxSensDetectionParam::on_qqOkCancel_rejected()
{
    defaultModalDialogEndRoutine(false);
}
