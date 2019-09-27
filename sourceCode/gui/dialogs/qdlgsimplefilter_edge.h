
#ifndef QDLGSIMPLEFILTER_EDGE_H
#define QDLGSIMPLEFILTER_EDGE_H

#include "qdlgsimplefilter.h"

namespace Ui {
    class CQDlgSimpleFilter_edge;
}

class CQDlgSimpleFilter_edge : public CQDlgSimpleFilter
{
    Q_OBJECT

public:
    explicit CQDlgSimpleFilter_edge(QWidget *parent = 0);
    ~CQDlgSimpleFilter_edge();

    void refresh();

private slots:
    void on_qqOkCancel_accepted();

    void on_qqOkCancel_rejected();

    void on_qqThreshold_editingFinished();

private:
    Ui::CQDlgSimpleFilter_edge *ui;
};

#endif // QDLGSIMPLEFILTER_EDGE_H
