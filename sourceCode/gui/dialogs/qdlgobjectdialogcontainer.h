#ifndef QDLGOBJECTDIALOGCONTAINER_H
#define QDLGOBJECTDIALOGCONTAINER_H

#include <dlgEx.h>
#include <QVBoxLayout>

namespace Ui {
    class CQDlgObjectDialogContainer;
}

class CQDlgObjectDialogContainer : public CDlgEx
{
    Q_OBJECT

public:
    explicit CQDlgObjectDialogContainer(QWidget *parent = 0);
    ~CQDlgObjectDialogContainer();

    void refresh();
    void dialogCallbackFunc(const SUIThreadCommand* cmdIn,SUIThreadCommand* cmdOut);

    QVBoxLayout* bl;
    int objTypeDlg;
    int currentPage;
    int desiredPage;

    CDlgEx* pageDlgs[2];
    int originalHeights[2];

private slots:
    void on_qqObjectProp_clicked();

    void on_qqCommonProp_clicked();

private:
    Ui::CQDlgObjectDialogContainer *ui;
};

#endif // QDLGOBJECTDIALOGCONTAINER_H
