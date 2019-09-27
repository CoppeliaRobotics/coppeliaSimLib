
#ifndef QDLGSIMPLEFILTER_3X3_H
#define QDLGSIMPLEFILTER_3X3_H

#include "qdlgsimplefilter.h"

namespace Ui {
    class CQDlgSimpleFilter_3x3;
}

class CQDlgSimpleFilter_3x3 : public CQDlgSimpleFilter
{
    Q_OBJECT

public:
    explicit CQDlgSimpleFilter_3x3(QWidget *parent = 0);
    ~CQDlgSimpleFilter_3x3();

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

private:
    Ui::CQDlgSimpleFilter_3x3 *ui;
};

#endif // QDLGSIMPLEFILTER_3X3_H
