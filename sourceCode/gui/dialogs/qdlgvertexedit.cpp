#include <qdlgvertexedit.h>
#include <ui_qdlgvertexedit.h>
#include <app.h>
#include <tt.h>
#include <simStrings.h>
#include <boost/lexical_cast.hpp>
#include <guiApp.h>

CQDlgVertexEdit::CQDlgVertexEdit(QWidget *parent) :
    CDlgEx(parent),
    ui(new Ui::CQDlgVertexEdit)
{
    ui->setupUi(this);
}

CQDlgVertexEdit::~CQDlgVertexEdit()
{
    delete ui;
}

void CQDlgVertexEdit::cancelEvent()
{
    // we override this cancel event. The container window should close, not this one!!
    GuiApp::mainWindow->dlgCont->close(SHAPE_EDITION_DLG);
}

void CQDlgVertexEdit::refresh()
{
    ui->qqShowHiddenVertices->setChecked(GuiApp::mainWindow->editModeContainer->getShapeEditMode()->getShowHiddenVerticeAndEdges());

    std::string tmp=std::string(IDS_TOTAL_VERTICES)+": "+boost::lexical_cast<std::string>(GuiApp::mainWindow->editModeContainer->getEditModeBufferSize())+"/"+
        boost::lexical_cast<std::string>(GuiApp::mainWindow->editModeContainer->getShapeEditMode()->getEditionVerticesSize()/3);
    ui->qqInfo->setText(tmp.c_str());

    ui->qqInsertTriangles->setEnabled(GuiApp::mainWindow->editModeContainer->getEditModeBufferSize()>2);
    ui->qqInsertFan->setEnabled(GuiApp::mainWindow->editModeContainer->getEditModeBufferSize()>3);
    ui->qqMakeDummies->setEnabled(GuiApp::mainWindow->editModeContainer->getEditModeBufferSize()!=0);
}

void CQDlgVertexEdit::on_qqShowHiddenVertices_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        GuiApp::mainWindow->editModeContainer->getShapeEditMode()->setShowHiddenVerticeAndEdges(!GuiApp::mainWindow->editModeContainer->getShapeEditMode()->getShowHiddenVerticeAndEdges());
        refresh();
    }
}

void CQDlgVertexEdit::on_qqInsertTriangles_clicked()
{
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        GuiApp::mainWindow->editModeContainer->getShapeEditMode()->insertTriangles();
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgVertexEdit::on_qqInsertFan_clicked()
{
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        GuiApp::mainWindow->editModeContainer->getShapeEditMode()->insertTriangleFan();
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgVertexEdit::on_qqMakeDummies_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        GuiApp::mainWindow->editModeContainer->getShapeEditMode()->makeDummies();
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgVertexEdit::on_qqClearSelection_clicked()
{
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        GuiApp::mainWindow->editModeContainer->deselectEditModeBuffer();
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgVertexEdit::on_qqInvertSelection_clicked()
{
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        for (int i=0;i<GuiApp::mainWindow->editModeContainer->getShapeEditMode()->getEditionVerticesSize()/3;i++)
            GuiApp::mainWindow->editModeContainer->getShapeEditMode()->xorAddItemToEditModeBuffer(i,true);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}
