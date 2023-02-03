
#include <qdlgmessageandcheckbox.h>
#include <ui_qdlgmessageandcheckbox.h>
#include <app.h>

CQDlgMessageAndCheckbox::CQDlgMessageAndCheckbox(QWidget *parent) : VDialog(parent,QT_MODAL_DLG_STYLE), ui(new Ui::CQDlgMessageAndCheckbox)
{
    ui->setupUi(this);
    checkboxState=false;
}

CQDlgMessageAndCheckbox::~CQDlgMessageAndCheckbox()
{
    delete ui;
}

void CQDlgMessageAndCheckbox::cancelEvent()
{
//  defaultModalDialogEndRoutine(false);
}

void CQDlgMessageAndCheckbox::okEvent()
{
//  defaultModalDialogEndRoutine(true);
}

void CQDlgMessageAndCheckbox::refresh()
{
    setWindowTitle(title.c_str());
    ui->qqText->setText(text.c_str());
    ui->qqCheckboxText->setText(checkbox.c_str());
    ui->qqCheckbox->setChecked(checkboxState);
}

void CQDlgMessageAndCheckbox::on_qqCheckbox_clicked()
{
    checkboxState=!checkboxState;
    refresh();
}

void CQDlgMessageAndCheckbox::on_qqOk_accepted()
{
    defaultModalDialogEndRoutine(true);
}
