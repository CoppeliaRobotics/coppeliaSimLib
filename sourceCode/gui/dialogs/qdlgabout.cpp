#include "qdlgabout.h"
#include "ui_qdlgabout.h"
#include "ttUtil.h"
#include "tt.h"
#include "simStrings.h"
#include "simConst.h"
#include "ser.h"
#include "app.h"
#include "simFlavor.h"

CQDlgAbout::CQDlgAbout(QWidget *parent) :
    VDialog(parent,QT_MODAL_DLG_STYLE|Qt::CustomizeWindowHint|Qt::WindowTitleHint), // since Qt5.1: Tool --> Dialog
    ui(new Ui::CQDlgAbout)
{
    ui->setupUi(this);
    initializationEvent();
}

CQDlgAbout::~CQDlgAbout()
{
    delete ui;
}

void CQDlgAbout::cancelEvent()
{
    defaultModalDialogEndRoutine(false);
}

void CQDlgAbout::okEvent()
{
    defaultModalDialogEndRoutine(true);
}

void CQDlgAbout::initializationEvent()
{
    defaultDialogInitializationRoutine();
    QPixmap img;
    img.load(CSimFlavor::getStringVal(1).c_str());
    ui->splashImage->setPixmap(img);
    std::string windowTitle;
    std::string txt;
    CSimFlavor::getAboutStr(windowTitle,txt);
    setWindowTitle(windowTitle.c_str());
    ui->info->setText(txt.c_str());
}

void CQDlgAbout::on_qqClose_clicked(QAbstractButton *button)
{
    defaultModalDialogEndRoutine(true);
}
