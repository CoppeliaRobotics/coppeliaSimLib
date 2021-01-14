
#ifndef QDLGIKCONDITIONAL_H
#define QDLGIKCONDITIONAL_H

#include "vDialog.h"
#include "ikGroup_old.h"
#include <QAbstractButton>

namespace Ui {
    class CQDlgIkConditional;
}

class CQDlgIkConditional : public VDialog
{
    Q_OBJECT

public:
    explicit CQDlgIkConditional(QWidget *parent = 0);
    ~CQDlgIkConditional();

    void cancelEvent();
    void okEvent();

    void refresh();

    bool inMainRefreshRoutine;
    CIkGroup_old* ikGroup;

    int doOnFailOrSuccessOf;
    int ikResult;
    bool restoreIfPosNotReached;
    bool restoreIfOrNotReached;

    bool firstHere;

private slots:
    void on_qqClose_clicked(QAbstractButton *button);

    void on_qqIkGroupCombo_currentIndexChanged(int index);

    void on_qqIkResultCombo_currentIndexChanged(int index);

    void on_qqFailedLinear_clicked();

    void on_qqFailedAngular_clicked();

private:
    Ui::CQDlgIkConditional *ui;
};

#endif // QDLGIKCONDITIONAL_H
