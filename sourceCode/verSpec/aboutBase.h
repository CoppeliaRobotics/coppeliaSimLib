
#pragma once

#include "pluginContainer.h"

class CAboutBase
{
public:
    static std::string handleVerSpec_1()
    {
        if (CPluginContainer::isMeshPluginAvailable())
            return(":/imageFiles_vrep/splashEdu.png"); // you may change this splash screen only if you use the commercial version of the plugins!
        else
            return(":/imageFiles_vrep/splashBasic.png");
    }
    static void handleVerSpec_2(std::string& title,std::string& txt)
    {
        title=IDS_ABOUT_V_REP;
        txt=CTTUtil::getFormattedString("V-REP custom compilation, Version ",VREP_PROGRAM_VERSION," ",VREP_PROGRAM_REVISION,"  64bit (serialization version ");
        txt+=CTTUtil::dwordToString(CSer::SER_SERIALIZATION_VERSION);
        txt+=")";
        txt+=CTTUtil::getFormattedString("\n(Qt Version ",QT_VERSION_STR,", ",VREP_COMPILER_STR,")");
        txt+="\n";
        txt+=CTTUtil::getFormattedString("\nBuilt ",__DATE__);
        txt+="\nCopyright (c) 2006-";
        std::string date(__DATE__);
        for (int i=0;i<4;i++)
            txt+=date[date.length()-4+i];
        txt+=CTTUtil::getFormattedString(" ",VREP_COMPANY_NAME_TXT,"\n",VREP_WEBPAGE_ADDRESS_TXT);
    }
};
