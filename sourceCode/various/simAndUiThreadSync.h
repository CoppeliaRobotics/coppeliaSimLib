
#pragma once

#include "vThread.h"
#include "vSimUiMutex.h"

class CSimAndUiThreadSync
{
public:
    CSimAndUiThreadSync(const char* functionName);
    virtual ~CSimAndUiThreadSync();

    bool uiThread_tryToLockForUiEventRead(int maxTime);
    bool uiThread_tryToLockForUiEventWrite(int maxTime);

    void simThread_lockForSimThreadWrite();
    void simThread_temporarilyAllowUiThreadToReadAndWrite();

    bool simOrUiThread_tryToLockForWrite_cApi();
    bool simOrUiThread_tryToLockForRead_cApi();

    int getObjectHandle();

    static bool simThread_forbidUiThreadToWrite(bool blocking);
    static void simThread_allowUiThreadToWrite();

    static void outputDebugMessage(const char* callLocation,const char* msg);
    static void outputNakedDebugMessage(const char* msg);

    static bool getShowActivityInConsole();
    static void setShowActivityInConsole(bool show);

    static bool getShowLockFailsActivityInConsole();
    static void setShowLockFailsActivityInConsole(bool show);

    static bool hasUiLockedResourcesForReadOrWrite();

private:

    static std::string _getLevelsString(const char* abr);

    int _lockFunctionResult; // -1=not yet tried to lock, 0=tried to lock but failed, 1=tried to lock and succeeded
    std::string _functionName;
    int _lockType; // 0=UI READ, 1=UI WRITE, 2=SIM THREAD WRITE, 3=SIM THREAD ALLOWS OVERRIDING READ OR WRITE BAN, 4=UI THREAD GRANTS SPECIAL READ/WRITE RIGHTS
    int _handle;

    static VSimUiMutex _uiReadPermission; // shared between the SIM and UI thread. Locked by UI when UI reads.
    static VSimUiMutex _uiWritePermission; // shared between the SIM and UI thread. Locked by UI when UI writes

    static int _ui_readLevel;
    static int _ui_writeLevel;
    static int _sim_readLevel;
    static int _sim_writeLevel;

    static int _sim_readLevel_temp;
    static int _sim_writeLevel_temp;

    static volatile bool _ui_writeRequest;

    static bool _showActivityInConsole;
    static bool _showLockFailsInConsole;

    static int _nextHandleValue;

    static bool _processAllMessages_tweak3;
};
