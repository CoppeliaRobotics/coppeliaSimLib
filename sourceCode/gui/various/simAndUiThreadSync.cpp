#include <simAndUiThreadSync.h>
#include <app.h>
#include <vDateTime.h>
#include <tt.h>
#include <utils.h>
#include <guiApp.h>

VSimUiMutex CSimAndUiThreadSync::_uiReadPermission;
VSimUiMutex CSimAndUiThreadSync::_uiWritePermission;

int CSimAndUiThreadSync::_ui_readLevel=0;
int CSimAndUiThreadSync::_ui_writeLevel=0;
int CSimAndUiThreadSync::_sim_readLevel=0;
int CSimAndUiThreadSync::_sim_writeLevel=0;
int CSimAndUiThreadSync::_sim_readLevel_temp=0;
int CSimAndUiThreadSync::_sim_writeLevel_temp=0;

volatile bool CSimAndUiThreadSync::_ui_writeRequest=false;

bool CSimAndUiThreadSync::_showActivityInConsole=false;
bool CSimAndUiThreadSync::_showLockFailsInConsole=false;

int CSimAndUiThreadSync::_nextHandleValue=0;

CSimAndUiThreadSync::CSimAndUiThreadSync(const char* functionName)
{
    _lockFunctionResult=-1;
    _lockType=-1;
    _functionName=functionName;
    _handle=_nextHandleValue++;
}

CSimAndUiThreadSync::~CSimAndUiThreadSync()
{
    if (_lockType==0)
    {
        if (_lockFunctionResult>0)
        {
            _ui_readLevel--;
            _uiReadPermission.unlock(); // release the read permission for the UI thread
        }
    }


    if (_lockType==1)
    {
        if (_lockFunctionResult>0)
        {
            _ui_writeLevel--;
            _uiWritePermission.unlock(); // release the write permission for the UI thread
        }
    }

    if (_lockType==2)
    {
        _sim_writeLevel--;
        _uiReadPermission.unlock(); // release the write permission for the SIM thread (i.e. allow the UI thread to read again)
    }

    if (_lockType==3)
    {
        for (int i=0;i<_sim_readLevel_temp;i++)
            _uiWritePermission.lock(VSimUiMutex::nonUi);
        _sim_readLevel=_sim_readLevel_temp;
        _sim_readLevel_temp=0;


        for (int i=0;i<_sim_writeLevel_temp;i++)
            _uiReadPermission.lock(VSimUiMutex::nonUi);
        _sim_writeLevel=_sim_writeLevel_temp;
        _sim_writeLevel_temp=0;
    }
}

bool CSimAndUiThreadSync::uiThread_tryToLockForUiEventRead(int maxTime)
{ // called by the UI thread only!
    _lockType=0; // UI READ

    if (!VThread::isUiThread())
        App::beep(); // we are NOT in the UI thread. This is a bug!

    if (_lockFunctionResult!=-1)
        return(false); // this condition is important, since this function will be called twice for the same object, and the second time it should return false!

    // We will try to take possession of the _uiReadPermission lock.
    // This will only succeed if the SIM thread is not writing.

    _lockFunctionResult=0; // means lock failed

    int startTime=(int)VDateTime::getTimeInMs();
    int to=1;
    if (maxTime==0)
        to=0;
    while (true)
    {
        if (_uiReadPermission.tryLock(VSimUiMutex::ui,to))
        { // the lock succeeded.
            _lockFunctionResult=1;
            _ui_readLevel++;
            break;
        }
        if ( (maxTime==0)||(VDateTime::getTimeDiffInMs(startTime)>maxTime) )
            break;

        // Following instruction can be important (but not critical if not present, because the lock will simply fail):
        // if that instruction is not present, then the SIM thread might be waiting for the UI
        // thread to do something (e.g. handle a vision sensor that needs to run in the UI thread).
        // While the SIM thread waits, the lock here will never succeed, unless we process the other
        // UI events here:

        // If we do not process all events, then the QFileDialog sometimes fails displaying and can
        // lead to application hang (on Linux, not on Windows. Mac?). We keep 2 tweaks here just in case!

        GuiApp::qtApp->processEvents(); // default
    }

    return(_lockFunctionResult>0);
}

bool CSimAndUiThreadSync::uiThread_tryToLockForUiEventWrite(int maxTime)
{ // called by the UI thread only!
    _lockType=1; // UI WRITE
    if (!VThread::isUiThread())
        App::beep(); // we are NOT in the UI thread. This is a bug!

    if (_lockFunctionResult!=-1)
        return(false); // this condition is important, since this function will be called twice for the same object, and the second time it should return false!

    // We will try to take possession of the _uiWritePermission lock.
    // This will only succeed if the SIM thread has released that lock (happens only in specific code sections).

    _lockFunctionResult=0; // means lock failed

    _ui_writeRequest=true; // so that the SIM thread waits in specific code sections (if not already there)

    int startTime=(int)VDateTime::getTimeInMs();
    while (VDateTime::getTimeDiffInMs(startTime)<=maxTime)
    {
        if (_uiWritePermission.tryLock(VSimUiMutex::ui,10))
        { // the lock succeeded.
            _lockFunctionResult=1;
            _ui_writeLevel++;
            break;
        }
        // Following instruction can be important (but not critical if not present, because the lock will simply fail):
        // if that instruction is not present, then the SIM thread might be waiting for the UI
        // thread to do something (e.g. handle a vision sensor that needs to run in the UI thread).
        // While the SIM thread waits, the lock here will never succeed, unless we process the other
        // UI events here:

        // If we do not process all events, then the QFileDialog sometimes fails displaying and can
        // lead to application hang (on Linux, not on Windows. Mac?). We keep 2 tweaks here just in case!
        GuiApp::qtApp->processEvents(); // default
    }

    _ui_writeRequest=false; // reset the write request

    return(_ui_writeLevel>0);
}

void CSimAndUiThreadSync::simThread_lockForSimThreadWrite()
{ // called by the SIM thread only!
    _lockType=2; // SIM THREAD WRITE

    if (VThread::isUiThread())
        App::beep(); // we are NOT in the UI thread. This is a bug!

    if (_lockFunctionResult!=-1)
        return; // should not happen!

    // We will try to take possession of the _uiReadPermission lock.
    // This will succeed once the UI thread is not reading anymore.

    _lockFunctionResult=1; // lock in this function will always succeed (blocking)

    _uiReadPermission.lock(VSimUiMutex::nonUi);
    _sim_writeLevel++;
}

bool CSimAndUiThreadSync::simOrUiThread_tryToLockForWrite_cApi()
{ // called by the SIM or UI thread, from the C API!
    if (VThread::isUiThread())
    {
        return(uiThread_tryToLockForUiEventWrite(800));
    }
    else
    {
        if (_lockFunctionResult!=-1)
            return(false); // this condition is important, since this function will be called twice for the same object, and the second time it should return false!
        simThread_lockForSimThreadWrite();
        return(true);
    }
}

bool CSimAndUiThreadSync::simOrUiThread_tryToLockForRead_cApi()
{ // called by the SIM or UI thread, from the C API!
    if (VThread::isUiThread())
    {
        return(uiThread_tryToLockForUiEventRead(5));
    }
    else
    {
        if (_lockFunctionResult!=-1)
            return(false); // this condition is important, since this function will be called twice for the same object, and the second time it should return false!
        _lockType=-1;
        _lockFunctionResult=1;
        return(true);
    }
}

bool CSimAndUiThreadSync::simThread_forbidUiThreadToWrite(bool blocking)
{ // this function is static!
    if (blocking)
    {
        _uiWritePermission.lock(VSimUiMutex::nonUi);
        _sim_readLevel++;
        if (_sim_readLevel!=1)
            App::beep(); // should not happen!

        return(true);
    }
    else
    {
        if (_ui_writeRequest)
            return(false);

        bool retVal=_uiWritePermission.tryLock(VSimUiMutex::nonUi);
        if (retVal)
        {
            _sim_readLevel++;
            if (_sim_readLevel!=1)
                App::beep(); // should not happen!
        }
        return(retVal);
    }
}

void CSimAndUiThreadSync::simThread_allowUiThreadToWrite()
{ // this function is static!
    _sim_readLevel--;
    if (_sim_readLevel!=0)
        App::beep(); // should not happen!
    _uiWritePermission.unlock();
}


void CSimAndUiThreadSync::simThread_temporarilyAllowUiThreadToReadAndWrite()
{   _lockType=3; // SIM THREAD TEMPORARILY ALLOWS THE UI THREAD TO READ AND WRITE

    if (VThread::isUiThread())
        App::beep(); // we are NOT in the UI thread. This is a bug!

    // We will release possession of the _uiReadPermission and _uiWritePermission locks.
    // This will succeed once the UI thread is not reading anymore.

    _lockFunctionResult=1; // lock in this function will always succeed (blocking)

    _sim_writeLevel_temp=_sim_writeLevel;
    _sim_writeLevel=0;
    for (int i=0;i<_sim_writeLevel_temp;i++)
        _uiReadPermission.unlock();

    _sim_readLevel_temp=_sim_readLevel;
    _sim_readLevel=0;
    for (int i=0;i<_sim_readLevel_temp;i++)
        _uiWritePermission.unlock();
}

bool CSimAndUiThreadSync::hasUiLockedResourcesForReadOrWrite()
{
    return(_ui_readLevel+_ui_writeLevel);
}

std::string CSimAndUiThreadSync::_getLevelsString(const char* abr)
{
    std::string retString(abr);
    retString+=utils::getIntString(false,_ui_readLevel);
    retString+=",";
    retString+=utils::getIntString(false,_ui_writeLevel);
    retString+=",";
    retString+=utils::getIntString(false,_sim_readLevel);
    retString+=",";
    retString+=utils::getIntString(false,_sim_writeLevel);
    return(retString);
}

void CSimAndUiThreadSync::outputDebugMessage(const char* callLocation,const char* msg)
{ // function is static!
}

void CSimAndUiThreadSync::outputNakedDebugMessage(const char* msg)
{ // function is static!
}

bool CSimAndUiThreadSync::getShowActivityInConsole()
{
    return(_showActivityInConsole);
}

void CSimAndUiThreadSync::setShowActivityInConsole(bool show)
{
    _showActivityInConsole=show;
}

bool CSimAndUiThreadSync::getShowLockFailsActivityInConsole()
{
    return(_showLockFailsInConsole);
}

void CSimAndUiThreadSync::setShowLockFailsActivityInConsole(bool show)
{
    _showLockFailsInConsole=show;
}

int CSimAndUiThreadSync::getObjectHandle()
{
    return(_handle);
}
