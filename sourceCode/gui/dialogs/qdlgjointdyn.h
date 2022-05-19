
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
/*
    void on_qqMotorEnabled_clicked();





    void on_qqControlEnabled_clicked();




    void on_qqApplyControlParameters_clicked();

    void on_qqPositionControl_clicked();

    void on_qqSpringControl_clicked();


*/
    void on_qqApplyParams_clicked();
    void on_qqCtrlMode_free_clicked();
    void on_qqCtrlMode_force_clicked();
    void on_qqForceMode_force_editingFinished();
    void on_qqCtrlMode_velocity_clicked();
    void on_qqVelocityMode_velocity_editingFinished();
    void on_qqVelocityMode_force_editingFinished();
    void on_qqVelocityMode_motorLock_clicked();
    void on_qqCtrlMode_position_clicked();
    void on_qqPositionMode_position_editingFinished();
    void on_qqPositionMode_force_editingFinished();
    void on_qqPositionMode_upperVel_editingFinished();
    void on_qqP_editingFinished();
    void on_qqI_editingFinished();
    void on_qqD_editingFinished();
    void on_qqPositionMode_cb_clicked();
    void on_qqCtrlMode_spring_clicked();
    void on_qqSpringMode_position_editingFinished();
    void on_qqSpringMode_force_editingFinished();
    void on_qqK_editingFinished();
    void on_qqC_editingFinished();
    void on_qqSpringMode_cb_clicked();
    void on_qqCtrlMode_custom_clicked();
    void on_qqAdjustEngineProperties_clicked();

private:
    Ui::CQDlgJointDyn *ui;
};

#endif // QDLGJOINTDYN_H
