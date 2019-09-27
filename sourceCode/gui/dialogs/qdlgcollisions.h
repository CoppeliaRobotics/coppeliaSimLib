
#ifndef QDLGCOLLISIONS_H
#define QDLGCOLLISIONS_H

#include "dlgEx.h"
#include <QListWidgetItem>

namespace Ui {
    class CQDlgCollisions;
}

class CQDlgCollisions : public CDlgEx
{
    Q_OBJECT

public:
    explicit CQDlgCollisions(QWidget *parent = 0);
    ~CQDlgCollisions();

    void refresh();
    void dialogCallbackFunc(const SUIThreadCommand* cmdIn,SUIThreadCommand* cmdOut);

    void cancelEvent();

    void updateObjectsInList();
    int getSelectedObjectID();
    void selectObjectInList(int objectID);
    bool inSelectionRoutine;

private slots:
    void onDeletePressed();
    void on_qqAddNewObject_clicked();

    void on_qqCollisionList_itemSelectionChanged();

    void on_qqCollisionList_itemChanged(QListWidgetItem *item);

    void on_qqEnableAll_clicked();

    void on_qqCollisionColor_clicked();

    void on_qqExplicitHandling_clicked();

    void on_qqColliderColorChanges_clicked();

    void on_qqComputeContour_clicked();

    void on_qqCollideeColorChanges_clicked();

    void on_qqAdjustContourColor_clicked();

    void on_qqContourWidth_editingFinished();

private:
    Ui::CQDlgCollisions *ui;
};

#endif // QDLGCOLLISIONS_H
