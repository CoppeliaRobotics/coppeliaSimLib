
#ifndef QDLGPROXSENSDETECTIONPARAM_H
#define QDLGPROXSENSDETECTIONPARAM_H

#include "vDialog.h"

namespace Ui {
    class CQDlgProxSensDetectionParam;
}

class CQDlgProxSensDetectionParam : public VDialog
{
    Q_OBJECT

public:
    explicit CQDlgProxSensDetectionParam(QWidget *parent = 0);
    ~CQDlgProxSensDetectionParam();

    void cancelEvent();
    void okEvent();

    void refresh();

    bool frontFace;
    bool backFace;
    bool fast;
    bool limitedAngle;
    float angle;
//    bool occlusionCheck;
    bool distanceContraint;
    float minimumDistance;
    bool randomizedDetection;
    int rayCount;
    int rayDetectionCount;

private slots:
    void on_qqFrontFace_clicked();

    void on_qqBackFace_clicked();

    void on_qqFast_clicked();

    void on_qqLimitedAngle_clicked();

    void on_qqAngle_editingFinished();

//    void on_qqOcclusionCheck_clicked();

    void on_qqMinDistEnabled_clicked();

    void on_qqMinDist_editingFinished();

    void on_qqRayCount_editingFinished();

    void on_qqRandomizedDetectionCount_editingFinished();

    void on_qqOkCancel_accepted();

    void on_qqOkCancel_rejected();

private:
    Ui::CQDlgProxSensDetectionParam *ui;
};

#endif // QDLGPROXSENSDETECTIONPARAM_H
