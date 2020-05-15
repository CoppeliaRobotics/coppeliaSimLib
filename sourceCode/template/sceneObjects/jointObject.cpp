#include "jointObject.h"
#include "app.h"

int CJoint::cnt=0;

CJoint::CJoint()
{
    cnt++;
    std::string tmp("joint created (total: ");
    tmp+=std::to_string(cnt)+")";
    simAddLog("Sync",sim_verbosity_debug,tmp.c_str());
}

CJoint::CJoint(int jointType)
{
    _jointType=jointType;
    cnt++;
    std::string tmp("dummy created (total: ");
    tmp+=std::to_string(cnt)+")";
    simAddLog("Sync",sim_verbosity_debug,tmp.c_str());
}

CJoint::~CJoint()
{
    cnt--;
    std::string tmp("dummy destroyed (total: ");
    tmp+=std::to_string(cnt)+")";
    simAddLog("Sync",sim_verbosity_debug,tmp.c_str());
}
