
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
    void on_qqMotorEnabled_clicked();

    void on_qqTargetVelocity_editingFinished();

    void on_qqAdjustEngineProperties_clicked();

    void on_qqMaxForce_editingFinished();

    void on_qqApplyDynamicProperties_clicked();

    void on_qqControlEnabled_clicked();

    void on_qqVelocityUpperLimit_editingFinished();

    void on_qqTargetPosition_editingFinished();

    void on_qqP_editingFinished();

    void on_qqI_editingFinished();

    void on_qqD_editingFinished();

    void on_qqApplyControlParameters_clicked();

    void on_qqPositionControl_clicked();

    void on_qqSpringControl_clicked();

    void on_qqK_editingFinished();

    void on_qqC_editingFinished();

    void on_qqMotorLockEnabled_clicked();

private:
    Ui::CQDlgJointDyn *ui;
};

#endif // QDLGJOINTDYN_H
