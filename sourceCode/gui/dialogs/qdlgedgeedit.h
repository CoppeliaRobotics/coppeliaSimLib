
#ifndef QDLGEDGEEDIT_H
#define QDLGEDGEEDIT_H

#include <dlgEx.h>

namespace Ui {
    class CQDlgEdgeEdit;
}

class CQDlgEdgeEdit : public CDlgEx
{
    Q_OBJECT

public:
    explicit CQDlgEdgeEdit(QWidget *parent = 0);
    ~CQDlgEdgeEdit();

    void refresh();

    void cancelEvent();

private slots:
    void on_qqShowHiddenEdges_clicked();

    void on_qqAutoFollowing_clicked();

    void on_qqEdgeAngle_editingFinished();

    void on_qqDirectionChangeAngle_editingFinished();

    void on_qqMakePath_clicked();

    void on_qqClearSelection_clicked();

    void on_qqInvertSelection_clicked();

private:
    Ui::CQDlgEdgeEdit *ui;
};

#endif // QDLGEDGEEDIT_H
