#include "qdlgcalcdialogcontainer_OLD.h"
#include "ui_qdlgcalcdialogcontainer_OLD.h"
#include "app.h"
#include "qdlgcollisions.h"
#include "qdlgdistances.h"
#include "qdlgik.h"

CQDlgCalcDialogContainer_OLD::CQDlgCalcDialogContainer_OLD(QWidget *parent) :
    CDlgEx(parent),
    ui(new Ui::CQDlgCalcDialogContainer_OLD)
{
    _dlgType=CALCULATION_DLG_OLD;
    ui->setupUi(this);

    topBorderWidth=35;
    QRect geom=ui->qqGroupBox->geometry();
    geom.setHeight(57);
    ui->qqGroupBox->setGeometry(geom);

    pageDlgs[0]=new CQDlgCollisions();
    originalHeights[0]=pageDlgs[0]->size().height();

    pageDlgs[1]=new CQDlgDistances();
    originalHeights[1]=pageDlgs[1]->size().height();

    pageDlgs[2]=new CQDlgIk();
    originalHeights[2]=pageDlgs[2]->size().height();

    currentPage=0;
    desiredPage=0;
    bl=new QVBoxLayout();
    bl->setContentsMargins(0,topBorderWidth,0,0);
    setLayout(bl);
    bl->addWidget(pageDlgs[0]);
    bl->addWidget(pageDlgs[1]);
    pageDlgs[1]->setVisible(false);
    bl->addWidget(pageDlgs[2]);
    pageDlgs[2]->setVisible(false);

    QSize s(pageDlgs[currentPage]->size());
    s.setHeight(originalHeights[currentPage]+topBorderWidth);
    setFixedSize(s);
}

CQDlgCalcDialogContainer_OLD::~CQDlgCalcDialogContainer_OLD()
{
    delete ui;
}

void CQDlgCalcDialogContainer_OLD::dialogCallbackFunc(const SUIThreadCommand* cmdIn,SUIThreadCommand* cmdOut)
{
    pageDlgs[currentPage]->dialogCallbackFunc(cmdIn,cmdOut);
}

void CQDlgCalcDialogContainer_OLD::refresh()
{
    ui->qqCollision->setChecked(desiredPage==0);
    ui->qqDistance->setChecked(desiredPage==1);
    ui->qqIk->setChecked(desiredPage==2);

    if (desiredPage!=currentPage)
    {
        pageDlgs[currentPage]->setVisible(false);
        currentPage=desiredPage;
        pageDlgs[currentPage]->setVisible(true);

        QSize s(pageDlgs[currentPage]->size());
        s.setHeight(originalHeights[currentPage]+topBorderWidth);

        setFixedSize(s);
    }
    pageDlgs[currentPage]->refresh();
}

void CQDlgCalcDialogContainer_OLD::on_qqCollision_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (App::userSettings->showOldDlgs)
        {
            desiredPage=0;
            refresh();
        }
    }
}

void CQDlgCalcDialogContainer_OLD::on_qqDistance_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (App::userSettings->showOldDlgs)
        {
            desiredPage=1;
            refresh();
        }
    }
}

void CQDlgCalcDialogContainer_OLD::on_qqIk_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if (App::userSettings->showOldDlgs)
        {
            desiredPage=2;
            refresh();
        }
    }
}

