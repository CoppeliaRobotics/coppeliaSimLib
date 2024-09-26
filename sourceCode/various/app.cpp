#include <app.h>
#include <vThread.h>
#include <utils.h>
#include <simStrings.h>
#include <vDateTime.h>
#include <pathPlanningInterface.h>
#include <vVarious.h>
#include <persistentDataContainer.h>
#include <apiErrors.h>
#include <mesh.h>
#include <threadPool_old.h>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <fileOperations.h>
#include <QTextDocument>
#include <simFlavor.h>
#include <QCoreApplication>
#ifdef SIM_WITH_GUI
#include <guiApp.h>
#endif

#ifdef WIN_SIM
#include <windows.h>
#include <dbghelp.h>
#else
#include <cxxabi.h>
#include <execinfo.h>
#include <signal.h>
#endif

#ifndef SIM_WITH_GUI
CSimQApp *App::qtApp = nullptr;
int App::_qApp_argc = 1;
char App::_qApp_arg0[] = {"CoppeliaSim"};
char *App::_qApp_argv[1] = {_qApp_arg0};
#endif
CSimThread *App::simThread = nullptr;
CUserSettings *App::userSettings = nullptr;
CFolderSystem *App::folders = nullptr;
CWorldContainer *App::worldContainer = nullptr;
CWorld *App::currentWorld = nullptr;
int App::_consoleVerbosity = sim_verbosity_default;
int App::_statusbarVerbosity = sim_verbosity_msgs;
int App::_dlgVerbosity = sim_verbosity_infos;
int App::_exitCode = 0;
bool App::_exitRequest = false;
bool App::_hierarchyEnabled = false;
bool App::_openGlDisplayEnabled = true;
volatile int App::_appStage = App::appstage_none;
std::string App::_consoleLogFilterStr;
std::string App::_startupScriptString;
std::map<std::string, std::map<int, std::map<std::string, bool>>> App::_logOnceMessages;
long long int App::_nextUniqueId = SIM_UIDSTART;
std::string App::_applicationDir;
std::vector<std::string> App::_applicationArguments;
std::map<std::string, std::string> App::_applicationNamedParams;
std::string App::_additionalAddOnScript1;
std::string App::_additionalAddOnScript2;
bool App::_consoleMsgsToFile = false;
std::string App::_consoleMsgsFilename = "debugLog.txt";
VFile *App::_consoleMsgsFile = nullptr;
VArchive *App::_consoleMsgsArchive = nullptr;
SignalHandler *App::_sigHandler = nullptr;
CGm *App::gm = nullptr;
std::vector<void*> App::callbacks;
InstancesList* App::instancesList = nullptr;


#ifdef WIN_SIM
LONG WINAPI _winExceptionHandler(PEXCEPTION_POINTERS pExceptionInfo)
{
    void *stack[62];
    HANDLE process = GetCurrentProcess();
    SymInitialize(process, 0, TRUE);
    unsigned short fr = CaptureStackBackTrace(0, 62, stack, nullptr);
    SYMBOL_INFO *symb = (SYMBOL_INFO *)calloc(sizeof(SYMBOL_INFO) + 1024 * sizeof(char), 1);
    symb->MaxNameLen = 1023;
    symb->SizeOfStruct = sizeof(SYMBOL_INFO);
    for (size_t i = 0; i < fr; i++)
    {
        SymFromAddr(process, (DWORD64)(stack[i]), 0, symb);
        printf("CoppeliaSim: debug: %zu: %s - 0x%0I64X\n", fr - i - 1, symb->Name, symb->Address);
    }
    free(symb);
    return EXCEPTION_EXECUTE_HANDLER;
}
#else
#define BACKTRACE_SIZE 50
void _segHandler(int sig)
{
    void *callstack[BACKTRACE_SIZE];
    size_t frames = backtrace(callstack, BACKTRACE_SIZE);
    fprintf(stderr, "\n\nError: signal %d:\n\n", sig);
#ifndef DONT_DEMANGLE_BACKTRACE
    char **strs = backtrace_symbols(callstack, frames);
    for (int i = 0; i < frames; ++i) {
        std::string line(strs[i]);
        size_t mangledStart = line.find(
#ifdef MAC_SIM
                " "
#else
                "("
#endif
                "_Z");
        if (mangledStart != std::string::npos) {
            mangledStart++;
            size_t mangledEnd = mangledStart;
            while(mangledEnd < line.length() &&
#ifdef MAC_SIM
                    line.at(mangledEnd) != ' '
#else
                    line.at(mangledEnd) != '+' &&
                    line.at(mangledEnd) != ')'
#endif
                    ) mangledEnd++;
            std::string mangled = line.substr(mangledStart, mangledEnd - mangledStart);
            int status;
            char *demangled = abi::__cxa_demangle(mangled.c_str(), nullptr, nullptr, &status);
            if (status == 0) {
                std::cout << line.substr(0, mangledStart) << demangled << line.substr(mangledEnd) << std::endl;
                free(demangled);
                continue;
            }
            free(demangled);
        }
        std::cout << line << std::endl;
    }
    free(strs);
#else // DONT_DEMANGLE_BACKTRACE
    backtrace_symbols_fd(callstack, frames, STDERR_FILENO);
#endif // DONT_DEMANGLE_BACKTRACE
    exit(1);
}
#endif

int App::getAppStage()
{
    return (_appStage);
}

void App::setAppStage(int s)
{
    _appStage = s;
}

void App::init(const char *appDir, int)
{
    instancesList = new InstancesList();
    CSimFlavor::run(13);
    gm = new CGm();
    if (appDir)
        _applicationDir = appDir;
    else
    {
        QFileInfo pathInfo(QCoreApplication::applicationFilePath());
        _applicationDir = pathInfo.path().toStdString();
    }
    QDir ad(_applicationDir.c_str());
    _applicationDir = ad.absolutePath().toStdString();

    VVarious::removePathFinalSlashOrBackslash(_applicationDir);
    std::string appD("Application directory is ");
    appD += _applicationDir;
    logMsg(sim_verbosity_loadinfos | sim_verbosity_onlyterminal, appD.c_str());
#ifdef WIN_SIM
    timeBeginPeriod(1);
    SetDllDirectoryA(_applicationDir.c_str());
#endif
    setAppStage(appstage_simInit1Done);

#ifdef SIM_WITH_GUI
    while (getAppStage() != appstage_guiInit1Done)
        VThread::sleep(1);
#else
    qtApp = new CSimQApp(_qApp_argc, _qApp_argv);
#endif

    VThread::setSimThread();
    _exitRequest = false;

#ifdef WIN_SIM
    SetUnhandledExceptionFilter(_winExceptionHandler);
#else
    signal(SIGSEGV, _segHandler);
#endif
    _sigHandler = new SignalHandler(SignalHandler::SIG_INT | SignalHandler::SIG_TERM | SignalHandler::SIG_CLOSE);

    userSettings = new CUserSettings();
    folders = new CFolderSystem();

    std::string str("CoppeliaSim v");
    str += SIM_PROGRAM_VERSION;
    str += " ";
    str += SIM_PROGRAM_REVISION;
    str += ", flavor: ";
#ifdef SIM_FL
    str += std::to_string(SIM_FL);
    str += ", ";
#else
    str += "n/a, ";
#endif
    str += SIM_PLATFORM;
    logMsg(sim_verbosity_loadinfos | sim_verbosity_onlyterminal, str.c_str());

    CThreadPool_old::init();
    CSimFlavor::run(0);
    srand((int)VDateTime::getTimeInMs()); // Important so that the computer ID has some "true" random component!
                                          // Remember that each thread starts with a same seed!!!
    worldContainer = new CWorldContainer();
    worldContainer->initialize();
    CFileOperations::createNewScene(false);

    if ((App::getConsoleVerbosity() >= sim_verbosity_trace) && (!App::userSettings->suppressStartupDialogs))
        App::logMsg(sim_verbosity_warnings, "tracing is turned on: this might lead to drastic performance loss.");

    setAppStage(appstage_simInit2Done);
#ifdef SIM_WITH_GUI
    while (getAppStage() != appstage_guiInit2Done)
        VThread::sleep(1);
    CSimAndUiThreadSync::simThread_forbidUiThreadToWrite(true); // lock initially...
#endif
    simThread = new CSimThread();

    // Some items below require the GUI to be initialized (e.g. the Commander plugin):
    worldContainer->sandboxScript = new CScriptObject(sim_scripttype_sandbox);
    worldContainer->sandboxScript->initSandbox();

    if (App::userSettings->runAddOns)
    {
        worldContainer->addOnScriptContainer->loadAllAddOns();
        worldContainer->addOnScriptContainer->callScripts(sim_syscb_init, nullptr, nullptr);
    }
    setAppStage(appstage_simRunning);

    if (!App::userSettings->doNotWritePersistentData)
    {
        CPersistentDataContainer cont;
        std::string val;
        cont.readData("SIMSETTINGS_SIM_CRASHED", val);
        if (val.size() > 0)
            App::logMsg(sim_verbosity_warnings,
                        (std::string("If CoppeliaSim crashed in previous session, you can find auto-saved scenes in ") +
                         App::folders->getAutoSavedScenesContainingPath())
                            .c_str());
        cont.writeData("SIMSETTINGS_SIM_CRASHED", "yes", true, false);
    }

    CSimFlavor::run(3);
    if (CSimFlavor::getBoolVal(17))
    {
        CSimFlavor::run(4);
        SSimulationThreadCommand cmd;
        cmd.cmdId = PLUS_HVUD_CMD;
        App::appendSimulationThreadCommand(cmd, 20.0);
    }
    {
        SSimulationThreadCommand cmd;
        cmd.cmdId = CHKLICM_CMD;
        App::appendSimulationThreadCommand(cmd, 5.0);
    }

    if (_startupScriptString.size() > 0)
    {
        int r=worldContainer->sandboxScript->executeScriptString(_startupScriptString.c_str(), nullptr);
        _startupScriptString.clear();
    }

#ifdef SIM_WITH_GUI
    // Prepare a few recurrent triggers:
    SSimulationThreadCommand cmd;
    cmd.cmdId = AUTO_SAVE_SCENE_CMD;
    App::appendSimulationThreadCommand(cmd, 2.0);
    cmd.cmdId = MEMORIZE_UNDO_STATE_IF_NEEDED_CMD;
    cmd.intParams.clear();
    App::appendSimulationThreadCommand(cmd, 2.2);
#endif
}

void App::cleanup()
{
    CSimFlavor::run(5);

    delete gm;
    gm = nullptr;

    while (worldContainer->getWorldCount() > 1)
        worldContainer->destroyCurrentWorld();
    currentWorld->clearScene(true);

    worldContainer->addOnScriptContainer->removeAllAddOns();
    worldContainer->sandboxScript->systemCallScript(sim_syscb_cleanup, nullptr, nullptr);
    CScriptObject::destroy(worldContainer->sandboxScript, true);
    worldContainer->sandboxScript = nullptr;
    worldContainer->pluginContainer->unloadNewPlugins(); // cleanup via (UI thread) and SIM thread

    CSimFlavor::run(10);

    delete simThread;
    simThread = nullptr;

#ifdef SIM_WITH_GUI
    CSimAndUiThreadSync::simThread_allowUiThreadToWrite(); // ...finally unlock
    GuiApp::qtApp->quit();
    if (getAppStage() == appstage_simRunning)
        setAppStage(appstage_guiCleanupRequest);
    while (getAppStage() != appstage_guiCleanupDone)
        VThread::sleep(1);
#endif

    worldContainer->deinitialize();
    delete worldContainer;
    worldContainer = nullptr;

    CThreadPool_old::cleanUp();

    delete folders;
    folders = nullptr;
    delete userSettings;
    userSettings = nullptr;

    _applicationArguments.clear();
    _applicationNamedParams.clear();
    _additionalAddOnScript1.clear();
    _additionalAddOnScript2.clear();
    if (_consoleMsgsFile != nullptr)
    {
        _consoleMsgsArchive->close();
        delete _consoleMsgsArchive;
        _consoleMsgsArchive = nullptr;
        _consoleMsgsFile->close();
        delete _consoleMsgsFile;
        _consoleMsgsFile = nullptr;
    }
    _consoleMsgsToFile = false;
    _consoleMsgsFilename = "debugLog.txt";
    _startupScriptString.clear();
    _consoleLogFilterStr.clear();
    _consoleVerbosity = sim_verbosity_default;
    _statusbarVerbosity = sim_verbosity_msgs;
    _dlgVerbosity = sim_verbosity_infos;

    VThread::unsetSimThread();
    delete _sigHandler;
    _exitCode = 0;
    delete instancesList;
    setAppStage(appstage_simCleanupDone);
#ifdef WIN_SIM
    timeEndPeriod(1);
#endif
    logMsg(sim_verbosity_loadinfos | sim_verbosity_onlyterminal, "CoppeliaSim ended.");
}

void App::loop(void (*callback)(), bool stepIfRunning)
{
    // Send the "instancePass" message to all plugins:
    int auxData[4] = {worldContainer->getModificationFlags(true), 0, 0, 0};
    worldContainer->pluginContainer->sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_instancepass, auxData);
#ifdef SIM_WITH_GUI
    SUIThreadCommand cmdIn;
    SUIThreadCommand cmdOut;
    cmdIn.cmdId = INSTANCE_PASS_FROM_UITHREAD_UITHREADCMD;
    GuiApp::uiThread->executeCommandViaUiThread(&cmdIn, &cmdOut);
#endif

    int editMode = NO_EDIT_MODE;
#ifdef SIM_WITH_GUI
    editMode = GuiApp::getEditModeType();
#endif
    if (currentWorld->simulation->isSimulationStopped() && (editMode == NO_EDIT_MODE))
    {
        worldContainer->dispatchEvents();
        worldContainer->callScripts(sim_syscb_nonsimulation, nullptr, nullptr);
    }
    App::currentWorld->sceneObjects->handleDataCallbacks();
    if (currentWorld->sceneObjects->hasSelectionChanged())
    {
        CInterfaceStack *stack = worldContainer->interfaceStackContainer->createStack();
        stack->pushTableOntoStack();
        stack->pushTextOntoStack("sel");
        std::vector<int> sel;
        currentWorld->sceneObjects->getSelectedObjectHandles(sel);
        stack->pushInt32ArrayOntoStack(sel.data(), sel.size());
        stack->insertDataIntoStackTable();
        worldContainer->callScripts(sim_syscb_selchange, stack, nullptr);
        worldContainer->interfaceStackContainer->destroyStack(stack);
    }
    if (currentWorld->simulation->isSimulationPaused())
    {
        CScriptObject *mainScript = currentWorld->sceneObjects->embeddedScriptContainer->getMainScript();
        if (mainScript != nullptr)
        {
            worldContainer->dispatchEvents();
            if (mainScript->systemCallMainScript(sim_syscb_suspended, nullptr, nullptr) == 0)
                worldContainer->callScripts(sim_syscb_suspended, nullptr, nullptr);
        }
    }

    //*******************************
    int auxValues[4];
    int messageID = 0;
    int dataSize;
    while (messageID != -1)
    {
        char *data = simGetSimulatorMessage_internal(&messageID, auxValues, &dataSize);
        if (messageID != -1)
        {
            if (messageID == sim_message_simulation_start_resume_request)
                simStartSimulation_internal();
            if (messageID == sim_message_simulation_pause_request)
                simPauseSimulation_internal();
            if (messageID == sim_message_simulation_stop_request)
                simStopSimulation_internal();
            if (data != NULL)
                simReleaseBuffer_internal(data);
        }
    }

    // Handle a running simulation:
    if (stepIfRunning && (simGetSimulationState_internal() & sim_simulation_advancing) != 0)
    {
        if ((!App::currentWorld->simulation->getIsRealTimeSimulation()) ||
            App::currentWorld->simulation->isRealTimeCalculationStepNeeded())
        {
            if ((!worldContainer->shouldTemporarilySuspendMainScript()) ||
                App::currentWorld->simulation->didStopRequestCounterChangeSinceSimulationStart())
            {
                CScriptObject *it = App::currentWorld->sceneObjects->embeddedScriptContainer->getMainScript();
                if (it != nullptr)
                {
                    worldContainer->calcInfo->simulationPassStart();
                    App::currentWorld->sceneObjects->embeddedScriptContainer->broadcastDataContainer.removeTimedOutObjects(
                        App::currentWorld->simulation->getSimulationTime()); // remove invalid elements
                    CThreadPool_old::prepareAllThreadsForResume_calledBeforeMainScript();
                    it->systemCallMainScript(-1, nullptr, nullptr);
                    worldContainer->calcInfo->simulationPassEnd();
                }
                App::currentWorld->simulation->advanceSimulationByOneStep();
            }
            // Following for backward compatibility:
            worldContainer->addOnScriptContainer->callScripts(sim_syscb_aos_run_old, nullptr, nullptr);
        }
        else
            worldContainer->callScripts(sim_syscb_realtimeidle, nullptr, nullptr);
    }
    //*******************************

    currentWorld->sceneObjects->eraseObjects(nullptr, true); // remove objects that have a delayed destruction
    currentWorld->sceneObjects->embeddedScriptContainer->removeDestroyedScripts(sim_scripttype_simulation);
    currentWorld->sceneObjects->embeddedScriptContainer->removeDestroyedScripts(sim_scripttype_customization);

    // Keep for backward compatibility:
    if (!currentWorld->simulation->isSimulationRunning()) // when simulation is running, we handle the add-on scripts
                                                          // after the main script was called
        worldContainer->addOnScriptContainer->callScripts(sim_syscb_aos_run_old, nullptr, nullptr);

    simThread->executeMessages(); // rendering, queued command execution, etc.
#ifdef SIM_WITH_GUI
    currentWorld->simulation->showAndHandleEmergencyStopButton(false, ""); // 10/10/2015
#else
    qtApp->processEvents();
#endif
}

long long int App::getFreshUniqueId()
{
    return (_nextUniqueId++);
}

App::App()
{
}

App::~App()
{
}

void App::beep(int frequ, int duration)
{
#ifdef SIM_WITH_GUI
    for (int i = 0; i < 3; i++)
    {
#ifdef WIN_SIM
        Beep(frequ, duration);
#else
        if (GuiApp::qtApp != nullptr)
            GuiApp::qtApp->beep();
#endif
        VThread::sleep(500);
    }
#endif
}

std::string App::getApplicationArgument(int index)
{
    if (_applicationArguments.size() == 0)
    {
        for (int i = 0; i < 9; i++)
            _applicationArguments.push_back("");
    }
    if (index < 9)
        return (_applicationArguments[index]);
    return ("");
}

void App::setApplicationArgument(int index, std::string arg)
{
    if (_applicationArguments.size() == 0)
    {
        for (int i = 0; i < 9; i++)
            _applicationArguments.push_back("");
    }
    if (index < 9)
        _applicationArguments[index] = arg;
}

void App::setAdditionalAddOnScript1(const char *script)
{
    _additionalAddOnScript1 = script;
}

std::string App::getAdditionalAddOnScript1()
{
    return (_additionalAddOnScript1);
}

void App::setAdditionalAddOnScript2(const char *script)
{
    _additionalAddOnScript2 = script;
}

std::string App::getAdditionalAddOnScript2()
{
    return (_additionalAddOnScript2);
}

std::string App::getApplicationNamedParam(const char *paramName)
{
    std::map<std::string, std::string>::iterator it = _applicationNamedParams.find(paramName);
    if (it != _applicationNamedParams.end())
        return (it->second);
    return ("");
}

int App::setApplicationNamedParam(const char *paramName, const char *param, int paramLength /*=0*/)
{
    int retVal = -1;
    if (strlen(paramName) > 0)
    {
        retVal = 0;
        if (getApplicationNamedParam(paramName).size() == 0)
            retVal = 1;
        int l = paramLength;
        if (l == 0)
        {
            if (param != nullptr)
                l = strlen(param);
        }
        if (l != 0)
            _applicationNamedParams[paramName] = std::string(param, param + l);
        else
        {
            std::map<std::string, std::string>::iterator it = _applicationNamedParams.find(paramName);
            if (it != _applicationNamedParams.end())
                _applicationNamedParams.erase(it);
        }
    }
    return (retVal);
}

std::string App::getConsoleLogFilter()
{
    return (_consoleLogFilterStr);
}

void App::setConsoleLogFilter(const char *filter)
{
    _consoleLogFilterStr = filter;
}

bool App::logPluginMsg(const char *pluginName, int verbosityLevel, const char *logMsg)
{
    bool retVal = false;

    CPlugin *it = nullptr;

    if (worldContainer != nullptr)
    {
        if (pluginName == nullptr)
            it = worldContainer->pluginContainer->getCurrentPlugin();
        else
        {
            it = worldContainer->pluginContainer->getPluginFromName(pluginName);
            if (it == nullptr)
                it = worldContainer->pluginContainer->getPluginFromName_old(pluginName, true);
        }
    }
    int realVerbosityLevel = verbosityLevel & 0x0fff;
    if ((it != nullptr) || ((pluginName != nullptr) && (strcmp(pluginName, "CoppeliaSimClient") == 0)))
    {
        if (it != nullptr)
        {
            int consoleV = it->getConsoleVerbosity();
            if (consoleV == sim_verbosity_useglobal)
                consoleV = _consoleVerbosity;
            int statusbarV = it->getStatusbarVerbosity();
            if (statusbarV == sim_verbosity_useglobal)
                statusbarV = _statusbarVerbosity;
            if ((consoleV >= realVerbosityLevel) || (statusbarV >= realVerbosityLevel))
                __logMsg(it->getName().c_str(), verbosityLevel, logMsg, consoleV, statusbarV);
        }
        else
            __logMsg(pluginName, verbosityLevel, logMsg);
        retVal = true;
    }
    else
    { // let's just print a naked message.
        if (_consoleVerbosity >= realVerbosityLevel)
        {
            std::string msg("[");
            if (pluginName == nullptr)
                msg += "unknown plugin";
            else
                msg += pluginName;
            msg += "]   ";
            msg += logMsg;
            msg += "\n";
            printf("%s", msg.c_str());
        }
        retVal = true;
    }
    return (retVal);
}

void App::logMsg(int verbosityLevel, const char *msg, int int1, int int2 /*=0*/, int int3 /*=0*/)
{
    int realVerbosityLevel = verbosityLevel & 0x0fff;
    if ((_consoleVerbosity >= realVerbosityLevel) || (_statusbarVerbosity >= realVerbosityLevel))
        _logMsg(nullptr, verbosityLevel, msg, int1, int2, int3);
}

void App::logScriptMsg(const CScriptObject *script, int verbosityLevel, const char *msg)
{
    int realVerbosityLevel = verbosityLevel & 0x0fff;
    if ((_consoleVerbosity >= realVerbosityLevel) || (_statusbarVerbosity >= realVerbosityLevel))
    {
        if (script != nullptr)
            __logMsg(script->getShortDescriptiveName().c_str(), verbosityLevel, msg);
        else
            __logMsg(nullptr, verbosityLevel, msg);
    }
}

int App::getVerbosityLevelFromString(const char *verbosityStr)
{
    int retVal = -1;
    if (strcmp(verbosityStr, "none") == 0)
        retVal = sim_verbosity_none;
    if (strcmp(verbosityStr, "errors") == 0)
        retVal = sim_verbosity_errors;
    if (strcmp(verbosityStr, "warnings") == 0)
        retVal = sim_verbosity_warnings;
    if (strcmp(verbosityStr, "loadinfos") == 0)
        retVal = sim_verbosity_loadinfos;
    if (strcmp(verbosityStr, "questions") == 0)
        retVal = sim_verbosity_questions;
    if (strcmp(verbosityStr, "scripterrors") == 0)
        retVal = sim_verbosity_scripterrors;
    if (strcmp(verbosityStr, "scriptwarnings") == 0)
        retVal = sim_verbosity_scriptwarnings;
    if (strcmp(verbosityStr, "scriptinfos") == 0)
        retVal = sim_verbosity_msgs;
    if (strcmp(verbosityStr, "infos") == 0)
        retVal = sim_verbosity_infos;
    if (strcmp(verbosityStr, "debug") == 0)
        retVal = sim_verbosity_debug;
    if (strcmp(verbosityStr, "trace") == 0)
        retVal = sim_verbosity_trace;
    if (strcmp(verbosityStr, "tracelua") == 0)
        retVal = sim_verbosity_tracelua;
    if (strcmp(verbosityStr, "traceall") == 0)
        retVal = sim_verbosity_traceall;
    return (retVal);
}

bool App::getConsoleMsgToFile()
{
    return (_consoleMsgsToFile);
}

void App::setConsoleMsgToFile(bool f)
{
    _consoleMsgsToFile = f;
}

std::string App::getConsoleMsgFile()
{
    return (_consoleMsgsFilename);
}

void App::setConsoleMsgFile(const char *f)
{
    _consoleMsgsFilename = f;
}

void App::logMsg(int verbosityLevel, const char *msg, const char *subStr1, const char *subStr2 /*=nullptr*/,
                 const char *subStr3 /*=nullptr*/)
{
    int realVerbosityLevel = verbosityLevel & 0x0fff;
    if ((_consoleVerbosity >= realVerbosityLevel) || (_statusbarVerbosity >= realVerbosityLevel))
        _logMsg(nullptr, verbosityLevel, msg, subStr1, subStr2, subStr3);
}

void App::logMsg(int verbosityLevel, const char *msg)
{
    int realVerbosityLevel = verbosityLevel & 0x0fff;
    if ((_consoleVerbosity >= realVerbosityLevel) || (_statusbarVerbosity >= realVerbosityLevel))
        __logMsg(nullptr, verbosityLevel, msg);
}

void App::_logMsg(const char *originName, int verbosityLevel, const char *msg, const char *subStr1,
                  const char *subStr2 /*=nullptr*/, const char *subStr3 /*=nullptr*/)
{
    size_t bs = strlen(msg) + 200;
    char *buff = new char[bs];
    if (subStr2 != nullptr)
    {
        if (subStr3 != nullptr)
            snprintf(buff, bs, msg, subStr1, subStr2, subStr3);
        else
            snprintf(buff, bs, msg, subStr1, subStr2);
    }
    else
        snprintf(buff, bs, msg, subStr1);
    __logMsg(originName, verbosityLevel, buff);
    delete[] buff;
}

void App::_logMsg(const char *originName, int verbosityLevel, const char *msg, int int1, int int2 /*=0*/,
                  int int3 /*=0*/)
{
    size_t bs = strlen(msg) + 200;
    char *buff = new char[bs];
    snprintf(buff, bs, msg, int1, int2, int3);
    __logMsg(originName, verbosityLevel, buff);
    delete[] buff;
}

std::string App::_getHtmlEscapedString(const char *str)
{
    std::string s(str);
    utils::replaceSubstring(s, "<", "*+-%A%-+*");
    utils::replaceSubstring(s, ">", "*+-%B%-+*");
    utils::replaceSubstring(s, "\n", "*+-%NL%-+*");
    utils::replaceSubstring(s, " ", "*+-%S%-+*");
    utils::replaceSubstring(s, "\t", "*+-%T%-+*");
    utils::replaceSubstring(s, "/", "*+-%FS%-+*");
    QString qstr(s.c_str());
    qstr = qstr.toHtmlEscaped();
    s = qstr.toStdString();
    utils::replaceSubstring(s, "*+-%NL%-+*", "<br/>");
    utils::replaceSubstring(s, "*+-%S%-+*", "&nbsp;");
    utils::replaceSubstring(s, "*+-%T%-+*", "&nbsp;&nbsp;&nbsp;&nbsp;");
    utils::replaceSubstring(s, "*+-%FS%-+*", "&#47;");
    utils::replaceSubstring(s, "*+-%A%-+*", "&lt;");
    utils::replaceSubstring(s, "*+-%B%-+*", "&gt;");
    return (s);
}

bool App::_consoleLogFilter(const char *msg)
{
    bool triggered = true;
    if (_consoleLogFilterStr.size() > 0)
    {
        std::string theMsg(msg);
        std::istringstream isso(_consoleLogFilterStr);
        std::string orBlock;
        while (std::getline(isso, orBlock, '|'))
        {
            std::istringstream issa(orBlock);
            std::string andWord;
            triggered = true;
            while (std::getline(issa, andWord, '&'))
            {
                if (theMsg.find(andWord) == std::string::npos)
                {
                    triggered = false;
                    break;
                }
            }
            if (triggered)
                break;
        }
    }
    return (!triggered);
}

static std::string replaceVars(const std::string &format, const std::map<std::string, std::string> &vars)
{
    std::string msg;
    size_t last = 0;
    while (last < format.length())
    {
        size_t posOpen = format.find("{", last);
        size_t posClose = format.find("}", posOpen);
        if (posOpen != std::string::npos && posClose != std::string::npos)
        {
            msg += format.substr(last, posOpen - last);
            auto key = format.substr(posOpen + 1, posClose - posOpen - 1);
            auto it = vars.find(key);
            if (it != vars.end())
                msg += it->second;
            last = posClose + 1;
        }
        else
            break;
    }
    if (last < format.length())
        msg += format.substr(last, std::string::npos);
    return msg;
}

void App::__logMsg(const char *originName, int verbosityLevel, const char *msg, int consoleVerbosity /*=-1*/,
                   int statusbarVerbosity /*=-1*/)
{
    static bool inside = false;
    static int64_t lastTime = 0;
    if (!inside)
    {
        int realVerbosityLevel = verbosityLevel & 0x0fff;

        if (verbosityLevel & sim_verbosity_once)
        {
            if (_logOnceMessages[originName][realVerbosityLevel][msg]) return;
            else _logOnceMessages[originName][realVerbosityLevel][msg] = true;
        }

        if ((worldContainer != nullptr) && VThread::isSimThread())
        {
            std::string orig("CoppeliaSim");
            if (originName != nullptr)
                orig = originName;
            CCbor *ev = worldContainer->createEvent("logMsg", -1, -1, nullptr, false);
            ev->appendKeyText("origin", orig.c_str());
            ev->appendKeyText("msg", msg);
            ev->appendKeyInt("verbosity", realVerbosityLevel);
            ev->openKeyMap("flags");
            ev->appendKeyBool("undecorated", verbosityLevel & sim_verbosity_undecorated);
            ev->appendKeyBool("onlyterminal", verbosityLevel & sim_verbosity_onlyterminal);
            ev->closeArrayOrMap();
            worldContainer->pushEvent();
        }

        inside = true;

        bool decorateMsg = ((verbosityLevel & sim_verbosity_undecorated) == 0) &&
                           ((App::userSettings == nullptr) || (!userSettings->undecoratedStatusbarMessages));
        static std::string consoleLogFormat, statusbarLogFormat, statusbarLogFormatUndecorated;
        if (consoleLogFormat.empty())
        {
            auto f = std::getenv("COPPELIASIM_CONSOLE_LOG_FORMAT");
            if (f == nullptr)
            {
                if ((userSettings != nullptr) && userSettings->timeStamp)
                    consoleLogFormat = "[{stamp}][{origin}:{verbosity}]   {message}";
                else
                    consoleLogFormat = "[{origin}:{verbosity}]   {message}";
            }
            else
                consoleLogFormat = f;
        }
        if (statusbarLogFormat.empty())
        {
            auto f = std::getenv("COPPELIASIM_STATUSBAR_LOG_FORMAT");
            statusbarLogFormat =
                f ? f : "<font color='grey'>[{origin}:{verbosity}]</font>    <font color='{color}'>{message}</font>";
        }
        if (statusbarLogFormatUndecorated.empty())
        {
            auto f = std::getenv("COPPELIASIM_STATUSBAR_LOG_FORMAT_UNDECORATED");
            statusbarLogFormatUndecorated = f ? f : "<font color='{color}'>{message}</font>";
        }
        std::map<std::string, std::string> vars;
        vars["message"] = msg;
        vars["origin"] = originName ? originName : "CoppeliaSim";
        vars["verbosity"] = "unknown";
#ifdef SIM_WITH_GUI
        if (GuiApp::qtApp != nullptr)
        {
            QColor col = GuiApp::qtApp->style()->standardPalette().windowText().color();
            int rgb[3];
            col.getRgb(rgb + 0, rgb + 1, rgb + 2);
            int incr = 56;
            if (rgb[0] > 128)
                incr = -incr;
            for (int i = 1; i < 3; i++)
                rgb[i] += incr;
            col.setRgb(rgb[0], rgb[1], rgb[2]);
            vars["color"] = col.name().toStdString();
        }
        else
#endif
            vars["color"] = "#383838";
        int64_t t =
            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
                .count();
        std::stringstream ss;
        ss << std::fixed << std::setprecision(3) << 0.001 * t;
        vars["time"] = ss.str();
        std::stringstream ss1;
        ss1 << std::fixed << std::setprecision(3) << 0.001 * (t - lastTime);
        vars["delta"] = ss1.str();
        lastTime = t;
        int st = int(VDateTime::getTimeInMs());
        int hours = st / 3600000;
        st -= hours * 3600000;
        int minutes = st / 60000;
        st -= minutes * 60000;
        int seconds = st / 1000;
        st -= seconds * 1000;
        std::stringstream ss2;
        ss2 << std::setfill('0') << std::setw(2) << hours << ":" << std::setfill('0') << std::setw(2) << minutes << ":"
            << std::setfill('0') << std::setw(2) << seconds << "." << std::setfill('3') << std::setw(3) << st;
        vars["stamp"] = ss2.str();
        vars["uts"] = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz").toStdString().c_str();

        if ((realVerbosityLevel == sim_verbosity_errors) || (realVerbosityLevel == sim_verbosity_scripterrors))
        {
            vars["verbosity"] = "error";
            vars["color"] = "red";
        }
        if ((realVerbosityLevel == sim_verbosity_warnings) || (realVerbosityLevel == sim_verbosity_scriptwarnings))
        {
            vars["verbosity"] = "warning";
            vars["color"] = "#D35400";
        }
        if (realVerbosityLevel == sim_verbosity_loadinfos)
            vars["verbosity"] = "loadinfo";
        if ((realVerbosityLevel == sim_verbosity_infos) ||
            (realVerbosityLevel ==
             sim_verbosity_scriptinfos)) // also sim_verbosity_msgs, which is same as sim_verbosity_scriptinfos
            vars["verbosity"] = "info";
        if (realVerbosityLevel == sim_verbosity_debug)
            vars["verbosity"] = "debug";
        if (realVerbosityLevel == sim_verbosity_trace)
            vars["verbosity"] = "trace";
        if (realVerbosityLevel == sim_verbosity_tracelua)
            vars["verbosity"] = "tracelua";
        if (realVerbosityLevel == sim_verbosity_traceall)
            vars["verbosity"] = "traceall";

        {
            std::string message(msg);
            // For backward compatibility with messages that already have HTML tags:
            size_t p = message.rfind("@html");
            if ((p != std::string::npos) && (p == message.size() - 5))
            { // strip HTML stuff off
                message.assign(message.c_str(), message.c_str() + message.size() - 5);
                QTextDocument doc;
                doc.setHtml(message.c_str());
                message = doc.toPlainText().toStdString();
            }
            vars["message"] = message;
        }

        //   boost::replace_all(vars["message"],"\n","\n    ");

        std::string consoleTxt(replaceVars(consoleLogFormat, vars) + "\n");
        if (userSettings == nullptr)
            consoleLogFormat.clear();

        if (!_consoleLogFilter(consoleTxt.c_str()))
        {
            if (consoleVerbosity == -1)
            {
                consoleVerbosity = _consoleVerbosity;

                bool headless = true;
#ifdef SIM_WITH_GUI
                headless = (GuiApp::mainWindow == nullptr);
#endif
                if (headless)
                    consoleVerbosity = _statusbarVerbosity;
            }

            if (consoleVerbosity >= realVerbosityLevel)
            {
                printf("%s", consoleTxt.c_str());
                if (_consoleMsgsToFile)
                {
                    if (_consoleMsgsFile == nullptr)
                    {
                        _consoleMsgsFile =
                            new VFile(_consoleMsgsFilename.c_str(), VFile::CREATE_WRITE | VFile::SHARE_EXCLUSIVE);
                        _consoleMsgsArchive = new VArchive(_consoleMsgsFile, VArchive::STORE);
                    }
                    for (size_t i = 0; i < consoleTxt.size(); i++)
                        (*_consoleMsgsArchive) << consoleTxt[i];
                    //(*_consoleMsgsArchive) << ((unsigned char)13) << ((unsigned char)10);
                    _consoleMsgsFile->flush();
                }
            }
        }
        if (statusbarVerbosity == -1)
            statusbarVerbosity = _statusbarVerbosity;
#ifdef SIM_WITH_GUI
        if ((statusbarVerbosity >= realVerbosityLevel) && (GuiApp::uiThread != nullptr) &&
            ((verbosityLevel & sim_verbosity_onlyterminal) == 0))
        {
            vars["message"] = _getHtmlEscapedString(vars["message"].c_str());
            std::string statusbarTxt =
                replaceVars(decorateMsg ? statusbarLogFormat : statusbarLogFormatUndecorated, vars);
            GuiApp::logMsgToStatusbar(statusbarTxt.c_str(), true);
        }
#endif
        inside = false;
    }
}

int App::getDlgVerbosity()
{ // sim_verbosity_none, etc.
    return (_dlgVerbosity);
}

void App::setDlgVerbosity(int v)
{ // sim_verbosity_none, etc.
    _dlgVerbosity = v;
}

void App::setStartupScriptString(const char *str)
{
    _startupScriptString = str;
}

void App::setExitCode(int c)
{
    _exitCode = c;
}

int App::getExitCode()
{
    return (_exitCode);
}

void App::undoRedo_sceneChanged(const char *txt)
{
#ifdef SIM_WITH_GUI
    if (VThread::isUiThread())
    {
        SSimulationThreadCommand cmd;
        cmd.cmdId = 999999;
        cmd.stringParams.push_back(txt);
        App::appendSimulationThreadCommand(cmd);
    }
    else
#endif
        currentWorld->undoBufferContainer->announceChange();
}

void App::undoRedo_sceneChangedGradual(const char *txt)
{
#ifdef SIM_WITH_GUI
    if (VThread::isUiThread())
    {
        SSimulationThreadCommand cmd;
        cmd.cmdId = 999996;
        cmd.stringParams.push_back(txt);
        App::appendSimulationThreadCommand(cmd);
    }
    else
        currentWorld->undoBufferContainer->announceChangeGradual();
#endif
}

int App::getConsoleVerbosity(const char *pluginName /*=nullptr*/)
{ // sim_verbosity_none, etc.
    int retVal = _consoleVerbosity;
    if (pluginName != nullptr)
    {
        CPlugin *pl = worldContainer->pluginContainer->getPluginFromName_old(pluginName, true);
        if (pl != nullptr)
        {
            if (pl->getConsoleVerbosity() != sim_verbosity_useglobal)
                retVal = pl->getConsoleVerbosity();
        }
    }
    return (retVal);
}

void App::setConsoleVerbosity(int v, const char *pluginName /*=nullptr*/)
{ // sim_verbosity_none, etc.
    if (pluginName != nullptr)
    {
        CPlugin *pl = worldContainer->pluginContainer->getPluginFromName_old(pluginName, true);
        if (pl != nullptr)
            pl->setConsoleVerbosity(v);
    }
    else
    {
        bool diff = (_consoleVerbosity != v);
        if (diff)
        {
            _consoleVerbosity = v;
            if ((App::worldContainer != nullptr) && App::worldContainer->getEventsEnabled())
            {
                const char *cmd = propApp_consoleVerbosity.name;
                CCbor *ev = App::worldContainer->createObjectChangedEvent(sim_handle_app, cmd, true);
                ev->appendKeyInt(cmd, _consoleVerbosity);
                App::worldContainer->pushEvent();
            }
        }
    }
}

int App::getStatusbarVerbosity(const char *pluginName /*=nullptr*/)
{ // sim_verbosity_none, etc.
    int retVal = _statusbarVerbosity;
    if (pluginName != nullptr)
    {
        CPlugin *pl = worldContainer->pluginContainer->getPluginFromName_old(pluginName, true);
        if (pl != nullptr)
        {
            if (pl->getStatusbarVerbosity() != sim_verbosity_useglobal)
                retVal = pl->getStatusbarVerbosity();
        }
    }
    return (retVal);
}

void App::setStatusbarVerbosity(int v, const char *pluginName /*=nullptr*/)
{ // sim_verbosity_none, etc.
    if (pluginName != nullptr)
    {
        CPlugin *pl = worldContainer->pluginContainer->getPluginFromName_old(pluginName, true);
        if (pl != nullptr)
            pl->setStatusbarVerbosity(v);
    }
    else
    {
        bool diff = (_statusbarVerbosity != v);
        if (diff)
        {
            _statusbarVerbosity = v;
            if ((App::worldContainer != nullptr) && App::worldContainer->getEventsEnabled())
            {
                const char *cmd = propApp_statusbarVerbosity.name;
                CCbor *ev = App::worldContainer->createObjectChangedEvent(sim_handle_app, cmd, true);
                ev->appendKeyInt(cmd, _statusbarVerbosity);
                App::worldContainer->pushEvent();
            }
        }
    }
}

bool App::getConsoleOrStatusbarVerbosityTriggered(int verbosityLevel)
{
    return ((_consoleVerbosity >= verbosityLevel) || (_statusbarVerbosity >= verbosityLevel));
}

std::string App::getApplicationDir()
{
    return (_applicationDir);
}

void App::postExitRequest()
{
    _exitRequest = true;
}

bool App::getExitRequest()
{
    return (_exitRequest);
}

void App::appendSimulationThreadCommand(int cmdId, int intP1, int intP2, double floatP1, double floatP2,
                                        const char *stringP1, const char *stringP2, double executionDelay)
{ // convenience function. All args have default values except for the first
    SSimulationThreadCommand cmd;
    cmd.cmdId = cmdId;
    cmd.intParams.push_back(intP1);
    cmd.intParams.push_back(intP2);
    cmd.doubleParams.push_back(floatP1);
    cmd.doubleParams.push_back(floatP2);
    if (stringP1 == nullptr)
        cmd.stringParams.push_back("");
    else
        cmd.stringParams.push_back(stringP1);
    if (stringP2 == nullptr)
        cmd.stringParams.push_back("");
    else
        cmd.stringParams.push_back(stringP2);
    appendSimulationThreadCommand(cmd, executionDelay);
}

void App::appendSimulationThreadCommand(SSimulationThreadCommand cmd, double executionDelay /*=0.0*/)
{
    static std::vector<SSimulationThreadCommand> delayed_cmd;
    static std::vector<double> delayed_delay;
    if (simThread != nullptr)
    {
        if (delayed_cmd.size() != 0)
        {
            for (unsigned int i = 0; i < delayed_cmd.size(); i++)
                simThread->appendSimulationThreadCommand(delayed_cmd[i], delayed_delay[i]);
            delayed_cmd.clear();
            delayed_delay.clear();
        }
        simThread->appendSimulationThreadCommand(cmd, executionDelay);
    }
    else
    { // can happen during the initialization phase, when the client loads a scene for instance
        delayed_cmd.push_back(cmd);
        delayed_delay.push_back(executionDelay);
    }
}

bool App::disassemble(int objectHandle, bool justTest, bool msgs /* = false*/)
{
    bool retVal = false;
    CSceneObject *it;
    it = App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
    CSceneObject *parent = it->getParent();
    if (parent != nullptr)
    {
        for (size_t i = 0; i < parent->getChildCount(); i++)
        {
            CSceneObject *child = parent->getChildFromIndex(i);
            if (child->getObjectType() == sim_sceneobject_dummy)
            {
                CDummy *dummy = (CDummy *)child;
                std::string childTag(dummy->getAssemblyTag());
                if (utils::checkAssemblyTagValidity(childTag.c_str(), nullptr))
                { // the parent has at least one valid assembly item

                    for (size_t j = 0; j < it->getChildCount(); j++)
                    {
                        child = it->getChildFromIndex(j);
                        if (child->getObjectType() == sim_sceneobject_dummy)
                        {
                            dummy = (CDummy *)child;
                            std::string childTag(dummy->getAssemblyTag());
                            if (utils::checkAssemblyTagValidity(nullptr, childTag.c_str()))
                            { // the child has at least one valid assembly item
                                retVal = true;
                                break;
                            }
                        }
                    }
                    break;
                }
            }
        }
        if (!retVal)
        { // backward compatibility method:
            retVal = (((parent->getObjectType() == sim_sceneobject_joint) ||
                       (parent->getObjectType() == sim_sceneobject_forcesensor)) &&
                      (it->getObjectType() == sim_sceneobject_shape));
        }
    }
    if (retVal && (!justTest))
    {
        if (msgs)
            App::logMsg(sim_verbosity_msgs, "Disassembling item...");
        App::currentWorld->sceneObjects->setObjectParent(it, nullptr, true);
        App::undoRedo_sceneChanged("");
        if (msgs)
            App::logMsg(sim_verbosity_msgs, "done.");
    }
    return retVal;
}

bool App::assemble(int parentHandle, int childHandle, bool justTest, bool msgs /* = false*/)
{ // e.g. robot (parent) <-- gripper (child)
    bool retVal = false;
    CSceneObject *it1;  // robot dummy
    CSceneObject *it2;  // gripper dummy (or object itself (special case))
    CSceneObject *obj1; // robot part
    CSceneObject *obj2; // gripper base
    it1 = App::currentWorld->sceneObjects->getObjectFromHandle(parentHandle);
    it2 = App::currentWorld->sceneObjects->getObjectFromHandle(childHandle);
    obj1 = it1->getParent();
    obj2 = it2->getParent();
    if ((it1->getObjectType() == sim_sceneobject_dummy) && (obj1 != nullptr))
    { // possibly new method of assembly (via 2 dummies)
        CDummy *dummy1 = (CDummy *)it1;
        std::string parentTag(dummy1->getAssemblyTag());
        if ((dummy1->getDummyType() == sim_dummytype_assembly) &&
            utils::checkAssemblyTagValidity(parentTag.c_str(), nullptr))
        { // we have the correct dummy type for the parent side
            if (it2->getObjectType() != sim_sceneobject_dummy)
            { // we might have the special case. Let's search for an appropriate dummy:
                obj2 = it2;
                it2 = nullptr;
                for (size_t i = 0; i < obj2->getChildCount(); i++)
                {
                    CSceneObject *child = obj2->getChildFromIndex(i);
                    if (child->getObjectType() == sim_sceneobject_dummy)
                    {
                        CDummy *dummy = (CDummy *)child;
                        std::string childTag(dummy->getAssemblyTag());
                        if ((dummy->getDummyType() == sim_dummytype_assembly) &&
                            utils::checkAssemblyTagValidity(parentTag.c_str(), childTag.c_str()))
                        { // we have the correct dummy type for the child side (we take the first compatible dummy
                          // found)
                            it2 = dummy;
                            break;
                        }
                    }
                }
            }
            if ((it2 != nullptr) && (obj2 != nullptr))
            {
                CDummy *dummy = (CDummy *)it2;
                std::string childTag(dummy->getAssemblyTag());
                if (utils::checkAssemblyTagValidity(parentTag.c_str(), childTag.c_str()))
                {
                    if (!obj1->hasAncestor(obj2)) // (obj2->getParent() != obj1) (if we are already connected, we might
                                                  // want to correct the pose)
                        retVal = true;
                }
            }
        }
    }

    if (!retVal)
    { // old method of assembling 2 objects. We limit the scope to joint/fsensor as parent and shape as child, since we
      // slowly want to get rid of that method
        it1 = App::currentWorld->sceneObjects->getObjectFromHandle(parentHandle);
        it2 = App::currentWorld->sceneObjects->getObjectFromHandle(childHandle);
        if ((it1->getParent() != it2) && (it2->getParent() != it1))
        {
            if (((it1->getObjectType() == sim_sceneobject_joint) ||
                 (it1->getObjectType() == sim_sceneobject_forcesensor)) &&
                (it2->getObjectType() == sim_sceneobject_shape))
            {
                std::vector<CSceneObject *> potParents;
                it1->getAllChildrenThatMayBecomeAssemblyParent(it2->getChildAssemblyMatchValuesPointer(), potParents);
                bool directAssembly =
                    it1->doesParentAssemblingMatchValuesMatchWithChild(it2->getChildAssemblyMatchValuesPointer());
                if (directAssembly || (potParents.size() == 1))
                {
                    retVal = true;
                    obj1 = it1;
                    obj2 = it2;
                }
            }
        }
        it1 = nullptr;
        it2 = nullptr;
    }
    if (retVal && (!justTest))
    {
        if (msgs)
            App::logMsg(sim_verbosity_msgs, "Assembling items...");
        if (it1 != nullptr)
        { // new method (via dummies)
            C7Vector newLocal(it1->getFullLocalTransformation() * it2->getFullLocalTransformation().getInverse());
            App::currentWorld->sceneObjects->setObjectParent(obj2, obj1, true);
            obj2->setLocalTransformation(newLocal);
        }
        else
        { // old method
            std::vector<CSceneObject *> potParents;
            obj1->getAllChildrenThatMayBecomeAssemblyParent(obj2->getChildAssemblyMatchValuesPointer(), potParents);
            bool directAssembly =
                obj1->doesParentAssemblingMatchValuesMatchWithChild(obj2->getChildAssemblyMatchValuesPointer());
            if (directAssembly || (potParents.size() == 1))
            {
                if (directAssembly)
                    App::currentWorld->sceneObjects->setObjectParent(obj2, obj1, true);
                else
                    App::currentWorld->sceneObjects->setObjectParent(obj2, potParents[0], true);
                if (obj2->getAssemblingLocalTransformationIsUsed())
                    obj2->setLocalTransformation(obj2->getAssemblingLocalTransformation());
            }
        }
        App::undoRedo_sceneChanged("");
        if (msgs)
            App::logMsg(sim_verbosity_msgs, "done.");
    }
    return retVal;
}

int App::setBoolProperty(long long int target, const char* pName, bool pState)
{
    int retVal = -1;
    if (target == sim_handle_app)
    {
        if (worldContainer != nullptr)
            retVal = worldContainer->setBoolProperty(pName, pState);
    }
    else if (target == sim_handle_appstorage)
    {
        // only setBufferProperty/getBufferProperty are operations with appstorage!
    }
    else if (currentWorld != nullptr)
        retVal = currentWorld->setBoolProperty(target, pName, pState);
    return retVal;
}

int App::getBoolProperty(long long int target, const char* pName, bool& pState)
{
    int retVal = -1;
    if (target == sim_handle_app)
    {
        if (worldContainer != nullptr)
            retVal = worldContainer->getBoolProperty(pName, pState);
    }
    else if (target == sim_handle_appstorage)
    {
        // only setBufferProperty/getBufferProperty are operations with appstorage!
    }
    else if (currentWorld != nullptr)
        retVal = currentWorld->getBoolProperty(target, pName, pState);
    return retVal;
}

int App::setIntProperty(long long int target, const char* pName, int pState)
{
    int retVal = -1;
    if (target == sim_handle_app)
    {
        if (worldContainer != nullptr)
            retVal = worldContainer->setIntProperty(pName, pState);
    }
    else if (target == sim_handle_appstorage)
    {
        // only setBufferProperty/getBufferProperty are operations with appstorage!
    }
    else if (currentWorld != nullptr)
        retVal = currentWorld->setIntProperty(target, pName, pState);
    return retVal;
}

int App::getIntProperty(long long int target, const char* pName, int& pState)
{
    int retVal = -1;
    if (target == sim_handle_app)
    {
        if (worldContainer != nullptr)
            retVal = worldContainer->getIntProperty(pName, pState);
    }
    else if (target == sim_handle_appstorage)
    {
        // only setBufferProperty/getBufferProperty are operations with appstorage!
    }
    else if (currentWorld != nullptr)
        retVal = currentWorld->getIntProperty(target, pName, pState);
    return retVal;
}

int App::setFloatProperty(long long int target, const char* pName, double pState)
{
    int retVal = -1;
    if (target == sim_handle_app)
    {
        if (worldContainer != nullptr)
            retVal = worldContainer->setFloatProperty(pName, pState);
    }
    else if (target == sim_handle_appstorage)
    {
        // only setBufferProperty/getBufferProperty are operations with appstorage!
    }
    else if (currentWorld != nullptr)
        retVal = currentWorld->setFloatProperty(target, pName, pState);
    return retVal;
}

int App::getFloatProperty(long long int target, const char* pName, double& pState)
{
    int retVal = -1;
    if (target == sim_handle_app)
    {
        if (worldContainer != nullptr)
            retVal = worldContainer->getFloatProperty(pName, pState);
    }
    else if (target == sim_handle_appstorage)
    {
        // only setBufferProperty/getBufferProperty are operations with appstorage!
    }
    else if (currentWorld != nullptr)
        retVal = currentWorld->getFloatProperty(target, pName, pState);
    return retVal;
}

int App::setStringProperty(long long int target, const char* pName, const char* pState)
{
    int retVal = -1;
    if (target == sim_handle_app)
    {
        if (worldContainer != nullptr)
            retVal = worldContainer->setStringProperty(pName, pState);
    }
    else if (target == sim_handle_appstorage)
    {
        // only setBufferProperty/getBufferProperty are operations with appstorage!
    }
    else if (currentWorld != nullptr)
        retVal = currentWorld->setStringProperty(target, pName, pState);
    return retVal;
}

int App::getStringProperty(long long int target, const char* pName, std::string& pState)
{
    int retVal = -1;
    if (target == sim_handle_app)
    {
        if (worldContainer != nullptr)
            retVal = worldContainer->getStringProperty(pName, pState);
    }
    else if (target == sim_handle_appstorage)
    {
        // only setBufferProperty/getBufferProperty are operations with appstorage!
    }
    else if (currentWorld != nullptr)
        retVal = currentWorld->getStringProperty(target, pName, pState);
    return retVal;
}

int App::setBufferProperty(long long int target, const char* pName, const char* buffer, int bufferL)
{
    int retVal = -1;
    if (buffer == nullptr)
        bufferL = 0;
    if (target == sim_handle_app)
    {
        if (worldContainer != nullptr)
            retVal = worldContainer->setBufferProperty(pName, buffer, bufferL);
    }
    else if (target == sim_handle_appstorage)
    {
        std::string PPName(utils::getWithoutPrefix(pName, "storage."));
        if (strncmp(PPName.c_str(), "customData.", 11) == 0)
        {
            std::string pN(PPName);
            pN.erase(0, 11);
            if (pN.size() > 0)
            {
                pN += "&customData"; // we add a suffix to separate user and system data
                CPersistentDataContainer cont("appStorage.dat");
                cont.writeData(pN.c_str(), std::string(buffer, buffer + bufferL), true, true);
                retVal = 1;
            }
        }
        else
        {
            std::string dummyVal;
            if (getBufferProperty(target, PPName.c_str(), dummyVal) == 1)
            { // we can only modify it if it exists
                CPersistentDataContainer cont("appStorage.dat");
                cont.writeData(PPName.c_str(), std::string(buffer, buffer + bufferL), true, true);
                retVal = 1;
            }
        }
    }
    else if (currentWorld != nullptr)
        retVal = currentWorld->setBufferProperty(target, pName, buffer, bufferL);
    return retVal;
}

int App::getBufferProperty(long long int target, const char* pName, std::string& pState)
{
    int retVal = -1;
    if (target == sim_handle_app)
    {
        if (worldContainer != nullptr)
            retVal = worldContainer->getBufferProperty(pName, pState);
    }
    else if (target == sim_handle_appstorage)
    {
        std::string pN(utils::getWithoutPrefix(pName, "storage."));
        if (strncmp(pN.c_str(), "customData.", 11) == 0)
        {
            pN.erase(0, 11);
            pN += "&customData"; // we add a suffix to separate user and system data
        }
        if (pN.size() > 0)
        {
            CPersistentDataContainer cont("appStorage.dat");
            if (cont.readData(pN.c_str(), pState))
                retVal = 1;
        }
    }
    else if (currentWorld != nullptr)
        retVal = currentWorld->getBufferProperty(target, pName, pState);
    return retVal;
}

int App::setVector3Property(long long int target, const char* pName, const C3Vector& pState)
{
    int retVal = -1;
    if (target == sim_handle_app)
    {
        if (worldContainer != nullptr)
            retVal = worldContainer->setVector3Property(pName, pState);
    }
    else if (target == sim_handle_appstorage)
    {
        // only setBufferProperty/getBufferProperty are operations with appstorage!
    }
    else if (currentWorld != nullptr)
        retVal = currentWorld->setVector3Property(target, pName, pState);
    return retVal;
}

int App::getVector3Property(long long int target, const char* pName, C3Vector& pState)
{
    int retVal = -1;
    if (target == sim_handle_app)
    {
        if (worldContainer != nullptr)
            retVal = worldContainer->getVector3Property(pName, pState);
    }
    else if (target == sim_handle_appstorage)
    {
        // only setBufferProperty/getBufferProperty are operations with appstorage!
    }
    else if (currentWorld != nullptr)
        retVal = currentWorld->getVector3Property(target, pName, pState);
    return retVal;
}

int App::setQuaternionProperty(long long int target, const char* pName, const C4Vector& pState)
{
    int retVal = -1;
    if (target == sim_handle_app)
    {
        if (worldContainer != nullptr)
            retVal = worldContainer->setQuaternionProperty(pName, pState);
    }
    else if (target == sim_handle_appstorage)
    {
        // only setBufferProperty/getBufferProperty are operations with appstorage!
    }
    else if (currentWorld != nullptr)
        retVal = currentWorld->setQuaternionProperty(target, pName, pState);
    return retVal;
}

int App::getQuaternionProperty(long long int target, const char* pName, C4Vector& pState)
{
    int retVal = -1;
    if (target == sim_handle_app)
    {
        if (worldContainer != nullptr)
            retVal = worldContainer->getQuaternionProperty(pName, pState);
    }
    else if (target == sim_handle_appstorage)
    {
        // only setBufferProperty/getBufferProperty are operations with appstorage!
    }
    else if (currentWorld != nullptr)
        retVal = currentWorld->getQuaternionProperty(target, pName, pState);
    return retVal;
}

int App::setPoseProperty(long long int target, const char* pName, const C7Vector& pState)
{
    int retVal = -1;
    if (target == sim_handle_app)
    {
        if (worldContainer != nullptr)
            retVal = worldContainer->setPoseProperty(pName, pState);
    }
    else if (target == sim_handle_appstorage)
    {
        // only setBufferProperty/getBufferProperty are operations with appstorage!
    }
    else if (currentWorld != nullptr)
        retVal = currentWorld->setPoseProperty(target, pName, pState);
    return retVal;
}

int App::getPoseProperty(long long int target, const char* pName, C7Vector& pState)
{
    int retVal = -1;
    if (target == sim_handle_app)
    {
        if (worldContainer != nullptr)
            retVal = worldContainer->getPoseProperty(pName, pState);
    }
    else if (target == sim_handle_appstorage)
    {
        // only setBufferProperty/getBufferProperty are operations with appstorage!
    }
    else if (currentWorld != nullptr)
        retVal = currentWorld->getPoseProperty(target, pName, pState);
    return retVal;
}

int App::setMatrix3x3Property(long long int target, const char* pName, const C3X3Matrix& pState)
{
    int retVal = -1;
    if (target == sim_handle_app)
    {
        if (worldContainer != nullptr)
            retVal = worldContainer->setMatrix3x3Property(pName, pState);
    }
    else if (target == sim_handle_appstorage)
    {
        // only setBufferProperty/getBufferProperty are operations with appstorage!
    }
    else if (currentWorld != nullptr)
        retVal = currentWorld->setMatrix3x3Property(target, pName, pState);
    return retVal;
}

int App::getMatrix3x3Property(long long int target, const char* pName, C3X3Matrix& pState)
{
    int retVal = -1;
    if (target == sim_handle_app)
    {
        if (worldContainer != nullptr)
            retVal = worldContainer->getMatrix3x3Property(pName, pState);
    }
    else if (target == sim_handle_appstorage)
    {
        // only setBufferProperty/getBufferProperty are operations with appstorage!
    }
    else if (currentWorld != nullptr)
        retVal = currentWorld->getMatrix3x3Property(target, pName, pState);
    return retVal;
}

int App::setMatrix4x4Property(long long int target, const char* pName, const C4X4Matrix& pState)
{
    int retVal = -1;
    if (target == sim_handle_app)
    {
        if (worldContainer != nullptr)
            retVal = worldContainer->setMatrix4x4Property(pName, pState);
    }
    else if (target == sim_handle_appstorage)
    {
        // only setBufferProperty/getBufferProperty are operations with appstorage!
    }
    else if (currentWorld != nullptr)
        retVal = currentWorld->setMatrix4x4Property(target, pName, pState);
    return retVal;
}

int App::getMatrix4x4Property(long long int target, const char* pName, C4X4Matrix& pState)
{
    int retVal = -1;
    if (target == sim_handle_app)
    {
        if (worldContainer != nullptr)
            retVal = worldContainer->getMatrix4x4Property(pName, pState);
    }
    else if (target == sim_handle_appstorage)
    {
        // only setBufferProperty/getBufferProperty are operations with appstorage!
    }
    else if (currentWorld != nullptr)
        retVal = currentWorld->getMatrix4x4Property(target, pName, pState);
    return retVal;
}

int App::setColorProperty(long long int target, const char* pName, const float* pState)
{
    int retVal = -1;
    if (target == sim_handle_app)
    {
        if (worldContainer != nullptr)
            retVal = worldContainer->setColorProperty(pName, pState);
    }
    else if (target == sim_handle_appstorage)
    {
        // only setBufferProperty/getBufferProperty are operations with appstorage!
    }
    else if (currentWorld != nullptr)
        retVal = currentWorld->setColorProperty(target, pName, pState);
    return retVal;
}

int App::getColorProperty(long long int target, const char* pName, float* pState)
{
    int retVal = -1;
    if (target == sim_handle_app)
    {
        if (worldContainer != nullptr)
            retVal = worldContainer->getColorProperty(pName, pState);
    }
    else if (target == sim_handle_appstorage)
    {
        // only setBufferProperty/getBufferProperty are operations with appstorage!
    }
    else if (currentWorld != nullptr)
        retVal = currentWorld->getColorProperty(target, pName, pState);
    return retVal;
}

int App::setVectorProperty(long long int target, const char* pName, const double* v, int vL)
{
    int retVal = -1;
    if (target == sim_handle_app)
    {
        if (worldContainer != nullptr)
            retVal = worldContainer->setVectorProperty(pName, v, vL);
    }
    else if (target == sim_handle_appstorage)
    {
        // only setBufferProperty/getBufferProperty are operations with appstorage!
    }
    else if (currentWorld != nullptr)
        retVal = currentWorld->setVectorProperty(target, pName, v, vL);
    return retVal;
}

int App::getVectorProperty(long long int target, const char* pName, std::vector<double>& pState)
{
    int retVal = -1;
    pState.clear();
    if (target == sim_handle_app)
    {
        if (worldContainer != nullptr)
            retVal = worldContainer->getVectorProperty(pName, pState);
    }
    else if (target == sim_handle_appstorage)
    {
        // only setBufferProperty/getBufferProperty are operations with appstorage!
    }
    else if (currentWorld != nullptr)
        retVal = currentWorld->getVectorProperty(target, pName, pState);
    return retVal;
}

int App::setIntVectorProperty(long long int target, const char* pName, const int* v, int vL)
{
    int retVal = -1;
    if (target == sim_handle_app)
    {
        if (worldContainer != nullptr)
            retVal = worldContainer->setIntVectorProperty(pName, v, vL);
    }
    else if (target == sim_handle_appstorage)
    {
        // only setBufferProperty/getBufferProperty are operations with appstorage!
    }
    else if (currentWorld != nullptr)
        retVal = currentWorld->setIntVectorProperty(target, pName, v, vL);
    return retVal;
}

int App::getIntVectorProperty(long long int target, const char* pName, std::vector<int>& pState)
{
    int retVal = -1;
    pState.clear();
    if (target == sim_handle_app)
    {
        if (worldContainer != nullptr)
            retVal = worldContainer->getIntVectorProperty(pName, pState);
    }
    else if (target == sim_handle_appstorage)
    {
        // only setBufferProperty/getBufferProperty are operations with appstorage!
    }
    else if (currentWorld != nullptr)
        retVal = currentWorld->getIntVectorProperty(target, pName, pState);
    return retVal;
}

int App::removeProperty(long long int target, const char* pName)
{
    int retVal = -1;
    if (target == sim_handle_app)
    {
        if (worldContainer != nullptr)
            retVal = worldContainer->removeProperty(pName);
    }
    else if (target == sim_handle_appstorage)
    {
        std::string pN(utils::getWithoutPrefix(pName, "storage."));
        bool canBeRemoved = false;
        if (strncmp(pN.c_str(), "customData.", 11) == 0)
        {
            canBeRemoved = true;
            pN.erase(0, 11);
            pN += "&customData"; // we add a suffix to separate user and system data
        }
        if (pN.size() > 0)
        {
            CPersistentDataContainer cont("appStorage.dat");
            int tp = cont.hasData(pN.c_str(), true);
            if (tp >= 0)
            {
                if (canBeRemoved)
                {
                    cont.clearData((propertyStrings[tp] + pN).c_str(), true);
                    retVal = 1;
                }
                else
                    retVal = 0;
            }
        }
    }
    else if (currentWorld != nullptr)
        retVal = currentWorld->removeProperty(target, pName);
    return retVal;
}

int App::getPropertyName(long long int target, int& index, std::string& pName, std::string& appartenance, bool staticParsing)
{
    int retVal = -1;
    if (target == sim_handle_app)
    {
        appartenance = "app";
        CWorldContainer* wc = nullptr;
        if (!staticParsing)
            wc = worldContainer;
        retVal = CWorldContainer::getPropertyName(index, pName, wc);
    }
    else if (target == sim_handle_appstorage)
    {
        if (!staticParsing)
        {
            appartenance = "storage";
            CPersistentDataContainer cont("appStorage.dat");
            if (cont.getPropertyName(index, pName))
            {
                size_t p = pName.find("&customData");
                if (p != std::string::npos)
                {
                    pName.erase(p);
                    pName = "customData." + pName;
                    //pName = "storage." + pName;
                }
                retVal = 1;
            }
        }
    }
    else if (currentWorld != nullptr)
    {
        appartenance = "app";
        CWorld* cw = nullptr;
        if (!staticParsing)
            cw = currentWorld;
        retVal = CWorld::getPropertyName(target, index, pName, appartenance, cw);
    }
    return retVal;
}

int App::getPropertyInfo(long long int target, const char* pName, int& info, std::string& infoTxt, bool staticParsing)
{
    int retVal = -1;
    if (target == sim_handle_app)
    {
        CWorldContainer* wc = nullptr;
        if (!staticParsing)
            wc = worldContainer;
        retVal = CWorldContainer::getPropertyInfo(pName, info, infoTxt, wc);
    }
    else if (target == sim_handle_appstorage)
    {
        if (!staticParsing)
        {
            std::string pN(utils::getWithoutPrefix(pName, "storage."));
            int inf = 0;
            if (strncmp(pN.c_str(), "customData.", 11) == 0)
            {
                pN.erase(0, 11);
                pN += "&customData";
                inf = 4; // removable
            }
            if (pN.size() > 0)
            {
                CPersistentDataContainer cont("appStorage.dat");
                int s;
                retVal = cont.hasData(pN.c_str(), true, &s);
                if (retVal >= 0)
                {
                    info = inf;
                    if (s > LARGE_PROPERTY_SIZE)
                        info = info | 0x100;
                }
            }
        }
    }
    else if (currentWorld != nullptr)
    {
        CWorld* cw = nullptr;
        if (!staticParsing)
            cw = currentWorld;
        retVal = CWorld::getPropertyInfo(target, pName, info, infoTxt, cw);
    }
    return retVal;
}

void App::setHierarchyEnabled(bool v)
{
    bool diff = (_hierarchyEnabled != v);
    if (diff)
    {
        _hierarchyEnabled = v;
        if ((App::worldContainer != nullptr) && App::worldContainer->getEventsEnabled())
        {
            const char *cmd = propApp_hierarchyEnabled.name;
            CCbor *ev = App::worldContainer->createObjectChangedEvent(sim_handle_app, cmd, true);
            ev->appendKeyBool(cmd, _hierarchyEnabled);
            App::worldContainer->pushEvent();
        }
#ifdef SIM_WITH_GUI
        if (GuiApp::mainWindow != nullptr)
        {
            if (!_hierarchyEnabled)
                GuiApp::mainWindow->oglSurface->setFocusObject(FOCUS_ON_PAGE);
            GuiApp::mainWindow->oglSurface->actualizeAllSurfacesSizeAndPosition();
            GuiApp::setToolbarRefreshFlag();
        }
#endif
    }
}

bool App::getHierarchyEnabled()
{
    return _hierarchyEnabled;
}

void App::setOpenGlDisplayEnabled(bool e)
{
    bool diff = (_openGlDisplayEnabled != e);
    if (diff)
    {
        _openGlDisplayEnabled = e;
        if ((App::worldContainer != nullptr) && App::worldContainer->getEventsEnabled())
        {
            const char *cmd = propApp_displayEnabled.name;
            CCbor *ev = App::worldContainer->createObjectChangedEvent(sim_handle_app, cmd, true);
            ev->appendKeyBool(cmd, _openGlDisplayEnabled);
            App::worldContainer->pushEvent();
        }
#ifdef SIM_WITH_GUI
        GuiApp::setToolbarRefreshFlag();
        GuiApp::setRefreshHierarchyViewFlag();
#endif
    }
}

bool App::getOpenGlDisplayEnabled()
{
    return _openGlDisplayEnabled;
}

int App::getPlatform()
{
    int retVal;
#ifdef WIN_SIM
    retVal = 0;
#endif
#ifdef MAC_SIM
    retVal = 1;
#endif
#ifdef LIN_SIM
    retVal = 2;
#endif
    return retVal;
}
