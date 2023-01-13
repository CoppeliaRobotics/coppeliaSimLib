#include "app.h"
#include "vThread.h"
#include "ttUtil.h"
#include "pluginContainer.h"
#include "simStrings.h"
#include "vDateTime.h"
#include "pathPlanningInterface.h"
#include "vVarious.h"
#include "tt.h"
#include "persistentDataContainer.h"
#include "apiErrors.h"
#include "mesh.h"
#include "rendering.h"
#include "simFlavor.h"
#include "threadPool_old.h"
#include <sstream>
#include <iomanip>
#include <boost/algorithm/string/replace.hpp>
#include <chrono>
#ifdef SIM_WITH_GUI
    #include "auxLibVideo.h"
    #include "vMessageBox.h"
    #include <QSplashScreen>
    #include <QBitmap>
    #include <QTextStream>
    #include <QScreen>
    #include <QDesktopWidget>
    #ifdef WIN_SIM
        #include <QStyleFactory>
    #endif
#endif
#ifdef SIM_WITH_QT
    #include <QHostInfo>
    #include <QTextDocument>
#endif
void (*_workThreadLoopCallback)();

CUiThread* App::uiThread=nullptr;
CSimThread* App::simThread=nullptr;
CUserSettings* App::userSettings=nullptr;
CFolderSystem* App::folders=nullptr;
int App::operationalUIParts=0; // sim_gui_menubar,sim_gui_popupmenus,sim_gui_toolbar1,sim_gui_toolbar2, etc.
std::string App::_applicationName="CoppeliaSim (Customized)";
CWorldContainer* App::worldContainer=nullptr;
CWorld* App::currentWorld=nullptr;
bool App::_exitRequest=false;
bool App::_browserEnabled=true;
bool App::_canInitSimThread=false;
int App::_consoleVerbosity=sim_verbosity_default;
int App::_statusbarVerbosity=sim_verbosity_msgs;
int App::_dlgVerbosity=sim_verbosity_infos;
int App::_exitCode=0;
bool App::_online=false;
std::string App::_consoleLogFilterStr;
std::string App::_startupScriptString;
long long int App::_nextUniqueId=0;

bool App::_simulatorIsRunning=false;
std::vector<std::string> App::_applicationArguments;
std::map<std::string,std::string> App::_applicationNamedParams;
std::string App::_additionalAddOnScript1;
std::string App::_additionalAddOnScript2;
volatile int App::_quitLevel=0;
bool App::_consoleMsgsToFile=false;
VFile* App::_consoleMsgsFile=nullptr;
VArchive* App::_consoleMsgsArchive=nullptr;
CGm* App::gm=nullptr;

int App::sc=1;
#ifdef SIM_WITH_QT
    CSimQApp* App::qtApp=nullptr;
    int App::_qApp_argc=1;
    char App::_qApp_arg0[]={"CoppeliaSim"};
    char* App::_qApp_argv[1]={_qApp_arg0};
#endif
#ifdef SIM_WITH_GUI
    CMainWindow* App::mainWindow=nullptr;
#endif

bool App::canInitSimThread()
{
    return(_canInitSimThread);
}

bool App::isQtAppBuilt()
{
    return(App::qtApp!=nullptr);
}

// Following simulation thread split into 'simThreadInit', 'simThreadDestroy' and 'simStep' is courtesy of Stephen James:
SIMPLE_VTHREAD_RETURN_TYPE _workThread(SIMPLE_VTHREAD_ARGUMENT_TYPE lpData)
{
    App::simulationThreadInit();
    while (!App::getExitRequest())
        App::simulationThreadLoop();
    App::simulationThreadDestroy();
    return(SIMPLE_VTHREAD_RETURN_VAL);
}

// Following simulation thread split into 'simulationThreadInit', 'simulationThreadDestroy' and 'simulationThreadLoop' is courtesy of Stephen James:
void App::simulationThreadInit()
{
    TRACE_INTERNAL;
    CThreadPool_old::init();
    _canInitSimThread=false;
    VThread::setSimulationMainThreadId();
    srand((int)VDateTime::getTimeInMs());    // Important so that the computer ID has some "true" random component!
                                        // Remember that each thread starts with a same seed!!!
    App::simThread=new CSimThread();
    #ifdef SIM_WITH_QT
        CSimAndUiThreadSync::simThread_forbidUiThreadToWrite(true); // lock initially...
    #endif

    // Send the "instancePass" message to all plugins already here (needed for some plugins to properly finish initialization):
    int auxData[4]={App::worldContainer->getModificationFlags(true),0,0,0};
    void* replyBuffer=CPluginContainer::sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_instancepass,auxData,nullptr,nullptr);
    if (replyBuffer!=nullptr)
        simReleaseBuffer_internal((char*)replyBuffer);
#ifdef SIM_WITH_GUI
    SUIThreadCommand cmdIn;
    SUIThreadCommand cmdOut;
    cmdIn.cmdId=INSTANCE_PASS_FROM_UITHREAD_UITHREADCMD;
    App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
#endif

    App::worldContainer->sandboxScript=new CScriptObject(sim_scripttype_sandboxscript);
    App::worldContainer->sandboxScript->setScriptTextFromFile((App::folders->getSystemPath()+"/"+"sandboxScript.lua").c_str());
    App::worldContainer->sandboxScript->systemCallScript(sim_syscb_init,nullptr,nullptr);
    if (_startupScriptString.size()>0)
    {
        App::worldContainer->sandboxScript->executeScriptString(_startupScriptString.c_str(),nullptr);
        _startupScriptString.clear();
    }
}

// Following simulation thread split into 'simulationThreadInit', 'simulationThreadDestroy' and 'simulationThreadLoop' is courtesy of Stephen James:
void App::simulationThreadDestroy()
{
    // Send the last "instancePass" message to all plugins:
    int auxData[4]={0,0,0,0};
    void* replyBuffer=CPluginContainer::sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_lastinstancepass,auxData,nullptr,nullptr);
    if (replyBuffer!=nullptr)
        simReleaseBuffer_internal((char*)replyBuffer);

    App::worldContainer->addOnScriptContainer->removeAllAddOns();
    App::worldContainer->sandboxScript->systemCallScript(sim_syscb_cleanup,nullptr,nullptr);
    CScriptObject::destroy(App::worldContainer->sandboxScript,true);
    App::worldContainer->sandboxScript=nullptr;

    App::setQuitLevel(1);

    #ifndef SIM_WITH_QT
        SUIThreadCommand cmdIn;
        SUIThreadCommand cmdOut;
        cmdIn.cmdId=NO_SIGNAL_SLOT_EXIT_UITHREADCMD;
        App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
    #else
        App::qtApp->quit();
    #endif
    while (App::getQuitLevel()==1)
        VThread::sleep(1);

    // Ok, the UI thread has left its exec and is waiting for us
    delete App::simThread;
    App::simThread=nullptr;

    App::worldContainer->copyBuffer->clearBuffer(); // important, some objects in the buffer might still call the mesh plugin or similar

    #ifdef SIM_WITH_QT
        CSimAndUiThreadSync::simThread_allowUiThreadToWrite(); // ...finally unlock
    #endif

    App::setQuitLevel(3); // tell the UI thread that we are done here

    VThread::unsetSimulationMainThreadId();
    VThread::endSimpleThread();
}

// Following simulation thread split into 'simulationThreadInit', 'simulationThreadDestroy' and 'simulationThreadLoop' is courtesy of Stephen James:
void App::simulationThreadLoop()
{
    // Send the "instancePass" message to all plugins:
    int auxData[4]={App::worldContainer->getModificationFlags(true),0,0,0};
    void* replyBuffer=CPluginContainer::sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_instancepass,auxData,nullptr,nullptr);
    if (replyBuffer!=nullptr)
        simReleaseBuffer_internal((char*)replyBuffer);
#ifdef SIM_WITH_GUI
    SUIThreadCommand cmdIn;
    SUIThreadCommand cmdOut;
    cmdIn.cmdId=INSTANCE_PASS_FROM_UITHREAD_UITHREADCMD;
    App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
#endif

    if ( App::currentWorld->simulation->isSimulationStopped()&&(App::getEditModeType()==NO_EDIT_MODE) )
    {
        App::worldContainer->dispatchEvents();
        App::worldContainer->callScripts(sim_syscb_nonsimulation,nullptr,nullptr);
    }
    if (App::currentWorld->simulation->isSimulationPaused())
    {
        CScriptObject* mainScript=App::currentWorld->embeddedScriptContainer->getMainScript();
        if (mainScript!=nullptr)
        {
            App::worldContainer->dispatchEvents();
            if (mainScript->systemCallMainScript(sim_syscb_suspended,nullptr,nullptr)==0)
                App::worldContainer->callScripts(sim_syscb_suspended,nullptr,nullptr);
        }
    }

    // Handle the main loop (one pass):
    if (_workThreadLoopCallback!=nullptr)
        _workThreadLoopCallback();

    App::currentWorld->embeddedScriptContainer->removeDestroyedScripts(sim_scripttype_childscript);
    App::currentWorld->embeddedScriptContainer->removeDestroyedScripts(sim_scripttype_customizationscript);

    // Keep for backward compatibility:
    if (!App::currentWorld->simulation->isSimulationRunning()) // when simulation is running, we handle the add-on scripts after the main script was called
        App::worldContainer->addOnScriptContainer->callScripts(sim_syscb_aos_run_old,nullptr,nullptr);

    #ifdef SIM_WITH_GUI
            App::currentWorld->simulation->showAndHandleEmergencyStopButton(false,""); // 10/10/2015
    #endif
    App::simThread->executeMessages(); // rendering, queued command execution, etc.
}

bool App::executeUiThreadCommand(SUIThreadCommand* cmdIn,SUIThreadCommand* cmdOut)
{
    return(uiThread->executeCommandViaUiThread(cmdIn,cmdOut));
}

void App::appendSimulationThreadCommand(int cmdId,int intP1,int intP2,double floatP1,double floatP2,const char* stringP1,const char* stringP2,int executionDelay)
{ // convenience function. All args have default values except for the first
    SSimulationThreadCommand cmd;
    cmd.cmdId=cmdId;
    cmd.intParams.push_back(intP1);
    cmd.intParams.push_back(intP2);
    cmd.floatParams.push_back(floatP1);
    cmd.floatParams.push_back(floatP2);
    if (stringP1==nullptr)
        cmd.stringParams.push_back("");
    else
        cmd.stringParams.push_back(stringP1);
    if (stringP2==nullptr)
        cmd.stringParams.push_back("");
    else
        cmd.stringParams.push_back(stringP2);
    appendSimulationThreadCommand(cmd,executionDelay);
}

void App::appendSimulationThreadCommand(SSimulationThreadCommand cmd,int executionDelay/*=0*/)
{
    static std::vector<SSimulationThreadCommand> delayed_cmd;
    static std::vector<int> delayed_delay;
    if (simThread!=nullptr)
    {
        if (delayed_cmd.size()!=0)
        {
            for (unsigned int i=0;i<delayed_cmd.size();i++)
                simThread->appendSimulationThreadCommand(delayed_cmd[i],delayed_delay[i]);
            delayed_cmd.clear();
            delayed_delay.clear();
        }
        simThread->appendSimulationThreadCommand(cmd,executionDelay);
    }
    else
    { // can happen during the initialization phase, when the client loads a scene for instance
        delayed_cmd.push_back(cmd);
        delayed_delay.push_back(executionDelay);
    }
}

void App::setBrowserEnabled(bool e)
{
    _browserEnabled=e;
    setToolbarRefreshFlag();
#ifdef SIM_WITH_GUI
    if (mainWindow!=nullptr)
        mainWindow->setBrowserVisible(_browserEnabled);
#endif
}

bool App::getBrowserEnabled()
{
    return(_browserEnabled);
}

long long int App::getFreshUniqueId()
{
    return(_nextUniqueId++);
}

App::App(bool headless)
{
    TRACE_INTERNAL;

    uiThread=nullptr;
    _initSuccessful=false;
    _browserEnabled=true;


    userSettings=new CUserSettings();
    folders=new CFolderSystem();

#ifdef SIM_WITH_OPENGL
    QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL,true);
#endif

#ifdef SIM_WITH_QT
#ifdef SIM_WITH_GUI
    int highResDisplayDefault=userSettings->highResDisplay;
    if (highResDisplayDefault==-1)
    {
        QApplication* ta=new QApplication(_qApp_argc,_qApp_argv);
        QScreen* scr=ta->primaryScreen();
        if (scr!=nullptr)
        {
            App::logMsg(sim_verbosity_loadinfos,"primary screen physical dots per inch: %s",std::to_string(int(scr->physicalDotsPerInch()+0.5)).c_str());
            QDesktopWidget* dw=ta->desktop();
            if (dw!=nullptr)
            {
                double val=(dw->logicalDpiX()/96.0)*100.0;
                App::logMsg(sim_verbosity_loadinfos,"display scaling (guessed): %s",std::to_string(int(val+0.5)).c_str());
#ifndef MAC_SIM
                if (val>=userSettings->guessedDisplayScalingThresholdFor2xOpenGl)
                    highResDisplayDefault=2;
#endif
            }
        }
        delete ta;
    }
    if (highResDisplayDefault==1)
    {
        qputenv("QT_SCALE_FACTOR","1.0");
        App::sc=2;
    }
    if (highResDisplayDefault==2)
    {
        qputenv("QT_AUTO_SCREEN_SCALE_FACTOR","1");
        App::sc=2;
#ifdef WIN_SIM
        // To address a bug with qscintilla on hdpi display:
        if (userSettings->scriptEditorFont=="")
            userSettings->scriptEditorFont="Consolas";
#endif
    }
    if (highResDisplayDefault==3)
    {
        if (userSettings->guiScaling>1.01)
            qputenv("QT_SCALE_FACTOR",std::to_string(userSettings->guiScaling).c_str());
        if (userSettings->oglScaling!=1)
            App::sc=userSettings->oglScaling;
    }
#endif
    qtApp=new CSimQApp(_qApp_argc,_qApp_argv);

    QHostInfo::lookupHost("www.coppeliarobotics.com",
        [=] (const QHostInfo &info)
        {
            if(info.error() == QHostInfo::NoError)
                App::_online = true;
        }
    );
#endif

#ifdef USING_QOPENGLWIDGET
    // Following mandatory on some platforms (e.g. OSX), call just after a QApplication was constructed:
    QSurfaceFormat format;
    format.setRenderableType(QSurfaceFormat::OpenGL);
    QSurfaceFormat::setDefaultFormat(format);
#endif

#ifdef SIM_WITH_QT
    qRegisterMetaType<std::string>("std::string");
#endif

#ifdef SIM_WITH_QT
#ifdef SIM_WITH_GUI
    Q_INIT_RESOURCE(targaFiles);
    Q_INIT_RESOURCE(toolbarFiles);
    Q_INIT_RESOURCE(variousImageFiles);
    Q_INIT_RESOURCE(imageFiles);
    if (userSettings->darkMode)
    {
        QFile ff(":qdarkstyle/style.qss");
        if (!ff.exists())
            App::logMsg(sim_verbosity_warnings,"unable to set dark mode.");
        else
        {
            ff.open(QFile::ReadOnly | QFile::Text);
            QTextStream ts(&ff);
            qApp->setStyleSheet(ts.readAll());
        }
    }
#endif
#endif

#ifdef WIN_SIM
    #ifdef SIM_WITH_GUI
        CSimQApp::setStyle(QStyleFactory::create("Fusion")); // Probably most compatible. Other platforms: best in native (other styles have problems)!
    #endif
#endif

#ifdef SIM_WITH_GUI
    if (!headless)
    {
        if (CAuxLibVideo::loadLibrary())
            App::logMsg(sim_verbosity_loadinfos,"loaded the video compression library.");
        else
        {
            std::string msg("could not find or correctly load the video compression library.");
#ifdef LIN_SIM
            msg+="\nTry following:";
            msg+="\n";
            msg+="\n$ sudo apt-get install libavcodec-dev libavformat-dev libswscale-dev";
            msg+="\nif above fails, try first:";
            msg+="\n$ sudo apt-get -f install";
            msg+="\n";
#endif
            App::logMsg(sim_verbosity_errors,msg.c_str());
        }
    }

    QFont f=QApplication::font();
    #ifdef WIN_SIM
        if (userSettings->guiFontSize_Win!=-1)
            f.setPixelSize(userSettings->guiFontSize_Win);
    #endif
    #ifdef MAC_SIM
        if (userSettings->guiFontSize_Mac!=-1)
            f.setPixelSize(userSettings->guiFontSize_Mac);
    #endif
    #ifdef LIN_SIM
        if (userSettings->guiFontSize_Linux!=-1)
            f.setPixelSize(userSettings->guiFontSize_Linux);
    #endif
    QApplication::setFont(f);
    #ifdef LIN_SIM // make the groupbox frame visible on Linux
        qtApp->setStyleSheet("QGroupBox {  border: 1px solid lightgray;} QGroupBox::title {  background-color: transparent; subcontrol-position: top left; padding:2 13px;}");
    #endif
#endif

    uiThread=new CUiThread();
    VThread::setUiThreadId();
    srand((int)VDateTime::getTimeInMs());    // Important so that the computer ID has some "true" random component!
                                        // Remember that each thread starts with a same seed!!!
    _initSuccessful=true;
    _exitCode=0;
}

App::~App()
{
    TRACE_INTERNAL;
    VThread::unsetUiThreadId();
    delete uiThread;
    uiThread=nullptr;

    // Clear the TAG that CoppeliaSim crashed! (because if we arrived here, we didn't crash!)
    CPersistentDataContainer cont;
    cont.writeData("SIMSETTINGS_SIM_CRASHED","No",!App::userSettings->doNotWritePersistentData);

    // Remove any remaining auto-saved file:
    for (int i=1;i<30;i++)
    {
        std::string testScene(App::folders->getAutoSavedScenesPath()+"/");
        testScene+=tt::FNb(i);
        testScene+=".";
        testScene+=SIM_SCENE_EXTENSION;
        if (VFile::doesFileExist(testScene.c_str()))
            VFile::eraseFile(testScene.c_str());
    }

    delete folders;
    folders=nullptr;
    delete userSettings;
    userSettings=nullptr;

#ifdef SIM_WITH_GUI
    CAuxLibVideo::unloadLibrary();
#endif

#ifdef SIM_WITH_QT
    if (qtApp!=nullptr)
    {
        #ifdef SIM_WITH_GUI
            Q_CLEANUP_RESOURCE(imageFiles);
            Q_CLEANUP_RESOURCE(variousImageFiles);
            Q_CLEANUP_RESOURCE(toolbarFiles);
            Q_CLEANUP_RESOURCE(targaFiles);
        #endif // SIM_WITH_GUI
        qtApp->disconnect();
//        qtApp->deleteLater(); // this crashes when trying to run CoppeliaSim several times from the same client app
        delete qtApp; // this crashes with some plugins, on MacOS

        /*
            QEventLoop destroyLoop;
            QObject::connect(qtApp,&QObject::destroyed,&destroyLoop,&QEventLoop::quit);
            qtApp->deleteLater();
            destroyLoop.exec();
            // crashes here above, just after qtApp destruction
        */
        qtApp=nullptr;
    }
#endif
    _applicationArguments.clear();
    _applicationNamedParams.clear();
    _additionalAddOnScript1.clear();
    _additionalAddOnScript2.clear();
    if (_consoleMsgsFile!=nullptr)
    {
        _consoleMsgsArchive->close();
        delete _consoleMsgsArchive;
        _consoleMsgsArchive=nullptr;
        _consoleMsgsFile->close();
        delete _consoleMsgsFile;
        _consoleMsgsFile=nullptr;
    }
    _consoleMsgsToFile=false;
    _startupScriptString.clear();
    _consoleLogFilterStr.clear();
    _consoleVerbosity=sim_verbosity_default;
    _statusbarVerbosity=sim_verbosity_msgs;
    _dlgVerbosity=sim_verbosity_infos;
}

bool App::wasInitSuccessful()
{
    return(_initSuccessful);
}

void App::postExitRequest()
{ // call only from sim thread!
    // Important to remove all objects before we destroy the main window,
    // since some of them might be linked it:
    uiThread->showOrHideEmergencyStop(false,"");
    uiThread->showOrHideProgressBar(true,-1,"Leaving...");
    while (worldContainer->getWorldCount()>1)
        worldContainer->destroyCurrentWorld();
    currentWorld->clearScene(true);
    uiThread->showOrHideProgressBar(false);
    _exitRequest=true;
}

bool App::getExitRequest()
{
    return(_exitRequest);
}

bool App::isSimulatorRunning()
{
    return(_simulatorIsRunning);
}

void App::beep(int frequ,int duration)
{
#ifdef SIM_WITH_GUI
    for (int i=0;i<3;i++)
    {
        #ifdef WIN_SIM
            Beep(frequ,duration);
        #else
            if (qtApp!=nullptr)
                qtApp->beep();
        #endif
        VThread::sleep(500);
    }
#endif
}

void App::setApplicationName(const char* name)
{
    _applicationName=CSimFlavor::getStringVal(2);
}

std::string App::getApplicationName()
{
    return(_applicationName);
}

void App::createWorldsContainer()
{
    TRACE_INTERNAL;
    worldContainer=new CWorldContainer();
    worldContainer->initialize();
}

void App::deleteWorldsContainer()
{
    TRACE_INTERNAL;
    worldContainer->deinitialize();
    delete worldContainer;
    worldContainer=nullptr;
}

void App::_runInitializationCallback(void(*initCallBack)())
{
    TRACE_INTERNAL;
    if (initCallBack!=nullptr)
        initCallBack(); // this should load all plugins

    App::worldContainer->scriptCustomFuncAndVarContainer->outputWarningWithFunctionNamesWithoutPlugin(true);
}

void App::_runDeinitializationCallback(void(*deinitCallBack)())
{
    TRACE_INTERNAL;
    if (deinitCallBack!=nullptr)
        deinitCallBack(); // this will unload all plugins!!
}

void App::run(void(*initCallBack)(),void(*loopCallBack)(),void(*deinitCallBack)(),bool launchSimThread)
{ // We arrive here with a single thread: the UI thread!
    TRACE_INTERNAL;
    _exitRequest=false;
#ifdef SIM_WITH_GUI
    if (mainWindow!=nullptr)
        mainWindow->setFocus(Qt::MouseFocusReason); // needed because at first Qt behaves strangely (really??)
    uiThread->setFileDialogsNative(userSettings->fileDialogs);
#endif

    _simulatorIsRunning=true;

    // Load the plugins via callback mechanism
    // Plugins are loaded by the UI thread! (still the only thread)
    _runInitializationCallback(initCallBack);

    // Now start the main simulation thread (i.e. the "SIM thread", the one that handles a simulation):
    _workThreadLoopCallback=loopCallBack;

    if (launchSimThread)
    {
        #ifndef SIM_WITH_QT
            VThread::launchThread(_workThread,false);
        #else
            VThread::launchSimpleThread(_workThread);
        #endif
    }
    else
        _canInitSimThread=true;

    // Wait for the simulation thread to be running:
    while (simThread==nullptr)
        VThread::sleep(1);
    _canInitSimThread=false;

#ifdef SIM_WITH_GUI
    // Prepare a few initial triggers:
    {
        SSimulationThreadCommand cmd;
        cmd.cmdId=AUTO_SAVE_SCENE_CMD;
        cmd.intParams.push_back(0); // load autosaved scenes, if crashed
        App::appendSimulationThreadCommand(cmd,2000); // was 1000

        cmd.cmdId=MEMORIZE_UNDO_STATE_IF_NEEDED_CMD;
        cmd.intParams.clear();
        App::appendSimulationThreadCommand(cmd,2200); // was 200
    }

    if (CSimFlavor::getBoolVal(17))
    {
        SSimulationThreadCommand cmd;
        CSimFlavor::run(4);
        cmd.cmdId=PLUS_CVU_CMD;
        App::appendSimulationThreadCommand(cmd,1500);
        cmd.cmdId=PLUS_HVUD_CMD;
        App::appendSimulationThreadCommand(cmd,20000);
    }
    {
        SSimulationThreadCommand cmd;
        cmd.cmdId=REFRESH_DIALOGS_CMD;
        appendSimulationThreadCommand(cmd,1000);
        cmd.cmdId=DISPLAY_WARNING_IF_DEBUGGING_CMD;
        appendSimulationThreadCommand(cmd,3000);
    }

    CSimFlavor::run(7);
    {
        SSimulationThreadCommand cmd;
        cmd.cmdId=CHKLICM_CMD;
        appendSimulationThreadCommand(cmd,5000);
    }
#endif

    std::string msg=CSimFlavor::getStringVal(18);
    if (msg.size()>0)
    {
        SSimulationThreadCommand cmd;
        cmd.cmdId=EDU_EXPIRED_CMD;
        cmd.stringParams.push_back(msg);
        appendSimulationThreadCommand(cmd,3000);
    }

    // The UI thread sits here during the whole application:
    _processGuiEventsUntilQuit();

    CSimFlavor::run(8);

#ifdef SIM_WITH_GUI
    if (mainWindow!=nullptr)
        mainWindow->codeEditorContainer->closeAll();
#endif

    CSimFlavor::run(5);

    // Wait until the SIM thread ended:
    _quitLevel=2; // indicate to the SIM thread that the UI thread has left its exec
    while (_quitLevel==2)
        VThread::sleep(1);

    // Ok, we unload the plugins. This happens with the UI thread!
    _runDeinitializationCallback(deinitCallBack);

    deinitGl_ifNeeded();
    _simulatorIsRunning=false;
}

void App::_processGuiEventsUntilQuit()
{
#ifndef SIM_WITH_QT
    uiThread->processGuiEventsUntilQuit_noSignalSlots();
#else
    qtApp->exec();
#endif
}

void App::setQuitLevel(int l)
{
    _quitLevel=l;
}

int App::getQuitLevel()
{
    return(_quitLevel);
}

std::string App::getApplicationArgument(int index)
{
    if (_applicationArguments.size()==0)
    {
        for (int i=0;i<9;i++)
            _applicationArguments.push_back("");
    }
    if (index<9)
        return(_applicationArguments[index]);
    return("");
}

void App::setApplicationArgument(int index,std::string arg)
{
    if (_applicationArguments.size()==0)
    {
        for (int i=0;i<9;i++)
            _applicationArguments.push_back("");
    }
    if (index<9)
        _applicationArguments[index]=arg;
}

void App::setAdditionalAddOnScript1(const char* script)
{
    _additionalAddOnScript1=script;
}

std::string App::getAdditionalAddOnScript1()
{
    return(_additionalAddOnScript1);
}

void App::setAdditionalAddOnScript2(const char* script)
{
    _additionalAddOnScript2=script;
}

std::string App::getAdditionalAddOnScript2()
{
    return(_additionalAddOnScript2);
}

std::string App::getApplicationNamedParam(const char* paramName)
{
    std::map<std::string,std::string>::iterator it=_applicationNamedParams.find(paramName);
    if (it!=_applicationNamedParams.end())
        return(it->second);
    return("");
}

int App::setApplicationNamedParam(const char* paramName,const char* param,int paramLength)
{
    int retVal=-1;
    if (strlen(paramName)>0)
    {
        retVal=0;
        if (getApplicationNamedParam(paramName).size()==0)
            retVal=1;
        if (paramLength!=0)
            _applicationNamedParams[paramName]=std::string(param,param+paramLength);
        else
        {
            std::map<std::string,std::string>::iterator it=_applicationNamedParams.find(paramName);
            if (it!=_applicationNamedParams.end())
                _applicationNamedParams.erase(it);
        }
    }
    return(retVal);
}

int App::getEditModeType()
{ // helper
    int retVal=NO_EDIT_MODE;
#ifdef SIM_WITH_GUI
    if (mainWindow!=nullptr)
        retVal=mainWindow->editModeContainer->getEditModeType();
#endif
    return(retVal);
}

void App::setRebuildHierarchyFlag()
{ // helper
#ifdef SIM_WITH_GUI
    if (mainWindow!=nullptr)
        mainWindow->oglSurface->hierarchy->setRebuildHierarchyFlag();
#endif
}

void App::setResetHierarchyViewFlag()
{ // helper
#ifdef SIM_WITH_GUI
    if (mainWindow!=nullptr)
        mainWindow->oglSurface->hierarchy->setResetViewFlag();
#endif
}

void App::setRefreshHierarchyViewFlag()
{ // helper
#ifdef SIM_WITH_GUI
    if (mainWindow!=nullptr)
        mainWindow->oglSurface->hierarchy->setRefreshViewFlag();
#endif
}

void App::setLightDialogRefreshFlag()
{ // helper
#ifdef SIM_WITH_GUI
    if (mainWindow!=nullptr)
        mainWindow->setLightDialogRefreshFlag();
#endif
}

void App::setFullDialogRefreshFlag()
{ // helper
#ifdef SIM_WITH_GUI
    if (mainWindow!=nullptr)
        mainWindow->setFullDialogRefreshFlag();
#endif
}

void App::setDialogRefreshDontPublishFlag()
{ // helper
#ifdef SIM_WITH_GUI
    if (mainWindow!=nullptr)
        mainWindow->setDialogRefreshDontPublishFlag();
#endif
}

void App::setToolbarRefreshFlag()
{ // helper
#ifdef SIM_WITH_GUI
    if (mainWindow!=nullptr)
        mainWindow->setToolbarRefreshFlag();
#endif
}

int App::getMouseMode()
{ // helper
    int retVal=0;
#ifdef SIM_WITH_GUI
    if (mainWindow!=nullptr)
        retVal=mainWindow->getMouseMode();
#endif
    return(retVal);
}

void App::setMouseMode(int mm)
{ // helper
#ifdef SIM_WITH_GUI
    if (mainWindow!=nullptr)
        mainWindow->setMouseMode(mm);
#endif
}

void App::setDefaultMouseMode()
{ // helper
#ifdef SIM_WITH_GUI
    if (mainWindow!=nullptr)
        mainWindow->setDefaultMouseMode();
#endif
}

bool App::isFullScreen()
{ // helper
    bool retVal=false;
#ifdef SIM_WITH_GUI
    if (mainWindow!=nullptr)
        retVal=mainWindow->isFullScreen();
#endif
    return(retVal);
}

void App::setFullScreen(bool f)
{ // helper
#ifdef SIM_WITH_GUI
    if (mainWindow!=nullptr)
        mainWindow->setFullScreen(f);
#endif
}

void App::_logMsgToStatusbar(const char* msg,bool html)
{
    if (!VThread::isCurrentThreadTheUiThread())
    { // we are NOT in the UI thread. We execute the command in a delayed manner:
        SUIThreadCommand cmdIn;
        cmdIn.cmdId=LOG_MSG_TO_STATUSBAR_UITHREADCMD;
        cmdIn.stringParams.push_back(msg);
        cmdIn.boolParams.push_back(html);
        uiThread->executeCommandViaUiThread(&cmdIn,nullptr);
    }
#ifdef SIM_WITH_GUI
    else
    {
        std::string str(msg);

        if (mainWindow!=nullptr)
        {
            std::string txtCol(mainWindow->palette().windowText().color().name().toStdString());
            if ((operationalUIParts&sim_gui_statusbar)&&(mainWindow->statusBar!=nullptr) )
            {
                if (html)
//                {
//                    str+="<font color="+txtCol+">"+" </font>"; // color is otherwise not reset
                    mainWindow->statusBar->appendHtml(str.c_str());
//                }
                else
                    mainWindow->statusBar->appendPlainText(str.c_str());
                mainWindow->statusBar->moveCursor(QTextCursor::End);
                mainWindow->statusBar->verticalScrollBar()->setValue(mainWindow->statusBar->verticalScrollBar()->maximum());
                mainWindow->statusBar->ensureCursorVisible();
            }
        }
    }
#endif
}

void App::clearStatusbar()
{
    if (!VThread::isCurrentThreadTheUiThread())
    { // we are NOT in the UI thread. We execute the command in a delayed manner:
        SUIThreadCommand cmdIn;
        SUIThreadCommand cmdOut;
        cmdIn.cmdId=CLEAR_STATUSBAR_UITHREADCMD;
        uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
    }
    else
    {
    #ifdef SIM_WITH_GUI
        if (mainWindow!=nullptr)
        {
            if ((operationalUIParts&sim_gui_statusbar)&&(mainWindow->statusBar!=nullptr) )
                mainWindow->statusBar->clear();
        }
    #endif
    }
}

float* App::getRGBPointerFromItem(int objType,int objID1,int objID2,int colComponent,std::string* auxDlgTitle)
{ // auxDlgTitle can be nullptr
    std::string __auxDlgTitle;
    std::string* _auxDlgTitle=&__auxDlgTitle;
    if (auxDlgTitle!=nullptr)
        _auxDlgTitle=auxDlgTitle;

    if (objType==COLOR_ID_AMBIENT_LIGHT)
    {
        _auxDlgTitle->assign("Ambient light");
        return(currentWorld->environment->ambientLightColor);
    }
    if (objType==COLOR_ID_BACKGROUND_UP)
    {
        _auxDlgTitle->assign("Background (up)");
        return(currentWorld->environment->backGroundColor);
    }
    if (objType==COLOR_ID_BACKGROUND_DOWN)
    {
        _auxDlgTitle->assign("Background (down)");
        return(currentWorld->environment->backGroundColorDown);
    }
    if (objType==COLOR_ID_FOG)
    {
        _auxDlgTitle->assign("Fog");
        return(currentWorld->environment->fogBackgroundColor);
    }
    if (objType==COLOR_ID_MIRROR)
    {
        _auxDlgTitle->assign("Mirror");
        CMirror* it=App::currentWorld->sceneObjects->getMirrorFromHandle(objID1);
        if ((it!=nullptr)&&it->getIsMirror())
            return(it->mirrorColor);
    }
    if (objType==COLOR_ID_OCTREE)
    {
        _auxDlgTitle->assign("OC tree");
        COctree* it=App::currentWorld->sceneObjects->getOctreeFromHandle(objID1);
        if (it!=nullptr)
            return(it->getColor()->getColorsPtr());
    }
    if (objType==COLOR_ID_POINTCLOUD)
    {
        _auxDlgTitle->assign("Point cloud");
        CPointCloud* it=App::currentWorld->sceneObjects->getPointCloudFromHandle(objID1);
        if (it!=nullptr)
            return(it->getColor()->getColorsPtr());
    }
    if (objType==COLOR_ID_GRAPH_2DCURVE)
    {
        _auxDlgTitle->assign("Graph - 2D curve");
        CGraph* it=currentWorld->sceneObjects->getGraphFromHandle(objID1);
        if (it!=nullptr)
        {
            CGraphDataComb_old* grDataComb=it->getGraphData2D(objID2);
            if (grDataComb!=nullptr)
                return(grDataComb->curveColor.getColorsPtr());
        }
    }
    if (objType==COLOR_ID_GRAPH_BACKGROUND)
    {
        _auxDlgTitle->assign("Graph - background");
        CGraph* it=currentWorld->sceneObjects->getGraphFromHandle(objID1);
        if (it!=nullptr)
            return(it->backgroundColor);
    }
    if (objType==COLOR_ID_GRAPH_GRID)
    {
        _auxDlgTitle->assign("Graph - grid");
        CGraph* it=currentWorld->sceneObjects->getGraphFromHandle(objID1);
        if (it!=nullptr)
            return(it->textColor);
    }
    if (objType==COLOR_ID_GRAPH_TIMECURVE)
    {
        _auxDlgTitle->assign("Graph - data stream");
        CGraph* it=currentWorld->sceneObjects->getGraphFromHandle(objID1);
        if (it!=nullptr)
        {
            CGraphData_old* grData=it->getGraphData(objID2);
            if (grData!=nullptr)
                return(grData->ambientColor);
        }
    }
    if ((objType==COLOR_ID_OPENGLBUTTON_UP)||(objType==COLOR_ID_OPENGLBUTTON_DOWN)||(objType==COLOR_ID_OPENGLBUTTON_TEXT))
    {
        if (objType==COLOR_ID_OPENGLBUTTON_UP)
            _auxDlgTitle->assign("Button - up");
        if (objType==COLOR_ID_OPENGLBUTTON_DOWN)
            _auxDlgTitle->assign("Button - down");
        if (objType==COLOR_ID_OPENGLBUTTON_TEXT)
            _auxDlgTitle->assign("Button - text");
        CButtonBlock* block=App::currentWorld->buttonBlockContainer->getBlockWithID(objID1);
        if (block!=nullptr)
        {
            CSoftButton* itButton=block->getButtonWithID(objID2);
            if (itButton!=nullptr)
            {
                if (objType==COLOR_ID_OPENGLBUTTON_UP)
                    return(itButton->backgroundColor);
                if (objType==COLOR_ID_OPENGLBUTTON_DOWN)
                    return(itButton->downBackgroundColor);
                if (objType==COLOR_ID_OPENGLBUTTON_TEXT)
                    return(itButton->textColor);
            }
        }
    }


    int allowedParts=0;
    CColorObject* vp=getVisualParamPointerFromItem(objType,objID1,objID2,_auxDlgTitle,&allowedParts);
    if (vp!=nullptr)
    {
        if ((colComponent==sim_colorcomponent_ambient_diffuse)&&(allowedParts&1))
            return((vp->getColorsPtr()+0));
        if ((colComponent==sim_colorcomponent_diffuse)&&(allowedParts&2))
            return((vp->getColorsPtr()+3));
        if ((colComponent==sim_colorcomponent_specular)&&(allowedParts&4))
            return((vp->getColorsPtr()+6));
        if ((colComponent==sim_colorcomponent_emission)&&(allowedParts&8))
            return((vp->getColorsPtr()+9));
        if ((colComponent==sim_colorcomponent_auxiliary)&&(allowedParts&16))
            return((vp->getColorsPtr()+12));
    }

    return(nullptr);
}

CColorObject* App::getVisualParamPointerFromItem(int objType,int objID1,int objID2,std::string* auxDlgTitle,int* allowedParts)
{ // auxDlgTitle and allowedParts can be nullptr. Bit-coded: 1=ambient/diffuse, 2=diffuse(light only), 4=spec, 8=emiss., 16=aux channels, 32=pulsation, 64=shininess, 128=opacity, 256=colorName, 512=ext. string
    std::string __auxDlgTitle;
    int __allowedParts;
    std::string* _auxDlgTitle=&__auxDlgTitle;
    int* _allowedParts=&__allowedParts;
    if (auxDlgTitle!=nullptr)
        _auxDlgTitle=auxDlgTitle;
    if (allowedParts!=nullptr)
        _allowedParts=allowedParts;

    if (objType==COLOR_ID_CAMERA_A)
    {
        _auxDlgTitle->assign("Camera");
        _allowedParts[0]=1+4+8+16+32+64;
        CCamera* it=currentWorld->sceneObjects->getCameraFromHandle(objID1);
        if (it!=nullptr)
            return(it->getColor(false));
    }
    if (objType==COLOR_ID_FORCESENSOR_A)
    {
        _auxDlgTitle->assign("Force sensor");
        _allowedParts[0]=1+4+8+16+32+64;
        CForceSensor* it=currentWorld->sceneObjects->getForceSensorFromHandle(objID1);
        if (it!=nullptr)
            return(it->getColor(false));
    }
    if (objType==COLOR_ID_JOINT_A)
    {
        _auxDlgTitle->assign("Joint");
        _allowedParts[0]=1+4+8+16+32+64;
        CJoint* it=currentWorld->sceneObjects->getJointFromHandle(objID1);
        if (it!=nullptr)
            return((CColorObject*)it->getColor(false));
    }
    if (objType==COLOR_ID_PATH)
    {
        _auxDlgTitle->assign("Path");
        _allowedParts[0]=1+4+8+16+32+64;
        CPath_old* it=currentWorld->sceneObjects->getPathFromHandle(objID1);
        if ( (it!=nullptr)&&(it->pathContainer!=nullptr) )
            return(&it->pathContainer->_lineColor);
    }
    if (objType==COLOR_ID_PATH_SHAPING)
    {
        _auxDlgTitle->assign("Path shaping");
        _allowedParts[0]=1+4+8+16+32+64;
        CPath_old* it=currentWorld->sceneObjects->getPathFromHandle(objID1);
        if (it!=nullptr)
            return(it->getShapingColor());
    }
    if (objType==COLOR_ID_GRAPH_3DCURVE)
    {
        _auxDlgTitle->assign("Graph - 3D curve");
        _allowedParts[0]=1+8;
        CGraph* it=currentWorld->sceneObjects->getGraphFromHandle(objID1);
        if (it!=nullptr)
        {
            CGraphDataComb_old* grDataComb=it->getGraphData3D(objID2);
            if (grDataComb!=nullptr)
                return(&grDataComb->curveColor);
        }
    }
    if (objType==COLOR_ID_COLLISION)
    {
        _auxDlgTitle->assign("Collision");
        _allowedParts[0]=1+4+8+16+32+64;
        return(&App::currentWorld->mainSettings->collisionColor);
    }
    if (objType==COLOR_ID_COLLISIONCONTOUR)
    {
        _auxDlgTitle->assign("Collision contour");
        _allowedParts[0]=1+4+8+16+32+64;
        CCollisionObject_old* it=App::currentWorld->collisions->getObjectFromHandle(objID1);
        if (it!=nullptr)
            return(it->getContourColor());
    }
    if (objType==COLOR_ID_DISTANCESEGMENT)
    {
        _auxDlgTitle->assign("Distance segment");
        _allowedParts[0]=1+4+8+16+32+64;
        CDistanceObject_old* it=App::currentWorld->distances->getObjectFromHandle(objID1);
        if (it!=nullptr)
            return(it->getSegmentColor());
    }
    if (objType==COLOR_ID_CLIPPINGPLANE)
    {
        _auxDlgTitle->assign("Clipping plane");
        _allowedParts[0]=1+4+8+16+32+64+128;
        CMirror* it=App::currentWorld->sceneObjects->getMirrorFromHandle(objID1);
        if ((it!=nullptr)&&(!it->getIsMirror()))
            return(it->getClipPlaneColor());
    }
    if (objType==COLOR_ID_LIGHT_CASING)
    {
        _auxDlgTitle->assign("Light - casing");
        _allowedParts[0]=1+4+8+16+64;
        CLight* it=currentWorld->sceneObjects->getLightFromHandle(objID1);
        if (it!=nullptr)
            return(it->getColor(false));
    }
    if (objType==COLOR_ID_LIGHT_LIGHT)
    {
        _auxDlgTitle->assign("Light");
        _allowedParts[0]=2+4;
        CLight* it=currentWorld->sceneObjects->getLightFromHandle(objID1);
        if (it!=nullptr)
            return(it->getColor(true));
    }
    if (objType==COLOR_ID_DUMMY)
    {
        _auxDlgTitle->assign("Dummy");
        _allowedParts[0]=1+4+8+16+32+64;
        CDummy* it=currentWorld->sceneObjects->getDummyFromHandle(objID1);
        if (it!=nullptr)
            return((CColorObject*)it->getDummyColor());
    }
    if (objType==COLOR_ID_VISIONSENSOR)
    {
        _auxDlgTitle->assign("Vision sensor");
        _allowedParts[0]=1+4+8+16+32;
        CVisionSensor* it=currentWorld->sceneObjects->getVisionSensorFromHandle(objID1);
        if (it!=nullptr)
            return(it->getColor());
    }
    if (objType==COLOR_ID_PROXSENSOR_VOLUME)
    {
        _auxDlgTitle->assign("Proximity sensor");
        _allowedParts[0]=1+4+8+16+32;
        CProxSensor* it=currentWorld->sceneObjects->getProximitySensorFromHandle(objID1);
        if (it!=nullptr)
            return(it->getColor(0));
    }
    if (objType==COLOR_ID_PROXSENSOR_RAY)
    {
        _auxDlgTitle->assign("Proximity sensor - ray");
        _allowedParts[0]=1+4+8+16+32;
        CProxSensor* it=currentWorld->sceneObjects->getProximitySensorFromHandle(objID1);
        if (it!=nullptr)
            return(it->getColor(1));
    }
    if (objType==COLOR_ID_MILL_PASSIVE)
    {
        _auxDlgTitle->assign("Mill - passive");
        _allowedParts[0]=1+4+8+16+32;
        CMill* it=currentWorld->sceneObjects->getMillFromHandle(objID1);
        if (it!=nullptr)
            return(it->getColor(false));
    }
    if (objType==COLOR_ID_MILL_ACTIVE)
    {
        _auxDlgTitle->assign("Mill - active");
        _allowedParts[0]=1+4+8+16+32;
        CMill* it=currentWorld->sceneObjects->getMillFromHandle(objID1);
        if (it!=nullptr)
            return(it->getColor(true));
    }
    if (objType==COLOR_ID_SHAPE)
    {
        _auxDlgTitle->assign("Shape");
        _allowedParts[0]=1+4+8+16+32+64+128+256+512;
        CShape* it=currentWorld->sceneObjects->getShapeFromHandle(objID1);
        if ((it!=nullptr)&&(!it->isCompound()))
            return(&it->getSingleMesh()->color);
    }
    if (objType==COLOR_ID_SHAPE_GEOMETRY)
    {
#ifdef SIM_WITH_GUI
        if ((App::mainWindow->editModeContainer->getEditModeObjectID()==objID1)&&(App::mainWindow->editModeContainer->getMultishapeEditMode()->getMultishapeGeometricComponentIndex()==objID2))
        {
            _auxDlgTitle->assign("Shape component");
            _allowedParts[0]=1+4+8+16+32+64+128+256+512;
            CShape* it=currentWorld->sceneObjects->getShapeFromHandle(objID1);
            if ((it!=nullptr)&&it->isCompound())
            {
                std::vector<CMesh*> allGeometrics;
                it->getMeshWrapper()->getAllShapeComponentsCumulative(allGeometrics);
                if ((objID2>=0)&&(objID2<int(allGeometrics.size())))
                    return(&allGeometrics[objID2]->color);
            }
        }
#endif
    }

    _allowedParts[0]=0;
    return(nullptr);
}

CTextureProperty* App::getTexturePropertyPointerFromItem(int objType,int objID1,int objID2,std::string* auxDlgTitle,bool* is3D,bool* valid,CMesh** geom)
{ // auxDlgTitle, is3D, isValid and geom can be nullptr.
    std::string __auxDlgTitle;
    bool __is3D=false;
    bool __isValid=false;
    CMesh* __geom=nullptr;
    std::string* _auxDlgTitle=&__auxDlgTitle;
    bool* _is3D=&__is3D;
    bool* _isValid=&__isValid;
    CMesh** _geom=&__geom;
    if (auxDlgTitle!=nullptr)
        _auxDlgTitle=auxDlgTitle;
    if (is3D!=nullptr)
        _is3D=is3D;
    if (valid!=nullptr)
        _isValid=valid;
    if (geom!=nullptr)
        _geom=geom;
    _isValid[0]=false;
    _geom[0]=nullptr;
    if (objType==TEXTURE_ID_SIMPLE_SHAPE)
    {
        _auxDlgTitle->assign("Shape");
        _is3D[0]=true;
        CShape* it=currentWorld->sceneObjects->getShapeFromHandle(objID1);
        if ( (it!=nullptr)&&(!it->isCompound()) )
        {
            _isValid[0]=true;
            _geom[0]=it->getSingleMesh();
            return(_geom[0]->getTextureProperty());
        }
    }
    if (objType==TEXTURE_ID_COMPOUND_SHAPE)
    {
        _auxDlgTitle->assign("Shape component");
        _is3D[0]=true;
        CShape* it=currentWorld->sceneObjects->getShapeFromHandle(objID1);
        if (it!=nullptr)
        {
            std::vector<CMesh*> allGeometrics;
            it->getMeshWrapper()->getAllShapeComponentsCumulative(allGeometrics);
            if ((objID2>=0)&&(objID2<int(allGeometrics.size())))
            {
                _isValid[0]=true;
                _geom[0]=allGeometrics[objID2];
                return(_geom[0]->getTextureProperty());
            }
        }
    }
    if (objType==TEXTURE_ID_OPENGL_GUI_BACKGROUND)
    {
        _auxDlgTitle->assign("OpenGl custom UI background");
        _is3D[0]=false;
        CButtonBlock* it=currentWorld->buttonBlockContainer->getBlockWithID(objID1);
        if (it!=nullptr)
        {
            _isValid[0]=true;
            return(it->getTextureProperty());
        }
    }
    if (objType==TEXTURE_ID_OPENGL_GUI_BUTTON)
    {
        _auxDlgTitle->assign("OpenGl custom UI button");
        _is3D[0]=false;
        CButtonBlock* it=currentWorld->buttonBlockContainer->getBlockWithID(objID1);
        if (it!=nullptr)
        {
            CSoftButton* butt=it->getButtonWithID(objID2);
            if (butt!=nullptr)
            {
                _isValid[0]=true;
                return(butt->getTextureProperty());
            }
        }
    }

    return(nullptr);
}

std::string App::getConsoleLogFilter()
{
    return(_consoleLogFilterStr);
}

void App::setConsoleLogFilter(const char* filter)
{
    _consoleLogFilterStr=filter;
}

bool App::logPluginMsg(const char* pluginName,int verbosityLevel,const char* logMsg)
{
    bool retVal=false;

    CPlugin* it=CPluginContainer::getPluginFromName(pluginName,true);
    if ( (it!=nullptr)||(strcmp(pluginName,"CoppeliaSimClient")==0) )
    {
        int realVerbosityLevel=verbosityLevel&0x0fff;
        if (it!=nullptr)
        {
            int consoleV=it->getConsoleVerbosity();
            if (consoleV==sim_verbosity_useglobal)
                consoleV=_consoleVerbosity;
            int statusbarV=it->getStatusbarVerbosity();
            if (statusbarV==sim_verbosity_useglobal)
                statusbarV=_statusbarVerbosity;
            if ( (consoleV>=realVerbosityLevel)||(statusbarV>=realVerbosityLevel) )
            {
                std::string plugN("simExt");
                plugN+=pluginName;
                __logMsg(plugN.c_str(),verbosityLevel,logMsg,consoleV,statusbarV);
            }
        }
        else
            __logMsg(pluginName,verbosityLevel,logMsg);
        retVal=true;
    }
    return(retVal);
}

void App::logMsg(int verbosityLevel,const char* msg,int int1,int int2/*=0*/,int int3/*=0*/)
{
    int realVerbosityLevel=verbosityLevel&0x0fff;
    if ( (_consoleVerbosity>=realVerbosityLevel)||(_statusbarVerbosity>=realVerbosityLevel) )
        _logMsg(nullptr,verbosityLevel,msg,int1,int2,int3);
}

void App::logScriptMsg(const char* scriptName,int verbosityLevel,const char* msg)
{
    int realVerbosityLevel=verbosityLevel&0x0fff;
    if ( (_consoleVerbosity>=realVerbosityLevel)||(_statusbarVerbosity>=realVerbosityLevel) )
        __logMsg(scriptName,verbosityLevel,msg);
}

int App::getVerbosityLevelFromString(const char* verbosityStr)
{
    int retVal=-1;
    if (strcmp(verbosityStr,"none")==0)
        retVal=sim_verbosity_none;
    if (strcmp(verbosityStr,"errors")==0)
        retVal=sim_verbosity_errors;
    if (strcmp(verbosityStr,"warnings")==0)
        retVal=sim_verbosity_warnings;
    if (strcmp(verbosityStr,"loadinfos")==0)
        retVal=sim_verbosity_loadinfos;
    if (strcmp(verbosityStr,"questions")==0)
        retVal=sim_verbosity_questions;
    if (strcmp(verbosityStr,"scripterrors")==0)
        retVal=sim_verbosity_scripterrors;
    if (strcmp(verbosityStr,"scriptwarnings")==0)
        retVal=sim_verbosity_scriptwarnings;
    if (strcmp(verbosityStr,"scriptinfos")==0)
        retVal=sim_verbosity_msgs;
    if (strcmp(verbosityStr,"infos")==0)
        retVal=sim_verbosity_infos;
    if (strcmp(verbosityStr,"debug")==0)
        retVal=sim_verbosity_debug;
    if (strcmp(verbosityStr,"trace")==0)
        retVal=sim_verbosity_trace;
    if (strcmp(verbosityStr,"tracelua")==0)
        retVal=sim_verbosity_tracelua;
    if (strcmp(verbosityStr,"traceall")==0)
        retVal=sim_verbosity_traceall;
    return(retVal);
}

bool App::getConsoleMsgToFile()
{
    return(_consoleMsgsToFile);
}

void App::setConsoleMsgToFile(bool f)
{
    _consoleMsgsToFile=f;
}

bool App::isCurrentThreadTheUiThread()
{
    return(VThread::isCurrentThreadTheUiThread());
}

void App::logMsg(int verbosityLevel,const char* msg,const char* subStr1,const char* subStr2/*=nullptr*/,const char* subStr3/*=nullptr*/)
{
    int realVerbosityLevel=verbosityLevel&0x0fff;
    if ( (_consoleVerbosity>=realVerbosityLevel)||(_statusbarVerbosity>=realVerbosityLevel) )
        _logMsg(nullptr,verbosityLevel,msg,subStr1,subStr2,subStr3);
}

void App::logMsg(int verbosityLevel,const char* msg)
{
    int realVerbosityLevel=verbosityLevel&0x0fff;
    if ( (_consoleVerbosity>=realVerbosityLevel)||(_statusbarVerbosity>=realVerbosityLevel) )
        __logMsg(nullptr,verbosityLevel,msg);
}

void App::_logMsg(const char* originName,int verbosityLevel,const char* msg,const char* subStr1,const char* subStr2/*=nullptr*/,const char* subStr3/*=nullptr*/)
{
    size_t bs=strlen(msg)+200;
    char* buff=new char[bs];
    if (subStr2!=nullptr)
    {
        if (subStr3!=nullptr)
            snprintf(buff,bs,msg,subStr1,subStr2,subStr3);
        else
            snprintf(buff,bs,msg,subStr1,subStr2);
    }
    else
        snprintf(buff,bs,msg,subStr1);
    __logMsg(originName,verbosityLevel,buff);
    delete[] buff;
}

void App::_logMsg(const char* originName,int verbosityLevel,const char* msg,int int1,int int2/*=0*/,int int3/*=0*/)
{
    size_t bs=strlen(msg)+200;
    char* buff=new char[bs];
    snprintf(buff,bs,msg,int1,int2,int3);
    __logMsg(originName,verbosityLevel,buff);
    delete[] buff;
}

std::string App::_getHtmlEscapedString(const char* str)
{
    std::string s(str);
    CTTUtil::replaceSubstring(s,"<","*+-%A%-+*");
    CTTUtil::replaceSubstring(s,">","*+-%B%-+*");
    CTTUtil::replaceSubstring(s,"\n","*+-%NL%-+*");
    CTTUtil::replaceSubstring(s," ","*+-%S%-+*");
    CTTUtil::replaceSubstring(s,"\t","*+-%T%-+*");
    CTTUtil::replaceSubstring(s,"/","*+-%FS%-+*");
#ifdef SIM_WITH_QT
    QString qstr(s.c_str());
    qstr=qstr.toHtmlEscaped();
    s=qstr.toStdString();
#endif
    CTTUtil::replaceSubstring(s,"*+-%NL%-+*","<br/>");
    CTTUtil::replaceSubstring(s,"*+-%S%-+*","&nbsp;");
    CTTUtil::replaceSubstring(s,"*+-%T%-+*","&nbsp;&nbsp;&nbsp;&nbsp;");
    CTTUtil::replaceSubstring(s,"*+-%FS%-+*","&#47;");
    CTTUtil::replaceSubstring(s,"*+-%A%-+*","&lt;");
    CTTUtil::replaceSubstring(s,"*+-%B%-+*","&gt;");
    return(s);
}

bool App::_consoleLogFilter(const char* msg)
{
    bool triggered=true;
    if (_consoleLogFilterStr.size()>0)
    {
        std::string theMsg(msg);
        std::istringstream isso(_consoleLogFilterStr);
        std::string orBlock;
        while (std::getline(isso,orBlock,'|'))
        {
            std::istringstream issa(orBlock);
            std::string andWord;
            triggered=true;
            while (std::getline(issa,andWord,'&'))
            {
                if (theMsg.find(andWord)==std::string::npos)
                {
                    triggered=false;
                    break;
                }
            }
            if (triggered)
                break;
        }
    }
    return(!triggered);
}

static std::string replaceVars(const std::string &format, const std::map<std::string,std::string> &vars)
{
    std::string msg;
    size_t last=0;
    while (last<format.length())
    {
        size_t posOpen=format.find("{",last);
        size_t posClose=format.find("}",posOpen);
        if (posOpen!=std::string::npos&&posClose!=std::string::npos)
        {
            msg+=format.substr(last,posOpen-last);
            auto key=format.substr(posOpen+1,posClose-posOpen-1);
            auto it=vars.find(key);
            if (it!=vars.end()) msg+=it->second;
            last=posClose+1;
        }
        else break;
    }
    if(last<format.length())
        msg+=format.substr(last,std::string::npos);
    return msg;
}

void App::__logMsg(const char* originName,int verbosityLevel,const char* msg,int consoleVerbosity/*=-1*/,int statusbarVerbosity/*=-1*/)
{
    static bool inside=false;
    static int64_t lastTime=0;
    if (!inside)
    {
        int realVerbosityLevel=verbosityLevel&0x0fff;
        inside=true;

        bool decorateMsg=((verbosityLevel&sim_verbosity_undecorated)==0)&&((App::userSettings==nullptr)||(!App::userSettings->undecoratedStatusbarMessages));
        static std::string consoleLogFormat,statusbarLogFormat,statusbarLogFormatUndecorated;
        if (consoleLogFormat.empty())
        {
            auto f=std::getenv("COPPELIASIM_CONSOLE_LOG_FORMAT");
            consoleLogFormat=f?f:"[{origin}:{verbosity}]   {message}";
        }
        if (statusbarLogFormat.empty())
        {
            auto f=std::getenv("COPPELIASIM_STATUSBAR_LOG_FORMAT");
            statusbarLogFormat=f?f:"<font color='grey'>[{origin}:{verbosity}]</font>    <font color='{color}'>{message}</font>";
        }
        if (statusbarLogFormatUndecorated.empty())
        {
            auto f=std::getenv("COPPELIASIM_STATUSBAR_LOG_FORMAT_UNDECORATED");
            statusbarLogFormatUndecorated=f?f:"<font color='{color}'>{message}</font>";
        }
        std::map<std::string,std::string> vars;
        vars["message"]=msg;
        vars["origin"]=originName?originName:"CoppeliaSim";
        vars["verbosity"]="unknown";
#ifdef SIM_WITH_GUI
        if (qtApp!=nullptr)
        {
            QColor col=qtApp->style()->standardPalette().windowText().color();
            int rgb[3];
            col.getRgb(rgb+0,rgb+1,rgb+2);
            int incr=56;
            if (rgb[0]>128)
                incr=-incr;
            for (int i=1;i<3;i++)
                rgb[i]+=incr;
            col.setRgb(rgb[0],rgb[1],rgb[2]);
            vars["color"]=col.name().toStdString();
        }
        else
#endif
            vars["color"]="#383838";
        int64_t t=std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        std::stringstream ss; ss<<std::fixed<<std::setprecision(3)<<0.001*t;
        vars["time"]=ss.str();
        std::stringstream ss1; ss1<<std::fixed<<std::setprecision(3)<<0.001*(t-lastTime);
        vars["delta"]=ss1.str();
        lastTime=t;

        if ( (realVerbosityLevel==sim_verbosity_errors)||(realVerbosityLevel==sim_verbosity_scripterrors) )
        {   vars["verbosity"]="error"; vars["color"]="red"; }
        if ( (realVerbosityLevel==sim_verbosity_warnings)||(realVerbosityLevel==sim_verbosity_scriptwarnings) )
        {   vars["verbosity"]="warning"; vars["color"]="#D35400"; }
        if (realVerbosityLevel==sim_verbosity_loadinfos)
            vars["verbosity"]="loadinfo";
        if ( (realVerbosityLevel==sim_verbosity_infos)||(realVerbosityLevel==sim_verbosity_scriptinfos) ) // also sim_verbosity_msgs, which is same as sim_verbosity_scriptinfos
            vars["verbosity"]="info";
        if (realVerbosityLevel==sim_verbosity_debug)
            vars["verbosity"]="debug";
        if (realVerbosityLevel==sim_verbosity_trace)
            vars["verbosity"]="trace";
        if (realVerbosityLevel==sim_verbosity_tracelua)
            vars["verbosity"]="tracelua";
        if (realVerbosityLevel==sim_verbosity_traceall)
            vars["verbosity"]="traceall";

        {
            std::string message(msg);
            // For backward compatibility with messages that already have HTML tags:
            size_t p=message.rfind("@html");
            if ( (p!=std::string::npos)&&(p==message.size()-5) )
            { // strip HTML stuff off
                message.assign(message.c_str(),message.c_str()+message.size()-5);
#ifdef SIM_WITH_QT
                QTextDocument doc;
                doc.setHtml(message.c_str());
                message=doc.toPlainText().toStdString();
#else
    // TODO_SIM_WITH_QT
#endif
            }
            vars["message"]=message;
        }

     //   boost::replace_all(vars["message"],"\n","\n    ");

        std::string consoleTxt(replaceVars(consoleLogFormat,vars)+"\n");
        if (!_consoleLogFilter(consoleTxt.c_str()))
        {
            if (consoleVerbosity==-1)
                consoleVerbosity=_consoleVerbosity;
            if (consoleVerbosity>=realVerbosityLevel)
            {
                printf("%s",consoleTxt.c_str());
                if (_consoleMsgsToFile)
                {
                    if (_consoleMsgsFile==nullptr)
                    {
                        _consoleMsgsFile=new VFile("debugLog.txt",VFile::CREATE_WRITE|VFile::SHARE_EXCLUSIVE);
                        _consoleMsgsArchive=new VArchive(_consoleMsgsFile,VArchive::STORE);
                    }
                    for (size_t i=0;i<consoleTxt.size();i++)
                        (*_consoleMsgsArchive) << consoleTxt[i];
                    (*_consoleMsgsArchive) << ((unsigned char)13) << ((unsigned char)10);
                    _consoleMsgsFile->flush();
                }
            }
        }
        if (statusbarVerbosity==-1)
            statusbarVerbosity=_statusbarVerbosity;
        if ( (statusbarVerbosity>=realVerbosityLevel)&&(uiThread!=nullptr)&&(simThread!=nullptr)&&((verbosityLevel&sim_verbosity_onlyterminal)==0) )
        {
            vars["message"]=_getHtmlEscapedString(vars["message"].c_str());
            std::string statusbarTxt=replaceVars(decorateMsg?statusbarLogFormat:statusbarLogFormatUndecorated,vars);
            _logMsgToStatusbar(statusbarTxt.c_str(),true);
        }
        inside=false;
    }
}

int App::getDlgVerbosity()
{ // sim_verbosity_none, etc.
    return(_dlgVerbosity);
}

void App::setDlgVerbosity(int v)
{ // sim_verbosity_none, etc.
    _dlgVerbosity=v;
}

void App::setStartupScriptString(const char* str)
{
    _startupScriptString=str;
}

void App::setExitCode(int c)
{
    _exitCode=c;
}

int App::getExitCode()
{
    return(_exitCode);
}

bool App::isOnline()
{
    return(_online);
}

void App::undoRedo_sceneChanged(const char* txt)
{
    if (VThread::isCurrentThreadTheUiThread())
    {
        SSimulationThreadCommand cmd;
        cmd.cmdId=999999;
        cmd.stringParams.push_back(txt);
        appendSimulationThreadCommand(cmd);
    }
    else
        currentWorld->undoBufferContainer->announceChange();
}

void App::undoRedo_sceneChangedGradual(const char* txt)
{
    if (VThread::isCurrentThreadTheUiThread())
    {
        SSimulationThreadCommand cmd;
        cmd.cmdId=999996;
        cmd.stringParams.push_back(txt);
        appendSimulationThreadCommand(cmd);
    }
    else
        currentWorld->undoBufferContainer->announceChangeGradual();
}

void App::undoRedo_sceneChangeStart(const char* txt)
{
    if  (VThread::isCurrentThreadTheUiThread())
    {
        SSimulationThreadCommand cmd;
        cmd.cmdId=999997;
        cmd.stringParams.push_back(txt);
        appendSimulationThreadCommand(cmd);
    }
    else
        currentWorld->undoBufferContainer->announceChangeStart();
}

void App::undoRedo_sceneChangeEnd()
{
    if  (VThread::isCurrentThreadTheUiThread())
    {
        SSimulationThreadCommand cmd;
        cmd.cmdId=999998;
        cmd.stringParams.push_back("");
        appendSimulationThreadCommand(cmd);
    }
    else
        currentWorld->undoBufferContainer->announceChangeEnd();
}

int App::getConsoleVerbosity(const char* pluginName/*=nullptr*/)
{ // sim_verbosity_none, etc.
    int retVal=_consoleVerbosity;
    if (pluginName!=nullptr)
    {
        CPlugin* pl=CPluginContainer::getPluginFromName(pluginName,true);
        if (pl!=nullptr)
        {
            if (pl->getConsoleVerbosity()!=sim_verbosity_useglobal)
                retVal=pl->getConsoleVerbosity();
        }
    }
    return(retVal);
}

void App::setConsoleVerbosity(int v,const char* pluginName/*=nullptr*/)
{ // sim_verbosity_none, etc.
    if (pluginName!=nullptr)
    {
        CPlugin* pl=CPluginContainer::getPluginFromName(pluginName,true);
        if (pl!=nullptr)
            pl->setConsoleVerbosity(v);
    }
    else
        _consoleVerbosity=v;
}

int App::getStatusbarVerbosity(const char* pluginName/*=nullptr*/)
{ // sim_verbosity_none, etc.
    int retVal=_statusbarVerbosity;
    if (pluginName!=nullptr)
    {
        CPlugin* pl=CPluginContainer::getPluginFromName(pluginName,true);
        if (pl!=nullptr)
        {
            if (pl->getStatusbarVerbosity()!=sim_verbosity_useglobal)
                retVal=pl->getStatusbarVerbosity();
        }
    }
    return(retVal);
}

void App::setStatusbarVerbosity(int v,const char* pluginName/*=nullptr*/)
{ // sim_verbosity_none, etc.
    if (pluginName!=nullptr)
    {
        CPlugin* pl=CPluginContainer::getPluginFromName(pluginName,true);
        if (pl!=nullptr)
            pl->setStatusbarVerbosity(v);
    }
    else
        _statusbarVerbosity=v;
}

bool App::getConsoleOrStatusbarVerbosityTriggered(int verbosityLevel)
{
    return( (_consoleVerbosity>=verbosityLevel)||(_statusbarVerbosity>=verbosityLevel) );
}

#ifdef SIM_WITH_GUI
void App::showSplashScreen()
{
    App::setShowConsole(false);
    QPixmap pixmap;

    pixmap.load(CSimFlavor::getStringVal(1).c_str());

    QSplashScreen splash(pixmap,Qt::WindowStaysOnTopHint);
    splash.setMask(pixmap.mask());
    QString txt("Version ");
    txt+=SIM_PROGRAM_VERSION;
    txt+=" ";
    txt+=SIM_PROGRAM_REVISION;
    txt+=", Built ";
    txt+=__DATE__;
    splash.showMessage(txt,Qt::AlignLeft|Qt::AlignBottom);
    splash.show();
    int ct=(int)VDateTime::getTimeInMs();
    while (VDateTime::getTimeDiffInMs(ct)<2000)
    {
        splash.raise();
        App::qtApp->processEvents();
        VThread::sleep(1);
    }
    splash.hide();
    App::setShowConsole(true);
}

void App::setIcon()
{
    App::qtApp->setWindowIcon(QIcon(CSimFlavor::getStringVal(4).c_str()));
}

void App::createMainWindow()
{
    TRACE_INTERNAL;
    mainWindow=new CMainWindow();
    mainWindow->initializeWindow();
    setShowConsole(userSettings->alwaysShowConsole);
}

void App::deleteMainWindow()
{
    TRACE_INTERNAL;
    delete mainWindow;
    mainWindow=nullptr;
}

void App::setShowConsole(bool s)
{
    #ifdef WIN_SIM
        if (s)
            ShowWindow(GetConsoleWindow(),SW_SHOW);
        else
            ShowWindow(GetConsoleWindow(),SW_HIDE);
    #endif
}
#endif
