
#ifndef QDLGSLIDER2_H
#define QDLGSLIDER2_H

#include "vDialog.h"
#include <QAbstractButton>

namespace Ui {
    class CQDlgSlider2;
}

class CQDlgSlider2 : public VDialog
{
    Q_OBJECT

public:
    explicit CQDlgSlider2(QWidget *parent = 0);
    ~CQDlgSlider2();

    void cancelEvent();
    void okEvent();

    void refresh();

    int opMode;
    int resolution;
    int iterationCnt;

private slots:
    void on_qqOkCancelButtons_accepted();
    void on_qqOkCancelButtons_rejected();
    void on_qqSlider1_sliderMoved(int position);
    void on_qqSlider2_sliderMoved(int position);

private:
    int _resolutionLevel;

    void _displayMeshInsideExtractionTexts();
    Ui::CQDlgSlider2 *ui;
};

#endif // QDLGSLIDER2_H
