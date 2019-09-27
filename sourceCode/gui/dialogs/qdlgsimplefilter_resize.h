
#ifndef QDLGSIMPLEFILTER_RESIZE_H
#define QDLGSIMPLEFILTER_RESIZE_H

#include "qdlgsimplefilter.h"

namespace Ui {
    class CQDlgSimpleFilter_resize;
}

class CQDlgSimpleFilter_resize : public CQDlgSimpleFilter
{
    Q_OBJECT

public:
    explicit CQDlgSimpleFilter_resize(QWidget *parent = 0);
    ~CQDlgSimpleFilter_resize();

    void refresh();

private slots:
    void on_qqOkCancel_accepted();

    void on_qqOkCancel_rejected();

    void on_qqX_editingFinished();

    void on_qqY_editingFinished();

private:
    Ui::CQDlgSimpleFilter_resize *ui;
};

#endif // QDLGSIMPLEFILTER_RESIZE_H
