#include <qdlgtriangleedit.h>
#include <ui_qdlgtriangleedit.h>
#include <app.h>
#include <simStrings.h>
#include <boost/lexical_cast.hpp>
#include <guiApp.h>

CQDlgTriangleEdit::CQDlgTriangleEdit(QWidget *parent) :
    CDlgEx(parent),
    ui(new Ui::CQDlgTriangleEdit)
{
    ui->setupUi(this);
}

CQDlgTriangleEdit::~CQDlgTriangleEdit()
{
    delete ui;
}

void CQDlgTriangleEdit::cancelEvent()
{
    // we override this cancel event. The container window should close, not this one!!
    GuiApp::mainWindow->dlgCont->close(SHAPE_EDITION_DLG);
}

void CQDlgTriangleEdit::refresh()
{
    std::string tmp=std::string(IDS_TOTAL_TRIANGLES)+": "+boost::lexical_cast<std::string>(GuiApp::mainWindow->editModeContainer->getEditModeBufferSize())+"/"+
        boost::lexical_cast<std::string>(GuiApp::mainWindow->editModeContainer->getShapeEditMode()->getEditionIndicesSize()/3);
    ui->qqInfo->setText(tmp.c_str());
    ui->qqMakeShape->setEnabled(GuiApp::mainWindow->editModeContainer->getEditModeBufferSize()!=0);
    ui->qqMakeCuboid->setEnabled(GuiApp::mainWindow->editModeContainer->getEditModeBufferSize()!=0);
    ui->qqMakeCylinder->setEnabled(GuiApp::mainWindow->editModeContainer->getEditModeBufferSize()!=0);
    ui->qqMakeSphere->setEnabled(GuiApp::mainWindow->editModeContainer->getEditModeBufferSize()!=0);
    ui->qqFlip->setEnabled(GuiApp::mainWindow->editModeContainer->getEditModeBufferSize()!=0);
    ui->qqSubdivide->setEnabled(true);
}

void CQDlgTriangleEdit::on_qqMakeShape_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        GuiApp::mainWindow->editModeContainer->getShapeEditMode()->makeShape();
    }
}

void CQDlgTriangleEdit::on_qqMakeCuboid_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        GuiApp::mainWindow->editModeContainer->getShapeEditMode()->makePrimitive(0);
    }
}

void CQDlgTriangleEdit::on_qqMakeCylinder_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        GuiApp::mainWindow->editModeContainer->getShapeEditMode()->makePrimitive(3);
    }
}

void CQDlgTriangleEdit::on_qqMakeSphere_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        GuiApp::mainWindow->editModeContainer->getShapeEditMode()->makePrimitive(1);
    }
}

void CQDlgTriangleEdit::on_qqFlip_clicked()
{
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        GuiApp::mainWindow->editModeContainer->getShapeEditMode()->flipTriangles();
        GuiApp::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgTriangleEdit::on_qqSubdivide_clicked()
{
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        GuiApp::mainWindow->editModeContainer->getShapeEditMode()->subdivideTriangles();
        GuiApp::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgTriangleEdit::on_qqClearSelection_clicked()
{
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        GuiApp::mainWindow->editModeContainer->deselectEditModeBuffer();
        GuiApp::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgTriangleEdit::on_qqInvertSelection_clicked()
{
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        for (int i=0;i<GuiApp::mainWindow->editModeContainer->getShapeEditMode()->getEditionIndicesSize()/3;i++)
            GuiApp::mainWindow->editModeContainer->getShapeEditMode()->xorAddItemToEditModeBuffer(i,true);
        GuiApp::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}
