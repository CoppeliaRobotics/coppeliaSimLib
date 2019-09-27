
#pragma once

#include "vMenubar.h"

class CHelpMenuBase
{
public:
    static void handleVerSpec_addMenu1(VMenu* menu)
    {
        menu->appendMenuItem(true,false,HELP_TOPICS_CMD,IDS_HELP_TOPICS_MENU_ITEM);
        menu->appendMenuItem(true,false,ABOUT_CMD,std::string(IDS_ABOUT_V_REP)+"...");
        menu->appendMenuItem(true,false,CREDITS_CMD,std::string(IDSN_CREDITS)+"...");
    }
    static bool handleVerSpec_addMenu2(){return(true);}
    static void handleVerSpec_addMenu3(VMenu* menu){}
    static bool handleVerSpec_processCommand1(int commandID){return(false);}
};
