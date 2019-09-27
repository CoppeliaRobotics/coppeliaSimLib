
#include "vrepMainHeader.h"
#include "dlgEx.h"
#include "app.h"
#include <QLineEdit>

int CDlgEx::doTransparencyCounter=0;

CDlgEx::CDlgEx(QWidget* pParent) : VDialog(pParent)
{
    _markedForDestruction=false;
    initializationEvent();
}
CDlgEx::CDlgEx(QWidget* pParent,Qt::WindowFlags specialFlags) : VDialog(pParent,specialFlags)
{
    _markedForDestruction=false;
    initializationEvent();
}

bool CDlgEx::event(QEvent* event)
{
    return(QDialog::event(event));
}

CDlgEx::~CDlgEx()
{
}

void CDlgEx::refresh()
{
}

void CDlgEx::dialogCallbackFunc(const SUIThreadCommand* cmdIn,SUIThreadCommand* cmdOut)
{
}

bool CDlgEx::needsDestruction()
{
    return(_markedForDestruction);
}

bool CDlgEx::doesInstanceSwitchRequireDestruction()
{
    return(false); // default behaviour
}


void CDlgEx::cancelEvent()
{ // We just hide the dialog and destroy it at next rendering pass
//#ifndef LIN_VREP
//    showDialog(false); // on Linux it seems that once a window is hidden, its position becomes (0,0)!!! So we don't hide it on Linux, we just destroy it later!
//#endif
    _markedForDestruction=true;
    App::setToolbarRefreshFlag();
}

void CDlgEx::okEvent()
{
}

void CDlgEx::initializationEvent()
{
    defaultDialogInitializationRoutine();
    refresh();
}

QLineEdit* CDlgEx::getSelectedLineEdit()
{
    QList<QLineEdit*> wl=((QWidget*)this)->findChildren<QLineEdit*>(QString());
    for (int i=0;i<wl.size();i++)
    {
        if (wl[i]->selectedText().size()>0)
            return(wl[i]);
    }
    return(nullptr);
}

void CDlgEx::selectLineEdit(QLineEdit* edit)
{
    if (edit!=nullptr)
    {
        edit->setFocus();
        edit->selectAll();
    }
}
