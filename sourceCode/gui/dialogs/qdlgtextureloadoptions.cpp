
#include "vrepMainHeader.h"
#include "qdlgtextureloadoptions.h"
#include "ui_qdlgtextureloadoptions.h"
#include "gV.h"
#include "tt.h"
#include "global.h"
#include "app.h"

int CQDlgTextureLoadOptions::scaleTo=256;
bool CQDlgTextureLoadOptions::scale=true;

CQDlgTextureLoadOptions::CQDlgTextureLoadOptions(QWidget *parent) :
    VDialog(parent,QT_MODAL_DLG_STYLE),
    ui(new Ui::CQDlgTextureLoadOptions)
{
    ui->setupUi(this);
}

CQDlgTextureLoadOptions::~CQDlgTextureLoadOptions()
{
    delete ui;
}

void CQDlgTextureLoadOptions::cancelEvent()
{
//  defaultModalDialogEndRoutine(false);
}

void CQDlgTextureLoadOptions::okEvent()
{
//  defaultModalDialogEndRoutine(true);
}

void CQDlgTextureLoadOptions::refresh()
{
    ui->qqScale->setChecked(scale);
    ui->qq128->setChecked(scaleTo==128);
    ui->qq256->setChecked(scaleTo==256);
    ui->qq512->setChecked(scaleTo==512);
    ui->qq1024->setChecked(scaleTo==1024);
    ui->qq2048->setChecked(scaleTo==2048);
}

void CQDlgTextureLoadOptions::on_qqOk_accepted()
{
    defaultModalDialogEndRoutine(true);
}

void CQDlgTextureLoadOptions::on_qqScale_clicked()
{
    scale=!scale;
    refresh();
}

void CQDlgTextureLoadOptions::on_qq256_clicked()
{
    scale=true;
    scaleTo=256;
    refresh();
}

void CQDlgTextureLoadOptions::on_qq512_clicked()
{
    scale=true;
    scaleTo=512;
    refresh();
}

void CQDlgTextureLoadOptions::on_qq1024_clicked()
{
    scale=true;
    scaleTo=1024;
    refresh();
}

void CQDlgTextureLoadOptions::on_qq2048_clicked()
{
    scale=true;
    scaleTo=2048;
    refresh();
}

void CQDlgTextureLoadOptions::on_qq128_clicked()
{
    scale=true;
    scaleTo=128;
    refresh();
}
