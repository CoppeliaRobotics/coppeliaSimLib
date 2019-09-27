
#include "vrepMainHeader.h"
#include "qdlgpathplanningaddnew.h"
#include "ui_qdlgpathplanningaddnew.h"
#include "gV.h"
#include "tt.h"
#include "app.h"
#include "v_repStrings.h"

CQDlgPathPlanningAddNew::CQDlgPathPlanningAddNew(QWidget *parent) :
    VDialog(parent,QT_MODAL_DLG_STYLE),
    ui(new Ui::CQDlgPathPlanningAddNew)
{
    ui->setupUi(this);
}

CQDlgPathPlanningAddNew::~CQDlgPathPlanningAddNew()
{
    delete ui;
}

void CQDlgPathPlanningAddNew::cancelEvent()
{
    defaultModalDialogEndRoutine(false);
}

void CQDlgPathPlanningAddNew::okEvent()
{
    _getValues();
    defaultModalDialogEndRoutine(true);
}

void CQDlgPathPlanningAddNew::refresh()
{
    ui->qqTaskType->clear();
    ui->qqTaskType->addItem(strTranslate(IDSN_HOLONOMIC),QVariant(sim_pathplanning_holonomic));
    ui->qqTaskType->addItem(strTranslate(IDSN_NON_HOLONOMIC),QVariant(sim_pathplanning_nonholonomic));
    ui->qqTaskType->setCurrentIndex(0);

    std::vector<std::string> names;
    std::vector<int> ids;

    ui->qqDummy->clear();
    names.clear();
    ids.clear();
    for (size_t i=0;i<App::ct->objCont->dummyList.size();i++)
    {
        CDummy* it2=App::ct->objCont->getDummy(App::ct->objCont->dummyList[i]);
        names.push_back(it2->getObjectName());
        ids.push_back(it2->getObjectHandle());
    }
    tt::orderStrings(names,ids);
    for (size_t i=0;i<names.size();i++)
        ui->qqDummy->addItem(names[i].c_str(),QVariant(ids[i]));
    ui->qqDummy->setCurrentIndex(0);

}

void CQDlgPathPlanningAddNew::_getValues()
{
    selectedTaskType=ui->qqTaskType->itemData(ui->qqTaskType->currentIndex()).toInt();
    selectedDummy=ui->qqDummy->itemData(ui->qqDummy->currentIndex()).toInt();
}

void CQDlgPathPlanningAddNew::on_qqOkCancel_accepted()
{
    _getValues();
    defaultModalDialogEndRoutine(true);
}

void CQDlgPathPlanningAddNew::on_qqOkCancel_rejected()
{
    defaultModalDialogEndRoutine(false);
}
