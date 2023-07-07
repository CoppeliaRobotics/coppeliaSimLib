#pragma once

#include <string>

#include <QMutex>
#include <QWaitCondition>
typedef QMutex WMutex;
typedef QWaitCondition WWaitCondition;

class VMutex
{
public:
    VMutex();
    virtual ~VMutex();

    void setName(const char* name);

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
    void _msg(const char* location,const char* info) const;

    WMutex _recursiveMutex;
    WMutex _simpleMutex;
    WWaitCondition _simpleWaitCondition;
    std::string _location;
    std::string _name;
};

