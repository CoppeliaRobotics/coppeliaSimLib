
#ifndef QDLGCOLORPULSATION_H
#define QDLGCOLORPULSATION_H

#include "vDialog.h"
#include "sceneObject.h"

namespace Ui {
    class CQDlgColorPulsation;
}

class CQDlgColorPulsation : public VDialog
{
    Q_OBJECT

public:
    explicit CQDlgColorPulsation(QWidget *parent = 0);
    ~CQDlgColorPulsation();

    void cancelEvent();
    void okEvent();

    void refresh();

    bool pulsationEnabled;
    bool pulsationRealTime;
    double pulsationFrequency;
    double pulsationPhase;
    double pulsationRatio;

private slots:
    void on_qqEnablePulsation_clicked();

    void on_qqRealTime_clicked();

    void on_qqSimulationTime_clicked();

    void on_qqFrequency_editingFinished();

    void on_qqPhase_editingFinished();

    void on_qqRatio_editingFinished();

    void on_qqOkCancel_accepted();

    void on_qqOkCancel_rejected();

private:
    Ui::CQDlgColorPulsation *ui;
};

#endif // QDLGCOLORPULSATION_H
