
#include "qdlgprogress.h"
#include "ui_qdlgprogress.h"

CQDlgProgress::CQDlgProgress(QWidget *parent) : QDialog(parent,Qt::SplashScreen), ui(new Ui::CQDlgProgress)
{
    ui->setupUi(this);
    ui->qqBar->setMinimum(0);
    ui->qqBar->setMaximum(100);
    ui->qqBar->setValue(0);
}

CQDlgProgress::~CQDlgProgress()
{
    delete ui;
}

void CQDlgProgress::updateProgress(double p,const char* txt)
{
    if (p<0.0)
        ui->qqBar->setMaximum(0);
    else
        ui->qqBar->setValue(int(p*100.0));
    ui->qqText->setText(txt);
}


