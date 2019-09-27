
#ifndef QDLGSHAPEDYN_H
#define QDLGSHAPEDYN_H

#include "dlgEx.h"

namespace Ui {
    class CQDlgShapeDyn;
}

class CQDlgShapeDyn : public CDlgEx
{
    Q_OBJECT

public:
    explicit CQDlgShapeDyn(QWidget *parent = 0);
    ~CQDlgShapeDyn();

    void refresh();
    void cancelEvent();

    static bool showDynamicWindow;
    bool inMainRefreshRoutine;

private slots:
    void on_qqDynamic_clicked();

    void on_qqSleepModeStart_clicked();

    void on_qqAdjustEngineProperties_clicked();

    void on_qqRespondable_clicked();

    void on_a_1_clicked();
    void on_a_2_clicked();
    void on_a_3_clicked();
    void on_a_4_clicked();
    void on_a_5_clicked();
    void on_a_6_clicked();
    void on_a_7_clicked();
    void on_a_8_clicked();
    void on_a_9_clicked();
    void on_a_10_clicked();
    void on_a_11_clicked();
    void on_a_12_clicked();
    void on_a_13_clicked();
    void on_a_14_clicked();
    void on_a_15_clicked();
    void on_a_16_clicked();

    void on_qqMass_editingFinished();

    void on_qqMassT2_clicked();

    void on_qqMassD2_clicked();

    void on_qqIX_editingFinished();

    void on_qqIY_editingFinished();

    void on_qqIZ_editingFinished();

    void on_qqIT2_clicked();

    void on_qqID2_clicked();

    void on_qqPX_editingFinished();

    void on_qqPY_editingFinished();

    void on_qqPZ_editingFinished();

    void on_qqAlpha_editingFinished();

    void on_qqBeta_editingFinished();

    void on_qqGamma_editingFinished();

    void on_qqApplyMassAndInertiaProperties_clicked();

    void on_qqTensorMatrix_clicked();

    void on_qqAutomaticToNonStatic_clicked();

    void on_qqApplyMaterialProperties_clicked();

    void on_qqComputeMassProperties_clicked();

private:
    void _toggleRespondableBits(int bits);

    Ui::CQDlgShapeDyn *ui;
};

#endif // QDLGSHAPEDYN_H
