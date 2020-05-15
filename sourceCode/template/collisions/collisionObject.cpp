#include "collisionObject.h"
#include "app.h"

int CCollisionObject::cnt=0;

CCollisionObject::CCollisionObject(int entity1Handle,int entity2Handle)
{
    _entity1Handle=entity1Handle;
    _entity2Handle=entity2Handle;
    cnt++;
    std::string tmp("collision object created (total: ");
    tmp+=std::to_string(cnt)+")";
    simAddLog("Sync",sim_verbosity_debug,tmp.c_str());
}

CCollisionObject::~CCollisionObject()
{
    cnt--;
    std::string tmp("collision object destroyed (total: ");
    tmp+=std::to_string(cnt)+")";
    simAddLog("Sync",sim_verbosity_debug,tmp.c_str());
}

