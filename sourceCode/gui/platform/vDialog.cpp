
#include "vrepMainHeader.h"
#include "vDialog.h"

Qt::WindowFlags VDialog::dialogStyle;

VDialog::VDialog(QWidget* pParent) : QDialog(pParent,dialogStyle)
{
}

VDialog::VDialog(QWidget* pParent,Qt::WindowFlags specialFlags) : QDialog(pParent,specialFlags)
{
}

VDialog::~VDialog()
{
//  printf("DLGSTYLE: %i\n",QT_MODAL_DLG_STYLE);
}

void VDialog::reject()
{
    cancelEvent();
}
void VDialog::accept()
{
    okEvent();
}

void VDialog::cancelEvent()
{ // virtual
}

void VDialog::okEvent()
{ // virtual
}

void VDialog::initializationEvent()
{ // virtual
}

void VDialog::defaultDialogInitializationRoutine()
{
}

void VDialog::defaultModalDialogEndRoutine(bool theResult)
{
    if (theResult)
        done(QDialog::Accepted);
    else
        done(QDialog::Rejected);
}


int VDialog::makeDialogModal()
{
    setFixedSize(size()); // To have fixed-size dialogs
    setModal(true);
    show(); // added on 26/10/2016
    activateWindow(); // added on 26/10/2016
    int result=exec();
    if (result==QDialog::Accepted)
        return(VDIALOG_MODAL_RETURN_OK);
    return(VDIALOG_MODAL_RETURN_CANCEL);
}

void VDialog::showDialog(bool showIt)
{
    if (showIt)
    {
        show();
        setFixedSize(size()); // To have fixed-size dialogs
    }
    else
        hide();
}

void VDialog::bringDialogToTop()
{
    activateWindow();
    raise();
}

void VDialog::getDialogPositionAndSize(int post[2],int sizet[2])
{ // pos and size can be nullptr
    QRect geom(geometry());
    if (sizet!=nullptr)
    {
        sizet[0]=geom.width();
        sizet[1]=geom.height();
    }
    if (post!=nullptr)
    {
        post[0]=geom.x();
        post[1]=geom.y();
    }
}

void VDialog::setDialogPositionAndSize(int post[2],int sizet[2])
{ // pos and size can be nullptr
    QRect geom(geometry());
    if (post!=nullptr)
    {
        if (sizet==nullptr)
            setGeometry(post[0],post[1],geom.width(),geom.height());
        else
            setGeometry(post[0],post[1],sizet[0],sizet[1]);
    }
    else
    {
        if (sizet!=nullptr)
            setGeometry(geom.x(),geom.y(),sizet[0],sizet[1]);
    }
}
