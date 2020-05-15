#include "ikGroup.h"
#include "app.h"

int CIkGroup::cnt=0;

CIkGroup::CIkGroup()
{
    cnt++;
    std::string tmp("IK group created (total: ");
    tmp+=std::to_string(cnt)+")";
    simAddLog("Sync",sim_verbosity_debug,tmp.c_str());
}

CIkGroup::~CIkGroup()
{
    cnt--;
    std::string tmp("IK group destroyed (total: ");
    tmp+=std::to_string(cnt)+")";
    simAddLog("Sync",sim_verbosity_debug,tmp.c_str());
}
