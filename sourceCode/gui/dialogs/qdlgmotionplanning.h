
#ifndef QDLGMOTIONPLANNING_H
#define QDLGMOTIONPLANNING_H

#include "dlgEx.h"
#include <QListWidgetItem>
#include <QShortcut>
#include <QComboBox>

namespace Ui {
    class CQDlgMotionPlanning;
}

class CQDlgMotionPlanning : public CDlgEx
{
    Q_OBJECT

public:
    explicit CQDlgMotionPlanning(QWidget *parent = 0);
    ~CQDlgMotionPlanning();

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
    void fillComboWithIkGroups(QComboBox* combo);
    bool selectItemInCombo(QComboBox* combo,int itemID);

    QShortcut* delKeyShortcut;
    QShortcut* backspaceKeyShortcut;

    private slots:
    void onDeletePressed();

    void on_qqList_itemChanged(QListWidgetItem *item);

    void on_qqList_itemSelectionChanged();

    void on_qqAddNew_clicked();

    void on_qqEditJointProp_clicked();

    void on_qqIkGroupCombo_currentIndexChanged(int index);

    void on_qqEntity1Combo_currentIndexChanged(int index);

    void on_qqEntity2Combo_currentIndexChanged(int index);

    void on_qqSelfColl_clicked();

    void on_qqSelfDist_clicked();

    void on_qqSelfDistDist_editingFinished();

    void on_qqRobotCombo_currentIndexChanged(int index);

    void on_qqObstacleCombo_currentIndexChanged(int index);

    void on_qqColl_clicked();

    void on_qqDist_clicked();

    void on_qqDistDist_editingFinished();

    void on_qqEditWorkspaceMetric_clicked();

    void on_qqComputeNodes_clicked();

    void on_showPhase1Nodes_clicked();

private:
    Ui::CQDlgMotionPlanning *ui;
};

#endif // QDLGMOTIONPLANNING_H
