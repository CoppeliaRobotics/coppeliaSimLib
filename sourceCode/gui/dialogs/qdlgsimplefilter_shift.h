
#ifndef QDLGSIMPLEFILTER_SHIFT_H
#define QDLGSIMPLEFILTER_SHIFT_H

#include "qdlgsimplefilter.h"

namespace Ui {
    class CQDlgSimpleFilter_shift;
}

class CQDlgSimpleFilter_shift : public CQDlgSimpleFilter
{
    Q_OBJECT

public:
    explicit CQDlgSimpleFilter_shift(QWidget *parent = 0);
    ~CQDlgSimpleFilter_shift();

    void refresh();

private slots:
    void on_qqOkCancel_accepted();

    void on_qqOkCancel_rejected();

    void on_qqX_editingFinished();

    void on_qqY_editingFinished();

    void on_qqWrap_clicked();

private:
    Ui::CQDlgSimpleFilter_shift *ui;
};

#endif // QDLGSIMPLEFILTER_SHIFT_H
