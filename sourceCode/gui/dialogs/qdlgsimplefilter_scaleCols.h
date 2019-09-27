
#ifndef QDLGSIMPLEFILTER_SCALECOLS_H
#define QDLGSIMPLEFILTER_SCALECOLS_H

#include "qdlgsimplefilter.h"

namespace Ui {
    class CQDlgSimpleFilter_scaleCols;
}

class CQDlgSimpleFilter_scaleCols : public CQDlgSimpleFilter
{
    Q_OBJECT

public:
    explicit CQDlgSimpleFilter_scaleCols(QWidget *parent = 0);
    ~CQDlgSimpleFilter_scaleCols();

    void refresh();

private slots:
    void on_qqOkCancel_accepted();

    void on_qqOkCancel_rejected();

    void on_qqRgb_clicked();

    void on_qqHsl_clicked();

    void on_qqRed1_editingFinished();

    void on_qqGreen1_editingFinished();

    void on_qqBlue1_editingFinished();

    void on_qqRed2_editingFinished();

    void on_qqGreen2_editingFinished();

    void on_qqBlue2_editingFinished();

    void on_qqRed3_editingFinished();

    void on_qqGreen3_editingFinished();

    void on_qqBlue3_editingFinished();

private:
    Ui::CQDlgSimpleFilter_scaleCols *ui;
};

#endif // QDLGSIMPLEFILTER_SCALECOLS_H
