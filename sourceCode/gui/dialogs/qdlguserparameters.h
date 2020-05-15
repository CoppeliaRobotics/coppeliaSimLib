#ifndef QDLGUSERPARAMETERS_H
#define QDLGUSERPARAMETERS_H

#include "vDialog.h"
#include "sceneObject.h"
#include <QListWidgetItem>
#include <QAbstractButton>

namespace Ui {
    class CQDlgUserParameters;
}

class CQDlgUserParameters : public VDialog
{
    Q_OBJECT

public:
    explicit CQDlgUserParameters(QWidget *parent = 0);
    ~CQDlgUserParameters();

    void refresh();
    void refreshPart2();

    void cancelEvent();
    void okEvent();

    void updateObjectsInList();
    int getSelectedObjectID();
    void selectObjectInList(int objectID);
    bool inSelectionRoutine;

    CSceneObject* object;

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
    Ui::CQDlgUserParameters *ui;
};

#endif // QDLGUSERPARAMETERS_H
