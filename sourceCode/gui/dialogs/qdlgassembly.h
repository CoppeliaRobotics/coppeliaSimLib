
#ifndef QDLGASSEMBLY_H
#define QDLGASSEMBLY_H

#include "vDialog.h"
#include "3DObject.h"
#include <QAbstractButton>

namespace Ui {
    class CQDlgAssembly;
}

class CQDlgAssembly : public VDialog
{
    Q_OBJECT

public:
    explicit CQDlgAssembly(QWidget *parent = 0);
    ~CQDlgAssembly();

    void cancelEvent();
    void okEvent();

    void refresh();

    C3DObject* obj;
private slots:
    void on_qqChildMatchValue_editingFinished();

    void on_qqParentMatchValue_editingFinished();

    void on_qqSetLocalMatrix_clicked();

    void on_qqClose_clicked(QAbstractButton *button);

    void on_qqHasMatrix_clicked();

private:
    Ui::CQDlgAssembly *ui;
};

#endif // QDLGASSEMBLY_H
