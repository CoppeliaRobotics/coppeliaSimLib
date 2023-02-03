
#ifndef QDLGCOMMONPROPERTIES_H
#define QDLGCOMMONPROPERTIES_H

#include <dlgEx.h>

namespace Ui {
    class CQDlgCommonProperties;
}

class CQDlgCommonProperties : public CDlgEx
{
    Q_OBJECT

public:
    explicit CQDlgCommonProperties(QWidget *parent = 0);
    ~CQDlgCommonProperties();

    void refresh();

    void cancelEvent();

    bool inPlaceState;
    double scalingState;
    bool inMainRefreshRoutine;

private slots:
    void on_qqSelectable_clicked();

    void on_qqSelectBaseInstead_clicked();

    void on_qqDontShowInModelSelection_clicked();

    void on_qqApplyGeneralProperties_clicked();

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

    void on_qqApplyLayers_clicked();

    void on_qqModelBase_clicked();

    void on_qqEditModelProperties_clicked();

    void on_qqCollidable_clicked();

    void on_qqMeasurable_clicked();

    void on_qqRenderable_clicked();

    void on_qqDetectable_clicked();

    void on_qqEditDetectableDetails_clicked();

    void on_qqApplySpecialProperties_clicked();

    void on_qqIgnoreForViewFitting_clicked();

    void on_qqOpenScalingDialog_clicked();

    void on_qqAssembling_clicked();

    void on_qqSelectInvisible_clicked();

    void on_qqDepthInvisible_clicked();

    void on_qqSelfCollisionIndicator_editingFinished();

    void on_qqViewableObjects_currentIndexChanged(int index);

    void on_qqExtensionString_editingFinished();

    void on_qqCannotBeDeleted_clicked();

    void on_qqCannotBeDeletedDuringSimul_clicked();

private:
    Ui::CQDlgCommonProperties *ui;
};

#endif // QDLGCOMMONPROPERTIES_H
