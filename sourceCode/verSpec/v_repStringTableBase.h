
#pragma once

#define IDSN_VREP_CRASH "V-REP Crash"
#ifndef MAC_VREP
    #define IDSN_VREP_CRASH_OR_NEW_INSTANCE_INFO "It seems that V-REP crashed in last session (or you might be running several instances of V-REP in parallel). Would you like to try to recover auto-saved scenes? (see also V-REP's installation folder for backuped scenes)"
#else
    #define IDSN_VREP_CRASH_OR_NEW_INSTANCE_INFO "It seems that V-REP crashed in last session. If V-REP didn't crash, you might be running several instances of V-REP in parallel, or you might have quit V-REP not correctly (<File --> Quit> is correct). Would you like to try to recover auto-saved scenes? (see also V-REP's installation folder for backuped scenes)"
#endif
