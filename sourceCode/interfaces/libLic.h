#pragma once

#include "vVarious.h"

#ifndef SIM_FL
#define SIM_FL (-1)
#endif

typedef bool (__cdecl *ptr_licPlugin_init)(bool,int,int,int,const char*,const char*,const char*,const char*,const char*,const char*);
typedef char* (__cdecl *ptr_licPlugin_getAboutStr)(char**);
typedef void (__cdecl *ptr_licPlugin_handleBrFile)(int,char*);
typedef int (__cdecl *ptr_licPlugin_handleReadOpenFile)(int,char*);
typedef void (__cdecl *ptr_licPlugin_setHld)(void*);
typedef void (__cdecl *ptr_licPlugin_ekd)();
typedef void (__cdecl *ptr_licPlugin_ifl)(const char*,int);
typedef bool (__cdecl *ptr_licPlugin_hflm)();
typedef void (__cdecl *ptr_licPlugin_run)(int);
typedef int (__cdecl *ptr_licPlugin_getIntVal)(int);
typedef int (__cdecl *ptr_licPlugin_getIntVal_2int)(int,int,int);
typedef int (__cdecl *ptr_licPlugin_getIntVal_str)(int,const char*);
typedef bool (__cdecl *ptr_licPlugin_getBoolVal)(int);
typedef bool (__cdecl *ptr_licPlugin_getBoolVal_int)(int,int);
typedef bool (__cdecl *ptr_licPlugin_getBoolVal_str)(int,const char*);
typedef char* (__cdecl *ptr_licPlugin_getStringVal)(int);
typedef char* (__cdecl *ptr_licPlugin_getStringVal_int)(int,int);

// FULLY STATIC CLASS
class CLibLic
{
public:
    static void loadLibrary();
    static void unloadLibrary();

    static int ver();
    static bool init();
    static void getAboutStr(std::string& title,std::string& txt);
    static void handleBrFile(int f,char* b);
    static int handleReadOpenFile(int f,char* b);
    static void setHld(void* w);
    static void ekd();
    static void ifl(const char* s,int p);
    static bool hflm();
    static void run(int w);
    static int getIntVal(int w);
    static int getIntVal_2int(int w,int v1,int v2);
    static int getIntVal_str(int w,const char* str);
    static bool getBoolVal(int w);
    static bool getBoolVal_int(int w,int v);
    static bool getBoolVal_str(int w,const char* str);
    static std::string getStringVal(int w);
    static std::string getStringVal_int(int w,int v);

private:
    static bool _loadLibrary(const char* pathAndFilename);
    static bool _getAuxLibProcAddresses();
    static WLibraryFunc _getProcAddress(const char* funcName);
    static WLibrary _lib;

    static ptr_licPlugin_init _licPlugin_init;
    static ptr_licPlugin_getAboutStr _licPlugin_getAboutStr;
    static ptr_licPlugin_handleBrFile _licPlugin_handleBrFile;
    static ptr_licPlugin_handleReadOpenFile _licPlugin_handleReadOpenFile;
    static ptr_licPlugin_setHld _licPlugin_setHld;
    static ptr_licPlugin_ekd _licPlugin_ekd;
    static ptr_licPlugin_ifl _licPlugin_ifl;
    static ptr_licPlugin_hflm _licPlugin_hflm;
    static ptr_licPlugin_run _licPlugin_run;
    static ptr_licPlugin_getIntVal _licPlugin_getIntVal;
    static ptr_licPlugin_getIntVal_2int _licPlugin_getIntVal_2int;
    static ptr_licPlugin_getIntVal_str _licPlugin_getIntVal_str;
    static ptr_licPlugin_getBoolVal _licPlugin_getBoolVal;
    static ptr_licPlugin_getBoolVal_int _licPlugin_getBoolVal_int;
    static ptr_licPlugin_getBoolVal_str _licPlugin_getBoolVal_str;
    static ptr_licPlugin_getStringVal _licPlugin_getStringVal;
    static ptr_licPlugin_getStringVal_int _licPlugin_getStringVal_int;
};
