#include "qdlgikconditional.h"
#include "ui_qdlgikconditional.h"
#include "app.h"
#include "tt.h"
#include "simStrings.h"

CQDlgIkConditional::CQDlgIkConditional(QWidget *parent) :
    VDialog(parent,QT_MODAL_DLG_STYLE),
    ui(new Ui::CQDlgIkConditional)
{
    ui->setupUi(this);
    firstHere=true;
    inMainRefreshRoutine=false;
}

CQDlgIkConditional::~CQDlgIkConditional()
{
    delete ui;
}

void CQDlgIkConditional::cancelEvent()
{
//  defaultModalDialogEndRoutine(false);
}

void CQDlgIkConditional::okEvent()
{
//  defaultModalDialogEndRoutine(true);
}

void CQDlgIkConditional::refresh()
{
    inMainRefreshRoutine=true;
    std::vector<std::string> names;
    std::vector<int> ids;
    if (firstHere)
    {
        firstHere=false;

        ui->qqIkGroupCombo->clear();
        ui->qqIkGroupCombo->addItem(IDSN_PERFORM_ALWAYS,QVariant(-1));
        for (size_t i=0;i<App::currentWorld->ikGroups->getObjectCount();i++)
        {
            CIkGroup_old* it=App::currentWorld->ikGroups->getObjectFromIndex(i);
            if (it->getObjectHandle()==ikGroup->getObjectHandle())
                break; // If no ik group comes before, we can't do anything!
            names.push_back(it->getObjectName());
            ids.push_back(it->getObjectHandle());
        }
        tt::orderStrings(names,ids);
        for (int i=0;i<int(names.size());i++)
            ui->qqIkGroupCombo->addItem(names[i].c_str(),QVariant(ids[i]));

        doOnFailOrSuccessOf=ikGroup->getDoOnFailOrSuccessOf();
        if (!ikGroup->getDoOnPerformed())
            ikResult=2;
        else
        {
            if (ikGroup->getDoOnFail())
                ikResult=0;
            else
                ikResult=1;
        }
        restoreIfPosNotReached=ikGroup->getRestoreIfPositionNotReached();
        restoreIfOrNotReached=ikGroup->getRestoreIfOrientationNotReached();
    }

    for (int i=0;i<ui->qqIkGroupCombo->count();i++)
    {
        if (ui->qqIkGroupCombo->itemData(i).toInt()==doOnFailOrSuccessOf)
        {
            ui->qqIkGroupCombo->setCurrentIndex(i);
            break;
        }
    }


    ui->qqIkResultCombo->clear();
    int sel=ui->qqIkGroupCombo->itemData(ui->qqIkGroupCombo->currentIndex()).toInt();
    ui->qqIkResultCombo->setEnabled(sel!=-1);

    ui->qqFailedAngular->setChecked(restoreIfOrNotReached);
    ui->qqFailedLinear->setChecked(restoreIfPosNotReached);

    if (doOnFailOrSuccessOf!=-1)
    {
        ui->qqIkResultCombo->addItem(IDSN_WAS_PERFORMED_AND_FAILED,QVariant(0));
        ui->qqIkResultCombo->addItem(IDSN_WAS_PERFORMED_AND_SUCCEEDED,QVariant(1));
        ui->qqIkResultCombo->addItem(IDSN_WAS_NOT_PERFORMED,QVariant(2));
        ui->qqIkResultCombo->setCurrentIndex(ikResult);
    }

    inMainRefreshRoutine=false;
}

void CQDlgIkConditional::on_qqClose_clicked(QAbstractButton *button)
{
    defaultModalDialogEndRoutine(true);
}

void CQDlgIkConditional::on_qqIkGroupCombo_currentIndexChanged(int index)
{
    if (!inMainRefreshRoutine)
    {
        doOnFailOrSuccessOf=ui->qqIkGroupCombo->itemData(ui->qqIkGroupCombo->currentIndex()).toInt();
//      ikGroup->setDoOnFailOrSuccessOf();
        refresh();
    }
}

void CQDlgIkConditional::on_qqIkResultCombo_currentIndexChanged(int index)
{
    if (!inMainRefreshRoutine)
    {
        int v=ui->qqIkResultCombo->itemData(ui->qqIkResultCombo->currentIndex()).toInt();
        ikResult=v;
//      ikGroup->setDoOnPerformed(v<2);
//      ikGroup->setDoOnFail(v==0);
        refresh();
    }
}

void CQDlgIkConditional::on_qqFailedLinear_clicked()
{
    restoreIfPosNotReached=!restoreIfPosNotReached;
//  ikGroup->setRestoreIfPositionNotReached(!ikGroup->getRestoreIfPositionNotReached());
    refresh();
}

void CQDlgIkConditional::on_qqFailedAngular_clicked()
{
    restoreIfOrNotReached=!restoreIfOrNotReached;
//  ikGroup->setRestoreIfOrientationNotReached(!ikGroup->getRestoreIfOrientationNotReached());
    refresh();
}

