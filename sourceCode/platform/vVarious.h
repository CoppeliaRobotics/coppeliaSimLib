#pragma once

#include <QLibrary>
typedef QLibrary* WLibrary;
typedef void* WLibraryFunc;

#define VVARIOUS_SHOWNORMAL 0
#define VVARIOUS_SHOWMAXIMIZED 1
#define VVARIOUS_HIDE 2

// FULLY STATIC CLASS
class VVarious
{
  public:
    static bool executeExternalApplication(const char* file, const char* arguments, const char* switchToDirectory,
                                           int showFlag);

    static WLibrary openLibrary(const char* filename, std::string* errMsg);
    static void closeLibrary(WLibrary lib, std::string* errMsg);
    static WLibraryFunc resolveLibraryFuncName(WLibrary lib, const char* funcName);

    static void removePathFinalSlashOrBackslash(std::string& pathWithOrWithoutFinalThing);

    static std::string splitPath_path(const char* fullPathAndName);
    static std::string splitPath_fileBase(const char* fullPathAndName);
    static std::string splitPath_fileExtension(const char* fullPathAndName);
    static std::string splitPath_fileBaseAndExtension(const char* fullPathAndName);
    static bool isAbsolutePath(const char* pathAndOptionalFilename);

#ifdef SIM_WITH_GUI
    static bool copyTextToClipboard(const char* text);
    static bool openOfflineUrl(const char* url);
    static bool openOnlineUrl(const char* url);
    static bool openTextFile(const char* file);
#endif
};
