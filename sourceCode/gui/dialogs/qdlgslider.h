#ifndef QDLGSLIDER_H
#define QDLGSLIDER_H

#include "vDialog.h"
#include <QAbstractButton>
#include "simTypes.h"

namespace Ui {
    class CQDlgSlider;
}

class CQDlgSlider : public VDialog
{
    Q_OBJECT

public:
    explicit CQDlgSlider(QWidget *parent = 0);
    ~CQDlgSlider();

    void cancelEvent();
    void okEvent();

    void refresh();

    int opMode;
    int triCnt;
    double decimationPercent;

private slots:

    void on_qqOkCancelButtons_accepted();
    void on_qqOkCancelButtons_rejected();
    void on_qqSlider_sliderMoved(int position);

private:
    void _displayDecimationText();
    Ui::CQDlgSlider *ui;
};

#endif // QDLGSLIDER_H
