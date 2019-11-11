
#include "easyLock.h"
#include "vThread.h"

#include "debugLogFile.h"

bool CEasyLock::_showActivity=false;

CEasyLock::CEasyLock(VMutex& mutex,const char* functionName)
{
    _wasAlreadyUnlocked=false;
    _mutex=&mutex;
    _functionName=functionName;
    _lock();
}

CEasyLock::~CEasyLock()
{
    if (!_wasAlreadyUnlocked)
        _unlock();
}

void CEasyLock::unlock()
{
    if (!_wasAlreadyUnlocked)
        _unlock();
    _wasAlreadyUnlocked=true;
}

void CEasyLock::lock()
{
    if (_wasAlreadyUnlocked)
        _lock();
    _wasAlreadyUnlocked=false;
}

void CEasyLock::_unlock()
{
    if (_showActivity)
    {
        std::string arrow("... ");
        if (VThread::isUiThreadIdSet())
        {
            if (VThread::isCurrentThreadTheUiThread())
                arrow+="(GUI) (";
            else
                arrow+="(SIM) (";
        }
        else
            arrow+="(GUI) (";
        CDebugLogFile::addDebugText(false,arrow.c_str(),_functionName.c_str(),") 'easy locking'\n");
    }

    _mutex->unlock();

    if (_showActivity)
    {
        std::string arrow("... ");
        if (VThread::isUiThreadIdSet())
        {
            if (VThread::isCurrentThreadTheUiThread())
                arrow+="(GUI) (";
            else
                arrow+="(SIM) (";
        }
        else
            arrow+="(GUI) (";
        CDebugLogFile::addDebugText(false,arrow.c_str(),_functionName.c_str(),") 'easy locked'\n");
    }
}

void CEasyLock::_lock()
{
    if (_showActivity)
    {
        std::string arrow("... ");
        if (VThread::isUiThreadIdSet())
        {
            if (VThread::isCurrentThreadTheUiThread())
                arrow+="(GUI) (";
            else
                arrow+="(SIM) (";
        }
        else
            arrow+="(GUI) (";
        CDebugLogFile::addDebugText(false,arrow.c_str(),_functionName.c_str(),") 'easy unlocking'\n");
    }

    _mutex->lock((_functionName+" (from CEasyLock::_lock)").c_str());

    if (_showActivity)
    {
        std::string arrow("... ");
        if (VThread::isUiThreadIdSet())
        {
            if (VThread::isCurrentThreadTheUiThread())
                arrow+="(GUI) (";
            else
                arrow+="(SIM) (";
        }
        else
            arrow+="(GUI) (";
        CDebugLogFile::addDebugText(false,arrow.c_str(),_functionName.c_str(),") 'easy unlocked'\n");
    }
}

void CEasyLock::setShowActivity(bool show)
{
    _showActivity=show;
}

bool CEasyLock::getShowActivity()
{
    return(_showActivity);
}
