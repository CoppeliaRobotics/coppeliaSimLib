
#ifndef QDLGMOTIONPLANNINGJOINTS_H
#define QDLGMOTIONPLANNINGJOINTS_H

#include "vDialog.h"
#include <QAbstractButton>

class QTableWidgetItem;

namespace Ui {
    class CQDlgMotionPlanningJoints;
}

class CQDlgMotionPlanningJoints : public VDialog
{
    Q_OBJECT

public:
    explicit CQDlgMotionPlanningJoints(QWidget *parent = 0);
    ~CQDlgMotionPlanningJoints();

    void refresh();

    void updateObjectsInTable();

    void cancelEvent();
    void okEvent();

    bool inMainRefreshRoutine;
    bool noTableSelectionAllowed;
    bool first;

    std::vector<std::string> jointNames;
    std::vector<int> jointSubdiv;
    std::vector<float> jointMetricWeight;

private slots:
    void on_qqClose_clicked(QAbstractButton *button);
    void on_qqTable_itemChanged(QTableWidgetItem *item);

private:
    Ui::CQDlgMotionPlanningJoints *ui;
};

#endif // QDLGMOTIONPLANNINGJOINTS_H
