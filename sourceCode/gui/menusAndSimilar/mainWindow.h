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
#include "qsim_openglwidget.h"
#include "global.h"
#include "dlgCont.h"
#include "simRecorder.h"
#include "oglSurface.h"
#include "editModeContainer.h"
#include "codeEditorContainer.h"
#include "simThread.h"
#include "modelFolderWidget.h"
#include "modelListWidget.h"

enum {FOCUS_ON_PAGE=0,FOCUS_ON_HIERARCHY,FOCUS_ON_SOFT_DIALOG,FOCUS_ON_VIEW_SELECTION_WINDOW,FOCUS_ON_PAGE_SELECTION_WINDOW,FOCUS_ON_UNKNOWN_OBJECT,FOCUS_ON_BROWSER};


class CMainWindow : public QMainWindow
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
    void setWindowDimensions(int x,int y);
    void windowResizeEvent(int x,int y);
    bool isFullScreen();
    void setFullScreen(bool f);
    void setOpenGlDisplayEnabled(bool e);
    bool getOpenGlDisplayEnabled();
    void setFocusObject(int f);
    void setCurrentCursor(int cur);
    void setBrowserVisible(bool v);

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
    void setCalcModulesToggleViaGuiEnabled_OLD(bool e);
    bool getCalcModulesToggleViaGuiEnabled_OLD();
    void setPlayViaGuiEnabled(bool e);
    bool getPlayViaGuiEnabled();
    void setPauseViaGuiEnabled(bool e);
    bool getPauseViaGuiEnabled();
    void setStopViaGuiEnabled(bool e);
    bool getStopViaGuiEnabled();

//------------------------
    void uiThread_renderScene();
    void uiThread_renderScene_noLock();
    void simThread_prepareToRenderScene();
    void refreshDialogs_uiThread();
    void callDialogFunction(const SUIThreadCommand* cmdIn,SUIThreadCommand* cmdOut);
//------------------------

    QSignalMapper* getPopupSignalMapper();
    COpenglWidget* openglWidget;
    CStatusBar* statusBar;
    CModelFolderWidget* modelFolderWidget;
    CModelListWidget* modelListWidget;
    QTabBar* tabBar;

    CDlgCont* dlgCont;
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

    void setMouseRay(const C3Vector* orig,const C3Vector* dir);
    bool getMouseRay(C3Vector& orig,C3Vector& dir);
    void mouseClickAction(bool down);
    int getMouseClickActionCounter(bool down);

private:
    int _renderOpenGlContent_callFromRenderingThreadOnly();
    void _actualizetoolbarButtonState();
    void _dropFilesIntoScene(const std::vector<std::string>& tttFiles,const std::vector<std::string>& ttmFiles);
    void _createDefaultToolBars();
    void _recomputeClientSizeAndPos();
    void _setClientArea(int x,int y);
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
    VMenu* _toolsSystemMenu;
    VMenu* _helpSystemMenu;
    VMenu* _instancesSystemMenu;



    QToolBar* _toolbar1;
    QToolBar* _toolbar2;
    QAction* _toolbarActionCameraShift;
    QAction* _toolbarActionCameraRotate;
    QAction* _toolbarActionCameraZoom;
    QAction* _toolbarActionCameraAngle;
    QAction* _toolbarActionCameraSizeToScreen;
    QAction* _toolbarActionObjectShift;
    QAction* _toolbarActionObjectRotate;

    QAction* _toolbarActionClickSelection;
    QAction* _toolbarActionAssemble;
    QAction* _toolbarActionTransferDna;
    QAction* _toolbarActionUndo;
    QAction* _toolbarActionRedo;
    QAction* _toolbarActionDynamicContentVisualization;
    QComboBox* _engineSelectCombo;
    QAction* _toolbarActionStart;
    QAction* _toolbarActionPause;
    QAction* _toolbarActionStop;
    QAction* _toolbarActionRealTime;
    QAction* _toolbarActionReduceSpeed;
    QAction* _toolbarActionIncreaseSpeed;
    QAction* _toolbarActionToggleVisualization;
    QAction* _toolbarActionPageSelector;
    QAction* _toolbarActionSimulationSettings;
    QAction* _toolbarActionObjectProperties;
    QAction* _toolbarActionCalculationModules_OLD;
    QAction* _toolbarActionCollections;
    QAction* _toolbarActionScripts;
    QAction* _toolbarActionShapeEdition;
    QAction* _toolbarActionPathEdition;
    QAction* _toolbarActionModelBrowser;
    QAction* _toolbarActionSceneHierarchy;
    QAction* _toolbarActionLayers;
    QAction* _toolbarActionAviRecorder;
    QAction* _toolbarActionUserSettings;

    std::vector<int> _dialogsClosedTemporarily_editModes;
    std::vector<int> _dialogsClosedTemporarily_viewSelector;
    std::vector<int> _dialogsClosedTemporarily_pageSelector;

    int _mouseButtonsState; // 1=left, 2=wheel activity, 4=right, 8=middle wheel down, 16=last mouse down was left and not ctrl pressed
    int _keyDownState; // 1=ctrl, 2=shift, 4=up, 8=down, 16=left, 32=right
    int _renderingTimeInMs;
    float _fps;
    bool _fullscreen;
    bool _hasStereo;
    float _stereoDistance;
    bool _leftEye;
    int _mouseRenderingPos[2];
    int _clientArea[2];
    int _focusObject;
    int _currentCursor;
    int _mouseWheelEventTime;
    bool _openGlDisplayEnabled;
    int _mouseMode;
    int _proxSensorClickSelectDown;
    int _proxSensorClickSelectUp;
    bool _mouseRayValid;
    C3Vector _mouseRayOrigin;
    C3Vector _mouseRayDirection;
    int _mouseClickActionCounter_down;
    int _mouseClickActionCounter_up;



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
    bool _toolbarButtonCalcModulesEnabled_OLD;

    int lastInstance;
    int timeCounter;
    int lastTimeRenderingStarted;
    int previousDisplayWasEnabled;
    int previousCursor;

public slots:
    void _engineSelectedViaToolbar(int index);

    void _simPopupMessageHandler(int id);
    void _simMessageHandler(int id);

    void _aboutToShowFileSystemMenu();
    void _aboutToShowEditSystemMenu();
    void _aboutToShowAddSystemMenu();
    void _aboutToShowSimulationSystemMenu();
    void _aboutToShowToolsSystemMenu();
    void _aboutToShowHelpSystemMenu();
    void _aboutToShowInstancesSystemMenu();
    void _aboutToShowCustomMenu();

    void statusbarSplitterMoved(int pos,int index);

    void tabBarIndexChanged(int newIndex);
};
