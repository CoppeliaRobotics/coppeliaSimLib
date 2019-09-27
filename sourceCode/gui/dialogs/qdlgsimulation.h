
#ifndef QDLGSIMULATION_H
#define QDLGSIMULATION_H

#include "dlgEx.h"

namespace Ui {
    class CQDlgSimulation;
}

class CQDlgSimulation : public CDlgEx
{
    Q_OBJECT

public:
    explicit CQDlgSimulation(QWidget *parent = 0);
    ~CQDlgSimulation();

    void refresh();

    bool inMainRefreshRoutine;
private slots:
    void on_qqTimeStep_editingFinished();

    void on_qqScriptExecutionPasses_editingFinished();

    void on_qqRealTime_clicked();

    void on_qqMultiplicationCoefficient_editingFinished();

    void on_qqCatchUp_clicked();

    void on_qqPauseWhenTimeHigher_clicked();

    void on_qqPauseTime_editingFinished();

    void on_qqPauseOnScriptError_clicked();

    void on_qqResetScene_clicked();

    void on_qqRemoveNewObjects_clicked();

    void on_qqConfigCombo_currentIndexChanged(int index);

    void on_qqFullscreen_clicked();

private:
    Ui::CQDlgSimulation *ui;
};

#endif // QDLGSIMULATION_H
