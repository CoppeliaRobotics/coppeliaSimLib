#ifndef QDLGSCALING_H
#define QDLGSCALING_H

#include "vDialog.h"
#include "simTypes.h"

namespace Ui {
    class CQDlgScaling;
}

class CQDlgScaling : public VDialog
{
    Q_OBJECT

public:
    explicit CQDlgScaling(QWidget *parent = 0);
    ~CQDlgScaling();

    void cancelEvent();
    void okEvent();

    void refresh();

    bool inPlace;
    double factor;

    void _doTheScaling();

private slots:
    void on_qqInPlace_clicked();

    void on_qqScalingFactor_editingFinished();

    void on_qqOkCancel_accepted();

    void on_qqOkCancel_rejected();

private:
    Ui::CQDlgScaling *ui;
};

#endif // QDLGSCALING_H
