
#ifndef QDLGMIRRORS_H
#define QDLGMIRRORS_H

#include <dlgEx.h>

namespace Ui {
    class CQDlgMirrors;
}

class CQDlgMirrors : public CDlgEx
{
    Q_OBJECT

public:
    explicit CQDlgMirrors(QWidget *parent = 0);
    ~CQDlgMirrors();

    void refresh();

    void cancelEvent();
    bool inMainRefreshRoutine;
private slots:
    void on_qqEnabled_clicked();
    void on_qqWidth_editingFinished();
    void on_qqHeight_editingFinished();
    void on_qqColor_clicked();

    void on_qqReflectance_editingFinished();

    void on_qqIsMirror_clicked();

    void on_qqIsClippingPlane_clicked();

    void on_qqEntityCombo_currentIndexChanged(int index);

    void on_qqDisableAllMirrors_clicked();

    void on_qqDisableAllClippingPlanes_clicked();

private:
    Ui::CQDlgMirrors *ui;
};

#endif // QDLGMIRRORS_H
