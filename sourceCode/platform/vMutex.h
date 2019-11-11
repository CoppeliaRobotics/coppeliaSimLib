#pragma once

#include <string>

#ifndef SIM_WITHOUT_QT_AT_ALL
    #include <QMutex>
    #include <QWaitCondition>
    typedef QMutex WMutex;
    typedef QWaitCondition WWaitCondition;
#else // SIM_WITHOUT_QT_AT_ALL
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
#endif // SIM_WITHOUT_QT_AT_ALL

class VMutex
{
public:
    VMutex();
    virtual ~VMutex();

    // When using recursive mutexes:
    void lock(const char* location);
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
#ifdef SIM_WITHOUT_QT_AT_ALL
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
#endif // SIM_WITHOUT_QT_AT_ALL

    WMutex _recursiveMutex;
    WMutex _simpleMutex;
    WWaitCondition _simpleWaitCondition;
    std::string _location;
};

