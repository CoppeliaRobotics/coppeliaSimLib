#include "qdlgedgeedit.h"
#include "ui_qdlgedgeedit.h"
#include "app.h"
#include "gV.h"
#include "tt.h"
#include "simStrings.h"
#include <boost/lexical_cast.hpp>

CQDlgEdgeEdit::CQDlgEdgeEdit(QWidget *parent) :
    CDlgEx(parent),
    ui(new Ui::CQDlgEdgeEdit)
{
    ui->setupUi(this);
}

CQDlgEdgeEdit::~CQDlgEdgeEdit()
{
    delete ui;
}

void CQDlgEdgeEdit::cancelEvent()
{
    // we override this cancel event. The container window should close, not this one!!
    App::mainWindow->dlgCont->close(SHAPE_EDITION_DLG);
}

void CQDlgEdgeEdit::refresh()
{
    ui->qqShowHiddenEdges->setChecked(App::mainWindow->editModeContainer->getShapeEditMode()->getShowHiddenVerticeAndEdges());
    ui->qqAutoFollowing->setChecked(App::mainWindow->editModeContainer->getShapeEditMode()->getAutomaticallyFollowEdges());
    ui->qqEdgeAngle->setEnabled(App::mainWindow->editModeContainer->getShapeEditMode()->getAutomaticallyFollowEdges());
    ui->qqDirectionChangeAngle->setEnabled(App::mainWindow->editModeContainer->getShapeEditMode()->getAutomaticallyFollowEdges());
    ui->qqEdgeAngle->setText(tt::getAngleFString(false,App::mainWindow->editModeContainer->getShapeEditMode()->getEdgeMaxAngle(),1).c_str());
    ui->qqDirectionChangeAngle->setText(tt::getAngleFString(false,App::mainWindow->editModeContainer->getShapeEditMode()->getEdgeDirectionChangeMaxAngle(),1).c_str());

    std::string tmp=std::string(IDS_TOTAL_EDGES)+": "+boost::lexical_cast<std::string>(App::mainWindow->editModeContainer->getEditModeBufferSize())+"/"+
        boost::lexical_cast<std::string>(App::mainWindow->editModeContainer->getShapeEditMode()->getEditionEdgesSize()/2);
    ui->qqInfo->setText(tmp.c_str());

    ui->qqMakePath->setEnabled(App::mainWindow->editModeContainer->getEditModeBufferSize()!=0);
}

void CQDlgEdgeEdit::on_qqShowHiddenEdges_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::mainWindow->editModeContainer->getShapeEditMode()->setShowHiddenVerticeAndEdges(!App::mainWindow->editModeContainer->getShapeEditMode()->getShowHiddenVerticeAndEdges());
        refresh();
    }
}

void CQDlgEdgeEdit::on_qqAutoFollowing_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::mainWindow->editModeContainer->getShapeEditMode()->setAutomaticallyFollowEdges(!App::mainWindow->editModeContainer->getShapeEditMode()->getAutomaticallyFollowEdges());
        refresh();
    }
}

void CQDlgEdgeEdit::on_qqEdgeAngle_editingFinished()
{
    if (!ui->qqEdgeAngle->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        double newVal=ui->qqEdgeAngle->text().toFloat(&ok);
        if (ok)
        {
            newVal=tt::getLimitedFloat(1.0*degToRad,180.0*degToRad,newVal*gv::userToRad);
            App::mainWindow->editModeContainer->getShapeEditMode()->setEdgeMaxAngle(newVal);
        }
        refresh();
    }
}

void CQDlgEdgeEdit::on_qqDirectionChangeAngle_editingFinished()
{
    if (!ui->qqDirectionChangeAngle->isModified())
        return;
    IF_UI_EVENT_CAN_READ_DATA
    {
        bool ok;
        double newVal=ui->qqDirectionChangeAngle->text().toFloat(&ok);
        if (ok)
        {
            newVal=tt::getLimitedFloat(0.0*degToRad,180.0*degToRad,newVal*gv::userToRad);
            App::mainWindow->editModeContainer->getShapeEditMode()->setEdgeDirectionChangeMaxAngle(newVal);
        }
        refresh();
    }
}

void CQDlgEdgeEdit::on_qqMakePath_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::mainWindow->editModeContainer->getShapeEditMode()->makePath();
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgEdgeEdit::on_qqClearSelection_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::mainWindow->editModeContainer->getShapeEditMode()->deselectEditModeBuffer();
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgEdgeEdit::on_qqInvertSelection_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        for (int i=0;i<App::mainWindow->editModeContainer->getShapeEditMode()->getEditionEdgesSize()/2;i++)
            App::mainWindow->editModeContainer->getShapeEditMode()->xorAddItemToEditModeBuffer(i,true);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}
