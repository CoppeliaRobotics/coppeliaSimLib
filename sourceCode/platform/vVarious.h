#pragma once

#ifndef SIM_WITHOUT_QT_AT_ALL
    #include <QLibrary>
    typedef QLibrary* WLibrary;
    typedef void* WLibraryFunc;
#else
    #include <string>
    #ifdef WIN_SIM
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
    static bool executeExternalApplication(const char* file,const char* arguments,const char* switchToDirectory,int showFlag);

    static WLibrary openLibrary(const char* filename);
    static void closeLibrary(WLibrary lib);
    static WLibraryFunc resolveLibraryFuncName(WLibrary lib,const char* funcName);

    static std::string getModulePath();
    static void removePathFinalSlashOrBackslash(std::string& pathWithOrWithoutFinalThing);

    static std::string splitPath_path(const char* fullPathAndName);
    static std::string splitPath_fileBase(const char* fullPathAndName);
    static std::string splitPath_fileExtension(const char* fullPathAndName);
    static std::string splitPath_fileBaseAndExtension(const char* fullPathAndName);
    static bool isAbsolutePath(const char* pathAndOptionalFilename);
    static bool copyTextToClipboard(const char* text);

#ifdef SIM_WITH_GUI
public:
    static bool openUrl(const char* url);
    static bool openTextFile(const char* file);
#endif
};
