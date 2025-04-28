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
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QDesktopWidget>
#endif
#ifdef WIN_SIM
#include <QStyleFactory>
#endif
#endif

CSimQApp* GuiApp::qtApp = nullptr;
int GuiApp::_qApp_argc = 1;
char GuiApp::_qApp_arg0[] = {"CoppeliaSim"};
char* GuiApp::_qApp_argv[1] = {_qApp_arg0};
CUiThread* GuiApp::uiThread = nullptr;
int GuiApp::operationalUIParts = 0; // sim_gui_menubar,sim_gui_popupmenus,sim_gui_toolbar1,sim_gui_toolbar2, etc.
bool GuiApp::_browserEnabled = true;
bool GuiApp::_online = false;
bool GuiApp::_showInertias = false;
luaWrap_lua_State* GuiApp::L = nullptr;
int GuiApp::sc = 1;
#ifdef SIM_WITH_GUI
CMainWindow* GuiApp::mainWindow = nullptr;
#endif

GuiApp::GuiApp()
{
    uiThread = nullptr;
    qtApp = nullptr;

    L = luaWrap_luaL_newstate();
    luaWrap_luaL_openlibs(L);
    luaWrap_luaL_dostring(L, "os.setlocale'C'");
    luaWrap_luaL_dostring(
        L, "pi = math.pi; sin = math.sin; cos = math.cos; tan = math.tan; abs = math.abs; acos = math.acos; asin = "
           "math.asin; atan2 = math.atan2; deg = math.deg; exp = math.exp; floor = math.floor; fmod = math.fmod; log = "
           "math.log; max = math.max; min = math.min; pow = math.pow; rad = math.rad; random = math.random; rand = "
           "random; sqrt = math.sqrt; round = function(_a) return math.floor(_a + 0.5) end");
}

GuiApp::~GuiApp()
{
    luaWrap_lua_close(L);
}

long long int GuiApp::getEvalInt(const char* str, bool* ok /*= nullptr*/)
{
    long long int retVal = 0.0;
    int top = luaWrap_lua_gettop(L);
    std::string s(str);
    if (s.find("=") == std::string::npos)
    {
        s = "return abs(" + s;
        s += ")";
    }
    if (ok != nullptr)
        ok[0] = false;
    if (luaWrap_luaL_dostring(L, s.c_str()) == 0)
    {
        if (luaWrap_lua_isinteger(L, -1))
        {
            retVal = luaWrap_lua_tointeger(L, -1);
            if (ok != nullptr)
                ok[0] = true;
        }
    }
    luaWrap_lua_settop(L, top);
    return retVal;
}

double GuiApp::getEvalDouble(const char* str, bool* ok /*= nullptr*/)
{
    double retVal = 0.0;
    int top = luaWrap_lua_gettop(L);
    std::string s(str);
    utils::removeSpacesAtBeginningAndEnd(s);
    if ((s.size() >= 1) && (s[0] == '+'))
        s.erase(s.begin());
    if (s.find("=") == std::string::npos)
    {
        s = "return (" + s;
        s += ")";
    }
    if (ok != nullptr)
        ok[0] = false;
    if (luaWrap_luaL_dostring(L, s.c_str()) == 0)
    {
        if (luaWrap_lua_isnumber(L, -1))
        {
            retVal = luaWrap_lua_tonumber(L, -1);
            if (ok != nullptr)
                ok[0] = true;
        }
    }
    luaWrap_lua_settop(L, top);
    return retVal;
}

void GuiApp::runGui(int options)
{
    while (App::getAppStage() != App::appstage_simInit1Done)
        VThread::sleep(1);
#ifdef USES_QGLWIDGET
#else
    QSurfaceFormat format;
    format.setSwapInterval(0); // turn VSync off
#ifdef WIN_SIM
    format.setVersion(
        3, 2); // macOS does not work when compatibilityProfile is on too, and it seems Linux too (in some cases)
#endif
    format.setProfile(QSurfaceFormat::CompatibilityProfile);
    format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
    format.setRenderableType(QSurfaceFormat::OpenGL);
    format.setRedBufferSize(8);
    format.setGreenBufferSize(8);
    format.setBlueBufferSize(8);
    format.setAlphaBufferSize(0);
    format.setStencilBufferSize(8);
    format.setDepthBufferSize(24);
    QSurfaceFormat::setDefaultFormat(format);
#endif

    QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL, true);
    QApplication* tempApp = new QApplication(_qApp_argc, _qApp_argv);
    uiThread = new CUiThread();
    VThread::setUiThread();
    App::setAppStage(App::appstage_guiInit1Done);
    while (App::getAppStage() != App::appstage_simInit2Done) // wait until SIM thread finished initialization
        VThread::sleep(1);

    _browserEnabled = true;
    for (int i = 0; i < 9; i++)
    {
        std::string str(App::getApplicationArgument(i));
        if ((str.compare(0, 9, "GUIITEMS_") == 0) && (str.length() > 9))
        {
            str.erase(str.begin(), str.begin() + 9);
            int val = 0;
            if (tt::stringToInt(str.c_str(), val))
            {
                options = val;
                break;
            }
        }
    }
    operationalUIParts = options;
    if (operationalUIParts & sim_gui_headless)
        operationalUIParts = sim_gui_headless;

    int highResDisplayDefault = App::userSettings->highResDisplay;
    if (highResDisplayDefault == -1)
    {
        QScreen* scr = tempApp->primaryScreen();
        if (scr != nullptr)
        {
            App::logMsg(sim_verbosity_loadinfos | sim_verbosity_onlyterminal,
                        "primary screen physical dots per inch: %s",
                        std::to_string(int(scr->physicalDotsPerInch() + 0.5)).c_str());
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            QDesktopWidget* dw = tempApp->desktop();
            if (dw != nullptr)
#else
            const QScreen* primaryScreen = tempApp->primaryScreen();
            if (primaryScreen != nullptr)
#endif
            {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                double val = (dw->logicalDpiX() / 96.0) * 100.0;
#else
                double val = (primaryScreen->logicalDotsPerInchX() / 96.0) * 100.0;
#endif
                App::logMsg(sim_verbosity_loadinfos | sim_verbosity_onlyterminal, "display scaling (guessed): %s",
                            std::to_string(int(val + 0.5)).c_str());
#ifndef MAC_SIM
                if (val >= App::userSettings->guessedDisplayScalingThresholdFor2xOpenGl)
                    highResDisplayDefault = 2;
#endif
            }
        }
    }

    delete tempApp;

    if (highResDisplayDefault == 1)
    {
        qputenv("QT_SCALE_FACTOR", "1.0");
        sc = 2;
    }

    if (highResDisplayDefault == 2)
    {
        qputenv("QT_AUTO_SCREEN_SCALE_FACTOR", "1");
        sc = 2;
#ifdef WIN_SIM
        // To address a bug with qscintilla on hdpi display:
        if (App::userSettings->scriptEditorFont == "")
            App::userSettings->scriptEditorFont = "Consolas";
#endif
    }
    if (highResDisplayDefault == 3)
    {
        if (App::userSettings->guiScaling > 1.01)
            qputenv("QT_SCALE_FACTOR", std::to_string(App::userSettings->guiScaling).c_str());
        if (App::userSettings->oglScaling != 1)
            sc = App::userSettings->oglScaling;
    }

    qtApp = new CSimQApp(_qApp_argc, _qApp_argv);

    QHostInfo::lookupHost("www.coppeliarobotics.com", [=](const QHostInfo& info) {
        if (info.error() == QHostInfo::NoError)
            _online = true;
    });

    qRegisterMetaType<std::string>("std::string");
    Q_INIT_RESOURCE(targaFiles);
    Q_INIT_RESOURCE(toolbarFiles);
    Q_INIT_RESOURCE(variousImageFiles);
    Q_INIT_RESOURCE(imageFiles);
    if (App::userSettings->darkMode)
    {
        QFile ff(":qdarkstyle/style.qss");
        if (!ff.exists())
            App::logMsg(sim_verbosity_warnings, "unable to set dark mode.");
        else
        {
            ff.open(QFile::ReadOnly | QFile::Text);
            QTextStream ts(&ff);
            qApp->setStyleSheet(ts.readAll());
        }
    }

#ifdef WIN_SIM
    CSimQApp::setStyle(QStyleFactory::create(
        "Fusion")); // Probably most compatible. Other platforms: best in native (other styles have problems)!
#endif

    if ((options & sim_gui_headless) == 0)
    {
#ifndef MAC_SIM
        if (CAuxLibVideo::loadLibrary())
            App::logMsg(sim_verbosity_loadinfos | sim_verbosity_onlyterminal, "loaded the video compression library.");
        else
        {
            std::string msg("could not find or correctly load the video compression library.");
#ifdef LIN_SIM
            msg += "\nTry following:";
            msg += "\n";
            msg += "\n$ sudo apt-get install libavcodec-dev libavformat-dev libswscale-dev";
            msg += "\nif above fails, try first:";
            msg += "\n$ sudo apt-get -f install";
            msg += "\n";
#endif
            App::logMsg(sim_verbosity_errors, msg.c_str());
        }
#endif
    }

    QFont f = QApplication::font();
#ifdef WIN_SIM
    if (App::userSettings->guiFontSize_Win != -1)
        f.setPixelSize(App::userSettings->guiFontSize_Win);
#endif
#ifdef MAC_SIM
    if (App::userSettings->guiFontSize_Mac != -1)
        f.setPixelSize(App::userSettings->guiFontSize_Mac);
#endif
#ifdef LIN_SIM
    if (App::userSettings->guiFontSize_Linux != -1)
        f.setPixelSize(App::userSettings->guiFontSize_Linux);
#endif
    QApplication::setFont(f);
#ifdef LIN_SIM // make the groupbox frame visible on Linux
    qtApp->setStyleSheet("QGroupBox {  border: 1px solid lightgray;} QGroupBox::title {  background-color: "
                         "transparent; subcontrol-position: top left; padding:2 13px;}");
#endif

    srand((int)VDateTime::getTimeInMs()); // Important so that the computer ID has some "true" random component!
                                          // Remember that each thread starts with a same seed!!!

    // Browser and hierarchy visibility is set in userset.txt. We can override it here:
    if ((operationalUIParts & sim_gui_hierarchy) == 0)
        App::setHierarchyEnabled(false);
    if ((operationalUIParts & sim_gui_browser) == 0)
        setBrowserEnabled(false);
    setIcon();
    initializeRendering();
    if ((operationalUIParts & sim_gui_headless) == 0)
    {
        if ((operationalUIParts & sim_gui_splash) != 0)
            showSplashScreen();
        createMainWindow();
        mainWindow->oglSurface->adjustBrowserAndHierarchySizesToDefault();
    }

    if (mainWindow != nullptr)
        mainWindow->setFocus(Qt::MouseFocusReason); // needed because at first Qt behaves strangely (really??)
    uiThread->setFileDialogsNative(App::userSettings->fileDialogs);

    _loadLegacyPlugins();

    int dl = CSimFlavor::getIntVal(5);
    if (dl > 0)
    {
        SSimulationThreadCommand cmd;
        cmd.cmdId = PLUS_RG_CMD;
        App::appendSimulationThreadCommand(cmd, double(dl));
    }
    {
        SSimulationThreadCommand cmd;
        cmd.cmdId = REFRESH_DIALOGS_CMD;
        App::appendSimulationThreadCommand(cmd, 1.0);
        cmd.cmdId = DISPLAY_WARNING_IF_DEBUGGING_CMD;
        App::appendSimulationThreadCommand(cmd, 3.0);
    }

    App::setAppStage(App::appstage_guiInit2Done); // now let the SIM thread run freely
    {
        SUIThreadCommand cmdIn;
        SUIThreadCommand cmdOut;
        cmdIn.cmdId = CREATE_DEFAULT_MENU_BAR_UITHREADCMD;
        uiThread->executeCommandViaUiThread(&cmdIn, &cmdOut);
    }

    if (CSimFlavor::getBoolVal(20))
        App::postExitRequest();

    qtApp->exec(); // sits here until quit

    // Send the last "instancePass" message to all old plugins:
    int auxData[4] = {0, 0, 0, 0};
    App::worldContainer->pluginContainer->sendEventCallbackMessageToAllPlugins_old(
        sim_message_eventcallback_lastinstancepass, auxData);
    App::worldContainer->pluginContainer->unloadLegacyPlugins();

    deinitGl_ifNeeded();

    deleteMainWindow();
    deinitializeRendering();

    VThread::unsetUiThread();
    delete uiThread;
    uiThread = nullptr;

    // Indicate that we didn't crash:
    CPersistentDataContainer cont;
    cont.writeData("SIMSETTINGS_SIM_CRASHED", "", !App::userSettings->doNotWritePersistentData, false);

    CAuxLibVideo::unloadLibrary();

    if (qtApp != nullptr)
    {
        Q_CLEANUP_RESOURCE(imageFiles);
        Q_CLEANUP_RESOURCE(variousImageFiles);
        Q_CLEANUP_RESOURCE(toolbarFiles);
        Q_CLEANUP_RESOURCE(targaFiles);
        qtApp->disconnect();
        delete qtApp;
        qtApp = nullptr;
    }
    App::setAppStage(App::appstage_guiCleanupDone);
}

void GuiApp::setBrowserEnabled(bool e)
{
    bool diff = (_browserEnabled != e);
    if (diff)
    {
        _browserEnabled = e;
        if ((App::worldContainer != nullptr) && App::worldContainer->getEventsEnabled())
        {
            const char* cmd = propApp_browserEnabled.name;
            CCbor* ev = App::worldContainer->createObjectChangedEvent(sim_handle_app, cmd, true);
            ev->appendKeyBool(cmd, _browserEnabled);
            App::worldContainer->pushEvent();
        }
#ifdef SIM_WITH_GUI
        setToolbarRefreshFlag();
        if (mainWindow != nullptr)
            mainWindow->setBrowserVisible(_browserEnabled);
#endif
    }
}

bool GuiApp::getBrowserEnabled()
{
    return (_browserEnabled);
}

bool GuiApp::executeUiThreadCommand(SUIThreadCommand* cmdIn, SUIThreadCommand* cmdOut)
{
    return (uiThread->executeCommandViaUiThread(cmdIn, cmdOut));
}

void GuiApp::_loadLegacyPlugins()
{ // from UI thread
    App::logMsg(sim_verbosity_loadinfos | sim_verbosity_onlyterminal, "simulator launched.");
    std::vector<std::string> theNames;
    std::vector<std::string> theDirAndNames;
    {
        QDir dir(App::getApplicationDir().c_str());
        dir.setFilter(QDir::Files | QDir::Hidden);
        dir.setSorting(QDir::Name);
        QStringList filters;
        int bnl = 8;
#ifdef WIN_SIM
        std::string tmp("v_repExt*.dll");
#endif
#ifdef MAC_SIM
        std::string tmp("libv_repExt*.dylib");
        bnl = 11;
#endif
#ifdef LIN_SIM
        std::string tmp("libv_repExt*.so");
        bnl = 11;
#endif
        filters << tmp.c_str();
        dir.setNameFilters(filters);
        QFileInfoList list = dir.entryInfoList();
        for (int i = 0; i < list.size(); ++i)
        {
            QFileInfo fileInfo = list.at(i);
            std::string bla(fileInfo.baseName().toLocal8Bit());
            std::string tmp;
            tmp.assign(bla.begin() + bnl, bla.end());
            if (tmp.find('_') == std::string::npos)
            {
                theNames.push_back(tmp);
                theDirAndNames.push_back(fileInfo.absoluteFilePath().toLocal8Bit().data());
            }
        }
    }

    {
        QDir dir(App::getApplicationDir().c_str());
        dir.setFilter(QDir::Files | QDir::Hidden);
        dir.setSorting(QDir::Name);
        QStringList filters;
        int bnl = 6;
#ifdef WIN_SIM
        std::string tmp("simExt*.dll");
#endif
#ifdef MAC_SIM
        std::string tmp("libsimExt*.dylib");
        bnl = 9;
#endif
#ifdef LIN_SIM
        std::string tmp("libsimExt*.so");
        bnl = 9;
#endif
        filters << tmp.c_str();
        dir.setNameFilters(filters);
        QFileInfoList list = dir.entryInfoList();
        for (int i = 0; i < list.size(); ++i)
        {
            QFileInfo fileInfo = list.at(i);
            std::string bla(fileInfo.baseName().toLocal8Bit());
            std::string tmp;
            tmp.assign(bla.begin() + bnl, bla.end());
            if (tmp.find('_') == std::string::npos)
            {
                theNames.push_back(tmp);
                theDirAndNames.push_back(fileInfo.absoluteFilePath().toLocal8Bit().data());
            }
        }
    }

    for (size_t i = 0; i < theNames.size(); i++)
    {
        if (theDirAndNames[i].compare("") != 0)
            simLoadModule_internal(theDirAndNames[i].c_str(), theNames[i].c_str()); // not yet loaded
    }
}

#ifdef SIM_WITH_GUI
void GuiApp::showSplashScreen()
{
    QPixmap pixmap;

    pixmap.load(CSimFlavor::getStringVal(1).c_str());

    QSplashScreen splash(pixmap, Qt::WindowStaysOnTopHint);
    splash.setMask(pixmap.mask());
    QString txt("Version ");
    txt += SIM_PROGRAM_VERSION;
    txt += " ";
    txt += SIM_PROGRAM_REVISION;
    txt += ", Built ";
    txt += __DATE__;
    splash.showMessage(txt, Qt::AlignLeft | Qt::AlignBottom);
    splash.show();
    int ct = (int)VDateTime::getTimeInMs();
    while (VDateTime::getTimeDiffInMs(ct) < 2000)
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
    mainWindow = new CMainWindow(operationalUIParts);
    mainWindow->initializeWindow();
    setShowConsole(App::userSettings->alwaysShowConsole);
}

void GuiApp::deleteMainWindow()
{
    TRACE_INTERNAL;
    delete mainWindow;
    mainWindow = nullptr;
}

void GuiApp::setShowConsole(bool s)
{
#ifdef WIN_SIM
    if (s)
        ShowWindow(GetConsoleWindow(), SW_SHOW);
    else
        ShowWindow(GetConsoleWindow(), SW_HIDE);
#endif
}
#endif

int GuiApp::getEditModeType()
{ // helper
    int retVal = NO_EDIT_MODE;
#ifdef SIM_WITH_GUI
    if (mainWindow != nullptr)
        retVal = mainWindow->editModeContainer->getEditModeType();
#endif
    return (retVal);
}

void GuiApp::setRebuildHierarchyFlag()
{ // helper
#ifdef SIM_WITH_GUI
    if (mainWindow != nullptr)
        mainWindow->oglSurface->hierarchy->setRebuildHierarchyFlag();
#endif
}

void GuiApp::setResetHierarchyViewFlag()
{ // helper
#ifdef SIM_WITH_GUI
    if (mainWindow != nullptr)
        mainWindow->oglSurface->hierarchy->setResetViewFlag();
#endif
}

void GuiApp::setRefreshHierarchyViewFlag()
{ // helper
#ifdef SIM_WITH_GUI
    if (mainWindow != nullptr)
        mainWindow->oglSurface->hierarchy->setRefreshViewFlag();
#endif
}

void GuiApp::setLightDialogRefreshFlag()
{ // helper
#ifdef SIM_WITH_GUI
    if (mainWindow != nullptr)
        mainWindow->setLightDialogRefreshFlag();
#endif
}

void GuiApp::setFullDialogRefreshFlag()
{ // helper
#ifdef SIM_WITH_GUI
    if (mainWindow != nullptr)
        mainWindow->setFullDialogRefreshFlag();
#endif
}

void GuiApp::setDialogRefreshDontPublishFlag()
{ // helper
#ifdef SIM_WITH_GUI
    if (mainWindow != nullptr)
        mainWindow->setDialogRefreshDontPublishFlag();
#endif
}

void GuiApp::setToolbarRefreshFlag()
{ // helper
#ifdef SIM_WITH_GUI
    if (mainWindow != nullptr)
        mainWindow->setToolbarRefreshFlag();
#endif
}

int GuiApp::getMouseMode()
{ // helper
    int retVal = 0;
#ifdef SIM_WITH_GUI
    if (mainWindow != nullptr)
        retVal = mainWindow->getMouseMode();
#endif
    return (retVal);
}

void GuiApp::setMouseMode(int mm)
{ // helper
#ifdef SIM_WITH_GUI
    if (mainWindow != nullptr)
        mainWindow->setMouseMode(mm);
#endif
}

void GuiApp::setDefaultMouseMode()
{ // helper
#ifdef SIM_WITH_GUI
    if (mainWindow != nullptr)
        mainWindow->setDefaultMouseMode();
#endif
}

bool GuiApp::isFullScreen()
{ // helper
    bool retVal = false;
#ifdef SIM_WITH_GUI
    if (mainWindow != nullptr)
        retVal = mainWindow->isFullScreen();
#endif
    return (retVal);
}

void GuiApp::setFullScreen(bool f)
{ // helper
#ifdef SIM_WITH_GUI
    if (mainWindow != nullptr)
        mainWindow->setFullScreen(f);
#endif
}

bool GuiApp::canShowDialogs()
{
    bool retVal = false;
#ifdef SIM_WITH_GUI
    retVal = (GuiApp::mainWindow != nullptr) && (!GuiApp::isFullScreen()) && ((GuiApp::operationalUIParts & sim_gui_dialogs) != 0);
#endif
    return retVal;
}

bool GuiApp::getShowInertias()
{
    return (_showInertias);
}

void GuiApp::setShowInertias(bool show)
{
    _showInertias = show;
}

void GuiApp::logMsgToStatusbar(const char* msg, bool html)
{
    if (!VThread::isUiThread())
    { // we are NOT in the UI thread. We execute the command in a delayed manner:
        SUIThreadCommand cmdIn;
        cmdIn.cmdId = LOG_MSG_TO_STATUSBAR_UITHREADCMD;
        cmdIn.stringParams.push_back(msg);
        cmdIn.boolParams.push_back(html);
        uiThread->executeCommandViaUiThread(&cmdIn, nullptr);
    }
#ifdef SIM_WITH_GUI
    else
    {
        std::string str(msg);

        if (mainWindow != nullptr)
        {
            std::string txtCol(mainWindow->palette().windowText().color().name().toStdString());
            if ((operationalUIParts & sim_gui_statusbar) && (mainWindow->statusBar != nullptr))
            {
                if (html)
                    //                {
                    //                    str+="<font color="+txtCol+">"+" </font>"; // color is otherwise not reset
                    mainWindow->statusBar->appendHtml(str.c_str());
                //                }
                else
                    mainWindow->statusBar->appendPlainText(str.c_str());
                mainWindow->statusBar->moveCursor(QTextCursor::End);
                mainWindow->statusBar->verticalScrollBar()->setValue(
                    mainWindow->statusBar->verticalScrollBar()->maximum());
                mainWindow->statusBar->ensureCursorVisible();
            }
        }
    }
#endif
}

bool GuiApp::isOnline()
{
    return (_online);
}

void GuiApp::clearStatusbar()
{
    if (!VThread::isUiThread())
    { // we are NOT in the UI thread. We execute the command in a delayed manner:
        SUIThreadCommand cmdIn;
        SUIThreadCommand cmdOut;
        cmdIn.cmdId = CLEAR_STATUSBAR_UITHREADCMD;
        GuiApp::uiThread->executeCommandViaUiThread(&cmdIn, &cmdOut);
    }
    else
    {
#ifdef SIM_WITH_GUI
        if (mainWindow != nullptr)
        {
            if ((operationalUIParts & sim_gui_statusbar) && (mainWindow->statusBar != nullptr))
                mainWindow->statusBar->clear();
        }
#endif
    }
}

float* GuiApp::getRGBPointerFromItem(int objType, int objID1, int objID2, int colComponent, std::string* auxDlgTitle)
{ // auxDlgTitle can be nullptr
    std::string __auxDlgTitle;
    std::string* _auxDlgTitle = &__auxDlgTitle;
    if (auxDlgTitle != nullptr)
        _auxDlgTitle = auxDlgTitle;

    if (objType == COLOR_ID_AMBIENT_LIGHT)
    {
        _auxDlgTitle->assign("Ambient light");
        return (App::currentWorld->environment->ambientLightColor);
    }
    if (objType == COLOR_ID_BACKGROUND_UP)
    {
        _auxDlgTitle->assign("Background (up)");
        return (App::currentWorld->environment->backGroundColor);
    }
    if (objType == COLOR_ID_BACKGROUND_DOWN)
    {
        _auxDlgTitle->assign("Background (down)");
        return (App::currentWorld->environment->backGroundColorDown);
    }
    if (objType == COLOR_ID_FOG)
    {
        _auxDlgTitle->assign("Fog");
        return (App::currentWorld->environment->fogBackgroundColor);
    }
    if (objType == COLOR_ID_MIRROR)
    {
        _auxDlgTitle->assign("Mirror");
        CMirror* it = App::currentWorld->sceneObjects->getMirrorFromHandle(objID1);
        if ((it != nullptr) && it->getIsMirror())
            return (it->mirrorColor);
    }
    if (objType == COLOR_ID_OCTREE)
    {
        _auxDlgTitle->assign("OC tree");
        COcTree* it = App::currentWorld->sceneObjects->getOctreeFromHandle(objID1);
        if (it != nullptr)
            return (it->getColor()->getColorsPtr());
    }
    if (objType == COLOR_ID_POINTCLOUD)
    {
        _auxDlgTitle->assign("Point cloud");
        CPointCloud* it = App::currentWorld->sceneObjects->getPointCloudFromHandle(objID1);
        if (it != nullptr)
            return (it->getColor()->getColorsPtr());
    }
    if (objType == COLOR_ID_GRAPH_2DCURVE)
    {
        _auxDlgTitle->assign("Graph - 2D curve");
        CGraph* it = App::currentWorld->sceneObjects->getGraphFromHandle(objID1);
        if (it != nullptr)
        {
            CGraphDataComb_old* grDataComb = it->getGraphData2D(objID2);
            if (grDataComb != nullptr)
                return (grDataComb->curveColor.getColorsPtr());
        }
    }
    if (objType == COLOR_ID_GRAPH_BACKGROUND)
    {
        _auxDlgTitle->assign("Graph - background");
        CGraph* it = App::currentWorld->sceneObjects->getGraphFromHandle(objID1);
        if (it != nullptr)
            return (it->backgroundColor);
    }
    if (objType == COLOR_ID_GRAPH_GRID)
    {
        _auxDlgTitle->assign("Graph - grid");
        CGraph* it = App::currentWorld->sceneObjects->getGraphFromHandle(objID1);
        if (it != nullptr)
            return (it->foregroundColor);
    }
    if (objType == COLOR_ID_GRAPH_TIMECURVE)
    {
        _auxDlgTitle->assign("Graph - data stream");
        CGraph* it = App::currentWorld->sceneObjects->getGraphFromHandle(objID1);
        if (it != nullptr)
        {
            CGraphData_old* grData = it->getGraphData(objID2);
            if (grData != nullptr)
                return (grData->ambientColor);
        }
    }
    if ((objType == COLOR_ID_OPENGLBUTTON_UP) || (objType == COLOR_ID_OPENGLBUTTON_DOWN) ||
        (objType == COLOR_ID_OPENGLBUTTON_TEXT))
    {
        if (objType == COLOR_ID_OPENGLBUTTON_UP)
            _auxDlgTitle->assign("Button - up");
        if (objType == COLOR_ID_OPENGLBUTTON_DOWN)
            _auxDlgTitle->assign("Button - down");
        if (objType == COLOR_ID_OPENGLBUTTON_TEXT)
            _auxDlgTitle->assign("Button - text");
        CButtonBlock* block = App::currentWorld->buttonBlockContainer_old->getBlockWithID(objID1);
        if (block != nullptr)
        {
            CSoftButton* itButton = block->getButtonWithID(objID2);
            if (itButton != nullptr)
            {
                if (objType == COLOR_ID_OPENGLBUTTON_UP)
                    return (itButton->backgroundColor);
                if (objType == COLOR_ID_OPENGLBUTTON_DOWN)
                    return (itButton->downBackgroundColor);
                if (objType == COLOR_ID_OPENGLBUTTON_TEXT)
                    return (itButton->textColor);
            }
        }
    }

    int allowedParts = 0;
    CColorObject* vp = getVisualParamPointerFromItem(objType, objID1, objID2, _auxDlgTitle, &allowedParts);
    if (vp != nullptr)
    {
        if ((colComponent == sim_colorcomponent_ambient_diffuse) && (allowedParts & 1))
            return ((vp->getColorsPtr() + 0));
        if ((colComponent == sim_colorcomponent_diffuse) && (allowedParts & 2))
            return ((vp->getColorsPtr() + 3));
        if ((colComponent == sim_colorcomponent_specular) && (allowedParts & 4))
            return ((vp->getColorsPtr() + 6));
        if ((colComponent == sim_colorcomponent_emission) && (allowedParts & 8))
            return ((vp->getColorsPtr() + 9));
        if ((colComponent == sim_colorcomponent_auxiliary) && (allowedParts & 16))
            return ((vp->getColorsPtr() + 12));
    }

    return (nullptr);
}

CColorObject* GuiApp::getVisualParamPointerFromItem(int objType, int objID1, int objID2, std::string* auxDlgTitle,
                                                    int* allowedParts)
{ // auxDlgTitle and allowedParts can be nullptr. Bit-coded: 1=ambient/diffuse, 2=diffuse(light only), 4=spec, 8=emiss.,
    // 16=aux channels, 32=pulsation, 64=shininess, 128=opacity, 256=colorName, 512=ext. string
    std::string __auxDlgTitle;
    int __allowedParts;
    std::string* _auxDlgTitle = &__auxDlgTitle;
    int* _allowedParts = &__allowedParts;
    if (auxDlgTitle != nullptr)
        _auxDlgTitle = auxDlgTitle;
    if (allowedParts != nullptr)
        _allowedParts = allowedParts;

    if (objType == COLOR_ID_CAMERA_A)
    {
        _auxDlgTitle->assign("Camera");
        _allowedParts[0] = 1 + 4 + 8 + 16 + 32 + 64;
        CCamera* it = App::currentWorld->sceneObjects->getCameraFromHandle(objID1);
        if (it != nullptr)
            return (it->getColor(false));
    }
    if (objType == COLOR_ID_FORCESENSOR_A)
    {
        _auxDlgTitle->assign("Force sensor");
        _allowedParts[0] = 1 + 4 + 8 + 16 + 32 + 64;
        CForceSensor* it = App::currentWorld->sceneObjects->getForceSensorFromHandle(objID1);
        if (it != nullptr)
            return (it->getColor(false));
    }
    if (objType == COLOR_ID_JOINT_A)
    {
        _auxDlgTitle->assign("Joint");
        _allowedParts[0] = 1 + 4 + 8 + 16 + 32 + 64;
        CJoint* it = App::currentWorld->sceneObjects->getJointFromHandle(objID1);
        if (it != nullptr)
            return ((CColorObject*)it->getColor(false));
    }
    if (objType == COLOR_ID_PATH)
    {
        _auxDlgTitle->assign("Path");
        _allowedParts[0] = 1 + 4 + 8 + 16 + 32 + 64;
        CPath_old* it = App::currentWorld->sceneObjects->getPathFromHandle(objID1);
        if ((it != nullptr) && (it->pathContainer != nullptr))
            return (&it->pathContainer->_lineColor);
    }
    if (objType == COLOR_ID_PATH_SHAPING)
    {
        _auxDlgTitle->assign("Path shaping");
        _allowedParts[0] = 1 + 4 + 8 + 16 + 32 + 64;
        CPath_old* it = App::currentWorld->sceneObjects->getPathFromHandle(objID1);
        if (it != nullptr)
            return (it->getShapingColor());
    }
    if (objType == COLOR_ID_GRAPH_3DCURVE)
    {
        _auxDlgTitle->assign("Graph - 3D curve");
        _allowedParts[0] = 1 + 8;
        CGraph* it = App::currentWorld->sceneObjects->getGraphFromHandle(objID1);
        if (it != nullptr)
        {
            CGraphDataComb_old* grDataComb = it->getGraphData3D(objID2);
            if (grDataComb != nullptr)
                return (&grDataComb->curveColor);
        }
    }
    if (objType == COLOR_ID_COLLISIONCONTOUR)
    {
        _auxDlgTitle->assign("Collision contour");
        _allowedParts[0] = 1 + 4 + 8 + 16 + 32 + 64;
        CCollisionObject_old* it = App::currentWorld->collisions_old->getObjectFromHandle(objID1);
        if (it != nullptr)
            return (it->getContourColor());
    }
    if (objType == COLOR_ID_DISTANCESEGMENT)
    {
        _auxDlgTitle->assign("Distance segment");
        _allowedParts[0] = 1 + 4 + 8 + 16 + 32 + 64;
        CDistanceObject_old* it = App::currentWorld->distances_old->getObjectFromHandle(objID1);
        if (it != nullptr)
            return (it->getSegmentColor());
    }
    if (objType == COLOR_ID_CLIPPINGPLANE)
    {
        _auxDlgTitle->assign("Clipping plane");
        _allowedParts[0] = 1 + 4 + 8 + 16 + 32 + 64 + 128;
        CMirror* it = App::currentWorld->sceneObjects->getMirrorFromHandle(objID1);
        if ((it != nullptr) && (!it->getIsMirror()))
            return (it->getClipPlaneColor());
    }
    if (objType == COLOR_ID_LIGHT_CASING)
    {
        _auxDlgTitle->assign("Light - casing");
        _allowedParts[0] = 1 + 4 + 8 + 16 + 64;
        CLight* it = App::currentWorld->sceneObjects->getLightFromHandle(objID1);
        if (it != nullptr)
            return (it->getColor(false));
    }
    if (objType == COLOR_ID_LIGHT_LIGHT)
    {
        _auxDlgTitle->assign("Light");
        _allowedParts[0] = 2 + 4;
        CLight* it = App::currentWorld->sceneObjects->getLightFromHandle(objID1);
        if (it != nullptr)
            return (it->getColor(true));
    }
    if (objType == COLOR_ID_DUMMY)
    {
        _auxDlgTitle->assign("Dummy");
        _allowedParts[0] = 1 + 4 + 8 + 16 + 32 + 64;
        CDummy* it = App::currentWorld->sceneObjects->getDummyFromHandle(objID1);
        if (it != nullptr)
            return ((CColorObject*)it->getDummyColor());
    }
    if (objType == COLOR_ID_SCRIPT)
    {
        _auxDlgTitle->assign("Script");
        _allowedParts[0] = 1 + 4 + 8 + 16 + 32 + 64;
        CScript* it = App::currentWorld->sceneObjects->getScriptFromHandle(objID1);
        if (it != nullptr)
            return ((CColorObject*)it->getScriptColor());
    }
    if (objType == COLOR_ID_VISIONSENSOR)
    {
        _auxDlgTitle->assign("Vision sensor");
        _allowedParts[0] = 1 + 4 + 8 + 16 + 32;
        CVisionSensor* it = App::currentWorld->sceneObjects->getVisionSensorFromHandle(objID1);
        if (it != nullptr)
            return (it->getColor());
    }
    if (objType == COLOR_ID_PROXSENSOR_VOLUME)
    {
        _auxDlgTitle->assign("Proximity sensor");
        _allowedParts[0] = 1 + 4 + 8 + 16 + 32;
        CProxSensor* it = App::currentWorld->sceneObjects->getProximitySensorFromHandle(objID1);
        if (it != nullptr)
            return (it->getColor(0));
    }
    if (objType == COLOR_ID_PROXSENSOR_RAY)
    {
        _auxDlgTitle->assign("Proximity sensor - ray");
        _allowedParts[0] = 1 + 4 + 8 + 16 + 32;
        CProxSensor* it = App::currentWorld->sceneObjects->getProximitySensorFromHandle(objID1);
        if (it != nullptr)
            return (it->getColor(1));
    }
    if (objType == COLOR_ID_MILL_PASSIVE)
    {
        _auxDlgTitle->assign("Mill - passive");
        _allowedParts[0] = 1 + 4 + 8 + 16 + 32;
        CMill* it = App::currentWorld->sceneObjects->getMillFromHandle(objID1);
        if (it != nullptr)
            return (it->getColor(false));
    }
    if (objType == COLOR_ID_MILL_ACTIVE)
    {
        _auxDlgTitle->assign("Mill - active");
        _allowedParts[0] = 1 + 4 + 8 + 16 + 32;
        CMill* it = App::currentWorld->sceneObjects->getMillFromHandle(objID1);
        if (it != nullptr)
            return (it->getColor(true));
    }
    if (objType == COLOR_ID_SHAPE)
    {
        _auxDlgTitle->assign("Shape");
        _allowedParts[0] = 1 + 4 + 8 + 16 + 32 + 64 + 128 + 256 + 512;
        CShape* it = App::currentWorld->sceneObjects->getShapeFromHandle(objID1);
        if ((it != nullptr) && (!it->isCompound()))
            return (&it->getSingleMesh()->color);
    }
    if (objType == COLOR_ID_SHAPE_GEOMETRY)
    {
#ifdef SIM_WITH_GUI
        if ((mainWindow->editModeContainer->getEditModeObjectID() == objID1) &&
            (mainWindow->editModeContainer->getMultishapeEditMode()->getMultishapeGeometricComponentIndex() == objID2))
        {
            _auxDlgTitle->assign("Shape component");
            _allowedParts[0] = 1 + 4 + 8 + 16 + 32 + 64 + 128 + 256 + 512;
            CShape* it = App::currentWorld->sceneObjects->getShapeFromHandle(objID1);
            if ((it != nullptr) && it->isCompound())
            {
                std::vector<CMesh*> allGeometrics;
                it->getMesh()->getAllMeshComponentsCumulative(C7Vector::identityTransformation, allGeometrics);
                if ((objID2 >= 0) && (objID2 < int(allGeometrics.size())))
                    return (&allGeometrics[objID2]->color);
            }
        }
#endif
    }

    _allowedParts[0] = 0;
    return (nullptr);
}

CTextureProperty* GuiApp::getTexturePropertyPointerFromItem(int objType, int objID1, int objID2,
                                                            std::string* auxDlgTitle, bool* is3D, bool* valid,
                                                            CMesh** geom)
{ // auxDlgTitle, is3D, isValid and geom can be nullptr.
    std::string __auxDlgTitle;
    bool __is3D = false;
    bool __isValid = false;
    CMesh* __geom = nullptr;
    std::string* _auxDlgTitle = &__auxDlgTitle;
    bool* _is3D = &__is3D;
    bool* _isValid = &__isValid;
    CMesh** _geom = &__geom;
    if (auxDlgTitle != nullptr)
        _auxDlgTitle = auxDlgTitle;
    if (is3D != nullptr)
        _is3D = is3D;
    if (valid != nullptr)
        _isValid = valid;
    if (geom != nullptr)
        _geom = geom;
    _isValid[0] = false;
    _geom[0] = nullptr;
    if (objType == TEXTURE_ID_SIMPLE_SHAPE)
    {
        _auxDlgTitle->assign("Shape");
        _is3D[0] = true;
        CShape* it = App::currentWorld->sceneObjects->getShapeFromHandle(objID1);
        if ((it != nullptr) && (!it->isCompound()))
        {
            _isValid[0] = true;
            _geom[0] = it->getSingleMesh();
            return (_geom[0]->getTextureProperty());
        }
    }
    if (objType == TEXTURE_ID_COMPOUND_SHAPE)
    {
        _auxDlgTitle->assign("Shape component");
        _is3D[0] = true;
        CShape* it = App::currentWorld->sceneObjects->getShapeFromHandle(objID1);
        if (it != nullptr)
        {
            std::vector<CMesh*> allGeometrics;
            it->getMesh()->getAllMeshComponentsCumulative(C7Vector::identityTransformation, allGeometrics);
            if ((objID2 >= 0) && (objID2 < int(allGeometrics.size())))
            {
                _isValid[0] = true;
                _geom[0] = allGeometrics[objID2];
                return (_geom[0]->getTextureProperty());
            }
        }
    }
    if (objType == TEXTURE_ID_OPENGL_GUI_BACKGROUND)
    {
        _auxDlgTitle->assign("OpenGl custom UI background");
        _is3D[0] = false;
        CButtonBlock* it = App::currentWorld->buttonBlockContainer_old->getBlockWithID(objID1);
        if (it != nullptr)
        {
            _isValid[0] = true;
            return (it->getTextureProperty());
        }
    }
    if (objType == TEXTURE_ID_OPENGL_GUI_BUTTON)
    {
        _auxDlgTitle->assign("OpenGl custom UI button");
        _is3D[0] = false;
        CButtonBlock* it = App::currentWorld->buttonBlockContainer_old->getBlockWithID(objID1);
        if (it != nullptr)
        {
            CSoftButton* butt = it->getButtonWithID(objID2);
            if (butt != nullptr)
            {
                _isValid[0] = true;
                return (butt->getTextureProperty());
            }
        }
    }
    return (nullptr);
}

bool GuiApp::canDisassemble(int objectHandle)
{
    bool retVal = false;
    IF_UI_EVENT_CAN_READ_DATA
    {
        retVal = App::disassemble(objectHandle, true);
    }
    return retVal;
}

bool GuiApp::canAssemble(int parentHandle, int childHandle)
{
    bool retVal = false;
    IF_UI_EVENT_CAN_READ_DATA
    {
        retVal = App::assemble(parentHandle, childHandle, true);
    }
    return retVal;
}
