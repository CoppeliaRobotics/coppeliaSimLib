
#ifndef QDLGSIMPLEFILTER_ROTATE_H
#define QDLGSIMPLEFILTER_ROTATE_H

#include "qdlgsimplefilter.h"

namespace Ui {
    class CQDlgSimpleFilter_rotate;
}

class CQDlgSimpleFilter_rotate : public CQDlgSimpleFilter
{
    Q_OBJECT

public:
    explicit CQDlgSimpleFilter_rotate(QWidget *parent = 0);
    ~CQDlgSimpleFilter_rotate();

    void refresh();

private slots:
    void on_qqOkCancel_accepted();

    void on_qqOkCancel_rejected();

    void on_qqAngle_editingFinished();

private:
    Ui::CQDlgSimpleFilter_rotate *ui;
};

#endif // QDLGSIMPLEFILTER_ROTATE_H
