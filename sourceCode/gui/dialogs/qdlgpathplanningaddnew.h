
#ifndef QDLGPATHPLANNINGADDNEW_H
#define QDLGPATHPLANNINGADDNEW_H

#include "vDialog.h"

namespace Ui {
    class CQDlgPathPlanningAddNew;
}

class CQDlgPathPlanningAddNew : public VDialog
{
    Q_OBJECT

public:
    explicit CQDlgPathPlanningAddNew(QWidget *parent = 0);
    ~CQDlgPathPlanningAddNew();

    void refresh();

    void cancelEvent();
    void okEvent();

    void _getValues();

    int selectedTaskType;
    int selectedDummy;

private slots:
    void on_qqOkCancel_accepted();

    void on_qqOkCancel_rejected();

private:
    Ui::CQDlgPathPlanningAddNew *ui;
};

#endif // QDLGPATHPLANNINGADDNEW_H
