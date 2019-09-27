
#ifndef QDLGNEWUI_H
#define QDLGNEWUI_H

#include "vDialog.h"

namespace Ui {
    class CQDlgNewUi;
}

class CQDlgNewUi : public VDialog
{
    Q_OBJECT

public:
    explicit CQDlgNewUi(QWidget *parent = 0);
    ~CQDlgNewUi();

    void cancelEvent();
    void okEvent();

    void refresh();

    int xSize;
    int ySize;
    int menuBarAttributes;

private slots:
    void on_qqTitlebar_clicked();

    void on_qqMinimizeMaximize_clicked();

    void on_qqCloseButton_clicked();

    void on_qqSizeX_editingFinished();

    void on_qqSizeY_editingFinished();

    void on_qqOkCancel_accepted();

    void on_qqOkCancel_rejected();

private:
    Ui::CQDlgNewUi *ui;
};

#endif // QDLGNEWUI_H
