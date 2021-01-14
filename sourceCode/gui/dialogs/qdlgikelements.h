
#ifndef QDLGIKELEMENTS_H
#define QDLGIKELEMENTS_H

#include "dlgEx.h"
#include "ikGroup_old.h"
#include <QAbstractButton>

namespace Ui {
    class CQDlgIkElements;
}

class CQDlgIkElements : public CDlgEx
{
    Q_OBJECT

public:
    explicit CQDlgIkElements(QWidget *parent = 0);
    ~CQDlgIkElements();

    void cancelEvent();
    void refresh();
    void dialogCallbackFunc(const SUIThreadCommand* cmdIn,SUIThreadCommand* cmdOut);
    bool needsDestruction();
    bool isLinkedDataValid();
    bool doesInstanceSwitchRequireDestruction();

    void _initialize(int ikGroupHandle);
    static void display(int ikGroupHandle,QWidget* theParentWindow);

    void updateObjectsInList();
    int getSelectedObjectID();
    void selectObjectInList(int objectID);

    bool inMainRefreshRoutine;
    bool inListSelectionRoutine;
    bool noListSelectionAllowed;

    static int _ikGroupHandle;
    static bool _invalid;

private slots:
    void onDeletePressed();

    void on_qqAddNewElement_clicked();

    void on_qqList_itemSelectionChanged();

    void on_qqActive_clicked();

    void on_qqBaseCombo_currentIndexChanged(int index);

    void on_qqX_clicked();

    void on_qqY_clicked();

    void on_qqZ_clicked();

    void on_qqAlphaBeta_clicked();

    void on_qqGamma_clicked();

    void on_qqRelativeCombo_currentIndexChanged(int index);

    void on_qqPrecisionLinear_editingFinished();

    void on_qqPrecisionAngular_editingFinished();

    void on_qqWeightLinear_editingFinished();

    void on_qqWeightAngular_editingFinished();

private:
    Ui::CQDlgIkElements *ui;
};

#endif // QDLGIKELEMENTS_H
