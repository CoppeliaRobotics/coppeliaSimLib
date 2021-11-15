
#ifndef QDLGFORCESENSORS_H
#define QDLGFORCESENSORS_H

#include "dlgEx.h"

namespace Ui {
    class CQDlgForceSensors;
}

class CQDlgForceSensors : public CDlgEx
{
    Q_OBJECT

public:
    explicit CQDlgForceSensors(QWidget *parent = 0);
    ~CQDlgForceSensors();

    void cancelEvent();

    void refresh();

private slots:
    void on_qqSize_editingFinished();

    void on_qqColorA_clicked();

    void on_qqApplyMain_clicked();

    void on_qqSampleSize_editingFinished();

    void on_qqAverage_clicked();

    void on_qqMedian_clicked();

    void on_qqApplyFilter_clicked();

    void on_qqForceThresholdEnable_clicked();

    void on_qqForceThreshold_editingFinished();

    void on_qqTorqueThresholdEnable_clicked();

    void on_qqTorqueThreshold_editingFinished();

    void on_qqViolationCount_editingFinished();

    void on_qqApplyBreaking_clicked();

private:
    Ui::CQDlgForceSensors *ui;
};

#endif // QDLGFORCESENSORS_H
