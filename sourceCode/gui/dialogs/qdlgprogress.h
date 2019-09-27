
#ifndef QDLGPROGRESS_H
#define QDLGPROGRESS_H

#include <QDialog>

namespace Ui {
    class CQDlgProgress;
}

class CQDlgProgress : public QDialog
{
    Q_OBJECT

public:
    explicit CQDlgProgress(QWidget *parent = 0);
    ~CQDlgProgress();

    void updateProgress(float p,const char* txt);

private:
    Ui::CQDlgProgress *ui;
};

#endif // QDLGPROGRESS_H
