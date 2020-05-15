#include "_worldContainer_.h"
#include "simConst.h"

_CWorldContainer_::_CWorldContainer_()
{
    currentWorld=nullptr;

    SSyncRoute rt;
    rt.objHandle=-1;
    rt.objType=sim_syncobj_worldcont;
    setSyncMsgRouting(nullptr,rt);
    setObjectCanSync(true);
}

_CWorldContainer_::~_CWorldContainer_()
{
}

void _CWorldContainer_::synchronizationMsg(std::vector<SSyncRoute>& routing,const SSyncMsg& msg)
{ // Overridden from _CSyncObject_
    CSyncObject::setOverallSyncEnabled(false);
    currentWorld->synchronizationMsg(routing,msg);
    CSyncObject::setOverallSyncEnabled(true);
}

