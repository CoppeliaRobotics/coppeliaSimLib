
#include "qdlgopenglsettings.h"
#include "ui_qdlgopenglsettings.h"
#include "app.h"
#include "tt.h"

CQDlgOpenGlSettings::CQDlgOpenGlSettings(QWidget *parent) :
    VDialog(parent,QT_MODAL_DLG_STYLE),
    ui(new Ui::CQDlgOpenGlSettings)
{
    ui->setupUi(this);
}

CQDlgOpenGlSettings::~CQDlgOpenGlSettings()
{
    delete ui;
}

void CQDlgOpenGlSettings::cancelEvent()
{
//  defaultModalDialogEndRoutine(false);
}

void CQDlgOpenGlSettings::okEvent()
{
//  defaultModalDialogEndRoutine(true);
}

void CQDlgOpenGlSettings::refresh()
{
    ui->qqContextDefault->setChecked(offscreenContextType==-1);
    ui->qqContextOffscreen->setChecked(offscreenContextType==0);
    ui->qqContextVisibleWindow->setChecked(offscreenContextType==1);
    ui->qqContextInvisibleWindow->setChecked(offscreenContextType==2);

    ui->qqFboDefault->setChecked(fboType==-1);
    ui->qqFboNonQt->setChecked(fboType==0);
    ui->qqFboQt->setChecked(fboType==1);

    ui->qqVboDefault->setChecked(vboOperation==-1);
    ui->qqVboDisabled->setChecked(vboOperation==0);
    ui->qqVboEnabled->setChecked(vboOperation==1);

//    ui->qqGuiRenderingOnly->setChecked((exclusiveGuiRendering==-1)||(exclusiveGuiRendering==1));
//    ui->qqCompatibilityTweak1->setChecked(compatibilityTweak1);
//    ui->qqUsingGlFinish->setChecked(glFinish_normal);
//    ui->qqUsingGlFinishVisionSensors->setChecked(glFinish_visionSensors);
    ui->qqIdleFps->setText(tt::getIString(false,idleFps).c_str());
//    ui->qqForceExt->setChecked(forceExt);

//    ui->qqMajorOpenGlVersion->setText(tt::getIString(false,glVersionMajor).c_str());
//    ui->qqMinorOpenGlVersion->setText(tt::getIString(false,glVersionMinor).c_str());
}

void CQDlgOpenGlSettings::on_qqOk_accepted()
{
    defaultModalDialogEndRoutine(true);
}

void CQDlgOpenGlSettings::on_qqContextDefault_clicked()
{
    offscreenContextType=-1;
    refresh();
}

void CQDlgOpenGlSettings::on_qqContextOffscreen_clicked()
{
    offscreenContextType=0;
    refresh();
}

void CQDlgOpenGlSettings::on_qqContextVisibleWindow_clicked()
{
    offscreenContextType=1;
    refresh();
}

void CQDlgOpenGlSettings::on_qqContextInvisibleWindow_clicked()
{
    offscreenContextType=2;
    refresh();
}

void CQDlgOpenGlSettings::on_qqFboDefault_clicked()
{
    fboType=-1;
    refresh();
}

void CQDlgOpenGlSettings::on_qqFboNonQt_clicked()
{
    fboType=0;
    refresh();
}

void CQDlgOpenGlSettings::on_qqFboQt_clicked()
{
    fboType=1;
    refresh();
}
/*
void CQDlgOpenGlSettings::on_qqGuiRenderingOnly_clicked()
{
    if ((exclusiveGuiRendering==-1)||(exclusiveGuiRendering==1))
        exclusiveGuiRendering=0;
    else
        exclusiveGuiRendering=1;
    refresh();
}

void CQDlgOpenGlSettings::on_qqCompatibilityTweak1_clicked()
{
    compatibilityTweak1=!compatibilityTweak1;
    refresh();
}

void CQDlgOpenGlSettings::on_qqUsingGlFinish_clicked()
{
    glFinish_normal=!glFinish_normal;
    refresh();
}

void CQDlgOpenGlSettings::on_qqUsingGlFinishVisionSensors_clicked()
{
    glFinish_visionSensors=!glFinish_visionSensors;
    refresh();
}

void CQDlgOpenGlSettings::on_qqMajorOpenGlVersion_editingFinished()
{
    if (!ui->qqMajorOpenGlVersion->isModified())
        return;
    int newVal;
    bool ok;
    newVal=ui->qqMajorOpenGlVersion->text().toInt(&ok);
    if (ok)
    {
        if (newVal!=-1)
            tt::limitValue(1,4,newVal);
        glVersionMajor=newVal;
    }
    refresh();
}

void CQDlgOpenGlSettings::on_qqMinorOpenGlVersion_editingFinished()
{
    if (!ui->qqMinorOpenGlVersion->isModified())
        return;
    int newVal;
    bool ok;
    newVal=ui->qqMinorOpenGlVersion->text().toInt(&ok);
    if (ok)
    {
        if (newVal!=-1)
            tt::limitValue(0,5,newVal);
        glVersionMinor=newVal;
    }
    refresh();
}

void CQDlgOpenGlSettings::on_qqForceExt_clicked()
{
    forceExt=!forceExt;
    refresh();
}

*/
void CQDlgOpenGlSettings::on_qqIdleFps_editingFinished()
{
    if (!ui->qqIdleFps->isModified())
        return;
    int newVal;
    bool ok;
    newVal=ui->qqIdleFps->text().toInt(&ok);
    if (ok)
    {
        tt::limitValue(2,25,newVal);
        idleFps=newVal;
    }
    refresh();
}

void CQDlgOpenGlSettings::on_qqVboDefault_clicked()
{
    vboOperation=-1;
    refresh();
}

void CQDlgOpenGlSettings::on_qqVboDisabled_clicked()
{
    vboOperation=0;
    refresh();
}

void CQDlgOpenGlSettings::on_qqVboEnabled_clicked()
{
    vboOperation=1;
    refresh();
}
