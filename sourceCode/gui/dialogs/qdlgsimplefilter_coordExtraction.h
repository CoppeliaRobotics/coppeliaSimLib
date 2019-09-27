
#ifndef QDLGSIMPLEFILTER_COORDEXTRACTION_H
#define QDLGSIMPLEFILTER_COORDEXTRACTION_H

#include "qdlgsimplefilter.h"

namespace Ui {
    class CQDlgSimpleFilter_coordExtraction;
}

class CQDlgSimpleFilter_coordExtraction : public CQDlgSimpleFilter
{
    Q_OBJECT

public:
    explicit CQDlgSimpleFilter_coordExtraction(QWidget *parent = 0);
    ~CQDlgSimpleFilter_coordExtraction();

    void refresh();

private slots:
    void on_qqOkCancel_accepted();

    void on_qqOkCancel_rejected();

    void on_qqX_editingFinished();

    void on_qqY_editingFinished();

    void on_qqAngular_clicked();

    void on_qqNonAngular_clicked();

private:
    Ui::CQDlgSimpleFilter_coordExtraction *ui;
};

#endif // QDLGSIMPLEFILTER_COORDEXTRACTION_H
