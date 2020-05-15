#ifndef QDLGCALCDIALOGCONTAINER_H
#define QDLGCALCDIALOGCONTAINER_H

#include "dlgEx.h"
#include <QVBoxLayout>

namespace Ui {
    class CQDlgCalcDialogContainer;
}

class CQDlgCalcDialogContainer : public CDlgEx
{
    Q_OBJECT

public:
    explicit CQDlgCalcDialogContainer(QWidget *parent = 0);
    ~CQDlgCalcDialogContainer();

    void refresh();
    void dialogCallbackFunc(const SUIThreadCommand* cmdIn,SUIThreadCommand* cmdOut);

    QVBoxLayout* bl;
    int currentPage;
    int desiredPage;
    int topBorderWidth;

    CDlgEx* pageDlgs[4];
    int originalHeights[4];


private slots:
    void on_qqCollision_clicked();

    void on_qqDistance_clicked();

    void on_qqIk_clicked();

    void on_qqDynamics_clicked();

private:
    Ui::CQDlgCalcDialogContainer *ui;
};

#endif // QDLGCALCDIALOGCONTAINER_H
