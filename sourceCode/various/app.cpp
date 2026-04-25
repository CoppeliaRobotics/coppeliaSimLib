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

static std::string OBJECT_META_INFO = R"(
{
    "superclass": "object",
    "namespaces": {
        "namedParam": {},
        "customData": {},
        "signal": {}
    }
}
)";

CSimThread* App::simThread = nullptr;
CUserSettings* App::userSettings = nullptr;
CFolderSystem* App::folders = nullptr;
CSceneContainer* App::scenes = nullptr;
CScene* App::scene = nullptr;
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
int App::_eventProtocolVersion = SIM_EVENT_PROTOCOL_VERSION;
Obj* App::_obj = new Obj(sim_handle_app, "app", OBJECT_META_INFO.c_str());


long long int App::_nextUniqueId = sim_object_variousstart;
#ifdef USE_LONG_LONG_HANDLES
long long int App::_nextHandle_object = sim_object_sceneobjectstart;
long long int App::_nextHandle_collection = sim_object_collectionstart;
long long int App::_nextHandle_script = sim_object_detachedscriptstart;
long long int App::_nextHandle_stack = sim_object_stackstart;
long long int App::_nextHandle_texture = sim_object_texturestart;
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
    scenes = new CSceneContainer();
    scenes->initialize();
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
    scenes->sandboxScript = new CDetachedScript(sim_scripttype_sandbox);
    scenes->sandboxScript->initScript();

    std::string autoLoadAddOns("true");
    getAppNamedParam("addOns.autoLoad", autoLoadAddOns);
    std::transform(autoLoadAddOns.begin(), autoLoadAddOns.end(), autoLoadAddOns.begin(), [](unsigned char c){ return std::tolower(c); });
    if ( App::userSettings->runAddOns && (autoLoadAddOns == "true") )
        scenes->addOnScriptContainer->loadAllFromAddOnFolder();
    scenes->addOnScriptContainer->loadAdditionalAddOns();
    scenes->addOnScriptContainer->callScripts(sim_syscb_init, nullptr, nullptr);
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
        int r = scenes->sandboxScript->executeScriptString(_startupScriptString.c_str(), nullptr);
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

    while (scenes->getSceneCount() > 1)
        scenes->destroyCurrentScene();
    scene->clearScene(true);

    // Following 2 important at this stage, as some destructors rely on plugins (e.g. simGeom):
    scenes->copyBuffer->clearMemorizedBuffer();
    scenes->copyBuffer->clearBuffer();

    scenes->addOnScriptContainer->removeAllAddOns();
    scenes->sandboxScript->systemCallScript(sim_syscb_cleanup, nullptr, nullptr);
    CDetachedScript::destroy(scenes->sandboxScript, true);
    scenes->sandboxScript = nullptr;
    scenes->pluginContainer->unloadNewPlugins(); // cleanup via (UI thread) and SIM thread

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

    scenes->deinitialize();
    delete scenes;
    scenes = nullptr;

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
    scenes->instancePass();
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
    if (scene->simulation->isSimulationStopped() && (editMode == NO_EDIT_MODE))
    {
        scenes->dispatchEvents();
        scenes->callScripts(sim_syscb_nonsimulation, nullptr, nullptr);
    }
    App::scenes->customObjects->saveToAppFolderIfNeeded();
    App::scene->sceneObjects->handleDataCallbacks();
    if (scene->sceneObjects->hasSelectionChanged())
    {
        CInterfaceStack* stack = scenes->interfaceStackContainer->createStack();
        stack->pushTableOntoStack();
        stack->pushTextOntoStack("sel");
        std::vector<int> sel;
        scene->sceneObjects->getSelectedObjectHandles(sel);
        stack->pushInt32ArrayOntoStack(sel.data(), sel.size());
        stack->insertDataIntoStackTable();
        scenes->callScripts(sim_syscb_selchange, stack, nullptr);
        scenes->interfaceStackContainer->destroyStack(stack);
    }
    if (scene->simulation->isSimulationPaused())
    {
        CDetachedScript* mainScript = scene->sceneObjects->embeddedScriptContainer->getMainScript();
        if (mainScript != nullptr)
        {
            scenes->dispatchEvents();
            if (mainScript->systemCallMainScript(sim_syscb_suspended, nullptr, nullptr) == 0)
                scenes->callScripts(sim_syscb_suspended, nullptr, nullptr);
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
        if ((!App::scene->simulation->getIsRealTimeSimulation()) || App::scene->simulation->isRealTimeCalculationStepNeeded())
        {
            if ((!scenes->shouldTemporarilySuspendMainScript()) || App::scene->simulation->didStopRequestCounterChangeSinceSimulationStart())
            {
                CDetachedScript* it = App::scene->sceneObjects->embeddedScriptContainer->getMainScript();
                if (it != nullptr)
                {
                    scenes->calcInfo->simulationPassStart();
                    App::scene->sceneObjects->embeddedScriptContainer->broadcastDataContainer.removeTimedOutObjects(App::scene->simulation->getSimulationTime()); // remove invalid elements
                    it->systemCallMainScript(-1, nullptr, nullptr);
                    scenes->calcInfo->simulationPassEnd();
                }
                App::scene->simulation->advanceSimulationByOneStep();
            }
            // Following for backward compatibility:
            scenes->addOnScriptContainer->callScripts(sim_syscb_aos_run_old, nullptr, nullptr);
        }
        else
            scenes->callScripts(sim_syscb_realtimeidle, nullptr, nullptr);
    }
    //*******************************

    scene->sceneObjects->eraseObjects(nullptr, true); // remove objects that have a delayed destruction
    scene->sceneObjects->embeddedScriptContainer->removeDestroyedScripts(sim_scripttype_simulation);
    scene->sceneObjects->embeddedScriptContainer->removeDestroyedScripts(sim_scripttype_customization);

    // Async reset some scripts:
    for (size_t i = 0; i < _scriptsToReset.size(); i++)
    {
        CDetachedScript* it = scenes->getDetachedScriptFromHandle(_scriptsToReset[i]);
        if (it != nullptr)
            it->initScript();
    }
    _scriptsToReset.clear();

    // Keep for backward compatibility:
    if (!scene->simulation->isSimulationRunning()) // when simulation is running, we handle the add-on scripts after the main script was called
        scenes->addOnScriptContainer->callScripts(sim_syscb_aos_run_old, nullptr, nullptr);

    simThread->executeMessages(); // rendering, queued command execution, etc.
#ifdef SIM_WITH_GUI
    scene->simulation->showAndHandleEmergencyStopButton(false, ""); // 10/10/2015
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
        scene->registerNewHandle(uniqueId, objectType);
    else
        uniqueId = _nextUniqueId++;
#else
    uniqueId = _nextUniqueId++;
#endif
    return uniqueId;
}

void App::releaseUniqueId(long long int uid, int objectType /*= -1 */)
{
    scene->releaseNewHandle(uid, objectType);
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
        retVal = scene->getNewHandleFromOldHandle(oldHandle);
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
        retVal = scene->getOldHandleFromNewHandle(newHandle);
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
        if ((scenes != nullptr) && scenes->getEventsEnabled())
        {
            std::string cmd(NAMEDPARAMPREFIX);
            cmd += paramName;
            CCbor* ev = scenes->createObjectChangedEvent(sim_handle_app, cmd.c_str(), false);
            ev->appendKeyText(cmd.c_str(), param);
            scenes->pushEvent();
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
        if ((scenes != nullptr) && scenes->getEventsEnabled())
        {
            std::string cmd(NAMEDPARAMPREFIX);
            cmd += paramName;
            CCbor* ev = scenes->createObjectChangedEvent(sim_handle_app, cmd.c_str(), false);
            ev->appendKeyNull(cmd.c_str());
            scenes->pushEvent();
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

    if (scenes != nullptr)
    {
        if (pluginName == nullptr)
            it = scenes->pluginContainer->getCurrentPlugin();
        else
        {
            it = scenes->pluginContainer->getPluginFromName(pluginName);
            if (it == nullptr)
                it = scenes->pluginContainer->getPluginFromName_old(pluginName, true);
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

void App::logScriptMsg(const CDetachedScript* script, int verbosityLevel, const char* msg)
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

        if ((scenes != nullptr) && VThread::isSimThread())
        {
            std::string orig("CoppeliaSim");
            if (originName != nullptr)
                orig = originName;
            CCbor* ev = scenes->createEvent("logMsg", -1, -1, nullptr, false);
            ev->appendKeyText("origin", orig.c_str());
            ev->appendKeyText("msg", msg);
            ev->appendKeyInt64("verbosity", realVerbosityLevel);
            ev->openKeyMap("flags");
            ev->appendKeyBool("undecorated", verbosityLevel & sim_verbosity_undecorated);
            ev->appendKeyBool("onlyterminal", verbosityLevel & sim_verbosity_onlyterminal);
            ev->closeArrayOrMap();
            scenes->pushEvent();
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
        if ((scenes != nullptr) && scenes->getEventsEnabled())
        {
            const char* cmd = propApp_dialogVerbosity.name;
            CCbor* ev = scenes->createObjectChangedEvent(sim_handle_app, cmd, true);
            ev->appendKeyInt64(cmd, _dlgVerbosity);
            scenes->pushEvent();
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
        scene->undoBufferContainer->announceChange();
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
        scene->undoBufferContainer->announceChangeGradual();
#endif
}

int App::getConsoleVerbosity(const char* pluginName /*=nullptr*/)
{ // sim_verbosity_none, etc.
    int retVal = _consoleVerbosity;
    if (pluginName != nullptr)
    {
        CPlugin* pl = scenes->pluginContainer->getPluginFromName_old(pluginName, true);
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
        CPlugin* pl = scenes->pluginContainer->getPluginFromName_old(pluginName, true);
        if (pl != nullptr)
            pl->setConsoleVerbosity(v);
    }
    else
    {
        bool diff = (_consoleVerbosity != v);
        if (diff)
        {
            _consoleVerbosity = v;
            if ((scenes != nullptr) && scenes->getEventsEnabled())
            {
                const char* cmd = propApp_consoleVerbosity.name;
                CCbor* ev = scenes->createObjectChangedEvent(sim_handle_app, cmd, true);
                ev->appendKeyInt64(cmd, _consoleVerbosity);
                scenes->pushEvent();
            }
        }
    }
}

int App::getStatusbarVerbosity(const char* pluginName /*=nullptr*/)
{ // sim_verbosity_none, etc.
    int retVal = _statusbarVerbosity;
    if (pluginName != nullptr)
    {
        CPlugin* pl = scenes->pluginContainer->getPluginFromName_old(pluginName, true);
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
        CPlugin* pl = scenes->pluginContainer->getPluginFromName_old(pluginName, true);
        if (pl != nullptr)
            pl->setStatusbarVerbosity(v);
    }
    else
    {
        bool diff = (_statusbarVerbosity != v);
        if (diff)
        {
            _statusbarVerbosity = v;
            if ((scenes != nullptr) && scenes->getEventsEnabled())
            {
                const char* cmd = propApp_statusbarVerbosity.name;
                CCbor* ev = scenes->createObjectChangedEvent(sim_handle_app, cmd, true);
                ev->appendKeyInt64(cmd, _statusbarVerbosity);
                scenes->pushEvent();
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
    it = App::scene->sceneObjects->getObjectFromHandle(objectHandle);
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
        App::scene->sceneObjects->setObjectParent(it, nullptr, true);
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
    it1 = App::scene->sceneObjects->getObjectFromHandle(parentHandle);
    it2 = App::scene->sceneObjects->getObjectFromHandle(childHandle);
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
        it1 = App::scene->sceneObjects->getObjectFromHandle(parentHandle);
        it2 = App::scene->sceneObjects->getObjectFromHandle(childHandle);
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
            App::scene->sceneObjects->setObjectParent(obj2, obj1, true);
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
                    App::scene->sceneObjects->setObjectParent(obj2, obj1, true);
                else
                    App::scene->sceneObjects->setObjectParent(obj2, potParents[0], true);
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

int App::setBoolProperty_t(long long int target, const char* ppName, bool pState)
{
    int retVal = sim_propertyret_unknownproperty;
    if (!_resolveTarget(target))
        retVal = sim_propertyret_unknowntarget;

    const char* pName = ppName;
    if (target == sim_handle_app)
    {
        if (strcmp(pName, propApp_hierarchyEnabled.name) == 0)
        {
            setHierarchyEnabled(pState);
            retVal = sim_propertyret_ok;
        }
        else if (strcmp(pName, propApp_browserEnabled.name) == 0)
        {
#ifdef SIM_WITH_GUI
            GuiApp::setBrowserEnabled(pState);
#endif
            retVal = sim_propertyret_ok;
        }
        else if (strcmp(pName, propApp_displayEnabled.name) == 0)
        {
            setOpenGlDisplayEnabled(pState);
            retVal = sim_propertyret_ok;
        }
    }
    else if ((target >= sim_object_detachedscriptstart) && (target <= sim_object_detachedscriptend))
    { // sandbox, main, add-ons, or old associated scripts:
        CDetachedScript* script = scenes->getDetachedScriptFromHandle(int(target));
        if (script != nullptr)
        {
            std::string _pName(ppName);
            const char* pName = _pName.c_str();
            retVal = script->setBoolProperty(pName, pState);
        }
        else
            retVal = sim_propertyret_unknowntarget;
    }
    else if ((target >= sim_object_stackstart) && (target <= sim_object_stackend))
        retVal = scenes->interfaceStackContainer->setBoolProperty_t(target, ppName, pState);
    else if ((target >= sim_object_customclassstart) && (target < sim_object_customappend) && (scenes != nullptr)) // don't forget classes!
        retVal = scenes->customObjects->setBoolProperty_t(target, ppName, pState);
    else if ((target >= sim_object_sceneobjectclassstart) && (target < sim_object_sceneobjectclassend) && (scenes != nullptr))
        retVal = scenes->customSceneObjectClasses->setBoolProperty_t(target, ppName, pState);
    else if (scene != nullptr)
        retVal = scene->setBoolProperty_t(target, pName, pState);
    else
        retVal = sim_propertyret_unknowntarget;
    return retVal;
}

int App::getBoolProperty_t(long long int target, const char* ppName, bool& pState)
{
    int retVal = sim_propertyret_unknownproperty;
    if (!_resolveTarget(target))
        retVal = sim_propertyret_unknowntarget;

    const char* pName = ppName;
    if (target == sim_handle_app)
    {
        if (strcmp(pName, propApp_hierarchyEnabled.name) == 0)
        {
            pState = getHierarchyEnabled();
            retVal = sim_propertyret_ok;
        }
        else if (strcmp(pName, propApp_browserEnabled.name) == 0)
        {
#ifdef SIM_WITH_GUI
            pState = GuiApp::getBrowserEnabled();
#else
            pState = false;
#endif
            retVal = sim_propertyret_ok;
        }
        else if (strcmp(pName, propApp_displayEnabled.name) == 0)
        {
            pState = getOpenGlDisplayEnabled();
            retVal = sim_propertyret_ok;
        }
        else if (strcmp(pName, propApp_canSave.name) == 0)
        {
            pState = canSave();
            retVal = sim_propertyret_ok;
        }
        else if (strcmp(pName, propApp_execUnsafe.name) == 0)
        {
            if (userSettings != nullptr)
            {
                pState = userSettings->execUnsafe;
                retVal = sim_propertyret_ok;
            }
            else
                retVal = 0;
        }
        else if (strcmp(pName, propApp_execUnsafeExt.name) == 0)
        {
            if (userSettings != nullptr)
            {
                pState = userSettings->execUnsafeExt;
                retVal = sim_propertyret_ok;
            }
            else
                retVal = 0;
        }
    }
    else if ((target >= sim_object_detachedscriptstart) && (target <= sim_object_detachedscriptend))
    { // sandbox, main, add-ons, or old associated scripts:
        CDetachedScript* script = scenes->getDetachedScriptFromHandle(int(target));
        if (script != nullptr)
        {
            std::string _pName(ppName);
            const char* pName = _pName.c_str();
            retVal = script->getBoolProperty(pName, pState);
        }
        else
            retVal = sim_propertyret_unknowntarget;
    }
    else if ((target >= sim_object_stackstart) && (target <= sim_object_stackend))
        retVal = scenes->interfaceStackContainer->getBoolProperty_t(target, ppName, pState);
    else if ((target >= sim_object_customclassstart) && (target < sim_object_customappend) && (scenes != nullptr)) // don't forget classes!
        retVal = scenes->customObjects->getBoolProperty_t(target, ppName, pState);
    else if ((target >= sim_object_sceneobjectclassstart) && (target < sim_object_sceneobjectclassend) && (scenes != nullptr))
        retVal = scenes->customSceneObjectClasses->getBoolProperty_t(target, ppName, pState);
    else if (scene != nullptr)
        retVal = scene->getBoolProperty_t(target, pName, pState);
    else
        retVal = sim_propertyret_unknowntarget;
    return retVal;
}

int App::setIntProperty_t(long long int target, const char* ppName, int pState)
{
    int retVal = sim_propertyret_unknownproperty;
    if (!_resolveTarget(target))
        retVal = sim_propertyret_unknowntarget;

    const char* pName = ppName;
    if (target == sim_handle_app)
    {
        if (strcmp(pName, propApp_protocolVersion.name) == 0)
        {
            setEventProtocolVersion(pState);
            retVal = sim_propertyret_ok;
        }
        else if (strcmp(pName, propApp_consoleVerbosity.name) == 0)
        {
            setConsoleVerbosity(pState);
            retVal = sim_propertyret_ok;
        }
        else if (strcmp(pName, propApp_statusbarVerbosity.name) == 0)
        {
            setStatusbarVerbosity(pState);
            retVal = sim_propertyret_ok;
        }
        else if (strcmp(pName, propApp_dialogVerbosity.name) == 0)
        {
            setDlgVerbosity(pState);
            retVal = sim_propertyret_ok;
        }
        else if (strcmp(pName, propApp_idleFps.name) == 0)
        {
            if (userSettings != nullptr)
                userSettings->setIdleFps_session(pState);
            retVal = sim_propertyret_ok;
        }
    }
    else if ((target >= sim_object_detachedscriptstart) && (target <= sim_object_detachedscriptend))
    { // sandbox, main, add-ons, or old associated scripts:
        CDetachedScript* script = scenes->getDetachedScriptFromHandle(int(target));
        if (script != nullptr)
        {
            const char* pName = ppName;
            retVal = script->setIntProperty(pName, pState);
        }
        else
            retVal = sim_propertyret_unknowntarget;
    }
    else if ((target >= sim_object_stackstart) && (target <= sim_object_stackend))
        retVal = scenes->interfaceStackContainer->setIntProperty_t(target, ppName, pState);
    else if ((target >= sim_object_customclassstart) && (target < sim_object_customappend) && (scenes != nullptr)) // don't forget classes!
        retVal = scenes->customObjects->setIntProperty_t(target, ppName, pState);
    else if ((target >= sim_object_sceneobjectclassstart) && (target < sim_object_sceneobjectclassend) && (scenes != nullptr))
        retVal = scenes->customSceneObjectClasses->setIntProperty_t(target, ppName, pState);
    else if (scene != nullptr)
        retVal = scene->setIntProperty_t(target, pName, pState);
    else
        retVal = sim_propertyret_unknowntarget;
    return retVal;
}

int App::getIntProperty_t(long long int target, const char* ppName, int& pState)
{
    int retVal = sim_propertyret_unknownproperty;
    if (!_resolveTarget(target))
        retVal = sim_propertyret_unknowntarget;

    const char* pName = ppName;
    if (target == sim_handle_app)
    {
        if (strcmp(pName, propApp_protocolVersion.name) == 0)
        {
            pState = _eventProtocolVersion;
            retVal = sim_propertyret_ok;
        }
        else if (strcmp(pName, propApp_productVersionNb.name) == 0)
        {
            pState = SIM_PROGRAM_FULL_VERSION_NB;
            retVal = sim_propertyret_ok;
        }
        else if (strcmp(pName, propApp_platform.name) == 0)
        {
            pState = getPlatform();
            retVal = sim_propertyret_ok;
        }
        else if (strcmp(pName, propApp_flavor.name) == 0)
        {
#ifdef SIM_FL
            pState = SIM_FL;
#else
            pState = -1;
#endif
            retVal = sim_propertyret_ok;
        }
        else if (strcmp(pName, propApp_qtVersion.name) == 0)
        {
            pState = (QT_VERSION >> 16) * 10000 + ((QT_VERSION >> 8) & 255) * 100 + (QT_VERSION & 255) * 1;
            retVal = sim_propertyret_ok;
        }
        else if (strcmp(pName, propApp_processId.name) == 0)
        {
            if (instancesList != nullptr)
                pState = instancesList->thisInstanceId();
            else
                pState = -1;
            retVal = sim_propertyret_ok;
        }
        else if (strcmp(pName, propApp_processCnt.name) == 0)
        {
            if (instancesList != nullptr)
                pState = instancesList->numInstances();
            else
                pState = -1;
            retVal = sim_propertyret_ok;
        }
        else if (strcmp(pName, propApp_consoleVerbosity.name) == 0)
        {
            pState = getConsoleVerbosity();
            retVal = sim_propertyret_ok;
        }
        else if (strcmp(pName, propApp_statusbarVerbosity.name) == 0)
        {
            pState = getStatusbarVerbosity();
            retVal = sim_propertyret_ok;
        }
        else if (strcmp(pName, propApp_dialogVerbosity.name) == 0)
        {
            pState = getDlgVerbosity();
            retVal = sim_propertyret_ok;
        }
        else if (strcmp(pName, propApp_headlessMode.name) == 0)
        {
            pState = getHeadlessMode();
            retVal = sim_propertyret_ok;
        }
        else if (strcmp(pName, propApp_idleFps.name) == 0)
        {
            if (userSettings != nullptr)
                pState = userSettings->getIdleFps();
            else
                pState = -1;
            retVal = sim_propertyret_ok;
        }
        else if (strcmp(pName, propApp_notifyDeprecated.name) == 0)
        {
            if (userSettings != nullptr)
                pState = userSettings->notifyDeprecated;
            else
                pState = -1;
            retVal = sim_propertyret_ok;
        }
    }
    else if ((target >= sim_object_detachedscriptstart) && (target <= sim_object_detachedscriptend))
    { // sandbox, main, add-ons, or old associated scripts:
        CDetachedScript* script = scenes->getDetachedScriptFromHandle(int(target));
        if (script != nullptr)
        {
            const char* pName = ppName;
            retVal = script->getIntProperty(pName, pState);
        }
        else
            retVal = sim_propertyret_unknowntarget;
    }
    else if ((target >= sim_object_stackstart) && (target <= sim_object_stackend))
        retVal = scenes->interfaceStackContainer->getIntProperty_t(target, ppName, pState);
    else if ((target >= sim_object_customclassstart) && (target < sim_object_customappend) && (scenes != nullptr)) // don't forget classes!
        retVal = scenes->customObjects->getIntProperty_t(target, ppName, pState);
    else if ((target >= sim_object_sceneobjectclassstart) && (target < sim_object_sceneobjectclassend) && (scenes != nullptr))
        retVal = scenes->customSceneObjectClasses->getIntProperty_t(target, ppName, pState);
    else if (scene != nullptr)
        retVal = scene->getIntProperty_t(target, pName, pState);
    else
        retVal = sim_propertyret_unknowntarget;
    return retVal;
}

int App::setLongProperty_t(long long int target, const char* ppName, long long int pState)
{
    int retVal = sim_propertyret_unknownproperty;
    if (!_resolveTarget(target))
        retVal = sim_propertyret_unknowntarget;

    const char* pName = ppName;
    if (target == sim_handle_app)
    {
    }
    else if ((target >= sim_object_detachedscriptstart) && (target <= sim_object_detachedscriptend))
    { // sandbox, main, add-ons, or old associated scripts:
        CDetachedScript* script = scenes->getDetachedScriptFromHandle(int(target));
        if (script != nullptr)
        {
            const char* pName = ppName;
            retVal = script->setLongProperty(pName, pState);
        }
        else
            retVal = sim_propertyret_unknowntarget;
    }
    else if ((target >= sim_object_stackstart) && (target <= sim_object_stackend))
        retVal = scenes->interfaceStackContainer->setLongProperty_t(target, ppName, pState);
    else if ((target >= sim_object_customclassstart) && (target < sim_object_customappend) && (scenes != nullptr)) // don't forget classes!
        retVal = scenes->customObjects->setLongProperty_t(target, ppName, pState);
    else if ((target >= sim_object_sceneobjectclassstart) && (target < sim_object_sceneobjectclassend) && (scenes != nullptr))
        retVal = scenes->customSceneObjectClasses->setLongProperty_t(target, ppName, pState);
    else if (scene != nullptr)
        retVal = scene->setLongProperty_t(target, pName, pState);
    else
        retVal = sim_propertyret_unknowntarget;
    return retVal;
}

int App::getLongProperty_t(long long int target, const char* ppName, long long int& pState)
{
    int retVal = sim_propertyret_unknownproperty;
    if (!_resolveTarget(target))
        retVal = sim_propertyret_unknowntarget;

    const char* pName = ppName;
    if (target == sim_handle_app)
    {
        retVal = _obj->getLongProperty(ppName, pState);
        if (retVal == sim_propertyret_unknownproperty)
        {
            if (strcmp(pName, propApp_pid.name) == 0)
            {
                pState = pid;
                retVal = sim_propertyret_ok;
            }
        }
    }
    else if ((target >= sim_object_detachedscriptstart) && (target <= sim_object_detachedscriptend))
    { // sandbox, main, add-ons, or old associated scripts:
        CDetachedScript* script = scenes->getDetachedScriptFromHandle(int(target));
        if (script != nullptr)
        {
            const char* pName = ppName;
            retVal = script->getLongProperty(pName, pState);
        }
        else
            retVal = sim_propertyret_unknowntarget;
    }
    else if ((target >= sim_object_stackstart) && (target <= sim_object_stackend))
        retVal = scenes->interfaceStackContainer->getLongProperty_t(target, ppName, pState);
    else if ((target >= sim_object_customclassstart) && (target < sim_object_customappend) && (scenes != nullptr)) // don't forget classes!
        retVal = scenes->customObjects->getLongProperty_t(target, ppName, pState);
    else if ((target >= sim_object_sceneobjectclassstart) && (target < sim_object_sceneobjectclassend) && (scenes != nullptr))
        retVal = scenes->customSceneObjectClasses->getLongProperty_t(target, ppName, pState);
    else if (scene != nullptr)
        retVal = scene->getLongProperty_t(target, pName, pState);
    else
        retVal = sim_propertyret_unknowntarget;
    return retVal;
}

int App::setHandleProperty_t(long long int target, const char* ppName, long long int pState)
{
    int retVal = sim_propertyret_unknownproperty;
    if (!_resolveTarget(target))
        retVal = sim_propertyret_unknowntarget;

    const char* pName = ppName;
    if (target == sim_handle_app)
    {
    }
    else if ((target >= sim_object_detachedscriptstart) && (target <= sim_object_detachedscriptend))
    { // sandbox, main, add-ons, or old associated scripts:
        CDetachedScript* script = scenes->getDetachedScriptFromHandle(int(target));
        if (script != nullptr)
        {
          //const char* pName = ppName;
          //retVal = script->setHandleProperty(pName, pState);
        }
        else
            retVal = sim_propertyret_unknowntarget;
    }
    else if ((target >= sim_object_stackstart) && (target <= sim_object_stackend))
        retVal = scenes->interfaceStackContainer->setHandleProperty_t(target, ppName, pState);
    else if ((target >= sim_object_customclassstart) && (target < sim_object_customappend) && (scenes != nullptr)) // don't forget classes!
        retVal = scenes->customObjects->setHandleProperty_t(target, ppName, pState);
    else if ((target >= sim_object_sceneobjectclassstart) && (target < sim_object_sceneobjectclassend) && (scenes != nullptr))
        retVal = scenes->customSceneObjectClasses->setHandleProperty_t(target, ppName, pState);
    else if (scene != nullptr)
        retVal = scene->setHandleProperty_t(target, pName, pState);
    else
        retVal = sim_propertyret_unknowntarget;
    return retVal;
}

int App::getHandleProperty_t(long long int target, const char* ppName, long long int& pState)
{
    int retVal = sim_propertyret_unknownproperty;
    if (!_resolveTarget(target))
        retVal = sim_propertyret_unknowntarget;

    const char* pName = ppName;
    if (target == sim_handle_app)
    {
        if (strcmp(pName, propApp_sandbox.name) == 0)
        {
            if ( (scenes != nullptr) && (scenes->sandboxScript != nullptr) )
                pState = scenes->sandboxScript->getScriptHandle();
            else
                pState = -1;
            retVal = sim_propertyret_ok;
        }
    }
    else if ((target >= sim_object_detachedscriptstart) && (target <= sim_object_detachedscriptend))
    { // sandbox, main, add-ons, or old associated scripts:
        CDetachedScript* script = scenes->getDetachedScriptFromHandle(int(target));
        if (script != nullptr)
            retVal = script->getHandleProperty(ppName, pState);
        else
            retVal = sim_propertyret_unknowntarget;
    }
    else if ((target >= sim_object_stackstart) && (target <= sim_object_stackend))
        retVal = scenes->interfaceStackContainer->getHandleProperty_t(target, ppName, pState);
    else if ((target >= sim_object_customclassstart) && (target < sim_object_customappend) && (scenes != nullptr)) // don't forget classes!
        retVal = scenes->customObjects->getHandleProperty_t(target, ppName, pState);
    else if ((target >= sim_object_sceneobjectclassstart) && (target < sim_object_sceneobjectclassend) && (scenes != nullptr))
        retVal = scenes->customSceneObjectClasses->getHandleProperty_t(target, ppName, pState);
    else if (scene != nullptr)
        retVal = scene->getHandleProperty_t(target, pName, pState);
    else
        retVal = sim_propertyret_unknowntarget;
    return retVal;
}

int App::setFloatProperty_t(long long int target, const char* ppName, double pState)
{
    int retVal = sim_propertyret_unknownproperty;
    if (!_resolveTarget(target))
        retVal = sim_propertyret_unknowntarget;

    const char* pName = ppName;
    if (target == sim_handle_app)
    {
        if (strcmp(pName, propApp_defaultTranslationStepSize.name) == 0)
        {
            if (userSettings != nullptr)
                userSettings->setTranslationStepSize(pState);
            retVal = sim_propertyret_ok;
        }
        else if (strcmp(pName, propApp_defaultRotationStepSize.name) == 0)
        {
            if (userSettings != nullptr)
                userSettings->setRotationStepSize(pState);
            retVal = sim_propertyret_ok;
        }
    }
    else if ((target >= sim_object_detachedscriptstart) && (target <= sim_object_detachedscriptend))
    { // sandbox, main, add-ons, or old associated scripts:
        CDetachedScript* script = scenes->getDetachedScriptFromHandle(int(target));
        if (script != nullptr)
        {
          //const char* pName = ppName;
          //retVal = script->setFloatProperty(pName, pState);
        }
        else
            retVal = sim_propertyret_unknowntarget;
    }
    else if ((target >= sim_object_stackstart) && (target <= sim_object_stackend))
        retVal = scenes->interfaceStackContainer->setFloatProperty_t(target, ppName, pState);
    else if ((target >= sim_object_customclassstart) && (target < sim_object_customappend) && (scenes != nullptr)) // don't forget classes!
        retVal = scenes->customObjects->setFloatProperty_t(target, ppName, pState);
    else if ((target >= sim_object_sceneobjectclassstart) && (target < sim_object_sceneobjectclassend) && (scenes != nullptr))
        retVal = scenes->customSceneObjectClasses->setFloatProperty_t(target, ppName, pState);
    else if (scene != nullptr)
        retVal = scene->setFloatProperty_t(target, pName, pState);
    else
        retVal = sim_propertyret_unknowntarget;
    return retVal;
}

int App::getFloatProperty_t(long long int target, const char* ppName, double& pState)
{
    int retVal = sim_propertyret_unknownproperty;
    if (!_resolveTarget(target))
        retVal = sim_propertyret_unknowntarget;

    const char* pName = ppName;
    if (target == sim_handle_app)
    {
        if (strcmp(pName, propApp_defaultTranslationStepSize.name) == 0)
        {
            if (userSettings != nullptr)
                pState = userSettings->getTranslationStepSize();
            else
                pState = 0.0;
            retVal = sim_propertyret_ok;
        }
        else if (strcmp(pName, propApp_defaultRotationStepSize.name) == 0)
        {
            if (userSettings != nullptr)
                pState = userSettings->getRotationStepSize();
            else
                pState = 0.0;
            retVal = sim_propertyret_ok;
        }
        else if (strcmp(pName, propApp_randomFloat.name) == 0)
        {
            pState = SIM_RAND_FLOAT;
            retVal = sim_propertyret_ok;
        }
        else if (strcmp(pName, propApp_systemTime.name) == 0)
        {
            pState = VDateTime::getTime();
            retVal = sim_propertyret_ok;
        }
    }
    else if ((target >= sim_object_detachedscriptstart) && (target <= sim_object_detachedscriptend))
    { // sandbox, main, add-ons, or old associated scripts:
        CDetachedScript* script = scenes->getDetachedScriptFromHandle(int(target));
        if (script != nullptr)
        {
          //const char* pName = ppName;
          //retVal = script->getFloatProperty(pName, pState);
        }
        else
            retVal = sim_propertyret_unknowntarget;
    }
    else if ((target >= sim_object_stackstart) && (target <= sim_object_stackend))
        retVal = scenes->interfaceStackContainer->getFloatProperty_t(target, ppName, pState);
    else if ((target >= sim_object_customclassstart) && (target < sim_object_customappend) && (scenes != nullptr)) // don't forget classes!
        retVal = scenes->customObjects->getFloatProperty_t(target, ppName, pState);
    else if ((target >= sim_object_sceneobjectclassstart) && (target < sim_object_sceneobjectclassend) && (scenes != nullptr))
        retVal = scenes->customSceneObjectClasses->getFloatProperty_t(target, ppName, pState);
    else if (scene != nullptr)
        retVal = scene->getFloatProperty_t(target, pName, pState);
    else
        retVal = sim_propertyret_unknowntarget;
    return retVal;
}

int App::setStringProperty_t(long long int target, const char* ppName, const std::string& pState)
{
    int retVal = sim_propertyret_unknownproperty;
    if (!_resolveTarget(target))
        retVal = sim_propertyret_unknowntarget;

    const char* pName = ppName;
    if (target == sim_handle_app)
    {
        std::string pN(pName);
        if (utils::replaceSubstringStart(pN, NAMEDPARAMPREFIX, ""))
        {
            if (pN.size() > 0)
            {
                setAppNamedParam(pN.c_str(), pState.c_str());
                retVal = sim_propertyret_ok;
            }
        }
        if (retVal == sim_propertyret_unknownproperty)
        {
            if (strcmp(pName, propApp_sceneDir.name) == 0)
            {
                if (folders != nullptr)
                    folders->setScenesPath(pState.c_str());
                retVal = sim_propertyret_ok;
            }
            else if (strcmp(pName, propApp_modelDir.name) == 0)
            {
                if (folders != nullptr)
                    folders->setModelsPath(pState.c_str());
                retVal = sim_propertyret_ok;
            }
            else if (strcmp(pName, propApp_importExportDir.name) == 0)
            {
                if (folders != nullptr)
                    folders->setImportExportPath(pState.c_str());
                retVal = sim_propertyret_ok;
            }
            else if (strcmp(pName, propApp_consoleVerbosityStr.name) == 0)
            {
                setStringVerbosity(0, pState.c_str());
                retVal = sim_propertyret_ok;
            }
            else if (strcmp(pName, propApp_statusbarVerbosityStr.name) == 0)
            {
                setStringVerbosity(1, pState.c_str());
                retVal = sim_propertyret_ok;
            }
            else if (strcmp(pName, propApp_dialogVerbosityStr.name) == 0)
            {
                setStringVerbosity(2, pState.c_str());
                retVal = sim_propertyret_ok;
            }
            else if (strcmp(pName, propApp_auxAddOn1.name) == 0)
            {
                setAdditionalAddOnScript1(pState.c_str());
                retVal = sim_propertyret_ok;
            }
            else if (strcmp(pName, propApp_auxAddOn2.name) == 0)
            {
                setAdditionalAddOnScript2(pState.c_str());
                retVal = sim_propertyret_ok;
            }
            else if (strcmp(pName, propApp_startupCode.name) == 0)
            {
                setStartupScriptString(pState.c_str());
                retVal = sim_propertyret_ok;
            }
            else if (strcmp(pName, propApp_appArg1.name) == 0)
            {
                setApplicationArgument(0, pState.c_str());
                retVal = sim_propertyret_ok;
            }
            else if (strcmp(pName, propApp_appArg2.name) == 0)
            {
                setApplicationArgument(1, pState.c_str());
                retVal = sim_propertyret_ok;
            }
            else if (strcmp(pName, propApp_appArg3.name) == 0)
            {
                setApplicationArgument(2, pState.c_str());
                retVal = sim_propertyret_ok;
            }
            else if (strcmp(pName, propApp_appArg4.name) == 0)
            {
                setApplicationArgument(3, pState.c_str());
                retVal = sim_propertyret_ok;
            }
            else if (strcmp(pName, propApp_appArg5.name) == 0)
            {
                setApplicationArgument(4, pState.c_str());
                retVal = sim_propertyret_ok;
            }
            else if (strcmp(pName, propApp_appArg6.name) == 0)
            {
                setApplicationArgument(5, pState.c_str());
                retVal = sim_propertyret_ok;
            }
            else if (strcmp(pName, propApp_appArg7.name) == 0)
            {
                setApplicationArgument(6, pState.c_str());
                retVal = sim_propertyret_ok;
            }
            else if (strcmp(pName, propApp_appArg8.name) == 0)
            {
                setApplicationArgument(7, pState.c_str());
                retVal = sim_propertyret_ok;
            }
            else if (strcmp(pName, propApp_appArg9.name) == 0)
            {
                setApplicationArgument(8, pState.c_str());
                retVal = sim_propertyret_ok;
            }
        }
    }
    else if ((target >= sim_object_detachedscriptstart) && (target <= sim_object_detachedscriptend))
    { // sandbox, main, add-ons, or old associated scripts:
        CDetachedScript* script = scenes->getDetachedScriptFromHandle(int(target));
        if (script != nullptr)
        {
            const char* pName = ppName;
            retVal = script->setStringProperty(pName, pState);
        }
        else
            retVal = sim_propertyret_unknowntarget;
    }
    else if ((target >= sim_object_stackstart) && (target <= sim_object_stackend))
        retVal = scenes->interfaceStackContainer->setStringProperty_t(target, ppName, pState);
    else if ((target >= sim_object_customclassstart) && (target < sim_object_customappend) && (scenes != nullptr)) // don't forget classes!
        retVal = scenes->customObjects->setStringProperty_t(target, ppName, pState);
    else if ((target >= sim_object_sceneobjectclassstart) && (target < sim_object_sceneobjectclassend) && (scenes != nullptr))
        retVal = scenes->customSceneObjectClasses->setStringProperty_t(target, ppName, pState);
    else if (scene != nullptr)
        retVal = scene->setStringProperty_t(target, pName, pState);
    else
        retVal = sim_propertyret_unknowntarget;
    return retVal;
}

int App::getStringProperty_t(long long int target, const char* ppName, std::string& pState)
{
    int retVal = sim_propertyret_unknownproperty;
    if (!_resolveTarget(target))
        retVal = sim_propertyret_unknowntarget;

    const char* pName = ppName;
    if (target == sim_handle_app)
    {
        retVal = _obj->getStringProperty(ppName, pState);
        if (retVal == sim_propertyret_unknownproperty)
        {
            std::string pN(pName);
            if (utils::replaceSubstringStart(pN, NAMEDPARAMPREFIX, ""))
            {
                if (pN.size() > 0)
                {
                    if (getAppNamedParam(pN.c_str(), pState))
                        retVal = sim_propertyret_ok;
                }
            }
        }
        if (retVal == sim_propertyret_unknownproperty)
        {
            if (strcmp(pName, propApp_sessionId.name) == 0)
            {
                if (scenes != nullptr)
                    pState = scenes->getSessionId();
                else
                    pState = "";
                retVal = sim_propertyret_ok;
            }
            else if (strcmp(pName, propApp_productVersion.name) == 0)
            {
                pState = SIM_VERSION_STR_SHORT;
                retVal = sim_propertyret_ok;
            }
            else if (strcmp(pName, propApp_appDir.name) == 0)
            {
                if (folders != nullptr)
                    pState = folders->getExecutablePath();
                else
                    pState = "";
                retVal = sim_propertyret_ok;
            }
            else if (strcmp(pName, propApp_machineId.name) == 0)
            {
                pState = CSimFlavor::getStringVal_int(0, sim_stringparam_machine_id);
                retVal = sim_propertyret_ok;
            }
            else if (strcmp(pName, propApp_legacyMachineId.name) == 0)
            {
                pState = CSimFlavor::getStringVal_int(0, sim_stringparam_machine_id_legacy);
                retVal = sim_propertyret_ok;
            }
            else if (strcmp(pName, propApp_tempDir.name) == 0)
            {
                if (folders != nullptr)
                    pState = folders->getTempDataPath();
                else
                    pState = "";
                retVal = sim_propertyret_ok;
            }
            else if (strcmp(pName, propApp_sceneTempDir.name) == 0)
            {
                if (folders != nullptr)
                    pState = folders->getSceneTempDataPath();
                else
                    pState = "";
                retVal = sim_propertyret_ok;
            }
            else if (strcmp(pName, propApp_settingsDir.name) == 0)
            {
                if (folders != nullptr)
                    pState = folders->getUserSettingsPath();
                else
                    pState = "";
                retVal = sim_propertyret_ok;
            }
            else if (strcmp(pName, propApp_luaDir.name) == 0)
            {
                if (folders != nullptr)
                    pState = folders->getLuaPath();
                else
                    pState = "";
                retVal = sim_propertyret_ok;
            }
            else if (strcmp(pName, propApp_pythonDir.name) == 0)
            {
                if (folders != nullptr)
                    pState = folders->getPythonPath();
                else
                    pState = "";
                retVal = sim_propertyret_ok;
            }
            else if (strcmp(pName, propApp_mujocoDir.name) == 0)
            {
                if (folders != nullptr)
                    pState = folders->getMujocoPath();
                else
                    pState = "";
                retVal = sim_propertyret_ok;
            }
            else if (strcmp(pName, propApp_systemDir.name) == 0)
            {
                if (folders != nullptr)
                    pState = folders->getSystemPath();
                else
                    pState = "";
                retVal = sim_propertyret_ok;
            }
            else if (strcmp(pName, propApp_resourceDir.name) == 0)
            {
                if (folders != nullptr)
                    pState = folders->getResourcesPath();
                else
                    pState = "";
                retVal = sim_propertyret_ok;
            }
            else if (strcmp(pName, propApp_addOnDir.name) == 0)
            {
                if (folders != nullptr)
                    pState = folders->getAddOnPath();
                else
                    pState = "";
                retVal = sim_propertyret_ok;
            }
            else if (strcmp(pName, propApp_sceneDir.name) == 0)
            {
                if (folders != nullptr)
                    pState = folders->getScenesPath();
                else
                    pState = "";
                retVal = sim_propertyret_ok;
            }
            else if (strcmp(pName, propApp_modelDir.name) == 0)
            {
                if (folders != nullptr)
                    pState = folders->getModelsPath();
                else
                    pState = "";
                retVal = sim_propertyret_ok;
            }
            else if (strcmp(pName, propApp_importExportDir.name) == 0)
            {
                if (folders != nullptr)
                    pState = folders->getImportExportPath();
                else
                    pState = "";
                retVal = sim_propertyret_ok;
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
                retVal = sim_propertyret_ok;
            }
            else if (strcmp(pName, propApp_sandboxLang.name) == 0)
            {
                if (userSettings != nullptr)
                    pState = userSettings->preferredSandboxLang;
                else
                    pState = "";
                retVal = sim_propertyret_ok;
            }
            else if (strcmp(pName, propApp_appArg1.name) == 0)
            {
                pState = getApplicationArgument(0);
                retVal = sim_propertyret_ok;
            }
            else if (strcmp(pName, propApp_appArg2.name) == 0)
            {
                pState = getApplicationArgument(1);
                retVal = sim_propertyret_ok;
            }
            else if (strcmp(pName, propApp_appArg3.name) == 0)
            {
                pState = getApplicationArgument(2);
                retVal = sim_propertyret_ok;
            }
            else if (strcmp(pName, propApp_appArg4.name) == 0)
            {
                pState = getApplicationArgument(3);
                retVal = sim_propertyret_ok;
            }
            else if (strcmp(pName, propApp_appArg5.name) == 0)
            {
                pState = getApplicationArgument(4);
                retVal = sim_propertyret_ok;
            }
            else if (strcmp(pName, propApp_appArg6.name) == 0)
            {
                pState = getApplicationArgument(5);
                retVal = sim_propertyret_ok;
            }
            else if (strcmp(pName, propApp_appArg7.name) == 0)
            {
                pState = getApplicationArgument(6);
                retVal = sim_propertyret_ok;
            }
            else if (strcmp(pName, propApp_appArg8.name) == 0)
            {
                pState = getApplicationArgument(7);
                retVal = sim_propertyret_ok;
            }
            else if (strcmp(pName, propApp_appArg9.name) == 0)
            {
                pState = getApplicationArgument(8);
                retVal = sim_propertyret_ok;
            }
            else if (strcmp(pName, propApp_dongleSerial.name) == 0)
            {
                pState = CSimFlavor::getStringVal(22);
                retVal = sim_propertyret_ok;
            }
            else if (strcmp(pName, propApp_machineSerialND.name) == 0)
            {
                pState = CSimFlavor::getStringVal(23);
                retVal = sim_propertyret_ok;
            }
            else if (strcmp(pName, propApp_machineSerial.name) == 0)
            {
                pState = CSimFlavor::getStringVal(24);
                retVal = sim_propertyret_ok;
            }
            else if (strcmp(pName, propApp_dongleID.name) == 0)
            {
                pState = CSimFlavor::getStringVal(25);
                retVal = sim_propertyret_ok;
            }
            else if (strcmp(pName, propApp_machineIDX.name) == 0)
            {
                pState = CSimFlavor::getStringVal(26);
                retVal = sim_propertyret_ok;
            }
            else if (strcmp(pName, propApp_machineID0.name) == 0)
            {
                pState = CSimFlavor::getStringVal(27);
                retVal = sim_propertyret_ok;
            }
            else if (strcmp(pName, propApp_machineID1.name) == 0)
            {
                pState = CSimFlavor::getStringVal(28);
                retVal = sim_propertyret_ok;
            }
            else if (strcmp(pName, propApp_machineID2.name) == 0)
            {
                pState = CSimFlavor::getStringVal(29);
                retVal = sim_propertyret_ok;
            }
            else if (strcmp(pName, propApp_machineID3.name) == 0)
            {
                pState = CSimFlavor::getStringVal(30);
                retVal = sim_propertyret_ok;
            }
            else if (strcmp(pName, propApp_randomString.name) == 0)
            {
                pState = utils::generateUniqueAlphaNumericString();
                retVal = sim_propertyret_ok;
            }
            else if (strcmp(pName, propApp_auxAddOn1.name) == 0)
            {
                pState = getAdditionalAddOnScript1();
                retVal = sim_propertyret_ok;
            }
            else if (strcmp(pName, propApp_auxAddOn2.name) == 0)
            {
                pState = getAdditionalAddOnScript2();
                retVal = sim_propertyret_ok;
            }
            else if (strcmp(pName, propApp_startupCode.name) == 0)
            {
                pState = _startupScriptString;
                retVal = sim_propertyret_ok;
            }
        }
    }
    else if ((target >= sim_object_detachedscriptstart) && (target <= sim_object_detachedscriptend))
    { // sandbox, main, add-ons, or old associated scripts:
        CDetachedScript* script = scenes->getDetachedScriptFromHandle(int(target));
        if (script != nullptr)
        {
            const char* pName = ppName;
            retVal = script->getStringProperty(pName, pState);
        }
        else
            retVal = sim_propertyret_unknowntarget;
    }
    else if ((target >= sim_object_stackstart) && (target <= sim_object_stackend))
        retVal = scenes->interfaceStackContainer->getStringProperty_t(target, ppName, pState);
    else if ((target >= sim_object_customclassstart) && (target < sim_object_customappend) && (scenes != nullptr)) // don't forget classes!
        retVal = scenes->customObjects->getStringProperty_t(target, ppName, pState);
    else if ((target >= sim_object_sceneobjectclassstart) && (target < sim_object_sceneobjectclassend) && (scenes != nullptr))
        retVal = scenes->customSceneObjectClasses->getStringProperty_t(target, ppName, pState);
    else if (scene != nullptr)
        retVal = scene->getStringProperty_t(target, pName, pState);
    else
        retVal = sim_propertyret_unknowntarget;
    return retVal;
}

int App::setTableProperty_t(long long int target, const char* ppName, const std::string& pState)
{
    int retVal = sim_propertyret_unknownproperty;
    if (!_resolveTarget(target))
        retVal = sim_propertyret_unknowntarget;

    const char* pName = ppName;
    if (target == sim_handle_app)
    {
    }
    else if ((target >= sim_object_detachedscriptstart) && (target <= sim_object_detachedscriptend))
    { // sandbox, main, add-ons, or old associated scripts:
        CDetachedScript* script = scenes->getDetachedScriptFromHandle(int(target));
        if (script != nullptr)
        {
        }
        else
            retVal = sim_propertyret_unknowntarget;
    }
    else if ((target >= sim_object_stackstart) && (target <= sim_object_stackend))
    {
    }
    else if ((target >= sim_object_customclassstart) && (target < sim_object_customappend) && (scenes != nullptr)) // don't forget classes!
        retVal = scenes->customObjects->setTableProperty_t(target, ppName, pState);
    else if ((target >= sim_object_sceneobjectclassstart) && (target < sim_object_sceneobjectclassend) && (scenes != nullptr))
        retVal = scenes->customSceneObjectClasses->setTableProperty_t(target, ppName, pState);
    else if (scene != nullptr)
    {
    }
    else
        retVal = sim_propertyret_unknowntarget;
    return retVal;
}

int App::getTableProperty_t(long long int target, const char* ppName, std::string& pState)
{
    int retVal = sim_propertyret_unknownproperty;
    if (!_resolveTarget(target))
        retVal = sim_propertyret_unknowntarget;

    const char* pName = ppName;
    if (target == sim_handle_app)
    {
    }
    else if ((target >= sim_object_detachedscriptstart) && (target <= sim_object_detachedscriptend))
    { // sandbox, main, add-ons, or old associated scripts:
        CDetachedScript* script = scenes->getDetachedScriptFromHandle(int(target));
        if (script != nullptr)
        {
        }
        else
            retVal = sim_propertyret_unknowntarget;
    }
    else if ((target >= sim_object_stackstart) && (target <= sim_object_stackend))
    {
    }
    else if ((target >= sim_object_customclassstart) && (target < sim_object_customappend) && (scenes != nullptr)) // don't forget classes!
        retVal = scenes->customObjects->getTableProperty_t(target, ppName, pState);
    else if ((target >= sim_object_sceneobjectclassstart) && (target < sim_object_sceneobjectclassend) && (scenes != nullptr))
        retVal = scenes->customSceneObjectClasses->getTableProperty_t(target, ppName, pState);
    else if (scene != nullptr)
    {
    }
    else
        retVal = sim_propertyret_unknowntarget;
    return retVal;
}

int App::setBufferProperty_t(long long int target, const char* ppName, const std::string& pState)
{
    int retVal = sim_propertyret_unknownproperty;
    if (!_resolveTarget(target))
        retVal = sim_propertyret_unknowntarget;

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
                if (_appStorage->writeData(pN.c_str(), pState, true, true))
                {
                    if ((scenes != nullptr) && scenes->getEventsEnabled())
                    {
                        CCbor* ev = scenes->createObjectChangedEvent(sim_handle_app, nullptr, false);
                        _appStorage->appendEventData(pN.c_str(), ev);
                        scenes->pushEvent();
                    }
                }
                retVal = sim_propertyret_ok;
            }
        }
        else if (utils::replaceSubstringStart(pN, SIGNALPREFIX, ""))
        {
            if (pN.size() > 0)
            {
                if (scenes != nullptr)
                {
                    bool diff = scenes->customAppData_volatile.setData(pN.c_str(), pState.data(), pState.size(), true);
                    if (diff && scenes->getEventsEnabled())
                    {
                        CCbor* ev = scenes->createObjectChangedEvent(sim_handle_app, nullptr, false);
                        scenes->customAppData_volatile.appendEventData(pN.c_str(), ev);
                        scenes->pushEvent();
                    }
                    retVal = sim_propertyret_ok;
                }
                else
                    retVal = 0;
            }
        }
    }
    else if ((target >= sim_object_detachedscriptstart) && (target <= sim_object_detachedscriptend))
    { // sandbox, main, add-ons, or old associated scripts:
        CDetachedScript* script = scenes->getDetachedScriptFromHandle(int(target));
        if (script != nullptr)
        {
          //const char* pName = ppName;
          //retVal = script->setBufferProperty(pName, buffer, bufferL);
        }
        else
            retVal = sim_propertyret_unknowntarget;
    }
    else if ((target >= sim_object_stackstart) && (target <= sim_object_stackend))
        retVal = scenes->interfaceStackContainer->setBufferProperty_t(target, ppName, pState);
    else if ((target >= sim_object_customclassstart) && (target < sim_object_customappend) && (scenes != nullptr)) // don't forget classes!
        retVal = scenes->customObjects->setBufferProperty_t(target, ppName, pState);
    else if ((target >= sim_object_sceneobjectclassstart) && (target < sim_object_sceneobjectclassend) && (scenes != nullptr))
        retVal = scenes->customSceneObjectClasses->setBufferProperty_t(target, ppName, pState);
    else if (scene != nullptr)
        retVal = scene->setBufferProperty_t(target, pName, pState);
    else
        retVal = sim_propertyret_unknowntarget;
    return retVal;
}

int App::getBufferProperty_t(long long int target, const char* ppName, std::string& pState)
{
    int retVal = sim_propertyret_unknownproperty;
    if (!_resolveTarget(target))
        retVal = sim_propertyret_unknowntarget;

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
                    retVal = sim_propertyret_ok;
            }
        }
        else if (utils::replaceSubstringStart(pN, SIGNALPREFIX, ""))
        {
            if (pN.size() > 0)
            {
                if (scenes != nullptr)
                {
                    if (scenes->customAppData_volatile.hasData(pN.c_str(), false) >= 0)
                    {
                        pState = scenes->customAppData_volatile.getData(pN.c_str());
                        retVal = sim_propertyret_ok;
                    }
                }
            }
        }
    }
    else if ((target >= sim_object_detachedscriptstart) && (target <= sim_object_detachedscriptend))
    { // sandbox, main, add-ons, or old associated scripts:
        CDetachedScript* script = scenes->getDetachedScriptFromHandle(int(target));
        if (script != nullptr)
        {
          //const char* pName = ppName;
          //retVal = script->getBufferProperty(pName, pState);
        }
        else
            retVal = sim_propertyret_unknowntarget;
    }
    else if ((target >= sim_object_stackstart) && (target <= sim_object_stackend))
        retVal = scenes->interfaceStackContainer->getBufferProperty_t(target, ppName, pState);
    else if ((target >= sim_object_customclassstart) && (target < sim_object_customappend) && (scenes != nullptr)) // don't forget classes!
        retVal = scenes->customObjects->getBufferProperty_t(target, ppName, pState);
    else if ((target >= sim_object_sceneobjectclassstart) && (target < sim_object_sceneobjectclassend) && (scenes != nullptr))
        retVal = scenes->customSceneObjectClasses->getBufferProperty_t(target, ppName, pState);
    else if (scene != nullptr)
        retVal = scene->getBufferProperty_t(target, pName, pState);
    else
        retVal = sim_propertyret_unknowntarget;
    return retVal;
}

int App::setIntArray2Property_t(long long int target, const char* ppName, const int* pState)
{
    int retVal = sim_propertyret_unknownproperty;
    if (!_resolveTarget(target))
        retVal = sim_propertyret_unknowntarget;

    const char* pName = ppName;
    if (target == sim_handle_app)
    {
    }
    else if ((target >= sim_object_detachedscriptstart) && (target <= sim_object_detachedscriptend))
    { // sandbox, main, add-ons, or old associated scripts:
        CDetachedScript* script = scenes->getDetachedScriptFromHandle(int(target));
        if (script != nullptr)
        {
          //const char* pName = ppName;
          //retVal = script->setIntArray2Property(pName, pState);
        }
        else
            retVal = sim_propertyret_unknowntarget;
    }
    else if ((target >= sim_object_stackstart) && (target <= sim_object_stackend))
        retVal = scenes->interfaceStackContainer->setIntArray2Property_t(target, ppName, pState);
    else if ((target >= sim_object_customclassstart) && (target < sim_object_customappend) && (scenes != nullptr)) // don't forget classes!
        retVal = scenes->customObjects->setIntArray2Property_t(target, ppName, pState);
    else if ((target >= sim_object_sceneobjectclassstart) && (target < sim_object_sceneobjectclassend) && (scenes != nullptr))
        retVal = scenes->customSceneObjectClasses->setIntArray2Property_t(target, ppName, pState);
    else if (scene != nullptr)
        retVal = scene->setIntArray2Property_t(target, pName, pState);
    else
        retVal = sim_propertyret_unknowntarget;
    return retVal;
}

int App::getIntArray2Property_t(long long int target, const char* ppName, int* pState)
{
    int retVal = sim_propertyret_unknownproperty;
    if (!_resolveTarget(target))
        retVal = sim_propertyret_unknowntarget;

    const char* pName = ppName;
    if (target == sim_handle_app)
    {
    }
    else if ((target >= sim_object_detachedscriptstart) && (target <= sim_object_detachedscriptend))
    { // sandbox, main, add-ons, or old associated scripts:
        CDetachedScript* script = scenes->getDetachedScriptFromHandle(int(target));
        if (script != nullptr)
        {
          //const char* pName = ppName;
          //retVal = script->getIntArray2Property(pName, pState);
        }
        else
            retVal = sim_propertyret_unknowntarget;
    }
    else if ((target >= sim_object_stackstart) && (target <= sim_object_stackend))
        retVal = scenes->interfaceStackContainer->getIntArray2Property_t(target, ppName, pState);
    else if ((target >= sim_object_customclassstart) && (target < sim_object_customappend) && (scenes != nullptr)) // don't forget classes!
        retVal = scenes->customObjects->getIntArray2Property_t(target, ppName, pState);
    else if ((target >= sim_object_sceneobjectclassstart) && (target < sim_object_sceneobjectclassend) && (scenes != nullptr))
        retVal = scenes->customSceneObjectClasses->getIntArray2Property_t(target, ppName, pState);
    else if (scene != nullptr)
        retVal = scene->getIntArray2Property_t(target, pName, pState);
    else
        retVal = sim_propertyret_unknowntarget;
    return retVal;
}

int App::setVector3Property_t(long long int target, const char* ppName, const C3Vector& pState)
{
    int retVal = sim_propertyret_unknownproperty;
    if (!_resolveTarget(target))
        retVal = sim_propertyret_unknowntarget;

    const char* pName = ppName;
    if (target == sim_handle_app)
    {
    }
    else if ((target >= sim_object_detachedscriptstart) && (target <= sim_object_detachedscriptend))
    { // sandbox, main, add-ons, or old associated scripts:
        CDetachedScript* script = scenes->getDetachedScriptFromHandle(int(target));
        if (script != nullptr)
        {
          //const char* pName = ppName;
          //retVal = script->setVector3Property(pName, pState);
        }
        else
            retVal = sim_propertyret_unknowntarget;
    }
    else if ((target >= sim_object_stackstart) && (target <= sim_object_stackend))
        retVal = scenes->interfaceStackContainer->setVector3Property_t(target, ppName, pState);
    else if ((target >= sim_object_customclassstart) && (target < sim_object_customappend) && (scenes != nullptr)) // don't forget classes!
        retVal = scenes->customObjects->setVector3Property_t(target, ppName, pState);
    else if ((target >= sim_object_sceneobjectclassstart) && (target < sim_object_sceneobjectclassend) && (scenes != nullptr))
        retVal = scenes->customSceneObjectClasses->setVector3Property_t(target, ppName, pState);
    else if (scene != nullptr)
        retVal = scene->setVector3Property_t(target, pName, pState);
    else
        retVal = sim_propertyret_unknowntarget;
    return retVal;
}

int App::getVector3Property_t(long long int target, const char* ppName, C3Vector& pState)
{
    int retVal = sim_propertyret_unknownproperty;
    if (!_resolveTarget(target))
        retVal = sim_propertyret_unknowntarget;

    const char* pName = ppName;
    if (target == sim_handle_app)
    {
    }
    else if ((target >= sim_object_detachedscriptstart) && (target <= sim_object_detachedscriptend))
    { // sandbox, main, add-ons, or old associated scripts:
        CDetachedScript* script = scenes->getDetachedScriptFromHandle(int(target));
        if (script != nullptr)
        {
          //const char* pName = ppName;
          //retVal = script->getVector3Property(pName, pState);
        }
        else
            retVal = sim_propertyret_unknowntarget;
    }
    else if ((target >= sim_object_stackstart) && (target <= sim_object_stackend))
        retVal = scenes->interfaceStackContainer->getVector3Property_t(target, ppName, pState);
    else if ((target >= sim_object_customclassstart) && (target < sim_object_customappend) && (scenes != nullptr)) // don't forget classes!
        retVal = scenes->customObjects->getVector3Property_t(target, ppName, pState);
    else if ((target >= sim_object_sceneobjectclassstart) && (target < sim_object_sceneobjectclassend) && (scenes != nullptr))
        retVal = scenes->customSceneObjectClasses->getVector3Property_t(target, ppName, pState);
    else if (scene != nullptr)
        retVal = scene->getVector3Property_t(target, pName, pState);
    else
        retVal = sim_propertyret_unknowntarget;
    return retVal;
}

int App::setMatrixProperty_t(long long int target, const char* ppName, const CMatrix& pState)
{
    int retVal = sim_propertyret_unknownproperty;
    if (!_resolveTarget(target))
        retVal = sim_propertyret_unknowntarget;

    const char* pName = ppName;
    if (target == sim_handle_app)
    {
    }
    else if ((target >= sim_object_detachedscriptstart) && (target <= sim_object_detachedscriptend))
    { // sandbox, main, add-ons, or old associated scripts:
        CDetachedScript* script = scenes->getDetachedScriptFromHandle(int(target));
        if (script != nullptr)
        {
        }
        else
            retVal = sim_propertyret_unknowntarget;
    }
    else if ((target >= sim_object_stackstart) && (target <= sim_object_stackend))
        retVal = scenes->interfaceStackContainer->setMatrixProperty_t(target, ppName, pState);
    else if ((target >= sim_object_customclassstart) && (target < sim_object_customappend) && (scenes != nullptr)) // don't forget classes!
        retVal = scenes->customObjects->setMatrixProperty_t(target, ppName, pState);
    else if ((target >= sim_object_sceneobjectclassstart) && (target < sim_object_sceneobjectclassend) && (scenes != nullptr))
        retVal = scenes->customSceneObjectClasses->setMatrixProperty_t(target, ppName, pState);
    else if (scene != nullptr)
        retVal = scene->setMatrixProperty_t(target, pName, pState);
    else
        retVal = sim_propertyret_unknowntarget;
    return retVal;
}

int App::getMatrixProperty_t(long long int target, const char* ppName, CMatrix& pState)
{
    int retVal = sim_propertyret_unknownproperty;
    if (!_resolveTarget(target))
        retVal = sim_propertyret_unknowntarget;

    const char* pName = ppName;
    if (target == sim_handle_app)
    {
    }
    else if ((target >= sim_object_detachedscriptstart) && (target <= sim_object_detachedscriptend))
    { // sandbox, main, add-ons, or old associated scripts:
        CDetachedScript* script = scenes->getDetachedScriptFromHandle(int(target));
        if (script != nullptr)
        {
        }
        else
            retVal = sim_propertyret_unknowntarget;
    }
    else if ((target >= sim_object_stackstart) && (target <= sim_object_stackend))
        retVal = scenes->interfaceStackContainer->getMatrixProperty_t(target, ppName, pState);
    else if ((target >= sim_object_customclassstart) && (target < sim_object_customappend) && (scenes != nullptr)) // don't forget classes!
        retVal = scenes->customObjects->getMatrixProperty_t(target, ppName, pState);
    else if ((target >= sim_object_sceneobjectclassstart) && (target < sim_object_sceneobjectclassend) && (scenes != nullptr))
        retVal = scenes->customSceneObjectClasses->getMatrixProperty_t(target, ppName, pState);
    else if (scene != nullptr)
        retVal = scene->getMatrixProperty_t(target, pName, pState);
    else
        retVal = sim_propertyret_unknowntarget;
    return retVal;
}

int App::setQuaternionProperty_t(long long int target, const char* ppName, const C4Vector& pState)
{
    int retVal = sim_propertyret_unknownproperty;
    if (!_resolveTarget(target))
        retVal = sim_propertyret_unknowntarget;

    const char* pName = ppName;
    if (target == sim_handle_app)
    {
    }
    else if ((target >= sim_object_detachedscriptstart) && (target <= sim_object_detachedscriptend))
    { // sandbox, main, add-ons, or old associated scripts:
        CDetachedScript* script = scenes->getDetachedScriptFromHandle(int(target));
        if (script != nullptr)
        {
          //const char* pName = ppName;
          //retVal = script->setQuaternionProperty(pName, pState);
        }
        else
            retVal = sim_propertyret_unknowntarget;
    }
    else if ((target >= sim_object_stackstart) && (target <= sim_object_stackend))
        retVal = scenes->interfaceStackContainer->setQuaternionProperty_t(target, ppName, pState);
    else if ((target >= sim_object_customclassstart) && (target < sim_object_customappend) && (scenes != nullptr)) // don't forget classes!
        retVal = scenes->customObjects->setQuaternionProperty_t(target, ppName, pState);
    else if ((target >= sim_object_sceneobjectclassstart) && (target < sim_object_sceneobjectclassend) && (scenes != nullptr))
        retVal = scenes->customSceneObjectClasses->setQuaternionProperty_t(target, ppName, pState);
    else if (scene != nullptr)
        retVal = scene->setQuaternionProperty_t(target, pName, pState);
    else
        retVal = sim_propertyret_unknowntarget;
    return retVal;
}

int App::getQuaternionProperty_t(long long int target, const char* ppName, C4Vector& pState)
{
    int retVal = sim_propertyret_unknownproperty;
    if (!_resolveTarget(target))
        retVal = sim_propertyret_unknowntarget;

    const char* pName = ppName;
    if (target == sim_handle_app)
    {
        if (strcmp(pName, propApp_randomQuaternion.name) == 0)
        {
            pState.buildRandomOrientation();
            retVal = sim_propertyret_ok;
        }
    }
    else if ((target >= sim_object_detachedscriptstart) && (target <= sim_object_detachedscriptend))
    { // sandbox, main, add-ons, or old associated scripts:
        CDetachedScript* script = scenes->getDetachedScriptFromHandle(int(target));
        if (script != nullptr)
        {
          //const char* pName = ppName;
          //retVal = script->getQuaternionProperty(pName, pState);
        }
        else
            retVal = sim_propertyret_unknowntarget;
    }
    else if ((target >= sim_object_stackstart) && (target <= sim_object_stackend))
        retVal = scenes->interfaceStackContainer->getQuaternionProperty_t(target, ppName, pState);
    else if ((target >= sim_object_customclassstart) && (target < sim_object_customappend) && (scenes != nullptr)) // don't forget classes!
        retVal = scenes->customObjects->getQuaternionProperty_t(target, ppName, pState);
    else if ((target >= sim_object_sceneobjectclassstart) && (target < sim_object_sceneobjectclassend) && (scenes != nullptr))
        retVal = scenes->customSceneObjectClasses->getQuaternionProperty_t(target, ppName, pState);
    else if (scene != nullptr)
        retVal = scene->getQuaternionProperty_t(target, pName, pState);
    else
        retVal = sim_propertyret_unknowntarget;
    return retVal;
}

int App::setPoseProperty_t(long long int target, const char* ppName, const C7Vector& pState)
{
    int retVal = sim_propertyret_unknownproperty;
    if (!_resolveTarget(target))
        retVal = sim_propertyret_unknowntarget;

    const char* pName = ppName;
    if (target == sim_handle_app)
    {
    }
    else if ((target >= sim_object_detachedscriptstart) && (target <= sim_object_detachedscriptend))
    { // sandbox, main, add-ons, or old associated scripts:
        CDetachedScript* script = scenes->getDetachedScriptFromHandle(int(target));
        if (script != nullptr)
        {
          //const char* pName = ppName;
          //retVal = script->setPoseProperty(pName, pState);
        }
        else
            retVal = sim_propertyret_unknowntarget;
    }
    else if ((target >= sim_object_stackstart) && (target <= sim_object_stackend))
        retVal = scenes->interfaceStackContainer->setPoseProperty_t(target, ppName, pState);
    else if ((target >= sim_object_customclassstart) && (target < sim_object_customappend) && (scenes != nullptr)) // don't forget classes!
        retVal = scenes->customObjects->setPoseProperty_t(target, ppName, pState);
    else if ((target >= sim_object_sceneobjectclassstart) && (target < sim_object_sceneobjectclassend) && (scenes != nullptr))
        retVal = scenes->customSceneObjectClasses->setPoseProperty_t(target, ppName, pState);
    else if (scene != nullptr)
        retVal = scene->setPoseProperty_t(target, pName, pState);
    else
        retVal = sim_propertyret_unknowntarget;
    return retVal;
}

int App::getPoseProperty_t(long long int target, const char* ppName, C7Vector& pState)
{
    int retVal = sim_propertyret_unknownproperty;
    if (!_resolveTarget(target))
        retVal = sim_propertyret_unknowntarget;

    const char* pName = ppName;
    if (target == sim_handle_app)
    {
    }
    else if ((target >= sim_object_detachedscriptstart) && (target <= sim_object_detachedscriptend))
    { // sandbox, main, add-ons, or old associated scripts:
        CDetachedScript* script = scenes->getDetachedScriptFromHandle(int(target));
        if (script != nullptr)
        {
          //const char* pName = ppName;
          //retVal = script->getPoseProperty(pName, pState);
        }
        else
            retVal = sim_propertyret_unknowntarget;
    }
    else if ((target >= sim_object_stackstart) && (target <= sim_object_stackend))
        retVal = scenes->interfaceStackContainer->getPoseProperty_t(target, ppName, pState);
    else if ((target >= sim_object_customclassstart) && (target < sim_object_customappend) && (scenes != nullptr)) // don't forget classes!
        retVal = scenes->customObjects->getPoseProperty_t(target, ppName, pState);
    else if ((target >= sim_object_sceneobjectclassstart) && (target < sim_object_sceneobjectclassend) && (scenes != nullptr))
        retVal = scenes->customSceneObjectClasses->getPoseProperty_t(target, ppName, pState);
    else if (scene != nullptr)
        retVal = scene->getPoseProperty_t(target, pName, pState);
    else
        retVal = sim_propertyret_unknowntarget;
    return retVal;
}

int App::setColorProperty_t(long long int target, const char* ppName, const float* pState)
{
    int retVal = sim_propertyret_unknownproperty;
    if (!_resolveTarget(target))
        retVal = sim_propertyret_unknowntarget;

    const char* pName = ppName;
    if (target == sim_handle_app)
    {
    }
    else if ((target >= sim_object_detachedscriptstart) && (target <= sim_object_detachedscriptend))
    { // sandbox, main, add-ons, or old associated scripts:
        CDetachedScript* script = scenes->getDetachedScriptFromHandle(int(target));
        if (script != nullptr)
        {
          //const char* pName = ppName;
          //retVal = script->setColorProperty(pName, pState);
        }
        else
            retVal = sim_propertyret_unknowntarget;
    }
    else if ((target >= sim_object_stackstart) && (target <= sim_object_stackend))
        retVal = scenes->interfaceStackContainer->setColorProperty_t(target, ppName, pState);
    else if ((target >= sim_object_customclassstart) && (target < sim_object_customappend) && (scenes != nullptr)) // don't forget classes!
        retVal = scenes->customObjects->setColorProperty_t(target, ppName, pState);
    else if ((target >= sim_object_sceneobjectclassstart) && (target < sim_object_sceneobjectclassend) && (scenes != nullptr))
        retVal = scenes->customSceneObjectClasses->setColorProperty_t(target, ppName, pState);
    else if (scene != nullptr)
        retVal = scene->setColorProperty_t(target, pName, pState);
    else
        retVal = sim_propertyret_unknowntarget;
    return retVal;
}

int App::getColorProperty_t(long long int target, const char* ppName, float* pState)
{
    int retVal = sim_propertyret_unknownproperty;
    if (!_resolveTarget(target))
        retVal = sim_propertyret_unknowntarget;

    const char* pName = ppName;
    if (target == sim_handle_app)
    {
    }
    else if ((target >= sim_object_detachedscriptstart) && (target <= sim_object_detachedscriptend))
    { // sandbox, main, add-ons, or old associated scripts:
        CDetachedScript* script = scenes->getDetachedScriptFromHandle(int(target));
        if (script != nullptr)
        {
          //const char* pName = ppName;
          //retVal = script->getColorProperty(pName, pState);
        }
        else
            retVal = sim_propertyret_unknowntarget;
    }
    else if ((target >= sim_object_stackstart) && (target <= sim_object_stackend))
        retVal = scenes->interfaceStackContainer->getColorProperty_t(target, ppName, pState);
    else if ((target >= sim_object_customclassstart) && (target < sim_object_customappend) && (scenes != nullptr)) // don't forget classes!
        retVal = scenes->customObjects->getColorProperty_t(target, ppName, pState);
    else if ((target >= sim_object_sceneobjectclassstart) && (target < sim_object_sceneobjectclassend) && (scenes != nullptr))
        retVal = scenes->customSceneObjectClasses->getColorProperty_t(target, ppName, pState);
    else if (scene != nullptr)
        retVal = scene->getColorProperty_t(target, pName, pState);
    else
        retVal = sim_propertyret_unknowntarget;
    return retVal;
}

int App::setFloatArrayProperty_t(long long int target, const char* ppName, const std::vector<double>& pState)
{
    int retVal = sim_propertyret_unknownproperty;
    if (!_resolveTarget(target))
        retVal = sim_propertyret_unknowntarget;

    const char* pName = ppName;
    if (target == sim_handle_app)
    {
    }
    else if ((target >= sim_object_detachedscriptstart) && (target <= sim_object_detachedscriptend))
    { // sandbox, main, add-ons, or old associated scripts:
        CDetachedScript* script = scenes->getDetachedScriptFromHandle(int(target));
        if (script != nullptr)
        {
          //const char* pName = ppName;
          //retVal = script->setFloatArrayProperty(pName, v, vL);
        }
        else
            retVal = sim_propertyret_unknowntarget;
    }
    else if ((target >= sim_object_stackstart) && (target <= sim_object_stackend))
        retVal = scenes->interfaceStackContainer->setFloatArrayProperty_t(target, ppName, pState);
    else if ((target >= sim_object_customclassstart) && (target < sim_object_customappend) && (scenes != nullptr)) // don't forget classes!
        retVal = scenes->customObjects->setFloatArrayProperty_t(target, ppName, pState);
    else if ((target >= sim_object_sceneobjectclassstart) && (target < sim_object_sceneobjectclassend) && (scenes != nullptr))
        retVal = scenes->customSceneObjectClasses->setFloatArrayProperty_t(target, ppName, pState);
    else if (scene != nullptr)
        retVal = scene->setFloatArrayProperty_t(target, pName, pState);
    else
        retVal = sim_propertyret_unknowntarget;
    return retVal;
}

int App::getFloatArrayProperty_t(long long int target, const char* ppName, std::vector<double>& pState)
{
    int retVal = sim_propertyret_unknownproperty;
    if (!_resolveTarget(target))
        retVal = sim_propertyret_unknowntarget;

    pState.clear();
    const char* pName = ppName;
    if (target == sim_handle_app)
    {
    }
    else if ((target >= sim_object_detachedscriptstart) && (target <= sim_object_detachedscriptend))
    { // sandbox, main, add-ons, or old associated scripts:
        CDetachedScript* script = scenes->getDetachedScriptFromHandle(int(target));
        if (script != nullptr)
        {
          //const char* pName = ppName;
          //retVal = script->getFloatArrayProperty(pName, pState);
        }
        else
            retVal = sim_propertyret_unknowntarget;
    }
    else if ((target >= sim_object_stackstart) && (target <= sim_object_stackend))
        retVal = scenes->interfaceStackContainer->getFloatArrayProperty_t(target, ppName, pState);
    else if ((target >= sim_object_customclassstart) && (target < sim_object_customappend) && (scenes != nullptr)) // don't forget classes!
        retVal = scenes->customObjects->getFloatArrayProperty_t(target, ppName, pState);
    else if ((target >= sim_object_sceneobjectclassstart) && (target < sim_object_sceneobjectclassend) && (scenes != nullptr))
        retVal = scenes->customSceneObjectClasses->getFloatArrayProperty_t(target, ppName, pState);
    else if (scene != nullptr)
        retVal = scene->getFloatArrayProperty_t(target, pName, pState);
    else
        retVal = sim_propertyret_unknowntarget;
    return retVal;
}

int App::setIntArrayProperty_t(long long int target, const char* ppName, const std::vector<int>& pState)
{
    int retVal = sim_propertyret_unknownproperty;
    if (!_resolveTarget(target))
        retVal = sim_propertyret_unknowntarget;

    const char* pName = ppName;
    if (target == sim_handle_app)
    {
    }
    else if ((target >= sim_object_detachedscriptstart) && (target <= sim_object_detachedscriptend))
    { // sandbox, main, add-ons, or old associated scripts:
        CDetachedScript* script = scenes->getDetachedScriptFromHandle(int(target));
        if (script != nullptr)
        {
          //const char* pName = ppName;
          //retVal = script->setIntArrayProperty(pName, v, vL);
        }
        else
            retVal = sim_propertyret_unknowntarget;
    }
    else if ((target >= sim_object_stackstart) && (target <= sim_object_stackend))
        retVal = scenes->interfaceStackContainer->setIntArrayProperty_t(target, ppName, pState);
    else if ((target >= sim_object_customclassstart) && (target < sim_object_customappend) && (scenes != nullptr)) // don't forget classes!
        retVal = scenes->customObjects->setIntArrayProperty_t(target, ppName, pState);
    else if ((target >= sim_object_sceneobjectclassstart) && (target < sim_object_sceneobjectclassend) && (scenes != nullptr))
        retVal = scenes->customSceneObjectClasses->setIntArrayProperty_t(target, ppName, pState);
    else if (scene != nullptr)
        retVal = scene->setIntArrayProperty_t(target, pName, pState);
    else
        retVal = sim_propertyret_unknowntarget;
    return retVal;
}

int App::getIntArrayProperty_t(long long int target, const char* ppName, std::vector<int>& pState)
{
    int retVal = sim_propertyret_unknownproperty;
    if (!_resolveTarget(target))
        retVal = sim_propertyret_unknowntarget;

    pState.clear();
    const char* pName = ppName;
    if (target == sim_handle_app)
    {
    }
    else if ((target >= sim_object_detachedscriptstart) && (target <= sim_object_detachedscriptend))
    { // sandbox, main, add-ons, or old associated scripts:
        CDetachedScript* script = scenes->getDetachedScriptFromHandle(int(target));
        if (script != nullptr)
        {
          //const char* pName = ppName;
          //retVal = script->getIntArrayProperty(pName, pState);
        }
        else
            retVal = sim_propertyret_unknowntarget;
    }
    else if ((target >= sim_object_stackstart) && (target <= sim_object_stackend))
        retVal = scenes->interfaceStackContainer->getIntArrayProperty_t(target, ppName, pState);
    else if ((target >= sim_object_customclassstart) && (target < sim_object_customappend) && (scenes != nullptr)) // don't forget classes!
        retVal = scenes->customObjects->getIntArrayProperty_t(target, ppName, pState);
    else if ((target >= sim_object_sceneobjectclassstart) && (target < sim_object_sceneobjectclassend) && (scenes != nullptr))
        retVal = scenes->customSceneObjectClasses->getIntArrayProperty_t(target, ppName, pState);
    else if (scene != nullptr)
        retVal = scene->getIntArrayProperty_t(target, pName, pState);
    else
        retVal = sim_propertyret_unknowntarget;
    return retVal;
}

int App::setHandleArrayProperty_t(long long int target, const char* ppName, const std::vector<long long int>& pState)
{
    int retVal = sim_propertyret_unknownproperty;
    if (!_resolveTarget(target))
        retVal = sim_propertyret_unknowntarget;

    const char* pName = ppName;
    if (target == sim_handle_app)
    {
    }
    else if ((target >= sim_object_detachedscriptstart) && (target <= sim_object_detachedscriptend))
    { // sandbox, main, add-ons, or old associated scripts:
        CDetachedScript* script = scenes->getDetachedScriptFromHandle(int(target));
        if (script != nullptr)
        {
          //const char* pName = ppName;
          //retVal = script->setHandleArrayProperty(pName, v, vL);
        }
        else
            retVal = sim_propertyret_unknowntarget;
    }
    else if ((target >= sim_object_stackstart) && (target <= sim_object_stackend))
        retVal = scenes->interfaceStackContainer->setHandleArrayProperty_t(target, ppName, pState);
    else if ((target >= sim_object_customclassstart) && (target < sim_object_customappend) && (scenes != nullptr)) // don't forget classes!
        retVal = scenes->customObjects->setHandleArrayProperty_t(target, ppName, pState);
    else if ((target >= sim_object_sceneobjectclassstart) && (target < sim_object_sceneobjectclassend) && (scenes != nullptr))
        retVal = scenes->customSceneObjectClasses->setHandleArrayProperty_t(target, ppName, pState);
    else if (scene != nullptr)
        retVal = scene->setHandleArrayProperty_t(target, pName, pState);
    else
        retVal = sim_propertyret_unknowntarget;
    return retVal;
}

int App::getHandleArrayProperty_t(long long int target, const char* ppName, std::vector<long long int>& pState)
{
    int retVal = sim_propertyret_unknownproperty;
    if (!_resolveTarget(target))
        retVal = sim_propertyret_unknowntarget;

    pState.clear();
    const char* pName = ppName;
    if (target == sim_handle_app)
    {
        if (strcmp(pName, propApp_addOns.name) == 0)
        {
            std::vector<int> addOnList(scenes->addOnScriptContainer->getAddOnHandles());
            for (size_t i = 0; i < addOnList.size(); i++)
                pState.push_back(addOnList[i]);
            retVal = sim_propertyret_ok;
        }
        else if (strcmp(pName, propApp_customObjects.name) == 0)
        {
            scenes->customObjects->getAllObjectHandles(pState);
            retVal = sim_propertyret_ok;
        }
        else if (strcmp(pName, propApp_customClasses.name) == 0)
        {
            scenes->customObjects->getAllClassHandles(pState);
            retVal = sim_propertyret_ok;
        }
        else if (strcmp(pName, propApp_customSceneObjectClasses.name) == 0)
        {
            scenes->customSceneObjectClasses->getAllClassHandles(pState);
            retVal = sim_propertyret_ok;
        }
    }
    else if ((target >= sim_object_detachedscriptstart) && (target <= sim_object_detachedscriptend))
    { // sandbox, main, add-ons, or old associated scripts:
        CDetachedScript* script = scenes->getDetachedScriptFromHandle(int(target));
        if (script != nullptr)
        {
          //const char* pName = ppName;
          //retVal = script->getHandleArrayProperty(pName, pState);
        }
        else
            retVal = sim_propertyret_unknowntarget;
    }
    else if ((target >= sim_object_stackstart) && (target <= sim_object_stackend))
        retVal = scenes->interfaceStackContainer->getHandleArrayProperty_t(target, ppName, pState);
    else if ((target >= sim_object_customclassstart) && (target < sim_object_customappend) && (scenes != nullptr)) // don't forget classes!
        retVal = scenes->customObjects->getHandleArrayProperty_t(target, ppName, pState);
    else if ((target >= sim_object_sceneobjectclassstart) && (target < sim_object_sceneobjectclassend) && (scenes != nullptr))
        retVal = scenes->customSceneObjectClasses->getHandleArrayProperty_t(target, ppName, pState);
    else if (scene != nullptr)
        retVal = scene->getHandleArrayProperty_t(target, pName, pState);
    else
        retVal = sim_propertyret_unknowntarget;
    return retVal;
}

int App::setStringArrayProperty_t(long long int target, const char* ppName, const std::vector<std::string>& pState)
{
    int retVal = sim_propertyret_unknownproperty;
    if (!_resolveTarget(target))
        retVal = sim_propertyret_unknowntarget;

    const char* pName = ppName;
    if (target == sim_handle_app)
    {
        if (strcmp(pName, propApp_appArgs.name) == 0)
        {
            std::vector<std::string> ps(pState);
            ps.resize(9);
            for (size_t i = 0; i < ps.size(); i++)
                setApplicationArgument(i, ps[i]);
            retVal = sim_propertyret_ok;
        }
    }
    else if ((target >= sim_object_detachedscriptstart) && (target <= sim_object_detachedscriptend))
    { // sandbox, main, add-ons, or old associated scripts:
        CDetachedScript* script = scenes->getDetachedScriptFromHandle(int(target));
        if (script != nullptr)
        {
          //const char* pName = ppName;
          //retVal = script->setStringArrayProperty(pName, pState);
        }
        else
            retVal = sim_propertyret_unknowntarget;
    }
    else if ((target >= sim_object_stackstart) && (target <= sim_object_stackend))
        retVal = scenes->interfaceStackContainer->setStringArrayProperty_t(target, ppName, pState);
    else if ((target >= sim_object_customclassstart) && (target < sim_object_customappend) && (scenes != nullptr)) // don't forget classes!
        retVal = scenes->customObjects->setStringArrayProperty_t(target, ppName, pState);
    else if ((target >= sim_object_sceneobjectclassstart) && (target < sim_object_sceneobjectclassend) && (scenes != nullptr))
        retVal = scenes->customSceneObjectClasses->setStringArrayProperty_t(target, ppName, pState);
    else if (scene != nullptr)
        retVal = scene->setStringArrayProperty_t(target, pName, pState);
    else
        retVal = sim_propertyret_unknowntarget;
    return retVal;
}

int App::getStringArrayProperty_t(long long int target, const char* ppName, std::vector<std::string>& pState)
{
    int retVal = sim_propertyret_unknownproperty;
    if (!_resolveTarget(target))
        retVal = sim_propertyret_unknowntarget;

    pState.clear();
    const char* pName = ppName;
    if (target == sim_handle_app)
    {
        if (strcmp(pName, propApp_appArgs.name) == 0)
        {
            pState = _applicationArguments;
            retVal = sim_propertyret_ok;
        }
        else if (strcmp(pName, propApp_pluginNames.name) == 0)
        {
            pState = _pluginNames;
            retVal = sim_propertyret_ok;
        }
    }
    else if ((target >= sim_object_detachedscriptstart) && (target <= sim_object_detachedscriptend))
    { // sandbox, main, add-ons, or old associated scripts:
        CDetachedScript* script = scenes->getDetachedScriptFromHandle(int(target));
        if (script != nullptr)
        {
          //const char* pName = ppName;
          //retVal = script->getStringArrayProperty(pName, pState);
        }
        else
            retVal = sim_propertyret_unknowntarget;
    }
    else if ((target >= sim_object_stackstart) && (target <= sim_object_stackend))
        retVal = scenes->interfaceStackContainer->getStringArrayProperty_t(target, ppName, pState);
    else if ((target >= sim_object_customclassstart) && (target < sim_object_customappend) && (scenes != nullptr)) // don't forget classes!
        retVal = scenes->customObjects->getStringArrayProperty_t(target, ppName, pState);
    else if ((target >= sim_object_sceneobjectclassstart) && (target < sim_object_sceneobjectclassend) && (scenes != nullptr))
        retVal = scenes->customSceneObjectClasses->getStringArrayProperty_t(target, ppName, pState);
    else if (scene != nullptr)
        retVal = scene->getStringArrayProperty_t(target, pName, pState);
    else
        retVal = sim_propertyret_unknowntarget;
    return retVal;
}

int App::setMethodProperty_t(long long int target, const char* ppName, const void* pState)
{
    int retVal = sim_propertyret_unknownproperty;
    if (!_resolveTarget(target))
        retVal = sim_propertyret_unknowntarget;

    const char* pName = ppName;
    if (target == sim_handle_app)
    {
    }
    else if ((target >= sim_object_detachedscriptstart) && (target <= sim_object_detachedscriptend))
    { // sandbox, main, add-ons, or old associated scripts:
    }
    else if ((target >= sim_object_stackstart) && (target <= sim_object_stackend))
    {
    }
    else if ((target >= sim_object_customclassstart) && (target < sim_object_customappend) && (scenes != nullptr)) // don't forget classes!
        retVal = scenes->customObjects->setMethodProperty_t(target, ppName, pState);
    else if ((target >= sim_object_sceneobjectclassstart) && (target < sim_object_sceneobjectclassend) && (scenes != nullptr))
        retVal = scenes->customSceneObjectClasses->setMethodProperty_t(target, ppName, pState);
    else if (scene != nullptr)
        retVal = scene->setMethodProperty_t(target, ppName, pState);
    else
        retVal = sim_propertyret_unknowntarget;
    return retVal;
}

int App::getMethodProperty_t(long long int target, const char* ppName, void*& pState)
{
    int retVal = sim_propertyret_unknownproperty;
    if (!_resolveTarget(target))
        retVal = sim_propertyret_unknowntarget;

    if (target == sim_handle_app)
    {
    }
    else if ((target >= sim_object_detachedscriptstart) && (target <= sim_object_detachedscriptend))
    { // sandbox, main, add-ons, or old associated scripts:
    }
    else if ((target >= sim_object_stackstart) && (target <= sim_object_stackend))
    {
    }
    else if ((target >= sim_object_customclassstart) && (target < sim_object_customappend) && (scenes != nullptr)) // don't forget classes!
        retVal = scenes->customObjects->getMethodProperty_t(target, ppName, pState);
    else if ((target >= sim_object_sceneobjectclassstart) && (target < sim_object_sceneobjectclassend) && (scenes != nullptr))
        retVal = scenes->customSceneObjectClasses->getMethodProperty_t(target, ppName, pState);
    else if (scene != nullptr)
        retVal = scene->getMethodProperty_t(target, ppName, pState);
    else
        retVal = sim_propertyret_unknowntarget;
    return retVal;
}

int App::setMethodProperty_t(long long int target, const char* ppName, const std::string& pState)
{
    int retVal = sim_propertyret_unknownproperty;
    if (!_resolveTarget(target))
        retVal = sim_propertyret_unknowntarget;

    const char* pName = ppName;
    if (target == sim_handle_app)
    {
    }
    else if ((target >= sim_object_detachedscriptstart) && (target <= sim_object_detachedscriptend))
    { // sandbox, main, add-ons, or old associated scripts:
    }
    else if ((target >= sim_object_stackstart) && (target <= sim_object_stackend))
    {
    }
    else if ((target >= sim_object_customclassstart) && (target < sim_object_customappend) && (scenes != nullptr)) // don't forget classes!
        retVal = scenes->customObjects->setMethodProperty_t(target, ppName, pState);
    else if ((target >= sim_object_sceneobjectclassstart) && (target < sim_object_sceneobjectclassend) && (scenes != nullptr))
        retVal = scenes->customSceneObjectClasses->setMethodProperty_t(target, ppName, pState);
    else if (scene != nullptr)
        retVal = scene->setMethodProperty_t(target, ppName, pState);
    else
        retVal = sim_propertyret_unknowntarget;
    return retVal;
}

int App::getMethodProperty_t(long long int target, const char* ppName, std::string& pState)
{
    int retVal = sim_propertyret_unknownproperty;
    if (!_resolveTarget(target))
        retVal = sim_propertyret_unknowntarget;

    if (target == sim_handle_app)
    {
    }
    else if ((target >= sim_object_detachedscriptstart) && (target <= sim_object_detachedscriptend))
    { // sandbox, main, add-ons, or old associated scripts:
    }
    else if ((target >= sim_object_stackstart) && (target <= sim_object_stackend))
    {
    }
    else if ((target >= sim_object_customclassstart) && (target < sim_object_customappend) && (scenes != nullptr)) // don't forget classes!
        retVal = scenes->customObjects->getMethodProperty_t(target, ppName, pState);
    else if ((target >= sim_object_sceneobjectclassstart) && (target < sim_object_sceneobjectclassend) && (scenes != nullptr))
        retVal = scenes->customSceneObjectClasses->getMethodProperty_t(target, ppName, pState);
    else if (scene != nullptr)
        retVal = scene->getMethodProperty_t(target, ppName, pState);
    else
        retVal = sim_propertyret_unknowntarget;
    return retVal;
}

int App::removeProperty_t(long long int target, const char* ppName)
{
    int retVal = sim_propertyret_unknownproperty;
    if (!_resolveTarget(target))
        retVal = sim_propertyret_unknowntarget;

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
                    if ((scenes != nullptr) && scenes->getEventsEnabled())
                    {
                        CCbor* ev = scenes->createObjectChangedEvent(sim_handle_app, nullptr, false);
                        _appStorage->appendEventData(pN.c_str(), ev, true);
                        scenes->pushEvent();
                    }
                }
                retVal = sim_propertyret_ok;
            }
        }
        else if (utils::replaceSubstringStart(pN, SIGNALPREFIX, ""))
        {
            if (pN.size() > 0)
            {
                if (scenes != nullptr)
                {
                    int tp = scenes->customAppData_volatile.hasData(pN.c_str(), true);
                    if (tp >= 0)
                    {
                        bool diff = scenes->customAppData_volatile.clearData((propertyStrings[tp] + pN).c_str());
                        if (diff && scenes->getEventsEnabled())
                        {
                            CCbor* ev = scenes->createObjectChangedEvent(sim_handle_app, nullptr, false);
                            scenes->customAppData_volatile.appendEventData(pN.c_str(), ev, true);
                            scenes->pushEvent();
                        }
                        retVal = sim_propertyret_ok;
                    }
                }
            }
        }
        else if (utils::replaceSubstringStart(pN, NAMEDPARAMPREFIX, ""))
        {
            if (pN.size() > 0)
            {
                if (removeAppNamedParam(pN.c_str()))
                    retVal = sim_propertyret_ok;
                else
                    retVal = 0;
            }
        }
    }
    else if ((target >= sim_object_detachedscriptstart) && (target <= sim_object_detachedscriptend))
    { // sandbox, main, add-ons, or old associated scripts:
        CDetachedScript* script = scenes->getDetachedScriptFromHandle(int(target));
        if (script != nullptr)
        {
          //const char* pName = ppName;
          //retVal = script->removeProperty(pName);
        }
        else
            retVal = sim_propertyret_unknowntarget;
    }
    else if ((target >= sim_object_stackstart) && (target <= sim_object_stackend))
        retVal = scenes->interfaceStackContainer->removeProperty_t(target, ppName);
    else if ((target >= sim_object_customclassstart) && (target < sim_object_customappend) && (scenes != nullptr)) // don't forget classes!
        retVal = scenes->customObjects->removeProperty_t(target, ppName);
    else if ((target >= sim_object_sceneobjectclassstart) && (target < sim_object_sceneobjectclassend) && (scenes != nullptr))
        retVal = scenes->customSceneObjectClasses->removeProperty_t(target, ppName);
    else if (scene != nullptr)
        retVal = scene->removeProperty_t(target, pName);
    else
        retVal = sim_propertyret_unknowntarget;
    return retVal;
}

int App::getPropertyName_t(long long int target, int& index, std::string& pName, std::string& appartenance, int excludeFlags)
{
    int retVal = sim_propertyret_unknownproperty;
    _resolveTarget(target);

    if (target == sim_handle_app)
    {
        retVal = _obj->getPropertyName(index, pName, appartenance, excludeFlags);
        if (retVal == sim_propertyret_unknownproperty)
        {
            appartenance = _obj->getObjectTypeStr();
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
                            retVal = sim_propertyret_ok;
                            break;
                        }
                    }
                }
            }
            if (retVal == sim_propertyret_unknownproperty)
            {
                //CPersistentDataContainer cont("appStorage.dat");
                if (_appStorage->getPropertyName(index, pName, excludeFlags))
                {
                    pName = CUSTOMDATAPREFIX + pName;
                    retVal = sim_propertyret_ok;
                }
            }
            if ((retVal == sim_propertyret_unknownproperty) && (scenes != nullptr))
            {
                if (scenes->customAppData_volatile.getPropertyName(index, pName, excludeFlags))
                {
                    pName = SIGNALPREFIX + pName;
                    retVal = sim_propertyret_ok;
                }
            }
            if (retVal == sim_propertyret_unknownproperty)
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
                                retVal = sim_propertyret_ok;
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
    else if ((target >= sim_object_customclassstart) && (target < sim_object_customappend) && (scenes != nullptr)) // don't forget classes!
        retVal = scenes->customObjects->getPropertyName_t(target, index, pName, appartenance, excludeFlags);
    else if ((target >= sim_object_sceneobjectclassstart) && (target < sim_object_sceneobjectclassend) && (scenes != nullptr))
        retVal = scenes->customSceneObjectClasses->getPropertyName_t(target, index, pName, appartenance, excludeFlags);
    else if ((target >= sim_object_detachedscriptstart) && (target <= sim_object_detachedscriptend))
    { // sandbox, main, add-ons, or old associated scripts:
        CDetachedScript* script = scenes->getDetachedScriptFromHandle(int(target));
        if (script != nullptr)
        {
            //            if ((script->getScriptType() != sim_scripttype_sandbox) && (script->getScriptType() != sim_scripttype_addon))
            //                appartenance = _objectTypeStr;
            retVal = script->getPropertyName(index, pName, appartenance, excludeFlags);
        }
        else
            retVal = sim_propertyret_unknowntarget;
    }
    else if ((target >= sim_object_stackstart) && (target <= sim_object_stackend))
        retVal = scenes->interfaceStackContainer->getPropertyName_t(target, index, pName, appartenance, excludeFlags);
    else if (scene != nullptr)
        retVal = scene->getPropertyName_t(target, index, pName, appartenance, excludeFlags);
    else
        retVal = sim_propertyret_unknowntarget;
    return retVal;
}

int App::getPropertyInfo_t(long long int target, const char* ppName, int& info, std::string& infoTxt)
{
    int retVal = sim_propertyret_unknownproperty;
    _resolveTarget(target);

    const char* pName = ppName;
    if (target == sim_handle_app)
    {
        retVal = _obj->getPropertyInfo(ppName, info, infoTxt);
        if (retVal == sim_propertyret_unknownproperty)
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
                        auto w = QJsonDocument::fromJson(allProps_app[i].shortInfoTxt.c_str()).object();
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
            if (retVal == sim_propertyret_unknownproperty)
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
            if ((retVal == sim_propertyret_unknownproperty) && (scenes != nullptr))
            {
                std::string pN(pName);
                if (utils::replaceSubstringStart(pN, SIGNALPREFIX, ""))
                {
                    int s;
                    retVal = scenes->customAppData_volatile.hasData(pN.c_str(), true, &s);
                    if (retVal >= 0)
                    {
                        info = SIGNALFLAGS;
                        if (s > LARGE_PROPERTY_SIZE)
                            info = info | sim_propertyinfo_largedata;
                        infoTxt = "";
                    }
                }
            }
            if (retVal == sim_propertyret_unknownproperty)
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
    else if ((target >= sim_object_customclassstart) && (target < sim_object_customappend) && (scenes != nullptr)) // don't forget classes!
        retVal = scenes->customObjects->getPropertyInfo_t(target, pName, info, infoTxt);
    else if ((target >= sim_object_sceneobjectclassstart) && (target < sim_object_sceneobjectclassend) && (scenes != nullptr))
        retVal = scenes->customSceneObjectClasses->getPropertyInfo_t(target, pName, info, infoTxt);
    else if ((target >= sim_object_detachedscriptstart) && (target <= sim_object_detachedscriptend))
    { // sandbox, main, add-ons, or old associated scripts:
        CDetachedScript* script = scenes->getDetachedScriptFromHandle(int(target));
        if (script != nullptr)
            retVal = script->getPropertyInfo(ppName, info, infoTxt);
        else
            retVal = sim_propertyret_unknowntarget;
    }
    else if ((target >= sim_object_stackstart) && (target <= sim_object_stackend))
        retVal = scenes->interfaceStackContainer->getPropertyInfo_t(target, ppName, info, infoTxt);
    else if (scene != nullptr)
        retVal = scene->getPropertyInfo_t(target, pName, info, infoTxt);
    else
        retVal = sim_propertyret_unknowntarget;
    return retVal;
}

int App::setPropertyInfo_t(long long int target, const char* ppName, int info, const char* infoTxt)
{
    int retVal = sim_propertyret_unknownproperty;
    _resolveTarget(target);

    if (target == sim_handle_app)
        retVal = sim_propertyret_unavailable;
    else if ((target >= sim_object_customclassstart) && (target < sim_object_customappend) && (scenes != nullptr)) // don't forget classes!
        retVal = scenes->customObjects->setPropertyInfo_t(target, ppName, info, infoTxt);
    else if ((target >= sim_object_sceneobjectclassstart) && (target < sim_object_sceneobjectclassend) && (scenes != nullptr))
        retVal = scenes->customSceneObjectClasses->setPropertyInfo_t(target, ppName, info, infoTxt);
    else if ((target >= sim_object_detachedscriptstart) && (target <= sim_object_detachedscriptend))
        retVal = sim_propertyret_unavailable; // sandbox, main, add-ons, or old associated scripts:
    else if ((target >= sim_object_stackstart) && (target <= sim_object_stackend))
        retVal = sim_propertyret_unavailable;
    else if (scene != nullptr)
        retVal = scene->setPropertyInfo_t(target, ppName, info, infoTxt);
    else
        retVal = sim_propertyret_unknowntarget;
    return retVal;
}

bool App::isTargetValid_t(long long int target)
{
    int ind = 0;
    std::string pName, appart;
    return App::getPropertyName_t(target, ind, pName, appart, 0) > 0;
}

bool App::_resolveTarget(long long int& target)
{
    bool retVal = true;
    if (target == sim_handle_sandbox)
    {
        if ((scenes != nullptr) && (scenes->sandboxScript != nullptr))
            target = scenes->sandboxScript->getScriptHandle();
        else
            retVal = false; // target does not exist
    }
    else if (target == sim_handle_mainscript)
    {
        if ((scene != nullptr) && (scene->sceneObjects->embeddedScriptContainer->getMainScript() != nullptr))
            target = scene->sceneObjects->embeddedScriptContainer->getMainScript()->getScriptHandle();
        else
            retVal = false; // target does not exist
    }
    return retVal;
}

void App::setHierarchyEnabled(bool v)
{
    bool diff = (_hierarchyEnabled != v);
    if (diff)
    {
        _hierarchyEnabled = v;
        if ((scenes != nullptr) && scenes->getEventsEnabled())
        {
            const char* cmd = propApp_hierarchyEnabled.name;
            CCbor* ev = scenes->createObjectChangedEvent(sim_handle_app, cmd, true);
            ev->appendKeyBool(cmd, _hierarchyEnabled);
            scenes->pushEvent();
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
        if ((scenes != nullptr) && scenes->getEventsEnabled())
        {
            const char* cmd = propApp_displayEnabled.name;
            CCbor* ev = scenes->createObjectChangedEvent(sim_handle_app, cmd, true);
            ev->appendKeyBool(cmd, _openGlDisplayEnabled);
            scenes->pushEvent();
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
    if (App::scene->simulation->isSimulationStopped() && CSimFlavor::getBoolVal(16))
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

int App::getEventProtocolVersion()
{
    return _eventProtocolVersion;
}

void App::setEventProtocolVersion(int v)
{
    if (v != _eventProtocolVersion)
    {
        _eventProtocolVersion = v;
        pushGenesisEvents();
    }
}

void App::pushGenesisEvents()
{
    if ((scenes != nullptr) && scenes->getEventsEnabled())
    {
        CSimFlavor::getStringVal(22); // trigger calculations and print mids
        CCbor* ev = scenes->createEvent(EVENTTYPE_GENESISBEGIN, -1, -1, nullptr, false);
        scenes->pushEvent();

        if (App::getEventProtocolVersion() == 2)
            ev = scenes->createEvent("appSession", sim_handle_app, sim_handle_app, nullptr, false);
        else
            ev = scenes->createEvent(EVENTTYPE_OBJECTCHANGED, sim_handle_app, sim_handle_app, nullptr, false);
        ev->appendKeyText(propApp_sessionId.name, scenes->getSessionId().c_str());
        ev->appendKeyText(propObject_objectType.name, _obj->getObjectTypeStr().c_str());
        ev->appendKeyInt64(propApp_protocolVersion.name, _eventProtocolVersion);
        ev->appendKeyText(propApp_productVersion.name, SIM_VERSION_STR_SHORT);
        ev->appendKeyInt64(propApp_productVersionNb.name, SIM_PROGRAM_FULL_VERSION_NB);
        ev->appendKeyInt64(propApp_platform.name, getPlatform());
#ifdef SIM_FL
        ev->appendKeyInt64(propApp_flavor.name, SIM_FL);
#else
        ev->appendKeyInt64(propApp_flavor.name, -1);
#endif
        ev->appendKeyInt64(propApp_qtVersion.name, (QT_VERSION >> 16) * 10000 + ((QT_VERSION >> 8) & 255) * 100 + (QT_VERSION & 255) * 1);
        int sbh = -1;
        if (scenes->sandboxScript != nullptr)
            sbh = scenes->sandboxScript->getScriptHandle();
        if (App::getEventProtocolVersion() <= 3)
            ev->appendKeyInt64(propApp_sandbox.name, sbh);
        else
            ev->appendKeyHandle(propApp_sandbox.name, sbh);
        if (instancesList != nullptr)
        {
            ev->appendKeyInt64(propApp_processId.name, instancesList->thisInstanceId());
            ev->appendKeyInt64(propApp_processCnt.name, instancesList->numInstances());
        }
        ev->appendKeyInt64(propApp_consoleVerbosity.name, getConsoleVerbosity());
        ev->appendKeyInt64(propApp_statusbarVerbosity.name, getStatusbarVerbosity());
        ev->appendKeyInt64(propApp_dialogVerbosity.name, getDlgVerbosity());
        ev->appendKeyTextArray(propApp_appArgs.name, _applicationArguments);
        ev->appendKeyTextArray(propApp_pluginNames.name, _pluginNames);
        std::vector<int> addOnList(scenes->addOnScriptContainer->getAddOnHandles());
        if (App::getEventProtocolVersion() <= 3)
            ev->appendKeyInt32Array(propApp_addOns.name, addOnList.data(), addOnList.size());
        else
            ev->appendKeyHandleArray(propApp_addOns.name, addOnList.data(), addOnList.size());

        std::vector<long long int> l;
        scenes->customObjects->getAllObjectHandles(l);
        ev->appendKeyHandleArray(propApp_customObjects.name, l.data(), l.size());
        std::vector<long long int> customClassList;
        scenes->customObjects->getAllClassHandles(l);
        ev->appendKeyHandleArray(propApp_customClasses.name, l.data(), l.size());
        l.clear();
        scenes->customSceneObjectClasses->getAllClassHandles(l);
        ev->appendKeyHandleArray(propApp_customSceneObjectClasses.name, l.data(), l.size());

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

        scenes->customAppData_volatile.appendEventData(nullptr, ev);

        if (userSettings != nullptr)
        {
            ev->appendKeyText(propApp_defaultPython.name, userSettings->defaultPython.c_str());
            ev->appendKeyText(propApp_sandboxLang.name, userSettings->preferredSandboxLang.c_str());
        }
        if (App::getEventProtocolVersion() == 2)
        {
            scenes->pushEvent();
            ev = scenes->createEvent("appSettingsChanged", sim_handle_app, sim_handle_app, nullptr, false);
        }
        if (userSettings != nullptr)
        {
            ev->appendKeyDouble(propApp_defaultTranslationStepSize.name, userSettings->getTranslationStepSize());
            ev->appendKeyDouble(propApp_defaultRotationStepSize.name, userSettings->getRotationStepSize());
            ev->appendKeyInt64(propApp_notifyDeprecated.name, userSettings->notifyDeprecated);
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
        ev->appendKeyInt64(propApp_headlessMode.name, getHeadlessMode());
        ev->appendKeyText(propApp_appArg1.name, getApplicationArgument(0).c_str());
        ev->appendKeyText(propApp_appArg2.name, getApplicationArgument(1).c_str());
        ev->appendKeyText(propApp_appArg3.name, getApplicationArgument(2).c_str());
        ev->appendKeyText(propApp_appArg4.name, getApplicationArgument(3).c_str());
        ev->appendKeyText(propApp_appArg5.name, getApplicationArgument(4).c_str());
        ev->appendKeyText(propApp_appArg6.name, getApplicationArgument(5).c_str());
        ev->appendKeyText(propApp_appArg7.name, getApplicationArgument(6).c_str());
        ev->appendKeyText(propApp_appArg8.name, getApplicationArgument(7).c_str());
        ev->appendKeyText(propApp_appArg9.name, getApplicationArgument(8).c_str());
        ev->appendKeyInt64(propApp_pid.name, pid);

        if (userSettings != nullptr)
            ev->appendKeyInt64(propApp_idleFps.name, userSettings->getIdleFps());

        scenes->pushEvent();

        ev = scenes->createEvent(EVENTTYPE_OBJECTCHANGED, sim_handle_app, sim_handle_app, nullptr, false);
        //CPersistentDataContainer cont("appStorage.dat");
        _appStorage->appendEventData(nullptr, ev);
        scenes->pushEvent();

        if (scenes->sandboxScript != nullptr)
            scenes->sandboxScript->pushObjectCreationEvent();

        if (scenes->addOnScriptContainer != nullptr)
            scenes->addOnScriptContainer->pushGenesisEvents();

        scene->pushGenesisEvents();

        if (scenes->customObjects != nullptr)
            scenes->customObjects->pushGenesisEvents();

        ev = scenes->createEvent(EVENTTYPE_GENESISEND, -1, -1, nullptr, false);
        scenes->pushEvent();
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
        if ((scenes != nullptr) && scenes->getEventsEnabled())
        {
            const char* cmd = propApp_pluginNames.name;
            CCbor* ev = scenes->createObjectChangedEvent(sim_handle_app, cmd, true);
            ev->appendKeyTextArray(cmd, _pluginNames);
            scenes->pushEvent();
        }
    }
}
