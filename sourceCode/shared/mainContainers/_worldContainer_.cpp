#include <_worldContainer_.h>
#include <simConst.h>

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
