#include <qdlgedgeedit.h>
#include <ui_qdlgedgeedit.h>
#include <app.h>
#include <tt.h>
#include <utils.h>
#include <simStrings.h>
#include <boost/lexical_cast.hpp>
#include <guiApp.h>

CQDlgEdgeEdit::CQDlgEdgeEdit(QWidget *parent) : CDlgEx(parent), ui(new Ui::CQDlgEdgeEdit)
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
    GuiApp::mainWindow->dlgCont->close(SHAPE_EDITION_DLG);
}

void CQDlgEdgeEdit::refresh()
{
    ui->qqShowHiddenEdges->setChecked(
        GuiApp::mainWindow->editModeContainer->getShapeEditMode()->getShowHiddenVerticeAndEdges());
    ui->qqAutoFollowing->setChecked(
        GuiApp::mainWindow->editModeContainer->getShapeEditMode()->getAutomaticallyFollowEdges());
    ui->qqEdgeAngle->setEnabled(
        GuiApp::mainWindow->editModeContainer->getShapeEditMode()->getAutomaticallyFollowEdges());
    ui->qqDirectionChangeAngle->setEnabled(
        GuiApp::mainWindow->editModeContainer->getShapeEditMode()->getAutomaticallyFollowEdges());
    ui->qqEdgeAngle->setText(
        utils::getAngleString(false, GuiApp::mainWindow->editModeContainer->getShapeEditMode()->getEdgeMaxAngle())
            .c_str());
    ui->qqDirectionChangeAngle->setText(
        utils::getAngleString(
            false, GuiApp::mainWindow->editModeContainer->getShapeEditMode()->getEdgeDirectionChangeMaxAngle())
            .c_str());

    std::string tmp = std::string(IDS_TOTAL_EDGES) + ": " +
                      boost::lexical_cast<std::string>(GuiApp::mainWindow->editModeContainer->getEditModeBufferSize()) +
                      "/" +
                      boost::lexical_cast<std::string>(
                          GuiApp::mainWindow->editModeContainer->getShapeEditMode()->getEditionEdgesSize() / 2);
    ui->qqInfo->setText(tmp.c_str());

    ui->qqMakePath->setEnabled(GuiApp::mainWindow->editModeContainer->getEditModeBufferSize() != 0);
}

void CQDlgEdgeEdit::on_qqShowHiddenEdges_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        GuiApp::mainWindow->editModeContainer->getShapeEditMode()->setShowHiddenVerticeAndEdges(
            !GuiApp::mainWindow->editModeContainer->getShapeEditMode()->getShowHiddenVerticeAndEdges());
        refresh();
    }
}

void CQDlgEdgeEdit::on_qqAutoFollowing_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        GuiApp::mainWindow->editModeContainer->getShapeEditMode()->setAutomaticallyFollowEdges(
            !GuiApp::mainWindow->editModeContainer->getShapeEditMode()->getAutomaticallyFollowEdges());
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
        double newVal = GuiApp::getEvalDouble(ui->qqEdgeAngle->text().toStdString().c_str(), &ok);
        if (ok)
        {
            newVal = tt::getLimitedFloat(1.0 * degToRad, 180.0 * degToRad, newVal * degToRad);
            GuiApp::mainWindow->editModeContainer->getShapeEditMode()->setEdgeMaxAngle(newVal);
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
        double newVal = GuiApp::getEvalDouble(ui->qqDirectionChangeAngle->text().toStdString().c_str(), &ok);
        if (ok)
        {
            newVal = tt::getLimitedFloat(0.0 * degToRad, 180.0 * degToRad, newVal * degToRad);
            GuiApp::mainWindow->editModeContainer->getShapeEditMode()->setEdgeDirectionChangeMaxAngle(newVal);
        }
        refresh();
    }
}

void CQDlgEdgeEdit::on_qqMakePath_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        GuiApp::mainWindow->editModeContainer->getShapeEditMode()->makePath();
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgEdgeEdit::on_qqClearSelection_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        GuiApp::mainWindow->editModeContainer->getShapeEditMode()->deselectEditModeBuffer();
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgEdgeEdit::on_qqInvertSelection_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        for (int i = 0; i < GuiApp::mainWindow->editModeContainer->getShapeEditMode()->getEditionEdgesSize() / 2; i++)
            GuiApp::mainWindow->editModeContainer->getShapeEditMode()->xorAddItemToEditModeBuffer(i, true);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}
