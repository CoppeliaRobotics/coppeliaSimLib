
#ifndef QDLGMESSAGEANDCHECKBOX_H
#define QDLGMESSAGEANDCHECKBOX_H

#include <vDialog.h>

namespace Ui {
    class CQDlgMessageAndCheckbox;
}

class CQDlgMessageAndCheckbox : public VDialog
{
    Q_OBJECT

public:
    explicit CQDlgMessageAndCheckbox(QWidget *parent = 0);
    ~CQDlgMessageAndCheckbox();

    void cancelEvent();
    void okEvent();

    void refresh();

    std::string title;
    std::string text;
    std::string checkbox;
    bool checkboxState;

private slots:
    void on_qqCheckbox_clicked();
    void on_qqOk_accepted();

private:
    Ui::CQDlgMessageAndCheckbox *ui;
};

#endif // QDLGMESSAGEANDCHECKBOX_H
