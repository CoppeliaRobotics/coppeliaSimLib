#include "ikElement.h"
#include "app.h"

int CIkElement::cnt=0;

CIkElement::CIkElement(int tooltip)
{
    _tipHandle=tooltip;
    cnt++;
    std::string tmp("IK element created (total: ");
    tmp+=std::to_string(cnt)+")";
    simAddLog("Sync",sim_verbosity_debug,tmp.c_str());
}

CIkElement::~CIkElement()
{
    cnt--;
    std::string tmp("IK element destroyed (total: ");
    tmp+=std::to_string(cnt)+")";
    simAddLog("Sync",sim_verbosity_debug,tmp.c_str());
}
