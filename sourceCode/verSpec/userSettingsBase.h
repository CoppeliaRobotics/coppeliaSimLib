
#pragma once

#include "confReaderAndWriter.h"

class CUserSettingsBase
{
public:
    CUserSettingsBase(){}
    virtual ~CUserSettingsBase(){}

    void handleVerSpecConstructor1(){}
    void handleVerSpecSaveUserSettings1(CConfReaderAndWriter& c){}
    void handleVerSpecLoadUserSettings1(CConfReaderAndWriter& c){}
    int ignoreAbortScriptExecTiming(){return(false);}
};
