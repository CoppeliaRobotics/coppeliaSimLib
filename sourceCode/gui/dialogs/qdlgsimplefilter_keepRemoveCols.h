
#ifndef QDLGSIMPLEFILTER_KEEPREMOVECOLS_H
#define QDLGSIMPLEFILTER_KEEPREMOVECOLS_H

#include "qdlgsimplefilter.h"

namespace Ui {
    class CQDlgSimpleFilter_keepRemoveCols;
}

class CQDlgSimpleFilter_keepRemoveCols : public CQDlgSimpleFilter
{
    Q_OBJECT

public:
    explicit CQDlgSimpleFilter_keepRemoveCols(QWidget *parent = 0);
    ~CQDlgSimpleFilter_keepRemoveCols();

    void refresh();

private slots:
    void on_qqOkCancel_accepted();

    void on_qqOkCancel_rejected();

    void on_qqRemove_clicked();

    void on_qqKeep_clicked();

    void on_qqBuffer1_clicked();

    void on_qqRgb_clicked();

    void on_qqHsl_clicked();

    void on_qqRed_editingFinished();

    void on_qqGreen_editingFinished();

    void on_qqBlue_editingFinished();

    void on_qqRedTol_editingFinished();

    void on_qqGreenTol_editingFinished();

    void on_qqBlueTol_editingFinished();

private:
    Ui::CQDlgSimpleFilter_keepRemoveCols *ui;
};

#endif // QDLGSIMPLEFILTER_KEEPREMOVECOLS_H
