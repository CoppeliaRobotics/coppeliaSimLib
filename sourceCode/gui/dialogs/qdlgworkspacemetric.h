
#ifndef QDLGWORKSPACEMETRIC_H
#define QDLGWORKSPACEMETRIC_H

#include "vDialog.h"
#include <QAbstractButton>

namespace Ui {
    class CQDlgWorkSpaceMetric;
}

class CQDlgWorkSpaceMetric : public VDialog
{
    Q_OBJECT

public:
    explicit CQDlgWorkSpaceMetric(QWidget *parent = 0);
    ~CQDlgWorkSpaceMetric();

    void refresh();

    void cancelEvent();
    void okEvent();

    float data[4];

private slots:
    void on_qqClose_clicked(QAbstractButton *button);

    void on_qqX_editingFinished();

    void on_qqY_editingFinished();

    void on_qqZ_editingFinished();

    void on_qqAbg_editingFinished();

private:
    Ui::CQDlgWorkSpaceMetric *ui;
};

#endif // QDLGWORKSPACEMETRIC_H
