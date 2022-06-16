
#ifndef QDLGJOINTDYN_H
#define QDLGJOINTDYN_H

#include "dlgEx.h"

namespace Ui {
    class CQDlgJointDyn;
}

class CQDlgJointDyn : public CDlgEx
{
    Q_OBJECT

public:
    explicit CQDlgJointDyn(QWidget *parent = 0);
    ~CQDlgJointDyn();

    void refresh();
    void cancelEvent();

private slots:
    void on_qqApplyParams_clicked();
    void on_qqForceMode_force_editingFinished();
    void on_qqVelocityMode_velocity_editingFinished();
    void on_qqVelocityMode_force_editingFinished();
    void on_qqVelocityMode_motorLock_clicked();
    void on_qqPositionMode_position_editingFinished();
    void on_qqPositionMode_force_editingFinished();
    void on_qqPositionMode_upperVel_editingFinished();
    void on_qqP_editingFinished();
    void on_qqI_editingFinished();
    void on_qqD_editingFinished();
    void on_qqPositionMode_cb_clicked();
    void on_qqSpringMode_position_editingFinished();
    void on_qqK_editingFinished();
    void on_qqC_editingFinished();
    void on_qqSpringMode_cb_clicked();
    void on_qqAdjustEngineProperties_clicked();

    void on_qqCombo_currentIndexChanged(int index);

    void on_qqPositionMode_pid_clicked();

    void on_qqPositionMode_ruckig_clicked();

    void on_qqPositionMode_maxVel_editingFinished();

    void on_qqPositionMode_maxAccel_editingFinished();

    void on_qqPositionMode_maxJerk_editingFinished();

    void on_qqVelocityMode_ruckig_clicked();

    void on_qqVelocityMode_maxAccel_editingFinished();

    void on_qqVelocityMode_maxJerk_editingFinished();

private:
    bool inMainRefreshRoutine;

    Ui::CQDlgJointDyn *ui;
};

#endif // QDLGJOINTDYN_H
