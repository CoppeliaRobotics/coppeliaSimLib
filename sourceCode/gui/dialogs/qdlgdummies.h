
#ifndef QDLGDUMMIES_H
#define QDLGDUMMIES_H

#include <dlgEx.h>

namespace Ui {
    class CQDlgDummies;
}

class CQDlgDummies : public CDlgEx
{
    Q_OBJECT

public:
    explicit CQDlgDummies(QWidget *parent = 0);
    ~CQDlgDummies();

    void refresh();

    void cancelEvent();

    bool inMainRefreshRoutine;

private slots:
    void on_qqSize_editingFinished();

    void on_qqColor_clicked();

    void on_qqApplyMainProperties_clicked();

    void on_qqLinkedDummyCombo_currentIndexChanged(int index);

    void on_qqLinkTypeCombo_currentIndexChanged(int index);

    void on_qqfollowParentOrientation_clicked();

    void on_qqFollow_clicked();

    void on_qqFree_clicked();

    void on_qqFixed_clicked();

    void on_qqOffset_editingFinished();

    void on_qqIncrement_editingFinished();

    void on_qqEditEngine_clicked();

private:
    Ui::CQDlgDummies *ui;
};

#endif // QDLGDUMMIES_H
