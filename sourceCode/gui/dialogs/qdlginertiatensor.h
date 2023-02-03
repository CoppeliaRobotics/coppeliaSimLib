#ifndef QDLGINERTIATENSOR_H
#define QDLGINERTIATENSOR_H

#include <vDialog.h>
#include <simTypes.h>

namespace Ui {
    class CQDlgInertiaTensor;
}

class CQDlgInertiaTensor : public VDialog
{
    Q_OBJECT

public:
    explicit CQDlgInertiaTensor(QWidget *parent = 0);
    ~CQDlgInertiaTensor();

    void cancelEvent();
    void okEvent();

    void refresh();

    double tensor[9];
    double com[9];
    bool applyToSelection;

private slots:
    void on_qqI11_editingFinished();

    void on_qqI21_editingFinished();

    void on_qqI31_editingFinished();

    void on_qqI12_editingFinished();

    void on_qqI22_editingFinished();

    void on_qqI32_editingFinished();

    void on_qqI13_editingFinished();

    void on_qqI23_editingFinished();

    void on_qqI33_editingFinished();

    void on_qqOkCancel_accepted();

    void on_qqOkCancel_rejected();

    void on_qqMx_editingFinished();

    void on_qqMy_editingFinished();

    void on_qqMz_editingFinished();

    void on_qqApplyToSelection_clicked(bool checked);

private:
    Ui::CQDlgInertiaTensor *ui;
};

#endif // QDLGINERTIATENSOR_H
