
#include "funcDebug.h"
#include "vThread.h"
#include "debugLogFile.h"

int CFuncDebug::_debugMask=0;

CFuncDebug::CFuncDebug(const char* functionName,int debugMaskToShow)
{
    _functionName=functionName;
    _debugMaskToShow=debugMaskToShow;
    if ((_debugMask&_debugMaskToShow)!=0)
    {
        std::string arrow("--> ");
        if (VThread::isUiThreadIdSet())
        {
            if (VThread::isCurrentThreadTheUiThread())
                arrow+="(GUI ";
            else
                arrow+="(SIM ";
        }
        else
            arrow+="(GUI ";


        if (debugMaskToShow==1)
            arrow+="FUNC) ";
        if (debugMaskToShow==2)
            arrow+="C API) ";
        if (debugMaskToShow==4)
            arrow+="LUA API) ";

        CDebugLogFile::addDebugText(false,arrow.c_str(),_functionName.c_str(),"\n");
    }
}

CFuncDebug::~CFuncDebug()
{
    if ((_debugMask&_debugMaskToShow)!=0)
    {
        std::string arrow("<-- ");
        if (VThread::isUiThreadIdSet())
        {
            if (VThread::isCurrentThreadTheUiThread())
                arrow+="(GUI ";
            else
                arrow+="(SIM ";
        }
        else
            arrow+="(GUI ";


        if (_debugMaskToShow==1)
            arrow+="FUNC) ";
        if (_debugMaskToShow==2)
            arrow+="C API) ";
        if (_debugMaskToShow==4)
            arrow+="LUA API) ";

        CDebugLogFile::addDebugText(false,arrow.c_str(),_functionName.c_str(),"\n");
    }
}

void CFuncDebug::print(const char* txt,int debugMaskToShow)
{ // static function
    if ((_debugMask&debugMaskToShow)!=0)
    {
        if (VThread::isUiThreadIdSet())
        {
            if (VThread::isCurrentThreadTheUiThread())
                CDebugLogFile::addDebugText(false,"==> (GUI FUNC) ",txt,"\n");
            else
                CDebugLogFile::addDebugText(false,"==> (SIM FUNC) ",txt,"\n");
        }
        else
            CDebugLogFile::addDebugText(false,"==> (GUI FUNC) ",txt,"\n");
    }
}


void CFuncDebug::setDebugMask(int m)
{
    _debugMask=m;
}

int CFuncDebug::getDebugMask()
{
    return(_debugMask);
}
