
#ifndef QDLGADDGRAPHCURVE_H
#define QDLGADDGRAPHCURVE_H

#include "vDialog.h"

namespace Ui {
    class CQDlgAddGraphCurve;
}

class CQDlgAddGraphCurve : public VDialog
{
    Q_OBJECT

public:
    explicit CQDlgAddGraphCurve(QWidget *parent = 0);
    ~CQDlgAddGraphCurve();

    void cancelEvent();
    void okEvent();

    void refresh();

    bool xyGraph;
    int dataIDX;
    int dataIDY;
    int dataIDZ;

private slots:
    void on_qqOkCancel_accepted();

    void on_qqOkCancel_rejected();

private:
    Ui::CQDlgAddGraphCurve *ui;
};

#endif // QDLGADDGRAPHCURVE_H
