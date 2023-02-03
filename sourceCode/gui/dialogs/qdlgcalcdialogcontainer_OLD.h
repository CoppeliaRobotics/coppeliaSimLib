#ifndef QDLGCALCDIALOGCONTAINER_H
#define QDLGCALCDIALOGCONTAINER_H

#include <dlgEx.h>
#include <QVBoxLayout>

namespace Ui {
    class CQDlgCalcDialogContainer_OLD;
}

class CQDlgCalcDialogContainer_OLD : public CDlgEx
{
    Q_OBJECT

public:
    explicit CQDlgCalcDialogContainer_OLD(QWidget *parent = 0);
    ~CQDlgCalcDialogContainer_OLD();

    void refresh();
    void dialogCallbackFunc(const SUIThreadCommand* cmdIn,SUIThreadCommand* cmdOut);

    QVBoxLayout* bl;
    int currentPage;
    int desiredPage;
    int topBorderWidth;

    CDlgEx* pageDlgs[3];
    int originalHeights[3];


private slots:
    void on_qqCollision_clicked();

    void on_qqDistance_clicked();

    void on_qqIk_clicked();

private:
    Ui::CQDlgCalcDialogContainer_OLD *ui;
};

#endif // QDLGCALCDIALOGCONTAINER_H
