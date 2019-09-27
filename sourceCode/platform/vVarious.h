
#pragma once

#include "vrepMainHeader.h"
#ifndef SIM_WITHOUT_QT_AT_ALL
    #include <QLibrary>
    typedef QLibrary* WLibrary;
    typedef void* WLibraryFunc;
#else
    #ifdef WIN_VREP
        #include <Windows.h>
        typedef HINSTANCE WLibrary;
        typedef FARPROC WLibraryFunc;
    #else
        typedef void* WLibrary;
        typedef void* WLibraryFunc;
    #endif
#endif

#define VVARIOUS_SHOWNORMAL 0
#define VVARIOUS_SHOWMAXIMIZED 1
#define VVARIOUS_HIDE 2

// FULLY STATIC CLASS
class VVarious  
{
public:
    static bool executeExternalApplication(const std::string& file,const std::string& arguments,const std::string& switchToDirectory,int showFlag);

    static WLibrary openLibrary(const char* filename);
    static void closeLibrary(WLibrary lib);
    static WLibraryFunc resolveLibraryFuncName(WLibrary lib,const char* funcName);

    static std::string getModulePath();
    static void removePathFinalSlashOrBackslash(std::string& pathWithOrWithoutFinalThing);

    static std::string splitPath_path(const std::string& fullPathAndName);
    static std::string splitPath_fileBase(const std::string& fullPathAndName);
    static std::string splitPath_fileExtension(const std::string& fullPathAndName);
    static std::string splitPath_fileBaseAndExtension(const std::string& fullPathAndName);
    static bool isAbsolutePath(const std::string& pathAndOptionalFilename);
    static bool copyTextToClipboard(const std::string& text);

#ifdef SIM_WITH_GUI
public:
    static bool openUrl(const std::string& url);
    static bool openTextFile(const std::string& file);
#endif
};
