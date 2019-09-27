
#include "vrepMainHeader.h"
#include "qdlgtranslationrotationcont.h"
#include "ui_qdlgtranslationrotationcont.h"
#include "app.h"
#include "toolBarCommand.h"
#include "qdlgtranslation.h"
#include "qdlgrotation.h"

CQDlgTranslationRotationCont::CQDlgTranslationRotationCont(QWidget *parent) :
    CDlgEx(parent),
    ui(new Ui::CQDlgTranslationRotationCont)
{
    _dlgType=TRANSLATION_ROTATION_DLG;
    ui->setupUi(this);
    pageDlgs[0]=new CQDlgTranslation();
    originalHeights[0]=pageDlgs[0]->size().height();
    pageDlgs[1]=new CQDlgRotation();
    originalHeights[1]=pageDlgs[1]->size().height();

    currentPage=0;
    desiredPage=0;
    bl=new QVBoxLayout();
    bl->setContentsMargins(0,0,0,0);
    setLayout(bl);
    bl->addWidget(pageDlgs[0]);
    bl->addWidget(pageDlgs[1]);
    pageDlgs[1]->setVisible(false);

    QSize s(pageDlgs[currentPage]->size());
    s.setHeight(originalHeights[currentPage]);
    setFixedSize(s);
}

CQDlgTranslationRotationCont::~CQDlgTranslationRotationCont()
{
    delete ui;
}

void CQDlgTranslationRotationCont::cancelEvent()
{
    SSimulationThreadCommand cmd;
    cmd.cmdId=SET_MOUSEMODE_GUITRIGGEREDCMD;
    cmd.intParams.push_back((App::getMouseMode()&0xff00)|sim_navigation_camerashift);
    App::appendSimulationThreadCommand(cmd);
//    App::setMouseMode((App::getMouseMode()&0xff00)|sim_navigation_camerashift);
}

void CQDlgTranslationRotationCont::okEvent()
{
}

void CQDlgTranslationRotationCont::refresh()
{
    if ((App::getMouseMode()&0x0f)==sim_navigation_objectshift)
        desiredPage=0;
    if ((App::getMouseMode()&0x0f)==sim_navigation_objectrotate)
        desiredPage=1;

    if (desiredPage==0)
        setWindowTitle("Object/Item Translation/Position");
    else
        setWindowTitle("Object/Item Rotation/Orientation");

    if (desiredPage!=currentPage)
    {
        pageDlgs[currentPage]->setVisible(false);
        currentPage=desiredPage;
        pageDlgs[currentPage]->setVisible(true);
        QSize s(pageDlgs[currentPage]->size());
        s.setHeight(originalHeights[currentPage]);
        setFixedSize(s);
    }
    pageDlgs[currentPage]->refresh();
}
