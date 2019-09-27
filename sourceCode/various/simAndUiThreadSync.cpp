
#include "vrepMainHeader.h"
#include "simAndUiThreadSync.h"
#include "app.h"
#include "vDateTime.h"
#include "debugLogFile.h"
#include "tt.h"

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
            if (_showActivityInConsole)
                CDebugLogFile::addDebugText(false,_getLevelsString("$$R ").c_str()," (GUI) 'unlock for read' (",_functionName.c_str(),")\n");
        }
    }


    if (_lockType==1)
    {
        if (_lockFunctionResult>0)
        {
            _ui_writeLevel--;

            if (_ui_writeLevel==0)
            { // following to avoid visual artifacts:
                for (int i=0;i<int(App::ct->objCont->objectList.size());i++)
                {
                    C3DObject* it=App::ct->objCont->getObjectFromHandle(App::ct->objCont->objectList[i]);
                    it->bufferMainDisplayStateVariables();
                }
            }

            _uiWritePermission.unlock(); // release the write permission for the UI thread

            if (_showActivityInConsole)
            {
                CDebugLogFile::addDebugText(false,_getLevelsString("$$W ").c_str()," (GUI) 'unlock for write' (",_functionName.c_str(),")\n");
                if (_ui_writeLevel==0)
                {
                    outputNakedDebugMessage("$$W -------------------------------------------------------\n");
                    outputNakedDebugMessage("$$W -------------------------------------------------------\n");
                    outputNakedDebugMessage("$$W\n");
                }
            }
        }
    }

    if (_lockType==2)
    {
        _sim_writeLevel--;
        _uiReadPermission.unlock(); // release the write permission for the SIM thread (i.e. allow the UI thread to read again)
        if (_showActivityInConsole)
        {
            CDebugLogFile::addDebugText(false,_getLevelsString("$$S ").c_str()," (SIM) 'unlock for write' (",_functionName.c_str(),")\n");
            if (_sim_writeLevel==0)
            {
                outputNakedDebugMessage("$$S +++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
                outputNakedDebugMessage("$$S +++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
                outputNakedDebugMessage("$$S\n");
            }
        }
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

        if (_showActivityInConsole)
        {
            CDebugLogFile::addDebugText(false,_getLevelsString("$$T ").c_str()," (SIM) 'restore previous locks' (",_functionName.c_str(),")\n");
            outputNakedDebugMessage("$$T #######################################################\n");
            outputNakedDebugMessage("$$T #######################################################\n");
            outputNakedDebugMessage("$$T\n");
        }
    }
}

bool CSimAndUiThreadSync::uiThread_tryToLockForUiEventRead(int maxTime)
{ // called by the UI thread only!
    _lockType=0; // UI READ

    if (!VThread::isCurrentThreadTheUiThread())
    { // we are NOT in the UI thread. This is a bug!
        CDebugLogFile::addDebugText(true,"$$$ERROR in uiThread_tryToLockForUiEventRead (",_functionName.c_str(),"): not called from the UI thread.\n");
        App::beep();
    }

    if (_lockFunctionResult!=-1)
        return(false); // this condition is important, since this function will be called twice for the same object, and the second time it should return false!

    // We will try to take possession of the _uiReadPermission lock.
    // This will only succeed if the SIM thread is not writing.

    _lockFunctionResult=0; // means lock failed

    if (_showActivityInConsole)
        CDebugLogFile::addDebugText(false,_getLevelsString("$$R ").c_str()," (GUI) 'try to lock for read' (",_functionName.c_str(),")\n");

    int startTime=VDateTime::getTimeInMs();
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

        App::qtApp->processEvents(); // default
    }

    if (_lockFunctionResult==1)
    { // the SIM thread doesn't write data. This means that the UI thread can read data.
        if (_showActivityInConsole)
            CDebugLogFile::addDebugText(false,_getLevelsString("$$R ").c_str()," (GUI) 'locked for read' (",_functionName.c_str(),")\n");
    }
    else
    { // the SIM thread is writing data. This means that the UI thread CANNOT read data.
        if (_showActivityInConsole||_showLockFailsInConsole)
        {
            outputNakedDebugMessage("$$R >>>>>>>>>>>>>>>>>>>>>>>\n");
            CDebugLogFile::addDebugText(false,_getLevelsString("$$R ").c_str()," (GUI) 'could not lock for read' (",_functionName.c_str(),")\n");
            outputNakedDebugMessage("$$R >>>>>>>>>>>>>>>>>>>>>>>\n");
        }
    }

    return(_lockFunctionResult>0);
}

bool CSimAndUiThreadSync::uiThread_tryToLockForUiEventWrite(int maxTime)
{ // called by the UI thread only!
    _lockType=1; // UI WRITE
    if (!VThread::isCurrentThreadTheUiThread())
    { // we are NOT in the UI thread. This is a bug!
        CDebugLogFile::addDebugText(true,"$$$ERROR in uiThread_tryToLockForUiEventWrite (",_functionName.c_str(),"): not called from the UI thread.\n");
        App::beep();
    }

    if (_lockFunctionResult!=-1)
        return(false); // this condition is important, since this function will be called twice for the same object, and the second time it should return false!

    // We will try to take possession of the _uiWritePermission lock.
    // This will only succeed if the SIM thread has released that lock (happens only in specific code sections).

    _lockFunctionResult=0; // means lock failed

    if (_showActivityInConsole)
    {
        CDebugLogFile::addDebugText(false,_getLevelsString("$$W ").c_str()," (GUI) 'try to lock for write' (",_functionName.c_str(),")\n");
    }

    _ui_writeRequest=true; // so that the SIM thread waits in specific code sections (if not already there)

    int startTime=VDateTime::getTimeInMs();
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
        App::qtApp->processEvents(); // default
    }

    _ui_writeRequest=false; // reset the write request

    if (_showActivityInConsole)
    {
        if (_lockFunctionResult>0)
        {
            if (_ui_writeLevel==1)
            {
                outputNakedDebugMessage("$$W\n");
                outputNakedDebugMessage("$$W -------------------------------------------------------\n");
                outputNakedDebugMessage("$$W -------------------------------------------------------\n");
            }
            CDebugLogFile::addDebugText(false,_getLevelsString("$$W ").c_str()," (GUI) 'locked for write' (",_functionName.c_str(),")\n");
        }
    }
    if (_showActivityInConsole||_showLockFailsInConsole)
    {
        if (_lockFunctionResult<=0)
        {
            outputNakedDebugMessage("$$W >>>>>>>>>>>>>>>>>>>>>>>\n");
            CDebugLogFile::addDebugText(false,_getLevelsString("$$W ").c_str()," (GUI) 'could not lock for write' (",_functionName.c_str(),")\n");
            outputNakedDebugMessage("$$W >>>>>>>>>>>>>>>>>>>>>>>\n");
        }
    }

    return(_ui_writeLevel>0);
}

void CSimAndUiThreadSync::simThread_lockForSimThreadWrite()
{ // called by the SIM thread only!
    _lockType=2; // SIM THREAD WRITE

    if (VThread::isCurrentThreadTheUiThread())
    { // we are NOT in the UI thread. This is a bug!
        CDebugLogFile::addDebugText(true,"$$SERROR in simThread_lockForSimThreadWrite (",_functionName.c_str(),"): called from a the UI thread.\n");
        App::beep();
    }

    if (_lockFunctionResult!=-1)
        return; // should not happen!

    // We will try to take possession of the _uiReadPermission lock.
    // This will succeed once the UI thread is not reading anymore.

    _lockFunctionResult=1; // lock in this function will always succeed (blocking)

    if (_showActivityInConsole)
        CDebugLogFile::addDebugText(false,_getLevelsString("$$S ").c_str()," (SIM) 'locking for write' (",_functionName.c_str(),")\n");

    _uiReadPermission.lock(VSimUiMutex::nonUi);
    _sim_writeLevel++;

    if (_showActivityInConsole)
    {
        if (_sim_writeLevel==1)
        {
            outputNakedDebugMessage("$$S\n");
            outputNakedDebugMessage("$$S +++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
            outputNakedDebugMessage("$$S +++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
        }
        CDebugLogFile::addDebugText(false,_getLevelsString("$$S ").c_str()," (SIM) 'locked for write' (",_functionName.c_str(),")\n");
    }
}

bool CSimAndUiThreadSync::simOrUiThread_tryToLockForWrite_cApi()
{ // called by the SIM or UI thread, from the C API!
    if (VThread::isCurrentThreadTheUiThread())
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
    if (VThread::isCurrentThreadTheUiThread())
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
        { // should not happen!
            CDebugLogFile::addDebugText(true,"$$SERROR in simThread_forbidUiThreadToWrite: multiple SIM read locks.\n");
            App::beep();
        }

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
            { // should not happen!
                CDebugLogFile::addDebugText(true,"$$SERROR in simThread_forbidUiThreadToWrite: multiple SIM read locks.\n");
                App::beep();
            }
        }
        return(retVal);
    }
}

void CSimAndUiThreadSync::simThread_allowUiThreadToWrite()
{ // this function is static!
    _sim_readLevel--;
    if (_sim_readLevel!=0)
    { // should not happen!
        CDebugLogFile::addDebugText(true,"$$SERROR in simThread_allowUiThreadToWrite: multiple SIM read locks.\n");
        App::beep();
    }
    _uiWritePermission.unlock();
}


void CSimAndUiThreadSync::simThread_temporarilyAllowUiThreadToReadAndWrite()
{   _lockType=3; // SIM THREAD TEMPORARILY ALLOWS THE UI THREAD TO READ AND WRITE

    if (VThread::isCurrentThreadTheUiThread())
    { // we are NOT in the UI thread. This is a bug!
        CDebugLogFile::addDebugText(true,"$$T ERROR in simThread_temporarilyAllowUiThreadToReadAndWrite (",_functionName.c_str(),"): called from a the UI thread.\n");
        App::beep();
    }

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

    if (_showActivityInConsole)
    {
        outputNakedDebugMessage("$$T\n");
        outputNakedDebugMessage("$$T #######################################################\n");
        outputNakedDebugMessage("$$T #######################################################\n");
        CDebugLogFile::addDebugText(false,_getLevelsString("$$T ").c_str()," (SIM) 'temporarily release locks' (",_functionName.c_str(),")\n");
    }
}

bool CSimAndUiThreadSync::hasUiLockedResourcesForReadOrWrite()
{
    return(_ui_readLevel+_ui_writeLevel);
}

std::string CSimAndUiThreadSync::_getLevelsString(const char* abr)
{
    std::string retString(abr);
    retString+=tt::FNb(0,_ui_readLevel);
    retString+=",";
    retString+=tt::FNb(0,_ui_writeLevel);
    retString+=",";
    retString+=tt::FNb(0,_sim_readLevel);
    retString+=",";
    retString+=tt::FNb(0,_sim_writeLevel);
    return(retString);
}

void CSimAndUiThreadSync::outputDebugMessage(const char* callLocation,const char* msg)
{ // function is static!
    if (_showActivityInConsole)
        CDebugLogFile::addDebugText(false,_getLevelsString("$$$ ").c_str()," 'aux debug msg' (",callLocation,"): ",msg,"\n");
}

void CSimAndUiThreadSync::outputNakedDebugMessage(const char* msg)
{ // function is static!
    if (_showActivityInConsole)
        CDebugLogFile::addDebugText(false,msg);
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
