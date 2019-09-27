
#ifndef QDLGSCRIPTPARAMETERS_H
#define QDLGSCRIPTPARAMETERS_H

#include "vDialog.h"
#include "luaScriptObject.h"
#include <QListWidgetItem>
#include <QAbstractButton>

namespace Ui {
    class CQDlgScriptParameters;
}

class CQDlgScriptParameters : public VDialog
{
    Q_OBJECT

public:
    explicit CQDlgScriptParameters(QWidget *parent = 0);
    ~CQDlgScriptParameters();

    void refresh();
    void refreshPart2();

    void cancelEvent();
    void okEvent();

    void updateObjectsInList();
    int getSelectedObjectID();
    void selectObjectInList(int objectID);
    bool inSelectionRoutine;

    CLuaScriptObject* script;

private slots:
    void onDeletePressed();

    void on_qqAddNew_clicked();

    void on_qqParameterList_itemChanged(QListWidgetItem *item);

    void on_qqValue_editingFinished();

    void on_qqUnit_editingFinished();

    void on_qqPrivate_clicked();

    void on_qqPersistent_clicked();

    void on_qqClose_clicked(QAbstractButton *button);

    void on_qqParameterList_itemSelectionChanged();

    void on_qqUp_clicked();

    void on_qqDown_clicked();

private:
    Ui::CQDlgScriptParameters *ui;
};

#endif // QDLGSCRIPTPARAMETERS_H
