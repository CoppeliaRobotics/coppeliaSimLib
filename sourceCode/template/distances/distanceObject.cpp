#include "distanceObject.h"
#include "app.h"

int CDistanceObject::cnt=0;

CDistanceObject::CDistanceObject(int entity1Handle,int entity2Handle)
{
    _entity1Handle=entity1Handle;
    _entity2Handle=entity2Handle;
    cnt++;
    std::string tmp("distance object created (total: ");
    tmp+=std::to_string(cnt)+")";
    simAddLog("Sync",sim_verbosity_debug,tmp.c_str());
}

CDistanceObject::~CDistanceObject()
{
    cnt--;
    std::string tmp("distance object destroyed (total: ");
    tmp+=std::to_string(cnt)+")";
    simAddLog("Sync",sim_verbosity_debug,tmp.c_str());
}

