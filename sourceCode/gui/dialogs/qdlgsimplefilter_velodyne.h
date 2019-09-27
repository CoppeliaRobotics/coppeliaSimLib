
#ifndef QDLGSIMPLEFILTER_VELODYNE_H
#define QDLGSIMPLEFILTER_VELODYNE_H

#include "qdlgsimplefilter.h"

namespace Ui {
    class CQDlgSimpleFilter_velodyne;
}

class CQDlgSimpleFilter_velodyne : public CQDlgSimpleFilter
{
    Q_OBJECT

public:
    explicit CQDlgSimpleFilter_velodyne(QWidget *parent = 0);
    ~CQDlgSimpleFilter_velodyne();

    void refresh();

private slots:
    void on_qqOkCancel_accepted();

    void on_qqOkCancel_rejected();

    void on_qqX_editingFinished();

    void on_qqY_editingFinished();

    void on_qqV_editingFinished();

private:
    Ui::CQDlgSimpleFilter_velodyne *ui;
};

#endif // QDLGSIMPLEFILTER_VELODYNE_H
