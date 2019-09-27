#pragma once

#include "app.h"

class CMiscBase
{
public:
    static bool handleVerSpec_canCtrlShiftSelectObjects()
    {
        #ifdef SIM_WITH_GUI
            // Following 2 lines since 2010/10/12 so that we can override model selection with shift+ctrl down at the same time!
            if ((App::mainWindow!=nullptr)&&(App::mainWindow->getKeyDownState()&1)&&(App::mainWindow->getKeyDownState()&2))
                return(true);
        #endif
        return(false);
    }
    static bool handleVerSpec_hasCorrectModelExtension(const std::string& filename)
    { // ".ttmt" is not a valid extension!
        int l=int(filename.length());
        char extt[4]=VREP_MODEL_EXTENSION;
        return ( (l>=5)&&(filename[l-4]=='.')&&(filename[l-3]==extt[0])&&(filename[l-2]==extt[1])&&(filename[l-1]==extt[2]) );
    }
    static bool handleVerSpec_canSelectAllObjects(){return(true);}
    static bool handleVerSpec_hasPopupDlg(){return(true);}
    static bool handleVerSpec_supportsOpenglBasedCustomUiEdition(){return(true);}
    static bool handleVerSpec_hasInfoBar(){return(true);}
    static bool handleVerSpec_canAbortScriptExecutionViaEmergencyButton(){return(true);}
    static std::string handleVerSpec_getAbortScriptExecutionEmergencyButtonTxt(){return("");}
    static bool handleVerSpec_statusbarDefaultInitiallyVisible(){return(true);}
    static bool handleVerSpec_statusbarMsgToConsole(){return(false);}
    static bool handleVerSpec_fullEditionAdd(){return(true);}
    static bool handleVerSpec_isDisplayNameAltName(){return(false);}
};
