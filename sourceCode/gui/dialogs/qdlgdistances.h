
#ifndef QDLGDISTANCES_H
#define QDLGDISTANCES_H

#include "dlgEx.h"
#include <QListWidgetItem>

namespace Ui {
    class CQDlgDistances;
}

class CQDlgDistances : public CDlgEx
{
    Q_OBJECT

public:
    explicit CQDlgDistances(QWidget *parent = 0);
    ~CQDlgDistances();

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

    void on_qqDistanceList_itemSelectionChanged();

    void on_qqDistanceList_itemChanged(QListWidgetItem *item);

    void on_qqEnableAll_clicked();

    void on_qqExplicitHandling_clicked();

    void on_qqUseThreshold_clicked();

    void on_qqThreshold_editingFinished();

    void on_qqDisplaySegment_clicked();

    void on_qqSegmentWidth_editingFinished();

    void on_qqAdjustSegmentColor_clicked();

private:
    Ui::CQDlgDistances *ui;
};

#endif // QDLGDISTANCES_H
