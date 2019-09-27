
#ifndef QDLGABOUT_H
#define QDLGABOUT_H
#include "vDialog.h"
#include <QAbstractButton>

namespace Ui {
    class CQDlgAbout;
}

class CQDlgAbout : public VDialog
{
    Q_OBJECT

public:
    explicit CQDlgAbout(QWidget *parent = 0);
    ~CQDlgAbout();

    void cancelEvent();
    void okEvent();
    void initializationEvent();

private slots:
    void on_qqClose_clicked(QAbstractButton *button);

private:
    Ui::CQDlgAbout *ui;

};
#endif // QDLGABOUT_H
