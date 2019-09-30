#include "qdlgcalcdialogcontainer.h"
#include "ui_qdlgcalcdialogcontainer.h"
#include "app.h"
#include "qdlgcollisions.h"
#include "qdlgdistances.h"
#include "qdlgconstraintsolver.h"
#include "qdlgdynamics.h"
#include "qdlgik.h"
#include "qdlgpathplanning.h"
#include "qdlgmotionplanning.h"

CQDlgCalcDialogContainer::CQDlgCalcDialogContainer(QWidget *parent) :
    CDlgEx(parent),
    ui(new Ui::CQDlgCalcDialogContainer)
{
    _dlgType=CALCULATION_DLG;
    ui->setupUi(this);

    if (App::userSettings->enableOldCalcModuleGuis)
        topBorderWidth=60;
    else
    {
        topBorderWidth=35;
        QRect geom=ui->qqGroupBox->geometry();
        geom.setHeight(57);
        ui->qqGroupBox->setGeometry(geom);
    }
    ui->qqPathPlanning->setVisible(App::userSettings->enableOldCalcModuleGuis);
    ui->qqMotionPlanning->setVisible(App::userSettings->enableOldCalcModuleGuis);
    ui->qqGcs->setVisible(App::userSettings->enableOldCalcModuleGuis);

    pageDlgs[0]=new CQDlgCollisions();
    originalHeights[0]=pageDlgs[0]->size().height();

    pageDlgs[1]=new CQDlgDistances();
    originalHeights[1]=pageDlgs[1]->size().height();

    pageDlgs[2]=new CQDlgIk();
    originalHeights[2]=pageDlgs[2]->size().height();

    pageDlgs[3]=new CQDlgDynamics();
    originalHeights[3]=pageDlgs[3]->size().height();

    pageDlgs[4]=new CQDlgPathPlanning();
    originalHeights[4]=pageDlgs[4]->size().height();

    pageDlgs[5]=new CQDlgMotionPlanning();
    originalHeights[5]=pageDlgs[5]->size().height();

    pageDlgs[6]=new CQDlgConstraintSolver();
    originalHeights[6]=pageDlgs[6]->size().height();

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
    bl->addWidget(pageDlgs[3]);
    pageDlgs[3]->setVisible(false);
    bl->addWidget(pageDlgs[4]);
    pageDlgs[4]->setVisible(false);
    bl->addWidget(pageDlgs[5]);
    pageDlgs[5]->setVisible(false);
    bl->addWidget(pageDlgs[6]);
    pageDlgs[6]->setVisible(false);

    QSize s(pageDlgs[currentPage]->size());
    s.setHeight(originalHeights[currentPage]+topBorderWidth);
    setFixedSize(s);
}

CQDlgCalcDialogContainer::~CQDlgCalcDialogContainer()
{
    delete ui;
}

void CQDlgCalcDialogContainer::dialogCallbackFunc(const SUIThreadCommand* cmdIn,SUIThreadCommand* cmdOut)
{
    pageDlgs[currentPage]->dialogCallbackFunc(cmdIn,cmdOut);
}

void CQDlgCalcDialogContainer::refresh()
{
    ui->qqCollision->setChecked(desiredPage==0);
    ui->qqDistance->setChecked(desiredPage==1);
    ui->qqIk->setChecked(desiredPage==2);
    ui->qqDynamics->setChecked(desiredPage==3);
    ui->qqPathPlanning->setChecked(desiredPage==4);
    ui->qqMotionPlanning->setChecked(desiredPage==5);
    ui->qqGcs->setChecked(desiredPage==6);

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

void CQDlgCalcDialogContainer::on_qqCollision_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        desiredPage=0;
        refresh();
    }
}

void CQDlgCalcDialogContainer::on_qqDistance_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        desiredPage=1;
        refresh();
    }
}

void CQDlgCalcDialogContainer::on_qqIk_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        desiredPage=2;
        refresh();
    }
}

void CQDlgCalcDialogContainer::on_qqDynamics_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        desiredPage=3;
        refresh();
    }
}

void CQDlgCalcDialogContainer::on_qqPathPlanning_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        desiredPage=4;
        refresh();
    }
}

void CQDlgCalcDialogContainer::on_qqMotionPlanning_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        desiredPage=5;
        refresh();
    }
}

void CQDlgCalcDialogContainer::on_qqGcs_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        desiredPage=6;
        refresh();
    }
}

