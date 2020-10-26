// Adapted code courtesy of "Tibo" from here: https://stackoverflow.com/questions/7581343/how-to-catch-ctrlc-on-windows-and-linux-with-qt

#include "sigHandler.h"
#include <iostream>
#include <set>
#include "app.h"

#ifndef _WIN32
    #include <signal.h>
#else
    #include <windows.h>
#endif //!_WIN32

// There can be only ONE SignalHandler per process
//SignalHandler* g_handler(nullptr);
#ifdef _WIN32
    BOOL WINAPI WIN32_handleFunc(DWORD);
    int WIN32_physicalToLogical(DWORD);
    DWORD WIN32_logicalToPhysical(int);
    std::set<int> g_registry;
#else //_WIN32
    void POSIX_handleFunc(int);
    int POSIX_physicalToLogical(int);
    int POSIX_logicalToPhysical(int);
#endif //_WIN32
bool handleSignal(int signal);

SignalHandler::SignalHandler(int mask) : _mask(mask), _restoremask(0)
{
#ifdef _WIN32
    SetConsoleCtrlHandler(WIN32_handleFunc, TRUE);
#endif //_WIN32

    for (int i=0;i<numSignals;i++)
    {
        int logical = 0x1 << i;
        if (_mask & logical)
        {
#ifdef _WIN32
            g_registry.insert(logical);
#else
            int sig = POSIX_logicalToPhysical(logical);
#ifdef LEGACY_POSIX_SIGNAL
            bool failed = signal(sig, POSIX_handleFunc) == SIG_ERR;
            (void)failed; // Silence the warning in non _DEBUG; TODO: something better
            _restoremask |= logical;
#else
            struct sigaction sa, sa_old;
            std::memset(&sa, 0, sizeof(sa));
            sa.sa_handler = POSIX_handleFunc;
            sigfillset(&sa.sa_mask);
            sigaction(sig, NULL, &sa_old);
            if (sa_old.sa_handler == SIG_DFL)
            {
                sigaction(sig, &sa, NULL);
                _restoremask |= logical;
            }
#endif
#endif //_WIN32
        }
    }
}

SignalHandler::~SignalHandler()
{
#ifdef _WIN32
    SetConsoleCtrlHandler(WIN32_handleFunc, FALSE);
#else
    for (int i=0;i<numSignals;i++)
    {
        int logical = 0x1 << i;
        if (_restoremask & logical)
        {
            int sig = POSIX_logicalToPhysical(logical);
#ifdef LEGACY_POSIX_SIGNAL
            signal(sig, SIG_DFL);
#else
            struct sigaction sa;
            std::memset(&sa, 0, sizeof(sa));
            sa.sa_handler = SIG_DFL;
            sigfillset(&sa.sa_mask);
            sigaction(sig, &sa, NULL);
#endif
        }
    }
#endif //_WIN32
}

#ifdef _WIN32
DWORD WIN32_logicalToPhysical(int signal)
{
    switch (signal)
    {
    case SignalHandler::SIG_INT: return CTRL_C_EVENT;
    case SignalHandler::SIG_TERM: return CTRL_BREAK_EVENT;
    case SignalHandler::SIG_CLOSE: return CTRL_CLOSE_EVENT;
    default:
        return ~(unsigned int)0; // SIG_ERR = -1
    }
}
#else
int POSIX_logicalToPhysical(int signal)
{
    switch (signal)
    {
    case SignalHandler::SIG_INT: return SIGINT;
    case SignalHandler::SIG_TERM: return SIGTERM;
    // In case the client asks for a SIG_CLOSE handler, accept and
    // bind it to a SIGTERM. Anyway the signal will never be raised
    case SignalHandler::SIG_CLOSE: return SIGTERM;
    case SignalHandler::SIG_RELOAD: return SIGHUP;
    default:
        return -1; // SIG_ERR = -1
    }
}
#endif //_WIN32

#ifdef _WIN32
int WIN32_physicalToLogical(DWORD signal)
{
    switch (signal)
    {
    case CTRL_C_EVENT: return SignalHandler::SIG_INT;
    case CTRL_BREAK_EVENT: return SignalHandler::SIG_TERM;
    case CTRL_CLOSE_EVENT: return SignalHandler::SIG_CLOSE;
    default:
        return SignalHandler::SIG_UNHANDLED;
    }
}
#else
int POSIX_physicalToLogical(int signal)
{
    switch (signal)
    {
    case SIGINT: return SignalHandler::SIG_INT;
    case SIGTERM: return SignalHandler::SIG_TERM;
    case SIGHUP: return SignalHandler::SIG_RELOAD;
    default:
        return SignalHandler::SIG_UNHANDLED;
    }
}
#endif //_WIN32


#ifdef _WIN32
BOOL WINAPI WIN32_handleFunc(DWORD signal)
{
    int signo = WIN32_physicalToLogical(signal);
    // The std::set is thread-safe in const reading access and we never
    // write to it after the program has started so we don't need to
    // protect this search by a mutex
    std::set<int>::const_iterator found = g_registry.find(signo);
    if (signo != -1 && found != g_registry.end())
    {
        return handleSignal(signo) ? TRUE : FALSE;
    }
    return FALSE;
}
#else
void POSIX_handleFunc(int signal)
{
    int signo = POSIX_physicalToLogical(signal);
    handleSignal(signo);
}
#endif //_WIN32

bool handleSignal(int signal)
{
    App::logMsg(sim_verbosity_loadinfos,"external exit request: %i",signal);
    SSimulationThreadCommand cmd;
    cmd.cmdId=FINAL_EXIT_REQUEST_CMD;
    App::appendSimulationThreadCommand(cmd);
    return true; // don't propagate the signal further
}
