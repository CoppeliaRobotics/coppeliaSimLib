#pragma once

#include <string>

#ifdef SIM_WITH_QT
    #include <QMutex>
    #include <QWaitCondition>
    typedef QMutex WMutex;
    typedef QWaitCondition WWaitCondition;
#else
    #include <vector>
    #ifdef WIN_SIM
        #include <Windows.h>
        typedef CRITICAL_SECTION WMutex;
        typedef std::vector<int> WWaitCondition;
    #else // WIN_SIM
        #include <pthread.h>
        typedef pthread_mutex_t WMutex;
        typedef pthread_cond_t WWaitCondition;
    #endif // WIN_SIM
#endif

class VMutex
{
public:
    VMutex();
    virtual ~VMutex();

    // When using recursive mutexes:
    void lock(const char* location=nullptr);
    bool tryLock();
    void unlock();

    // When using non-recursive mutexes:
    void lock_simple(const char* location);
    bool tryLock_simple();
    void unlock_simple();

    // Wait conditions:
    void wait_simple();
    void wakeAll_simple();

private:
#ifndef SIM_WITH_QT
    bool _areThreadIDsSame(VTHREAD_ID_TYPE threadA,VTHREAD_ID_TYPE threadB);
    VTHREAD_ID_TYPE _getCurrentThreadId();
    void _switchThread();
    void __sl(WMutex mutex);
    void __su(WMutex mutex);

    int _lockLevel;
    VTHREAD_ID_TYPE _lockThreadId;
#ifdef WIN_SIM
    int _nextWaitConditionId;
#endif // WIN_SIM
#endif

    WMutex _recursiveMutex;
    WMutex _simpleMutex;
    WWaitCondition _simpleWaitCondition;
    std::string _location;
};

