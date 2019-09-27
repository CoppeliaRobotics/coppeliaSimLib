
#include "vrepMainHeader.h"
#include "qdlgassembly.h"
#include "ui_qdlgassembly.h"
#include "v_rep_internal.h"
#include "tt.h"

#include "app.h"
#include "v_repStrings.h"
#include "vMessageBox.h"

CQDlgAssembly::CQDlgAssembly(QWidget *parent) :
    VDialog(parent,QT_MODAL_DLG_STYLE),
    ui(new Ui::CQDlgAssembly)
{
    obj=nullptr;
    ui->setupUi(this);
}

CQDlgAssembly::~CQDlgAssembly()
{
    delete ui;
}

void CQDlgAssembly::cancelEvent()
{
//  defaultModalDialogEndRoutine(false);
}

void CQDlgAssembly::okEvent()
{
//  defaultModalDialogEndRoutine(true);
}

void CQDlgAssembly::refresh()
{
    ui->qqSetLocalMatrix->setEnabled((obj->getAssemblyMatchValues(true).length()!=0)&&obj->getAssemblingLocalTransformationIsUsed());
    ui->qqChildMatchValue->setText(obj->getAssemblyMatchValues(true).c_str());
    ui->qqParentMatchValue->setText(obj->getAssemblyMatchValues(false).c_str());
    ui->qqHasMatrix->setChecked(obj->getAssemblingLocalTransformationIsUsed());
}

void CQDlgAssembly::on_qqChildMatchValue_editingFinished()
{
    if (!ui->qqChildMatchValue->isModified())
        return;
    obj->setAssemblyMatchValues(true,ui->qqChildMatchValue->text().toStdString().c_str());
    refresh();
}

void CQDlgAssembly::on_qqParentMatchValue_editingFinished()
{
    if (!ui->qqParentMatchValue->isModified())
        return;
    obj->setAssemblyMatchValues(false,ui->qqParentMatchValue->text().toStdString().c_str());
    refresh();
}

void CQDlgAssembly::on_qqSetLocalMatrix_clicked()
{
    unsigned short res=App::uiThread->messageBox_question(App::mainWindow,strTranslate(IDSN_ASSEMBLING_METHOD),strTranslate(IDSN_ASSEMBLING_METHOD_QUESTION),VMESSAGEBOX_YES_NO);
    if (res==VMESSAGEBOX_REPLY_YES)
    {
        C7Vector transform(obj->getLocalTransformationPart1());
        obj->setAssemblingLocalTransformation(transform);
        refresh();
    }
}

void CQDlgAssembly::on_qqClose_clicked(QAbstractButton *button)
{
    App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
    defaultModalDialogEndRoutine(true);
}

void CQDlgAssembly::on_qqHasMatrix_clicked()
{
    obj->setAssemblingLocalTransformationIsUsed(!obj->getAssemblingLocalTransformationIsUsed());
    refresh();
}
