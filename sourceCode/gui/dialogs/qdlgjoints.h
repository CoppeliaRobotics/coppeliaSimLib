
#ifndef QDLGJOINTS_H
#define QDLGJOINTS_H

#include "dlgEx.h"

namespace Ui {
    class CQDlgJoints;
}

class CQDlgJoints : public CDlgEx
{
    Q_OBJECT

public:
    explicit CQDlgJoints(QWidget *parent = 0);
    ~CQDlgJoints();

    void refresh();

    void cancelEvent();

    bool inMainRefreshRoutine;

    static bool showDynamicWindow;

private slots:
    void on_qqCyclic_clicked();

    void on_qqPitch_editingFinished();

    void on_qqMinimum_editingFinished();

    void on_qqRange_editingFinished();

    void on_qqPosition_editingFinished();

    void on_qqApplyConfig_clicked();

    void on_qqJointModeCombo_currentIndexChanged(int index);

    void on_qqApplyMode_clicked();

    void on_qqAdjustDependency_clicked();

    void on_qqAdjustDynamicParameters_clicked();

    void on_qqLength_editingFinished();

    void on_qqDiameter_editingFinished();

    void on_qqAdjustColorA_clicked();

    void on_qqApplyAppearance_clicked();

private:
    Ui::CQDlgJoints *ui;
};

#endif // QDLGJOINTS_H
