
#ifndef QDLGSIMPLEFILTER_PIXELCHANGE_H
#define QDLGSIMPLEFILTER_PIXELCHANGE_H

#include "qdlgsimplefilter.h"

namespace Ui {
    class CQDlgSimpleFilter_pixelChange;
}

class CQDlgSimpleFilter_pixelChange : public CQDlgSimpleFilter
{
    Q_OBJECT

public:
    explicit CQDlgSimpleFilter_pixelChange(QWidget *parent = 0);
    ~CQDlgSimpleFilter_pixelChange();

    void refresh();

private slots:
    void on_qqOkCancel_accepted();

    void on_qqOkCancel_rejected();

    void on_qqX_editingFinished();

private:
    Ui::CQDlgSimpleFilter_pixelChange *ui;
};

#endif // QDLGSIMPLEFILTER_PIXELCHANGE_H
