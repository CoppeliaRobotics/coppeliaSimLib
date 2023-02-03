
#ifndef QDLGENTITYVSENTITYSELECTION_H
#define QDLGENTITYVSENTITYSELECTION_H

#include <vDialog.h>
#include <QComboBox>

namespace Ui {
    class CQDlgEntityVsEntitySelection;
}

class CQDlgEntityVsEntitySelection : public VDialog
{
    Q_OBJECT

public:
    explicit CQDlgEntityVsEntitySelection(QWidget *parent = 0);
    ~CQDlgEntityVsEntitySelection();

    void cancelEvent();
    void okEvent();

    void initialize(int theMode); // 0=collisions, 1=distances, 2=IK avoidance

    int mode;
    int entity1;
    int entity2;

private slots:
    void on_qqOkCancel_accepted();

    void on_qqOkCancel_rejected();

private:
    void fillComboWithCollisionEntities(QComboBox* combo,bool insertAllAtBeginning);
    void fillComboWithDistanceEntities(QComboBox* combo,bool insertAllAtBeginning);
    bool checkSelectionValidity();
    Ui::CQDlgEntityVsEntitySelection *ui;
};

#endif // QDLGENTITYVSENTITYSELECTION_H
