
#pragma once

#include "vDialog.h"
#include "uiThread.h"

class QLineEdit;

class CDlgEx : public VDialog  
{
public:
    CDlgEx(QWidget* pParent);
    CDlgEx(QWidget* pParent,Qt::WindowFlags specialFlags);
    bool event(QEvent* event);
    virtual ~CDlgEx();
    void cancelEvent();
    void okEvent();
    void initializationEvent();

    virtual void dialogCallbackFunc(const SUIThreadCommand* cmdIn,SUIThreadCommand* cmdOut);
    virtual void refresh();
    virtual bool needsDestruction();
    virtual bool doesInstanceSwitchRequireDestruction();

    static int doTransparencyCounter;
protected:
    QLineEdit* getSelectedLineEdit();
    void selectLineEdit(QLineEdit* edit);

    bool _markedForDestruction;
    int _dlgType;
};
