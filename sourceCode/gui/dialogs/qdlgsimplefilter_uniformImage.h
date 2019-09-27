
#ifndef QDLGSIMPLEFILTER_UNIFORMIMAGE_H
#define QDLGSIMPLEFILTER_UNIFORMIMAGE_H

#include "qdlgsimplefilter.h"

namespace Ui {
    class CQDlgSimpleFilter_uniformImage;
}

class CQDlgSimpleFilter_uniformImage : public CQDlgSimpleFilter
{
    Q_OBJECT

public:
    explicit CQDlgSimpleFilter_uniformImage(QWidget *parent = 0);
    ~CQDlgSimpleFilter_uniformImage();

    void refresh();

private slots:
    void on_qqOkCancel_accepted();

    void on_qqOkCancel_rejected();

    void on_qqRed_editingFinished();

    void on_qqGreen_editingFinished();

    void on_qqBlue_editingFinished();

private:
    Ui::CQDlgSimpleFilter_uniformImage *ui;
};

#endif // QDLGSIMPLEFILTER_UNIFORMIMAGE_H
