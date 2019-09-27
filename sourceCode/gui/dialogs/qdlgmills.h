
#ifndef QDLGMILLS_H
#define QDLGMILLS_H

#include "dlgEx.h"

namespace Ui {
    class CQDlgMills;
}

class CQDlgMills : public CDlgEx
{
    Q_OBJECT

public:
    explicit CQDlgMills(QWidget *parent = 0);
    ~CQDlgMills();

    void refresh();

    void cancelEvent();

    bool inMainRefreshRoutine;
private slots:
    void on_qqEnableAll_clicked();

    void on_qqExplicitHandling_clicked();

    void on_qqBaseSize_editingFinished();

    void on_qqAdjustVolume_clicked();

    void on_qqEntityToCut_currentIndexChanged(int index);

    void on_qqPassiveVolumeColor_clicked();

    void on_qqActiveVolumeColor_clicked();

private:
    Ui::CQDlgMills *ui;
};

#endif // QDLGMILLS_H
