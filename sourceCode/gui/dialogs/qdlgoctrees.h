
#ifndef QDLGOCTREES_H
#define QDLGOCTREES_H

#include "dlgEx.h"

namespace Ui {
    class CQDlgOctrees;
}

class CQDlgOctrees : public CDlgEx
{
    Q_OBJECT

public:
    explicit CQDlgOctrees(QWidget *parent = 0);
    ~CQDlgOctrees();

    void refresh();

    void cancelEvent();

private slots:
    void on_qqSize_editingFinished();

    void on_qqColor_clicked();

    void on_qqShowOctree_clicked();

    void on_qqRandomColors_clicked();

    void on_qqUsePoints_clicked();

    void on_qqPointSize_editingFinished();

    void on_qqClear_clicked();

    void on_qqInsert_clicked();

    void on_qqSubtract_clicked();

    void on_qqEmissiveColor_clicked();

private:
    Ui::CQDlgOctrees *ui;
};

#endif // QDLGOCTREES_H
