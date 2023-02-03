#include <qdlgtriangleedit.h>
#include <ui_qdlgtriangleedit.h>
#include <app.h>
#include <simStrings.h>
#include <boost/lexical_cast.hpp>

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
    App::mainWindow->dlgCont->close(SHAPE_EDITION_DLG);
}

void CQDlgTriangleEdit::refresh()
{
    std::string tmp=std::string(IDS_TOTAL_TRIANGLES)+": "+boost::lexical_cast<std::string>(App::mainWindow->editModeContainer->getEditModeBufferSize())+"/"+
        boost::lexical_cast<std::string>(App::mainWindow->editModeContainer->getShapeEditMode()->getEditionIndicesSize()/3);
    ui->qqInfo->setText(tmp.c_str());
    ui->qqMakeShape->setEnabled(App::mainWindow->editModeContainer->getEditModeBufferSize()!=0);
    ui->qqMakeCuboid->setEnabled(App::mainWindow->editModeContainer->getEditModeBufferSize()!=0);
    ui->qqMakeCylinder->setEnabled(App::mainWindow->editModeContainer->getEditModeBufferSize()!=0);
    ui->qqMakeSphere->setEnabled(App::mainWindow->editModeContainer->getEditModeBufferSize()!=0);
    ui->qqFlip->setEnabled(App::mainWindow->editModeContainer->getEditModeBufferSize()!=0);
    ui->qqSubdivide->setEnabled(true);
}

void CQDlgTriangleEdit::on_qqMakeShape_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::mainWindow->editModeContainer->getShapeEditMode()->makeShape();
    }
}

void CQDlgTriangleEdit::on_qqMakeCuboid_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::mainWindow->editModeContainer->getShapeEditMode()->makePrimitive(0);
    }
}

void CQDlgTriangleEdit::on_qqMakeCylinder_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::mainWindow->editModeContainer->getShapeEditMode()->makePrimitive(3);
    }
}

void CQDlgTriangleEdit::on_qqMakeSphere_clicked()
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        App::mainWindow->editModeContainer->getShapeEditMode()->makePrimitive(1);
    }
}

void CQDlgTriangleEdit::on_qqFlip_clicked()
{
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        App::mainWindow->editModeContainer->getShapeEditMode()->flipTriangles();
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgTriangleEdit::on_qqSubdivide_clicked()
{
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        App::mainWindow->editModeContainer->getShapeEditMode()->subdivideTriangles();
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgTriangleEdit::on_qqClearSelection_clicked()
{
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        App::mainWindow->editModeContainer->deselectEditModeBuffer();
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}

void CQDlgTriangleEdit::on_qqInvertSelection_clicked()
{
    IF_UI_EVENT_CAN_WRITE_DATA
    {
        for (int i=0;i<App::mainWindow->editModeContainer->getShapeEditMode()->getEditionIndicesSize()/3;i++)
            App::mainWindow->editModeContainer->getShapeEditMode()->xorAddItemToEditModeBuffer(i,true);
        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
    }
}
