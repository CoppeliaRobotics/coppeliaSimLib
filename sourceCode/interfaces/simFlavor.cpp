#ifndef SIM_PLUS
#include <simFlavor.h>
#include <app.h>
#include <simConst.h>
#include <ttUtil.h>
#include <ser.h>
#include <simStringTable.h>
#include <boost/format.hpp>
#include <boost/algorithm/string/predicate.hpp>

int CSimFlavor::ver()
{
    return(-1);
}

void CSimFlavor::getAboutStr(std::string& title,std::string& txt)
{
    title=IDS_ABOUT_SIM;
    txt="Custom CoppeliaSim";
}

int CSimFlavor::handleReadOpenFile(int f,char* b)
{
    if ( (f<=CSer::filetype_csim_bin_thumbnails_file)||(f==CSer::filetype_csim_bin_scene_buff)||(f==CSer::filetype_csim_bin_model_buff)||(f==CSer::filetype_csim_bin_ui_file) )
        return(1);
    return(-1);
}

void CSimFlavor::setHld(void* w)
{
}

void CSimFlavor::ekd()
{
}

void CSimFlavor::run(int w)
{
}

int CSimFlavor::getIntVal(int w)
{
    if (w==0)
        return(0);
    if (w==1)
        return(CSer::filetype_csim_bin_scene_file);
    if (w==2)
        return(-1);
    return(-1);
}

int CSimFlavor::getIntVal_2int(int w,int v1,int v2)
{
    if (w==0)
        return(1);
    return(-1);
}

int CSimFlavor::getIntVal_str(int w,const char* n)
{
    if (w==0)
    {
        std::string ext(".");
        ext+=SIM_SCENE_EXTENSION;
        if (boost::algorithm::ends_with(n,ext.c_str()))
            return(CSer::filetype_csim_bin_scene_file);
        ext=".";
        ext+=SIM_MODEL_EXTENSION;
        if (boost::algorithm::ends_with(n,ext.c_str()))
            return(CSer::filetype_csim_bin_model_file);
        ext=".";
        ext+=SIM_XML_SCENE_EXTENSION;
        if (boost::algorithm::ends_with(n,ext.c_str()))
            return(CSer::filetype_csim_xml_simplescene_file);
        ext=".";
        ext+=SIM_XML_MODEL_EXTENSION;
        if (boost::algorithm::ends_with(n,ext.c_str()))
            return(CSer::filetype_csim_xml_simplemodel_file);
        return(CSer::filetype_unspecified_file);
    }
    if (w==1)
        return(0);
    if (w==2)
        return(0);
    return(-1);
}

bool CSimFlavor::getBoolVal(int w)
{
    if (w==0)
        return(false);
    if (w==1)
        return(false);
    if (w==2)
        return(true);
    if (w==3)
        return(true);
    if (w==4)
        return(true);
    if (w==5)
        return(true);
    if (w==6)
        return(true);
    if (w==7)
        return(true);
    if (w==8)
        return(false);
    if (w==9)
        return(true);
    if (w==10)
        return(true);
    if (w==12)
        return(true);
    if (w==13)
        return(false);
    if (w==14)
        return(true);
    if (w==15)
        return(false);
    if (w==16)
        return(true);
    if (w==17)
        return(false);
    if (w==18)
        return(false);
    return(false);
}

bool CSimFlavor::getBoolVal_str(int w,const char* str)
{
    if (w==0)
        return(boost::algorithm::ends_with(str,std::string(".")+SIM_MODEL_EXTENSION));
    return(false);
}

std::string CSimFlavor::getStringVal(int w)
{
    if (w==0)
        return("");
    if (w==1)
        return(":/imageFiles/splashBasic.png");
    if (w==2)
        return("CoppeliaSim (customized)");
    if (w==3)
    {
        std::string v(CSimFlavor::getStringVal(2));
        v+=boost::str(boost::format(" V%s %s") % SIM_PROGRAM_VERSION % SIM_PROGRAM_REVISION);
        return(v);
    }
    if (w==4)
        return(":/imageFiles/icon.png");
    if (w==5)
        return(IDS_HELP_TOPICS_MENU_ITEM);
    if (w==6)
        return(std::string(IDS_ABOUT_SIM)+"...");
    if (w==7)
        return(std::string(IDSN_CREDITS)+"...");
    if (w==8)
        return("");
    if (w==11)
        return("CoppeliaSim Crash");
    if (w==12)
    {
        #ifdef MAC_SIM
            return("It seems that CoppeliaSim crashed in last session. If CoppeliaSim didn't crash, you might be running several instances of CoppeliaSim in parallel, or you might have quit CoppeliaSim not correctly (<File --> Quit> is correct). Would you like to try to recover auto-saved scenes? (see also CoppeliaSim's installation folder for backuped scenes)");
        #else
            return("It seems that CoppeliaSim crashed in last session (or you might be running several instances of CoppeliaSim in parallel). Would you like to try to recover auto-saved scenes? (see also CoppeliaSim's installation folder for backuped scenes)");
        #endif
    }
    if (w==15)
        return("non-mobile");
    if (w==16)
        return("dfltscn.ttt");
    return("");
}

std::string CSimFlavor::getStringVal_int(int w,int v)
{
    if (w==0)
        return("");
    if (w==1)
    {
        if (v==0)
            return(SIM_SCENE_EXTENSION);
        if (v==1)
            return(SIM_XML_SCENE_EXTENSION);
        return("");
    }
    if (w==2)
    {
        if (v==0)
            return(SIM_MODEL_EXTENSION);
        if (v==1)
            return(SIM_XML_MODEL_EXTENSION);
        return("");
    }
    return("");
}
#endif
