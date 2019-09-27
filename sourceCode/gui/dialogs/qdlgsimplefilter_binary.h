
#ifndef QDLGSIMPLEFILTER_BINARY_H
#define QDLGSIMPLEFILTER_BINARY_H

#include "qdlgsimplefilter.h"

namespace Ui {
    class CQDlgSimpleFilter_binary;
}

class CQDlgSimpleFilter_binary : public CQDlgSimpleFilter
{
    Q_OBJECT

public:
    explicit CQDlgSimpleFilter_binary(QWidget *parent = 0);
    ~CQDlgSimpleFilter_binary();

    void refresh();

private slots:
    void on_qqOkCancel_accepted();

    void on_qqOkCancel_rejected();

    void on_qqThreshold_editingFinished();

    void on_qqTriggerEnabled_clicked();

    void on_qqVisualize_clicked();

    void on_qqProportion_editingFinished();

    void on_qqProportionTol_editingFinished();

    void on_qqPosX_editingFinished();

    void on_qqPosXTol_editingFinished();

    void on_qqPosY_editingFinished();

    void on_qqPosYTol_editingFinished();

    void on_qqAngle_editingFinished();

    void on_qqAngleTol_editingFinished();

    void on_qqRoundness_editingFinished();

private:
    Ui::CQDlgSimpleFilter_binary *ui;
};

#endif // QDLGSIMPLEFILTER_BINARY_H
