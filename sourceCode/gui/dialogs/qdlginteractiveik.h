
#ifndef QDLGINTERACTIVEIK_H
#define QDLGINTERACTIVEIK_H

#include "dlgEx.h"

namespace Ui {
    class CQDlgInteractiveIk;
}

class CQDlgInteractiveIk : public CDlgEx
{
    Q_OBJECT

public:
    explicit CQDlgInteractiveIk(QWidget *parent = 0);
    ~CQDlgInteractiveIk();

    void refresh();

private slots:

private:
    Ui::CQDlgInteractiveIk *ui;
};

#endif // QDLGINTERACTIVEIK_H
