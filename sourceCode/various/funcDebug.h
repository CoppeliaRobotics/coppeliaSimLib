
#pragma once

#include "vrepMainHeader.h"

class CFuncDebug
{
public:
    CFuncDebug(const char* functionName,int debugMaskToShow);
    virtual ~CFuncDebug();

    static void print(const char* txt,int debugMaskToShow);

    static void setDebugMask(int m);
    static int getDebugMask();

private:
    static int _debugMask;
    std::string _functionName;
    int _debugMaskToShow;
};
