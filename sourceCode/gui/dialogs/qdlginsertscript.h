
#ifndef QDLGINSERTSCRIPT_H
#define QDLGINSERTSCRIPT_H

#include "vDialog.h"

namespace Ui {
    class CQDlgInsertScript;
}

class CQDlgInsertScript : public VDialog
{
    Q_OBJECT

public:
    explicit CQDlgInsertScript(QWidget *parent = 0);
    ~CQDlgInsertScript();

    void initialize();

    void cancelEvent();
    void okEvent();

    int scriptType;
private slots:
    void on_qqOkCancel_accepted();

    void on_qqOkCancel_rejected();

private:
    Ui::CQDlgInsertScript *ui;
};

#endif // QDLGINSERTSCRIPT_H
