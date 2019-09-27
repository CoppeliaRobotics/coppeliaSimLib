
#ifndef QDLGUIROLLEDUP_H
#define QDLGUIROLLEDUP_H

#include "vDialog.h"
#include "buttonBlock.h"
#include <QAbstractButton>

namespace Ui {
    class CQDlgUiRolledUp;
}

class CQDlgUiRolledUp : public VDialog
{
    Q_OBJECT

public:
    explicit CQDlgUiRolledUp(QWidget *parent = 0);
    ~CQDlgUiRolledUp();

    void cancelEvent();
    void okEvent();

    void refresh();

    int itBlockID;

private slots:
    void on_qqFirstH_editingFinished();

    void on_qqLastH_editingFinished();

    void on_qqFirstV_editingFinished();

    void on_qqLastV_editingFinished();

    void on_qqClose_clicked(QAbstractButton *button);

private:
    Ui::CQDlgUiRolledUp *ui;
};

#endif // QDLGUIROLLEDUP_H
