
#pragma once

#include "vVarious.h"

typedef char (__cdecl *pVideo_recorderGetEncoderString)(int index,char* encoderName_200Chars);
typedef char (__cdecl *pVideo_recorderInitialize)(int resX,int resY,const char* fileAndPath,int frameRate,int encoderIndex);
typedef char (__cdecl *pVideo_recorderAddFrame)(const unsigned char* buffer);
typedef char (__cdecl *pVideo_recorderEnd)(void);

// FULLY STATIC CLASS
class CAuxLibVideo  
{
public:
    static void loadLibrary(bool headless);
    static void unloadLibrary();

    static pVideo_recorderGetEncoderString video_recorderGetEncoderString;
    static pVideo_recorderInitialize video_recorderInitialize;
    static pVideo_recorderAddFrame video_recorderAddFrame;
    static pVideo_recorderEnd video_recorderEnd;

private:
    static bool _loadLibrary(const char* pathAndFilename);
    static bool _getAuxLibProcAddresses();
    static WLibraryFunc _getProcAddress(const char* funcName);
    static WLibrary _lib;
};
