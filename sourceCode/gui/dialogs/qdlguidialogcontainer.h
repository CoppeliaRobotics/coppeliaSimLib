
#ifndef QDLGUIDIALOGCONTAINER_H
#define QDLGUIDIALOGCONTAINER_H

#include "dlgEx.h"
#include <QVBoxLayout>

namespace Ui {
    class CQDlgUiDialogContainer;
}

class CQDlgUiDialogContainer : public CDlgEx
{
    Q_OBJECT

public:
    explicit CQDlgUiDialogContainer(QWidget *parent = 0);
    ~CQDlgUiDialogContainer();

    void refresh();
    void cancelEvent();
    void okEvent();

    QVBoxLayout* bl;
    int currentPage;

    CDlgEx* pageDlgs[2];
    int originalHeights[2];

private slots:

private:
    Ui::CQDlgUiDialogContainer *ui;
};

#endif // QDLGUIDIALOGCONTAINER_H
