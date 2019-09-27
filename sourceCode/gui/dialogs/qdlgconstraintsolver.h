
#ifndef QDLGCONSTRAINTSOLVER_H
#define QDLGCONSTRAINTSOLVER_H

#include "dlgEx.h"
#include <QListWidgetItem>

namespace Ui {
    class CQDlgConstraintSolver;
}

class CQDlgConstraintSolver : public CDlgEx
{
    Q_OBJECT

public:
    explicit CQDlgConstraintSolver(QWidget *parent = 0);
    ~CQDlgConstraintSolver();

    void refresh();
    void dialogCallbackFunc(const SUIThreadCommand* cmdIn,SUIThreadCommand* cmdOut);

    void cancelEvent();

    void updateObjectsInList();
    int getSelectedObjectID();
    void selectObjectInList(int objectID);
    bool inSelectionRoutine;

private slots:
    void onDeletePressed();
    void on_qqAllEnabled_clicked();

    void on_qqAddNew_clicked();

    void on_qqObjectList_itemSelectionChanged();

    void on_qqObjectList_itemChanged(QListWidgetItem *item);

    void on_qqExplicitHandling_clicked();

    void on_qqIterations_editingFinished();

    void on_qqInterpolation_editingFinished();

    void on_qqDamping_editingFinished();

    void on_qqLinearVariation_editingFinished();

    void on_qqAngularVariation_editingFinished();

    void on_qqLoopClosurePosTol_editingFinished();

    void on_qqLoopClosureAngTol_editingFinished();

private:
    Ui::CQDlgConstraintSolver *ui;
};

#endif // QDLGCONSTRAINTSOLVER_H
