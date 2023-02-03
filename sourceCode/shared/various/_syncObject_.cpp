#include <_syncObject_.h>
#ifdef SIM_LIB
#include <app.h>
#endif

bool _CSyncObject_::_overallSyncEnabled=true;

_CSyncObject_::_CSyncObject_()
{
    _objectCanSync=false;

    _rt.objHandles[0]=-1;
    _rt.objTypes[0]=255;
    _rt.objHandles[1]=-1;
    _rt.objTypes[1]=255;
    _rt.objHandles[2]=-1;
    _rt.objTypes[2]=255;
}

_CSyncObject_::~_CSyncObject_()
{
}

void _CSyncObject_::setSyncMsgRouting(const std::vector<SSyncRoute>* ancestorRoute,const SSyncRoute& ownIdentity)
{
    if (ancestorRoute!=nullptr)
        _routing.assign(ancestorRoute->begin(),ancestorRoute->end());
    else
        _routing.clear();
    _routing.push_back(ownIdentity);
    for (size_t i=0;i<_routing.size();i++)
    {
        _rt.objHandles[i]=_routing[i].objHandle;
        _rt.objTypes[i]=_routing[i].objType;
    }
}

const std::vector<SSyncRoute>* _CSyncObject_::getSyncMsgRouting() const
{
    return(&_routing);
}

bool _CSyncObject_::isRoutingSet() const
{
    return(_routing.size()>0);
}

bool _CSyncObject_::getObjectCanSync() const
{
    /*
    if (_objectCanSync)
    {
        if (!VThread::isCurrentThreadNotTheUiThreadOrUiThreadNotYetSet())
            App::logMsg(sim_verbosity_errors,"getObjectCanSync() was called from the UI thread.");
    }
    */
    return(_objectCanSync); // slave can sync when not incoming msg underway
}

bool _CSyncObject_::setObjectCanSync(bool s)
{
    bool retVal=true;
    if (s&&(!_overallSyncEnabled))
        retVal=false;
    else
    {
        _objectCanSync=s;
        retVal=s;
    }
    return(retVal);
}

void _CSyncObject_::setOverallSyncEnabled(bool e)
{ // static function
    _overallSyncEnabled=e;
}

bool _CSyncObject_::getOverallSyncEnabled()
{ // static function
    return(_overallSyncEnabled);
}
