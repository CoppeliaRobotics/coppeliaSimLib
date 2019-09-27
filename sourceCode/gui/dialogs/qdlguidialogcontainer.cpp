
#include "vrepMainHeader.h"
#include "qdlguidialogcontainer.h"
#include "ui_qdlguidialogcontainer.h"
#include "app.h"

#include "qdlgui.h"
#include "qdlguibuttons.h"

#define TOP_BORDER_WIDTH 0

CQDlgUiDialogContainer::CQDlgUiDialogContainer(QWidget *parent) :
    CDlgEx(parent),
    ui(new Ui::CQDlgUiDialogContainer)
{
    ui->setupUi(this);
    pageDlgs[0]=new CQDlgUi();
    originalHeights[0]=pageDlgs[0]->size().height();

    pageDlgs[1]=new CQDlgUiButtons();
    originalHeights[1]=pageDlgs[1]->size().height();

    currentPage=0;
    bl=new QVBoxLayout();
    bl->setContentsMargins(0,TOP_BORDER_WIDTH,0,0);
    setLayout(bl);
    bl->addWidget(pageDlgs[0]);
    bl->addWidget(pageDlgs[1]);
    pageDlgs[1]->setVisible(false);

    QSize s(pageDlgs[currentPage]->size());
    s.setHeight(originalHeights[currentPage]+TOP_BORDER_WIDTH);
    setFixedSize(s);
/*
#ifndef WIN_VREP
    // Since Qt5, we have problems on Linux (resizing-->dlg shifts in position) and Mac (resising-->ugly glitch)
    // In that case we keep constant-size dialogs.
    if (QT_VERSION>=0x050000)
    {
        int dlgMaxHeight=0;
        for (int i=0;i<2;i++)
        {
            int si=pageDlgs[i]->size().height();
            if (si>dlgMaxHeight)
                dlgMaxHeight=si;
        }
        s.setHeight(dlgMaxHeight+TOP_BORDER_WIDTH);
        setFixedSize(s);
    }
#endif
*/
}

CQDlgUiDialogContainer::~CQDlgUiDialogContainer()
{
    delete ui;
}

void CQDlgUiDialogContainer::cancelEvent()
{
    App::mainWindow->editModeContainer->processCommand(ANY_EDIT_MODE_FINISH_AND_APPLY_CHANGES_EMCMD,nullptr);
//  defaultModalDialogEndRoutine(false);
}

void CQDlgUiDialogContainer::okEvent()
{
//  defaultModalDialogEndRoutine(true);
}

void CQDlgUiDialogContainer::refresh()
{
    int uiID=App::ct->buttonBlockContainer->getBlockInEdition();
    CButtonBlock* it=App::ct->buttonBlockContainer->getBlockWithID(uiID);
    int desiredPage=currentPage;
    if ( (it!=nullptr)&&(App::ct->buttonBlockContainer->selectedButtons.size()>0) )
        desiredPage=1;
    else
        desiredPage=0;
    if (desiredPage!=currentPage)
    {
        pageDlgs[currentPage]->setVisible(false);
        currentPage=desiredPage;
        pageDlgs[currentPage]->setVisible(true);

        QSize s(pageDlgs[currentPage]->size());
        s.setHeight(originalHeights[currentPage]+TOP_BORDER_WIDTH);

#ifdef MAC_VREP
        if (QT_VERSION>=0x050000)
        { // Since Qt5, we have problems on Mac (resising-->ugly glitch)
            setVisible(false);
            setFixedSize(s);
            setVisible(true);
        }
        else
            setFixedSize(s);
#else
        setFixedSize(s);
#endif


/*
#ifdef WIN_VREP
        // Since Qt5, we have problems on Linux (resizing-->dlg shifts in position) and Mac (resising-->ugly glitch)
        // In that case we keep constant-size dialogs.
        QSize s(pageDlgs[currentPage]->size());
        s.setHeight(originalHeights[currentPage]+TOP_BORDER_WIDTH);
        setFixedSize(s);
#else
        if (QT_VERSION<0x050000)
        {
            QSize s(pageDlgs[currentPage]->size());
            s.setHeight(originalHeights[currentPage]+TOP_BORDER_WIDTH);
            setFixedSize(s);
        }
#endif
*/
    }

    pageDlgs[currentPage]->refresh();
}
