
#ifndef QDLGSIMPLEFILTER_BLOB_H
#define QDLGSIMPLEFILTER_BLOB_H

#include "qdlgsimplefilter.h"

namespace Ui {
    class CQDlgSimpleFilter_blob;
}

class CQDlgSimpleFilter_blob : public CQDlgSimpleFilter
{
    Q_OBJECT

public:
    explicit CQDlgSimpleFilter_blob(QWidget *parent = 0);
    ~CQDlgSimpleFilter_blob();

    void refresh();

private slots:
    void on_qqOkCancel_accepted();

    void on_qqOkCancel_rejected();

    void on_qqThreshold_editingFinished();

    void on_qqDiffColor_clicked();

    void on_qqMinSize_editingFinished();

private:
    Ui::CQDlgSimpleFilter_blob *ui;
};

#endif // QDLGSIMPLEFILTER_BLOB_H
