
#ifndef QDLGUI_H
#define QDLGUI_H

#include "dlgEx.h"
#include <QListWidgetItem>
#include <QShortcut>

namespace Ui {
    class CQDlgUi;
}

class CQDlgUi : public CDlgEx
{
    Q_OBJECT

public:
    explicit CQDlgUi(QWidget *parent = 0);
    ~CQDlgUi();

    void refresh();

    void cancelEvent();


    void updateObjectsInList();
    void selectObjectInList(int objectID);

    bool inMainRefreshRoutine;

    private slots:
    void on_qqAddNew_clicked();

    void on_qqCellCountH_editingFinished();

    void on_qqCellCountV_editingFinished();

    void on_qqCellSizeH_editingFinished();

    void on_qqCellSizeV_editingFinished();

    void on_qqUiPosH_editingFinished();

    void on_qqUiPosV_editingFinished();

    void on_qqUiBorderL_clicked();

    void on_qqUiBorderT_clicked();

    void on_qqVisible_clicked();

    void on_qqMoveable_clicked();

    void on_qqVisibleDuringSimulation_clicked();

    void on_qqFixedWidthFont_clicked();

    void on_qqVisibleWhenObjectSelected_clicked();

    void on_qqSelectObject_clicked();

    void on_qqPageCombo_currentIndexChanged(int index);

    void on_qqAssociatedObject_currentIndexChanged(int index);

    void on_qqSetTexture_clicked();

    void on_qqAdjustRolledUpSizes_clicked();

private:
    Ui::CQDlgUi *ui;
};

#endif // QDLGUI_H
