#include "collection.h"
#include "app.h"

int CCollection::cnt=0;

CCollection::CCollection()
{
    cnt++;
    std::string tmp("collection created (total: ");
    tmp+=std::to_string(cnt)+")";
    simAddLog("Sync",sim_verbosity_debug,tmp.c_str());
}

CCollection::~CCollection()
{
    cnt--;
    std::string tmp("collection destroyed (total: ");
    tmp+=std::to_string(cnt)+")";
    simAddLog("Sync",sim_verbosity_debug,tmp.c_str());
}
