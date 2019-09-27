
#include "vrepMainHeader.h"
#include "qdlgmotionplanningjoints.h"
#include "ui_qdlgmotionplanningjoints.h"
#include "gV.h"
#include "tt.h"
#include "app.h"
#include "v_repStringTable.h"

CQDlgMotionPlanningJoints::CQDlgMotionPlanningJoints(QWidget *parent) :
    VDialog(parent,QT_MODAL_DLG_STYLE),
    ui(new Ui::CQDlgMotionPlanningJoints)
{
    ui->setupUi(this);
    inMainRefreshRoutine=false;
    noTableSelectionAllowed=false;
    first=true;
}

CQDlgMotionPlanningJoints::~CQDlgMotionPlanningJoints()
{
    delete ui;
}

void CQDlgMotionPlanningJoints::cancelEvent()
{
//  defaultModalDialogEndRoutine(false);
}

void CQDlgMotionPlanningJoints::okEvent()
{
//  defaultModalDialogEndRoutine(true);
}

void CQDlgMotionPlanningJoints::refresh()
{
    inMainRefreshRoutine=true;

    if (first)
        updateObjectsInTable();

    int retV=1;
    for (size_t i=0;i<jointSubdiv.size();i++)
        retV*=jointSubdiv[i];

    ui->qqNodeCnt->setText(tt::FNb(0,retV,false).c_str());

    inMainRefreshRoutine=false;
    first=false;
}


void CQDlgMotionPlanningJoints::updateObjectsInTable()
{
    noTableSelectionAllowed=true;
    ui->qqTable->clear();
    ui->qqTable->insertColumn(0);
    ui->qqTable->insertColumn(1);
    ui->qqTable->insertColumn(2);
    ui->qqTable->setColumnWidth(0,140);
    ui->qqTable->setColumnWidth(1,100);
    ui->qqTable->setColumnWidth(2,75);
    QStringList labels;
    labels << IDSN_JOINT_NAME << IDSN_RANGE_SUBDIVISIONS << IDSN_METRIC_WEIGHT;
    ui->qqTable->setHorizontalHeaderLabels(labels);

    for (size_t i=0;i<jointNames.size();i++)
    {
        ui->qqTable->insertRow((int)i);
        QTableWidgetItem* item=new QTableWidgetItem();
        ui->qqTable->setItem((int)i,0,item);

        item->setFlags(Qt::ItemIsEnabled);
        item->setText(jointNames[i].c_str());

        item=new QTableWidgetItem();
        ui->qqTable->setItem((int)i,1,item);
        item->setFlags(Qt::ItemIsEnabled|Qt::ItemIsEditable);
        item->setText(tt::FNb(0,jointSubdiv[i]-1,false).c_str());

        item=new QTableWidgetItem();
        ui->qqTable->setItem((int)i,2,item);
        item->setFlags(Qt::ItemIsEnabled|Qt::ItemIsEditable);
        item->setText(tt::FNb(0,jointMetricWeight[i],2,false).c_str());
    }
    noTableSelectionAllowed=false;
}


void CQDlgMotionPlanningJoints::on_qqClose_clicked(QAbstractButton *button)
{
    defaultModalDialogEndRoutine(true);
}


void CQDlgMotionPlanningJoints::on_qqTable_itemChanged(QTableWidgetItem *item)
{
    if (inMainRefreshRoutine)
        return;
    if (item!=nullptr)
    {
        int jointIndex=item->row();
        if (item->column()==1)
        { // subdivisions changed
            bool ok;
            int newVal=item->text().toInt(&ok);
            if (ok)
            {
                newVal=tt::getLimitedInt(2,49,newVal);
                jointSubdiv[jointIndex]=newVal+1;
            }
            item->setText(tt::FNb(0,jointSubdiv[jointIndex]-1,false).c_str());
        }
        if (item->column()==2)
        { // weight changed
            bool ok;
            float newVal=item->text().toFloat(&ok);
            if (ok)
            {
                newVal=tt::getLimitedFloat(0.0f,1000.0f,newVal);
                jointMetricWeight[jointIndex]=newVal;
            }
            item->setText(tt::FNb(0,jointMetricWeight[jointIndex],2,false).c_str());
        }
        refresh();
    }
}
