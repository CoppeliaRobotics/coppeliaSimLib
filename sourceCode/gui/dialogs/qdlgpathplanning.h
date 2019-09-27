
#ifndef QDLGPATHPLANNING_H
#define QDLGPATHPLANNING_H

#include "dlgEx.h"
#include <QListWidgetItem>
#include <QShortcut>
#include <QComboBox>

namespace Ui {
    class CQDlgPathPlanning;
}

class CQDlgPathPlanning : public CDlgEx
{
    Q_OBJECT

public:
    explicit CQDlgPathPlanning(QWidget *parent = 0);
    ~CQDlgPathPlanning();

    void refresh();
    void dialogCallbackFunc(const SUIThreadCommand* cmdIn,SUIThreadCommand* cmdOut);

    void cancelEvent();

    void updateObjectsInList();
    int getSelectedObjectID();
    void selectObjectInList(int objectID);

    bool inMainRefreshRoutine;
    bool inListSelectionRoutine;
    bool noListSelectionAllowed;

    void fillComboWithCollisionOrDistanceEntities(QComboBox* combo,bool collisionEntities,bool insertAllAtBeginning,bool insertNoneAtBeginning,int idToExclude);
    bool selectItemInCombo(QComboBox* combo,int itemID);

    QShortcut* delKeyShortcut;
    QShortcut* backspaceKeyShortcut;

    private slots:
    void onDeletePressed();

    void on_qqList_itemChanged(QListWidgetItem *item);

    void on_qqList_itemSelectionChanged();

    void on_qqAddNew_clicked();

    void on_qqGoalCombo_currentIndexChanged(int index);

    void on_qqPathCombo_currentIndexChanged(int index);

    void on_qqCheckTypeCombo_currentIndexChanged(int index);

    void on_qqRobotCombo_currentIndexChanged(int index);

    void on_qqObstacleCombo_currentIndexChanged(int index);

    void on_qqMinDist_editingFinished();

    void on_qqMaxDist_editingFinished();

    void on_qqAdjustSearchParameters_clicked();

    void on_qqUsePartialPath_clicked();

    void on_qqShowNodes_clicked();

    void on_qqMaxCalcTime_editingFinished();

    void on_qqPostProcessingPasses_editingFinished();

    void on_qqComputePath_clicked();

private:
    Ui::CQDlgPathPlanning *ui;
};

#endif // QDLGPATHPLANNING_H
