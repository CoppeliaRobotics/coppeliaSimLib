
#ifndef QDLGSIMPLEFILTER_CIRCULARCUT_H
#define QDLGSIMPLEFILTER_CIRCULARCUT_H

#include "qdlgsimplefilter.h"

namespace Ui {
    class CQDlgSimpleFilter_circularCut;
}

class CQDlgSimpleFilter_circularCut : public CQDlgSimpleFilter
{
    Q_OBJECT

public:
    explicit CQDlgSimpleFilter_circularCut(QWidget *parent = 0);
    ~CQDlgSimpleFilter_circularCut();

    void refresh();

private slots:
    void on_qqOkCancel_accepted();

    void on_qqOkCancel_rejected();

    void on_qqRadius_editingFinished();

    void on_qqInverse_clicked();

private:
    Ui::CQDlgSimpleFilter_circularCut *ui;
};

#endif // QDLGSIMPLEFILTER_CIRCULARCUT_H
