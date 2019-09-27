
#ifndef QDLGTRIANGLEEDIT_H
#define QDLGTRIANGLEEDIT_H

#include "dlgEx.h"

namespace Ui {
    class CQDlgTriangleEdit;
}

class CQDlgTriangleEdit : public CDlgEx
{
    Q_OBJECT

public:
    explicit CQDlgTriangleEdit(QWidget *parent = 0);
    ~CQDlgTriangleEdit();

    void refresh();

    void cancelEvent();

private slots:
    void on_qqMakeShape_clicked();

    void on_qqMakeCuboid_clicked();

    void on_qqMakeCylinder_clicked();

    void on_qqMakeSphere_clicked();

    void on_qqFlip_clicked();

    void on_qqSubdivide_clicked();

    void on_qqClearSelection_clicked();

    void on_qqInvertSelection_clicked();

private:
    Ui::CQDlgTriangleEdit *ui;
};

#endif // QDLGTRIANGLEEDIT_H
