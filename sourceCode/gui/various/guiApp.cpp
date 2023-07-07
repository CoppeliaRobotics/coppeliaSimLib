#include <guiApp.h>
#include <app.h>
#include <utils.h>
#include <tt.h>
#include <simFlavor.h>
#include <QHostInfo>
#include <vDateTime.h>
#include <rendering.h>
#ifdef SIM_WITH_GUI
    #include <auxLibVideo.h>
    #include <vMessageBox.h>
    #include <QSplashScreen>
    #include <QBitmap>
    #include <QTextStream>
    #include <QScreen>
    #include <QDesktopWidget>
    #ifdef WIN_SIM
        #include <QStyleFactory>
    #endif
#endif

CSimQApp* GuiApp::qtApp=nullptr;
int GuiApp::_qApp_argc=1;
char GuiApp::_qApp_arg0[]={"CoppeliaSim"};
char* GuiApp::_qApp_argv[1]={_qApp_arg0};
CUiThread* GuiApp::uiThread=nullptr;
int GuiApp::operationalUIParts=0; // sim_gui_menubar,sim_gui_popupmenus,sim_gui_toolbar1,sim_gui_toolbar2, etc.
bool GuiApp::_browserEnabled=true;
bool GuiApp::_online=false;
bool GuiApp::_showInertias=false;
CGm* GuiApp::gm=nullptr;
int GuiApp::sc=1;
#ifdef SIM_WITH_GUI
    CMainWindow* GuiApp::mainWindow=nullptr;
#endif

GuiApp::GuiApp()
{
}

GuiApp::~GuiApp()
{
}

void GuiApp::cleanupGui()
{
    TRACE_INTERNAL;

#ifdef SIM_WITH_GUI
    deleteMainWindow();
#endif
    delete gm;

    VThread::unsetUiThread();
    delete uiThread;
    uiThread=nullptr;

    // Clear the TAG that CoppeliaSim crashed! (because if we arrived here, we didn't crash!)
    CPersistentDataContainer cont;
    cont.writeData("SIMSETTINGS_SIM_CRASHED","No",!App::userSettings->doNotWritePersistentData);

    // Remove any remaining auto-saved file:
    for (int i=1;i<30;i++)
    {
        std::string testScene(App::folders->getAutoSavedScenesPath()+"/");
        testScene+=utils::getIntString(false,i);
        testScene+=".";
        testScene+=SIM_SCENE_EXTENSION;
        if (VFile::doesFileExist(testScene.c_str()))
            VFile::eraseFile(testScene.c_str());
    }

#ifdef SIM_WITH_GUI
    CAuxLibVideo::unloadLibrary();
#endif

    if (qtApp!=nullptr)
    {
        #ifdef SIM_WITH_GUI
            Q_CLEANUP_RESOURCE(imageFiles);
            Q_CLEANUP_RESOURCE(variousImageFiles);
            Q_CLEANUP_RESOURCE(toolbarFiles);
            Q_CLEANUP_RESOURCE(targaFiles);
        #endif // SIM_WITH_GUI
        qtApp->disconnect();
        delete qtApp;
        qtApp=nullptr;
    }
    App::setAppStage(App::appstage_guiCleanupDone);
}

void GuiApp::initGui(int options)
{
    TRACE_INTERNAL;
    while (App::getAppStage()!=App::appstage_simInitDone) // wait until SIM thread finished first phase of initialization
        VThread::sleep(1);

    uiThread=nullptr;
    _browserEnabled=true;
    gm=new CGm();

    CSimFlavor::run(0);

    for (int i=0;i<9;i++)
    {
        std::string str(App::getApplicationArgument(i));
        if ( (str.compare(0,9,"GUIITEMS_")==0)&&(str.length()>9) )
        {
            str.erase(str.begin(),str.begin()+9);
            int val=0;
            if (tt::stringToInt(str.c_str(),val))
            {
                options=val;
                break;
            }
        }
    }

    operationalUIParts=options;
    if (operationalUIParts&sim_gui_headless)
        operationalUIParts=sim_gui_headless;

#ifdef SIM_WITH_GUI
    QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL,true);
    int highResDisplayDefault=App::userSettings->highResDisplay;
    if (highResDisplayDefault==-1)
    {
        QApplication* ta=new QApplication(_qApp_argc,_qApp_argv);
        QScreen* scr=ta->primaryScreen();
        if (scr!=nullptr)
        {
            App::logMsg(sim_verbosity_loadinfos|sim_verbosity_onlyterminal,"primary screen physical dots per inch: %s",std::to_string(int(scr->physicalDotsPerInch()+0.5)).c_str());
            QDesktopWidget* dw=ta->desktop();
            if (dw!=nullptr)
            {
                double val=(dw->logicalDpiX()/96.0)*100.0;
                App::logMsg(sim_verbosity_loadinfos|sim_verbosity_onlyterminal,"display scaling (guessed): %s",std::to_string(int(val+0.5)).c_str());
#ifndef MAC_SIM
                if (val>=App::userSettings->guessedDisplayScalingThresholdFor2xOpenGl)
                    highResDisplayDefault=2;
#endif
            }
        }
        delete ta;
    }
    if (highResDisplayDefault==1)
    {
        qputenv("QT_SCALE_FACTOR","1.0");
        GuiApp::sc=2;
    }

    if (highResDisplayDefault==2)
    {
        qputenv("QT_AUTO_SCREEN_SCALE_FACTOR","1");
        GuiApp::sc=2;
#ifdef WIN_SIM
        // To address a bug with qscintilla on hdpi display:
        if (App::userSettings->scriptEditorFont=="")
            App::userSettings->scriptEditorFont="Consolas";
#endif
    }
    if (highResDisplayDefault==3)
    {
        if (App::userSettings->guiScaling>1.01)
            qputenv("QT_SCALE_FACTOR",std::to_string(App::userSettings->guiScaling).c_str());
        if (App::userSettings->oglScaling!=1)
            GuiApp::sc=App::userSettings->oglScaling;
    }
#endif
    qtApp=new CSimQApp(_qApp_argc,_qApp_argv);

    QHostInfo::lookupHost("www.coppeliarobotics.com",
        [=] (const QHostInfo &info)
        {
            if(info.error() == QHostInfo::NoError)
                _online = true;
        }
    );

#ifdef USING_QOPENGLWIDGET
    // Following mandatory on some platforms (e.g. OSX), call just after a QApplication was constructed:
    QSurfaceFormat format;
    format.setRenderableType(QSurfaceFormat::OpenGL);
    QSurfaceFormat::setDefaultFormat(format);
#endif

    qRegisterMetaType<std::string>("std::string");
#ifdef SIM_WITH_GUI
    Q_INIT_RESOURCE(targaFiles);
    Q_INIT_RESOURCE(toolbarFiles);
    Q_INIT_RESOURCE(variousImageFiles);
    Q_INIT_RESOURCE(imageFiles);
    if (App::userSettings->darkMode)
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

#ifdef WIN_SIM
    #ifdef SIM_WITH_GUI
        CSimQApp::setStyle(QStyleFactory::create("Fusion")); // Probably most compatible. Other platforms: best in native (other styles have problems)!
    #endif
#endif

#ifdef SIM_WITH_GUI
    if ( (options&sim_gui_headless)==0 )
    {
        if (CAuxLibVideo::loadLibrary())
            App::logMsg(sim_verbosity_loadinfos|sim_verbosity_onlyterminal,"loaded the video compression library.");
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
        if (App::userSettings->guiFontSize_Win!=-1)
            f.setPixelSize(App::userSettings->guiFontSize_Win);
    #endif
    #ifdef MAC_SIM
        if (App::userSettings->guiFontSize_Mac!=-1)
            f.setPixelSize(App::userSettings->guiFontSize_Mac);
    #endif
    #ifdef LIN_SIM
        if (App::userSettings->guiFontSize_Linux!=-1)
            f.setPixelSize(App::userSettings->guiFontSize_Linux);
    #endif
    QApplication::setFont(f);
    #ifdef LIN_SIM // make the groupbox frame visible on Linux
        qtApp->setStyleSheet("QGroupBox {  border: 1px solid lightgray;} QGroupBox::title {  background-color: transparent; subcontrol-position: top left; padding:2 13px;}");
    #endif
#endif

    uiThread=new CUiThread();
    VThread::setUiThread();
    srand((int)VDateTime::getTimeInMs());    // Important so that the computer ID has some "true" random component!
                                        // Remember that each thread starts with a same seed!!!

#ifdef SIM_WITH_GUI
    // Browser and hierarchy visibility is set in userset.txt. We can override it here:
    if ((operationalUIParts&sim_gui_hierarchy)==0)
        COglSurface::_hierarchyEnabled=false;
    if ((operationalUIParts&sim_gui_browser)==0)
        setBrowserEnabled(false);
    setIcon();
    if ( (operationalUIParts&sim_gui_headless)==0 )
    {
        showSplashScreen();
        createMainWindow();
        mainWindow->oglSurface->adjustBrowserAndHierarchySizesToDefault();
    }
#endif
}

void GuiApp::runGui()
{
    TRACE_INTERNAL;

#ifdef SIM_WITH_GUI
    if (mainWindow!=nullptr)
        mainWindow->setFocus(Qt::MouseFocusReason); // needed because at first Qt behaves strangely (really??)
    uiThread->setFileDialogsNative(App::userSettings->fileDialogs);
#endif

    _loadLegacyPlugins();

    App::setAppStage(App::appstage_guiInitDone);    // now let the SIM thread run freely

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
        App::appendSimulationThreadCommand(cmd,1000);
        cmd.cmdId=DISPLAY_WARNING_IF_DEBUGGING_CMD;
        App::appendSimulationThreadCommand(cmd,3000);
    }

    CSimFlavor::run(7);
    {
        SSimulationThreadCommand cmd;
        cmd.cmdId=CHKLICM_CMD;
        App::appendSimulationThreadCommand(cmd,5000);
    }
#endif

    std::string msg=CSimFlavor::getStringVal(18);
    if (msg.size()>0)
    {
        SSimulationThreadCommand cmd;
        cmd.cmdId=EDU_EXPIRED_CMD;
        cmd.stringParams.push_back(msg);
        App::appendSimulationThreadCommand(cmd,3000);
    }

    qtApp->exec(); // sits here until quit

    CSimFlavor::run(8);
    CSimFlavor::run(5);

    // Send the last "instancePass" message to all old plugins:
    int auxData[4]={0,0,0,0};
    App::worldContainer->pluginContainer->sendEventCallbackMessageToAllPlugins_old(sim_message_eventcallback_lastinstancepass,auxData);
    App::worldContainer->pluginContainer->unloadLegacyPlugins();

    deinitGl_ifNeeded();
}

void GuiApp::setBrowserEnabled(bool e)
{
    _browserEnabled=e;
    setToolbarRefreshFlag();
#ifdef SIM_WITH_GUI
    if (mainWindow!=nullptr)
        mainWindow->setBrowserVisible(_browserEnabled);
#endif
}

bool GuiApp::getBrowserEnabled()
{
    return(_browserEnabled);
}

bool GuiApp::executeUiThreadCommand(SUIThreadCommand* cmdIn,SUIThreadCommand* cmdOut)
{
    return(uiThread->executeCommandViaUiThread(cmdIn,cmdOut));
}

void GuiApp::_loadLegacyPlugins()
{ // from UI thread
    App::logMsg(sim_verbosity_loadinfos|sim_verbosity_onlyterminal,"simulator launched.");
    std::vector<std::string> theNames;
    std::vector<std::string> theDirAndNames;
    {
        QDir dir(App::getApplicationDir().c_str());
        dir.setFilter(QDir::Files|QDir::Hidden);
        dir.setSorting(QDir::Name);
        QStringList filters;
        int bnl=8;
        #ifdef WIN_SIM
            std::string tmp("v_repExt*.dll");
        #endif
        #ifdef MAC_SIM
            std::string tmp("libv_repExt*.dylib");
            bnl=11;
        #endif
        #ifdef LIN_SIM
            std::string tmp("libv_repExt*.so");
            bnl=11;
        #endif
        filters << tmp.c_str();
        dir.setNameFilters(filters);
        QFileInfoList list=dir.entryInfoList();
        for (int i=0;i<list.size();++i)
        {
            QFileInfo fileInfo=list.at(i);
            std::string bla(fileInfo.baseName().toLocal8Bit());
            std::string tmp;
            tmp.assign(bla.begin()+bnl,bla.end());
            if (tmp.find('_')==std::string::npos)
            {
                theNames.push_back(tmp);
                theDirAndNames.push_back(fileInfo.absoluteFilePath().toLocal8Bit().data());
            }
        }
    }

    {
        QDir dir(App::getApplicationDir().c_str());
        dir.setFilter(QDir::Files|QDir::Hidden);
        dir.setSorting(QDir::Name);
        QStringList filters;
        int bnl=6;
        #ifdef WIN_SIM
            std::string tmp("simExt*.dll");
        #endif
        #ifdef MAC_SIM
            std::string tmp("libsimExt*.dylib");
            bnl=9;
        #endif
        #ifdef LIN_SIM
            std::string tmp("libsimExt*.so");
            bnl=9;
        #endif
        filters << tmp.c_str();
        dir.setNameFilters(filters);
        QFileInfoList list=dir.entryInfoList();
        for (int i=0;i<list.size();++i)
        {
            QFileInfo fileInfo=list.at(i);
            std::string bla(fileInfo.baseName().toLocal8Bit());
            std::string tmp;
            tmp.assign(bla.begin()+bnl,bla.end());
            if (tmp.find('_')==std::string::npos)
            {
                theNames.push_back(tmp);
                theDirAndNames.push_back(fileInfo.absoluteFilePath().toLocal8Bit().data());
            }
        }
    }

    for (size_t i=0;i<theNames.size();i++)
    {
        if (theDirAndNames[i].compare("")!=0)
            simLoadModule_internal(theDirAndNames[i].c_str(),theNames[i].c_str()); // not yet loaded
    }
}

#ifdef SIM_WITH_GUI
void GuiApp::showSplashScreen()
{
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
        qtApp->processEvents();
        VThread::sleep(1);
    }
    splash.hide();
}

void GuiApp::setIcon()
{
    qtApp->setWindowIcon(QIcon(CSimFlavor::getStringVal(4).c_str()));
}

void GuiApp::createMainWindow()
{
    TRACE_INTERNAL;
    mainWindow=new CMainWindow();
    mainWindow->initializeWindow();
    setShowConsole(App::userSettings->alwaysShowConsole);
}

void GuiApp::deleteMainWindow()
{
    TRACE_INTERNAL;
    delete mainWindow;
    mainWindow=nullptr;
}

void GuiApp::setShowConsole(bool s)
{
#ifdef WIN_SIM
    if (s)
        ShowWindow(GetConsoleWindow(),SW_SHOW);
    else
        ShowWindow(GetConsoleWindow(),SW_HIDE);
#endif
}
#endif

int GuiApp::getEditModeType()
{ // helper
    int retVal=NO_EDIT_MODE;
#ifdef SIM_WITH_GUI
    if (mainWindow!=nullptr)
        retVal=GuiApp::mainWindow->editModeContainer->getEditModeType();
#endif
    return(retVal);
}

void GuiApp::setRebuildHierarchyFlag()
{ // helper
#ifdef SIM_WITH_GUI
    if (mainWindow!=nullptr)
        mainWindow->oglSurface->hierarchy->setRebuildHierarchyFlag();
#endif
}

void GuiApp::setResetHierarchyViewFlag()
{ // helper
#ifdef SIM_WITH_GUI
    if (mainWindow!=nullptr)
        mainWindow->oglSurface->hierarchy->setResetViewFlag();
#endif
}

void GuiApp::setRefreshHierarchyViewFlag()
{ // helper
#ifdef SIM_WITH_GUI
    if (mainWindow!=nullptr)
        mainWindow->oglSurface->hierarchy->setRefreshViewFlag();
#endif
}

void GuiApp::setLightDialogRefreshFlag()
{ // helper
#ifdef SIM_WITH_GUI
    if (mainWindow!=nullptr)
        mainWindow->setLightDialogRefreshFlag();
#endif
}

void GuiApp::setFullDialogRefreshFlag()
{ // helper
#ifdef SIM_WITH_GUI
    if (mainWindow!=nullptr)
        mainWindow->setFullDialogRefreshFlag();
#endif
}

void GuiApp::setDialogRefreshDontPublishFlag()
{ // helper
#ifdef SIM_WITH_GUI
    if (mainWindow!=nullptr)
        mainWindow->setDialogRefreshDontPublishFlag();
#endif
}

void GuiApp::setToolbarRefreshFlag()
{ // helper
#ifdef SIM_WITH_GUI
    if (mainWindow!=nullptr)
        mainWindow->setToolbarRefreshFlag();
#endif
}

int GuiApp::getMouseMode()
{ // helper
    int retVal=0;
#ifdef SIM_WITH_GUI
    if (mainWindow!=nullptr)
        retVal=mainWindow->getMouseMode();
#endif
    return(retVal);
}

void GuiApp::setMouseMode(int mm)
{ // helper
#ifdef SIM_WITH_GUI
    if (mainWindow!=nullptr)
        mainWindow->setMouseMode(mm);
#endif
}

void GuiApp::setDefaultMouseMode()
{ // helper
#ifdef SIM_WITH_GUI
    if (mainWindow!=nullptr)
        mainWindow->setDefaultMouseMode();
#endif
}

bool GuiApp::isFullScreen()
{ // helper
    bool retVal=false;
#ifdef SIM_WITH_GUI
    if (mainWindow!=nullptr)
        retVal=mainWindow->isFullScreen();
#endif
    return(retVal);
}

void GuiApp::setFullScreen(bool f)
{ // helper
#ifdef SIM_WITH_GUI
    if (mainWindow!=nullptr)
        mainWindow->setFullScreen(f);
#endif
}

bool GuiApp::getShowInertias()
{
    return(_showInertias);
}

void GuiApp::setShowInertias(bool show)
{
    _showInertias=show;
}

void GuiApp::logMsgToStatusbar(const char* msg,bool html)
{
    if (!VThread::isUiThread())
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

bool GuiApp::isOnline()
{
    return(_online);
}

