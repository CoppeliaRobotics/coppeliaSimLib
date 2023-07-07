#pragma once

#include <vMutex.h>
#include <QTime>
#include <QThread>
#ifndef WIN_SIM
    #include <unistd.h>
    #include <pthread.h>
#endif

// FULLY STATIC CLASS
class VThread  
{
public:
    static void launchThread(VTHREAD_START_ADDRESS startAddress);
    static void endThread();
    static void launchQtThread(SIMPLE_VTHREAD_START_ADDRESS startAddress);
    static void endQtThread();
    static int getThreadId_apiQueried();
    static int getCoreCount();
    static void setSimThread();
    static void unsetSimThread();
    static bool isUiThreadSet();
    static void setUiThread();
    static void unsetUiThread();
    static bool isSimThread();
    static bool isUiThread();
    static bool areThreadIdsSame(VTHREAD_ID_TYPE threadA,VTHREAD_ID_TYPE threadB);
    static VTHREAD_ID_TYPE getCurrentThreadId();
    static void switchThread();
    static void sleep(int ms);

private:
    static bool _simThreadSet;
    static bool _uiThreadSet;
    static VTHREAD_ID_TYPE _simThreadId;
    static VTHREAD_ID_TYPE _uiThreadId;
    static VMutex _lock;
    static std::vector<VTHREAD_ID_TYPE> _apiQueriedThreadIds;
};

