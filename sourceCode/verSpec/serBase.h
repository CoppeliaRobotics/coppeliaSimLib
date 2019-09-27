#pragma once

#include "vrepConfig.h"
#include <boost/algorithm/string/predicate.hpp>

class CSer;

class CSerBase
{
public:
    CSerBase(){}
    virtual ~CSerBase(){}

    void handleVerSpecConstructor1(CSer* ser){}
    void handleVerSpecConstructor2(CSer* ser){}
    bool handleVerSpecWriteClose1(bool compress,char filetype){return compress;}
    void handleVerSpecWriteClose2(CSer* ser,char filetype){}
    int handleVerSpecReadOpen(CSer* ser,char filetype){if ( (filetype<=3)||(filetype==6)||(filetype==7) ) return(1); return(-3);}
    
    static char typeFromName(const char* filename)
    {
        std::string ext(".");
        ext+=VREP_SCENE_EXTENSION;
        if (boost::algorithm::ends_with(filename,ext.c_str()))
            return(1);
        ext=".";
        ext+=VREP_MODEL_EXTENSION;
        if (boost::algorithm::ends_with(filename,ext.c_str()))
            return(2);
        return(0);
    }
};
