
#include "vrepMainHeader.h"
#include "vMutex.h"

#ifdef SIM_WITHOUT_QT_AT_ALL
VMutex::VMutex()
{
    _lockLevel=0;
#ifdef WIN_VREP
    InitializeCriticalSection(&_simpleMutex);
    InitializeCriticalSection(&_recursiveMutex);
    _nextWaitConditionId=0;
#else // WIN_VREP
    pthread_mutex_init(&_simpleMutex,0);
    pthread_mutex_init(&_recursiveMutex,0);
    pthread_cond_init (&_simpleWaitCondition,0);
#endif // WIN_VREP
}
#else // SIM_WITHOUT_QT_AT_ALL
VMutex::VMutex() : _recursiveMutex(QMutex::Recursive), _simpleMutex(QMutex::NonRecursive)
{
}
#endif // SIM_WITHOUT_QT_AT_ALL

VMutex::~VMutex()
{
#ifdef SIM_WITHOUT_QT_AT_ALL
#ifdef WIN_VREP // WIN_VREP
    DeleteCriticalSection(&_simpleMutex);
    DeleteCriticalSection(&_recursiveMutex);
#else // WIN_VREP
    pthread_mutex_destroy(&_simpleMutex);
    pthread_mutex_destroy(&_recursiveMutex);
    pthread_cond_destroy(&_simpleWaitCondition);
#endif // WIN_VREP
#endif // SIM_WITHOUT_QT_AT_ALL
}

// Recursive here:
void VMutex::lock()
{
#ifdef SIM_WITHOUT_QT_AT_ALL
#ifdef WIN_VREP
    EnterCriticalSection(&_recursiveMutex);
#else // WIN_VREP
    __sl(_simpleMutex);
    if ( _areThreadIDsSame(_lockThreadId,_getCurrentThreadId()) && (_lockLevel>0) )
    { // Already locked by this thread
        _lockLevel++;
        __su(_simpleMutex);
        return;
    }
    // first level lock
    __su(_simpleMutex);
    __sl(_recursiveMutex);
    __sl(_simpleMutex);
    _lockThreadId=_getCurrentThreadId();
    _lockLevel=1;
    __su(_simpleMutex);
#endif // WIN_VREP
#else // SIM_WITHOUT_QT_AT_ALL
    _recursiveMutex.lock();
#endif // SIM_WITHOUT_QT_AT_ALL
}

bool VMutex::tryLock()
{
#ifdef SIM_WITHOUT_QT_AT_ALL
#ifdef WIN_VREP
    return(TryEnterCriticalSection(&_recursiveMutex)!=0);
#else // WIN_VREP
    __sl(_simpleMutex);
    if ( _areThreadIDsSame(_lockThreadId,_getCurrentThreadId()) && (_lockLevel>0) )
    { // Already locked by this thread
        _lockLevel++;
        __su(_simpleMutex);
        return(true);
    }
    // trying first level lock
    __su(_simpleMutex);
    bool rv;
    rv=(pthread_mutex_lock(&_recursiveMutex)!=-1);
    if (rv)
    {
        __sl(_simpleMutex);
        _lockThreadId=_getCurrentThreadId();
        _lockLevel=1;
        __su(_simpleMutex);
        return(true);
    }
    else
        return(false);
#endif // WIN_VREP
#else // SIM_WITHOUT_QT_AT_ALL
    return(_recursiveMutex.tryLock(0));
#endif // SIM_WITHOUT_QT_AT_ALL
}

void VMutex::unlock()
{
#ifdef SIM_WITHOUT_QT_AT_ALL
#ifdef WIN_VREP
    LeaveCriticalSection(&_recursiveMutex);
#else // WIN_VREP
    __sl(_simpleMutex);
    _lockLevel--;
    if (_lockLevel==0)
    {
        __su(_simpleMutex);
        __su(_recursiveMutex);
    }
    else
        __su(_simpleMutex);
#endif // WIN_VREP
#else // SIM_WITHOUT_QT_AT_ALL
    _recursiveMutex.unlock();
#endif // SIM_WITHOUT_QT_AT_ALL
}


// Non-recursive here:
void VMutex::lock_simple()
{
#ifdef SIM_WITHOUT_QT_AT_ALL
    __sl(_simpleMutex);
#else // SIM_WITHOUT_QT_AT_ALL
    _simpleMutex.lock();
#endif // SIM_WITHOUT_QT_AT_ALL
}

bool VMutex::tryLock_simple()
{
#ifdef SIM_WITHOUT_QT_AT_ALL
#ifdef WIN_VREP
    return(TryEnterCriticalSection(&_simpleMutex)!=0);
//  return(WaitForSingleObject(_simpleMutex,0)==WAIT_OBJECT_0);
#else // WIN_VREP
    return(pthread_mutex_lock(&_simpleMutex)!=-1);
#endif // WIN_VREP
#else // SIM_WITHOUT_QT_AT_ALL
    return(_simpleMutex.tryLock(0));
#endif // SIM_WITHOUT_QT_AT_ALL
}

void VMutex::unlock_simple()
{
#ifdef SIM_WITHOUT_QT_AT_ALL
    __su(_simpleMutex);
#else // SIM_WITHOUT_QT_AT_ALL
    _simpleMutex.unlock();
#endif // SIM_WITHOUT_QT_AT_ALL
}

void VMutex::wait_simple()
{ // make sure lock_simple was called before!
#ifdef SIM_WITHOUT_QT_AT_ALL
#ifdef WIN_VREP
    // TODO_SIM_WITHOUT_QT_AT_ALL
    // This routine works, but should be rewritten with OS-specific mechanisms
    int id=_nextWaitConditionId++;
    int nid=_nextWaitConditionId; // nid just to avoid having to always lock and go through the vector
    _simpleWaitCondition.push_back(id);
    __su(_simpleMutex);
    while (true)
    {
        while (nid==_nextWaitConditionId)
            _switchThread();
        // something has changed
        bool leave=true;
        __sl(_simpleMutex);
        for (size_t i=0;i<_simpleWaitCondition.size();i++)
        {
            if (_simpleWaitCondition[i]==id)
            { // this wait condition is still active
                leave=false;
                break;
            }
        }
        nid=_nextWaitConditionId;
        if (leave)
            break;
        else
            __su(_simpleMutex);
    }
#else // WIN_VREP
    pthread_cond_wait(&_simpleWaitCondition,&_simpleMutex);
#endif // WIN_VREP
#else // SIM_WITHOUT_QT_AT_ALL
    _simpleWaitCondition.wait(&_simpleMutex);
#endif // SIM_WITHOUT_QT_AT_ALL
}

void VMutex::wakeAll_simple()
{ // make sure lock_simple was called before!
#ifdef SIM_WITHOUT_QT_AT_ALL
#ifdef WIN_VREP
    // TODO_SIM_WITHOUT_QT_AT_ALL
    // This routine works, but should be rewritten with OS-specific mechanisms
    _nextWaitConditionId++;
    _simpleWaitCondition.clear();
#else // WIN_VREP
    pthread_cond_broadcast(&_simpleWaitCondition);
#endif // WIN_VREP
#else // SIM_WITHOUT_QT_AT_ALL
    _simpleWaitCondition.wakeAll();
#endif // SIM_WITHOUT_QT_AT_ALL
}

#ifdef SIM_WITHOUT_QT_AT_ALL
bool VMutex::_areThreadIDsSame(VTHREAD_ID_TYPE threadA,VTHREAD_ID_TYPE threadB)
{
#ifdef WIN_VREP
    return(threadA==threadB);
#else // WIN_VREP
    return(pthread_equal(threadA,threadB)!=0);
#endif // WIN_VREP
}

VTHREAD_ID_TYPE VMutex::_getCurrentThreadId()
{
#ifdef WIN_VREP
    return(GetCurrentThreadId());
#else // WIN_VREP
    return(pthread_self());
#endif // WIN_VREP
}

void VMutex::_switchThread()
{
#ifdef WIN_VREP
    SwitchToThread();
#endif // WIN_VREP
#ifdef MAC_VREP
    pthread_yield_np();
#endif // MAC_VREP
#ifdef LIN_VREP
    pthread_yield();
#endif // LIN_VREP
}

void VMutex::__sl(WMutex mutex)
{
#ifdef WIN_VREP
    EnterCriticalSection(&mutex);
#else // WIN_VREP
    while (pthread_mutex_lock(&mutex)==-1)
        _switchThread();
#endif // WIN_VREP
}

void VMutex::__su(WMutex mutex)
{
#ifdef WIN_VREP
    LeaveCriticalSection(&mutex);
#else // WIN_VREP
    pthread_mutex_unlock(&mutex);
#endif // WIN_VREP
}
#endif // SIM_WITHOUT_QT_AT_ALL
