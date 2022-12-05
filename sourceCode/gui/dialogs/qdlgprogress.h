#ifndef QDLGPROGRESS_H
#define QDLGPROGRESS_H

#include <QDialog>
#include "simTypes.h"

namespace Ui {
    class CQDlgProgress;
}

class CQDlgProgress : public QDialog
{
    Q_OBJECT

public:
    explicit CQDlgProgress(QWidget *parent = 0);
    ~CQDlgProgress();

    void updateProgress(double p,const char* txt);

private:
    Ui::CQDlgProgress *ui;
};

#endif // QDLGPROGRESS_H
