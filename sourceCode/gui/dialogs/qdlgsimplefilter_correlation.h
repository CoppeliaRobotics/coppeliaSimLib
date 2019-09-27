
#ifndef QDLGSIMPLEFILTER_CORRELATION_H
#define QDLGSIMPLEFILTER_CORRELATION_H

#include "qdlgsimplefilter.h"

namespace Ui {
    class CQDlgSimpleFilter_correlation;
}

class CQDlgSimpleFilter_correlation : public CQDlgSimpleFilter
{
    Q_OBJECT

public:
    explicit CQDlgSimpleFilter_correlation(QWidget *parent = 0);
    ~CQDlgSimpleFilter_correlation();

    void refresh();

private slots:
    void on_qqOkCancel_accepted();

    void on_qqOkCancel_rejected();

    void on_qqGreyScale_clicked();

    void on_qqVisualizeCenter_clicked();

    void on_qqTemplateX_editingFinished();

    void on_qqTemplateY_editingFinished();

    void on_qqFrameX_editingFinished();

    void on_qqFrameY_editingFinished();

private:
    Ui::CQDlgSimpleFilter_correlation *ui;
};

#endif // QDLGSIMPLEFILTER_CORRELATION_H
