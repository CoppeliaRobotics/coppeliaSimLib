#pragma once

#include <QDialog>

#define VDIALOG_MODAL_RETURN_OK 1
#define VDIALOG_MODAL_RETURN_CANCEL 0

class VDialog : public QDialog
{
    Q_OBJECT
public:
    VDialog(QWidget* pParent);
    VDialog(QWidget* pParent,Qt::WindowFlags specialFlags);
    virtual ~VDialog();

    virtual void cancelEvent();
    virtual void okEvent();
    virtual void initializationEvent();

    void defaultDialogInitializationRoutine();
    void defaultModalDialogEndRoutine(bool theResult);

    int makeDialogModal();
    void showDialog(bool showIt);
    void bringDialogToTop();
    void getDialogPositionAndSize(int post[2],int sizet[2]);
    void setDialogPositionAndSize(int post[2],int sizet[2]);

    static Qt::WindowFlags dialogStyle;
public slots:
    void reject();
    void accept();
};
