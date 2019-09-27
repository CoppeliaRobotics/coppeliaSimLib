// Adapted code courtesy of "Tibo" from here: https://stackoverflow.com/questions/7581343/how-to-catch-ctrlc-on-windows-and-linux-with-qt

#pragma once

class SignalHandler
{
public:
    SignalHandler(int mask);
    virtual ~SignalHandler();

    enum SIGNALS
    {
        SIG_UNHANDLED   = 0,    // Physical signal not supported by this class
        SIG_NOOP        = 1,    // The application is requested to do a no-op (only a target that platform-specific signals map to when they can't be raised anyway)
        SIG_INT         = 2,    // Control+C (should terminate but consider that it's a normal way to do so; can delay a bit)
        SIG_TERM        = 4,    // Control+Break (should terminate now without regarding the consquences)
        SIG_CLOSE       = 8,    // Container window closed (should perform normal termination, like Ctrl^C) [Windows only; on Linux it maps to SIG_TERM]
        SIG_RELOAD      = 16,   // Reload the configuration [Linux only, physical signal is SIGHUP; on Windows it maps to SIG_NOOP]
        DEFAULT_SIGNALS = SIG_INT | SIG_TERM | SIG_CLOSE,
    };
    static const int numSignals = 6;

//    static bool handleSignal(int signal);

private:
    int _mask;
};
