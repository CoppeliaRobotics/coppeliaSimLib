#include <vThread.h>

#ifdef WIN_SIM
    #include <Windows.h>
    #include <process.h>
#endif
#include <wThread.h>

VMutex VThread::_lock;
bool VThread::_simThreadSet=false;
bool VThread::_uiThreadSet=false;
VTHREAD_ID_TYPE VThread::_simThreadId=VTHREAD_ID_DEAD;
VTHREAD_ID_TYPE VThread::_uiThreadId=VTHREAD_ID_DEAD;
std::vector<VTHREAD_ID_TYPE> VThread::_apiQueriedThreadIds;

void VThread::launchThread(VTHREAD_START_ADDRESS startAddress)
{
#ifdef WIN_SIM
    uintptr_t theWinThread=_beginthread(startAddress,0,0);
#else
    pthread_t th;
    pthread_create(&th,nullptr,startAddress,nullptr);
#endif
}

void VThread::endThread()
{
#ifndef WIN_SIM
    pthread_detach(pthread_self());
#endif
}

void VThread::launchQtThread(SIMPLE_VTHREAD_START_ADDRESS startAddress)
{
    _lock.lock_simple("VThread::launchQtThread");
    Thread* it=new Thread();
    Thread::startAddress=startAddress;
    Thread::startAddressIsFree=false;
    it->start(QThread::NormalPriority);
    while (!Thread::startAddressIsFree)
        QThread::yieldCurrentThread();
    _lock.unlock_simple();
}

void VThread::endQtThread()
{
}

int VThread::getThreadId_apiQueried()
{ // this is an artificial ID, just needed externally. 0=GUI thread, 1=main sim thread.
    if ((_apiQueriedThreadIds.size()==0)&&_simThreadSet&&_uiThreadSet)
    {
        _apiQueriedThreadIds.push_back(_uiThreadId);
        _apiQueriedThreadIds.push_back(_simThreadId);
    }
    if (_apiQueriedThreadIds.size()==0)
        return(-1);
    VTHREAD_ID_TYPE t=getCurrentThreadId();
    for (int i=0;i<int(_apiQueriedThreadIds.size());i++)
    {
        if (areThreadIdsSame(_apiQueriedThreadIds[i],t))
            return(i);
    }
    _apiQueriedThreadIds.push_back(t);
    return(int(_apiQueriedThreadIds.size())-1);
}

int VThread::getCoreCount()
{
    int retVal=QThread::idealThreadCount();
    if (retVal<1)
        retVal=1;
    return(retVal);
}

void VThread::setSimThread()
{
    _simThreadId=getCurrentThreadId();
    _simThreadSet=true;
}

void VThread::unsetSimThread()
{
    _simThreadSet=false;
}

bool VThread::isUiThreadSet()
{
    return(_uiThreadSet);
}

void VThread::setUiThread()
{
    _uiThreadId=getCurrentThreadId();
    _uiThreadSet=true;
}

void VThread::unsetUiThread()
{
    _uiThreadSet=false;
}

bool VThread::isUiThread()
{
#ifdef SIM_WITH_GUI
    if (!_uiThreadSet)
        return(true); // the main thread is the UI thread
    return(areThreadIdsSame(_uiThreadId,getCurrentThreadId()));
#else
    return(false);
#endif
}

bool VThread::isSimThread()
{
    if (!_simThreadSet)
        return(false); // the initial thread is the one that will become the UI thread
    return(areThreadIdsSame(_simThreadId,getCurrentThreadId()));
}

bool VThread::areThreadIdsSame(VTHREAD_ID_TYPE threadA,VTHREAD_ID_TYPE threadB)
{
#ifdef WIN_SIM
    return(threadA==threadB); // normally this is the only thread ID comparison place!! Except for another one in VMutex
#else
    return(pthread_equal(threadA,threadB)!=0); // normally this is the only thread ID comparison place!! Except for another one in VMutex (THREAD_ID_COMPARISON)
#endif
}

VTHREAD_ID_TYPE VThread::getCurrentThreadId()
{
#ifdef WIN_SIM
    return(GetCurrentThreadId());
#else
    return(pthread_self());
#endif
}

void VThread::switchThread()
{
#ifdef WIN_SIM
    SwitchToThread();
#endif
#ifdef MAC_SIM
    pthread_yield_np();
#endif
#ifdef LIN_SIM
    pthread_yield();
#endif
}

void VThread::sleep(int ms)
{
#ifdef WIN_SIM
    Sleep(ms);
#else
    if (ms==0)
        sched_yield();
    else
    {
        struct timespec ts;
        ts.tv_sec=ms/1000;
        ts.tv_nsec=(ms%1000)*1000000;
        nanosleep(&ts, NULL);
    }
#endif
}
