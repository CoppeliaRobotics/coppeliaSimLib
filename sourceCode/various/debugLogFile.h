#pragma once

#include "vFile.h"
#include "vArchive.h"

class CDebugLogFile
{ // fully static
public:
    CDebugLogFile();
    virtual ~CDebugLogFile();
    static void addDebugText(bool forceAlsoToConsole,const char* txt);
    static void addDebugText(bool forceAlsoToConsole,const char* txt1,const char* txt2);
    static void addDebugText(bool forceAlsoToConsole,const char* txt1,const char* txt2,const char* txt3);
    static void addDebugText(bool forceAlsoToConsole,const char* txt1,const char* txt2,const char* txt3,const char* txt4);
    static void addDebugText(bool forceAlsoToConsole,const char* txt1,const char* txt2,const char* txt3,const char* txt4,const char* txt5);
    static void addDebugText(bool forceAlsoToConsole,const char* txt1,const char* txt2,const char* txt3,const char* txt4,const char* txt5,const char* txt6);
    static bool getDebugToFile();
    static void setDebugToFile(bool toFile);
    static bool getAddTime();
    static void setAddTime(bool t);

private:
    static bool _debugToFile;
    static bool _addTime;
    static VFile* _file;
    static VArchive* _archive;
};
