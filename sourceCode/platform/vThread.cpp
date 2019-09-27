#include "vrepMainHeader.h"
#include "vThread.h"

#ifdef WIN_VREP
    #include <Windows.h>
    #include <process.h>
#endif
#ifndef SIM_WITHOUT_QT_AT_ALL
    #include "wThread.h"
#endif

VMutex VThread::_lock;
bool VThread::_simulationMainThreadIdSet=false;
bool VThread::_uiThreadIdSet=false;
VTHREAD_ID_TYPE VThread::_simulationMainThreadId=VTHREAD_ID_DEAD;
VTHREAD_ID_TYPE VThread::_uiThreadId=VTHREAD_ID_DEAD;
std::vector<VTHREAD_ID_TYPE> VThread::_apiQueriedThreadIds;
VTHREAD_AFFINITY_MASK VThread::_mainThreadAffinityMask=0; // We use the OS default

void VThread::setProcessorCoreAffinity(int mode)
{
#ifdef WIN_VREP
    if (mode<0)
    {
        _mainThreadAffinityMask=0; // Means: we keep what the os choose
        return;
    }
    if (mode>0)
    { // we (try to) use the mask given in "mode":
        _mainThreadAffinityMask=mode;

        unsigned long oldAffinity=SetThreadAffinityMask(GetCurrentThread(),_mainThreadAffinityMask);
        if (oldAffinity==0)
            _mainThreadAffinityMask=0; // we failed setting what we wanted... we use the os default
        return;
    }
    else
    {
        bool found=false;
        for (int i=0;i<64;i++)
        {
            _mainThreadAffinityMask=(1<<i);
            unsigned long oldAffinity=SetThreadAffinityMask(GetCurrentThread(),_mainThreadAffinityMask);
            if (oldAffinity!=0)
            { // Means we could set the new affinity!
                found=true;
                break; 
            }
        }
        if (!found)
            _mainThreadAffinityMask=0;
    }
#else
    // not yet implemented
#endif
}

void VThread::launchThread(VTHREAD_START_ADDRESS startAddress,bool followMainThreadAffinity)
{
#ifdef WIN_VREP
    unsigned long aff=0;
    if ( (_mainThreadAffinityMask!=0)&&followMainThreadAffinity )
        aff=_mainThreadAffinityMask;

    uintptr_t theWinThread=_beginthread(startAddress,0,0);
    if (aff!=0)
        SetThreadAffinityMask((HANDLE)theWinThread,aff);
#else
    quint32 aff=0;
    if ( (_mainThreadAffinityMask!=0)&&followMainThreadAffinity )
        aff=_mainThreadAffinityMask;
    pthread_t th;
    pthread_create(&th,nullptr,startAddress,nullptr);
#endif
}

void VThread::endThread()
{
#ifndef WIN_VREP
    pthread_detach(pthread_self());
#endif
}

void VThread::endSimpleThread()
{
#ifdef SIM_WITHOUT_QT_AT_ALL
    endThread();
#endif
}

#ifndef SIM_WITHOUT_QT_AT_ALL
void VThread::launchSimpleThread(SIMPLE_VTHREAD_START_ADDRESS startAddress)
{
    _lock.lock_simple();
    Thread* it=new Thread();
    Thread::startAddress=startAddress;
    Thread::startAddressIsFree=false;
    it->start(QThread::NormalPriority);
    while (!Thread::startAddressIsFree)
        QThread::yieldCurrentThread();
    _lock.unlock_simple();
}
#endif

int VThread::getThreadId_apiQueried()
{ // this is an artificial ID, just needed externally. 0=GUI thread, 1=main sim thread.
    if ((_apiQueriedThreadIds.size()==0)&&_simulationMainThreadIdSet&&_uiThreadIdSet)
    {
        _apiQueriedThreadIds.push_back(_uiThreadId);
        _apiQueriedThreadIds.push_back(_simulationMainThreadId);
    }
    if (_apiQueriedThreadIds.size()==0)
        return(-1);
    VTHREAD_ID_TYPE t=getCurrentThreadId();
    for (int i=0;i<int(_apiQueriedThreadIds.size());i++)
    {
        if (areThreadIDsSame(_apiQueriedThreadIds[i],t))
            return(i);
    }
    _apiQueriedThreadIds.push_back(t);
    return(int(_apiQueriedThreadIds.size())-1);
}

int VThread::getCoreCount()
{
    int retVal=0;
#ifdef SIM_WITHOUT_QT_AT_ALL
    retVal=1; // TODO_SIM_WITHOUT_QT_AT_ALL
#else
    retVal=QThread::idealThreadCount();
#endif
    if (retVal<1)
        retVal=1;
    return(retVal);
}

bool VThread::isSimulationMainThreadIdSet()
{
    return(_simulationMainThreadIdSet);
}

void VThread::setSimulationMainThreadId()
{
    _simulationMainThreadId=getCurrentThreadId();
    _simulationMainThreadIdSet=true;
}

void VThread::unsetSimulationMainThreadId()
{
    _simulationMainThreadIdSet=false;
}

bool VThread::isUiThreadIdSet()
{
    return(_uiThreadIdSet);
}

void VThread::setUiThreadId()
{
    _uiThreadId=getCurrentThreadId();
    _uiThreadIdSet=true;
}

void VThread::unsetUiThreadId()
{
    _uiThreadIdSet=false;
}

bool VThread::isCurrentThreadTheUiThread()
{
    if (!_uiThreadIdSet)
        return(true); // the main thread is the UI thread
    return(areThreadIDsSame(_uiThreadId,getCurrentThreadId()));
}


bool VThread::isCurrentThreadTheMainSimulationThread()
{
    if (!_simulationMainThreadIdSet)
        return(false); // the initial thread is the one that will become the UI thread
    return(areThreadIDsSame(_simulationMainThreadId,getCurrentThreadId()));
}

bool VThread::areThreadIDsSame(VTHREAD_ID_TYPE threadA,VTHREAD_ID_TYPE threadB)
{
#ifdef WIN_VREP
    return(threadA==threadB); // normally this is the only thread ID comparison place!! Except for another one in VMutex
#else
    return(pthread_equal(threadA,threadB)!=0); // normally this is the only thread ID comparison place!! Except for another one in VMutex (THREAD_ID_COMPARISON)
#endif
}

VTHREAD_ID_TYPE VThread::getCurrentThreadId()
{
#ifdef WIN_VREP
    return(GetCurrentThreadId());
#else
    return(pthread_self());
#endif
}

void VThread::switchThread()
{
#ifdef WIN_VREP
    SwitchToThread();
#endif
#ifdef MAC_VREP
    pthread_yield_np();
#endif
#ifdef LIN_VREP
    pthread_yield();
#endif
}

void VThread::sleep(int ms)
{
#ifdef WIN_VREP
    Sleep(ms);
#else
    usleep(ms*1000);
#endif
}
