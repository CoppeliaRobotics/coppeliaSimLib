#include "worldContainer.h"
#include "app.h"

CWorldContainer::CWorldContainer()
{
    currentWorld=new CWorld();
    currentWorld->initializeWorld();
}

CWorldContainer::~CWorldContainer()
{
    currentWorld->deleteWorld();
    delete currentWorld;
}

void CWorldContainer::synchronizationMsg(std::vector<SSyncRoute>& routing,const SSyncMsg& msg)
{ // Overridden from _CWorldContainer_
    std::string tmp("synchronization message: msg ");
    tmp+=std::to_string(msg.msg);
    tmp+=" (routing size: ";
    tmp+=std::to_string(routing.size());
    tmp+="):";
    for (size_t i=0;i<routing.size();i++)
    {
        tmp+="\n    objectType: ";
        tmp+=std::to_string(routing[i].objType);
        tmp+=", handle: ";
        tmp+=std::to_string(routing[i].objHandle);
    }
    simAddLog("Sync",sim_verbosity_debug,tmp.c_str());
    _CWorldContainer_::synchronizationMsg(routing,msg);
}
