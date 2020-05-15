#include "libLic.h"
#include "app.h"
#include "simConst.h"
#include "ttUtil.h"
#include "ser.h"
#include "simStringTable.h"
#include <boost/format.hpp>
#include <boost/algorithm/string/predicate.hpp>

WLibrary CLibLic::_lib=nullptr;

ptr_licPlugin_init CLibLic::_licPlugin_init=nullptr;
ptr_licPlugin_getAboutStr CLibLic::_licPlugin_getAboutStr=nullptr;
ptr_licPlugin_handleBrFile CLibLic::_licPlugin_handleBrFile=nullptr;
ptr_licPlugin_handleReadOpenFile CLibLic::_licPlugin_handleReadOpenFile=nullptr;
ptr_licPlugin_setHld CLibLic::_licPlugin_setHld=nullptr;
ptr_licPlugin_ekd CLibLic::_licPlugin_ekd=nullptr;
ptr_licPlugin_ifl CLibLic::_licPlugin_ifl=nullptr;
ptr_licPlugin_hflm CLibLic::_licPlugin_hflm=nullptr;
ptr_licPlugin_run CLibLic::_licPlugin_run=nullptr;
ptr_licPlugin_getIntVal CLibLic::_licPlugin_getIntVal=nullptr;
ptr_licPlugin_getIntVal_2int CLibLic::_licPlugin_getIntVal_2int=nullptr;
ptr_licPlugin_getIntVal_str CLibLic::_licPlugin_getIntVal_str=nullptr;
ptr_licPlugin_getBoolVal CLibLic::_licPlugin_getBoolVal=nullptr;
ptr_licPlugin_getBoolVal_int CLibLic::_licPlugin_getBoolVal_int=nullptr;
ptr_licPlugin_getBoolVal_str CLibLic::_licPlugin_getBoolVal_str=nullptr;
ptr_licPlugin_getStringVal CLibLic::_licPlugin_getStringVal=nullptr;
ptr_licPlugin_getStringVal_int CLibLic::_licPlugin_getStringVal_int=nullptr;

void CLibLic::loadLibrary()
{
    std::string libPathAndName(VVarious::getModulePath());
    libPathAndName+="/";
#ifdef WIN_SIM
    libPathAndName+="simExtLic.dll";
#endif
#ifdef MAC_SIM
    libPathAndName+="libsimExtLic.dylib";
#endif
#ifdef LIN_SIM
    libPathAndName+="libsimExtLic.so";
#endif
    if (_loadLibrary(libPathAndName.c_str()))
    {
        if (!_getAuxLibProcAddresses())
            unloadLibrary();
    }
}

bool CLibLic::_loadLibrary(const char* pathAndFilename)
{
    _lib=VVarious::openLibrary(pathAndFilename);
    return(_lib!=nullptr);
}

void CLibLic::unloadLibrary()
{
    if (_lib!=nullptr)
        VVarious::closeLibrary(_lib);
    _lib=nullptr;
}

WLibraryFunc CLibLic::_getProcAddress(const char* funcName)
{
    return(VVarious::resolveLibraryFuncName(_lib,funcName));
}

bool CLibLic::_getAuxLibProcAddresses()
{
    _licPlugin_init=(ptr_licPlugin_init)(_getProcAddress("licPlugin_init"));
    _licPlugin_getAboutStr=(ptr_licPlugin_getAboutStr)(_getProcAddress("licPlugin_getAboutStr"));
    _licPlugin_handleBrFile=(ptr_licPlugin_handleBrFile)(_getProcAddress("licPlugin_handleBrFile"));
    _licPlugin_handleReadOpenFile=(ptr_licPlugin_handleReadOpenFile)(_getProcAddress("licPlugin_handleReadOpenFile"));
    _licPlugin_setHld=(ptr_licPlugin_setHld)(_getProcAddress("licPlugin_setHld"));
    _licPlugin_ekd=(ptr_licPlugin_ekd)(_getProcAddress("licPlugin_ekd"));
    _licPlugin_ifl=(ptr_licPlugin_ifl)(_getProcAddress("licPlugin_ifl"));
    _licPlugin_hflm=(ptr_licPlugin_hflm)(_getProcAddress("licPlugin_hflm"));
    _licPlugin_run=(ptr_licPlugin_run)(_getProcAddress("licPlugin_run"));
    _licPlugin_getIntVal=(ptr_licPlugin_getIntVal)(_getProcAddress("licPlugin_getIntVal"));
    _licPlugin_getIntVal_2int=(ptr_licPlugin_getIntVal_2int)(_getProcAddress("licPlugin_getIntVal_2int"));
    _licPlugin_getIntVal_str=(ptr_licPlugin_getIntVal_str)(_getProcAddress("licPlugin_getIntVal_str"));
    _licPlugin_getBoolVal=(ptr_licPlugin_getBoolVal)(_getProcAddress("licPlugin_getBoolVal"));
    _licPlugin_getBoolVal_int=(ptr_licPlugin_getBoolVal_int)(_getProcAddress("licPlugin_getBoolVal_int"));
    _licPlugin_getBoolVal_str=(ptr_licPlugin_getBoolVal_str)(_getProcAddress("licPlugin_getBoolVal_str"));
    _licPlugin_getStringVal=(ptr_licPlugin_getStringVal)(_getProcAddress("licPlugin_getStringVal"));
    _licPlugin_getStringVal_int=(ptr_licPlugin_getStringVal_int)(_getProcAddress("licPlugin_getStringVal_int"));

    return (_licPlugin_init!=nullptr);
}

bool CLibLic::init()
{
    std::string qtVer("non-Qt");
#ifndef SIM_WITHOUT_QT_AT_ALL
    qtVer=QT_VERSION_STR;
#endif
    if (_licPlugin_init!=nullptr)
        return(_licPlugin_init(true,CLibLic::ver(),SIM_PROGRAM_VERSION_NB,SIM_PROGRAM_REVISION_NB,SIM_PROGRAM_VERSION,SIM_PROGRAM_REVISION,CTTUtil::dwordToString(CSer::SER_SERIALIZATION_VERSION).c_str(),SIM_COMPILER_STR,__DATE__,qtVer.c_str(),App::getConsoleVerbosity()));
    return(false);
}

void CLibLic::getAboutStr(std::string& title,std::string& txt)
{
    if (_licPlugin_init!=nullptr)
    {
        char* t2;
        char* t1=_licPlugin_getAboutStr(&t2);
        title=t2;
        txt=t1;
        delete[] t1;
        delete[] t2;
    }
    else
    {
        title=IDS_ABOUT_SIM;
        txt="Custom CoppeliaSim";
    }
}

void CLibLic::handleBrFile(int f,char* b)
{
    if (_licPlugin_init!=nullptr)
        _licPlugin_handleBrFile(f,b);
}

int CLibLic::handleReadOpenFile(int f,char* b)
{
    if (_licPlugin_init!=nullptr)
        return(_licPlugin_handleReadOpenFile(f,b));
    else
    {
        if ( (f<=CSer::filetype_csim_bin_thumbnails_file)||(f==CSer::filetype_csim_bin_scene_buff)||(f==CSer::filetype_csim_bin_model_buff)||(f==CSer::filetype_csim_bin_ui_file) )
            return(1);
        return(-1);
    }
    return(1);
}

void CLibLic::setHld(void* w)
{
    if (_licPlugin_init!=nullptr)
        _licPlugin_setHld(w);
}

void CLibLic::ekd()
{
    if (_licPlugin_init!=nullptr)
        _licPlugin_ekd();
}

void CLibLic::ifl(const char* s,int p)
{
    if (_licPlugin_init!=nullptr)
        _licPlugin_ifl(s,p);
}

bool CLibLic::hflm()
{
    if (_licPlugin_init!=nullptr)
        return(_licPlugin_hflm());
    return(false);
}

void CLibLic::run(int w)
{
    if (_licPlugin_init!=nullptr)
        _licPlugin_run(w);
}

int CLibLic::getIntVal(int w)
{
    if (_licPlugin_init!=nullptr)
        return(_licPlugin_getIntVal(w));
    else
    {
        if (w==0)
            return(0);
        if (w==1)
            return(CSer::filetype_csim_bin_scene_file);
        if (w==2)
        {
            if (SIM_FL>=2)
                return(SIM_FL);
            return(-1);
        }
    }
    return(-1);
}

int CLibLic::getIntVal_2int(int w,int v1,int v2)
{
    if (_licPlugin_init!=nullptr)
        return(_licPlugin_getIntVal_2int(w,v1,v2));
    else
    {
        if (w==0)
            return(1);
    }
    return(-1);
}

int CLibLic::getIntVal_str(int w,const char* n)
{
    if (_licPlugin_init!=nullptr)
        return(_licPlugin_getIntVal_str(w,n));
    else
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
            return(CSer::filetype_unspecified_file);
            ext=".";
            ext+=SIM_XML_SCENE_EXTENSION;
            if (boost::algorithm::ends_with(n,ext.c_str()))
                return(CSer::filetype_csim_xml_simplescene_file);
            ext=".";
            ext+=SIM_XML_MODEL_EXTENSION;
            if (boost::algorithm::ends_with(n,ext.c_str()))
                return(CSer::filetype_csim_xml_simplemodel_file);
        }
        if (w==1)
            return(0);
        if (w==2)
            return(0);
    }
    return(-1);
}

bool CLibLic::getBoolVal(int w)
{
    if (_licPlugin_init!=nullptr)
        return(_licPlugin_getBoolVal(w));
    else
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
        if (w==11)
            return(true);
        if (w==12)
            return(true);
        if (w==13)
            return(false);
        if (w==14)
            return(SIM_FL<2);
        if (w==15)
            return(false);
        if (w==16)
            return(SIM_FL==-1);
        if (w==17)
            return(false);
    }
    return(false);
}

bool CLibLic::getBoolVal_int(int w,int v)
{
    if (_licPlugin_init!=nullptr)
        return(_licPlugin_getBoolVal_int(w,v));
    else
    {
        if (w==0)
            return(false);
    }
    return(false);
}

bool CLibLic::getBoolVal_str(int w,const char* str)
{
    if (_licPlugin_init!=nullptr)
        return(_licPlugin_getBoolVal_str(w,str));
    else
    {
        if (w==0)
            return(boost::algorithm::ends_with(str,std::string(".")+SIM_MODEL_EXTENSION));
        if (w==1)
            return(boost::algorithm::ends_with(str,std::string(".")+SIM_XR_SCENE_EXTENSION));
    }
    return(false);
}

std::string CLibLic::getStringVal(int w)
{
    std::string retVal;
    if (w==3)
    {
        retVal=boost::str(boost::format(" V%s %s") % SIM_PROGRAM_VERSION % SIM_PROGRAM_REVISION);
        std::string v(CLibLic::getStringVal(2));
        retVal=v+retVal;
    }
    else
    {
        if (_licPlugin_init!=nullptr)
        {
            char* b=_licPlugin_getStringVal(w);
            retVal=b;
            delete[] b;
        }
        else
        {
            if (w==0)
                return("");
            if (w==1)
                return(":/imageFiles/splashBasic.png");
            if (w==2)
                return("CoppeliaSim (customized)");
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
            if (w==13)
                return("scenes");
            if (w==14)
                return("models");
            if (w==15)
                return("non-mobile");
            if (w==16)
                return("dfltscn.ttt");
        }
    }
    return(retVal);
}

std::string CLibLic::getStringVal_int(int w,int v)
{
    std::string retVal;
    if (_licPlugin_init!=nullptr)
    {
        char* b=_licPlugin_getStringVal_int(w,v);
        retVal=b;
        delete[] b;
    }
    else
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
    }
    return(retVal);
}

int CLibLic::ver()
{
#ifndef SIM_FL
    return(-1);
#else
    return(SIM_FL);
#endif
}
