#ifndef QDLGIK_H
#define QDLGIK_H

#include "dlgEx.h"
#include <QListWidgetItem>
#include <QShortcut>

namespace Ui {
    class CQDlgIk;
}

class CQDlgIk : public CDlgEx
{
    Q_OBJECT

public:
    explicit CQDlgIk(QWidget *parent = 0);
    ~CQDlgIk();

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

    void on_qqIkEnabled_clicked();

    void on_qqAddNewGroup_clicked();

    void on_qqList_itemChanged(QListWidgetItem *item);

    void on_qqList_itemSelectionChanged();

    void on_qqUp_clicked();

    void on_qqDown_clicked();

    void on_qqExplicitHandling_clicked();

    void on_qqGroupIsActive_clicked();

    void on_qqIgnoreMaxStepSizes_clicked();

    void on_qqCalcMethodCombo_currentIndexChanged(int index);

    void on_qqDamping_editingFinished();

    void on_qqMaxIterations_editingFinished();

    void on_qqEditConditional_clicked();

    void on_qqEditIkElements_clicked();

private:
    Ui::CQDlgIk *ui;
};

#endif // QDLGIK_H



