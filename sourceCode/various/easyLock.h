#pragma once

#include <string>

class VMutex; // forward declaration

class CEasyLock  
{
public:
    CEasyLock(VMutex& mutex,const char* functionName); // locks during construction
    virtual ~CEasyLock();
    void unlock();
    void lock();

    static void setShowActivity(bool show);
    static bool getShowActivity();
private:
    void _unlock();
    void _lock();

    static bool _showActivity;
    bool _wasAlreadyUnlocked;
    VMutex* _mutex;
    std::string _functionName;
};
