#pragma once

#include <QMainWindow>
#include <QToolBar>
#include <QComboBox>
#include <QToolButton>
#include <QSignalMapper>
#include <QSplitter>
#include <QMenu>
#include <QAction>
#include <QComboBox>
#include <QBoxLayout>
#include "statusBar.h"
#include "mainWindowBase.h"
#include "qvrep_openglwidget.h"
#include "global.h"
#include "dlgCont.h"
#include "customMenuBarItemContainer.h"
#include "simRecorder.h"
#include "VPoint.h"
#include "oglSurface.h"
#include "editModeContainer.h"
#include "codeEditorContainer.h"
#include "simThread.h"
#include "modelFolderWidget.h"
#include "modelListWidget.h"
#include "sceneHierarchyWidget.h"

enum {FOCUS_ON_PAGE=0,FOCUS_ON_HIERARCHY,FOCUS_ON_SOFT_DIALOG,FOCUS_ON_SCENE_SELECTION_WINDOW,FOCUS_ON_VIEW_SELECTION_WINDOW,FOCUS_ON_PAGE_SELECTION_WINDOW,FOCUS_ON_UNKNOWN_OBJECT,FOCUS_ON_BROWSER};

struct SSceneThumbnail
{
    unsigned char* textureData;
    int textureResolution[2];
};


class CMainWindow : public QMainWindow, public CMainWindowBase
{
    Q_OBJECT
public:
    CMainWindow();
    virtual ~CMainWindow();

    void executeCommand(SUIThreadCommand* cmdIn,SUIThreadCommand* cmdOut);
    void initializeWindow();

    bool event(QEvent* event);
    void dragEnterEvent(QDragEnterEvent* dEvent);
    void dropEvent(QDropEvent* dEvent);
    QMenu* createPopupMenu() { return nullptr; } // required to avoid having a popup when over the toolbar (and other situations). 4/12/2011

    void refreshDimensions();
    void setWindowDimensions(int x,int y,bool clientSurface,bool maximized);
    void windowResizeEvent(int x,int y);
    bool isFullScreen();
    void setFullScreen(bool f);
    void setOpenGlDisplayEnabled(bool e);
    bool getOpenGlDisplayEnabled();
    void setFocusObject(int f);
    void setCurrentCursor(int cur);
    void setBrowserVisible(bool v);

    void flashStatusbar();
    void setFlyModeCameraHandle(int h);
    int getFlyModeCameraHandle();
    void setProxSensorClickSelectDown(int v);
    int getProxSensorClickSelectDown();
    void setProxSensorClickSelectUp(int v);
    int getProxSensorClickSelectUp();

    bool getHasStereo();
    void setStereoDistance(float d);
    float getStereoDistance();
    bool getLeftEye();

    void createDefaultMenuBar();
    void removeDefaultMenuBar();

    int getMouseButtonState();
    void setMouseButtonState(int state);
    void getMouseRenderingPos(int pos[2]);

    int getKeyDownState();
    void setKeyDownState(int state);

    int getMouseMode();
    void setMouseMode(int mm);
    void setDefaultMouseMode();

    void setLightDialogRefreshFlag();
    void setFullDialogRefreshFlag();
    void setDialogRefreshDontPublishFlag();
    void setToolbarRefreshFlag();
    void activateMainWindow();
    void closeDlg(int dlgId);
    void openOrBringDlgToFront(int dlgId);
    bool prepareSceneThumbnail(const SSimulationThreadCommand& command);
    unsigned char* getSceneThumbnail(int instanceIndex,int resolution[2]);

    bool getObjectShiftToggleViaGuiEnabled();
    void setObjectShiftToggleViaGuiEnabled(bool e);
    bool getObjectRotateToggleViaGuiEnabled();
    void setObjectRotateToggleViaGuiEnabled(bool e);
    void setHierarchyToggleViaGuiEnabled(bool e);
    bool getHierarchyToggleViaGuiEnabled();
    void setBrowserToggleViaGuiEnabled(bool e);
    bool getBrowserToggleViaGuiEnabled();
    void setObjPropToggleViaGuiEnabled(bool e);
    bool getObjPropToggleViaGuiEnabled();
    void setCalcModulesToggleViaGuiEnabled(bool e);
    bool getCalcModulesToggleViaGuiEnabled();
    void setPlayViaGuiEnabled(bool e);
    bool getPlayViaGuiEnabled();
    void setPauseViaGuiEnabled(bool e);
    bool getPauseViaGuiEnabled();
    void setStopViaGuiEnabled(bool e);
    bool getStopViaGuiEnabled();

//------------------------
    void uiThread_renderScene(bool bufferMainDisplayStateVariables);
    void uiThread_renderScene_noLock(bool bufferMainDisplayStateVariables);
    void simThread_prepareToRenderScene();
    void refreshDialogs_uiThread();
    void callDialogFunction(const SUIThreadCommand* cmdIn,SUIThreadCommand* cmdOut);
//------------------------

    QSignalMapper* getPopupSignalMapper();
    COpenglWidget* openglWidget;
    CStatusBar* statusBar;
    CModelFolderWidget* modelFolderWidget;
    CModelListWidget* modelListWidget;
    CSceneHierarchyWidget* sceneHierarchyWidget;
    QHBoxLayout* sceneHierarchyLayout;
    QTabBar* tabBar;

    CDlgCont* dlgCont;
    CCustomMenuBarItemContainer* customMenuBarItemContainer;
    CSimRecorder* simulationRecorder;
    COglSurface* oglSurface;
    CEditModeContainer* editModeContainer;
    CCodeEditorContainer* codeEditorContainer;

    void onLeftMouseButtonDownTT(int xPos,int yPos);
    void onMiddleMouseButtonDownTT(int xPos,int yPos);
    void onRightMouseButtonDownTT(int xPos,int yPos);
    void onLeftMouseButtonDoubleClickTT(int xPos,int yPos);
    void onLeftMouseButtonUpTT(int xPos,int yPos);
    void onMiddleMouseButtonUpTT(int xPos,int yPos);
    void onRightMouseButtonUpTT(int xPos,int yPos);
    void onMouseMoveTT(int xPos,int yPos);
    void onWheelRotateTT(int delta,int xPos,int yPos);
    int modelDragMoveEvent(int xPos,int yPos,C3Vector* desiredModelPosition);

    void onKeyPress(SMouseOrKeyboardOrResizeEvent e);
    void onKeyRelease(SMouseOrKeyboardOrResizeEvent e);

    void simulationAboutToStart();
    void simulationEnded();
    void editModeAboutToStart();
    void editModeEnded();

    void newInstanceAboutToBeCreated();
    void newInstanceWasJustCreated();
    void instanceAboutToBeDestroyed(int currentInstanceIndex);
    void instanceAboutToChange(int newInstanceIndex);
    void instanceHasChanged(int newInstanceIndex);
    void newSceneNameWasSet(const char* name);

    void closeTemporarilyNonEditModeDialogs();
    void reopenTemporarilyClosedNonEditModeDialogs();
    void closeTemporarilyDialogsForViewSelector();
    void reopenTemporarilyClosedDialogsForViewSelector();
    void closeTemporarilyDialogsForPageSelector();
    void reopenTemporarilyClosedDialogsForPageSelector();
    void closeTemporarilyDialogsForSceneSelector();
    void reopenTemporarilyClosedDialogsForSceneSelector();

private:
    int _renderOpenGlContent_callFromRenderingThreadOnly();
    void _actualizetoolbarButtonState();
    void _dropFilesIntoScene(const std::vector<std::string>& tttFiles,const std::vector<std::string>& ttmFiles);
    void _createDefaultToolBars();
    void _recomputeClientSizeAndPos();
    void _setInitialDimensions(bool maximized);
    void _setClientArea(int x,int y);
    void _resetStatusbarFlashIfNeeded();
    void _closeDialogTemporarilyIfOpened(int dlgID,std::vector<int>& vect);

    QSignalMapper* _signalMapper;
    QSignalMapper* _popupSignalMapper;
    QSplitter* _statusbarSplitter;
    QSplitter* _modelBrowserSplitter;
    QSplitter* _sceneHierarchySplitter;
    QSplitter* _modelBrowser;

    VMenubar* _menubar;
    VMenu* _fileSystemMenu;
    VMenu* _editSystemMenu;
    VMenu* _addSystemMenu;
    VMenu* _simulationSystemMenu;
    VMenu* _windowSystemMenu;
    VMenu* _addOnSystemMenu;
    VMenu* _helpSystemMenu;
    VMenu* _instancesSystemMenu;
    VMenu* _layoutSystemMenu;
    VMenu* _jobsSystemMenu;
    VMenu* _sSystemMenu;


    std::vector<SSceneThumbnail> _sceneThumbnails;
    std::vector<CSceneHierarchyWidget*> _sceneHierarchyWidgetList;


    QToolBar* _toolbar1;
    QToolBar* _toolbar2;
    QAction* _toolbarActionCameraShift;
    QAction* _toolbarActionCameraRotate;
    QAction* _toolbarActionCameraZoom;
    QAction* _toolbarActionCameraAngle;
    QAction* _toolbarActionCameraSizeToScreen;
//    QAction* _toolbarActionCameraFly;
    QAction* _toolbarActionObjectShift;
    QAction* _toolbarActionObjectRotate;

    #ifdef NEWIKFUNC
        QAction* _toolbarActionIk;
    #endif

    QAction* _toolbarActionClickSelection;
    QAction* _toolbarActionAssemble;
    QAction* _toolbarActionTransferDna;
    QAction* _toolbarActionUndo;
    QAction* _toolbarActionRedo;
    QAction* _toolbarActionDynamicContentVisualization;
    QComboBox* _engineSelectCombo;
    QComboBox* _enginePrecisionCombo;
    QComboBox* _timeStepConfigCombo;
    QAction* _toolbarActionVerify;
    QAction* _toolbarActionStart;
    QAction* _toolbarActionPause;
    QAction* _toolbarActionStop;
    QAction* _toolbarActionRealTime;
    QAction* _toolbarActionOnline;
    QAction* _toolbarActionReduceSpeed;
    QAction* _toolbarActionIncreaseSpeed;
    QAction* _toolbarActionThreadedRendering;
    QAction* _toolbarActionToggleVisualization;
    QAction* _toolbarActionPageSelector;
    QAction* _toolbarActionSceneSelector;
    QAction* _toolbarActionSimulationSettings;
    QAction* _toolbarActionObjectProperties;
    QAction* _toolbarActionCalculationModules;
    QAction* _toolbarActionCollections;
    QAction* _toolbarActionScripts;
    QAction* _toolbarActionShapeEdition;
    QAction* _toolbarAction2dElements;
    QAction* _toolbarActionPathEdition;
    QAction* _toolbarActionSelection;
    QAction* _toolbarActionModelBrowser;
    QAction* _toolbarActionSceneHierarchy;
    QAction* _toolbarActionLayers;
    QAction* _toolbarActionAviRecorder;
    QAction* _toolbarActionUserSettings;

    std::vector<int> _dialogsClosedTemporarily_editModes;
    std::vector<int> _dialogsClosedTemporarily_viewSelector;
    std::vector<int> _dialogsClosedTemporarily_pageSelector;
    std::vector<int> _dialogsClosedTemporarily_sceneSelector;

    int _mouseButtonsState; // 1=left, 2=wheel activity, 4=right, 8=middle wheel down, 16=last mouse down was left and not ctrl pressed
    int _keyDownState; // 1=ctrl, 2=shift, 4=up, 8=down, 16=left, 32=right
    int _renderingTimeInMs;
    float _fps;
    bool _fullscreen;
    bool _hasStereo;
    float _stereoDistance;
    bool _leftEye;
    int _statusbarFlashTime;
    VPoint _mouseRenderingPos;
    VPoint _clientArea;
    int _focusObject;
    int _currentCursor;
    int _mouseWheelEventTime;
    bool _openGlDisplayEnabled;
    int _mouseMode;
    int _flyModeCameraHandle;
    int _proxSensorClickSelectDown;
    int _proxSensorClickSelectUp;
    SSimulationThreadCommand _prepareSceneThumbnailCmd;


    bool _lightDialogRefreshFlag;
    bool _fullDialogRefreshFlag;
    bool _dialogRefreshDontPublishFlag;
    bool _toolbarRefreshFlag;

    bool _toolbarButtonObjectShiftEnabled;
    bool _toolbarButtonObjectRotateEnabled;
    bool _toolbarButtonHierarchyEnabled;
    bool _toolbarButtonBrowserEnabled;
    bool _toolbarButtonPlayEnabled;
    bool _toolbarButtonPauseEnabled;
    bool _toolbarButtonStopEnabled;
    bool _toolbarButtonObjPropEnabled;
    bool _toolbarButtonCalcModulesEnabled;

    int lastInstance;
    int timeCounter;
    int lastTimeRenderingStarted;
    int previousDisplayWasEnabled;
    int previousCursor;

public slots:
    void _engineSelectedViaToolbar(int index);
    void _enginePrecisionViaToolbar(int index);
    void _timeStepConfigViaToolbar(int index);

    void _vrepPopupMessageHandler(int id);
    void _vrepMessageHandler(int id);

    void _aboutToShowFileSystemMenu();
    void _aboutToShowEditSystemMenu();
    void _aboutToShowAddSystemMenu();
    void _aboutToShowSimulationSystemMenu();
    void _aboutToShowWindowSystemMenu();
    void _aboutToShowAddOnSystemMenu();
    void _aboutToShowHelpSystemMenu();
    void _aboutToShowInstancesSystemMenu();
    void _aboutToShowCustomMenu();
    void _aboutToShowLayoutSystemMenu();
    void _aboutToShowJobsSystemMenu();
    void _aboutToShowSSystemMenu();

    void statusbarSplitterMoved(int pos,int index);

    void tabBarIndexChanged(int newIndex);
};
