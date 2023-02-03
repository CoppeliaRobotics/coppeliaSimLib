#pragma once
#include <vMenubar.h>

class CHelpMenu  
{
public:
    CHelpMenu();
    virtual ~CHelpMenu();

    static void addMenu(VMenu* menu);
    static bool processCommand(int commandID);
};
