
#ifndef QDLGSCRIPTS_H
#define QDLGSCRIPTS_H

#include "dlgEx.h"
#include <QListWidgetItem>

namespace Ui {
    class CQDlgScripts;
}

class CQDlgScripts : public CDlgEx
{
    Q_OBJECT

public:
    explicit CQDlgScripts(QWidget *parent = 0);
    ~CQDlgScripts();

    void refresh();
    void dialogCallbackFunc(const SUIThreadCommand* cmdIn,SUIThreadCommand* cmdOut);

    void updateObjectsInList();
    int getSelectedObjectID();
    void selectObjectInList(int objectID);
    bool inSelectionRoutine;
    bool rebuildingRoutine;
    bool inMainRefreshRoutine;

    static int scriptViewMode;

private slots:
    void onDeletePressed();

    void on_qqScriptList_itemSelectionChanged();

    void on_qqScriptList_itemDoubleClicked(QListWidgetItem *item);

    void on_qqDisabled_clicked();

    void on_qqExecuteOnce_clicked();

    void on_qqExecutionOrder_currentIndexChanged(int index);

    void on_qqCombo_currentIndexChanged(int index);

    void on_qqTreeTraversalDirection_currentIndexChanged(int index);

    void on_qqDebugMode_currentIndexChanged(int index);

private:
    Ui::CQDlgScripts *ui;
};

#endif // QDLGSCRIPTS_H
