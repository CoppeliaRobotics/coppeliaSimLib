#pragma once

#include "toolDlgWrapper.h"
#include "dlgEx.h"
#include "vMenubar.h"

class CDlgCont
{
public:
    CDlgCont(QWidget* pWindow);
    virtual ~CDlgCont();

    void initialize(QWidget* pWindow);
    void refresh();
    void callDialogFunction(const SUIThreadCommand* cmdIn,SUIThreadCommand* cmdOut);
    void destroyWhatNeedsDestruction();
    void visibleInstanceAboutToSwitch();

    bool openOrBringToFront(int dlgID);
    void close(int dlgID);
    bool toggle(int dlgID);

    void showDialogsButDontOpenThem();
    void hideDialogsButDontCloseThem();


    CToolDlgWrapper* _getDialogWrapper(int dlgID);
    VDialog* getDialog(int dlgID);
    bool isVisible(int dlgID);
    void killAllDialogs(); // Need initialization again after that
    void getWindowPos(int dlgID,int pos[2],bool& visible);
    void setWindowPos(int dlgID,int pos[2],bool visible);

    void keyPress(int key);
    void addMenu(VMenu* menu);
    bool processCommand(int commandID);
    std::vector<CToolDlgWrapper*> dialogs;

    bool _destroyingContainerNow;
private:
    QWidget* parentWindow;
};
