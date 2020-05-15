#include "dummy.h"
#include "app.h"

int CDummy::cnt=0;

CDummy::CDummy()
{
    cnt++;
    std::string tmp("dummy created (total: ");
    tmp+=std::to_string(cnt)+")";
    simAddLog("Sync",sim_verbosity_debug,tmp.c_str());
}

CDummy::~CDummy()
{
    cnt--;
    std::string tmp("dummy destroyed (total: ");
    tmp+=std::to_string(cnt)+")";
    simAddLog("Sync",sim_verbosity_debug,tmp.c_str());
}

