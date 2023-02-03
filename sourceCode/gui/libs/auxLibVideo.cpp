#include <auxLibVideo.h>
#include <app.h>

WLibrary CAuxLibVideo::_lib=nullptr;

pVideo_recorderGetEncoderString CAuxLibVideo::video_recorderGetEncoderString=0;
pVideo_recorderInitialize CAuxLibVideo::video_recorderInitialize=0;
pVideo_recorderAddFrame CAuxLibVideo::video_recorderAddFrame=0;
pVideo_recorderEnd CAuxLibVideo::video_recorderEnd=0;

bool CAuxLibVideo::loadLibrary()
{
    bool retVal=false;
    std::string vidLibPathAndName(VVarious::getModulePath());
    vidLibPathAndName+="/";
#ifdef WIN_SIM
    vidLibPathAndName+="vvcl.dll";
#endif
#ifdef MAC_SIM
    vidLibPathAndName+="libvvcl.dylib";
#endif
#ifdef LIN_SIM
    vidLibPathAndName+="libvvcl.so";
#endif
    if (_loadLibrary(vidLibPathAndName.c_str()))
        retVal=_getAuxLibProcAddresses();
    return(retVal);
}

bool CAuxLibVideo::_loadLibrary(const char* pathAndFilename)
{
    _lib=VVarious::openLibrary(pathAndFilename);
    return(_lib!=nullptr);
}

void CAuxLibVideo::unloadLibrary()
{
    if (_lib!=nullptr)
        VVarious::closeLibrary(_lib);
    _lib=nullptr;
}

WLibraryFunc CAuxLibVideo::_getProcAddress(const char* funcName)
{
    return(VVarious::resolveLibraryFuncName(_lib,funcName));
}

bool CAuxLibVideo::_getAuxLibProcAddresses()
{
    video_recorderGetEncoderString=(pVideo_recorderGetEncoderString)(_getProcAddress("getAvailableEncoderName"));
    video_recorderInitialize=(pVideo_recorderInitialize)(_getProcAddress("recorderInitialize"));
    video_recorderAddFrame=(pVideo_recorderAddFrame)(_getProcAddress("recorderAddFrame"));
    video_recorderEnd=(pVideo_recorderEnd)(_getProcAddress("recorderEnd"));

    if (video_recorderGetEncoderString==0) return false;
    if (video_recorderInitialize==0) return false;
    if (video_recorderAddFrame==0) return false;
    if (video_recorderEnd==0) return false;
    return true;
}
