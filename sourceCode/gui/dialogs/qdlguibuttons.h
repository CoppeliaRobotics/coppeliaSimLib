
#ifndef QDLGUIBUTTONS_H
#define QDLGUIBUTTONS_H

#include "dlgEx.h"
#include "softButton.h"

namespace Ui {
    class CQDlgUiButtons;
}

class CQDlgUiButtons : public CDlgEx
{
    Q_OBJECT

public:
    explicit CQDlgUiButtons(QWidget *parent = 0);
    ~CQDlgUiButtons();

    void refresh();

    void cancelEvent();

    bool inMainRefreshRoutine;

    CSoftButton* getLastSelectedButton();
    int getLastSelectedButtonId();
    int getSelectedButtonId(int index);

private slots:
    void on_qqInsertButtons_clicked();

    void on_qqInsertMergedButton_clicked();

    void on_qqButtonHandle_editingFinished();

    void on_qqTypeCombo_currentIndexChanged(int index);

    void on_qqEnabled_clicked();

    void on_qqStayDown_clicked();

    void on_qqRollUp_clicked();

    void on_qqCenteredH_clicked();

    void on_qqUpDownEvent_clicked();

    void on_qqCloseAction_clicked();

    void on_qqCenteredV_clicked();

    void on_qqBorderless_clicked();

    void on_qqIgnoreMouse_clicked();

    void on_qqApplyType_clicked();

    void on_qqLabelUp_editingFinished();

    void on_qqLabelDown_editingFinished();

    void on_qqApplyLabel_clicked();

    void on_qqColorUp_clicked();

    void on_qqColorDown_clicked();

    void on_qqColorLabel_clicked();

    void on_qqApplyColor_clicked();

    void on_qqTransparent_clicked();

    void on_qqNoBackground_clicked();

    void on_qqSetTexture_clicked();

    void on_qqApplyOtherProperties_clicked();

private:
    Ui::CQDlgUiButtons *ui;
};

#endif // QDLGUIBUTTONS_H
