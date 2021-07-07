
#ifndef QDLGGRAPHS_H
#define QDLGGRAPHS_H

#include "dlgEx.h"
#include <QListWidgetItem>
#include <QShortcut>

namespace Ui {
    class CQDlgGraphs;
}

class CQDlgGraphs : public CDlgEx
{
    Q_OBJECT

public:
    explicit CQDlgGraphs(QWidget *parent = 0);
    ~CQDlgGraphs();

    void refresh();
    void dialogCallbackFunc(const SUIThreadCommand* cmdIn,SUIThreadCommand* cmdOut);

    void cancelEvent();

    void updateObjectsInList();
    int getSelectedObjectID();
    void selectObjectInList(int objectID);

    bool inMainRefreshRoutine;
    bool inListSelectionRoutine;
    bool noListSelectionAllowed;

    QShortcut* delKeyShortcut;
    QShortcut* backspaceKeyShortcut;

private slots:
    void onDeletePressed();
    void on_qqAddNewDataStream_clicked();

    void on_qqRecordingList_itemSelectionChanged();

    void on_qqRecordingList_itemChanged(QListWidgetItem *item);

    void on_qqExplicitHandling_clicked();

    void on_qqBufferIsCyclic_clicked();

    void on_qqObjectSize_editingFinished();

    void on_qqBufferSize_editingFinished();

    void on_qqRemoveAllStatics_clicked();

    void on_qqAdjustBackgroundColor_clicked();

    void on_qqAdjustGridColor_clicked();

    void on_qqTransformationCombo_currentIndexChanged(int index);

    void on_qqTransformationCoeff_editingFinished();

    void on_qqTransformationOffset_editingFinished();

    void on_qqMovingAveragePeriod_editingFinished();

    void on_qqTimeGraphVisible_clicked();

    void on_qqShowLabel_clicked();

    void on_qqLinkPoints_clicked();

    void on_qqAdjustCurveColor_clicked();

    void on_qqDuplicateToStatic_clicked();

    void on_qqEditXYGraphs_clicked();

    void on_qqEdit3DCurves_clicked();

    void on_qqRemoveAll_clicked();

private:
    Ui::CQDlgGraphs *ui;
};

#endif // QDLGGRAPHS_H
