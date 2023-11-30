#include <vMutex.h>
#include <vThread.h>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
VMutex::VMutex() : _recursiveMutex(QMutex::Recursive), _simpleMutex(QMutex::NonRecursive)
#else
VMutex::VMutex()
#endif
{
}

VMutex::~VMutex()
{
}

void VMutex::setName(const char *name)
{
    _name = name;
}

void VMutex::_msg(const char *location, const char *info) const
{
    std::string loc("unknown");
    if (location != nullptr)
        loc = location;
    if (VThread::isSimThread())
        printf("SIM thread: VMutex '%s', location '%s': %s\n", _name.c_str(), loc.c_str(), info);
    else
        printf("GUI thread: VMutex '%s', location '%s': %s\n", _name.c_str(), loc.c_str(), info);
}

// Recursive here:
void VMutex::lock(const char *location /*=nullptr*/)
{
    if (_name.size() > 0)
    {
        _msg(location, "locking...");
        if (location != nullptr)
            _location = location;
    }
    else
        _location.clear();
    _recursiveMutex.lock();
    if (_name.size() > 0)
        _msg(location, "locked.");
}

bool VMutex::tryLock()
{
    return (_recursiveMutex.tryLock(0));
}

void VMutex::unlock()
{
    _recursiveMutex.unlock();
    if (_name.size() > 0)
    {
        _msg(_location.c_str(), "unlocked.");
        _location.clear();
    }
}

// Non-recursive here:
void VMutex::lock_simple(const char *location)
{
    _simpleMutex.lock();
}

bool VMutex::tryLock_simple()
{
    return (_simpleMutex.tryLock(0));
}

void VMutex::unlock_simple()
{
    _simpleMutex.unlock();
}

void VMutex::wait_simple()
{ // make sure lock_simple was called before!
    _simpleWaitCondition.wait(&_simpleMutex);
}

void VMutex::wakeAll_simple()
{ // make sure lock_simple was called before!
    _simpleWaitCondition.wakeAll();
}
