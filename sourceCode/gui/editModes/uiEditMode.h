
#pragma once

#include "vrepMainHeader.h"

class VMenu;
class CButtonBlockContainer;

class CUiEditMode
{
public:
    CUiEditMode(CButtonBlockContainer* buttonBlockContainer);
    virtual ~CUiEditMode();

    void addMenu(VMenu* menu);
    bool processCommand(int commandID);

private:
    CButtonBlockContainer* _buttonBlockContainer;
};
