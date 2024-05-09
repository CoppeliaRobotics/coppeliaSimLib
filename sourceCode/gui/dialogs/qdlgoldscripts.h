
#ifndef QDLGOLDSCRIPTS_H
#define QDLGOLDSCRIPTS_H

#include <dlgEx.h>
#include <QListWidgetItem>

namespace Ui
{
class CQDlgOldScripts;
}

class CQDlgOldScripts : public CDlgEx
{
    Q_OBJECT

  public:
    explicit CQDlgOldScripts(QWidget *parent = 0);
    ~CQDlgOldScripts();

    void refresh();
    void dialogCallbackFunc(const SUIThreadCommand *cmdIn, SUIThreadCommand *cmdOut);

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

  private:
    Ui::CQDlgOldScripts *ui;
};

#endif // QDLGOLDSCRIPTS_H
