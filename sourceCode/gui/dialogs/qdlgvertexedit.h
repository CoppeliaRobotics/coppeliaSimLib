
#ifndef QDLGVERTEXEDIT_H
#define QDLGVERTEXEDIT_H

#include "dlgEx.h"

namespace Ui {
    class CQDlgVertexEdit;
}

class CQDlgVertexEdit : public CDlgEx
{
    Q_OBJECT

public:
    explicit CQDlgVertexEdit(QWidget *parent = 0);
    ~CQDlgVertexEdit();

    void refresh();

    void cancelEvent();

protected:
private slots:
    void on_qqShowHiddenVertices_clicked();

    void on_qqInsertTriangles_clicked();

    void on_qqInsertFan_clicked();

    void on_qqMakeDummies_clicked();

    void on_qqClearSelection_clicked();

    void on_qqInvertSelection_clicked();

private:
    Ui::CQDlgVertexEdit *ui;
};

#endif // QDLGVERTEXEDIT_H
