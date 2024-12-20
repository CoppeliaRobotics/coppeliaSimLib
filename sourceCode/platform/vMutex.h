#pragma once

#include <string>
#include <QMutex>
#include <QWaitCondition>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#else
#include <QRecursiveMutex>
#endif
typedef QMutex WMutex;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#else
typedef QRecursiveMutex WRecursiveMutex;
#endif
typedef QWaitCondition WWaitCondition;

class VMutex
{
  public:
    VMutex();
    virtual ~VMutex();

    void setName(const char* name);

    // When using recursive mutexes:
    void lock(const char* location = nullptr);
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
    void _msg(const char* location, const char* info) const;

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    WMutex _recursiveMutex;
#else
    WRecursiveMutex _recursiveMutex;
#endif
    WMutex _simpleMutex;
    WWaitCondition _simpleWaitCondition;
    std::string _location;
    std::string _name;
};
