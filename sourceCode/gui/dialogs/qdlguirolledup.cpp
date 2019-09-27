
#include "vrepMainHeader.h"
#include "qdlguirolledup.h"
#include "ui_qdlguirolledup.h"
#include "tt.h"
#include "app.h"

CQDlgUiRolledUp::CQDlgUiRolledUp(QWidget *parent) :
    VDialog(parent,QT_MODAL_DLG_STYLE),
    ui(new Ui::CQDlgUiRolledUp)
{
    ui->setupUi(this);
}

CQDlgUiRolledUp::~CQDlgUiRolledUp()
{
    delete ui;
}

void CQDlgUiRolledUp::cancelEvent()
{
//  defaultModalDialogEndRoutine(false);
}

void CQDlgUiRolledUp::okEvent()
{
//  defaultModalDialogEndRoutine(true);
}
#include <QGroupBox>
void CQDlgUiRolledUp::refresh()
{ // the dialog is modal
    CButtonBlock* itBlock=App::ct->buttonBlockContainer->getBlockWithID(itBlockID);
    if (itBlock!=nullptr)
    {
        VPoint s;
        itBlock->getRollupMin(s);
        ui->qqFirstH->setText(tt::getIString(false,s.x+1).c_str());
        ui->qqFirstV->setText(tt::getIString(false,s.y+1).c_str());
        itBlock->getRollupMax(s);
        ui->qqLastH->setText(tt::getIString(false,s.x+1).c_str());
        ui->qqLastV->setText(tt::getIString(false,s.y+1).c_str());
    }
}

void CQDlgUiRolledUp::on_qqFirstH_editingFinished()
{
    if (!ui->qqFirstH->isModified())
        return;
    CButtonBlock* itBlock=App::ct->buttonBlockContainer->getBlockWithID(itBlockID);
    if (itBlock!=nullptr)
    {
        bool ok;
        int newVal=ui->qqFirstH->text().toInt(&ok);
        if (ok)
        {
            VPoint sizeMin;
            itBlock->getRollupMin(sizeMin);
            sizeMin.x=newVal-1;
            itBlock->setRollupMin(sizeMin); // the dialog is modal. We modify the resources here AND in the sim thread
            VPoint sizeMax;
            itBlock->getRollupMax(sizeMax);
            SSimulationThreadCommand cmd;
            cmd.cmdId=SET_ROLLEDUPSIZES_ROLLEDUPGUITRIGGEREDCMD;
            cmd.intParams.push_back(itBlockID);
            cmd.intParams.push_back(sizeMin.x);
            cmd.intParams.push_back(sizeMin.y);
            cmd.intParams.push_back(sizeMax.x);
            cmd.intParams.push_back(sizeMax.y);
            App::appendSimulationThreadCommand(cmd);
        }
    }
    refresh();
}

void CQDlgUiRolledUp::on_qqLastH_editingFinished()
{
    if (!ui->qqLastH->isModified())
        return;
    CButtonBlock* itBlock=App::ct->buttonBlockContainer->getBlockWithID(itBlockID);
    if (itBlock!=nullptr)
    {
        bool ok;
        int newVal=ui->qqLastH->text().toInt(&ok);
        if (ok)
        {
            VPoint sizeMax;
            itBlock->getRollupMax(sizeMax);
            sizeMax.x=newVal-1;
            itBlock->setRollupMax(sizeMax); // the dialog is modal. We modify the resources here AND in the sim thread
            VPoint sizeMin;
            itBlock->getRollupMin(sizeMin);
            SSimulationThreadCommand cmd;
            cmd.cmdId=SET_ROLLEDUPSIZES_ROLLEDUPGUITRIGGEREDCMD;
            cmd.intParams.push_back(itBlockID);
            cmd.intParams.push_back(sizeMin.x);
            cmd.intParams.push_back(sizeMin.y);
            cmd.intParams.push_back(sizeMax.x);
            cmd.intParams.push_back(sizeMax.y);
            App::appendSimulationThreadCommand(cmd);
        }
    }
    refresh();
}

void CQDlgUiRolledUp::on_qqFirstV_editingFinished()
{
    if (!ui->qqFirstV->isModified())
        return;
    CButtonBlock* itBlock=App::ct->buttonBlockContainer->getBlockWithID(itBlockID);
    if (itBlock!=nullptr)
    {
        bool ok;
        int newVal=ui->qqFirstV->text().toInt(&ok);
        if (ok)
        {
            VPoint sizeMin;
            itBlock->getRollupMin(sizeMin);
            sizeMin.y=newVal-1;
            itBlock->setRollupMin(sizeMin); // the dialog is modal. We modify the resources here AND in the sim thread
            VPoint sizeMax;
            itBlock->getRollupMax(sizeMax);
            SSimulationThreadCommand cmd;
            cmd.cmdId=SET_ROLLEDUPSIZES_ROLLEDUPGUITRIGGEREDCMD;
            cmd.intParams.push_back(itBlockID);
            cmd.intParams.push_back(sizeMin.x);
            cmd.intParams.push_back(sizeMin.y);
            cmd.intParams.push_back(sizeMax.x);
            cmd.intParams.push_back(sizeMax.y);
            App::appendSimulationThreadCommand(cmd);
        }
    }
    refresh();
}

void CQDlgUiRolledUp::on_qqLastV_editingFinished()
{
    if (!ui->qqLastV->isModified())
        return;
    CButtonBlock* itBlock=App::ct->buttonBlockContainer->getBlockWithID(itBlockID);
    if (itBlock!=nullptr)
    {
        bool ok;
        int newVal=ui->qqLastV->text().toInt(&ok);
        if (ok)
        {
            VPoint sizeMax;
            itBlock->getRollupMax(sizeMax);
            sizeMax.y=newVal-1;
            itBlock->setRollupMax(sizeMax); // the dialog is modal. We modify the resources here AND in the sim thread
            VPoint sizeMin;
            itBlock->getRollupMin(sizeMin);
            SSimulationThreadCommand cmd;
            cmd.cmdId=SET_ROLLEDUPSIZES_ROLLEDUPGUITRIGGEREDCMD;
            cmd.intParams.push_back(itBlockID);
            cmd.intParams.push_back(sizeMin.x);
            cmd.intParams.push_back(sizeMin.y);
            cmd.intParams.push_back(sizeMax.x);
            cmd.intParams.push_back(sizeMax.y);
            App::appendSimulationThreadCommand(cmd);
        }
    }
    refresh();
}

void CQDlgUiRolledUp::on_qqClose_clicked(QAbstractButton *button)
{
    defaultModalDialogEndRoutine(true);
}
