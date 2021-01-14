
#ifndef QDLGPATHEDIT_H
#define QDLGPATHEDIT_H

#include "dlgEx.h"
#include "path_old.h"

namespace Ui {
    class CQDlgPathEdit;
}

class CQDlgPathEdit : public CDlgEx
{
    Q_OBJECT

public:
    explicit CQDlgPathEdit(QWidget *parent = 0);
    ~CQDlgPathEdit();

    void refresh();
    void cancelEvent();
    void okEvent();

    CPathCont_old* getPathCont();

private slots:
    void on_qqClosed_clicked();

    void on_qqFlat_clicked();

    void on_qqAutomaticOrientation_clicked();

    void on_qqKeepXup_clicked();

    void on_qqFactor1_editingFinished();

    void on_qqFactor2_editingFinished();

    void on_qqPointCount_editingFinished();

    void on_qqVirtualDistance_editingFinished();

    void on_qqApply_clicked();

    void on_qqClearSelection_clicked();

    void on_qqInvertSelection_clicked();

    void on_qqAuxFlags_editingFinished();

    void on_qqAuxChannel1_editingFinished();

    void on_qqAuxChannel2_editingFinished();

    void on_qqAuxChannel3_editingFinished();

    void on_qqAuxChannel4_editingFinished();

    void on_qqMakeDummies_clicked();

private:
    Ui::CQDlgPathEdit *ui;
};

#endif // QDLGPATHEDIT_H
