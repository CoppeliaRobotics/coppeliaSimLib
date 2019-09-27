
#ifndef QDLGIKAVOIDANCE_H
#define QDLGIKAVOIDANCE_H

#include "vDialog.h"
#include "ikGroup.h"
#include <QAbstractButton>

namespace Ui {
    class CQDlgIkAvoidance;
}

class CQDlgIkAvoidance : public VDialog
{
    Q_OBJECT

public:
    explicit CQDlgIkAvoidance(QWidget *parent = 0);
    ~CQDlgIkAvoidance();

    void cancelEvent();
    void okEvent();

    void refresh();

    CikGroup* ikGroup;
    bool avoidanceConstraintEnabled;
    float avoidanceDistance;
    int avoidanceEntity1;
    int avoidanceEntity2;
    bool first;

private slots:
    void on_qqSelectEntities_clicked();

    void on_qqDistance_editingFinished();

    void on_qqClose_clicked(QAbstractButton *button);

private:
    Ui::CQDlgIkAvoidance *ui;
};

#endif // QDLGIKAVOIDANCE_H
