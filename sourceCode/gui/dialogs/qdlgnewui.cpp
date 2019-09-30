#include "qdlgnewui.h"
#include "ui_qdlgnewui.h"
#include "v_rep_internal.h"
#include "tt.h"
#include "app.h"

CQDlgNewUi::CQDlgNewUi(QWidget *parent) :
    VDialog(parent,QT_MODAL_DLG_STYLE),
    ui(new Ui::CQDlgNewUi)
{
    ui->setupUi(this);
    xSize=10;
    ySize=8;
    menuBarAttributes=sim_ui_menu_title|sim_ui_menu_minimize|sim_ui_menu_close;
}

CQDlgNewUi::~CQDlgNewUi()
{
    delete ui;
}

void CQDlgNewUi::cancelEvent()
{
    defaultModalDialogEndRoutine(false);
}

void CQDlgNewUi::okEvent()
{
    defaultModalDialogEndRoutine(true);
}

void CQDlgNewUi::refresh()
{
    ui->qqSizeX->setText(tt::getIString(false,xSize).c_str());
    ui->qqSizeY->setText(tt::getIString(false,ySize).c_str());
    ui->qqMinimizeMaximize->setEnabled(menuBarAttributes&sim_ui_menu_title);
    ui->qqCloseButton->setEnabled(menuBarAttributes&sim_ui_menu_title);
    ui->qqTitlebar->setChecked(menuBarAttributes&sim_ui_menu_title);
    ui->qqMinimizeMaximize->setChecked(menuBarAttributes&sim_ui_menu_minimize);
    ui->qqCloseButton->setChecked(menuBarAttributes&sim_ui_menu_close);
}

void CQDlgNewUi::on_qqTitlebar_clicked()
{
    menuBarAttributes^=sim_ui_menu_title;
    if ((menuBarAttributes&sim_ui_menu_title)==0)
        menuBarAttributes=0;
    refresh();
}

void CQDlgNewUi::on_qqMinimizeMaximize_clicked()
{
    menuBarAttributes^=sim_ui_menu_minimize;
    refresh();
}

void CQDlgNewUi::on_qqCloseButton_clicked()
{
    menuBarAttributes^=sim_ui_menu_close;
    refresh();
}

void CQDlgNewUi::on_qqSizeX_editingFinished()
{
    bool ok;
    int newVal=ui->qqSizeX->text().toInt(&ok);
    if (ok)
    {
        tt::limitValue(3,256,newVal);
        xSize=newVal;
    }
    refresh();
}

void CQDlgNewUi::on_qqSizeY_editingFinished()
{
    bool ok;
    int newVal=ui->qqSizeY->text().toInt(&ok);
    if (ok)
    {
        tt::limitValue(2,256,newVal);
        ySize=newVal;
    }
    refresh();
}

void CQDlgNewUi::on_qqOkCancel_accepted()
{
    defaultModalDialogEndRoutine(true);
}

void CQDlgNewUi::on_qqOkCancel_rejected()
{
    defaultModalDialogEndRoutine(false);
}
