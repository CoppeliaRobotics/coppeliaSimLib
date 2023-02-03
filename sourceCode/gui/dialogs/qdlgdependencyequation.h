
#ifndef QDLGDEPENDENCYEQUATION_H
#define QDLGDEPENDENCYEQUATION_H

#include <vDialog.h>
#include <QAbstractButton>

namespace Ui {
    class CQDlgDependencyEquation;
}

class CQDlgDependencyEquation : public VDialog
{
    Q_OBJECT

public:
    explicit CQDlgDependencyEquation(QWidget *parent = 0);
    ~CQDlgDependencyEquation();

    void cancelEvent();
    void okEvent();

    void refresh();

    bool inMainRefreshRoutine;

private slots:
    void on_qqClose_clicked(QAbstractButton *button);

    void on_qqOffset_editingFinished();

    void on_qqCoeff_editingFinished();

    void on_qqCombo_currentIndexChanged(int index);

private:
    Ui::CQDlgDependencyEquation *ui;
};

#endif // QDLGDEPENDENCYEQUATION_H
