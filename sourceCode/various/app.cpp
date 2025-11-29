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
CSimQApp* App::qtApp = nullptr;
int App::_qApp_argc = 1;
char App::_qApp_arg0[] = {"CoppeliaSim"};
char* App::_qApp_argv[1] = {_qApp_arg0};
#endif

static std::string OBJECT_TYPE = "app";
static std::string OBJECT_META_INFO = R"(
{
    "superclass": "object",
    "namespaces": {
        "namedParam": {},
        "customData": {},
        "signal": {}
    },
    "methods": {
        )" OBJECT_META_METHODS R"(
    }
}
)";

CSimThread* App::simThread = nullptr;
CUserSettings* App::userSettings = nullptr;
CFolderSystem* App::folders = nullptr;
CWorldContainer* App::worldContainer = nullptr;
CWorld* App::currentWorld = nullptr;
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
std::string App::_applicationDir;
CPersistentDataContainer* App::_appStorage = nullptr;
std::vector<std::string> App::_applicationArguments;
std::map<std::string, std::string> App::_applicationNamedParams;
std::string App::_additionalAddOnScript1;
std::string App::_additionalAddOnScript2;
bool App::_consoleMsgsToFile = false;
std::string App::_consoleMsgsFilename = "debugLog.txt";
VFile* App::_consoleMsgsFile = nullptr;
VArchive* App::_consoleMsgsArchive = nullptr;
SignalHandler* App::_sigHandler = nullptr;
CGm* App::gm = nullptr;
std::vector<void*> App::callbacks;
InstancesList* App::instancesList = nullptr;
qint64 App::pid = -1;
std::vector<int> App::_scriptsToReset;
VMutex App::_appSemaphore;
std::map<std::string, SSysSemaphore> App::_systemSemaphores;
std::vector<std::string> App::_pluginNames;

long long int App::_nextUniqueId = SIM_UIDSTART;
#ifdef USE_LONG_LONG_HANDLES
long long int App::_nextHandle_object = SIM_IDSTART_SCENEOBJECT;
long long int App::_nextHandle_collection = SIM_IDSTART_COLLECTION;
long long int App::_nextHandle_script = SIM_IDSTART_LUASCRIPT;
long long int App::_nextHandle_stack = SIM_IDSTART_INTERFACESTACK;
long long int App::_nextHandle_texture = SIM_IDSTART_TEXTURE;
long long int App::_nextHandle_mesh = SIM_IDSTART_MESH;
#endif

#ifdef WIN_SIM
LONG WINAPI _winExceptionHandler(PEXCEPTION_POINTERS pExceptionInfo)
{
    void* stack[62];
    HANDLE process = GetCurrentProcess();
    SymInitialize(process, 0, TRUE);
    unsigned short fr = CaptureStackBackTrace(0, 62, stack, nullptr);
    SYMBOL_INFO* symb = (SYMBOL_INFO*)calloc(sizeof(SYMBOL_INFO) + 1024 * sizeof(char), 1);
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
    void* callstack[BACKTRACE_SIZE];
    size_t frames = backtrace(callstack, BACKTRACE_SIZE);
    fprintf(stderr, "\n\nError: signal %d:\n\n", sig);
#ifndef DONT_DEMANGLE_BACKTRACE
    char** strs = backtrace_symbols(callstack, frames);
    for (int i = 0; i < frames; ++i)
    {
        std::string line(strs[i]);
        size_t mangledStart = line.find(
#ifdef MAC_SIM
            " "
#else
            "("
#endif
            "_Z");
        if (mangledStart != std::string::npos)
        {
            mangledStart++;
            size_t mangledEnd = mangledStart;
            while (mangledEnd < line.length() &&
#ifdef MAC_SIM
                   line.at(mangledEnd) != ' '
#else
                   line.at(mangledEnd) != '+' &&
                   line.at(mangledEnd) != ')'
#endif
            )
                mangledEnd++;
            std::string mangled = line.substr(mangledStart, mangledEnd - mangledStart);
            int status;
            char* demangled = abi::__cxa_demangle(mangled.c_str(), nullptr, nullptr, &status);
            if (status == 0)
            {
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

void App::init(const char* appDir, int)
{
    _appStorage = new CPersistentDataContainer("appStorage.dat");
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
    pid = QCoreApplication::applicationPid();
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
    worldContainer->sandboxScript->initScript();

    std::string autoLoadAddOns("true");
    getAppNamedParam("addOns.autoLoad", autoLoadAddOns);
    std::transform(autoLoadAddOns.begin(), autoLoadAddOns.end(), autoLoadAddOns.begin(), [](unsigned char c){ return std::tolower(c); });
    if ( App::userSettings->runAddOns && (autoLoadAddOns == "true") )
        worldContainer->addOnScriptContainer->loadAllFromAddOnFolder();
    worldContainer->addOnScriptContainer->loadAdditionalAddOns();
    worldContainer->addOnScriptContainer->callScripts(sim_syscb_init, nullptr, nullptr);
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
        int r = worldContainer->sandboxScript->executeScriptString(_startupScriptString.c_str(), nullptr);
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
    if (CSimFlavor::getBoolVal(19))
        App::postExitRequest();
}

void App::cleanup()
{
// To print all event field names, that occured in this session:
//    for (const auto& item : CCbor::allEVentFieldNames)
//        std::cout << item << std::endl;
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
    delete _appStorage;
    systemSemaphore(nullptr, false);
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
        CInterfaceStack* stack = worldContainer->interfaceStackContainer->createStack();
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
        CScriptObject* mainScript = currentWorld->sceneObjects->embeddedScriptContainer->getMainScript();
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
        char* data = CALL_C_API_CLEAR_ERRORS(simGetSimulatorMessage, &messageID, auxValues, &dataSize);
        if (messageID != -1)
        {
            if (messageID == sim_message_simulation_start_resume_request)
                CALL_C_API_CLEAR_ERRORS(simStartSimulation);
            if (messageID == sim_message_simulation_pause_request)
                CALL_C_API_CLEAR_ERRORS(simPauseSimulation);
            if (messageID == sim_message_simulation_stop_request)
                CALL_C_API_CLEAR_ERRORS(simStopSimulation);
            if (data != NULL)
                CALL_C_API_CLEAR_ERRORS(simReleaseBuffer, data);
        }
    }

    // Handle a running simulation:
    if (stepIfRunning && (CALL_C_API_CLEAR_ERRORS(simGetSimulationState) & sim_simulation_advancing) != 0)
    {
        if ((!App::currentWorld->simulation->getIsRealTimeSimulation()) || App::currentWorld->simulation->isRealTimeCalculationStepNeeded())
        {
            if ((!worldContainer->shouldTemporarilySuspendMainScript()) || App::currentWorld->simulation->didStopRequestCounterChangeSinceSimulationStart())
            {
                CScriptObject* it = App::currentWorld->sceneObjects->embeddedScriptContainer->getMainScript();
                if (it != nullptr)
                {
                    worldContainer->calcInfo->simulationPassStart();
                    App::currentWorld->sceneObjects->embeddedScriptContainer->broadcastDataContainer.removeTimedOutObjects(App::currentWorld->simulation->getSimulationTime()); // remove invalid elements
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

    // Async reset some scripts:
    for (size_t i = 0; i < _scriptsToReset.size(); i++)
    {
        CScriptObject* it = App::worldContainer->getScriptObjectFromHandle(_scriptsToReset[i]);
        if (it != nullptr)
            it->initScript();
    }
    _scriptsToReset.clear();

    // Keep for backward compatibility:
    if (!currentWorld->simulation->isSimulationRunning()) // when simulation is running, we handle the add-on scripts after the main script was called
        worldContainer->addOnScriptContainer->callScripts(sim_syscb_aos_run_old, nullptr, nullptr);

    simThread->executeMessages(); // rendering, queued command execution, etc.
#ifdef SIM_WITH_GUI
    currentWorld->simulation->showAndHandleEmergencyStopButton(false, ""); // 10/10/2015
#else
    qtApp->processEvents();
#endif
}

long long int App::getFreshUniqueId(int objectType)
{
    long long int uniqueId = -1;
#ifdef USE_LONG_LONG_HANDLES
    if (objectType == sim_objecttype_sceneobject)
        uniqueId = _nextHandle_object++;
    if (objectType == sim_objecttype_collection)
        uniqueId = _nextHandle_collection++;
    if (objectType == sim_objecttype_script)
        uniqueId = _nextHandle_script++;
    if (objectType == sim_objecttype_interfacestack)
        uniqueId = _nextHandle_stack++;
    if (objectType == sim_objecttype_texture)
        uniqueId = _nextHandle_texture++;
    if (objectType == sim_objecttype_mesh)
        uniqueId = _nextHandle_mesh++;
    if (uniqueId != -1)
        currentWorld->registerNewHandle(uniqueId, objectType);
    else
        uniqueId = _nextUniqueId++;
#else
    uniqueId = _nextUniqueId++;
#endif
    return uniqueId;
}

void App::releaseUniqueId(long long int uid, int objectType /*= -1 */)
{
    currentWorld->releaseNewHandle(uid, objectType);
    //    releaseForAppWide(uid, -1);
}

UID App::getNewHandleFromOldHandle(int oldHandle)
{
    UID retVal = oldHandle;
#ifdef USE_LONG_LONG_HANDLES
    if (oldHandle >= 0)
    {
        UID handleFlags = oldHandle & sim_handleflag_flagmask;
        oldHandle = oldHandle & sim_handleflag_handlemask;
        retVal = currentWorld->getNewHandleFromOldHandle(oldHandle);
        if (retVal >= 0)
            retVal = retVal | (handleFlags * 0x100000000);
    }
#endif
    return retVal;
}

int App::getOldHandleFromNewHandle(UID newHandle)
{
    int retVal = int(newHandle);
#ifdef USE_LONG_LONG_HANDLES
    if (newHandle >= 0)
    {
        UID handleFlags = newHandle & 0x3c0000000000000;
        newHandle = newHandle & 0x03fffffffffffff;
        retVal = currentWorld->getOldHandleFromNewHandle(newHandle);
        if (retVal >= 0)
            retVal = retVal | int(handleFlags / 0x100000000);
    }
#endif
    return retVal;
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

void App::setAdditionalAddOnScript1(const char* script)
{
    _additionalAddOnScript1 = script;
}

std::string App::getAdditionalAddOnScript1()
{
    return (_additionalAddOnScript1);
}

void App::setAdditionalAddOnScript2(const char* script)
{
    _additionalAddOnScript2 = script;
}

std::string App::getAdditionalAddOnScript2()
{
    return (_additionalAddOnScript2);
}

bool App::getAppNamedParam(const char* paramName, std::string& param)
{
    bool retVal = false;
    std::map<std::string, std::string>::iterator it = _applicationNamedParams.find(paramName);
    if (it != _applicationNamedParams.end())
    {
        param = it->second;
        retVal = true;
    }
    return retVal;
}

void App::setAppNamedParam(const char* paramName, const char* param, int paramLength)
{
    bool diff = false;
    if (paramLength == 0)
        paramLength = int(strlen(param));
    std::string newVal = std::string(param, param + paramLength);
    std::string oldVal;
    if (getAppNamedParam(paramName, oldVal))
        diff = (newVal != oldVal);
    else
        diff = true;
    if (diff)
    {
        _applicationNamedParams[paramName] = newVal;
        if ((App::worldContainer != nullptr) && App::worldContainer->getEventsEnabled())
        {
            std::string cmd(NAMEDPARAMPREFIX);
            cmd += paramName;
            CCbor* ev = App::worldContainer->createObjectChangedEvent(sim_handle_app, cmd.c_str(), false);
            ev->appendKeyText(cmd.c_str(), param);
            App::worldContainer->pushEvent();
        }
    }
}

bool App::removeAppNamedParam(const char* paramName)
{
    bool retVal = false;
    std::map<std::string, std::string>::iterator it = _applicationNamedParams.find(paramName);
    if (it != _applicationNamedParams.end())
    {
        _applicationNamedParams.erase(it);
        retVal = true;
        if ((App::worldContainer != nullptr) && App::worldContainer->getEventsEnabled())
        {
            std::string cmd(NAMEDPARAMPREFIX);
            cmd += paramName;
            CCbor* ev = App::worldContainer->createObjectChangedEvent(sim_handle_app, cmd.c_str(), false);
            ev->appendKeyNull(cmd.c_str());
            App::worldContainer->pushEvent();
        }
    }
    return retVal;
}

std::string App::getConsoleLogFilter()
{
    return (_consoleLogFilterStr);
}

void App::setConsoleLogFilter(const char* filter)
{
    _consoleLogFilterStr = filter;
}

bool App::logPluginMsg(const char* pluginName, int verbosityLevel, const char* logMsg)
{
    bool retVal = false;

    CPlugin* it = nullptr;

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

void App::logMsg(int verbosityLevel, const char* msg, int int1, int int2 /*=0*/, int int3 /*=0*/)
{
    int realVerbosityLevel = verbosityLevel & 0x0fff;
    if ((_consoleVerbosity >= realVerbosityLevel) || (_statusbarVerbosity >= realVerbosityLevel))
        _logMsg(nullptr, verbosityLevel, msg, int1, int2, int3);
}

void App::logScriptMsg(const CScriptObject* script, int verbosityLevel, const char* msg)
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

int App::getVerbosityLevelFromString(const char* verbosityStr)
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

void App::setConsoleMsgFile(const char* f)
{
    _consoleMsgsFilename = f;
}

void App::logMsg(int verbosityLevel, const char* msg, const char* subStr1, const char* subStr2 /*=nullptr*/,
                 const char* subStr3 /*=nullptr*/)
{
    int realVerbosityLevel = verbosityLevel & 0x0fff;
    if ((_consoleVerbosity >= realVerbosityLevel) || (_statusbarVerbosity >= realVerbosityLevel))
        _logMsg(nullptr, verbosityLevel, msg, subStr1, subStr2, subStr3);
}

void App::logMsg(int verbosityLevel, const char* msg)
{
    int realVerbosityLevel = verbosityLevel & 0x0fff;
    if ((_consoleVerbosity >= realVerbosityLevel) || (_statusbarVerbosity >= realVerbosityLevel))
        __logMsg(nullptr, verbosityLevel, msg);
}

void App::_logMsg(const char* originName, int verbosityLevel, const char* msg, const char* subStr1,
                  const char* subStr2 /*=nullptr*/, const char* subStr3 /*=nullptr*/)
{
    size_t bs = strlen(msg) + 200;
    char* buff = new char[bs];
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

void App::_logMsg(const char* originName, int verbosityLevel, const char* msg, int int1, int int2 /*=0*/,
                  int int3 /*=0*/)
{
    size_t bs = strlen(msg) + 200;
    char* buff = new char[bs];
    snprintf(buff, bs, msg, int1, int2, int3);
    __logMsg(originName, verbosityLevel, buff);
    delete[] buff;
}

std::string App::_getHtmlEscapedString(const char* str)
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

bool App::_consoleLogFilter(const char* msg)
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

static std::string replaceVars(const std::string& format, const std::map<std::string, std::string>& vars)
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

void App::__logMsg(const char* originName, int verbosityLevel, const char* msg, int consoleVerbosity /*=-1*/, int statusbarVerbosity /*=-1*/)
{
    static bool inside = false;
    static int64_t lastTime = 0;
    if (!inside)
    {
        int realVerbosityLevel = verbosityLevel & 0x0fff;

        if (verbosityLevel & sim_verbosity_once)
        {
            if (_logOnceMessages[originName][realVerbosityLevel][msg])
                return;
            else
                _logOnceMessages[originName][realVerbosityLevel][msg] = true;
        }

        if ((worldContainer != nullptr) && VThread::isSimThread())
        {
            std::string orig("CoppeliaSim");
            if (originName != nullptr)
                orig = originName;
            CCbor* ev = worldContainer->createEvent("logMsg", -1, -1, nullptr, false);
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
            statusbarLogFormat = f ? f : "<font color='grey'>[{origin}:{verbosity}]</font>    <font color='{color}'>{message}</font>";
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
        int64_t t = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
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
    bool diff = (_dlgVerbosity != v);
    if (diff)
    {
        _dlgVerbosity = v;
        if ((App::worldContainer != nullptr) && App::worldContainer->getEventsEnabled())
        {
            const char* cmd = propApp_dialogVerbosity.name;
            CCbor* ev = App::worldContainer->createObjectChangedEvent(sim_handle_app, cmd, true);
            ev->appendKeyInt(cmd, _dlgVerbosity);
            App::worldContainer->pushEvent();
        }
    }
}

void App::setStartupScriptString(const char* str)
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

void App::undoRedo_sceneChanged(const char* txt)
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

void App::undoRedo_sceneChangedGradual(const char* txt)
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

int App::getConsoleVerbosity(const char* pluginName /*=nullptr*/)
{ // sim_verbosity_none, etc.
    int retVal = _consoleVerbosity;
    if (pluginName != nullptr)
    {
        CPlugin* pl = worldContainer->pluginContainer->getPluginFromName_old(pluginName, true);
        if (pl != nullptr)
        {
            if (pl->getConsoleVerbosity() != sim_verbosity_useglobal)
                retVal = pl->getConsoleVerbosity();
        }
    }
    return (retVal);
}

void App::setStringVerbosity(int what, const char* str)
{
    int v = sim_verbosity_none;
    if (strcmp(str, "errors") == 0)
        v = sim_verbosity_errors;
    if (strcmp(str, "warnings") == 0)
        v = sim_verbosity_warnings;
    if (strcmp(str, "loadinfos") == 0)
        v = sim_verbosity_loadinfos;
    if (strcmp(str, "questions") == 0)
        v = sim_verbosity_questions;
    if (strcmp(str, "scripterrors") == 0)
        v = sim_verbosity_scripterrors;
    if (strcmp(str, "scriptwarnings") == 0)
        v = sim_verbosity_scriptwarnings;
    if (strcmp(str, "scriptinfos") == 0)
        v = sim_verbosity_scriptinfos;
    if (strcmp(str, "msgs") == 0)
        v = sim_verbosity_msgs;
    if (strcmp(str, "infos") == 0)
        v = sim_verbosity_infos;
    if (strcmp(str, "debug") == 0)
        v = sim_verbosity_debug;
    if (strcmp(str, "trace") == 0)
        v = sim_verbosity_trace;
    if (strcmp(str, "tracelua") == 0)
        v = sim_verbosity_tracelua;
    if (strcmp(str, "traceall") == 0)
        v = sim_verbosity_traceall;
    if (what == 0)
        App::setConsoleVerbosity(v);
    else if (what == 1)
        App::setStatusbarVerbosity(v);
    else
        App::setDlgVerbosity(v);
}

void App::setConsoleVerbosity(int v, const char* pluginName /*=nullptr*/)
{ // sim_verbosity_none, etc.
    if (pluginName != nullptr)
    {
        CPlugin* pl = worldContainer->pluginContainer->getPluginFromName_old(pluginName, true);
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
                const char* cmd = propApp_consoleVerbosity.name;
                CCbor* ev = App::worldContainer->createObjectChangedEvent(sim_handle_app, cmd, true);
                ev->appendKeyInt(cmd, _consoleVerbosity);
                App::worldContainer->pushEvent();
            }
        }
    }
}

int App::getStatusbarVerbosity(const char* pluginName /*=nullptr*/)
{ // sim_verbosity_none, etc.
    int retVal = _statusbarVerbosity;
    if (pluginName != nullptr)
    {
        CPlugin* pl = worldContainer->pluginContainer->getPluginFromName_old(pluginName, true);
        if (pl != nullptr)
        {
            if (pl->getStatusbarVerbosity() != sim_verbosity_useglobal)
                retVal = pl->getStatusbarVerbosity();
        }
    }
    return (retVal);
}

void App::setStatusbarVerbosity(int v, const char* pluginName /*=nullptr*/)
{ // sim_verbosity_none, etc.
    if (pluginName != nullptr)
    {
        CPlugin* pl = worldContainer->pluginContainer->getPluginFromName_old(pluginName, true);
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
                const char* cmd = propApp_statusbarVerbosity.name;
                CCbor* ev = App::worldContainer->createObjectChangedEvent(sim_handle_app, cmd, true);
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
                                        const char* stringP1, const char* stringP2, double executionDelay)
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
    CSceneObject* it;
    it = App::currentWorld->sceneObjects->getObjectFromHandle(objectHandle);
    CSceneObject* parent = it->getParent();
    if (parent != nullptr)
    {
        for (size_t i = 0; i < parent->getChildCount(); i++)
        {
            CSceneObject* child = parent->getChildFromIndex(i);
            if (child->getObjectType() == sim_sceneobject_dummy)
            {
                CDummy* dummy = (CDummy*)child;
                std::string childTag(dummy->getAssemblyTag());
                if (utils::checkAssemblyTagValidity(childTag.c_str(), nullptr))
                { // the parent has at least one valid assembly item

                    for (size_t j = 0; j < it->getChildCount(); j++)
                    {
                        child = it->getChildFromIndex(j);
                        if (child->getObjectType() == sim_sceneobject_dummy)
                        {
                            dummy = (CDummy*)child;
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
    CSceneObject* it1;  // robot dummy
    CSceneObject* it2;  // gripper dummy (or object itself (special case))
    CSceneObject* obj1; // robot part
    CSceneObject* obj2; // gripper base
    it1 = App::currentWorld->sceneObjects->getObjectFromHandle(parentHandle);
    it2 = App::currentWorld->sceneObjects->getObjectFromHandle(childHandle);
    obj1 = it1->getParent();
    obj2 = it2->getParent();
    if ((it1->getObjectType() == sim_sceneobject_dummy) && (obj1 != nullptr))
    { // possibly new method of assembly (via 2 dummies)
        CDummy* dummy1 = (CDummy*)it1;
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
                    CSceneObject* child = obj2->getChildFromIndex(i);
                    if (child->getObjectType() == sim_sceneobject_dummy)
                    {
                        CDummy* dummy = (CDummy*)child;
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
                CDummy* dummy = (CDummy*)it2;
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
                std::vector<CSceneObject*> potParents;
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
            std::vector<CSceneObject*> potParents;
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

int App::setBoolProperty(long long int target, const char* ppName, bool pState)
{
    if ((target == sim_handle_sandbox) && (worldContainer != nullptr) && (worldContainer->sandboxScript != nullptr))
        target = worldContainer->sandboxScript->getScriptHandle();

    int retVal = -1;
    const char* pName = ppName;
    if (target == sim_handle_app)
    {
        if (strcmp(pName, propApp_hierarchyEnabled.name) == 0)
        {
            setHierarchyEnabled(pState);
            retVal = 1;
        }
        else if (strcmp(pName, propApp_browserEnabled.name) == 0)
        {
#ifdef SIM_WITH_GUI
            GuiApp::setBrowserEnabled(pState);
#endif
            retVal = 1;
        }
        else if (strcmp(pName, propApp_displayEnabled.name) == 0)
        {
            setOpenGlDisplayEnabled(pState);
            retVal = 1;
        }
    }
    else if (currentWorld != nullptr)
        retVal = currentWorld->setBoolProperty(target, pName, pState);
    return retVal;
}

int App::getBoolProperty(long long int target, const char* ppName, bool& pState)
{
    if ((target == sim_handle_sandbox) && (worldContainer != nullptr) && (worldContainer->sandboxScript != nullptr))
        target = worldContainer->sandboxScript->getScriptHandle();

    int retVal = -1;
    const char* pName = ppName;
    if (target == sim_handle_app)
    {
        if (strcmp(pName, propApp_hierarchyEnabled.name) == 0)
        {
            pState = getHierarchyEnabled();
            retVal = 1;
        }
        else if (strcmp(pName, propApp_browserEnabled.name) == 0)
        {
#ifdef SIM_WITH_GUI
            pState = GuiApp::getBrowserEnabled();
#else
            pState = false;
#endif
            retVal = 1;
        }
        else if (strcmp(pName, propApp_displayEnabled.name) == 0)
        {
            pState = getOpenGlDisplayEnabled();
            retVal = 1;
        }
        else if (strcmp(pName, propApp_canSave.name) == 0)
        {
            pState = canSave();
            retVal = 1;
        }
        else if (strcmp(pName, propApp_execUnsafe.name) == 0)
        {
            if (userSettings != nullptr)
            {
                pState = userSettings->execUnsafe;
                retVal = 1;
            }
            else
                retVal = 0;
        }
        else if (strcmp(pName, propApp_execUnsafeExt.name) == 0)
        {
            if (userSettings != nullptr)
            {
                pState = userSettings->execUnsafeExt;
                retVal = 1;
            }
            else
                retVal = 0;
        }
    }
    else if (currentWorld != nullptr)
        retVal = currentWorld->getBoolProperty(target, pName, pState);
    return retVal;
}

int App::setIntProperty(long long int target, const char* ppName, int pState)
{
    if ((target == sim_handle_sandbox) && (worldContainer != nullptr) && (worldContainer->sandboxScript != nullptr))
        target = worldContainer->sandboxScript->getScriptHandle();

    int retVal = -1;
    const char* pName = ppName;
    if (target == sim_handle_app)
    {
        if (strcmp(pName, propApp_consoleVerbosity.name) == 0)
        {
            setConsoleVerbosity(pState);
            retVal = 1;
        }
        else if (strcmp(pName, propApp_statusbarVerbosity.name) == 0)
        {
            setStatusbarVerbosity(pState);
            retVal = 1;
        }
        else if (strcmp(pName, propApp_dialogVerbosity.name) == 0)
        {
            setDlgVerbosity(pState);
            retVal = 1;
        }
        else if (strcmp(pName, propApp_idleFps.name) == 0)
        {
            if (userSettings != nullptr)
                userSettings->setIdleFps_session(pState);
            retVal = 1;
        }
    }
    else if (currentWorld != nullptr)
        retVal = currentWorld->setIntProperty(target, pName, pState);
    return retVal;
}

int App::getIntProperty(long long int target, const char* ppName, int& pState)
{
    if ((target == sim_handle_sandbox) && (worldContainer != nullptr) && (worldContainer->sandboxScript != nullptr))
        target = worldContainer->sandboxScript->getScriptHandle();

    int retVal = -1;
    const char* pName = ppName;
    if (target == sim_handle_app)
    {
        if (strcmp(pName, propApp_protocolVersion.name) == 0)
        {
            pState = SIM_EVENT_PROTOCOL_VERSION;
            retVal = 1;
        }
        else if (strcmp(pName, propApp_productVersionNb.name) == 0)
        {
            pState = SIM_PROGRAM_FULL_VERSION_NB;
            retVal = 1;
        }
        else if (strcmp(pName, propApp_platform.name) == 0)
        {
            pState = getPlatform();
            retVal = 1;
        }
        else if (strcmp(pName, propApp_flavor.name) == 0)
        {
#ifdef SIM_FL
            pState = SIM_FL;
#else
            pState = -1;
#endif
            retVal = 1;
        }
        else if (strcmp(pName, propApp_qtVersion.name) == 0)
        {
            pState = (QT_VERSION >> 16) * 10000 + ((QT_VERSION >> 8) & 255) * 100 + (QT_VERSION & 255) * 1;
            retVal = 1;
        }
        else if (strcmp(pName, propApp_processId.name) == 0)
        {
            if (instancesList != nullptr)
                pState = instancesList->thisInstanceId();
            else
                pState = -1;
            retVal = 1;
        }
        else if (strcmp(pName, propApp_processCnt.name) == 0)
        {
            if (instancesList != nullptr)
                pState = instancesList->numInstances();
            else
                pState = -1;
            retVal = 1;
        }
        else if (strcmp(pName, propApp_consoleVerbosity.name) == 0)
        {
            pState = getConsoleVerbosity();
            retVal = 1;
        }
        else if (strcmp(pName, propApp_statusbarVerbosity.name) == 0)
        {
            pState = getStatusbarVerbosity();
            retVal = 1;
        }
        else if (strcmp(pName, propApp_dialogVerbosity.name) == 0)
        {
            pState = getDlgVerbosity();
            retVal = 1;
        }
        else if (strcmp(pName, propApp_headlessMode.name) == 0)
        {
            pState = getHeadlessMode();
            retVal = 1;
        }
        else if (strcmp(pName, propApp_idleFps.name) == 0)
        {
            if (userSettings != nullptr)
                pState = userSettings->getIdleFps();
            else
                pState = -1;
            retVal = 1;
        }
        else if (strcmp(pName, propApp_notifyDeprecated.name) == 0)
        {
            if (userSettings != nullptr)
                pState = userSettings->notifyDeprecated;
            else
                pState = -1;
            retVal = 1;
        }
    }
    else if (currentWorld != nullptr)
        retVal = currentWorld->getIntProperty(target, pName, pState);
    return retVal;
}

int App::setLongProperty(long long int target, const char* ppName, long long int pState)
{
    if ((target == sim_handle_sandbox) && (worldContainer != nullptr) && (worldContainer->sandboxScript != nullptr))
        target = worldContainer->sandboxScript->getScriptHandle();

    int retVal = -1;
    const char* pName = ppName;
    if (target == sim_handle_app)
    {
    }
    else if (currentWorld != nullptr)
        retVal = currentWorld->setLongProperty(target, pName, pState);
    return retVal;
}

int App::getLongProperty(long long int target, const char* ppName, long long int& pState)
{
    if ((target == sim_handle_sandbox) && (worldContainer != nullptr) && (worldContainer->sandboxScript != nullptr))
        target = worldContainer->sandboxScript->getScriptHandle();

    int retVal = -1;
    const char* pName = ppName;
    if (target == sim_handle_app)
    {
        if (strcmp(pName, propApp_pid.name) == 0)
        {
            pState = pid;
            retVal = 1;
        }
    }
    else if (currentWorld != nullptr)
        retVal = currentWorld->getLongProperty(target, pName, pState);
    return retVal;
}

int App::setHandleProperty(long long int target, const char* ppName, long long int pState)
{
    if ((target == sim_handle_sandbox) && (worldContainer != nullptr) && (worldContainer->sandboxScript != nullptr))
        target = worldContainer->sandboxScript->getScriptHandle();

    int retVal = -1;
    const char* pName = ppName;
    if (target == sim_handle_app)
    {
    }
    else if (currentWorld != nullptr)
        retVal = currentWorld->setHandleProperty(target, pName, pState);
    return retVal;
}

int App::getHandleProperty(long long int target, const char* ppName, long long int& pState)
{
    if ((target == sim_handle_sandbox) && (worldContainer != nullptr) && (worldContainer->sandboxScript != nullptr))
        target = worldContainer->sandboxScript->getScriptHandle();

    int retVal = -1;
    const char* pName = ppName;
    if (target == sim_handle_app)
    {
        if (strcmp(pName, propApp_handle.name) == 0)
        {
            pState = sim_handle_app;
            retVal = 1;
        }
        else if (strcmp(pName, propApp_sandbox.name) == 0)
        {
            if ( (worldContainer != nullptr) && (worldContainer->sandboxScript != nullptr) )
                pState = worldContainer->sandboxScript->getScriptHandle();
            else
                pState = -1;
            retVal = 1;
        }
    }
    else if (currentWorld != nullptr)
        retVal = currentWorld->getHandleProperty(target, pName, pState);
    return retVal;
}

int App::setFloatProperty(long long int target, const char* ppName, double pState)
{
    if ((target == sim_handle_sandbox) && (worldContainer != nullptr) && (worldContainer->sandboxScript != nullptr))
        target = worldContainer->sandboxScript->getScriptHandle();

    int retVal = -1;
    const char* pName = ppName;
    if (target == sim_handle_app)
    {
        if (strcmp(pName, propApp_defaultTranslationStepSize.name) == 0)
        {
            if (userSettings != nullptr)
                userSettings->setTranslationStepSize(pState);
            retVal = 1;
        }
        else if (strcmp(pName, propApp_defaultRotationStepSize.name) == 0)
        {
            if (userSettings != nullptr)
                userSettings->setRotationStepSize(pState);
            retVal = 1;
        }
    }
    else if (currentWorld != nullptr)
        retVal = currentWorld->setFloatProperty(target, pName, pState);
    return retVal;
}

int App::getFloatProperty(long long int target, const char* ppName, double& pState)
{
    if ((target == sim_handle_sandbox) && (worldContainer != nullptr) && (worldContainer->sandboxScript != nullptr))
        target = worldContainer->sandboxScript->getScriptHandle();

    int retVal = -1;
    const char* pName = ppName;
    if (target == sim_handle_app)
    {
        if (strcmp(pName, propApp_defaultTranslationStepSize.name) == 0)
        {
            if (userSettings != nullptr)
                pState = userSettings->getTranslationStepSize();
            else
                pState = 0.0;
            retVal = 1;
        }
        else if (strcmp(pName, propApp_defaultRotationStepSize.name) == 0)
        {
            if (userSettings != nullptr)
                pState = userSettings->getRotationStepSize();
            else
                pState = 0.0;
            retVal = 1;
        }
        else if (strcmp(pName, propApp_randomFloat.name) == 0)
        {
            pState = SIM_RAND_FLOAT;
            retVal = 1;
        }
    }
    else if (currentWorld != nullptr)
        retVal = currentWorld->getFloatProperty(target, pName, pState);
    return retVal;
}

int App::setStringProperty(long long int target, const char* ppName, const char* pState)
{
    if ((target == sim_handle_sandbox) && (worldContainer != nullptr) && (worldContainer->sandboxScript != nullptr))
        target = worldContainer->sandboxScript->getScriptHandle();

    int retVal = -1;
    const char* pName = ppName;
    if (target == sim_handle_app)
    {
        std::string pN(pName);
        if (utils::replaceSubstringStart(pN, NAMEDPARAMPREFIX, ""))
        {
            if (pN.size() > 0)
            {
                setAppNamedParam(pN.c_str(), pState);
                retVal = 1;
            }
        }
        if (retVal == -1)
        {
            if (strcmp(pName, propApp_sceneDir.name) == 0)
            {
                if (folders != nullptr)
                    folders->setScenesPath(pState);
                retVal = 1;
            }
            else if (strcmp(pName, propApp_modelDir.name) == 0)
            {
                if (folders != nullptr)
                    folders->setModelsPath(pState);
                retVal = 1;
            }
            else if (strcmp(pName, propApp_importExportDir.name) == 0)
            {
                if (folders != nullptr)
                    folders->setImportExportPath(pState);
                retVal = 1;
            }
            else if (strcmp(pName, propApp_consoleVerbosityStr.name) == 0)
            {
                setStringVerbosity(0, pState);
                retVal = 1;
            }
            else if (strcmp(pName, propApp_statusbarVerbosityStr.name) == 0)
            {
                setStringVerbosity(1, pState);
                retVal = 1;
            }
            else if (strcmp(pName, propApp_dialogVerbosityStr.name) == 0)
            {
                setStringVerbosity(2, pState);
                retVal = 1;
            }
            else if (strcmp(pName, propApp_auxAddOn1.name) == 0)
            {
                setAdditionalAddOnScript1(pState);
                retVal = 1;
            }
            else if (strcmp(pName, propApp_auxAddOn2.name) == 0)
            {
                setAdditionalAddOnScript2(pState);
                retVal = 1;
            }
            else if (strcmp(pName, propApp_startupCode.name) == 0)
            {
                setStartupScriptString(pState);
                retVal = 1;
            }
            else if (strcmp(pName, propApp_appArg1.name) == 0)
            {
                setApplicationArgument(0, pState);
                retVal = 1;
            }
            else if (strcmp(pName, propApp_appArg2.name) == 0)
            {
                setApplicationArgument(1, pState);
                retVal = 1;
            }
            else if (strcmp(pName, propApp_appArg3.name) == 0)
            {
                setApplicationArgument(2, pState);
                retVal = 1;
            }
            else if (strcmp(pName, propApp_appArg4.name) == 0)
            {
                setApplicationArgument(3, pState);
                retVal = 1;
            }
            else if (strcmp(pName, propApp_appArg5.name) == 0)
            {
                setApplicationArgument(4, pState);
                retVal = 1;
            }
            else if (strcmp(pName, propApp_appArg6.name) == 0)
            {
                setApplicationArgument(5, pState);
                retVal = 1;
            }
            else if (strcmp(pName, propApp_appArg7.name) == 0)
            {
                setApplicationArgument(6, pState);
                retVal = 1;
            }
            else if (strcmp(pName, propApp_appArg8.name) == 0)
            {
                setApplicationArgument(7, pState);
                retVal = 1;
            }
            else if (strcmp(pName, propApp_appArg9.name) == 0)
            {
                setApplicationArgument(8, pState);
                retVal = 1;
            }
        }
    }
    else if (currentWorld != nullptr)
        retVal = currentWorld->setStringProperty(target, pName, pState);
    return retVal;
}

int App::getStringProperty(long long int target, const char* ppName, std::string& pState)
{
    if ((target == sim_handle_sandbox) && (worldContainer != nullptr) && (worldContainer->sandboxScript != nullptr))
        target = worldContainer->sandboxScript->getScriptHandle();

    int retVal = -1;
    const char* pName = ppName;
    if (target == sim_handle_app)
    {
        std::string pN(pName);
        if (utils::replaceSubstringStart(pN, NAMEDPARAMPREFIX, ""))
        {
            if (pN.size() > 0)
            {
                if (getAppNamedParam(pN.c_str(), pState))
                    retVal = 1;
            }
        }
        if (retVal == -1)
        {
            if (strcmp(pName, propApp_sessionId.name) == 0)
            {
                if (worldContainer != nullptr)
                    pState = worldContainer->getSessionId();
                else
                    pState = "";
                retVal = 1;
            }
            else if (strcmp(pName, propApp_objectType.name) == 0)
            {
                pState = OBJECT_TYPE;
                retVal = 1;
            }
            else if (strcmp(pName, propApp_objectMetaInfo.name) == 0)
            {
                pState = OBJECT_META_INFO;
                retVal = 1;
            }
            else if (strcmp(pName, propApp_productVersion.name) == 0)
            {
                pState = SIM_VERSION_STR_SHORT;
                retVal = 1;
            }
            else if (strcmp(pName, propApp_appDir.name) == 0)
            {
                if (folders != nullptr)
                    pState = folders->getExecutablePath();
                else
                    pState = "";
                retVal = 1;
            }
            else if (strcmp(pName, propApp_machineId.name) == 0)
            {
                pState = CSimFlavor::getStringVal_int(0, sim_stringparam_machine_id);
                retVal = 1;
            }
            else if (strcmp(pName, propApp_legacyMachineId.name) == 0)
            {
                pState = CSimFlavor::getStringVal_int(0, sim_stringparam_machine_id_legacy);
                retVal = 1;
            }
            else if (strcmp(pName, propApp_tempDir.name) == 0)
            {
                if (folders != nullptr)
                    pState = folders->getTempDataPath();
                else
                    pState = "";
                retVal = 1;
            }
            else if (strcmp(pName, propApp_sceneTempDir.name) == 0)
            {
                if (folders != nullptr)
                    pState = folders->getSceneTempDataPath();
                else
                    pState = "";
                retVal = 1;
            }
            else if (strcmp(pName, propApp_settingsDir.name) == 0)
            {
                if (folders != nullptr)
                    pState = folders->getUserSettingsPath();
                else
                    pState = "";
                retVal = 1;
            }
            else if (strcmp(pName, propApp_luaDir.name) == 0)
            {
                if (folders != nullptr)
                    pState = folders->getLuaPath();
                else
                    pState = "";
                retVal = 1;
            }
            else if (strcmp(pName, propApp_pythonDir.name) == 0)
            {
                if (folders != nullptr)
                    pState = folders->getPythonPath();
                else
                    pState = "";
                retVal = 1;
            }
            else if (strcmp(pName, propApp_mujocoDir.name) == 0)
            {
                if (folders != nullptr)
                    pState = folders->getMujocoPath();
                else
                    pState = "";
                retVal = 1;
            }
            else if (strcmp(pName, propApp_systemDir.name) == 0)
            {
                if (folders != nullptr)
                    pState = folders->getSystemPath();
                else
                    pState = "";
                retVal = 1;
            }
            else if (strcmp(pName, propApp_resourceDir.name) == 0)
            {
                if (folders != nullptr)
                    pState = folders->getResourcesPath();
                else
                    pState = "";
                retVal = 1;
            }
            else if (strcmp(pName, propApp_addOnDir.name) == 0)
            {
                if (folders != nullptr)
                    pState = folders->getAddOnPath();
                else
                    pState = "";
                retVal = 1;
            }
            else if (strcmp(pName, propApp_sceneDir.name) == 0)
            {
                if (folders != nullptr)
                    pState = folders->getScenesPath();
                else
                    pState = "";
                retVal = 1;
            }
            else if (strcmp(pName, propApp_modelDir.name) == 0)
            {
                if (folders != nullptr)
                    pState = folders->getModelsPath();
                else
                    pState = "";
                retVal = 1;
            }
            else if (strcmp(pName, propApp_importExportDir.name) == 0)
            {
                if (folders != nullptr)
                    pState = folders->getImportExportPath();
                else
                    pState = "";
                retVal = 1;
            }
            else if (strcmp(pName, propApp_defaultPython.name) == 0)
            {
                if (userSettings != nullptr)
                    pState = userSettings->defaultPython;
                else
                    pState = "";
                if (pState == "")
                {
                    #ifdef WIN_SIM
                        pState = "py";
                    #else
                        pState = "python3";
                    #endif
                }
                retVal = 1;
            }
            else if (strcmp(pName, propApp_sandboxLang.name) == 0)
            {
                if (userSettings != nullptr)
                    pState = userSettings->preferredSandboxLang;
                else
                    pState = "";
                retVal = 1;
            }
            else if (strcmp(pName, propApp_appArg1.name) == 0)
            {
                pState = getApplicationArgument(0);
                retVal = 1;
            }
            else if (strcmp(pName, propApp_appArg2.name) == 0)
            {
                pState = getApplicationArgument(1);
                retVal = 1;
            }
            else if (strcmp(pName, propApp_appArg3.name) == 0)
            {
                pState = getApplicationArgument(2);
                retVal = 1;
            }
            else if (strcmp(pName, propApp_appArg4.name) == 0)
            {
                pState = getApplicationArgument(3);
                retVal = 1;
            }
            else if (strcmp(pName, propApp_appArg5.name) == 0)
            {
                pState = getApplicationArgument(4);
                retVal = 1;
            }
            else if (strcmp(pName, propApp_appArg6.name) == 0)
            {
                pState = getApplicationArgument(5);
                retVal = 1;
            }
            else if (strcmp(pName, propApp_appArg7.name) == 0)
            {
                pState = getApplicationArgument(6);
                retVal = 1;
            }
            else if (strcmp(pName, propApp_appArg8.name) == 0)
            {
                pState = getApplicationArgument(7);
                retVal = 1;
            }
            else if (strcmp(pName, propApp_appArg9.name) == 0)
            {
                pState = getApplicationArgument(8);
                retVal = 1;
            }
            else if (strcmp(pName, propApp_dongleSerial.name) == 0)
            {
                pState = CSimFlavor::getStringVal(22);
                retVal = 1;
            }
            else if (strcmp(pName, propApp_machineSerialND.name) == 0)
            {
                pState = CSimFlavor::getStringVal(23);
                retVal = 1;
            }
            else if (strcmp(pName, propApp_machineSerial.name) == 0)
            {
                pState = CSimFlavor::getStringVal(24);
                retVal = 1;
            }
            else if (strcmp(pName, propApp_dongleID.name) == 0)
            {
                pState = CSimFlavor::getStringVal(25);
                retVal = 1;
            }
            else if (strcmp(pName, propApp_machineIDX.name) == 0)
            {
                pState = CSimFlavor::getStringVal(26);
                retVal = 1;
            }
            else if (strcmp(pName, propApp_machineID0.name) == 0)
            {
                pState = CSimFlavor::getStringVal(27);
                retVal = 1;
            }
            else if (strcmp(pName, propApp_machineID1.name) == 0)
            {
                pState = CSimFlavor::getStringVal(28);
                retVal = 1;
            }
            else if (strcmp(pName, propApp_machineID2.name) == 0)
            {
                pState = CSimFlavor::getStringVal(29);
                retVal = 1;
            }
            else if (strcmp(pName, propApp_machineID3.name) == 0)
            {
                pState = CSimFlavor::getStringVal(30);
                retVal = 1;
            }
            else if (strcmp(pName, propApp_randomString.name) == 0)
            {
                pState = utils::generateUniqueAlphaNumericString();
                retVal = 1;
            }
            else if (strcmp(pName, propApp_auxAddOn1.name) == 0)
            {
                pState = getAdditionalAddOnScript1();
                retVal = 1;
            }
            else if (strcmp(pName, propApp_auxAddOn2.name) == 0)
            {
                pState = getAdditionalAddOnScript2();
                retVal = 1;
            }
            else if (strcmp(pName, propApp_startupCode.name) == 0)
            {
                pState = _startupScriptString;
                retVal = 1;
            }
        }
    }
    else if (currentWorld != nullptr)
        retVal = currentWorld->getStringProperty(target, pName, pState);
    return retVal;
}

int App::setBufferProperty(long long int target, const char* ppName, const char* buffer, int bufferL)
{
    if ((target == sim_handle_sandbox) && (worldContainer != nullptr) && (worldContainer->sandboxScript != nullptr))
        target = worldContainer->sandboxScript->getScriptHandle();

    int retVal = -1;
    if (buffer == nullptr)
        bufferL = 0;
    const char* pName = ppName;
    if (target == sim_handle_app)
    {
        std::string pN(pName);
        if (utils::replaceSubstringStart(pN, CUSTOMDATAPREFIX, ""))
        {
            if (pN.size() > 0)
            {
                pN += "&customData"; // we add a suffix to separate user and system data
                // CPersistentDataContainer cont("appStorage.dat");
                if (_appStorage->writeData(pN.c_str(), std::string(buffer, buffer + bufferL), true, true))
                {
                    if ((App::worldContainer != nullptr) && App::worldContainer->getEventsEnabled())
                    {
                        CCbor* ev = App::worldContainer->createObjectChangedEvent(sim_handle_app, nullptr, false);
                        _appStorage->appendEventData(pN.c_str(), ev);
                        App::worldContainer->pushEvent();
                    }
                }
                retVal = 1;
            }
        }
        else if (utils::replaceSubstringStart(pN, SIGNALPREFIX, ""))
        {
            if (pN.size() > 0)
            {
                if (worldContainer != nullptr)
                {
                    bool diff = worldContainer->customAppData_volatile.setData(pN.c_str(), buffer, bufferL, true);
                    if (diff && worldContainer->getEventsEnabled())
                    {
                        CCbor* ev = worldContainer->createObjectChangedEvent(sim_handle_app, nullptr, false);
                        worldContainer->customAppData_volatile.appendEventData(pN.c_str(), ev);
                        worldContainer->pushEvent();
                    }
                    retVal = 1;
                }
                else
                    retVal = 0;
            }
        }
    }
    else if (currentWorld != nullptr)
        retVal = currentWorld->setBufferProperty(target, pName, buffer, bufferL);
    return retVal;
}

int App::getBufferProperty(long long int target, const char* ppName, std::string& pState)
{
    if ((target == sim_handle_sandbox) && (worldContainer != nullptr) && (worldContainer->sandboxScript != nullptr))
        target = worldContainer->sandboxScript->getScriptHandle();

    int retVal = -1;
    const char* pName = ppName;
    if (target == sim_handle_app)
    {
        std::string pN(pName);
        if (utils::replaceSubstringStart(pN, CUSTOMDATAPREFIX, ""))
        {
            pN += "&customData"; // we add a suffix to separate user and system data
            if (pN.size() > 0)
            {
                //CPersistentDataContainer cont("appStorage.dat");
                if (_appStorage->readData(pN.c_str(), pState))
                    retVal = 1;
            }
        }
        else if (utils::replaceSubstringStart(pN, SIGNALPREFIX, ""))
        {
            if (pN.size() > 0)
            {
                if (worldContainer != nullptr)
                {
                    if (worldContainer->customAppData_volatile.hasData(pN.c_str(), false) >= 0)
                    {
                        pState = worldContainer->customAppData_volatile.getData(pN.c_str());
                        retVal = 1;
                    }
                }
            }
        }
    }
    else if (currentWorld != nullptr)
        retVal = currentWorld->getBufferProperty(target, pName, pState);
    return retVal;
}

int App::setIntArray2Property(long long int target, const char* ppName, const int* pState)
{
    if ((target == sim_handle_sandbox) && (worldContainer != nullptr) && (worldContainer->sandboxScript != nullptr))
        target = worldContainer->sandboxScript->getScriptHandle();

    int retVal = -1;
    const char* pName = ppName;
    if (target == sim_handle_app)
    {
    }
    else if (currentWorld != nullptr)
        retVal = currentWorld->setIntArray2Property(target, pName, pState);
    return retVal;
}

int App::getIntArray2Property(long long int target, const char* ppName, int* pState)
{
    if ((target == sim_handle_sandbox) && (worldContainer != nullptr) && (worldContainer->sandboxScript != nullptr))
        target = worldContainer->sandboxScript->getScriptHandle();

    int retVal = -1;
    const char* pName = ppName;
    if (target == sim_handle_app)
    {
    }
    else if (currentWorld != nullptr)
        retVal = currentWorld->getIntArray2Property(target, pName, pState);
    return retVal;
}

int App::setVector2Property(long long int target, const char* ppName, const double* pState)
{
    if ((target == sim_handle_sandbox) && (worldContainer != nullptr) && (worldContainer->sandboxScript != nullptr))
        target = worldContainer->sandboxScript->getScriptHandle();

    int retVal = -1;
    const char* pName = ppName;
    if (target == sim_handle_app)
    {
    }
    else if (currentWorld != nullptr)
        retVal = currentWorld->setVector2Property(target, pName, pState);
    return retVal;
}

int App::getVector2Property(long long int target, const char* ppName, double* pState)
{
    if ((target == sim_handle_sandbox) && (worldContainer != nullptr) && (worldContainer->sandboxScript != nullptr))
        target = worldContainer->sandboxScript->getScriptHandle();

    int retVal = -1;
    const char* pName = ppName;
    if (target == sim_handle_app)
    {
    }
    else if (currentWorld != nullptr)
        retVal = currentWorld->getVector2Property(target, pName, pState);
    return retVal;
}

int App::setVector3Property(long long int target, const char* ppName, const C3Vector& pState)
{
    if ((target == sim_handle_sandbox) && (worldContainer != nullptr) && (worldContainer->sandboxScript != nullptr))
        target = worldContainer->sandboxScript->getScriptHandle();

    int retVal = -1;
    const char* pName = ppName;
    if (target == sim_handle_app)
    {
    }
    else if (currentWorld != nullptr)
        retVal = currentWorld->setVector3Property(target, pName, pState);
    return retVal;
}

int App::getVector3Property(long long int target, const char* ppName, C3Vector& pState)
{
    if ((target == sim_handle_sandbox) && (worldContainer != nullptr) && (worldContainer->sandboxScript != nullptr))
        target = worldContainer->sandboxScript->getScriptHandle();

    int retVal = -1;
    const char* pName = ppName;
    if (target == sim_handle_app)
    {
    }
    else if (currentWorld != nullptr)
        retVal = currentWorld->getVector3Property(target, pName, pState);
    return retVal;
}

int App::setQuaternionProperty(long long int target, const char* ppName, const C4Vector& pState)
{
    if ((target == sim_handle_sandbox) && (worldContainer != nullptr) && (worldContainer->sandboxScript != nullptr))
        target = worldContainer->sandboxScript->getScriptHandle();

    int retVal = -1;
    const char* pName = ppName;
    if (target == sim_handle_app)
    {
    }
    else if (currentWorld != nullptr)
        retVal = currentWorld->setQuaternionProperty(target, pName, pState);
    return retVal;
}

int App::getQuaternionProperty(long long int target, const char* ppName, C4Vector& pState)
{
    if ((target == sim_handle_sandbox) && (worldContainer != nullptr) && (worldContainer->sandboxScript != nullptr))
        target = worldContainer->sandboxScript->getScriptHandle();

    int retVal = -1;
    const char* pName = ppName;
    if (target == sim_handle_app)
    {
        if (strcmp(pName, propApp_randomQuaternion.name) == 0)
        {
            pState.buildRandomOrientation();
            retVal = 1;
        }
    }
    else if (currentWorld != nullptr)
        retVal = currentWorld->getQuaternionProperty(target, pName, pState);
    return retVal;
}

int App::setPoseProperty(long long int target, const char* ppName, const C7Vector& pState)
{
    if ((target == sim_handle_sandbox) && (worldContainer != nullptr) && (worldContainer->sandboxScript != nullptr))
        target = worldContainer->sandboxScript->getScriptHandle();

    int retVal = -1;
    const char* pName = ppName;
    if (target == sim_handle_app)
    {
    }
    else if (currentWorld != nullptr)
        retVal = currentWorld->setPoseProperty(target, pName, pState);
    return retVal;
}

int App::getPoseProperty(long long int target, const char* ppName, C7Vector& pState)
{
    if ((target == sim_handle_sandbox) && (worldContainer != nullptr) && (worldContainer->sandboxScript != nullptr))
        target = worldContainer->sandboxScript->getScriptHandle();

    int retVal = -1;
    const char* pName = ppName;
    if (target == sim_handle_app)
    {
    }
    else if (currentWorld != nullptr)
        retVal = currentWorld->getPoseProperty(target, pName, pState);
    return retVal;
}

int App::setColorProperty(long long int target, const char* ppName, const float* pState)
{
    if ((target == sim_handle_sandbox) && (worldContainer != nullptr) && (worldContainer->sandboxScript != nullptr))
        target = worldContainer->sandboxScript->getScriptHandle();

    int retVal = -1;
    const char* pName = ppName;
    if (target == sim_handle_app)
    {
    }
    else if (currentWorld != nullptr)
        retVal = currentWorld->setColorProperty(target, pName, pState);
    return retVal;
}

int App::getColorProperty(long long int target, const char* ppName, float* pState)
{
    if ((target == sim_handle_sandbox) && (worldContainer != nullptr) && (worldContainer->sandboxScript != nullptr))
        target = worldContainer->sandboxScript->getScriptHandle();

    int retVal = -1;
    const char* pName = ppName;
    if (target == sim_handle_app)
    {
    }
    else if (currentWorld != nullptr)
        retVal = currentWorld->getColorProperty(target, pName, pState);
    return retVal;
}

int App::setFloatArrayProperty(long long int target, const char* ppName, const double* v, int vL)
{
    if ((target == sim_handle_sandbox) && (worldContainer != nullptr) && (worldContainer->sandboxScript != nullptr))
        target = worldContainer->sandboxScript->getScriptHandle();

    int retVal = -1;
    const char* pName = ppName;
    if (target == sim_handle_app)
    {
    }
    else if (currentWorld != nullptr)
        retVal = currentWorld->setFloatArrayProperty(target, pName, v, vL);
    return retVal;
}

int App::getFloatArrayProperty(long long int target, const char* ppName, std::vector<double>& pState)
{
    if ((target == sim_handle_sandbox) && (worldContainer != nullptr) && (worldContainer->sandboxScript != nullptr))
        target = worldContainer->sandboxScript->getScriptHandle();

    int retVal = -1;
    pState.clear();
    const char* pName = ppName;
    if (target == sim_handle_app)
    {
    }
    else if (currentWorld != nullptr)
        retVal = currentWorld->getFloatArrayProperty(target, pName, pState);
    return retVal;
}

int App::setIntArrayProperty(long long int target, const char* ppName, const int* v, int vL)
{
    if ((target == sim_handle_sandbox) && (worldContainer != nullptr) && (worldContainer->sandboxScript != nullptr))
        target = worldContainer->sandboxScript->getScriptHandle();

    int retVal = -1;
    const char* pName = ppName;
    if (target == sim_handle_app)
    {
    }
    else if (currentWorld != nullptr)
        retVal = currentWorld->setIntArrayProperty(target, pName, v, vL);
    return retVal;
}

int App::getIntArrayProperty(long long int target, const char* ppName, std::vector<int>& pState)
{
    if ((target == sim_handle_sandbox) && (worldContainer != nullptr) && (worldContainer->sandboxScript != nullptr))
        target = worldContainer->sandboxScript->getScriptHandle();

    int retVal = -1;
    pState.clear();
    const char* pName = ppName;
    if (target == sim_handle_app)
    {
    }
    else if (currentWorld != nullptr)
        retVal = currentWorld->getIntArrayProperty(target, pName, pState);
    return retVal;
}

int App::setHandleArrayProperty(long long int target, const char* ppName, const long long int* v, int vL)
{
    if ((target == sim_handle_sandbox) && (worldContainer != nullptr) && (worldContainer->sandboxScript != nullptr))
        target = worldContainer->sandboxScript->getScriptHandle();

    int retVal = -1;
    const char* pName = ppName;
    if (target == sim_handle_app)
    {
    }
    else if (currentWorld != nullptr)
        retVal = currentWorld->setHandleArrayProperty(target, pName, v, vL);
    return retVal;
}

int App::getHandleArrayProperty(long long int target, const char* ppName, std::vector<long long int>& pState)
{
    if ((target == sim_handle_sandbox) && (worldContainer != nullptr) && (worldContainer->sandboxScript != nullptr))
        target = worldContainer->sandboxScript->getScriptHandle();

    int retVal = -1;
    pState.clear();
    const char* pName = ppName;
    if (target == sim_handle_app)
    {
        if (strcmp(pName, propApp_addOns.name) == 0)
        {
            std::vector<int> addOnList(worldContainer->addOnScriptContainer->getAddOnHandles());
            for (size_t i = 0; i < addOnList.size(); i++)
                pState.push_back(addOnList[i]);
            retVal = 1;
        }
    }
    else if (currentWorld != nullptr)
        retVal = currentWorld->getHandleArrayProperty(target, pName, pState);
    return retVal;
}

int App::setStringArrayProperty(long long int target, const char* ppName, const std::vector<std::string>& pState)
{
//    if ((target == sim_handle_sandbox) && (worldContainer != nullptr) && (worldContainer->sandboxScript != nullptr))
//        target = worldContainer->sandboxScript->getScriptHandle();

    int retVal = -1;
    const char* pName = ppName;
    if (target == sim_handle_app)
    {
        if (strcmp(pName, propApp_appArgs.name) == 0)
        {
            std::vector<std::string> ps(pState);
            ps.resize(9);
            for (size_t i = 0; i < ps.size(); i++)
                setApplicationArgument(i, ps[i]);
            retVal = 1;
        }
    }
    else if (currentWorld != nullptr)
        retVal = currentWorld->setStringArrayProperty(target, pName, pState);
    return retVal;
}

int App::getStringArrayProperty(long long int target, const char* ppName, std::vector<std::string>& pState)
{
//    if ((target == sim_handle_sandbox) && (worldContainer != nullptr) && (worldContainer->sandboxScript != nullptr))
//        target = worldContainer->sandboxScript->getScriptHandle();

    int retVal = -1;
    pState.clear();
    const char* pName = ppName;
    if (target == sim_handle_app)
    {
        if (strcmp(pName, propApp_appArgs.name) == 0)
        {
            pState = _applicationArguments;
            retVal = 1;
        }
        else if (strcmp(pName, propApp_plugins.name) == 0)
        {
            pState = _pluginNames;
            retVal = 1;
        }
    }
    else if (currentWorld != nullptr)
        retVal = currentWorld->getStringArrayProperty(target, pName, pState);
    return retVal;
}

int App::removeProperty(long long int target, const char* ppName)
{
    if ((target == sim_handle_sandbox) && (worldContainer != nullptr) && (worldContainer->sandboxScript != nullptr))
        target = worldContainer->sandboxScript->getScriptHandle();

    int retVal = -1;
    const char* pName = ppName;
    if (target == sim_handle_app)
    {
        std::string pN(pName);
        if (utils::replaceSubstringStart(pN, CUSTOMDATAPREFIX, ""))
        {
            pN += "&customData"; // we add a suffix to separate user and system data
            //CPersistentDataContainer cont("appStorage.dat");
            int tp = _appStorage->hasData(pN.c_str(), true);
            if (tp >= 0)
            {
                if (_appStorage->clearData((propertyStrings[tp] + pN).c_str(), true))
                {
                    if ((App::worldContainer != nullptr) && App::worldContainer->getEventsEnabled())
                    {
                        CCbor* ev = App::worldContainer->createObjectChangedEvent(sim_handle_app, nullptr, false);
                        _appStorage->appendEventData(pN.c_str(), ev, true);
                        App::worldContainer->pushEvent();
                    }
                }
                retVal = 1;
            }
        }
        else if (utils::replaceSubstringStart(pN, SIGNALPREFIX, ""))
        {
            if (pN.size() > 0)
            {
                if (worldContainer != nullptr)
                {
                    int tp = worldContainer->customAppData_volatile.hasData(pN.c_str(), true);
                    if (tp >= 0)
                    {
                        bool diff = worldContainer->customAppData_volatile.clearData((propertyStrings[tp] + pN).c_str());
                        if (diff && worldContainer->getEventsEnabled())
                        {
                            CCbor* ev = worldContainer->createObjectChangedEvent(sim_handle_app, nullptr, false);
                            worldContainer->customAppData_volatile.appendEventData(pN.c_str(), ev, true);
                            worldContainer->pushEvent();
                        }
                        retVal = 1;
                    }
                }
            }
        }
        else if (utils::replaceSubstringStart(pN, NAMEDPARAMPREFIX, ""))
        {
            if (pN.size() > 0)
            {
                if (removeAppNamedParam(pN.c_str()))
                    retVal = 1;
                else
                    retVal = 0;
            }
        }
    }
    else if (currentWorld != nullptr)
        retVal = currentWorld->removeProperty(target, pName);
    return retVal;
}

int App::getPropertyName(long long int target, int& index, std::string& pName, std::string& appartenance, bool staticParsing, int excludeFlags)
{
    if ((target == sim_handle_sandbox) && (worldContainer != nullptr) && (worldContainer->sandboxScript != nullptr))
        target = worldContainer->sandboxScript->getScriptHandle();
    int retVal = -1;
    appartenance = OBJECT_TYPE;
    if (target == sim_handle_app)
    {
        for (size_t i = 0; i < allProps_app.size(); i++)
        {
            if ((pName.size() == 0) || utils::startsWith(allProps_app[i].name, pName.c_str()))
            {
                if ((allProps_app[i].flags & excludeFlags) == 0)
                {
                    index--;
                    if (index == -1)
                    {
                        pName = allProps_app[i].name;
                        retVal = 1;
                        break;
                    }
                }
            }
        }
        if (!staticParsing)
        {
            if (retVal == -1)
            {
                //CPersistentDataContainer cont("appStorage.dat");
                if (_appStorage->getPropertyName(index, pName, excludeFlags))
                {
                    pName = CUSTOMDATAPREFIX + pName;
                    retVal = 1;
                }
            }
            if ((retVal == -1) && (worldContainer != nullptr))
            {
                if (worldContainer->customAppData_volatile.getPropertyName(index, pName, excludeFlags))
                {
                    pName = SIGNALPREFIX + pName;
                    retVal = 1;
                }
            }
            if (retVal == -1)
            {
                for (const auto& pair : _applicationNamedParams)
                {
                    int flags = NAMEDPARAMFLAGS;
                    if (pair.second.size() > LARGE_PROPERTY_SIZE)
                        flags |= sim_propertyinfo_largedata;
                    if ((pName.size() == 0) || utils::startsWith((NAMEDPARAMPREFIX + pair.first).c_str(), pName.c_str()))
                    {
                        if ((flags & excludeFlags) == 0)
                        {
                            index--;
                            if (index == -1)
                            {
                                pName = NAMEDPARAMPREFIX + pair.first;
                                retVal = 1;
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
    else if (currentWorld != nullptr)
    {
        CWorld* cw = nullptr;
        if (!staticParsing)
            cw = currentWorld;
        retVal = CWorld::getPropertyName(target, index, pName, appartenance, cw, excludeFlags);
    }
    if (retVal == 1)
    { // Following needed to accomodate for Lua's object representation
        if (pName == "objectType")
            appartenance = "object";
        else if (pName == "objectMetaInfo")
            appartenance = "object";
        else if (pName == "handle")
            appartenance = "object";
    }
    return retVal;
}

int App::getPropertyInfo(long long int target, const char* ppName, int& info, std::string& infoTxt, bool staticParsing)
{
    if ((target == sim_handle_sandbox) && (worldContainer != nullptr) && (worldContainer->sandboxScript != nullptr))
        target = worldContainer->sandboxScript->getScriptHandle();

    int retVal = -1;
    const char* pName = ppName;
    if (target == sim_handle_app)
    {
        for (size_t i = 0; i < allProps_app.size(); i++)
        {
            if (strcmp(allProps_app[i].name, pName) == 0)
            {
                retVal = allProps_app[i].type;
                info = allProps_app[i].flags;
                if (infoTxt == "j")
                    infoTxt = allProps_app[i].shortInfoTxt;
                else
                {
                    auto w = QJsonDocument::fromJson(allProps_app[i].shortInfoTxt).object();
                    std::string descr = w["description"].toString().toStdString();
                    std::string label = w["label"].toString().toStdString();
                    if ( (infoTxt == "s") || (descr == "") )
                        infoTxt = label;
                    else
                        infoTxt = descr;
                }
                break;
            }
        }
        if (!staticParsing)
        {
            if (retVal == -1)
            {
                std::string pN(pName);
                if (utils::replaceSubstringStart(pN, CUSTOMDATAPREFIX, ""))
                {
                    pN += "&customData";
                    //CPersistentDataContainer cont("appStorage.dat");
                    int s;
                    retVal = _appStorage->hasData(pN.c_str(), true, &s);
                    if (retVal >= 0)
                    {
                        info = CUSTOMDATAFLAGS;
                        if (s > LARGE_PROPERTY_SIZE)
                            info = info | sim_propertyinfo_largedata;
                        infoTxt = "";
                    }
                }
            }
            if ((retVal == -1) && (worldContainer != nullptr))
            {
                std::string pN(pName);
                if (utils::replaceSubstringStart(pN, SIGNALPREFIX, ""))
                {
                    int s;
                    retVal = worldContainer->customAppData_volatile.hasData(pN.c_str(), true, &s);
                    if (retVal >= 0)
                    {
                        info = SIGNALFLAGS;
                        if (s > LARGE_PROPERTY_SIZE)
                            info = info | sim_propertyinfo_largedata;
                        infoTxt = "";
                    }
                }
            }
            if (retVal == -1)
            {
                std::string pN(pName);
                pN.erase(0, 11);
                if (pN.size() > 0)
                {
                    std::string param;
                    if (getAppNamedParam(pN.c_str(), param))
                    {
                        retVal = sim_propertytype_string;
                        info = NAMEDPARAMFLAGS;
                        if (param.size() > LARGE_PROPERTY_SIZE)
                            info = info | 0x100;
                        infoTxt = "";
                    }
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
            const char* cmd = propApp_hierarchyEnabled.name;
            CCbor* ev = App::worldContainer->createObjectChangedEvent(sim_handle_app, cmd, true);
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
            const char* cmd = propApp_displayEnabled.name;
            CCbor* ev = App::worldContainer->createObjectChangedEvent(sim_handle_app, cmd, true);
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

int App::getHeadlessMode()
{ // 0: no, 1: suppressed GUI, 2: true headless mode
    int retVal = 2;
#ifdef SIM_WITH_GUI
    retVal = 1;
    if (GuiApp::mainWindow != nullptr)
        retVal = 0;
#endif
    return retVal;
}

bool App::canSave()
{
    bool retVal = false;
    if (App::currentWorld->simulation->isSimulationStopped() && CSimFlavor::getBoolVal(16))
    {
        retVal = true;
#ifdef SIM_WITH_GUI
        if (GuiApp::getEditModeType() != NO_EDIT_MODE)
            retVal = false;
#endif
    }
    return retVal;
}

void App::asyncResetScript(int scriptHandle)
{
    _scriptsToReset.push_back(scriptHandle);
}

bool App::appSemaphore(bool acquire, bool block /*= true*/)
{
    bool retVal = true;
    if (acquire)
    {
        if (block)
            _appSemaphore.lock();
        else
            retVal = _appSemaphore.tryLock();
    }
    else
        _appSemaphore.unlock();
    return retVal;
}

bool App::systemSemaphore(const char* key, bool acquire)
{
    bool retVal = true;
    if (key == nullptr)
    {
        if (acquire)
            retVal = false;
        else
        {
            for (auto sem = _systemSemaphores.begin(); sem != _systemSemaphores.end(); ++sem)
            {
                QSystemSemaphore* semaphore = sem->second.semaphore;
                semaphore->release();
                delete semaphore;
            }
            _systemSemaphores.clear();
        }
    }
    else
    {
        auto sem = _systemSemaphores.find(key);
        if (acquire)
        {
            if (sem == _systemSemaphores.end())
            {
                QSystemSemaphore* semaphore = new QSystemSemaphore(key, 1, QSystemSemaphore::Open);
                semaphore->acquire();
                SSysSemaphore s;
                s.semaphore = semaphore;
                s.cnt = 1;
                _systemSemaphores[key] = s;
            }
            else
                sem->second.cnt++;
        }
        else
        {
            if (sem != _systemSemaphores.end())
            {
                if (sem->second.cnt > 1)
                    sem->second.cnt--;
                else
                {
                    QSystemSemaphore* semaphore = sem->second.semaphore;
                    semaphore->release();
                    delete semaphore;
                    _systemSemaphores.erase(key);
                }
            }
            else
                retVal = false;
        }
    }
    return retVal;
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

void App::pushGenesisEvents()
{
    if ((worldContainer != nullptr) && worldContainer->getEventsEnabled())
    {
        CSimFlavor::getStringVal(22); // trigger calculations and print mids
        CCbor* ev = worldContainer->createEvent(EVENTTYPE_GENESISBEGIN, -1, -1, nullptr, false);
        worldContainer->pushEvent();

#if SIM_EVENT_PROTOCOL_VERSION == 2
        ev = worldContainer->createEvent("appSession", sim_handle_app, sim_handle_app, nullptr, false);
#else
        ev = worldContainer->createEvent(EVENTTYPE_OBJECTCHANGED, sim_handle_app, sim_handle_app, nullptr, false);
#endif
        ev->appendKeyText(propApp_sessionId.name, worldContainer->getSessionId().c_str());
        ev->appendKeyText(propApp_objectType.name, OBJECT_TYPE.c_str());
        ev->appendKeyInt(propApp_protocolVersion.name, SIM_EVENT_PROTOCOL_VERSION);
        ev->appendKeyText(propApp_productVersion.name, SIM_VERSION_STR_SHORT);
        ev->appendKeyInt(propApp_productVersionNb.name, SIM_PROGRAM_FULL_VERSION_NB);
        ev->appendKeyInt(propApp_platform.name, getPlatform());
#ifdef SIM_FL
        ev->appendKeyInt(propApp_flavor.name, SIM_FL);
#else
        ev->appendKeyInt(propApp_flavor.name, -1);
#endif
        ev->appendKeyInt(propApp_qtVersion.name, (QT_VERSION >> 16) * 10000 + ((QT_VERSION >> 8) & 255) * 100 + (QT_VERSION & 255) * 1);
        int sbh = -1;
        if (worldContainer->sandboxScript != nullptr)
            sbh = worldContainer->sandboxScript->getScriptHandle();
        ev->appendKeyInt(propApp_sandbox.name, sbh);
        if (instancesList != nullptr)
        {
            ev->appendKeyInt(propApp_processId.name, instancesList->thisInstanceId());
            ev->appendKeyInt(propApp_processCnt.name, instancesList->numInstances());
        }
        ev->appendKeyInt(propApp_consoleVerbosity.name, getConsoleVerbosity());
        ev->appendKeyInt(propApp_statusbarVerbosity.name, getStatusbarVerbosity());
        ev->appendKeyInt(propApp_dialogVerbosity.name, getDlgVerbosity());
        ev->appendKeyTextArray(propApp_appArgs.name, _applicationArguments);
        ev->appendKeyTextArray(propApp_plugins.name, _pluginNames);
        std::vector<int> addOnList(worldContainer->addOnScriptContainer->getAddOnHandles());
        ev->appendKeyIntArray(propApp_addOns.name, addOnList.data(), addOnList.size());

        for (const auto& pair : _applicationNamedParams)
        {
            std::string t(NAMEDPARAMPREFIX);
            t += pair.first;
            ev->appendKeyText(t.c_str(), pair.second.c_str());
        }

        if (folders != nullptr)
        {
            ev->appendKeyText(propApp_appDir.name, folders->getExecutablePath().c_str());
            ev->appendKeyText(propApp_tempDir.name, folders->getTempDataPath().c_str());
            ev->appendKeyText(propApp_sceneTempDir.name, folders->getSceneTempDataPath().c_str());
            ev->appendKeyText(propApp_settingsDir.name, folders->getUserSettingsPath().c_str());
            ev->appendKeyText(propApp_luaDir.name, folders->getLuaPath().c_str());
            ev->appendKeyText(propApp_pythonDir.name, folders->getPythonPath().c_str());
            ev->appendKeyText(propApp_mujocoDir.name, folders->getMujocoPath().c_str());
            ev->appendKeyText(propApp_systemDir.name, folders->getSystemPath().c_str());
            ev->appendKeyText(propApp_resourceDir.name, folders->getResourcesPath().c_str());
            ev->appendKeyText(propApp_addOnDir.name, folders->getAddOnPath().c_str());
            ev->appendKeyText(propApp_sceneDir.name, folders->getScenesPath().c_str());
            ev->appendKeyText(propApp_modelDir.name, folders->getModelsPath().c_str());
            ev->appendKeyText(propApp_importExportDir.name, folders->getImportExportPath().c_str());
        }

        worldContainer->customAppData_volatile.appendEventData(nullptr, ev);

        if (userSettings != nullptr)
        {
            ev->appendKeyText(propApp_defaultPython.name, userSettings->defaultPython.c_str());
            ev->appendKeyText(propApp_sandboxLang.name, userSettings->preferredSandboxLang.c_str());
        }
#if SIM_EVENT_PROTOCOL_VERSION == 2
        worldContainer->pushEvent();
        ev = worldContainer->createEvent("appSettingsChanged", sim_handle_app, sim_handle_app, nullptr, false);
#endif
        if (userSettings != nullptr)
        {
            ev->appendKeyDouble(propApp_defaultTranslationStepSize.name, userSettings->getTranslationStepSize());
            ev->appendKeyDouble(propApp_defaultRotationStepSize.name, userSettings->getRotationStepSize());
            ev->appendKeyInt(propApp_notifyDeprecated.name, userSettings->notifyDeprecated);
            ev->appendKeyBool(propApp_execUnsafe.name, userSettings->execUnsafe);
            ev->appendKeyBool(propApp_execUnsafeExt.name, userSettings->execUnsafeExt);
            ev->appendKeyText(propApp_dongleSerial.name, CSimFlavor::getStringVal(22).c_str());
            ev->appendKeyText(propApp_machineSerialND.name, CSimFlavor::getStringVal(23).c_str());
            ev->appendKeyText(propApp_machineSerial.name, CSimFlavor::getStringVal(24).c_str());
            ev->appendKeyText(propApp_dongleID.name, CSimFlavor::getStringVal(25).c_str());
            ev->appendKeyText(propApp_machineIDX.name, CSimFlavor::getStringVal(26).c_str());
            ev->appendKeyText(propApp_machineID0.name, CSimFlavor::getStringVal(27).c_str());
            ev->appendKeyText(propApp_machineID1.name, CSimFlavor::getStringVal(28).c_str());
            ev->appendKeyText(propApp_machineID2.name, CSimFlavor::getStringVal(29).c_str());
            ev->appendKeyText(propApp_machineID3.name, CSimFlavor::getStringVal(30).c_str());
        }
#ifdef SIM_WITH_GUI
        ev->appendKeyBool(propApp_browserEnabled.name, GuiApp::getBrowserEnabled());
#else
        ev->appendKeyBool(propApp_browserEnabled.name, false);
#endif
        ev->appendKeyBool(propApp_hierarchyEnabled.name, getHierarchyEnabled());
        ev->appendKeyBool(propApp_displayEnabled.name, getOpenGlDisplayEnabled());
        ev->appendKeyInt(propApp_headlessMode.name, getHeadlessMode());
        ev->appendKeyText(propApp_appArg1.name, getApplicationArgument(0).c_str());
        ev->appendKeyText(propApp_appArg2.name, getApplicationArgument(1).c_str());
        ev->appendKeyText(propApp_appArg3.name, getApplicationArgument(2).c_str());
        ev->appendKeyText(propApp_appArg4.name, getApplicationArgument(3).c_str());
        ev->appendKeyText(propApp_appArg5.name, getApplicationArgument(4).c_str());
        ev->appendKeyText(propApp_appArg6.name, getApplicationArgument(5).c_str());
        ev->appendKeyText(propApp_appArg7.name, getApplicationArgument(6).c_str());
        ev->appendKeyText(propApp_appArg8.name, getApplicationArgument(7).c_str());
        ev->appendKeyText(propApp_appArg9.name, getApplicationArgument(8).c_str());
        ev->appendKeyInt(propApp_pid.name, pid);

        if (userSettings != nullptr)
            ev->appendKeyInt(propApp_idleFps.name, userSettings->getIdleFps());

        worldContainer->pushEvent();

        ev = worldContainer->createEvent(EVENTTYPE_OBJECTCHANGED, sim_handle_app, sim_handle_app, nullptr, false);
        //CPersistentDataContainer cont("appStorage.dat");
        _appStorage->appendEventData(nullptr, ev);
        worldContainer->pushEvent();

        if (worldContainer->sandboxScript != nullptr)
            worldContainer->sandboxScript->pushObjectCreationEvent();

        if (worldContainer->addOnScriptContainer != nullptr)
            worldContainer->addOnScriptContainer->pushGenesisEvents();

        currentWorld->pushGenesisEvents();

        ev = worldContainer->createEvent(EVENTTYPE_GENESISEND, -1, -1, nullptr, false);
        worldContainer->pushEvent();
    }
}

void App::setPluginList(const std::vector<CPlugin*>* plugins)
{
    std::vector<std::string> oldNames(_pluginNames);
    _pluginNames.clear();
    for (size_t i = 0; i < plugins->size(); i++)
        _pluginNames.push_back(plugins->at(i)->getName());
    if (_pluginNames != oldNames)
    {
        if ((App::worldContainer != nullptr) && App::worldContainer->getEventsEnabled())
        {
            const char* cmd = propApp_plugins.name;
            CCbor* ev = App::worldContainer->createObjectChangedEvent(sim_handle_app, cmd, true);
            ev->appendKeyTextArray(cmd, _pluginNames);
            App::worldContainer->pushEvent();
        }
    }
}
