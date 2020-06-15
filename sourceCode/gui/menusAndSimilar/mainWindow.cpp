// This file requires serious refactoring!!

#include "simInternal.h"
#include "mainWindow.h"
#include "oGL.h"
#include "simulation.h"
#include "oglSurface.h"
#include "sceneObjectOperations.h"
#include "fileOperations.h"
#include "imgLoaderSaver.h"
#include "toolBarCommand.h"
#include "addOperations.h"
#include "dlgCont.h"
#include "helpMenu.h"
#include "gV.h"
#include "tt.h"
#include "app.h"
#include "pluginContainer.h"
#include "auxLibVideo.h"
#include "vVarious.h"
#include "simStrings.h"
#include "vDateTime.h"
#include "vFileFinder.h"
#include "ttUtil.h"
#include "rendering.h"
#include "vMessageBox.h"
#include <boost/lexical_cast.hpp>
#include "qdlgmessageandcheckbox.h"
#include "simFlavor.h"
#include <QMimeData>
#include <QDragEnterEvent>
#include <QUrl>
#include <QToolButton>
#include <QLabel>
#include <QWindow>
#ifdef MAC_SIM
    #include <curses.h> // required for the beep() function
#endif
#ifdef LIN_SIM
    #include <GL/glx.h>
#endif

const int DEFAULT_MOUSE_MODE=sim_navigation_camerashift|sim_navigation_clickselection|sim_navigation_ctrlselection|sim_navigation_shiftselection|sim_navigation_camerazoomwheel|sim_navigation_camerarotaterightbutton;

CMainWindow::CMainWindow() : QMainWindow()
{
    TRACE_INTERNAL;
    _focusObject=FOCUS_ON_PAGE;
    _clientArea.x=1024;
    _clientArea.y=768;
    _toolbar1=nullptr;
    _toolbar2=nullptr;
    _menubar=nullptr;
    _fullscreen=false;
    _hasStereo=false;
    _stereoDistance=0.0f;
    _statusbarFlashTime=-1;
    _leftEye=true;
    _openGlDisplayEnabled=true;
    _mouseMode=DEFAULT_MOUSE_MODE;
    _flyModeCameraHandle=-1;
    _proxSensorClickSelectDown=0;
    _proxSensorClickSelectUp=0;

    lastInstance=-1;
    timeCounter=VDateTime::getTimeInMs();
    lastTimeRenderingStarted=VDateTime::getTimeInMs();
    previousDisplayWasEnabled=0;
    previousCursor=-1;

    _lightDialogRefreshFlag=false;
    _fullDialogRefreshFlag=false;
    _dialogRefreshDontPublishFlag=false;
    _toolbarRefreshFlag=false;


    _toolbarButtonObjectShiftEnabled=true;
    _toolbarButtonObjectRotateEnabled=true;
    _toolbarButtonHierarchyEnabled=true;
    _toolbarButtonBrowserEnabled=true;
    _toolbarButtonObjPropEnabled=true;
    _toolbarButtonCalcModulesEnabled=true;

    _toolbarButtonPlayEnabled=true;
    _toolbarButtonPauseEnabled=true;
    _toolbarButtonStopEnabled=true;

    sceneHierarchyWidget=nullptr;

    editModeContainer=new CEditModeContainer();
    oglSurface=new COglSurface();
    codeEditorContainer=new CCodeEditorContainer();

    // the simulator instances were created before the main window was created,
    // so duplicate the correct instance count here:
    for (int i=0;i<App::worldContainer->getWorldCount();i++)
        newInstanceAboutToBeCreated();

    if ( (App::userSettings->highResDisplay==1)||((devicePixelRatio()>1.2f)&&(App::userSettings->highResDisplay==-1)) )
        App::sc=2;

    customMenuBarItemContainer=new CCustomMenuBarItemContainer();
    dlgCont=new CDlgCont(this);

    simulationRecorder=new CSimRecorder(App::folders->getVideosPath().c_str());
    _mouseButtonsState=0;
    _keyDownState=0;

    resize(1024,768);

    // setWindowTitle adds multiple app icons on Linux somehow..
#ifndef LIN_SIM
    setWindowTitle(App::getApplicationName().c_str()); // somehow it is important for Linux. Even if this title gets later overwritten, Linux keeps this text to display the app name when minimized
#endif



   // --- Browser ---
    modelListWidget=new CModelListWidget();
    modelListWidget->setMaximumWidth(170*4);
    modelListWidget->setMinimumWidth(180);

    modelFolderWidget=new CModelFolderWidget(modelListWidget,"Model browser",App::folders->getModelsPath().c_str(),CSimFlavor::getStringVal(15).c_str());

    #ifdef MAC_SIM
        if (modelFolderWidget->hasError())
        {
            SSimulationThreadCommand cmd;
            cmd.cmdId=DISPLAY_MESSAGE_CMD;
            cmd.intParams.push_back(sim_msgbox_type_warning);
            cmd.stringParams.push_back("Model folder not found");
            cmd.stringParams.push_back(IDSNS_MAC_FILE_ATTRIBUTE_PROBLEM);
            App::appendSimulationThreadCommand(cmd,5000);
        }
    #endif

    _modelBrowser=new QSplitter(Qt::Vertical);
    _modelBrowser->addWidget(modelFolderWidget);
    _modelBrowser->addWidget(modelListWidget);
    if (!App::getBrowserEnabled())
        _modelBrowser->setVisible(false); // do not explicitely set to true (not nice artifacts during creation). Is true by default anyways
// -----------

// --- Hierarchy container ---
    sceneHierarchyWidget=new CSceneHierarchyWidget();
    _sceneHierarchyWidgetList.push_back(sceneHierarchyWidget);

    sceneHierarchyLayout=new QHBoxLayout();
    sceneHierarchyLayout->addWidget(sceneHierarchyWidget);
    sceneHierarchyLayout->setSpacing(0);
    sceneHierarchyLayout->setContentsMargins(0,0,0,0);
    QWidget* hierarchyContainer=new QWidget();
    hierarchyContainer->setLayout(sceneHierarchyLayout);
    hierarchyContainer->setVisible(false); // do not explicitely set to true (not nice artifacts during creation). Is true by default anyways
// -----------

// --- OpenGl widget ---
    openglWidget=new COpenglWidget();
    if (App::userSettings->stereoDist>0.0f)
    {
        if (openglWidget->format().stereo())
        {
            App::logMsg(sim_verbosity_loadinfos,"enabled stereo (OpenGL).");
            _hasStereo=true;
            setStereoDistance(App::userSettings->stereoDist);
        }
        else
            App::logMsg(sim_verbosity_errors,"could not enable stereo (OpenGL).");
    }
    #ifdef LIN_SIM
        std::string msg("if CoppeliaSim crashes now, try to install libgl1-mesa-dev on your system:");
        msg+="\n    >sudo apt install libgl1-mesa-dev";
        App::logMsg(sim_verbosity_loadinfos,msg.c_str());
    #endif
    #ifndef USING_QOPENGLWIDGET
        openglWidget->makeCurrent();
        initGl_ifNeeded();
    #endif
    #ifdef LIN_SIM
        App::logMsg(sim_verbosity_infos,"...did not crash.");
    #endif
// -----------

// --- ( Hierarchy container + openGl widget ) splitter ---
    _sceneHierarchySplitter=new QSplitter(Qt::Horizontal);
    _sceneHierarchySplitter->addWidget(hierarchyContainer);
    _sceneHierarchySplitter->addWidget(openglWidget);
// -----------

// --- Tab widget ---
    tabBar=new QTabBar();
    tabBar->addTab(App::currentWorld->mainSettings->getSceneNameForUi().c_str());

    #ifdef MAC_SIM
        tabBar->setExpanding(true);
    #else
        tabBar->setExpanding(false);
    #endif

    tabBar->setDocumentMode(true);
//    tabBar->setStyleSheet("QTabBar::tab { height: 20px;}");
    tabBar->setElideMode(Qt::ElideRight);
    //tabBar->setVisible(false);
    connect(tabBar,SIGNAL(currentChanged(int)),this,SLOT(tabBarIndexChanged(int)));
// -----------

// --- Tabbar + hierarchy + OpenGl
    QVBoxLayout* vlay=new QVBoxLayout();
    vlay->addWidget(tabBar);
    vlay->addWidget(_sceneHierarchySplitter);
    vlay->setSpacing(0);
    vlay->setStretchFactor(tabBar,0);
    vlay->setStretchFactor(openglWidget,1);
    vlay->setContentsMargins(0,0,0,0);
    QWidget* tabHierarchyAndOpenGl=new QWidget();
    tabHierarchyAndOpenGl->setLayout(vlay);
// -----------

// --- Status bar ---
    statusBar=new CStatusBar();
    statusBar->setReadOnly(true);
    statusBar->setLineWrapMode(QPlainTextEdit::WidgetWidth);
    statusBar->setMaximumBlockCount(5000);
    statusBar->moveCursor(QTextCursor::End);
    statusBar->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    statusBar->setMinimumHeight(50);
    statusBar->setMaximumHeight(600);
    statusBar->setObjectName("statusBar");
    if ((App::operationalUIParts&sim_gui_statusbar)==0)
        statusBar->setVisible(false); // do not explicitely set to true (not nice artifacts during creation). Is true by default anyways
// ---------------

// --- statusbar splitter ---
    _statusbarSplitter=new QSplitter(Qt::Vertical);
    _statusbarSplitter->addWidget(tabHierarchyAndOpenGl);
    _statusbarSplitter->addWidget(statusBar);
    _statusbarSplitter->setCollapsible(0,false);
    _statusbarSplitter->setCollapsible(1,true);
    _statusbarSplitter->setOpaqueResize(false);
    QList<int> splitSizes;
    if (App::userSettings->statusbarInitiallyVisible)
        splitSizes << 10000 << 360;
    else
        splitSizes << 10000 << 0;
    _statusbarSplitter->setSizes(splitSizes);
    connect(_statusbarSplitter,SIGNAL(splitterMoved(int,int)),this,SLOT(statusbarSplitterMoved(int,int)));
// -----------------


// --- Browser splitter ---
    _modelBrowserSplitter=new QSplitter(Qt::Horizontal);
    _modelBrowserSplitter->addWidget(_modelBrowser);
    _modelBrowserSplitter->addWidget(_statusbarSplitter);
    _modelBrowserSplitter->setCollapsible(0,false);
    _modelBrowserSplitter->setOpaqueResize(false);
    splitSizes.clear();
    splitSizes << 1 << 10000;
    _modelBrowserSplitter->setSizes(splitSizes);
// -----------------

    setCentralWidget(_modelBrowserSplitter);



    _signalMapper=new QSignalMapper(this);
    connect(_signalMapper,SIGNAL(mapped(int)),this,SLOT(_simMessageHandler(int)));
    _popupSignalMapper=new QSignalMapper(this);
    connect(_popupSignalMapper,SIGNAL(mapped(int)),this,SLOT(_simPopupMessageHandler(int)));
    setAcceptDrops(true);
}

CMainWindow::~CMainWindow()
{
    TRACE_INTERNAL;
    removeDefaultMenuBar();
    disconnect(_popupSignalMapper);
    delete _popupSignalMapper;
    disconnect(_signalMapper);
    delete _signalMapper;
    delete simulationRecorder;
    delete dlgCont;
    delete customMenuBarItemContainer;

    ogl::freeOutlineFont();
    ogl::freeBitmapFonts();

    while (_sceneHierarchyWidgetList.size()!=0)
        instanceAboutToBeDestroyed(int(_sceneHierarchyWidgetList.size()-1));

    delete codeEditorContainer;
    delete oglSurface;
    delete editModeContainer;
}

void CMainWindow::setBrowserVisible(bool v)
{
    if (_modelBrowser!=nullptr)
        _modelBrowser->setVisible(v);
}

void CMainWindow::initializeWindow()
{
    createDefaultMenuBar();
    _createDefaultToolBars();
    _setInitialDimensions(true);
}

void CMainWindow::flashStatusbar()
{ // Call only from GUI
    if (statusBar!=nullptr)
    {
        statusBar->setStyleSheet("background-color: yellow");
        if (App::userSettings->darkMode)
            statusBar->verticalScrollBar()->setStyleSheet("background: transparent");
        else
            statusBar->verticalScrollBar()->setStyleSheet("background-color: white"); // since Qt 5.12.5 the scrollbar's color is not reverted with above command, but white
        _statusbarFlashTime=VDateTime::getTimeInMs();
    }
}

void CMainWindow::setFlyModeCameraHandle(int h)
{
    _flyModeCameraHandle=h;
}

int CMainWindow::getFlyModeCameraHandle()
{
    return(_flyModeCameraHandle);
}

void CMainWindow::setProxSensorClickSelectDown(int v)
{
    _proxSensorClickSelectDown=v;
}

int CMainWindow::getProxSensorClickSelectDown()
{
    return(_proxSensorClickSelectDown);
}

void CMainWindow::setProxSensorClickSelectUp(int v)
{
    _proxSensorClickSelectUp=v;
}

int CMainWindow::getProxSensorClickSelectUp()
{
    return(_proxSensorClickSelectUp);
}

void CMainWindow::_resetStatusbarFlashIfNeeded()
{ // Call only from GUI
    if (_statusbarFlashTime!=-1)
    {
        if (VDateTime::getTimeDiffInMs(_statusbarFlashTime)>1000)
        {
            if (App::userSettings->darkMode)
                statusBar->setStyleSheet("background: transparent");
            else
                statusBar->setStyleSheet("background-color: white");
            _statusbarFlashTime=-1;
        }
    }
}

bool CMainWindow::getObjectShiftToggleViaGuiEnabled()
{
    return(_toolbarButtonObjectShiftEnabled);
}

void CMainWindow::setObjectShiftToggleViaGuiEnabled(bool e)
{
    _toolbarButtonObjectShiftEnabled=e;
    _actualizetoolbarButtonState();
}

bool CMainWindow::getObjectRotateToggleViaGuiEnabled()
{
    return(_toolbarButtonObjectRotateEnabled);
}

void CMainWindow::setObjectRotateToggleViaGuiEnabled(bool e)
{
    _toolbarButtonObjectRotateEnabled=e;
    _actualizetoolbarButtonState();
}

void CMainWindow::setHierarchyToggleViaGuiEnabled(bool e)
{
    _toolbarButtonHierarchyEnabled=e;
    _actualizetoolbarButtonState();
}

bool CMainWindow::getHierarchyToggleViaGuiEnabled()
{
    return(_toolbarButtonHierarchyEnabled);
}

void CMainWindow::setBrowserToggleViaGuiEnabled(bool e)
{
    _toolbarButtonBrowserEnabled=e;
    _actualizetoolbarButtonState();
}

bool CMainWindow::getBrowserToggleViaGuiEnabled()
{
    return(_toolbarButtonBrowserEnabled);
}

void CMainWindow::setObjPropToggleViaGuiEnabled(bool e)
{
    _toolbarButtonObjPropEnabled=e;
    _actualizetoolbarButtonState();
}

bool CMainWindow::getObjPropToggleViaGuiEnabled()
{
    return(_toolbarButtonObjPropEnabled);
}

void CMainWindow::setCalcModulesToggleViaGuiEnabled(bool e)
{
    _toolbarButtonCalcModulesEnabled=e;
    _actualizetoolbarButtonState();
}

bool CMainWindow::getCalcModulesToggleViaGuiEnabled()
{
    return(_toolbarButtonCalcModulesEnabled);
}

void CMainWindow::setPlayViaGuiEnabled(bool e)
{
    _toolbarButtonPlayEnabled=e;
    _actualizetoolbarButtonState();
}

bool CMainWindow::getPlayViaGuiEnabled()
{
    return(_toolbarButtonPlayEnabled);
}

void CMainWindow::setPauseViaGuiEnabled(bool e)
{
    _toolbarButtonPauseEnabled=e;
    _actualizetoolbarButtonState();
}

bool CMainWindow::getPauseViaGuiEnabled()
{
    return(_toolbarButtonPauseEnabled);
}

void CMainWindow::setStopViaGuiEnabled(bool e)
{
    _toolbarButtonStopEnabled=e;
    _actualizetoolbarButtonState();
}

bool CMainWindow::getStopViaGuiEnabled()
{
    return(_toolbarButtonStopEnabled);
}

bool CMainWindow::isFullScreen()
{
    return(_fullscreen);
}

void CMainWindow::setFullScreen(bool f)
{
    if (_fullscreen!=f)
    {
        if (VThread::isCurrentThreadTheUiThread())
        { // we are in the UI thread. We execute the command now:
            if (f)
            {
                openglWidget->setParent(nullptr);
                openglWidget->showFullScreen();
                // Following is somehow needed. The generated resize event from showFullScreen provides previous window size values..
                windowResizeEvent(openglWidget->size().width(),openglWidget->size().height());
            }
            else
            {
                openglWidget->showNormal();
                _sceneHierarchySplitter->insertWidget(1,openglWidget);
                windowResizeEvent(width(),height());
            }
        }
        else
        { // We are NOT in the UI thread. We execute the command via the UI thread:
            SUIThreadCommand cmdIn;
            SUIThreadCommand cmdOut;
            cmdIn.cmdId=MAIN_WINDOW_SET_FULLSCREEN_MWTHREADCMD;
            cmdIn.boolParams.push_back(f);
            App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
        }
    }
    _fullscreen=f;
}

void CMainWindow::setOpenGlDisplayEnabled(bool e)
{
    _openGlDisplayEnabled=e;
    App::setToolbarRefreshFlag();
    App::setRefreshHierarchyViewFlag();
}

bool CMainWindow::getOpenGlDisplayEnabled()
{
    return(_openGlDisplayEnabled);
}

void CMainWindow::setFocusObject(int f)
{
    _focusObject=f;
}

bool CMainWindow::getHasStereo()
{
    return(_hasStereo);
}

void CMainWindow::setStereoDistance(float d)
{
    _stereoDistance=d;
}

float CMainWindow::getStereoDistance()
{
    return(_stereoDistance);
}

bool CMainWindow::getLeftEye()
{
    return(_leftEye);
}


int CMainWindow::getMouseButtonState()
{ // 1=left, 2=wheel activity, 4=right, 8=middle wheel down
    return(_mouseButtonsState);
}

void CMainWindow::setMouseButtonState(int state)
{ // 1=left, 2=wheel activity, 4=right, 8=middle wheel down
    _mouseButtonsState=state;
}

int CMainWindow::getKeyDownState()
{ // 1=ctrl, 2=shift, 4=up, 8=down, 16=left, 32=right
    return(_keyDownState);
}

void CMainWindow::setKeyDownState(int state)
{ // 1=ctrl, 2=shift, 4=up, 8=down, 16=left, 32=right
    _keyDownState=state;
}

QSignalMapper* CMainWindow::getPopupSignalMapper()
{
    return(_popupSignalMapper);
}

void CMainWindow::_setInitialDimensions(bool maximized)
{
    setWindowDimensions(1024,768,true,maximized);
}

void CMainWindow::_setClientArea(int x,int y)
{
    _clientArea.x=x;
    _clientArea.y=y;
    simulationRecorder->setRecordingSizeChanged(x,y);
}

void CMainWindow::windowResizeEvent(int x,int y)
{
    _setClientArea(x,y);
    _recomputeClientSizeAndPos();
    _renderOpenGlContent_callFromRenderingThreadOnly();
}

void CMainWindow::setWindowDimensions(int x,int y,bool clientSurface,bool maximized)
{
    if (!maximized)
        showNormal();
    int frameWidth=0;
    int frameHeight=0;
    if (!clientSurface)
    {
        frameWidth=frameGeometry().width()-size().width();
        frameHeight=frameGeometry().height()-size().height();
    }
    x-=frameWidth;
    y-=frameHeight;
    resize(x,y);
    if (maximized)
        showMaximized();
}

void CMainWindow::refreshDimensions()
{
    _recomputeClientSizeAndPos();
}

void CMainWindow::simThread_prepareToRenderScene()
{
    TRACE_INTERNAL;
    // Following is for camera tracking (when simulation is stopped). If sim is running, it is handled in simHandleVarious!
    if (App::currentWorld->simulation->isSimulationStopped())
    {
        for (size_t i=0;i<App::currentWorld->sceneObjects->getCameraCount();i++)
        {
            CCamera*  it=App::currentWorld->sceneObjects->getCameraFromIndex(i);
            it->handleTrackingAndHeadAlwaysUp();
        }
    }
    // Following is for dummy position assignment to path trajectory when not simulating (and not pausing):
    if (App::currentWorld->simulation->isSimulationStopped())
    {
        for (size_t i=0;i<App::currentWorld->sceneObjects->getPathCount();i++)
        {
            CPath* it=App::currentWorld->sceneObjects->getPathFromIndex(i);
            it->resetPath();
        }
    }
}



void CMainWindow::uiThread_renderScene()
{
    TRACE_INTERNAL;
    IF_UI_EVENT_CAN_READ_DATA
    { // ok, we are allowed to render (i.e. the simulation thread doesn't modify anything serious)
        uiThread_renderScene_noLock();
    }
}

void CMainWindow::uiThread_renderScene_noLock()
{
    TRACE_INTERNAL;
    App::worldContainer->calcInfo->clearRenderingTime();
    App::worldContainer->calcInfo->renderingStart();
    _renderOpenGlContent_callFromRenderingThreadOnly();
    App::worldContainer->calcInfo->renderingEnd();
}

void CMainWindow::callDialogFunction(const SUIThreadCommand* cmdIn,SUIThreadCommand* cmdOut)
{
    dlgCont->callDialogFunction(cmdIn,cmdOut);
}

void CMainWindow::refreshDialogs_uiThread()
{
    void* returnVal=CPluginContainer::sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_guipass,nullptr,nullptr,nullptr);
    delete[] (char*)returnVal;

    _resetStatusbarFlashIfNeeded();

    // We refresh dialogs and the toolbar here:
    //----------------------------------------------------------------------------------
    if (App::worldContainer->getCurrentWorldIndex()!=lastInstance)
    {
        App::setFullDialogRefreshFlag();
        App::setToolbarRefreshFlag();
        createDefaultMenuBar();
        lastInstance=App::worldContainer->getCurrentWorldIndex();
        refreshDimensions();
        // If the instance was switched, we close all material/color dialogs:
        dlgCont->visibleInstanceAboutToSwitch();
    }

    dlgCont->destroyWhatNeedsDestruction();

    if (_lightDialogRefreshFlag||_fullDialogRefreshFlag)
    {
        if (!_dialogRefreshDontPublishFlag)
        {
            int data[4]={_fullDialogRefreshFlag?2:0,0,0,0};
            void* returnVal=CPluginContainer::sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_refreshdialogs,data,nullptr,nullptr);
            delete[] (char*)returnVal;
        }
        _dialogRefreshDontPublishFlag=false;
        dlgCont->refresh();
        if (_fullDialogRefreshFlag)
            App::setRebuildHierarchyFlag();
        App::setRefreshHierarchyViewFlag();
        _lightDialogRefreshFlag=false;
        _fullDialogRefreshFlag=false;
    }

    if (_toolbarRefreshFlag)
        _actualizetoolbarButtonState();
    _toolbarRefreshFlag=false;
    //----------------------------------------------------------------------------------

#ifndef LIN_SIM
    // setWindowTitle somehow adds multiple app icons on Linux...
    int ct=VDateTime::getTimeInMs();
    if ( (VDateTime::getTimeDiffInMs(timeCounter)>1000)||((VDateTime::getTimeDiffInMs(timeCounter)>100)&&(!getOpenGlDisplayEnabled())) )
    { // Refresh the main window text every 1/4 seconds:
        timeCounter=ct;
        std::string title;
        if (getOpenGlDisplayEnabled())
        {
            title=(IDS____RENDERING__);
            title+=boost::lexical_cast<std::string>(_renderingTimeInMs)+" ms";
        }
        else
        { // We display the simulation time instead:
            title=IDS____SIMULATION_TIME__;
            title+=gv::getHourMinuteSecondMilisecondStr(float(App::currentWorld->simulation->getSimulationTime_ns())/1000000.0f);
        }
        title+=" (";
        title+=tt::FNb(0,_fps,1,false);
        title+=" fps)";
        if (editModeContainer->getEditModeType()&TRIANGLE_EDIT_MODE)
            title+=IDS____TRIANGLE_EDIT_MODE;
        else if (editModeContainer->getEditModeType()&VERTEX_EDIT_MODE)
            title+=IDS____VERTEX_EDIT_MODE;
        else if (editModeContainer->getEditModeType()&EDGE_EDIT_MODE)
            title+=IDS____EDGE_EDIT_MODE;
        else if (editModeContainer->getEditModeType()&PATH_EDIT_MODE)
            title+=IDS____PATH_EDIT_MODE;

        if (editModeContainer->getEditModeType()==NO_EDIT_MODE)
        {
            if (App::currentWorld->simulation->isSimulationRunning())
                title+=IDS____SIMULATION_RUNNING;
            else if (App::currentWorld->simulation->isSimulationPaused())
                title+=IDS____SIMULATION_PAUSED;
            else
                title+=IDS____SIMULATION_STOPPED;
        }

        if (App::currentWorld->mainSettings->getScenePathAndName().compare("")==0)
            title=std::string(IDS_NEW_FILE)+title;
        else
            title=App::currentWorld->mainSettings->getSceneName()+title;
        title=App::getApplicationName()+" - "+title;
        setWindowTitle(title.c_str());
    }
#endif

    if (VDateTime::getTimeDiffInMs(_mouseWheelEventTime)>300)
        _mouseButtonsState&=0xffff-2; // We clear the mouse wheel event
}

int CMainWindow::_renderOpenGlContent_callFromRenderingThreadOnly()
{ // Called only from the rendering thread!!!
    TRACE_INTERNAL;
    int startTime=VDateTime::getTimeInMs();
    _fps=1.0f/(float(VDateTime::getTimeDiffInMs(lastTimeRenderingStarted,startTime))/1000.0f);
    lastTimeRenderingStarted=startTime;

    if (_fullDialogRefreshFlag)
        App::setRebuildHierarchyFlag();
    if (_lightDialogRefreshFlag)
        App::setRefreshHierarchyViewFlag();

    bool swapTheBuffers=false;
    if (!windowHandle()->isExposed())
        return(0);

    if ( (!getOpenGlDisplayEnabled())&&(App::currentWorld->simulation!=nullptr)&&(App::currentWorld->simulation->isSimulationStopped()) )
        setOpenGlDisplayEnabled(true);

    if (getOpenGlDisplayEnabled())
    {
        swapTheBuffers=true;
        int oglDebugTime=startTime;
        openglWidget->makeContextCurrent();
        if (App::userSettings->debugOpenGl)
        {
            int oglDebugTimeNow=VDateTime::getTimeInMs();
            App::logMsg(sim_verbosity_debug,"openGl debug --> doneCurrent + makeCurrent: %i",VDateTime::getTimeDiffInMs(oglDebugTime,oglDebugTimeNow));
            oglDebugTime=oglDebugTimeNow;
        }

        int mp[2]={_mouseRenderingPos.x,_mouseRenderingPos.y};

        CPluginContainer::sendSpecialEventCallbackMessageToSomePlugins(sim_message_eventcallback_renderingpass,nullptr,nullptr,nullptr);

        if (!_hasStereo)
            oglSurface->render(_currentCursor,_mouseButtonsState,mp,nullptr);
        else
        {
            _leftEye=true;
            glDrawBuffer(GL_BACK_LEFT);
            oglSurface->render(_currentCursor,_mouseButtonsState,mp,nullptr);
            _leftEye=false;
            glDrawBuffer(GL_BACK_RIGHT);
            oglSurface->render(_currentCursor,_mouseButtonsState,mp,nullptr);
        }

        if (App::userSettings->debugOpenGl)
        {
            int oglDebugTimeNow=VDateTime::getTimeInMs();
            App::logMsg(sim_verbosity_debug,"openGl debug --> sendEventCallbackMessageToAllPlugins + render: %i",VDateTime::getTimeDiffInMs(oglDebugTime,oglDebugTimeNow));
            oglDebugTime=oglDebugTimeNow;
        }

        previousDisplayWasEnabled=0;
        if (App::userSettings->useGlFinish) // false by default!
            glFinish(); // Might be important later (synchronization problems)
                    // removed on 2009/12/09 upon recomendation of gamedev community
                    // re-put on 2010/01/11 because it slows down some graphic cards in a non-proportional way (e.g. 1 object=x ms, 5 objects=20x ms)
                    // re-removed again (by default) on 31/01/2013. Thanks a lot to Cedric Pradalier for pointing problems appearing with the NVidia drivers

        if (App::userSettings->debugOpenGl)
        {
            int oglDebugTimeNow=VDateTime::getTimeInMs();
            App::logMsg(sim_verbosity_debug,"openGl debug --> glFinish (%i, %i): %i",App::userSettings->useGlFinish,App::userSettings->vsync,VDateTime::getTimeDiffInMs(oglDebugTime,oglDebugTimeNow));
            App::logMsg(sim_verbosity_debug,"openGl debug --> VENDOR:%s",(char*)glGetString(GL_VENDOR));
            App::logMsg(sim_verbosity_debug,"openGl debug --> RENDERER:%s",(char*)glGetString(GL_RENDERER));
            App::logMsg(sim_verbosity_debug,"openGl debug --> VERSION:%s",(char*)glGetString(GL_VERSION));
            oglDebugTime=oglDebugTimeNow;
        }
    }
    else
    {
        if (previousDisplayWasEnabled<2)
        { // clear the screen
            // We draw a dark grey view:

            swapTheBuffers=true;
            openglWidget->makeContextCurrent();
            glDisable(GL_SCISSOR_TEST);
            glViewport(-2000,-2000,4000,4000);
            glClearColor(0.0f,0.0f,0.0f,1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            if (App::userSettings->useGlFinish) // false by default!
                glFinish(); // Might be important later (synchronization problems)
                        // removed on 2009/12/09 upon recomendation of gamedev community
                        // re-put on 2010/01/11 because it slows down some graphic cards in a non-proportional way (e.g. 1 object=x ms, 5 objects=20x ms)
                        // re-removed again (by default) on 31/01/2013. Thanks a lot to Cedric Pradalier for pointing problems appearing with the NVidia drivers

        }
        if (previousDisplayWasEnabled<2)
            previousDisplayWasEnabled++;
    }

    if (simulationRecorder->getIsRecording())
        simulationRecorder->recordFrameIfNeeded(_clientArea.x,_clientArea.y,0,0);

    if (swapTheBuffers) // condition added on 31/1/2012... might help because some VMWare installations crash when disabling the rendering
    {
        int oglDebugTime=VDateTime::getTimeInMs();
        // the only time in the whole application (except for COpenglWidget::paintGL() ) where we can call
        // this command, otherwise we have problems with some graphic cards and VMWare on MAC:
        #ifndef USING_QOPENGLWIDGET
            openglWidget->swapBuffers();
        #else
            openglWidget->update();
        #endif
        if (App::userSettings->debugOpenGl)
        {
            int oglDebugTimeNow=VDateTime::getTimeInMs();
            App::logMsg(sim_verbosity_debug,"openGl debug --> swapBuffers: %i",VDateTime::getTimeDiffInMs(oglDebugTime,oglDebugTimeNow));
            oglDebugTime=oglDebugTimeNow;
        }

        openglWidget->doneCurrent();

        if (App::userSettings->debugOpenGl)
        {
            int oglDebugTimeNow=VDateTime::getTimeInMs();
            App::logMsg(sim_verbosity_debug,"openGl debug --> doneCurrent: %i",VDateTime::getTimeDiffInMs(oglDebugTime,oglDebugTimeNow));
            oglDebugTime=oglDebugTimeNow;
        }
    }

    _renderingTimeInMs=VDateTime::getTimeDiffInMs(startTime);
    return(_renderingTimeInMs);
}


void CMainWindow::createDefaultMenuBar()
{
    if (App::operationalUIParts&sim_gui_menubar)
    { // Default menu bar
        removeDefaultMenuBar();
        _menubar=new VMenubar();

        bool menuBarEnabled=true;
        if (oglSurface->isViewSelectionActive()||oglSurface->isPageSelectionActive())
            menuBarEnabled=false;

        // Since Qt5, Mac MenuBars don't have separators anymore... this is a quick and dirty workaround:
        #define DUMMY_SPACE_QMENUBAR_QT5 ""
        #ifdef MAC_SIM
            #undef DUMMY_SPACE_QMENUBAR_QT5
            #define DUMMY_SPACE_QMENUBAR_QT5 "    "
        #endif

        if (editModeContainer->getEditModeType()==NO_EDIT_MODE)
        {
            _fileSystemMenu=new VMenu();
            _menubar->appendMenuAndDetach(_fileSystemMenu,menuBarEnabled,(std::string(IDS_FILE_MENU_ITEM)+DUMMY_SPACE_QMENUBAR_QT5).c_str());
            connect(_fileSystemMenu->getQMenu(),SIGNAL(aboutToShow()),this,SLOT(_aboutToShowFileSystemMenu()));
        }

        if (editModeContainer->getEditModeType()!=MULTISHAPE_EDIT_MODE)
        {
            _editSystemMenu=new VMenu();
            _menubar->appendMenuAndDetach(_editSystemMenu,menuBarEnabled,(std::string(IDS_EDIT_MENU_ITEM)+DUMMY_SPACE_QMENUBAR_QT5).c_str());
            connect(_editSystemMenu->getQMenu(),SIGNAL(aboutToShow()),this,SLOT(_aboutToShowEditSystemMenu()));
        }

        if (editModeContainer->getEditModeType()==NO_EDIT_MODE)
        {
            if (CSimFlavor::getBoolVal(11))
            {
                _addSystemMenu=new VMenu();
                _menubar->appendMenuAndDetach(_addSystemMenu,menuBarEnabled,(std::string(IDS_ADD_MENU_ITEM)+DUMMY_SPACE_QMENUBAR_QT5).c_str());
                connect(_addSystemMenu->getQMenu(),SIGNAL(aboutToShow()),this,SLOT(_aboutToShowAddSystemMenu()));
            }
            _simulationSystemMenu=new VMenu();
            _menubar->appendMenuAndDetach(_simulationSystemMenu,menuBarEnabled,(std::string(IDS_SIMULATION_MENU_ITEM)+DUMMY_SPACE_QMENUBAR_QT5).c_str());
            connect(_simulationSystemMenu->getQMenu(),SIGNAL(aboutToShow()),this,SLOT(_aboutToShowSimulationSystemMenu()));
        }

        if (editModeContainer->getEditModeType()==NO_EDIT_MODE)
        {
            _windowSystemMenu=new VMenu();
            _menubar->appendMenuAndDetach(_windowSystemMenu,menuBarEnabled,(std::string(IDS_TOOLS_MENU_ITEM)+DUMMY_SPACE_QMENUBAR_QT5).c_str());
            connect(_windowSystemMenu->getQMenu(),SIGNAL(aboutToShow()),this,SLOT(_aboutToShowWindowSystemMenu()));

            if (customMenuBarItemContainer->allItems.size()!=0)
            { // Plugins
                customMenuBarItemContainer->_menuHandle=new VMenu();
                _menubar->appendMenuAndDetach(customMenuBarItemContainer->_menuHandle,menuBarEnabled,(std::string(IDS_MODULES_MENU_ITEM)+DUMMY_SPACE_QMENUBAR_QT5).c_str());
                connect(customMenuBarItemContainer->_menuHandle->getQMenu(),SIGNAL(aboutToShow()),this,SLOT(_aboutToShowCustomMenu()));
            }

            if (CSimFlavor::getBoolVal(11))
            {
                _addOnSystemMenu=new VMenu();
                _menubar->appendMenuAndDetach(_addOnSystemMenu,menuBarEnabled,(std::string(IDS_ADDON_MENU_ITEM)+DUMMY_SPACE_QMENUBAR_QT5).c_str());
                connect(_addOnSystemMenu->getQMenu(),SIGNAL(aboutToShow()),this,SLOT(_aboutToShowAddOnSystemMenu()));
            }

            {
                _instancesSystemMenu=new VMenu();
                _menubar->appendMenuAndDetach(_instancesSystemMenu,menuBarEnabled,(std::string(IDS_INSTANCES_MENU_ITEM)+DUMMY_SPACE_QMENUBAR_QT5).c_str());
                connect(_instancesSystemMenu->getQMenu(),SIGNAL(aboutToShow()),this,SLOT(_aboutToShowInstancesSystemMenu()));
            }

            if (CSimFlavor::getBoolVal_int(0,App::userSettings->xrTest))
            {
                _layoutSystemMenu=new VMenu();
                _menubar->appendMenuAndDetach(_layoutSystemMenu,menuBarEnabled,(std::string(IDS_LAYOUT_MENU_ITEM)+DUMMY_SPACE_QMENUBAR_QT5).c_str());
                connect(_layoutSystemMenu->getQMenu(),SIGNAL(aboutToShow()),this,SLOT(_aboutToShowLayoutSystemMenu()));
            }

            if (CSimFlavor::getBoolVal_int(0,App::userSettings->xrTest))
            {
                _jobsSystemMenu=new VMenu();
                _menubar->appendMenuAndDetach(_jobsSystemMenu,menuBarEnabled,(std::string(IDS_JOBS_MENU_ITEM)+DUMMY_SPACE_QMENUBAR_QT5).c_str());
                connect(_jobsSystemMenu->getQMenu(),SIGNAL(aboutToShow()),this,SLOT(_aboutToShowJobsSystemMenu()));
            }
        }

        if (editModeContainer->getEditModeType()==NO_EDIT_MODE)
        {
            _helpSystemMenu=new VMenu();
            _menubar->appendMenuAndDetach(_helpSystemMenu,menuBarEnabled,(std::string(IDS_HELP_MENU_ITEM)+DUMMY_SPACE_QMENUBAR_QT5).c_str());
            connect(_helpSystemMenu->getQMenu(),SIGNAL(aboutToShow()),this,SLOT(_aboutToShowHelpSystemMenu()));
        }
    }
    refreshDimensions();
}

void CMainWindow::removeDefaultMenuBar()
{
    if (_menubar!=nullptr) //App::operationalUIParts&sim_gui_menubar)
    {
        delete _menubar;
        _menubar=nullptr;
        refreshDimensions();
    }
}

void CMainWindow::_createDefaultToolBars()
{
    if ((_toolbar1==nullptr)&&(App::operationalUIParts&sim_gui_toolbar1))
    {
        _toolbar1=new QToolBar(tr("Navigation"));
        _toolbar1->setIconSize(QSize(28,28));
        _toolbar1->setAllowedAreas(Qt::TopToolBarArea|Qt::BottomToolBarArea);
        addToolBar(Qt::TopToolBarArea,_toolbar1);
        #ifdef MAC_SIM
            _toolbar1->setMovable(false); // 14/7/2013: since Qt5.1.0 the toolbar looks just plain white when undocked under MacOS
        #endif

        _toolbarActionCameraShift=_toolbar1->addAction(QIcon(":/toolbarFiles/cameraShift.png"),tr(IDS_TOOLBAR_TOOLTIP_CAMERA_SHIFT));
        _toolbarActionCameraShift->setCheckable(true);
        connect(_toolbarActionCameraShift,SIGNAL(triggered()),_signalMapper,SLOT(map()));
        _signalMapper->setMapping(_toolbarActionCameraShift,CAMERA_SHIFT_NAVIGATION_CMD);

        _toolbarActionCameraRotate=_toolbar1->addAction(QIcon(":/toolbarFiles/cameraRotate.png"),tr(IDS_TOOLBAR_TOOLTIP_CAMERA_ROTATE));
        _toolbarActionCameraRotate->setCheckable(true);
        connect(_toolbarActionCameraRotate,SIGNAL(triggered()),_signalMapper,SLOT(map()));
        _signalMapper->setMapping(_toolbarActionCameraRotate,CAMERA_ROTATE_NAVIGATION_CMD);

        _toolbarActionCameraZoom=_toolbar1->addAction(QIcon(":/toolbarFiles/cameraZoom.png"),tr(IDS_TOOLBAR_TOOLTIP_CAMERA_ZOOM));
        _toolbarActionCameraZoom->setCheckable(true);
        connect(_toolbarActionCameraZoom,SIGNAL(triggered()),_signalMapper,SLOT(map()));
        _signalMapper->setMapping(_toolbarActionCameraZoom,CAMERA_ZOOM_NAVIGATION_CMD);

        if (CSimFlavor::getBoolVal(11))
        {
            _toolbarActionCameraAngle=_toolbar1->addAction(QIcon(":/toolbarFiles/cameraAngle.png"),tr(IDS_TOOLBAR_TOOLTIP_CAMERA_OPENING_ANGLE));
            _toolbarActionCameraAngle->setCheckable(true);
            connect(_toolbarActionCameraAngle,SIGNAL(triggered()),_signalMapper,SLOT(map()));
            _signalMapper->setMapping(_toolbarActionCameraAngle,CAMERA_ANGLE_NAVIGATION_CMD);
        }

        _toolbarActionCameraSizeToScreen=_toolbar1->addAction(QIcon(":/toolbarFiles/cameraResize.png"),tr(IDS_TOOLBAR_TOOLTIP_CAMERA_FIT_TO_SCREEN));
        _toolbarActionCameraSizeToScreen->setCheckable(false);
        connect(_toolbarActionCameraSizeToScreen,SIGNAL(triggered()),_signalMapper,SLOT(map()));
        _signalMapper->setMapping(_toolbarActionCameraSizeToScreen,CAMERA_SHIFT_TO_FRAME_SELECTION_CMD);

/*
        _toolbarActionCameraFly=_toolbar1->addAction(QIcon(":/toolbarFiles/cameraFly.png"),tr(IDS_TOOLBAR_TOOLTIP_CAMERA_FLY));
        _toolbarActionCameraFly->setCheckable(true);
        connect(_toolbarActionCameraFly,SIGNAL(triggered()),_signalMapper,SLOT(map()));
        _signalMapper->setMapping(_toolbarActionCameraFly,CAMERA_FLY_NAVIGATION_CMD);
        */

        _toolbar1->addSeparator();

        if (CSimFlavor::getBoolVal(11))
        {
            _toolbarActionClickSelection=_toolbar1->addAction(QIcon(":/toolbarFiles/clickSelection.png"),tr(IDS_TOOLBAR_TOOLTIP_CLICK_SELECTION));
            _toolbarActionClickSelection->setCheckable(true);
            connect(_toolbarActionClickSelection,SIGNAL(triggered()),_signalMapper,SLOT(map()));
            _signalMapper->setMapping(_toolbarActionClickSelection,OBJECT_SELECTION_SELECTION_CMD);
        }

        _toolbarActionObjectShift=_toolbar1->addAction(QIcon(":/toolbarFiles/objectShift.png"),tr(IDS_TOOLBAR_TOOLTIP_OBJECT_SHIFT));
        _toolbarActionObjectShift->setCheckable(true);
        connect(_toolbarActionObjectShift,SIGNAL(triggered()),_signalMapper,SLOT(map()));
        _signalMapper->setMapping(_toolbarActionObjectShift,OBJECT_SHIFT_NAVIGATION_CMD);

        _toolbarActionObjectRotate=_toolbar1->addAction(QIcon(":/toolbarFiles/objectRotate.png"),tr(IDS_TOOLBAR_TOOLTIP_OBJECT_ROTATE));
        _toolbarActionObjectRotate->setCheckable(true);
        connect(_toolbarActionObjectRotate,SIGNAL(triggered()),_signalMapper,SLOT(map()));
        _signalMapper->setMapping(_toolbarActionObjectRotate,OBJECT_ROTATE_NAVIGATION_CMD);

        _toolbar1->addSeparator();

        _toolbarActionAssemble=_toolbar1->addAction(QIcon(":/toolbarFiles/assemble.png"),tr(IDSN_ASSEMBLE));
        _toolbarActionAssemble->setCheckable(false);
        connect(_toolbarActionAssemble,SIGNAL(triggered()),_signalMapper,SLOT(map()));
        _signalMapper->setMapping(_toolbarActionAssemble,SCENE_OBJECT_OPERATION_ASSEMBLE_SOOCMD);

        if (CSimFlavor::getBoolVal(11))
        {
            _toolbarActionTransferDna=_toolbar1->addAction(QIcon(":/toolbarFiles/transferDna.png"),tr(IDSN_TRANSFER_DNA));
            _toolbarActionTransferDna->setCheckable(false);
            connect(_toolbarActionTransferDna,SIGNAL(triggered()),_signalMapper,SLOT(map()));
            _signalMapper->setMapping(_toolbarActionTransferDna,SCENE_OBJECT_OPERATION_TRANSFER_DNA_SOOCMD);
        }

        _toolbar1->addSeparator();

        _toolbarActionUndo=_toolbar1->addAction(QIcon(":/toolbarFiles/undo.png"),tr(IDSN_UNDO));
        _toolbarActionUndo->setCheckable(false);
        connect(_toolbarActionUndo,SIGNAL(triggered()),_signalMapper,SLOT(map()));
        _signalMapper->setMapping(_toolbarActionUndo,SCENE_OBJECT_OPERATION_UNDO_SOOCMD);

        _toolbarActionRedo=_toolbar1->addAction(QIcon(":/toolbarFiles/redo.png"),tr(IDSN_REDO));
        _toolbarActionRedo->setCheckable(false);
        connect(_toolbarActionRedo,SIGNAL(triggered()),_signalMapper,SLOT(map()));
        _signalMapper->setMapping(_toolbarActionRedo,SCENE_OBJECT_OPERATION_REDO_SOOCMD);
        _toolbar1->addSeparator();

        if (CSimFlavor::getBoolVal_int(0,App::userSettings->xrTest))
        {
            _toolbarActionVerify=_toolbar1->addAction(QIcon(":/toolbarFiles/verify.png"),tr(IDS_TOOLBAR_TOOLTIP_VERIFY));
            _toolbarActionVerify->setCheckable(false);
            connect(_toolbarActionVerify,SIGNAL(triggered()),_signalMapper,SLOT(map()));
            _signalMapper->setMapping(_toolbarActionVerify,XR_COMMAND_1_SCCMD+11);
        }

        if (CSimFlavor::getBoolVal(11))
        {
            _toolbarActionDynamicContentVisualization=_toolbar1->addAction(QIcon(":/toolbarFiles/dynamics.png"),tr(IDS_TOOLBAR_TOOLTIP_VISUALIZE_DYNAMIC_CONTENT));
            _toolbarActionDynamicContentVisualization->setCheckable(true);
            connect(_toolbarActionDynamicContentVisualization,SIGNAL(triggered()),_signalMapper,SLOT(map()));
            _signalMapper->setMapping(_toolbarActionDynamicContentVisualization,SIMULATION_COMMANDS_TOGGLE_DYNAMIC_CONTENT_VISUALIZATION_SCCMD);
        }

        _engineSelectCombo=new QComboBox();

        #ifdef WIN_SIM
            _engineSelectCombo->setMinimumWidth(90);
            _engineSelectCombo->setMaximumWidth(90);
            _engineSelectCombo->setMinimumHeight(24);
            _engineSelectCombo->setMaximumHeight(24);
        #endif
        #ifdef MAC_SIM
            _engineSelectCombo->setMinimumWidth(95);
            _engineSelectCombo->setMaximumWidth(95);
            _engineSelectCombo->setMinimumHeight(24);
            _engineSelectCombo->setMaximumHeight(24);
        #endif
        #ifdef LIN_SIM
            _engineSelectCombo->setMinimumWidth(90);
            _engineSelectCombo->setMaximumWidth(90);
            _engineSelectCombo->setMinimumHeight(24);
            _engineSelectCombo->setMaximumHeight(24);
        #endif

        _engineSelectCombo->addItem(tr(IDS_BULLET_2_78));
        _engineSelectCombo->addItem(tr(IDS_BULLET_2_83));
        _engineSelectCombo->addItem(tr(IDS_ODE));
        _engineSelectCombo->addItem(tr(IDS_VORTEX));
        _engineSelectCombo->addItem(tr(IDS_NEWTON));
        _engineSelectCombo->setToolTip(IDS_TOOLBAR_TOOLTIP_DYNAMICS_ENGINE);
        _toolbar1->addWidget(_engineSelectCombo);
        connect(_engineSelectCombo,SIGNAL(activated(int)),this,SLOT(_engineSelectedViaToolbar(int)));

        if (CSimFlavor::getBoolVal(11))
        {
            _enginePrecisionCombo=new QComboBox();

            #ifdef WIN_SIM
                _enginePrecisionCombo->setMinimumWidth(120);
                _enginePrecisionCombo->setMaximumWidth(120);
                _enginePrecisionCombo->setMinimumHeight(24);
                _enginePrecisionCombo->setMaximumHeight(24);
            #endif
            #ifdef MAC_SIM
                _enginePrecisionCombo->setMinimumWidth(125);
                _enginePrecisionCombo->setMaximumWidth(125);
                _enginePrecisionCombo->setMinimumHeight(24);
                _enginePrecisionCombo->setMaximumHeight(24);
            #endif
            #ifdef LIN_SIM
                _enginePrecisionCombo->setMinimumWidth(120);
                _enginePrecisionCombo->setMaximumWidth(120);
                _enginePrecisionCombo->setMinimumHeight(24);
                _enginePrecisionCombo->setMaximumHeight(24);
            #endif

            _enginePrecisionCombo->addItem(tr(IDSN_ENGINE_VERY_PRECISE));
            _enginePrecisionCombo->addItem(tr(IDSN_ENGINE_PRECISE));
            _enginePrecisionCombo->addItem(tr(IDSN_ENGINE_FAST));
            _enginePrecisionCombo->addItem(tr(IDSN_ENGINE_VERY_FAST));
            _enginePrecisionCombo->addItem(tr(IDSN_ENGINE_CUSTOM));
            _enginePrecisionCombo->setToolTip(IDS_TOOLBAR_TOOLTIP_DYNAMICS_SETTINGS);
            _toolbar1->addWidget(_enginePrecisionCombo);
            connect(_enginePrecisionCombo,SIGNAL(activated(int)),this,SLOT(_enginePrecisionViaToolbar(int)));
        }

        if (CSimFlavor::getBoolVal(11))
        {
            _timeStepConfigCombo=new QComboBox();

            #ifdef WIN_SIM
                _timeStepConfigCombo->setMinimumWidth(135);
                _timeStepConfigCombo->setMaximumWidth(135);
                _timeStepConfigCombo->setMinimumHeight(24);
                _timeStepConfigCombo->setMaximumHeight(24);
            #endif
            #ifdef MAC_SIM
                _timeStepConfigCombo->setMinimumWidth(140);
                _timeStepConfigCombo->setMaximumWidth(140);
                _timeStepConfigCombo->setMinimumHeight(24);
                _timeStepConfigCombo->setMaximumHeight(24);
            #endif
            #ifdef LIN_SIM
                _timeStepConfigCombo->setMinimumWidth(135);
                _timeStepConfigCombo->setMaximumWidth(135);
                _timeStepConfigCombo->setMinimumHeight(24);
                _timeStepConfigCombo->setMaximumHeight(24);
            #endif

            _timeStepConfigCombo->addItem(tr(IDSN_TIME_STEP_CONFIG_200));
            _timeStepConfigCombo->addItem(tr(IDSN_TIME_STEP_CONFIG_100));
            _timeStepConfigCombo->addItem(tr(IDSN_TIME_STEP_CONFIG_50));
            _timeStepConfigCombo->addItem(tr(IDSN_TIME_STEP_CONFIG_25));
            _timeStepConfigCombo->addItem(tr(IDSN_TIME_STEP_CONFIG_10));

            float dt=(float(App::currentWorld->simulation->getSimulationTimeStep_speedModified_ns(5))/1000.0f);
            std::string txt("dt=");
            txt+=tt::FNb(0,dt,1,false);
            txt+=IDSN_TIME_STEP_CONFIG_CUSTOM;
            _timeStepConfigCombo->addItem(txt.c_str());

            _timeStepConfigCombo->setToolTip(IDS_TOOLBAR_TOOLTIP_SIMULATION_TIME_STEP);
            _toolbar1->addWidget(_timeStepConfigCombo);
            connect(_timeStepConfigCombo,SIGNAL(activated(int)),this,SLOT(_timeStepConfigViaToolbar(int)));
        }

        _toolbarActionStart=_toolbar1->addAction(QIcon(":/toolbarFiles/start.png"),tr(IDS_TOOLBAR_TOOLTIP_SIMULATION_START));
        _toolbarActionStart->setCheckable(true);
        connect(_toolbarActionStart,SIGNAL(triggered()),_signalMapper,SLOT(map()));
        _signalMapper->setMapping(_toolbarActionStart,SIMULATION_COMMANDS_START_RESUME_SIMULATION_REQUEST_SCCMD);

        _toolbarActionPause=_toolbar1->addAction(QIcon(":/toolbarFiles/pause.png"),tr(IDS_PAUSE_SIMULATION_MENU_ITEM));
        _toolbarActionPause->setCheckable(true);
        connect(_toolbarActionPause,SIGNAL(triggered()),_signalMapper,SLOT(map()));
        _signalMapper->setMapping(_toolbarActionPause,SIMULATION_COMMANDS_PAUSE_SIMULATION_REQUEST_SCCMD);

        _toolbarActionStop=_toolbar1->addAction(QIcon(":/toolbarFiles/stop.png"),tr(IDS_STOP_SIMULATION_MENU_ITEM));
        _toolbarActionStop->setCheckable(false);
        connect(_toolbarActionStop,SIGNAL(triggered()),_signalMapper,SLOT(map()));
        _signalMapper->setMapping(_toolbarActionStop,SIMULATION_COMMANDS_STOP_SIMULATION_REQUEST_SCCMD);

        if (CSimFlavor::getBoolVal_int(0,App::userSettings->xrTest))
        {
            _toolbarActionOnline=_toolbar1->addAction(QIcon(":/toolbarFiles/online.png"),tr(IDS_TOOLBAR_TOOLTIP_ONLINE));
            _toolbarActionOnline->setCheckable(true);
            connect(_toolbarActionOnline,SIGNAL(triggered()),_signalMapper,SLOT(map()));
            _signalMapper->setMapping(_toolbarActionOnline,SIMULATION_COMMANDS_TOGGLE_ONLINE_SCCMD);
        }

        if (CSimFlavor::getBoolVal(11))
        {
            _toolbarActionRealTime=_toolbar1->addAction(QIcon(":/toolbarFiles/realTime.png"),tr(IDS_TOOLBAR_TOOLTIP_REALTIMESIMULATION));
            _toolbarActionRealTime->setCheckable(true);
            connect(_toolbarActionRealTime,SIGNAL(triggered()),_signalMapper,SLOT(map()));
            _signalMapper->setMapping(_toolbarActionRealTime,SIMULATION_COMMANDS_TOGGLE_REAL_TIME_SIMULATION_SCCMD);
        }

        if (CSimFlavor::getBoolVal(11))
        {
            _toolbarActionReduceSpeed=_toolbar1->addAction(QIcon(":/toolbarFiles/reduceSpeed.png"),tr(IDSN_SLOW_DOWN_SIMULATION));
            _toolbarActionReduceSpeed->setCheckable(false);
            connect(_toolbarActionReduceSpeed,SIGNAL(triggered()),_signalMapper,SLOT(map()));
            _signalMapper->setMapping(_toolbarActionReduceSpeed,SIMULATION_COMMANDS_SLOWER_SIMULATION_SCCMD);
        }

        if (CSimFlavor::getBoolVal(11))
        {
            _toolbarActionIncreaseSpeed=_toolbar1->addAction(QIcon(":/toolbarFiles/increaseSpeed.png"),tr(IDSN_SPEED_UP_SIMULATION));
            _toolbarActionIncreaseSpeed->setCheckable(false);
            connect(_toolbarActionIncreaseSpeed,SIGNAL(triggered()),_signalMapper,SLOT(map()));
            _signalMapper->setMapping(_toolbarActionIncreaseSpeed,SIMULATION_COMMANDS_FASTER_SIMULATION_SCCMD);
        }

        _toolbarActionToggleVisualization=_toolbar1->addAction(QIcon(":/toolbarFiles/toggleVisualization.png"),tr(IDSN_TOGGLE_VISUALIZATION));
        _toolbarActionToggleVisualization->setCheckable(true);
        connect(_toolbarActionToggleVisualization,SIGNAL(triggered()),_signalMapper,SLOT(map()));
        _signalMapper->setMapping(_toolbarActionToggleVisualization,SIMULATION_COMMANDS_TOGGLE_VISUALIZATION_SCCMD);
        _toolbar1->addSeparator();

        _toolbarActionPageSelector=_toolbar1->addAction(QIcon(":/toolbarFiles/pageSelector.png"),tr(IDSN_PAGE_SELECTOR));
        _toolbarActionPageSelector->setCheckable(true);
        connect(_toolbarActionPageSelector,SIGNAL(triggered()),_signalMapper,SLOT(map()));
        _signalMapper->setMapping(_toolbarActionPageSelector,PAGE_SELECTOR_CMD);

    }
    if ((_toolbar2==nullptr)&&(App::operationalUIParts&sim_gui_toolbar2))
    {
        _toolbar2=new QToolBar(tr("Tools"));
        _toolbar2->setIconSize(QSize(28,28));
        _toolbar2->setAllowedAreas(Qt::LeftToolBarArea|Qt::RightToolBarArea);
        addToolBar(Qt::LeftToolBarArea,_toolbar2);
        #ifdef MAC_SIM
            _toolbar2->setMovable(false); // 14/7/2013: since Qt5.1.0 the toolbar looks just plain white when undocked under MacOS
        #endif

        if (CSimFlavor::getBoolVal(11))
        {
            _toolbarActionSimulationSettings=_toolbar2->addAction(QIcon(":/toolbarFiles/simulationSettings.png"),tr(IDSN_SIMULATION_SETTINGS));
            _toolbarActionSimulationSettings->setCheckable(true);
            connect(_toolbarActionSimulationSettings,SIGNAL(triggered()),_signalMapper,SLOT(map()));
            _signalMapper->setMapping(_toolbarActionSimulationSettings,TOGGLE_SIMULATION_DLG_CMD);
            _toolbar2->addSeparator();
        }

        if (CSimFlavor::getBoolVal(12))
        {
            _toolbarActionObjectProperties=_toolbar2->addAction(QIcon(":/toolbarFiles/commonProperties.png"),tr(IDSN_OBJECT_PROPERTIES_MENU_ITEM));
            _toolbarActionObjectProperties->setCheckable(true);
            connect(_toolbarActionObjectProperties,SIGNAL(triggered()),_signalMapper,SLOT(map()));
            _signalMapper->setMapping(_toolbarActionObjectProperties,TOGGLE_OBJECT_DLG_CMD);
        }

        if (CSimFlavor::getBoolVal(12))
        {
            _toolbarActionCalculationModules=_toolbar2->addAction(QIcon(":/toolbarFiles/calcmods.png"),tr(IDSN_CALCULATION_MODULE_PROPERTIES_MENU_ITEM));
            _toolbarActionCalculationModules->setCheckable(true);
            connect(_toolbarActionCalculationModules,SIGNAL(triggered()),_signalMapper,SLOT(map()));
            _signalMapper->setMapping(_toolbarActionCalculationModules,TOGGLE_CALCULATION_DLG_CMD);
            _toolbar2->addSeparator();
        }

        if (CSimFlavor::getBoolVal(12))
        {
            _toolbarActionCollections=_toolbar2->addAction(QIcon(":/toolbarFiles/collections.png"),tr(IDSN_COLLECTIONS));
            _toolbarActionCollections->setCheckable(true);
            connect(_toolbarActionCollections,SIGNAL(triggered()),_signalMapper,SLOT(map()));
            _signalMapper->setMapping(_toolbarActionCollections,TOGGLE_COLLECTION_DLG_CMD);
        }

        if (CSimFlavor::getBoolVal(12))
        {
            _toolbarActionScripts=_toolbar2->addAction(QIcon(":/toolbarFiles/scripts.png"),tr(IDSN_SCRIPTS));
            _toolbarActionScripts->setCheckable(true);
            connect(_toolbarActionScripts,SIGNAL(triggered()),_signalMapper,SLOT(map()));
            _signalMapper->setMapping(_toolbarActionScripts,TOGGLE_LUA_SCRIPT_DLG_CMD);
        }

        if (CSimFlavor::getBoolVal(12))
        {
            _toolbarActionShapeEdition=_toolbar2->addAction(QIcon(":/toolbarFiles/shapeEdition.png"),tr(IDS_SHAPE_EDITION_TOOLBAR_TIP));
            _toolbarActionShapeEdition->setCheckable(true);
            connect(_toolbarActionShapeEdition,SIGNAL(triggered()),_signalMapper,SLOT(map()));
            _signalMapper->setMapping(_toolbarActionShapeEdition,SHAPE_EDIT_MODE_TOGGLE_ON_OFF_EMCMD);
        }

        if (CSimFlavor::getBoolVal(12))
        {
            _toolbarActionPathEdition=_toolbar2->addAction(QIcon(":/toolbarFiles/pathEdition.png"),tr(IDS_PATH_EDITION_TOOLBAR_TIP));
            _toolbarActionPathEdition->setCheckable(true);
            connect(_toolbarActionPathEdition,SIGNAL(triggered()),_signalMapper,SLOT(map()));
            _signalMapper->setMapping(_toolbarActionPathEdition,PATH_EDIT_MODE_TOGGLE_ON_OFF_EMCMD);
            _toolbar2->addSeparator();
        }


        if (CSimFlavor::getBoolVal(12))
        {
            _toolbarActionSelection=_toolbar2->addAction(QIcon(":/toolbarFiles/selection.png"),tr(IDSN_SELECTION_DIALOG));
            _toolbarActionSelection->setCheckable(true);
            connect(_toolbarActionSelection,SIGNAL(triggered()),_signalMapper,SLOT(map()));
            _signalMapper->setMapping(_toolbarActionSelection,TOGGLE_SELECTION_DLG_CMD);
        }

        _toolbarActionModelBrowser=_toolbar2->addAction(QIcon(":/toolbarFiles/modelBrowser.png"),tr(IDSN_MODEL_BROWSER));
        _toolbarActionModelBrowser->setCheckable(true);
        connect(_toolbarActionModelBrowser,SIGNAL(triggered()),_signalMapper,SLOT(map()));
        _signalMapper->setMapping(_toolbarActionModelBrowser,TOGGLE_BROWSER_DLG_CMD);

        if (CSimFlavor::getBoolVal(11))
        {
            _toolbarActionSceneHierarchy=_toolbar2->addAction(QIcon(":/toolbarFiles/sceneHierarchy.png"),tr(IDSN_SCENE_HIERARCHY));
            _toolbarActionSceneHierarchy->setCheckable(true);
            connect(_toolbarActionSceneHierarchy,SIGNAL(triggered()),_signalMapper,SLOT(map()));
            _signalMapper->setMapping(_toolbarActionSceneHierarchy,TOGGLE_HIERARCHY_DLG_CMD);
        }

        if (CSimFlavor::getBoolVal(11))
        {
            _toolbarActionLayers=_toolbar2->addAction(QIcon(":/toolbarFiles/layers.png"),tr(IDS_LAYERS));
            _toolbarActionLayers->setCheckable(true);
            connect(_toolbarActionLayers,SIGNAL(triggered()),_signalMapper,SLOT(map()));
            _signalMapper->setMapping(_toolbarActionLayers,TOGGLE_LAYERS_DLG_CMD);
        }

        _toolbarActionAviRecorder=_toolbar2->addAction(QIcon(":/toolbarFiles/aviRecorder.png"),tr(IDSN_AVI_RECORDER));
        _toolbarActionAviRecorder->setCheckable(true);
        connect(_toolbarActionAviRecorder,SIGNAL(triggered()),_signalMapper,SLOT(map()));
        _signalMapper->setMapping(_toolbarActionAviRecorder,TOGGLE_AVI_RECORDER_DLG_CMD);

        _toolbarActionUserSettings=_toolbar2->addAction(QIcon(":/toolbarFiles/userSettings.png"),tr(IDSN_USER_SETTINGS));
        _toolbarActionUserSettings->setCheckable(CSimFlavor::getBoolVal(11));
        connect(_toolbarActionUserSettings,SIGNAL(triggered()),_signalMapper,SLOT(map()));
        _signalMapper->setMapping(_toolbarActionUserSettings,TOGGLE_SETTINGS_DLG_CMD);
 
    }
    refreshDimensions();
    App::setToolbarRefreshFlag();

}

bool CMainWindow::event(QEvent* event)
{
    if (event->type()==QEvent::Close)
    {
        setOpenGlDisplayEnabled(true); // We might be in fast simulation mode...
        CFileOperations::processCommand(FILE_OPERATION_EXIT_SIMULATOR_FOCMD);
        event->ignore();
        return(true);
    }
    return(QMainWindow::event(event));
}

void CMainWindow::dragEnterEvent(QDragEnterEvent* dEvent)
{
    if (dEvent->mimeData()->hasUrls())
    {
        QStringList pathList;
        QList<QUrl> urlList=dEvent->mimeData()->urls();
        if (urlList.size()>0)
        {
            int sceneCnt=0;
            int modelCnt=0;
            int fileCnt=0;
            for (int i=0;i<urlList.size()&&(i<100);++i)
            {
                std::string pathFile=urlList.at(i).toLocalFile().toStdString();
                std::string extension(VVarious::splitPath_fileExtension(pathFile.c_str()));

                if (extension.compare(SIM_SCENE_EXTENSION)==0)
                    sceneCnt++;
                else
                    sceneCnt+=CSimFlavor::getIntVal_str(1,extension.c_str());
                if (extension.compare(SIM_MODEL_EXTENSION)==0)
                    modelCnt++;
                else
                    modelCnt+=CSimFlavor::getIntVal_str(2,extension.c_str());
                fileCnt++;
            }
            if ( (fileCnt==sceneCnt)&&(sceneCnt>0) )
                dEvent->acceptProposedAction();
            if ( (fileCnt==modelCnt)&&(modelCnt>0) )
                dEvent->acceptProposedAction();
        }
    }
}

void CMainWindow::dropEvent(QDropEvent* dEvent)
{
    if (dEvent->mimeData()->hasUrls())
    {
        QStringList pathList;
        QList<QUrl> urlList=dEvent->mimeData()->urls();
        if (urlList.size()>0)
        {
            std::vector<std::string> scenes;
            std::vector<std::string> models;
            int fileCnt=0;
            for (int i=0;i<urlList.size()&&(i<100);++i)
            {
                std::string pathFile=urlList.at(i).toLocalFile().toLocal8Bit().data();
                std::string extension(VVarious::splitPath_fileExtension(pathFile.c_str()));
                if (extension.compare(SIM_SCENE_EXTENSION)==0)
                    scenes.push_back(pathFile);
                if (extension.compare(SIM_MODEL_EXTENSION)==0)
                    models.push_back(pathFile);
                if (CSimFlavor::getIntVal_str(1,extension.c_str())==1)
                    scenes.push_back(pathFile);
                if (CSimFlavor::getIntVal_str(2,extension.c_str())==1)
                    models.push_back(pathFile);
                fileCnt++;
            }
            if ( ( (fileCnt==int(scenes.size()))&&(scenes.size()>0) ) ||  ( (fileCnt==int(models.size()))&&(models.size()>0) )  )
                _dropFilesIntoScene(scenes,models);
        }
    }
}


void CMainWindow::onLeftMouseButtonDoubleClickTT(int xPos,int yPos)
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    _mouseRenderingPos.x=xPos;
    _mouseRenderingPos.y=_clientArea.y-yPos;
    int selectionStatus=NOSELECTION;
    int mm=getMouseMode();
    if ( (getKeyDownState()&2)&&(mm&sim_navigation_shiftselection) )
        selectionStatus=SHIFTSELECTION; 
    else if ( (getKeyDownState()&1)&&(mm&sim_navigation_ctrlselection) )
        selectionStatus=CTRLSELECTION;
    if (getOpenGlDisplayEnabled())
        oglSurface->leftMouseButtonDoubleClick(_mouseRenderingPos.x,_mouseRenderingPos.y,selectionStatus);
    setOpenGlDisplayEnabled(true); // enable the display again
}


void CMainWindow::onLeftMouseButtonDownTT(int xPos,int yPos)
{ // keys: bit0: ctrl, bit1: shift
    _mouseRenderingPos.x=xPos;
    _mouseRenderingPos.y=_clientArea.y-yPos;
    setMouseButtonState(getMouseButtonState()|1);
    int mm=getMouseMode();
    int selectionStatus=NOSELECTION;
    if ( ((getKeyDownState()&3)==3)&&(mm&sim_navigation_ctrlselection) )
        selectionStatus=CTRLSELECTION;                                                      
    else if ( (getKeyDownState()&2)&&(mm&sim_navigation_shiftselection) )
        selectionStatus=SHIFTSELECTION; 
    else if ( (getKeyDownState()&1)&&(mm&sim_navigation_ctrlselection) )
        selectionStatus=CTRLSELECTION;
    oglSurface->clearCaughtElements(0xffff-sim_left_button);
    if (getOpenGlDisplayEnabled())
        oglSurface->leftMouseButtonDown(_mouseRenderingPos.x,_mouseRenderingPos.y,selectionStatus);
    setOpenGlDisplayEnabled(true); // Enable the display again
    setCurrentCursor(oglSurface->getCursor(xPos,yPos));
}

void CMainWindow::onMiddleMouseButtonDownTT(int xPos,int yPos)
{
    _mouseRenderingPos.x=xPos;
    _mouseRenderingPos.y=_clientArea.y-yPos;
    oglSurface->clearCaughtElements(0xffff-sim_middle_button);
    setMouseButtonState(getMouseButtonState()|8);
    int upperMouseMode=((getMouseMode()&0xff00)|sim_navigation_clickselection)-sim_navigation_clickselection; // sim_navigation_clickselection because otherwise we have a problem (12/06/2011)
    if (getMouseMode()&sim_navigation_camerarotaterightbutton)
        setMouseMode(upperMouseMode|sim_navigation_camerarotate); // default
    else
        setMouseMode(upperMouseMode|sim_navigation_passive);

    if (getOpenGlDisplayEnabled())
        oglSurface->middleMouseButtonDown(_mouseRenderingPos.x,_mouseRenderingPos.y);
    setOpenGlDisplayEnabled(true); // Enable the display again
}

void CMainWindow::onRightMouseButtonDownTT(int xPos,int yPos)
{
    setMouseButtonState(getMouseButtonState()|4);
    _mouseRenderingPos.x=xPos;
    _mouseRenderingPos.y=_clientArea.y-yPos;
    if (App::userSettings->navigationBackwardCompatibility)
        App::setLightDialogRefreshFlag();
    oglSurface->clearCaughtElements(0xffff-sim_right_button);
    if (getOpenGlDisplayEnabled())
        oglSurface->rightMouseButtonDown(_mouseRenderingPos.x,_mouseRenderingPos.y);
    setOpenGlDisplayEnabled(true); // Enable the display again
}

void CMainWindow::onLeftMouseButtonUpTT(int xPos,int yPos)
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    _mouseRenderingPos.x=xPos;
    _mouseRenderingPos.y=_clientArea.y-yPos;
    _flyModeCameraHandle=-1;
    App::setLightDialogRefreshFlag(); // to refresh dlgs when an object has been dragged for instance
    if (oglSurface->getCaughtElements()&sim_left_button)
        oglSurface->leftMouseButtonUp(_mouseRenderingPos.x,_mouseRenderingPos.y);
    oglSurface->clearCaughtElements(0xffff-sim_left_button);
    setMouseButtonState(getMouseButtonState()&(0xffff-1));
    setCurrentCursor(oglSurface->getCursor(xPos,yPos));
}

void CMainWindow::onMiddleMouseButtonUpTT(int xPos,int yPos)
{ // Middle mouse button up is exclusively reserved for rotation
    _mouseRenderingPos.x=xPos;
    _mouseRenderingPos.y=_clientArea.y-yPos;
    if (oglSurface->getCaughtElements()&sim_middle_button)
        oglSurface->middleMouseButtonUp(_mouseRenderingPos.x,_mouseRenderingPos.y);
    oglSurface->clearCaughtElements(0xffff-sim_middle_button);
}

void CMainWindow::onRightMouseButtonUpTT(int xPos,int yPos)
{ // Right mouse button up is exclusively reserved for pop-up menu
    _mouseRenderingPos.x=xPos;
    _mouseRenderingPos.y=_clientArea.y-yPos;
    if (oglSurface->getCaughtElements()&sim_right_button)
    {
        QPoint glob(openglWidget->mapToGlobal(QPoint(xPos,yPos)));
        oglSurface->rightMouseButtonUp(_mouseRenderingPos.x,_mouseRenderingPos.y,glob.x(),glob.y(),this);
    }
    oglSurface->clearCaughtElements(0xffff-sim_right_button);
    setMouseButtonState(getMouseButtonState()&(0xffff-4));
}

void CMainWindow::getMouseRenderingPos(int pos[2])
{
    pos[0]=_mouseRenderingPos.x;
    pos[1]=_mouseRenderingPos.y;
}

void CMainWindow::onWheelRotateTT(int delta,int xPos,int yPos)
{
    _mouseRenderingPos.x=xPos;
    _mouseRenderingPos.y=_clientArea.y-yPos;
    setMouseButtonState(getMouseButtonState()|2);
    if (getMouseMode()&sim_navigation_camerazoomwheel)
    {
        if (getOpenGlDisplayEnabled())
            oglSurface->mouseWheel(delta,_mouseRenderingPos.x,_mouseRenderingPos.y);
    }
    _mouseWheelEventTime=VDateTime::getTimeInMs();
}

void CMainWindow::onMouseMoveTT(int xPos,int yPos)
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    _mouseRenderingPos.x=xPos;
    _mouseRenderingPos.y=_clientArea.y-yPos;
    int cur=-1;
    if (getOpenGlDisplayEnabled())
    {
        int bts=sim_right_button|sim_middle_button|sim_left_button;
        if (App::userSettings->navigationBackwardCompatibility)
            bts=sim_right_button|sim_left_button;

        oglSurface->mouseMove(_mouseRenderingPos.x,_mouseRenderingPos.y,(oglSurface->getCaughtElements()&bts)==0);
        cur=oglSurface->getCursor(_mouseRenderingPos.x,_mouseRenderingPos.y);
    }
    if ((_mouseButtonsState&1)==0)
        setCurrentCursor(cur);
}

int CMainWindow::modelDragMoveEvent(int xPos,int yPos,C3Vector* desiredModelPosition)
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    _mouseRenderingPos.x=xPos;
    _mouseRenderingPos.y=_clientArea.y-yPos;
    if (getOpenGlDisplayEnabled())
    {
        int ret=oglSurface->modelDragMoveEvent(_mouseRenderingPos.x,_mouseRenderingPos.y,desiredModelPosition);
        return(ret);
    }
    return(1); // We can drop the model at the default location
}

void CMainWindow::setCurrentCursor(int cur)
{
//    printf("Cursor: %i\n",cur);
    if (cur!=-1)
        _currentCursor=cur;
    else
        _currentCursor=sim_cursor_arrow;
    if (previousCursor!=_currentCursor)
    {
        setCursor(Qt::PointingHandCursor);
        if (_currentCursor==sim_cursor_arrow)
            setCursor(Qt::ArrowCursor);
        if (_currentCursor==sim_cursor_finger)
            setCursor(Qt::PointingHandCursor);
        if (_currentCursor==sim_cursor_all_directions)
            setCursor(Qt::SizeAllCursor);
        if (_currentCursor==sim_cursor_horizontal_directions)
            setCursor(Qt::SizeHorCursor);
        if (_currentCursor==sim_cursor_vertical_directions)
            setCursor(Qt::SizeVerCursor);
        if (_currentCursor==sim_cursor_slash_directions)
            setCursor(Qt::SizeBDiagCursor);
        if (_currentCursor==sim_cursor_backslash_directions)
            setCursor(Qt::SizeFDiagCursor);
        if (_currentCursor==sim_cursor_open_hand)
            setCursor(Qt::OpenHandCursor);
        if (_currentCursor==sim_cursor_closed_hand)
            setCursor(Qt::ClosedHandCursor);
        if (_currentCursor==sim_cursor_cross)
            setCursor(Qt::CrossCursor);
        previousCursor=_currentCursor;
    }
}

void CMainWindow::simulationAboutToStart()
{
    tabBar->setEnabled(false);
    editModeContainer->simulationAboutToStart();
    // reset those:
    _proxSensorClickSelectDown=0;
    _proxSensorClickSelectUp=0;
}

void CMainWindow::simulationEnded()
{
    tabBar->setEnabled(true);
    editModeContainer->simulationEnded();
    // reset those:
    _proxSensorClickSelectDown=0;
    _proxSensorClickSelectUp=0;
}

void CMainWindow::editModeAboutToStart()
{
    tabBar->setEnabled(false);
}

void CMainWindow::editModeEnded()
{
    tabBar->setEnabled(true);
}


void CMainWindow::_actualizetoolbarButtonState()
{ // This is only for the default toolbars
    bool allowFitToView=false;
    int pageIndex=App::currentWorld->pageContainer->getActivePageIndex();
    CSPage* page=App::currentWorld->pageContainer->getPage(pageIndex);
    if (page!=nullptr)
    {
        int ind=page->getLastMouseDownViewIndex();
        if (ind==-1)
            ind=0;
        CSView* view=page->getView(ind);
        if (view!=nullptr)
        {
            CCamera* cam=App::currentWorld->sceneObjects->getCameraFromHandle(view->getLinkedObjectID());
            allowFitToView=(cam!=nullptr);
        }
    }

    bool noEditMode=(editModeContainer->getEditModeType()==NO_EDIT_MODE);
    bool noUiNorMultishapeEditMode=(editModeContainer->getEditModeType()!=MULTISHAPE_EDIT_MODE);
    bool noSelector=((!oglSurface->isPageSelectionActive())&&(!oglSurface->isViewSelectionActive()));

    bool assembleEnabled=false;
    bool disassembleEnabled=false;
    size_t selS=App::currentWorld->sceneObjects->getSelectionCount();
    if (selS==1)
    { // here we can only have disassembly
        CSceneObject* it=App::currentWorld->sceneObjects->getLastSelectionObject();
        disassembleEnabled=(it->getParent()!=nullptr)&&(it->getAssemblyMatchValues(true).length()!=0);
    }
    else if (selS==2)
    { // here we can have assembly or disassembly
        CSceneObject* it1=App::currentWorld->sceneObjects->getLastSelectionObject();
        CSceneObject* it2=App::currentWorld->sceneObjects->getObjectFromHandle(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(0));
        if ((it1->getParent()==it2)||(it2->getParent()==it1))
        {
            if ( (it1->getParent()==it2)&&(it1->getAssemblyMatchValues(true).length()!=0) )
                disassembleEnabled=true; // disassembly
            if ( (it2->getParent()==it1)&&(it2->getAssemblyMatchValues(true).length()!=0) )
                disassembleEnabled=true; // disassembly
        }
        else
        { // assembly
            std::vector<CSceneObject*> potParents;
            it1->getAllChildrenThatMayBecomeAssemblyParent(it2->getChildAssemblyMatchValuesPointer(),potParents);
            bool directAssembly=it1->doesParentAssemblingMatchValuesMatchWithChild(it2->getChildAssemblyMatchValuesPointer());
            if ( directAssembly||(potParents.size()==1) )
                assembleEnabled=true;
            else
            { // here we might have the opposite of what we usually do to assemble (i.e. last selection should always be parent, but not here)
                // we assemble anyways if the roles are unequivoque:
                if ( it2->doesParentAssemblingMatchValuesMatchWithChild(it1->getChildAssemblyMatchValuesPointer()) )
                    assembleEnabled=true;
            }
        }
    }

    bool transferDnaAllowed=false;
    if ( (selS==1)&&noSelector&&(editModeContainer->getEditModeType()==NO_EDIT_MODE)&&App::currentWorld->simulation->isSimulationStopped() )
    {
        CSceneObject* it=App::currentWorld->sceneObjects->getLastSelectionObject();
        if (it->getLocalObjectProperty()&sim_objectproperty_canupdatedna)
        { 
            bool model=it->getModelBase();
            // Check if we have a sibling in the scene:
            for (size_t i=0;i<App::currentWorld->sceneObjects->getObjectCount();i++)
            {
                CSceneObject* it2=App::currentWorld->sceneObjects->getObjectFromIndex(i);
                if ( (it2!=it)&&(it2->getLocalObjectProperty()&sim_objectproperty_canupdatedna)&&(it2->getDnaString().compare(it->getDnaString())==0) )
                {
                    if (!model)
                    {
                        transferDnaAllowed=true; // yes, and it is not a model
                        break;
                    }
                    else
                    { // Here we also have to check that the sibling model is not located in the same hierarchy as this one:
                        bool sameHierarchy=false;
                        while (true)
                        {
                            if (it2==nullptr)
                                break;
                            if (it2==it)
                            {
                                sameHierarchy=true;
                                break;
                            }
                            it2=it2->getParent();
                        }
                        if (!sameHierarchy)
                        {
                            transferDnaAllowed=true;
                            break;
                        }
                    }
                }
            }
        }
    }


    if (_toolbar1!=nullptr)
    { // We enable/disable some buttons:
        _toolbarActionCameraShift->setEnabled(noSelector);
        _toolbarActionCameraRotate->setEnabled(noSelector);
        _toolbarActionCameraZoom->setEnabled(noSelector);
        if (CSimFlavor::getBoolVal(11))
            _toolbarActionCameraAngle->setEnabled(noSelector);
        _toolbarActionCameraSizeToScreen->setEnabled(allowFitToView&&noSelector);
//        _toolbarActionCameraFly->setEnabled(noSelector);

        if (disassembleEnabled)
            _toolbarActionAssemble->setIcon(QIcon(":/toolbarFiles/disassemble.png"));
        else
            _toolbarActionAssemble->setIcon(QIcon(":/toolbarFiles/assemble.png"));
        _toolbarActionAssemble->setEnabled(assembleEnabled||disassembleEnabled);

        if (CSimFlavor::getBoolVal(11))
            _toolbarActionTransferDna->setEnabled(transferDnaAllowed);

        _toolbarActionObjectShift->setEnabled(noUiNorMultishapeEditMode&&noSelector&&_toolbarButtonObjectShiftEnabled);
        bool rot=true;
        if (App::currentWorld->sceneObjects!=nullptr)
            rot=editModeContainer->pathPointManipulation->getSelectedPathPointIndicesSize_nonEditMode()==0;
        _toolbarActionObjectRotate->setEnabled(noUiNorMultishapeEditMode&&rot&&noSelector&&_toolbarButtonObjectRotateEnabled);

        if (CSimFlavor::getBoolVal(11))
            _toolbarActionClickSelection->setEnabled(noSelector);

        _toolbarActionUndo->setEnabled(App::currentWorld->undoBufferContainer->canUndo()&&noSelector);
        _toolbarActionRedo->setEnabled(App::currentWorld->undoBufferContainer->canRedo()&&noSelector);

        if (CSimFlavor::getBoolVal(11))
            _toolbarActionDynamicContentVisualization->setEnabled((!App::currentWorld->simulation->isSimulationStopped())&&noSelector);

        _engineSelectCombo->setEnabled((editModeContainer->getEditModeType()==NO_EDIT_MODE)&&App::currentWorld->simulation->isSimulationStopped()&&App::currentWorld->dynamicsContainer->getDynamicsEnabled()&&noSelector);
        if (CSimFlavor::getBoolVal(11))
            _enginePrecisionCombo->setEnabled((editModeContainer->getEditModeType()==NO_EDIT_MODE)&&App::currentWorld->simulation->isSimulationStopped()&&App::currentWorld->dynamicsContainer->getDynamicsEnabled()&&noSelector);
        if (CSimFlavor::getBoolVal(11))
            _timeStepConfigCombo->setEnabled((editModeContainer->getEditModeType()==NO_EDIT_MODE)&&App::currentWorld->simulation->isSimulationStopped()&&noSelector);
        if (CSimFlavor::getBoolVal_int(0,App::userSettings->xrTest))
            _toolbarActionVerify->setEnabled((editModeContainer->getEditModeType()==NO_EDIT_MODE)&&App::currentWorld->simulation->isSimulationStopped()&&noSelector);
        _toolbarActionStart->setEnabled(_toolbarButtonPlayEnabled&&(editModeContainer->getEditModeType()==NO_EDIT_MODE)&&(!App::currentWorld->simulation->isSimulationRunning())&&noSelector);
        _toolbarActionPause->setEnabled(_toolbarButtonPauseEnabled&&(editModeContainer->getEditModeType()==NO_EDIT_MODE)&&App::currentWorld->simulation->isSimulationRunning()&&noSelector);
        _toolbarActionStop->setEnabled(_toolbarButtonStopEnabled&&(editModeContainer->getEditModeType()==NO_EDIT_MODE)&&(!App::currentWorld->simulation->isSimulationStopped())&&noSelector);
        if (CSimFlavor::getBoolVal_int(0,App::userSettings->xrTest))
            _toolbarActionOnline->setEnabled((editModeContainer->getEditModeType()==NO_EDIT_MODE)&&App::currentWorld->simulation->isSimulationStopped()&&noSelector);
        if (CSimFlavor::getBoolVal(11))
            _toolbarActionRealTime->setEnabled((editModeContainer->getEditModeType()==NO_EDIT_MODE)&&App::currentWorld->simulation->isSimulationStopped()&&noSelector);
        if (CSimFlavor::getBoolVal(11))
            _toolbarActionReduceSpeed->setEnabled(App::currentWorld->simulation->canGoSlower()&&noSelector);
        if (CSimFlavor::getBoolVal(11))
            _toolbarActionIncreaseSpeed->setEnabled(App::currentWorld->simulation->canGoFaster()&&noSelector);
        _toolbarActionToggleVisualization->setEnabled(App::currentWorld->simulation->isSimulationRunning()&&noSelector);

        _toolbarActionPageSelector->setEnabled((!oglSurface->isViewSelectionActive()));

        // Now we check/uncheck some buttons:
        _toolbarActionCameraShift->setChecked((getMouseMode()&0x00ff)==sim_navigation_camerashift);
        _toolbarActionCameraRotate->setChecked((getMouseMode()&0x00ff)==sim_navigation_camerarotate);
        _toolbarActionCameraZoom->setChecked((getMouseMode()&0x00ff)==sim_navigation_camerazoom);
        if (CSimFlavor::getBoolVal(11))
            _toolbarActionCameraAngle->setChecked((getMouseMode()&0x00ff)==sim_navigation_cameraangle);
//       _toolbarActionCameraFly->setChecked((getMouseMode()&0x00ff)==sim_navigation_camerafly);
        _toolbarActionObjectShift->setChecked((getMouseMode()&0x00ff)==sim_navigation_objectshift);
        _toolbarActionObjectRotate->setChecked((getMouseMode()&0x00ff)==sim_navigation_objectrotate);

        if (CSimFlavor::getBoolVal(11))
            _toolbarActionClickSelection->setChecked((getMouseMode()&0x0300)==sim_navigation_clickselection);
        if (CSimFlavor::getBoolVal(11))
            _toolbarActionDynamicContentVisualization->setChecked(App::currentWorld->simulation->getDynamicContentVisualizationOnly());

        int ver;
        int eng=App::currentWorld->dynamicsContainer->getDynamicEngineType(&ver);
        if ( (eng==sim_physics_bullet)&&(ver==0) )
            _engineSelectCombo->setCurrentIndex(0);
        if ( (eng==sim_physics_bullet)&&(ver==283) )
            _engineSelectCombo->setCurrentIndex(1);
        if (eng==sim_physics_ode)
            _engineSelectCombo->setCurrentIndex(2);
        if (eng==sim_physics_vortex)
            _engineSelectCombo->setCurrentIndex(3);
        if (eng==sim_physics_newton)
            _engineSelectCombo->setCurrentIndex(4);

        if (CSimFlavor::getBoolVal(11))
        {
            _enginePrecisionCombo->setCurrentIndex(App::currentWorld->dynamicsContainer->getUseDynamicDefaultCalculationParameters());
            if (App::currentWorld->simulation->isSimulationStopped())
            {
                _timeStepConfigCombo->setToolTip(IDS_TOOLBAR_TOOLTIP_SIMULATION_TIME_STEP);
                _timeStepConfigCombo->setItemText(0,tr(IDSN_TIME_STEP_CONFIG_200));
                _timeStepConfigCombo->setItemText(1,tr(IDSN_TIME_STEP_CONFIG_100));
                _timeStepConfigCombo->setItemText(2,tr(IDSN_TIME_STEP_CONFIG_50));
                _timeStepConfigCombo->setItemText(3,tr(IDSN_TIME_STEP_CONFIG_25));
                _timeStepConfigCombo->setItemText(4,tr(IDSN_TIME_STEP_CONFIG_10));

    //          _timeStepConfigCombo->setItemText(5,tr(IDSN_TIME_STEP_CONFIG_CUSTOM));
                float dt=(float(App::currentWorld->simulation->getSimulationTimeStep_speedModified_ns(5))/1000.0f);
                std::string txt("dt=");
                txt+=tt::FNb(0,dt,1,false);
                txt+=IDSN_TIME_STEP_CONFIG_CUSTOM;
                _timeStepConfigCombo->setItemText(5,txt.c_str());

                _timeStepConfigCombo->setCurrentIndex(App::currentWorld->simulation->getDefaultSimulationParameterIndex());
            }
            else
            {
                _timeStepConfigCombo->setToolTip(IDS_TOOLBAR_TOOLTIP_DT_SIMULATION_TIME_STEP_AND_PPF);
                float dt=(float(App::currentWorld->simulation->getSimulationTimeStep_speedModified_ns())/1000.0f);
                std::string txt("dt=");
                txt+=tt::FNb(0,dt,1,false);
                txt+=" ms, ppf=";
                txt+=tt::FNb(App::currentWorld->simulation->getSimulationPassesPerRendering_speedModified());
                _timeStepConfigCombo->setItemText(App::currentWorld->simulation->getDefaultSimulationParameterIndex(),txt.c_str());
            }
        }

        _toolbarActionStart->setChecked(App::currentWorld->simulation->isSimulationRunning());
        _toolbarActionPause->setChecked(App::currentWorld->simulation->isSimulationPaused());

        if (CSimFlavor::getBoolVal_int(0,App::userSettings->xrTest))
        {
            _toolbarActionOnline->setChecked(App::currentWorld->simulation->getOnlineMode());
            if (App::currentWorld->simulation->getOnlineMode())
                _toolbarActionOnline->setIcon(QIcon(":/toolbarFiles/onlineOn.png"));
            else
                _toolbarActionOnline->setIcon(QIcon(":/toolbarFiles/online.png"));
        }
        if (CSimFlavor::getBoolVal(11))
            _toolbarActionRealTime->setChecked(App::currentWorld->simulation->getRealTimeSimulation());

        _toolbarActionToggleVisualization->setChecked(!getOpenGlDisplayEnabled());
        _toolbarActionPageSelector->setChecked(oglSurface->isPageSelectionActive());
    }
    if (_toolbar2!=nullptr)
    { // We enable/disable some buttons:
        if (CSimFlavor::getBoolVal(11))
            _toolbarActionSimulationSettings->setEnabled(noEditMode&&noSelector);

        if (CSimFlavor::getBoolVal(12))
            _toolbarActionObjectProperties->setEnabled(_toolbarButtonObjPropEnabled&&noEditMode&&noSelector);
        if (CSimFlavor::getBoolVal(12))
            _toolbarActionCalculationModules->setEnabled(_toolbarButtonCalcModulesEnabled&&noEditMode&&noSelector);
        if (CSimFlavor::getBoolVal(12))
            _toolbarActionCollections->setEnabled(noEditMode&&noSelector);
        if (CSimFlavor::getBoolVal(12))
            _toolbarActionScripts->setEnabled(noEditMode&&noSelector);
        if (CSimFlavor::getBoolVal(12))
            _toolbarActionShapeEdition->setEnabled((noSelector&&(selS==1)&&App::currentWorld->sceneObjects->isLastSelectionAShape()&&App::currentWorld->simulation->isSimulationStopped()&&(editModeContainer->getEditModeType()==NO_EDIT_MODE))||(editModeContainer->getEditModeType()&SHAPE_EDIT_MODE)||(editModeContainer->getEditModeType()&MULTISHAPE_EDIT_MODE));
        if (CSimFlavor::getBoolVal(12))
            _toolbarActionPathEdition->setEnabled((noSelector&&(selS==1)&&App::currentWorld->sceneObjects->isLastSelectionAPath()&&App::currentWorld->simulation->isSimulationStopped()&&(editModeContainer->getEditModeType()==NO_EDIT_MODE))||(editModeContainer->getEditModeType()&PATH_EDIT_MODE));
        if (CSimFlavor::getBoolVal(12))
            _toolbarActionSelection->setEnabled(noEditMode&&noSelector);

        _toolbarActionModelBrowser->setEnabled(noEditMode&&noSelector&&_toolbarButtonBrowserEnabled);

        if (CSimFlavor::getBoolVal(11))
            _toolbarActionSceneHierarchy->setEnabled(noEditMode&&noSelector&&_toolbarButtonHierarchyEnabled&&((!App::userSettings->sceneHierarchyHiddenDuringSimulation)||App::currentWorld->simulation->isSimulationStopped()) );
        if (CSimFlavor::getBoolVal(11))
            _toolbarActionLayers->setEnabled(true);

        _toolbarActionAviRecorder->setEnabled(noEditMode&&noSelector&&(CAuxLibVideo::video_recorderGetEncoderString!=nullptr));
        _toolbarActionUserSettings->setEnabled(noEditMode&&noSelector);

        // Now we check/uncheck some buttons:
        if (CSimFlavor::getBoolVal(11))
            _toolbarActionSimulationSettings->setChecked(dlgCont->isVisible(SIMULATION_DLG));

        if (CSimFlavor::getBoolVal(12))
            _toolbarActionObjectProperties->setChecked(dlgCont->isVisible(OBJECT_DLG));
        if (CSimFlavor::getBoolVal(12))
            _toolbarActionCalculationModules->setChecked(dlgCont->isVisible(CALCULATION_DLG));
        if (CSimFlavor::getBoolVal(12))
            _toolbarActionCollections->setChecked(dlgCont->isVisible(COLLECTION_DLG));
        if (CSimFlavor::getBoolVal(12))
            _toolbarActionScripts->setChecked(dlgCont->isVisible(LUA_SCRIPT_DLG));
        if (CSimFlavor::getBoolVal(12))
            _toolbarActionShapeEdition->setChecked(editModeContainer->getEditModeType()&SHAPE_EDIT_MODE);
        if (CSimFlavor::getBoolVal(12))
            _toolbarActionPathEdition->setChecked(editModeContainer->getEditModeType()==PATH_EDIT_MODE);
        if (CSimFlavor::getBoolVal(12))
            _toolbarActionSelection->setChecked(dlgCont->isVisible(SELECTION_DLG));

        _toolbarActionModelBrowser->setChecked(dlgCont->isVisible(BROWSER_DLG));

        if (CSimFlavor::getBoolVal(11))
            _toolbarActionSceneHierarchy->setChecked(dlgCont->isVisible(HIERARCHY_DLG));
        if (CSimFlavor::getBoolVal(11))
            _toolbarActionLayers->setChecked(dlgCont->isVisible(LAYERS_DLG));

        _toolbarActionAviRecorder->setChecked(dlgCont->isVisible(AVI_RECORDER_DLG));
        _toolbarActionUserSettings->setChecked(dlgCont->isVisible(SETTINGS_DLG));
    }
}

void CMainWindow::_recomputeClientSizeAndPos()
{
    if ( windowHandle()&&windowHandle()->isExposed()&&(_clientArea.x!=0)&&(_clientArea.y!=0) ) // Added the two last args to avoid a collaps of the hierarchy when switching to another app (2011/01/26)
        oglSurface->setSurfaceSizeAndPosition(_clientArea.x,_clientArea.y+App::userSettings->renderingSurfaceVResize,0,App::userSettings->renderingSurfaceVShift);
}

void CMainWindow::_dropFilesIntoScene(const std::vector<std::string>& tttFiles,const std::vector<std::string>& ttmFiles)
{
    IF_UI_EVENT_CAN_READ_DATA
    {
        if ( ((tttFiles.size()>0)&&(ttmFiles.size()==0))||((tttFiles.size()==0)&&(ttmFiles.size()>0)) )
        {
            if (editModeContainer->getEditModeType()!=NO_EDIT_MODE)
                App::uiThread->messageBox_warning(this,"Drag and drop",IDS_END_EDIT_MODE_BEFORE_PROCEEDING,VMESSAGEBOX_OKELI);
            else
            {
                if (tttFiles.size()>0)
                { // loading (a) scene(s):
                    if (!App::currentWorld->simulation->isSimulationStopped())
                        App::uiThread->messageBox_warning(this,"Drag and drop",IDS_STOP_SIMULATION_BEFORE_PROCEEDING,VMESSAGEBOX_OKELI);
                    else
                    {
                        for (size_t i=0;i<tttFiles.size();i++)
                        {
                            SSimulationThreadCommand cmd;
                            cmd.cmdId=OPEN_DRAG_AND_DROP_SCENE_CMD;
                            cmd.stringParams.push_back(tttFiles[i]);
                            cmd.boolParams.push_back(tttFiles.size()<2);
                            App::appendSimulationThreadCommand(cmd);
                        }
                    }
                }
                else
                {
                    if (ttmFiles.size()>0)
                    { // loading (a) model(s):
                        for (int i=0;i<int(ttmFiles.size());i++)
                        {
                            SSimulationThreadCommand cmd;
                            cmd.cmdId=FILE_OPERATION_OPEN_DRAG_AND_DROP_MODEL_FOCMD;
                            cmd.stringParams.push_back(ttmFiles[i]);
                            cmd.boolParams.push_back(ttmFiles.size()<2);
                            cmd.boolParams.push_back(i==int(ttmFiles.size()-1));
                            App::appendSimulationThreadCommand(cmd);
                        }
                    }
                }
            }
        }
    }
}


void CMainWindow::_engineSelectedViaToolbar(int index)
{
    if (index==0)
        App::currentWorld->simulation->processCommand(SIMULATION_COMMANDS_TOGGLE_TO_BULLET_2_78_ENGINE_SCCMD);
    if (index==1)
        App::currentWorld->simulation->processCommand(SIMULATION_COMMANDS_TOGGLE_TO_BULLET_2_83_ENGINE_SCCMD);
    if (index==2)
        App::currentWorld->simulation->processCommand(SIMULATION_COMMANDS_TOGGLE_TO_ODE_ENGINE_SCCMD);
    if (index==3)
        App::currentWorld->simulation->processCommand(SIMULATION_COMMANDS_TOGGLE_TO_VORTEX_ENGINE_SCCMD);
    if (index==4)
        App::currentWorld->simulation->processCommand(SIMULATION_COMMANDS_TOGGLE_TO_NEWTON_ENGINE_SCCMD);
}

void CMainWindow::_enginePrecisionViaToolbar(int index)
{
    App::currentWorld->dynamicsContainer->setUseDynamicDefaultCalculationParameters(index);
    POST_SCENE_CHANGED_ANNOUNCEMENT(""); // **************** UNDO THINGY ****************
}

void CMainWindow::_timeStepConfigViaToolbar(int index)
{
    App::currentWorld->simulation->setDefaultSimulationParameterIndex(index);
    POST_SCENE_CHANGED_ANNOUNCEMENT(""); // **************** UNDO THINGY ****************
}

void CMainWindow::_simPopupMessageHandler(int id)
{
    if (VMenubar::doNotExecuteCommandButMemorizeIt)
    {
        VMenubar::memorizedCommand=id;
        VMenubar::doNotExecuteCommandButMemorizeIt=false;
        return;
    }
    _simMessageHandler(id);
}

void CMainWindow::_simMessageHandler(int id)
{
    bool processed=false;
    processed=CToolBarCommand::processCommand(id);
    if (!processed)
        processed=CFileOperations::processCommand(id);
    if (!processed)
        processed=CSceneObjectOperations::processCommand(id);
    if (!processed)
        processed=editModeContainer->processCommand(id,nullptr);
    if (!processed)
        processed=CAddOperations::processCommand(id,nullptr);
    if (!processed)
        processed=App::currentWorld->simulation->processCommand(id);
    if (!processed)
        processed=App::worldContainer->addOnScriptContainer->processCommand(id);
    if (!processed)
        processed=dlgCont->processCommand(id);
    if (!processed)
        processed=CHelpMenu::processCommand(id);
    if (!processed)
        processed=App::worldContainer->processGuiCommand(id);
    if (!processed)
        processed=App::currentWorld->environment->processGuiCommand(id);
    if (!processed)
        processed=customMenuBarItemContainer->processCommand(id);
    App::setToolbarRefreshFlag();
}

void CMainWindow::_aboutToShowFileSystemMenu()
{
    _fileSystemMenu->clear();
    CFileOperations::addMenu(_fileSystemMenu);
}

void CMainWindow::_aboutToShowEditSystemMenu()
{
    _editSystemMenu->clear();
    if (editModeContainer->getEditModeType()==NO_EDIT_MODE)
        CSceneObjectOperations::addMenu(_editSystemMenu);
    else
        editModeContainer->addMenu(_editSystemMenu,nullptr);
}

void CMainWindow::_aboutToShowAddSystemMenu()
{
    _addSystemMenu->clear();
    CAddOperations::addMenu(_addSystemMenu,nullptr,false);
}

void CMainWindow::_aboutToShowSimulationSystemMenu()
{
    _simulationSystemMenu->clear();
    App::currentWorld->simulation->addMenu(_simulationSystemMenu);
}

void CMainWindow::_aboutToShowAddOnSystemMenu()
{
    _addOnSystemMenu->clear();
    App::worldContainer->addOnScriptContainer->addMenu(_addOnSystemMenu);
}

void CMainWindow::_aboutToShowWindowSystemMenu()
{
    _windowSystemMenu->clear();
    dlgCont->addMenu(_windowSystemMenu);
}

void CMainWindow::_aboutToShowHelpSystemMenu()
{
    _helpSystemMenu->clear();
    CHelpMenu::addMenu(_helpSystemMenu);
}

void CMainWindow::_aboutToShowInstancesSystemMenu()
{
    _instancesSystemMenu->clear();
    App::worldContainer->addMenu(_instancesSystemMenu);
}

void CMainWindow::_aboutToShowLayoutSystemMenu()
{
    _layoutSystemMenu->clear();
    App::currentWorld->environment->addLayoutMenu(_layoutSystemMenu);
}

void CMainWindow::_aboutToShowJobsSystemMenu()
{
    _jobsSystemMenu->clear();
    App::currentWorld->environment->addJobsMenu(_jobsSystemMenu);
}

void CMainWindow::_aboutToShowCustomMenu()
{
    customMenuBarItemContainer->_menuHandle->clear();
    customMenuBarItemContainer->addMenus(customMenuBarItemContainer->_menuHandle);
}

void CMainWindow::statusbarSplitterMoved(int pos,int index)
{
    if (_statusbarSplitter->sizes()[1]<=60)
        statusBar->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    else
        statusBar->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
}

void CMainWindow::onKeyPress(SMouseOrKeyboardOrResizeEvent e)
{
    TRACE_INTERNAL;
    if (e.specialKey!=-1)
        oglSurface->keyPress(e.specialKey,this);
    else
    {
        bool processed=false;
        if (e.key==Qt::Key_Control)
        {
            setKeyDownState(getKeyDownState()|1);
            processed=true;
        }
        if (e.key==Qt::Key_Shift)
        {
            setKeyDownState(getKeyDownState()|2);
            processed=true;
        }
        if (e.key==Qt::Key_Up)
        {
            setKeyDownState(getKeyDownState()|4);
            if (getOpenGlDisplayEnabled())
                oglSurface->keyPress(UP_KEY,this);
            processed=true;
        }
        if (e.key==Qt::Key_Down)
        {
            setKeyDownState(getKeyDownState()|8);
            if (getOpenGlDisplayEnabled())
                oglSurface->keyPress(DOWN_KEY,this);
            processed=true;
        }
        if (e.key==Qt::Key_Left)
        {
            setKeyDownState(getKeyDownState()|16);
            if (getOpenGlDisplayEnabled())
                oglSurface->keyPress(LEFT_KEY,this);
            processed=true;
        }
        if (e.key==Qt::Key_Right)
        {
            setKeyDownState(getKeyDownState()|32);
            if (getOpenGlDisplayEnabled())
                oglSurface->keyPress(RIGHT_KEY,this);
            processed=true;
        }
        if (e.key==Qt::Key_Delete)
        {
            if (getOpenGlDisplayEnabled())
                oglSurface->keyPress(DELETE_KEY,this);
            processed=true;
        }
        if (e.key==Qt::Key_Escape)
        {
            if (!getOpenGlDisplayEnabled())
                setOpenGlDisplayEnabled(true);  // Esc enables the display again
            oglSurface->setFocusObject(FOCUS_ON_PAGE);
            setFocusObject(oglSurface->getFocusObject());
            oglSurface->keyPress(ESC_KEY,this);
            processed=true;
        }
        if (e.key==Qt::Key_Tab)
        {
            if (getOpenGlDisplayEnabled())
                oglSurface->keyPress(TAB_KEY,this);
            processed=true;
        }
        if ((e.key==Qt::Key_Enter)||(e.key==Qt::Key_Return))
        {
            if (getOpenGlDisplayEnabled())
                oglSurface->keyPress(ENTER_KEY,this);
            processed=true;
        }
        if (e.key==Qt::Key_Backspace)
        {
            if (getOpenGlDisplayEnabled())
                oglSurface->keyPress(BACKSPACE_KEY,this);
            processed=true;
        }
        if (getKeyDownState()&1)
        { // Very specific to CoppeliaSim, except for ctrl+Q (which doesn't exist by default on Windows)
            if (e.key==81)
            {
                oglSurface->keyPress(CTRL_Q_KEY,this);
                processed=true;
            }
            if (e.key==32)
            {
                oglSurface->keyPress(CTRL_SPACE_KEY,this);
                processed=true;
            }
            if (e.key==68)
            {
                oglSurface->keyPress(CTRL_D_KEY,this);
                processed=true;
            }
            if (e.key==71)
            {
                oglSurface->keyPress(CTRL_G_KEY,this);
                processed=true;
            }
            if (e.key==69)
            {
                oglSurface->keyPress(CTRL_E_KEY,this);
                processed=true;
            }
            if (e.key==66)
            {
                CToolBarCommand::processCommand(CAMERA_SHIFT_TO_FRAME_SELECTION_CMD);
                processed=true;
            }
        }

        if (!processed)
        {
            QByteArray ba(e.unicodeText.toLatin1());
            if (ba.length()>=1)
            {
                if (getOpenGlDisplayEnabled())
                    oglSurface->keyPress(int(ba.at(0)),this);
            }
        }
    }
}

void CMainWindow::onKeyRelease(SMouseOrKeyboardOrResizeEvent e)
{
    TRACE_INTERNAL;
    int key=e.key;
    if (key==Qt::Key_Control)
        setKeyDownState(getKeyDownState()&(0xffff-1));
    if (key==Qt::Key_Shift)
        setKeyDownState(getKeyDownState()&(0xffff-2));
    if (key==Qt::Key_Up)
        setKeyDownState(getKeyDownState()&(0xffff-4));
    if (key==Qt::Key_Down)
        setKeyDownState(getKeyDownState()&(0xffff-8));
    if (key==Qt::Key_Left)
        setKeyDownState(getKeyDownState()&(0xffff-16));
    if (key==Qt::Key_Right)
        setKeyDownState(getKeyDownState()&(0xffff-32));
}

void CMainWindow::setDefaultMouseMode()
{
    TRACE_INTERNAL;
    setMouseMode(DEFAULT_MOUSE_MODE);
}

int CMainWindow::getMouseMode()
{
//    sim_navigation_clickselection
    return(_mouseMode);
}

void CMainWindow::setMouseMode(int mm)
{ // can be called by any thread
    TRACE_INTERNAL;
    if ( (!oglSurface->isViewSelectionActive())&&(!oglSurface->isPageSelectionActive()) )
    {
        _mouseMode=mm;
        int bla=mm&0x00f;
        if ((bla==sim_navigation_objectshift)||(bla==sim_navigation_objectrotate))
        {
            openOrBringDlgToFront(TRANSLATION_ROTATION_DLG);
            activateMainWindow();
        }
        else
            closeDlg(TRANSLATION_ROTATION_DLG);

        App::setToolbarRefreshFlag();
        App::setFullDialogRefreshFlag();
    }
}

void CMainWindow::setLightDialogRefreshFlag()
{
    _lightDialogRefreshFlag=true;
    _toolbarRefreshFlag=true;
}

void CMainWindow::setFullDialogRefreshFlag()
{
    _fullDialogRefreshFlag=true;
    _toolbarRefreshFlag=true;
}

void CMainWindow::setDialogRefreshDontPublishFlag()
{
    _dialogRefreshDontPublishFlag=true;
}

void CMainWindow::setToolbarRefreshFlag()
{
    _toolbarRefreshFlag=true;
}

void CMainWindow::newInstanceAboutToBeCreated()
{
    TRACE_INTERNAL;
    if (_sceneHierarchyWidgetList.size()>0)
    {
        sceneHierarchyWidget=new CSceneHierarchyWidget();
        _sceneHierarchyWidgetList.push_back(sceneHierarchyWidget);
        sceneHierarchyLayout->addWidget(sceneHierarchyWidget);
        sceneHierarchyWidget->setVisible(false);//here_for_new_hierarchy
    }
    if (codeEditorContainer!=nullptr)
        codeEditorContainer->showOrHideAll(false);
}

void CMainWindow::newInstanceWasJustCreated()
{
    TRACE_INTERNAL;
    tabBar->addTab(App::currentWorld->mainSettings->getSceneNameForUi().c_str());
    tabBar->setCurrentIndex(App::worldContainer->getCurrentWorldIndex());
}

void CMainWindow::instanceAboutToBeDestroyed(int currentInstanceIndex)
{
    TRACE_INTERNAL;
    codeEditorContainer->sceneClosed(App::currentWorld->environment->getSceneUniqueID());

    sceneHierarchyLayout->removeWidget(sceneHierarchyWidget);
    delete sceneHierarchyWidget;
    sceneHierarchyWidget=nullptr;
    _sceneHierarchyWidgetList.erase(_sceneHierarchyWidgetList.begin()+currentInstanceIndex);

    tabBar->removeTab(currentInstanceIndex);
}

void CMainWindow::instanceAboutToChange(int newInstanceIndex)
{
    TRACE_INTERNAL;
    if (codeEditorContainer!=nullptr)
        codeEditorContainer->showOrHideAll(false);
    if (sceneHierarchyWidget!=nullptr)
        sceneHierarchyWidget->setVisible(false);

    _flyModeCameraHandle=-1;
}

void CMainWindow::instanceHasChanged(int newInstanceIndex)
{
    TRACE_INTERNAL;
    if ( (newInstanceIndex>=0)&&(newInstanceIndex<int(_sceneHierarchyWidgetList.size())) )
        sceneHierarchyWidget=_sceneHierarchyWidgetList[newInstanceIndex];

    if (tabBar->currentIndex()!=newInstanceIndex)
        tabBar->setCurrentIndex(newInstanceIndex);

    setOpenGlDisplayEnabled(true);
    if (codeEditorContainer!=nullptr)
        codeEditorContainer->showOrHideAll(true);
}


void CMainWindow::newSceneNameWasSet(const char* name)
{
    tabBar->setTabText(tabBar->currentIndex(),name);
}

void CMainWindow::tabBarIndexChanged(int newIndex)
{
    App::worldContainer->processGuiCommand(SWITCH_TOINSTANCEWITHTHUMBNAILSAVEINDEX0_GUIGUICMD+newIndex);
}

void CMainWindow::_closeDialogTemporarilyIfOpened(int dlgID,std::vector<int>& vect)
{
    if (App::mainWindow->dlgCont->isVisible(dlgID))
    {
        App::mainWindow->dlgCont->close(dlgID);
        vect.push_back(dlgID);
    }
}

void CMainWindow::closeTemporarilyDialogsForPageSelector()
{
    if (VThread::isCurrentThreadTheUiThread())
    { // we are in the UI thread. We execute the command now:
        if (codeEditorContainer!=nullptr)
            codeEditorContainer->showOrHideAll(false);
        _closeDialogTemporarilyIfOpened(SETTINGS_DLG,_dialogsClosedTemporarily_pageSelector);
        _closeDialogTemporarilyIfOpened(SELECTION_DLG,_dialogsClosedTemporarily_pageSelector);
        _closeDialogTemporarilyIfOpened(SIMULATION_DLG,_dialogsClosedTemporarily_pageSelector);
        _closeDialogTemporarilyIfOpened(ENVIRONMENT_DLG,_dialogsClosedTemporarily_pageSelector);
        _closeDialogTemporarilyIfOpened(COLLECTION_DLG,_dialogsClosedTemporarily_pageSelector);
        _closeDialogTemporarilyIfOpened(LUA_SCRIPT_DLG,_dialogsClosedTemporarily_pageSelector);
        _closeDialogTemporarilyIfOpened(OBJECT_DLG,_dialogsClosedTemporarily_pageSelector);
        _closeDialogTemporarilyIfOpened(CALCULATION_DLG,_dialogsClosedTemporarily_pageSelector);
        _closeDialogTemporarilyIfOpened(TRANSLATION_ROTATION_DLG,_dialogsClosedTemporarily_pageSelector);
        _closeDialogTemporarilyIfOpened(BROWSER_DLG,_dialogsClosedTemporarily_pageSelector);

        dlgCont->close(COLOR_DLG);
        dlgCont->close(LIGHTMATERIAL_DLG);
        dlgCont->close(MATERIAL_DLG);
        dlgCont->close(FOG_DLG);
        dlgCont->close(TEXTURE_DLG);
    }
    else
    { // We are NOT in the UI thread. We execute the command via the UI thread:
        SUIThreadCommand cmdIn;
        SUIThreadCommand cmdOut;
        cmdIn.cmdId=MAIN_WINDOW_PAGE_SELECTOR_DLG_CLOSE_MWUITHREADCMD;
        App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
    }
}

void CMainWindow::reopenTemporarilyClosedDialogsForPageSelector()
{
    if (VThread::isCurrentThreadTheUiThread())
    { // we are in the UI thread. We execute the command now:
        for (int i=0;i<int(_dialogsClosedTemporarily_pageSelector.size());i++)
            dlgCont->openOrBringToFront(_dialogsClosedTemporarily_pageSelector[i]);
        _dialogsClosedTemporarily_pageSelector.clear();
        if (codeEditorContainer!=nullptr)
            codeEditorContainer->showOrHideAll(true);
    }
    else
    { // We are NOT in the UI thread. We execute the command via the UI thread:
        SUIThreadCommand cmdIn;
        SUIThreadCommand cmdOut;
        cmdIn.cmdId=MAIN_WINDOW_PAGE_SELECTOR_DLG_REOPEN_MWUITHREADCMD;
        App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
    }
}


void CMainWindow::closeTemporarilyDialogsForViewSelector()
{
    if (VThread::isCurrentThreadTheUiThread())
    { // we are in the UI thread. We execute the command now:
        if (codeEditorContainer!=nullptr)
            codeEditorContainer->showOrHideAll(false);
        _closeDialogTemporarilyIfOpened(SETTINGS_DLG,_dialogsClosedTemporarily_viewSelector);
        _closeDialogTemporarilyIfOpened(SELECTION_DLG,_dialogsClosedTemporarily_viewSelector);
        _closeDialogTemporarilyIfOpened(SIMULATION_DLG,_dialogsClosedTemporarily_viewSelector);
        _closeDialogTemporarilyIfOpened(ENVIRONMENT_DLG,_dialogsClosedTemporarily_viewSelector);
        _closeDialogTemporarilyIfOpened(COLLECTION_DLG,_dialogsClosedTemporarily_viewSelector);
        _closeDialogTemporarilyIfOpened(LUA_SCRIPT_DLG,_dialogsClosedTemporarily_viewSelector);
        _closeDialogTemporarilyIfOpened(OBJECT_DLG,_dialogsClosedTemporarily_viewSelector);
        _closeDialogTemporarilyIfOpened(CALCULATION_DLG,_dialogsClosedTemporarily_viewSelector);
        _closeDialogTemporarilyIfOpened(TRANSLATION_ROTATION_DLG,_dialogsClosedTemporarily_viewSelector);
        _closeDialogTemporarilyIfOpened(BROWSER_DLG,_dialogsClosedTemporarily_viewSelector);

        dlgCont->close(COLOR_DLG);
        dlgCont->close(LIGHTMATERIAL_DLG);
        dlgCont->close(MATERIAL_DLG);
        dlgCont->close(FOG_DLG);
        dlgCont->close(TEXTURE_DLG);
    }
    else
    { // We are NOT in the UI thread. We execute the command via the UI thread:
        SUIThreadCommand cmdIn;
        SUIThreadCommand cmdOut;
        cmdIn.cmdId=MAIN_WINDOW_VIEW_SELECTOR_DLG_CLOSE_MWUITHREADCMD;
        App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
    }
}

void CMainWindow::reopenTemporarilyClosedDialogsForViewSelector()
{
    if (VThread::isCurrentThreadTheUiThread())
    { // we are in the UI thread. We execute the command now:
        for (int i=0;i<int(_dialogsClosedTemporarily_viewSelector.size());i++)
            dlgCont->openOrBringToFront(_dialogsClosedTemporarily_viewSelector[i]);
        _dialogsClosedTemporarily_viewSelector.clear();
        if (codeEditorContainer!=nullptr)
            codeEditorContainer->showOrHideAll(true);
    }
    else
    { // We are NOT in the UI thread. We execute the command via the UI thread:
        SUIThreadCommand cmdIn;
        SUIThreadCommand cmdOut;
        cmdIn.cmdId=MAIN_WINDOW_VIEW_SELECTOR_DLG_REOPEN_MWUITHREADCMD;
        App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
    }
}

void CMainWindow::reopenTemporarilyClosedNonEditModeDialogs()
{
    if (VThread::isCurrentThreadTheUiThread())
    { // we are in the UI thread. We execute the command now:
        for (int i=0;i<int(_dialogsClosedTemporarily_editModes.size());i++)
            dlgCont->openOrBringToFront(_dialogsClosedTemporarily_editModes[i]);
        _dialogsClosedTemporarily_editModes.clear();
        if (codeEditorContainer!=nullptr)
            codeEditorContainer->showOrHideAll(true);
    }
    else
    { // We are NOT in the UI thread. We execute the command via the UI thread:
        SUIThreadCommand cmdIn;
        SUIThreadCommand cmdOut;
        cmdIn.cmdId=MAIN_WINDOW_NON_EDIT_MODE_DLG_REOPEN_MWUITHREADCMD;
        App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
    }
}

void CMainWindow::closeTemporarilyNonEditModeDialogs()
{
    if (VThread::isCurrentThreadTheUiThread())
    { // we are in the UI thread. We execute the command now:
        if (codeEditorContainer!=nullptr)
            codeEditorContainer->showOrHideAll(false);
        _closeDialogTemporarilyIfOpened(SETTINGS_DLG,_dialogsClosedTemporarily_editModes);
        _closeDialogTemporarilyIfOpened(SELECTION_DLG,_dialogsClosedTemporarily_editModes);
        _closeDialogTemporarilyIfOpened(SIMULATION_DLG,_dialogsClosedTemporarily_editModes);
        _closeDialogTemporarilyIfOpened(ENVIRONMENT_DLG,_dialogsClosedTemporarily_editModes);
        _closeDialogTemporarilyIfOpened(COLLECTION_DLG,_dialogsClosedTemporarily_editModes);
        _closeDialogTemporarilyIfOpened(LUA_SCRIPT_DLG,_dialogsClosedTemporarily_editModes);
        _closeDialogTemporarilyIfOpened(OBJECT_DLG,_dialogsClosedTemporarily_editModes);
        _closeDialogTemporarilyIfOpened(CALCULATION_DLG,_dialogsClosedTemporarily_editModes);
        _closeDialogTemporarilyIfOpened(BROWSER_DLG,_dialogsClosedTemporarily_editModes);

        dlgCont->close(COLOR_DLG);
        dlgCont->close(LIGHTMATERIAL_DLG);
        dlgCont->close(MATERIAL_DLG);
        dlgCont->close(FOG_DLG);
        dlgCont->close(TEXTURE_DLG);
    }
    else
    { // We are NOT in the UI thread. We execute the command via the UI thread:
        SUIThreadCommand cmdIn;
        SUIThreadCommand cmdOut;
        cmdIn.cmdId=MAIN_WINDOW_NON_EDIT_MODE_DLG_CLOSE_MWUITHREADCMD;
        App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
    }
}

void CMainWindow::openOrBringDlgToFront(int dlgId)
{
    TRACE_INTERNAL;
    if (VThread::isCurrentThreadTheUiThread())
    { // we are in the UI thread. We execute the command now:
        dlgCont->openOrBringToFront(dlgId);
    }
    else
    { // We are NOT in the UI thread. We execute the command via the UI thread:
        SUIThreadCommand cmdIn;
        SUIThreadCommand cmdOut;
        cmdIn.cmdId=MAIN_WINDOW_OPEN_DLG_OR_BRING_TO_FRONT_MWUITHREADCMD;
        cmdIn.intParams.push_back(dlgId);
        App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
    }
}

void CMainWindow::closeDlg(int dlgId)
{
    TRACE_INTERNAL;
    if (VThread::isCurrentThreadTheUiThread())
    { // we are in the UI thread. We execute the command now:
        dlgCont->close(dlgId);
    }
    else
    { // We are NOT in the UI thread. We execute the command via the UI thread:
        SUIThreadCommand cmdIn;
        SUIThreadCommand cmdOut;
        cmdIn.cmdId=MAIN_WINDOW_CLOSE_DLG_MWUITHREADCMD;
        cmdIn.intParams.push_back(dlgId);
        App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
    }
}

void CMainWindow::activateMainWindow()
{
    TRACE_INTERNAL;
    if (VThread::isCurrentThreadTheUiThread())
    { // we are in the UI thread. We execute the command now:
        activateWindow();
    }
    else
    { // We are NOT in the UI thread. We execute the command via the UI thread:
        SUIThreadCommand cmdIn;
        SUIThreadCommand cmdOut;
        cmdIn.cmdId=MAIN_WINDOW_ACTIVATE_MWUITHREADCMD;
        App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
    }
}

void CMainWindow::executeCommand(SUIThreadCommand* cmdIn,SUIThreadCommand* cmdOut)
{
    if (cmdIn->cmdId==MAIN_WINDOW_NON_EDIT_MODE_DLG_CLOSE_MWUITHREADCMD)
        closeTemporarilyNonEditModeDialogs();
    if (cmdIn->cmdId==MAIN_WINDOW_NON_EDIT_MODE_DLG_REOPEN_MWUITHREADCMD)
        reopenTemporarilyClosedNonEditModeDialogs();
    if (cmdIn->cmdId==MAIN_WINDOW_VIEW_SELECTOR_DLG_CLOSE_MWUITHREADCMD)
        closeTemporarilyDialogsForViewSelector();
    if (cmdIn->cmdId==MAIN_WINDOW_VIEW_SELECTOR_DLG_REOPEN_MWUITHREADCMD)
        reopenTemporarilyClosedDialogsForViewSelector();
    if (cmdIn->cmdId==MAIN_WINDOW_PAGE_SELECTOR_DLG_CLOSE_MWUITHREADCMD)
        closeTemporarilyDialogsForPageSelector();
    if (cmdIn->cmdId==MAIN_WINDOW_PAGE_SELECTOR_DLG_REOPEN_MWUITHREADCMD)
        reopenTemporarilyClosedDialogsForPageSelector();
    if (cmdIn->cmdId==MAIN_WINDOW_SET_FULLSCREEN_MWTHREADCMD)
        setFullScreen(cmdIn->boolParams[0]);
    if (cmdIn->cmdId==MAIN_WINDOW_ACTIVATE_MWUITHREADCMD)
        activateMainWindow();
    if (cmdIn->cmdId==MAIN_WINDOW_CLOSE_DLG_MWUITHREADCMD)
        closeDlg(cmdIn->intParams[0]);
    if (cmdIn->cmdId==MAIN_WINDOW_OPEN_DLG_OR_BRING_TO_FRONT_MWUITHREADCMD)
        openOrBringDlgToFront(cmdIn->intParams[0]);
}
