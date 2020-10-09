#include "qdlginsertscript.h"
#include "ui_qdlginsertscript.h"
#include "simInternal.h"
#include "simStrings.h"
#include "app.h"

CQDlgInsertScript::CQDlgInsertScript(QWidget *parent) :
    VDialog(parent,QT_MODAL_DLG_STYLE),
    ui(new Ui::CQDlgInsertScript)
{
    ui->setupUi(this);
}

CQDlgInsertScript::~CQDlgInsertScript()
{
    delete ui;
}

void CQDlgInsertScript::cancelEvent()
{
    defaultModalDialogEndRoutine(false);
}

void CQDlgInsertScript::okEvent()
{
    scriptType=ui->qqCombo->itemData(ui->qqCombo->currentIndex()).toInt();
    defaultModalDialogEndRoutine(true);
}

void CQDlgInsertScript::initialize()
{
    ui->qqCombo->clear();
    ui->qqCombo->addItem(IDS_MAIN_SCRIPT,0);//QVariant(sim_scripttype_mainscript));
    ui->qqCombo->addItem(IDS_CHILD_SCRIPT_UNTHREADED,1);//QVariant(sim_scripttype_childscript));
    ui->qqCombo->addItem(IDS_CHILD_SCRIPT_THREADED,2);//QVariant(sim_scripttype_childscript|sim_scripttype_threaded_old));
    if (App::userSettings->makeOldThreadedScriptsAvailable)
        ui->qqCombo->addItem("Threaded (deprecated, backward compatibility version)",3);
    ui->qqCombo->addItem(IDS_CUSTOMIZATION_SCRIPT,4);//QVariant(sim_scripttype_customizationscript));
}

void CQDlgInsertScript::on_qqOkCancel_accepted()
{
    scriptType=ui->qqCombo->itemData(ui->qqCombo->currentIndex()).toInt();
    defaultModalDialogEndRoutine(true);
}

void CQDlgInsertScript::on_qqOkCancel_rejected()
{
    defaultModalDialogEndRoutine(false);
}
