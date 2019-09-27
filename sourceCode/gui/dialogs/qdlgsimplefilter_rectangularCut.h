
#ifndef QDLGSIMPLEFILTER_RECTANGULARCUT_H
#define QDLGSIMPLEFILTER_RECTANGULARCUT_H

#include "qdlgsimplefilter.h"

namespace Ui {
    class CQDlgSimpleFilter_rectangularCut;
}

class CQDlgSimpleFilter_rectangularCut : public CQDlgSimpleFilter
{
    Q_OBJECT

public:
    explicit CQDlgSimpleFilter_rectangularCut(QWidget *parent = 0);
    ~CQDlgSimpleFilter_rectangularCut();

    void refresh();

private slots:
    void on_qqOkCancel_accepted();

    void on_qqOkCancel_rejected();

    void on_qqX_editingFinished();

    void on_qqY_editingFinished();

    void on_qqInverse_clicked();

private:
    Ui::CQDlgSimpleFilter_rectangularCut *ui;
};

#endif // QDLGSIMPLEFILTER_RECTANGULARCUT_H
