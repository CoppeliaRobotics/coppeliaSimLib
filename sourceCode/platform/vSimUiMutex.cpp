
#include "vrepMainHeader.h"
#include "vSimUiMutex.h"

VSimUiMutex::VSimUiMutex()
{
    _lockLevel=0;
}

VSimUiMutex::~VSimUiMutex()
{
}

void VSimUiMutex::lock(int threadType)
{
    tryLock(threadType,-1); // negative time out --> no time out!
}

bool VSimUiMutex::tryLock(int threadType,int timeOut/*=0*/)
{ // if timeOut is negative, there is no time out!
    bool retVal=false;

    _cs_aux.lock();

    bool doTheRealLock=false;
    if (_lockLevel==0)
    { // not locked
        doTheRealLock=true;
    }
    else
    {
        if (threadType==_threadType)
        { // already locked by self
            _lockLevel++;
            retVal=true;
        }
        else
        { // already locked by other
            doTheRealLock=true;
        }
    }

    if (doTheRealLock)
    {
        _cs_aux.unlock();
        retVal=_cs.tryLock(timeOut);
        _cs_aux.lock();
        if (retVal)
        {
            _lockLevel=1;
            _threadType=threadType;
        }
    }

    _cs_aux.unlock();

    return(retVal);
}

void VSimUiMutex::unlock()
{
    _cs_aux.lock();
    _lockLevel--;
    if (_lockLevel==0)
        _cs.unlock();
    _cs_aux.unlock();
}

