
#pragma once

#include <QMutex>

class VSimUiMutex
{
public:
    enum VSimUiMutexThread
    {
        ui=0,
        nonUi
    };

    VSimUiMutex();
    virtual ~VSimUiMutex();

    void lock(int threadType);
    bool tryLock(int threadType,int timeOut=0);
    void unlock();

private:
    volatile int _lockLevel;
    volatile int _threadType;
    QMutex _cs;
    QMutex _cs_aux;
};

