
#ifndef QDLGTEXTURESELECTION_H
#define QDLGTEXTURESELECTION_H

#include <vDialog.h>

namespace Ui {
    class CQDlgTextureSelection;
}

class CQDlgTextureSelection : public VDialog
{
    Q_OBJECT

public:
    explicit CQDlgTextureSelection(QWidget *parent = 0);
    ~CQDlgTextureSelection();

    void cancelEvent();
    void okEvent();

    void refresh();

    int selectedTextureObject;

private slots:
    void on_qqOkCancel_accepted();

    void on_qqOkCancel_rejected();

private:
    Ui::CQDlgTextureSelection *ui;
};

#endif // QDLGTEXTURESELECTION_H
