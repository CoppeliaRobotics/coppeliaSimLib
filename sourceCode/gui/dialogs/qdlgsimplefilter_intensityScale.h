
#ifndef QDLGSIMPLEFILTER_INTENSITYSCALE_H
#define QDLGSIMPLEFILTER_INTENSITYSCALE_H

#include "qdlgsimplefilter.h"

namespace Ui {
    class CQDlgSimpleFilter_intensityScale;
}

class CQDlgSimpleFilter_intensityScale : public CQDlgSimpleFilter
{
    Q_OBJECT

public:
    explicit CQDlgSimpleFilter_intensityScale(QWidget *parent = 0);
    ~CQDlgSimpleFilter_intensityScale();

    void refresh();

private slots:
    void on_qqOkCancel_accepted();

    void on_qqOkCancel_rejected();

    void on_qqGreyScale_clicked();

    void on_qqIntensityScale_clicked();

    void on_qqStart_editingFinished();

    void on_qqEnd_editingFinished();

private:
    Ui::CQDlgSimpleFilter_intensityScale *ui;
};

#endif // QDLGSIMPLEFILTER_INTENSITYSCALE_H
