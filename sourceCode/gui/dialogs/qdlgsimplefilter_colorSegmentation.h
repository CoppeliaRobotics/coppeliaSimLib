
#ifndef QDLGSIMPLEFILTER_COLORSEGMENTATION_H
#define QDLGSIMPLEFILTER_COLORSEGMENTATION_H

#include "qdlgsimplefilter.h"

namespace Ui {
    class CQDlgSimpleFilter_colorSegmentation;
}

class CQDlgSimpleFilter_colorSegmentation : public CQDlgSimpleFilter
{
    Q_OBJECT

public:
    explicit CQDlgSimpleFilter_colorSegmentation(QWidget *parent = 0);
    ~CQDlgSimpleFilter_colorSegmentation();

    void refresh();

private slots:
    void on_qqOkCancel_accepted();

    void on_qqOkCancel_rejected();

    void on_qqDist_editingFinished();

private:
    Ui::CQDlgSimpleFilter_colorSegmentation *ui;
};

#endif // QDLGSIMPLEFILTER_COLORSEGMENTATION_H
