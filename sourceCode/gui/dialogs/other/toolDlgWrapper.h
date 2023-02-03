
#pragma once

#include <dlgEx.h>
#include <uiThread.h>

class CToolDlgWrapper  
{
public:
    CToolDlgWrapper(int dlgType,int resID);
    virtual ~CToolDlgWrapper();
    void dialogCallbackFunc(const SUIThreadCommand* cmdIn,SUIThreadCommand* cmdOut);
    void refresh();
    void setVisible(bool visible,QWidget* parentWindow);
    bool getVisible();
    void visibleInstanceAboutToSwitch();
    void destroyIfDestructionNeeded();
    void getPosition(int pos[2]);
    void setPosition(int x,int y);
    int getDialogType();
    CDlgEx* getDialog();

    void showDialogButDontOpenIt();
    void hideDialogButDontCloseIt();
private:
    CDlgEx* toolDialog;
    int dialogType;
    int resourceID;
    int position[2];
};
