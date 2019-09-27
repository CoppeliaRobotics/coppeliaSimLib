
#include "vrepMainHeader.h"
#include "qdlgsimpleSearch.h"
#include "ui_qdlgsimpleSearch.h"

std::string CQDlgSimpleSearch::textToSearch="";
bool CQDlgSimpleSearch::matchCase=false;


CQDlgSimpleSearch::CQDlgSimpleSearch(QWidget *parent) :
    VDialog(parent,QT_MODAL_DLG_STYLE),
    ui(new Ui::CQDlgSimpleSearch)
{
    ui->setupUi(this);
}

CQDlgSimpleSearch::~CQDlgSimpleSearch()
{
    delete ui;
}

void CQDlgSimpleSearch::cancelEvent()
{
//  defaultModalDialogEndRoutine(false);
}

void CQDlgSimpleSearch::okEvent()
{
//  defaultModalDialogEndRoutine(true);
}

void CQDlgSimpleSearch::init()
{
    ui->qqMatchCase->setChecked(matchCase);
    ui->qqText->setText(textToSearch.c_str());
    ui->qqText->selectAll();
}

void CQDlgSimpleSearch::on_qqMatchCase_clicked()
{
   matchCase=!matchCase;
}

void CQDlgSimpleSearch::on_qqFind_clicked()
{
    textToSearch=ui->qqText->text().toStdString();
    defaultModalDialogEndRoutine(true);
}

void CQDlgSimpleSearch::on_qqText_returnPressed()
{
    textToSearch=ui->qqText->text().toStdString();
    defaultModalDialogEndRoutine(true);
}
