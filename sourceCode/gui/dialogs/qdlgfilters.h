
#ifndef QDLGFILTERS_H
#define QDLGFILTERS_H

#include "dlgEx.h"
#include <QListWidgetItem>

namespace Ui {
    class CQDlgFilters;
}

class CQDlgFilters : public CDlgEx
{
    Q_OBJECT

public:
    explicit CQDlgFilters(QWidget *parent = 0);
    ~CQDlgFilters();

    void refresh();
    void dialogCallbackFunc(const SUIThreadCommand* cmdIn,SUIThreadCommand* cmdOut);
    void cancelEvent();

    void updateObjectsInList();
    int getSelectedObjectID();
    void selectObjectInList(int objectID);
    bool _editFilter(int index);

    bool inMainRefreshRoutine;
    int currentComboIndex;

private slots:
    void onDeletePressed();

    void on_qqFilterCombo_currentIndexChanged(int index);

    void on_qqFilterList_itemDoubleClicked(QListWidgetItem *item);

    void on_qqAddFilter_clicked();

    void on_qqUp_clicked();

    void on_qqDown_clicked();

    void on_qqComponentEnabled_clicked();

    void on_qqFilterList_itemSelectionChanged();

    void on_qqApply_clicked();

private:
    Ui::CQDlgFilters *ui;
};

#endif // QDLGFILTERS_H
