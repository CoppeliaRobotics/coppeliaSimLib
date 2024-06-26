#pragma once

#include <folderSystem.h>
#include <userSettings.h>
#include <worldContainer.h>
#include <sigHandler.h>
#include <simThread.h>
#include <gm.h>
#include <instance_id.h>
#ifndef SIM_WITH_GUI
#include <simQApp.h>
#endif

class App
{
  public:
    enum
    {
        appstage_none = 0,
        appstage_simInit1Done,
        appstage_guiInit1Done, // set by UI thread. An temp dummy QApplication is created to make sure Qt doesn't
                               // complain about wrong thread
        appstage_simInit2Done, // the SIM thread is ready, except for the simThread object, and sandbox/add-ons
                               // load/initializations
        appstage_guiInit2Done, // set by UI thread. The UI thread is ready
        appstage_simRunning,
        appstage_guiCleanupRequest,
        appstage_guiCleanupDone, // set by UI thread
        appstage_simCleanupDone,
    };

    App();
    virtual ~App();

    static long long int getFreshUniqueId();

    static void beep(int frequ = 5000, int duration = 1000);

    static void init(const char *appDir, int options);
    static void cleanup();
    static void loop(void (*callback)(), bool stepIfRunning);

    static std::string getApplicationDir();
    static void postExitRequest();
    static bool getExitRequest();

    static void appendSimulationThreadCommand(int cmdId, int intP1 = -1, int intP2 = -1, double floatP1 = 0.0,
                                              double floatP2 = 0.0, const char *stringP1 = nullptr,
                                              const char *stringP2 = nullptr, double executionDelay = 0.0);
    static void appendSimulationThreadCommand(SSimulationThreadCommand cmd, double executionDelay = 0.0);

    static std::string getApplicationArgument(int index);
    static void setApplicationArgument(int index, std::string arg);
    static std::string getApplicationNamedParam(const char *paramName);
    static int setApplicationNamedParam(const char *paramName, const char *param, int paramLength = 0);
    static void setAdditionalAddOnScript1(const char *script);
    static std::string getAdditionalAddOnScript1();
    static void setAdditionalAddOnScript2(const char *script);
    static std::string getAdditionalAddOnScript2();
    static bool assemble(int parentHandle, int childHandle, bool justTest, bool msgs = false);
    static bool disassemble(int objectHandle, bool justTest, bool msgs = false);

    static std::string getConsoleLogFilter();
    static void setConsoleLogFilter(const char *filter);
    static bool logPluginMsg(const char *pluginName, int verbosityLevel, const char *logMsg);
    static void logMsg(int verbosityLevel, const char *msg);
    static void logMsg(int verbosityLevel, const char *msg, const char *subStr1, const char *subStr2 = nullptr,
                       const char *subStr3 = nullptr);
    static void logMsg(int verbosityLevel, const char *msg, int int1, int int2 = 0, int int3 = 0);
    static void logScriptMsg(const CScriptObject *script, int verbosityLevel, const char *msg);
    static int getConsoleVerbosity(const char *pluginName = nullptr);
    static void setConsoleVerbosity(int v, const char *pluginName = nullptr);
    static int getStatusbarVerbosity(const char *pluginName = nullptr);
    static void setStatusbarVerbosity(int v, const char *pluginName = nullptr);
    static bool getConsoleOrStatusbarVerbosityTriggered(int verbosityLevel);
    static int getVerbosityLevelFromString(const char *verbosityStr);
    static bool getConsoleMsgToFile();
    static void setConsoleMsgToFile(bool f);
    static std::string getConsoleMsgFile();
    static void setConsoleMsgFile(const char *f);
    static int getDlgVerbosity();
    static void setDlgVerbosity(int v);
    static void setStartupScriptString(const char *str);
    static void setExitCode(int c);
    static int getExitCode();
    static int getAppStage();
    static void setAppStage(int s);

    static void undoRedo_sceneChanged(const char *txt);
    static void undoRedo_sceneChangedGradual(const char *txt);

    static CFolderSystem *folders;
    static CUserSettings *userSettings;
    static CSimThread *simThread;
    static CWorldContainer *worldContainer;
    static CWorld *currentWorld; // actually worldContainer->currentWorld
    static CGm *gm;
    static std::vector<void*> callbacks;
    static InstancesList* instancesList;

#ifndef SIM_WITH_GUI
    static CSimQApp *qtApp;
#endif

  private:
    static void _simulatorLoop(bool stepIfRunning = true);
    static void _logMsg(const char *originName, int verbosityLevel, const char *msg, const char *subStr1,
                        const char *subStr2 = nullptr, const char *subStr3 = nullptr);
    static void _logMsg(const char *originName, int verbosityLevel, const char *msg, int int1, int int2 = 0,
                        int int3 = 0);
    static void __logMsg(const char *originName, int verbosityLevel, const char *msg, int consoleVerbosity = -1,
                         int statusbarVerbosity = -1);
    static bool _consoleLogFilter(const char *msg);
    static std::string _getHtmlEscapedString(const char *str);
    static bool _consoleMsgsToFile;
    static std::string _consoleMsgsFilename;
    static VFile *_consoleMsgsFile;
    static VArchive *_consoleMsgsArchive;
//    static std::string _instanceId;
//    static int _instanceIndex;

    static long long int _nextUniqueId;
    static SignalHandler *_sigHandler;

    static int _qApp_argc;
    static char _qApp_arg0[];
    static char *_qApp_argv[1];
    static std::vector<std::string> _applicationArguments;
    static std::map<std::string, std::string> _applicationNamedParams;
    static std::string _additionalAddOnScript1;
    static std::string _additionalAddOnScript2;
    static int _consoleVerbosity;
    static int _statusbarVerbosity;
    static int _dlgVerbosity;
    static int _exitCode;
    static bool _exitRequest;
    static volatile int _appStage;
    static std::string _consoleLogFilterStr;
    static std::string _startupScriptString;
    static std::map<std::string /*originName*/, std::map<int /*verbosityLevel*/, std::map<std::string /*msg*/, bool>>> _logOnceMessages;

    static std::string _applicationDir;
};

class CFuncTrace
{
  public:
    CFuncTrace(const char *functionName, int traceVerbosity)
    {
        if (App::getConsoleOrStatusbarVerbosityTriggered(traceVerbosity))
        {
            _txt = functionName;
            _verbosity = traceVerbosity;
            if (_verbosity == sim_verbosity_traceall)
                _txt += " (C)";
            if (_verbosity == sim_verbosity_tracelua)
                _txt += " (Lua API)";
            App::logMsg(_verbosity, (std::string("--> ") + _txt).c_str());
        }
    };
    virtual ~CFuncTrace()
    {
        if (_txt.size() > 0)
            App::logMsg(_verbosity, (std::string("<-- ") + _txt).c_str());
    };

  private:
    std::string _txt;
    int _verbosity;
};
