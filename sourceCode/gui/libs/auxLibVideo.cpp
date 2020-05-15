#include "auxLibVideo.h"
#include "app.h"

WLibrary CAuxLibVideo::_lib=nullptr;

pVideo_recorderGetEncoderString CAuxLibVideo::video_recorderGetEncoderString=0;
pVideo_recorderInitialize CAuxLibVideo::video_recorderInitialize=0;
pVideo_recorderAddFrame CAuxLibVideo::video_recorderAddFrame=0;
pVideo_recorderEnd CAuxLibVideo::video_recorderEnd=0;

void CAuxLibVideo::loadLibrary(bool headless)
{
    if (!headless)
    {
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
        {
            if (!_getAuxLibProcAddresses())
            { // Error
                unloadLibrary();
                SSimulationThreadCommand cmd;
                cmd.cmdId=DISPLAY_MESSAGE_CMD;
                cmd.intParams.push_back(sim_msgbox_type_info);
                cmd.stringParams.push_back("Video compression library");
                cmd.stringParams.push_back("Could not find all required functions in the video compression library.");
                App::appendSimulationThreadCommand(cmd,5000);
                App::logMsg(sim_verbosity_errors,"could not find all required functions in the video compression library.");
            }
            else
                App::logMsg(sim_verbosity_loadinfos,"loaded the video compression library.");
        }
        else
        {
            SSimulationThreadCommand cmd;
            cmd.cmdId=DISPLAY_MESSAGE_CMD;
            cmd.intParams.push_back(sim_msgbox_type_info);
            cmd.stringParams.push_back("Video compression library");
            #ifdef LIN_SIM
            cmd.stringParams.push_back("Could not find or correctly load the video compression library.\n\nTry following:\n\n>sudo apt-get install libavcodec-dev libavformat-dev libswscale-dev\nif above fails, try first:\nsudo apt-get -f install");
            #else
            cmd.stringParams.push_back("Could not find or correctly load the video compression library.");
            #endif
            if ( (!App::userSettings->doNotShowVideoCompressionLibraryLoadError)&&(!App::userSettings->suppressStartupDialogs) )
                App::appendSimulationThreadCommand(cmd,5000);
            else
                App::addStatusbarMessage(cmd.stringParams[1].c_str());
            if (App::getConsoleVerbosity()>=sim_verbosity_errors)
            {
                std::string msg("could not find or correctly load the video compression library.");
                #ifdef LIN_SIM
                msg+="\n    Try following:";
                msg+="\n";
                msg+="\n    >sudo apt-get install libavcodec-dev libavformat-dev libswscale-dev";
                msg+="\n    if above fails, try first:";
                msg+="\n    >sudo apt-get -f install";
                msg+="\n");
                #endif
                App::logMsg(sim_verbosity_errors,msg.c_str());
            }
        }
    }
    else
        _lib=nullptr;
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
