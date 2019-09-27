
#ifndef QDLGDATASTREAMSELECTION_H
#define QDLGDATASTREAMSELECTION_H

#include "vDialog.h"
#include <QComboBox>

namespace Ui {
    class CQDlgDataStreamSelection;
}

class CQDlgDataStreamSelection : public VDialog
{
    Q_OBJECT

public:
    explicit CQDlgDataStreamSelection(QWidget *parent = 0);
    ~CQDlgDataStreamSelection();

    void refresh();
    void cancelEvent();
    void okEvent();

    bool inRefreshRoutine;
    bool first;
    int box1Id;
    int box2Id;

    void addElementToCombobox(QComboBox* box,int dataType);

private slots:
    void on_qqTypeCombo_currentIndexChanged(int index);

    void on_qqItemCombo_currentIndexChanged(int index);

    void on_qqOkCancel_accepted();

    void on_qqOkCancel_rejected();

private:
    Ui::CQDlgDataStreamSelection *ui;
};

#endif // QDLGDATASTREAMSELECTION_H
