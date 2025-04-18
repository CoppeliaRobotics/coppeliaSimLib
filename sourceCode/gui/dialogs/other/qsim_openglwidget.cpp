#include <qsim_openglwidget.h>
#include <QMimeData>
#include <QMouseEvent>
#include <QWindow>
#include <QTimer>
#include <vDateTime.h>
#include <utils.h>
#include <app.h>
#include <toolBarCommand.h>
#include <rendering.h>
#include <simFlavor.h>
#ifdef SIM_WITH_GUI
#include <guiApp.h>
#endif

int disableWheelRotateForOne500ms = -1;
int disableMouseMoveFor200ms = -1;
bool ignoreLeftMouseUp = false;
bool ignoreMiddleMouseUp = false;
bool ignoreRightMouseUp = false;
int _savedMouseMode;

std::vector<SMouseOrKeyboardOrResizeEvent> _bufferedMouseOrKeyboardOrResizeEvents;

#ifdef USES_QGLWIDGET
COpenglWidget::COpenglWidget(QWidget* parent)
    : QGLWidget(QGLFormat((App::userSettings->stereoDist <= 0.0) ? (QGL::DoubleBuffer) : (QGL::StereoBuffers)), parent)
#else
COpenglWidget::COpenglWidget(QWidget* parent)
    : QOpenGLWidget(parent)
#endif
{
    TRACE_INTERNAL;

    _bufferedMouseOrKeyboardOrResizeEvents.clear();
    disableWheelRotateForOne500ms = -1;
    disableMouseMoveFor200ms = -1;
    ignoreLeftMouseUp = false;
    ignoreMiddleMouseUp = false;
    ignoreRightMouseUp = false;

    setMouseTracking(true);
    setFocusPolicy(Qt::WheelFocus);

    QTimer* timer_100ms = new QTimer(this);
    connect(timer_100ms, SIGNAL(timeout()), this, SLOT(_timer100ms_fire()));
    timer_100ms->start(100);
    _modelDragAndDropInfo = nullptr;
    setAcceptDrops(true);
}

COpenglWidget::~COpenglWidget()
{
}

void COpenglWidget::makeContextCurrent()
{
    makeCurrent();
}

void COpenglWidget::resizeEvent(QResizeEvent* rEvent)
{
    _handleMouseAndKeyboardAndResizeEvents(rEvent, 7);
#ifdef USES_QGLWIDGET
#else
    QOpenGLWidget::resizeEvent(rEvent);
#endif
}

void COpenglWidget::_resizeEvent(SMouseOrKeyboardOrResizeEvent e)
{
    TRACE_INTERNAL;

    int x, y;
    _computeMousePos(e.x, e.y, x, y);
    GuiApp::mainWindow->windowResizeEvent(x, y);
}

void COpenglWidget::initializeGL()
{
#ifdef USES_QGLWIDGET
    setAutoBufferSwap(false);
#else
    initializeOpenGLFunctions();
    initGl_openGLWidget();
    QOpenGLWidget::initializeGL();
#endif
}

void COpenglWidget::paintGL()
{
#ifdef USES_QGLWIDGET
#else
    GuiApp::mainWindow->renderScene();
//    QOpenGLWidget::paintGL();
#endif
}

void COpenglWidget::_setCtrlAndShiftKeyState(bool ctrlDown, bool shiftDown)
{
    int state = GuiApp::mainWindow->getKeyDownState() & (0xffff - 1 - 2);
    if (ctrlDown)
        state |= 1;
    if (shiftDown)
        state |= 2;
    GuiApp::mainWindow->setKeyDownState(state);
}

void COpenglWidget::mouseMoveEvent(QMouseEvent* mEvent)
{
    TRACE_INTERNAL;
    _handleMouseAndKeyboardAndResizeEvents(mEvent, 3);
}

void COpenglWidget::_mouseMoveEvent(SMouseOrKeyboardOrResizeEvent e)
{
    TRACE_INTERNAL;
    int x, y;
    _computeMousePos(e.x, e.y, x, y);
    _setCtrlAndShiftKeyState(e.ctrlDown, e.shiftDown);

    if ((disableMouseMoveFor200ms == -1) || (VDateTime::getTimeDiffInMs(disableMouseMoveFor200ms) > 200))
    { // when a mouse button was released, sometimes the mouse moves involontarily
        disableMouseMoveFor200ms = -1;
        GuiApp::mainWindow->onMouseMoveTT(x, y);
    }
}

void COpenglWidget::mousePressEvent(QMouseEvent* mEvent)
{
    TRACE_INTERNAL;
    _handleMouseAndKeyboardAndResizeEvents(mEvent, 0);
}

void COpenglWidget::_mousePressEvent(SMouseOrKeyboardOrResizeEvent e)
{
    TRACE_INTERNAL;

    int x, y;
    _computeMousePos(e.x, e.y, x, y);
    _setCtrlAndShiftKeyState(e.ctrlDown, e.shiftDown);

    GuiApp::mainWindow->setMouseButtonState(GuiApp::mainWindow->getMouseButtonState() & (0xffff - 16));
    if (e.button == 0)
    { // Left button down
        GuiApp::mainWindow->mouseClickAction(true);
        if (!e.ctrlDown)
            GuiApp::mainWindow->setMouseButtonState(GuiApp::mainWindow->getMouseButtonState() | 16);

        if ((GuiApp::mainWindow->getMouseButtonState() & 12) == 0)
        { // registered only if left right and middle buttons are not down
            GuiApp::mainWindow->onLeftMouseButtonDownTT(x, y);
            ignoreLeftMouseUp = false;
        }
        else
            ignoreLeftMouseUp = true;
    }
    if (!(GuiApp::isFullScreen()))
    {
        if (e.button == 2)
        { // right button down
            if ((GuiApp::mainWindow->getMouseButtonState() & 9) == 0)
            { // registered only if left and middle buttons are not down
                if (App::userSettings->navigationBackwardCompatibility)
                {
                    _savedMouseMode = GuiApp::getMouseMode();
                    int upperMouseMode = ((GuiApp::getMouseMode() & 0xff00) | sim_navigation_clickselection) -
                                         sim_navigation_clickselection; // sim_navigation_clickselection because
                                                                        // otherwise we have a problem (12/06/2011)
                    if (GuiApp::getMouseMode() & sim_navigation_camerarotaterightbutton)
                        GuiApp::setMouseMode(upperMouseMode | sim_navigation_camerarotate); // default
                    else
                        GuiApp::setMouseMode(upperMouseMode | sim_navigation_passive);
                }
                GuiApp::mainWindow->onRightMouseButtonDownTT(x, y);
                ignoreRightMouseUp = false;
            }
            else
                ignoreRightMouseUp = true;
        }
    }
    if (e.button == 1)
    { // middle button down
        if ((GuiApp::mainWindow->getMouseButtonState() & 5) == 0)
        { // registered only if left and right buttons are not down
            if (!App::userSettings->navigationBackwardCompatibility)
            { // Regular routine
                _savedMouseMode = GuiApp::getMouseMode();
                GuiApp::mainWindow->onMiddleMouseButtonDownTT(x, y);
            }
            else
            { // Routine that supports the old navigation mode
                if ((App::userSettings->middleMouseButtonSwitchesModes) && (!(GuiApp::isFullScreen())))
                {
                    bool noSelector = true;
                    if (GuiApp::mainWindow->oglSurface->isPageSelectionActive() ||
                        GuiApp::mainWindow->oglSurface->isViewSelectionActive())
                        noSelector = false;
                    if (noSelector)
                    {
                        if ((GuiApp::getMouseMode() & 0x00ff) == sim_navigation_camerashift)
                            GuiApp::setMouseMode((GuiApp::getMouseMode() & 0xff00) | sim_navigation_objectshift);
                        else
                        {
                            if ((GuiApp::getMouseMode() & 0x00ff) == sim_navigation_objectshift)
                                GuiApp::setMouseMode((GuiApp::getMouseMode() & 0xff00) | sim_navigation_objectrotate);
                            else
                                GuiApp::setMouseMode((GuiApp::getMouseMode() & 0xff00) | sim_navigation_camerashift);
                        }
                    }
                    GuiApp::mainWindow->setMouseButtonState(GuiApp::mainWindow->getMouseButtonState() | 8);
                }
            }
            ignoreMiddleMouseUp = false;
        }
        else
            ignoreMiddleMouseUp = true;
    }

    CDlgEx::doTransparencyCounter++;
}

void COpenglWidget::mouseReleaseEvent(QMouseEvent* mEvent)
{
    TRACE_INTERNAL;
    _handleMouseAndKeyboardAndResizeEvents(mEvent, 1);
}

void COpenglWidget::_mouseReleaseEvent(SMouseOrKeyboardOrResizeEvent e)
{
    TRACE_INTERNAL;

    int x, y;
    _computeMousePos(e.x, e.y, x, y);
    _setCtrlAndShiftKeyState(e.ctrlDown, e.shiftDown);

    if (e.button == 0)
    { // left button
        GuiApp::mainWindow->mouseClickAction(false);
        CEnvironment::setShapeTexturesTemporarilyDisabled(false);
        CEnvironment::setShapeEdgesTemporarilyDisabled(false);
        CEnvironment::setCustomUisTemporarilyDisabled(false);
        if (!ignoreLeftMouseUp)
        {
            disableMouseMoveFor200ms = (int)VDateTime::getTimeInMs(); // when the left mouse button was released,
                                                                      // sometimes the mouse moves involontarily
            GuiApp::mainWindow->onLeftMouseButtonUpTT(x, y);
        }
    }
    if (!(GuiApp::isFullScreen()))
    {
        if (!ignoreRightMouseUp)
        {
            if (e.button == 2)
            {                                                             // right button
                disableMouseMoveFor200ms = (int)VDateTime::getTimeInMs(); // when the right mouse button was released,
                                                                          // sometimes the mouse moves involontarily
                GuiApp::mainWindow->onRightMouseButtonUpTT(x, y);
                if (App::userSettings->navigationBackwardCompatibility && (!(GuiApp::mainWindow->isFullScreen())))
                    GuiApp::setMouseMode(_savedMouseMode);
            }
        }
    }
    if (e.button == 1)
    { // middle button
        CEnvironment::setShapeTexturesTemporarilyDisabled(false);
        CEnvironment::setShapeEdgesTemporarilyDisabled(false);
        CEnvironment::setCustomUisTemporarilyDisabled(false);
        if (!ignoreMiddleMouseUp)
        {
            if (!App::userSettings->navigationBackwardCompatibility)
            {
                disableWheelRotateForOne500ms =
                    (int)VDateTime::getTimeInMs();                        // when the middle mouse button was released, sometimes the wheel
                                                                          // rotates involontarily
                disableMouseMoveFor200ms = (int)VDateTime::getTimeInMs(); // when the middle mouse button was released,
                                                                          // sometimes the mouse moves involontarily
                GuiApp::mainWindow->onMiddleMouseButtonUpTT(x, y);
                GuiApp::setMouseMode(_savedMouseMode);
            }
            GuiApp::mainWindow->setMouseButtonState(GuiApp::mainWindow->getMouseButtonState() & (0xffff - 8));
        }
    }
}

void COpenglWidget::mouseDoubleClickEvent(QMouseEvent* mEvent)
{
    TRACE_INTERNAL;
    _handleMouseAndKeyboardAndResizeEvents(mEvent, 2);
}

void COpenglWidget::_mouseDoubleClickEvent(SMouseOrKeyboardOrResizeEvent e)
{
    TRACE_INTERNAL;

    int x, y;
    _computeMousePos(e.x, e.y, x, y);
    _setCtrlAndShiftKeyState(e.ctrlDown, e.shiftDown);

    if (!(GuiApp::isFullScreen()))
    {
        if (e.button == 0)
        { // left button
            GuiApp::mainWindow->onLeftMouseButtonDoubleClickTT(x, y);
        }
        if (e.button == 1)
        { // Middle button. similar thing further up (in mouse press)
            if (App::userSettings->navigationBackwardCompatibility)
            { // to support the old navigation method
                if (App::userSettings->middleMouseButtonSwitchesModes)
                {
                    bool noSelector = true;
                    if (GuiApp::mainWindow->oglSurface->isPageSelectionActive() ||
                        GuiApp::mainWindow->oglSurface->isViewSelectionActive())
                        noSelector = false;
                    if (noSelector)
                    {
                        if ((GuiApp::getMouseMode() & 0x00ff) == sim_navigation_camerashift)
                            GuiApp::setMouseMode((GuiApp::getMouseMode() & 0xff00) | sim_navigation_objectshift);
                        else
                        {
                            if ((GuiApp::getMouseMode() & 0x00ff) == sim_navigation_objectshift)
                                GuiApp::setMouseMode((GuiApp::getMouseMode() & 0xff00) | sim_navigation_objectrotate);
                            else
                                GuiApp::setMouseMode((GuiApp::getMouseMode() & 0xff00) | sim_navigation_camerashift);
                        }
                    }
                }
            }
        }
    }
}

void COpenglWidget::wheelEvent(QWheelEvent* wEvent)
{
    TRACE_INTERNAL;
    _handleMouseAndKeyboardAndResizeEvents(wEvent, 4);
}

void COpenglWidget::_wheelEvent(SMouseOrKeyboardOrResizeEvent e)
{
    TRACE_INTERNAL;
    int x, y;
    _computeMousePos(e.x, e.y, x, y);

    if ((GuiApp::mainWindow->getMouseButtonState() & 13) == 0)
    { // wheel only works when no other button is down
        if ((disableWheelRotateForOne500ms == -1) || (VDateTime::getTimeDiffInMs(disableWheelRotateForOne500ms) > 300))
        { // when the middle mouse button was released, sometimes the wheel rotates involontarily
            disableWheelRotateForOne500ms = -1;
            if ((GuiApp::mainWindow->getMouseButtonState() & 8) == 0)
            {
                _setCtrlAndShiftKeyState(e.ctrlDown, e.shiftDown);
                GuiApp::mainWindow->onWheelRotateTT(e.wheelDelta, x, y);
            }
        }
    }
}

bool COpenglWidget::focusNextPrevChild(bool next)
{ // Needed so that Tab-keys are also generating a keyPressEvent!
    return (false);
}

void COpenglWidget::keyPressEvent(QKeyEvent* kEvent)
{
    TRACE_INTERNAL;
    _handleMouseAndKeyboardAndResizeEvents(kEvent, 5);
}

void COpenglWidget::_keyPressEvent(SMouseOrKeyboardOrResizeEvent e)
{
    TRACE_INTERNAL;
    GuiApp::mainWindow->onKeyPress(e);
}

void COpenglWidget::keyReleaseEvent(QKeyEvent* kEvent)
{
    TRACE_INTERNAL;
    _handleMouseAndKeyboardAndResizeEvents(kEvent, 6);
}

void COpenglWidget::_keyReleaseEvent(SMouseOrKeyboardOrResizeEvent e)
{
    TRACE_INTERNAL;
    GuiApp::mainWindow->onKeyRelease(e);
}

void COpenglWidget::_timer100ms_fire()
{
    TRACE_INTERNAL;
    if (!CSimAndUiThreadSync::hasUiLockedResourcesForReadOrWrite())
        _handleMouseAndKeyboardAndResizeEvents(nullptr, 8);
}

void COpenglWidget::_handleMouseAndKeyboardAndResizeEvents(void* event, int t)
{
    TRACE_INTERNAL;
    if (event != nullptr)
    {
        SMouseOrKeyboardOrResizeEvent e;
        e.eventType = t;
        if (t <= 3)
        { // mouse events
            QMouseEvent* mEvent = (QMouseEvent*)event;
            e.x = mEvent->x();
            e.y = mEvent->y();
            e.ctrlDown = ((mEvent->modifiers() & Qt::ControlModifier) != 0);
            e.shiftDown = ((mEvent->modifiers() & Qt::ShiftModifier) != 0);
            if (mEvent->button() == Qt::LeftButton)
                e.button = 0;
            if (mEvent->button() == Qt::MiddleButton)
                e.button = 1;
            if (mEvent->button() == Qt::RightButton)
                e.button = 2;
        }
        if (t == 4)
        { // mouse wheel events
            QWheelEvent* wEvent = (QWheelEvent*)event;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            e.x = wEvent->x();
            e.y = wEvent->y();
#else
            e.x = wEvent->position().x();
            e.y = wEvent->position().y();
#endif
            e.ctrlDown = ((wEvent->modifiers() & Qt::ControlModifier) != 0);
            e.shiftDown = ((wEvent->modifiers() & Qt::ShiftModifier) != 0);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            e.wheelDelta = wEvent->delta();
#else
            e.wheelDelta = wEvent->angleDelta().y();
#endif
        }
        if ((t >= 5) && (t <= 6))
        { // keyboard events
            QKeyEvent* kEvent = (QKeyEvent*)event;
            if (t == 5)
            { // press
                bool processed = false;
                e.key = -1;
                e.specialKey = -1;
                if (kEvent->matches(QKeySequence::Cut))
                {
                    if (App::getOpenGlDisplayEnabled())
                        e.specialKey = CTRL_X_KEY;
                    processed = true;
                }
                if (kEvent->matches(QKeySequence::Copy))
                {
                    if (App::getOpenGlDisplayEnabled())
                        e.specialKey = CTRL_C_KEY;
                    processed = true;
                }
                if (kEvent->matches(QKeySequence::Paste))
                {
                    if (App::getOpenGlDisplayEnabled())
                        e.specialKey = CTRL_V_KEY;
                    processed = true;
                }
                if (kEvent->matches(QKeySequence::Undo))
                {
                    if (App::getOpenGlDisplayEnabled())
                        e.specialKey = CTRL_Z_KEY;
                    processed = true;
                }
                if (kEvent->matches(QKeySequence::Redo))
                {
                    if (App::getOpenGlDisplayEnabled())
                        e.specialKey = CTRL_Y_KEY;
                    processed = true;
                }
                if (kEvent->matches(QKeySequence::Save))
                {
                    if (App::getOpenGlDisplayEnabled())
                        e.specialKey = CTRL_S_KEY;
                    processed = true;
                }
                if (kEvent->matches(QKeySequence::SelectAll))
                {
                    if (CSimFlavor::getBoolVal(2))
                    {
                        if (App::getOpenGlDisplayEnabled())
                            e.specialKey = CTRL_A_KEY;
                        processed = true;
                    }
                }
                if (kEvent->matches(QKeySequence::Open))
                {
                    if (App::getOpenGlDisplayEnabled())
                        e.specialKey = CTRL_O_KEY;
                    processed = true;
                }
                if (kEvent->matches(QKeySequence::Close))
                {
                    if (App::getOpenGlDisplayEnabled())
                        e.specialKey = CTRL_W_KEY;
                    processed = true;
                }
                if (kEvent->matches(QKeySequence::New))
                {
                    if (App::getOpenGlDisplayEnabled())
                        e.specialKey = CTRL_N_KEY;
                    processed = true;
                }
                if (!processed)
                {
                    e.key = kEvent->key();
                    e.unicodeText = kEvent->text();
                }
            }
            else
            { // release
                e.key = kEvent->key();
            }
        }
        if (t == 7)
        { // OpenGl surface resize
            QResizeEvent* rEvent = (QResizeEvent*)event;
            e.x = rEvent->size().width();
            e.y = rEvent->size().height();
            setProperty("hierarchyWidth", App::getHierarchyEnabled() * GuiApp::mainWindow->oglSurface->getHierarchyWidth() / GuiApp::sc);
        }
        _bufferedMouseOrKeyboardOrResizeEvents.push_back(e);
    }
    else
    { // timer event
        SMouseOrKeyboardOrResizeEvent e;
        e.eventType = t;
        _bufferedMouseOrKeyboardOrResizeEvents.push_back(e);
    }

    if (_bufferedMouseOrKeyboardOrResizeEvents.size() != 0)
    {
        IF_UI_EVENT_CAN_READ_DATA_NO_WAIT
        {
            while (_bufferedMouseOrKeyboardOrResizeEvents.size() != 0)
            {
                SMouseOrKeyboardOrResizeEvent e = _bufferedMouseOrKeyboardOrResizeEvents[0];
                _bufferedMouseOrKeyboardOrResizeEvents.erase(_bufferedMouseOrKeyboardOrResizeEvents.begin());
                if (e.eventType == 0)
                    _mousePressEvent(e);
                if (e.eventType == 1)
                    _mouseReleaseEvent(e);
                if (e.eventType == 2)
                    _mouseDoubleClickEvent(e);
                if (e.eventType == 3)
                    _mouseMoveEvent(e);
                if (e.eventType == 4)
                    _wheelEvent(e);
                if (e.eventType == 5)
                    _keyPressEvent(e);
                if (e.eventType == 6)
                    _keyReleaseEvent(e);
                if (e.eventType == 7)
                    _resizeEvent(e);
                if (e.eventType == 8)
                {
                    if ((_lastGlobalMousePos[0] != QCursor::pos().x()) ||
                        (_lastGlobalMousePos[1] != QCursor::pos().y()))
                    {
                        _lastGlobalMousePos[0] = QCursor::pos().x();
                        _lastGlobalMousePos[1] = QCursor::pos().y();
                        if ((GuiApp::mainWindow->getMouseButtonState() & 1) == 0)
                            GuiApp::mainWindow->setCurrentCursor(sim_cursor_arrow);
                    }
                }
            }
        }
    }
}

void COpenglWidget::_computeMousePos(int inX, int inY, int& outX, int& outY)
{
    outX = GuiApp::mainWindow->devicePixelRatio() * inX;
    outY = GuiApp::mainWindow->devicePixelRatio() * inY;
    /*
    double sx=windowHandle()->devicePixelRatio();
    double sy=windowHandle()->devicePixelRatio();
    outX=int(double(inX)*sx+0.5);
    outY=int(double(inY)*sy+0.5);
    */
    _lastGlobalMousePos[0] = QCursor::pos().x();
    _lastGlobalMousePos[1] = QCursor::pos().y();
}

void COpenglWidget::dragEnterEvent(QDragEnterEvent* dEvent)
{
    if (dEvent->mimeData()->hasText())
    {
        const SModelThumbnailInfo* thumbnail = GuiApp::mainWindow->modelListWidget->getThumbnailInfoFromModelName(
            dEvent->mimeData()->text().toStdString().c_str(), nullptr);
        if (thumbnail != nullptr)
            dEvent->accept();
    }
    else
#ifdef USES_QGLWIDGET
        QGLWidget::dragEnterEvent(dEvent);
#else
        QOpenGLWidget::dragEnterEvent(dEvent);
#endif
}

void COpenglWidget::dragLeaveEvent(QDragLeaveEvent* dEvent)
{
    _modelDragAndDropInfo = nullptr;
#ifdef USES_QGLWIDGET
    QGLWidget::dragLeaveEvent(dEvent);
#else
    QOpenGLWidget::dragLeaveEvent(dEvent);
#endif
}

void COpenglWidget::dropEvent(QDropEvent* dEvent)
{
    GuiApp::mainWindow->oglSurface->hierarchy->endModelDrag();
    if (dEvent->mimeData()->hasText())
    {
        int x, y;
        _computeMousePos(dEvent->pos().x(), dEvent->pos().y(), x, y);
        _modelDragAndDropInfo = GuiApp::mainWindow->modelListWidget->getThumbnailInfoFromModelName(dEvent->mimeData()->text().toStdString().c_str(), nullptr);
        if (_modelDragAndDropInfo != nullptr)
        {
            C3Vector desiredModelPosition;
            int okToDrop = GuiApp::mainWindow->modelDragMoveEvent(x, y, &desiredModelPosition);
            GuiApp::mainWindow->oglSurface->hierarchy->endModelDrag();
            if (okToDrop >= -1)
            {
                std::string pathAndName = _modelDragAndDropInfo->filepath;
                if (pathAndName.length() != 0)
                {
                    SSimulationThreadCommand cmd;
                    cmd.cmdId = MODEL_BROWSER_DRAG_AND_DROP_CMD;
                    cmd.stringParams.push_back(pathAndName);
                    cmd.doubleParams.push_back(desiredModelPosition(0));
                    cmd.doubleParams.push_back(desiredModelPosition(1));
                    cmd.doubleParams.push_back(desiredModelPosition(2));
                    cmd.intParams.push_back(okToDrop);
                    App::appendSimulationThreadCommand(cmd); // that command will clear _modelDragAndDropInfo once the model was loaded
                }
                else
                    _modelDragAndDropInfo = nullptr;
            }
            else
                _modelDragAndDropInfo = nullptr;
        }
        else
            _modelDragAndDropInfo = nullptr;
    }
    else
    {
        _modelDragAndDropInfo = nullptr;
#ifdef USES_QGLWIDGET
        QGLWidget::dropEvent(dEvent);
#else
        QOpenGLWidget::dropEvent(dEvent);
#endif
    }
}

void COpenglWidget::dragMoveEvent(QDragMoveEvent* dEvent)
{
    if (dEvent->mimeData()->hasText())
    {
        int x, y;
        _computeMousePos(dEvent->pos().x(), dEvent->pos().y(), x, y);
        SModelThumbnailInfo* info = GuiApp::mainWindow->modelListWidget->getThumbnailInfoFromModelName(dEvent->mimeData()->text().toStdString().c_str(), nullptr);
        if (info != nullptr)
        {
            C3Vector desiredModelPosition;
            int okToDrop = GuiApp::mainWindow->modelDragMoveEvent(x, y, &desiredModelPosition);
            if (okToDrop < 0)
                GuiApp::mainWindow->oglSurface->hierarchy->endModelDrag();
            if (okToDrop >= -1)
            {
                dEvent->accept();
                if (okToDrop < 0)
                {
                    _modelDragAndDropInfo = info;
                    _modelDragAndDropInfo->desiredDropPos = desiredModelPosition;
                }
                else
                    _modelDragAndDropInfo = nullptr;
            }
            else
            {
                dEvent->ignore();
                _modelDragAndDropInfo = nullptr;
            }
        }
        else
        {
            dEvent->ignore();
            _modelDragAndDropInfo = nullptr;
        }
    }
    else
    {
        _modelDragAndDropInfo = nullptr;
#ifdef USES_QGLWIDGET
        QGLWidget::dragMoveEvent(dEvent);
#else
        QOpenGLWidget::dragMoveEvent(dEvent);
#endif
    }
}

SModelThumbnailInfo* COpenglWidget::getModelDragAndDropInfo()
{
    return (_modelDragAndDropInfo);
}

void COpenglWidget::clearModelDragAndDropInfo()
{
    _modelDragAndDropInfo = nullptr;
}
