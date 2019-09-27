
#ifndef QDLGDYNAMICS_H
#define QDLGDYNAMICS_H

#include "dlgEx.h"

namespace Ui {
    class CQDlgDynamics;
}

class CQDlgDynamics : public CDlgEx
{
    Q_OBJECT

public:
    explicit CQDlgDynamics(QWidget *parent = 0);
    ~CQDlgDynamics();

    void refresh();

    void cancelEvent();

    bool inMainRefreshRoutine;
private slots:
    void on_qqEnabled_clicked();

    void on_qqEngineCombo_currentIndexChanged(int index);

    void on_qqContactPoints_clicked();

    void on_qqAdjustEngine_clicked();

    void on_qqGravityX_editingFinished();

    void on_qqGravityY_editingFinished();

    void on_qqGravityZ_editingFinished();

private:
    Ui::CQDlgDynamics *ui;
};

#endif // QDLGDYNAMICS_H
