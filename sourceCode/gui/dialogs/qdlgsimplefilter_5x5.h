
#ifndef QDLGSIMPLEFILTER_5X5_H
#define QDLGSIMPLEFILTER_5X5_H

#include "qdlgsimplefilter.h"

namespace Ui {
    class CQDlgSimpleFilter_5x5;
}

class CQDlgSimpleFilter_5x5 : public CQDlgSimpleFilter
{
    Q_OBJECT

public:
    explicit CQDlgSimpleFilter_5x5(QWidget *parent = 0);
    ~CQDlgSimpleFilter_5x5();

    void refresh();

private slots:
    void on_qqOkCancel_accepted();

    void on_qqOkCancel_rejected();

    void on_qqPasses_editingFinished();

    void on_qqMultiplier_editingFinished();

    void on_qq0_editingFinished();
    void on_qq1_editingFinished();
    void on_qq2_editingFinished();
    void on_qq3_editingFinished();
    void on_qq4_editingFinished();
    void on_qq5_editingFinished();
    void on_qq6_editingFinished();
    void on_qq7_editingFinished();
    void on_qq8_editingFinished();
    void on_qq9_editingFinished();
    void on_qq10_editingFinished();
    void on_qq11_editingFinished();
    void on_qq12_editingFinished();
    void on_qq13_editingFinished();
    void on_qq14_editingFinished();
    void on_qq15_editingFinished();
    void on_qq16_editingFinished();
    void on_qq17_editingFinished();
    void on_qq18_editingFinished();
    void on_qq19_editingFinished();
    void on_qq20_editingFinished();
    void on_qq21_editingFinished();
    void on_qq22_editingFinished();
    void on_qq23_editingFinished();
    void on_qq24_editingFinished();

private:
    Ui::CQDlgSimpleFilter_5x5 *ui;
};

#endif // QDLGSIMPLEFILTER_5X5_H
