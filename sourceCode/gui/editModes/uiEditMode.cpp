
#include "uiEditMode.h"
#include "buttonBlockContainer.h"
#include "vMenubar.h"
#include "global.h"
#include "v_repStringTable.h"

CUiEditMode::CUiEditMode(CButtonBlockContainer* buttonBlockContainer)
{
    _buttonBlockContainer=buttonBlockContainer;
    _buttonBlockContainer->setButtonEditMode_editMode(true);
}

CUiEditMode::~CUiEditMode()
{
    _buttonBlockContainer->setButtonEditMode_editMode(false);
}

void CUiEditMode::addMenu(VMenu* menu)
{
    CButtonBlock* it=_buttonBlockContainer->getBlockWithID(_buttonBlockContainer->getBlockInEdition());
    menu->appendMenuItem(it!=nullptr,false,UI_EDIT_MODE_COPY_UI_EMCMD,IDS_COPY_SELECTED_2D_ELEMENT_MENU_ITEM);
    menu->appendMenuItem(_buttonBlockContainer->getCopyOfBlock_forEditMode()!=nullptr,false,UI_EDIT_MODE_PASTE_UI_EMCMD,IDS_PASTE_BUFFER_MENU_ITEM);
    menu->appendMenuItem(it!=nullptr,false,UI_EDIT_MODE_DELETE_UI_EMCMD,IDS_DELETE_SELECTED_2D_ELEMENT_MENU_ITEM);
    menu->appendMenuItem(it!=nullptr,false,UI_EDIT_MODE_CUT_UI_EMCMD,IDS_CUT_SELECTED_2D_ELEMENT_MENU_ITEM);

    menu->appendMenuSeparator();

    menu->appendMenuItem(true,false,ANY_EDIT_MODE_FINISH_AND_APPLY_CHANGES_EMCMD,IDS_LEAVE_2D_ELEMENT_EDIT_MODE_MENU_ITEM);
}

bool CUiEditMode::processCommand(int commandID)
{ // Return value is true if the command was successful
    bool retVal=true;

    if (commandID==UI_EDIT_MODE_COPY_UI_EMCMD)
    {
        _buttonBlockContainer->copyBlockInEdition_editMode();
        return(retVal);
    }

    if (commandID==UI_EDIT_MODE_PASTE_UI_EMCMD)
    {
        _buttonBlockContainer->pasteCopiedBlock_editMode();
        return(retVal);
    }

    if (commandID==UI_EDIT_MODE_DELETE_UI_EMCMD)
    {
        _buttonBlockContainer->deleteBlockInEdition_editMode();
        return(retVal);
    }

    if (commandID==UI_EDIT_MODE_CUT_UI_EMCMD)
    {
        int savedSel=_buttonBlockContainer->getBlockInEdition();
        _buttonBlockContainer->copyBlockInEdition_editMode();
        _buttonBlockContainer->setBlockInEdition(savedSel);
        _buttonBlockContainer->deleteBlockInEdition_editMode();
        return(retVal);
    }

    if (commandID==UI_EDIT_MODE_DELETE_SELECTED_BUTTONS_EMCMD)
    {
        retVal=_buttonBlockContainer->deleteSelectedButtons_editMode();
        return(retVal);
    }

    return(false);
}
