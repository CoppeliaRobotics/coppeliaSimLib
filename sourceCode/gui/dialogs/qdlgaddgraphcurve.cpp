#include "qdlgaddgraphcurve.h"
#include "ui_qdlgaddgraphcurve.h"
#include "gV.h"
#include "tt.h"
#include "graphingRoutines.h"
#include "app.h"


CQDlgAddGraphCurve::CQDlgAddGraphCurve(QWidget *parent) :
    VDialog(parent,QT_MODAL_DLG_STYLE),
    ui(new Ui::CQDlgAddGraphCurve)
{
    ui->setupUi(this);
}

CQDlgAddGraphCurve::~CQDlgAddGraphCurve()
{
    delete ui;
}

void CQDlgAddGraphCurve::cancelEvent()
{
    defaultModalDialogEndRoutine(false);
}

void CQDlgAddGraphCurve::okEvent()
{
//  defaultModalDialogEndRoutine(true);
}

void CQDlgAddGraphCurve::refresh()
{
    CGraph* it=App::ct->objCont->getLastSelection_graph();
    ui->qqZValue->setVisible(!xyGraph);
    ui->qqComboZ->setVisible(!xyGraph);

    ui->qqComboX->clear();
    ui->qqComboY->clear();
    ui->qqComboZ->clear();
    ui->qqComboX->addItem("0.0",QVariant(-1));
    ui->qqComboY->addItem("0.0",QVariant(-1));
    ui->qqComboZ->addItem("0.0",QVariant(-1));
    for (size_t i=0;i<it->daten.size();i++)
    {
        int theID=it->daten[i]->getIdentifier();
        ui->qqComboX->addItem(it->daten[i]->getName().c_str(),QVariant(theID));
        ui->qqComboY->addItem(it->daten[i]->getName().c_str(),QVariant(theID));
        if (!xyGraph)
            ui->qqComboZ->addItem(it->daten[i]->getName().c_str(),QVariant(theID));
    }
}

void CQDlgAddGraphCurve::on_qqOkCancel_accepted()
{
    dataIDX=ui->qqComboX->itemData(ui->qqComboX->currentIndex()).toInt();
    dataIDY=ui->qqComboY->itemData(ui->qqComboY->currentIndex()).toInt();
    dataIDZ=ui->qqComboZ->itemData(ui->qqComboZ->currentIndex()).toInt();
    defaultModalDialogEndRoutine(true);
}

void CQDlgAddGraphCurve::on_qqOkCancel_rejected()
{
    defaultModalDialogEndRoutine(false);
}
