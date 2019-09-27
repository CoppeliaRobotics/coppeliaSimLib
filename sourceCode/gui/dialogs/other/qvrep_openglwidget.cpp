#include "qvrep_openglwidget.h"
#include <QMimeData>
#include <QMouseEvent>
#include <QWindow>
#include <QTimer>
#include "vDateTime.h"
#include "ttUtil.h"
#include "app.h"
#include "toolBarCommand.h"
#include "funcDebug.h"
#include "rendering.h"
#include "miscBase.h"

int disableWheelRotateForOne500ms=-1;
int disableMouseMoveFor200ms=-1;
bool ignoreLeftMouseUp=false;
bool ignoreMiddleMouseUp=false;
bool ignoreRightMouseUp=false;
int _savedMouseMode;

std::vector<SMouseOrKeyboardOrResizeEvent> _bufferedMouseOrKeyboardOrResizeEvents;

#ifdef USING_QOPENGLWIDGET
COpenglWidget::COpenglWidget(QWidget *parent) : QOpenGLWidget(parent)
#else
COpenglWidget::COpenglWidget(QWidget *parent) : QGLWidget(QGLFormat((App::userSettings->stereoDist<=0.0f) ? (QGL::DoubleBuffer) : (QGL::StereoBuffers)),parent)
#endif
{
    FUNCTION_DEBUG;
#ifdef USING_QOPENGLWIDGET
    if (App::userSettings->stereoDist>0.0f)
    {
        QSurfaceFormat s(QSurfaceFormat::StereoBuffers);
        s.setRenderableType(QSurfaceFormat::OpenGL);
        setFormat(s);
    }
    else
    {
        QSurfaceFormat s;
        s.setRenderableType(QSurfaceFormat::OpenGL);
        setFormat(s);
    }
#endif
    _bufferedMouseOrKeyboardOrResizeEvents.clear();
    disableWheelRotateForOne500ms=-1;
    disableMouseMoveFor200ms=-1;
    ignoreLeftMouseUp=false;
    ignoreMiddleMouseUp=false;
    ignoreRightMouseUp=false;

    setMouseTracking(true);
    setFocusPolicy(Qt::WheelFocus);

    QTimer* timer_100ms=new QTimer(this);
    connect(timer_100ms, SIGNAL(timeout()), this, SLOT(_timer100ms_fire()));
    timer_100ms->start(100);
    _modelDragAndDropInfo=nullptr;
    setAcceptDrops(true);
}

COpenglWidget::~COpenglWidget()
{
}

void COpenglWidget::initializeGL()
{
    FUNCTION_DEBUG;
#ifndef USING_QOPENGLWIDGET
    setAutoBufferSwap(false);
#else
    makeContextCurrent();
    initGl_ifNeeded();
#endif
}

void COpenglWidget::makeContextCurrent()
{
    makeCurrent();
}

void COpenglWidget::paintEvent(QPaintEvent* event)
{
    FUNCTION_DEBUG;
}

void COpenglWidget::resizeEvent(QResizeEvent* rEvent)
{
    FUNCTION_DEBUG;
#ifdef USING_QOPENGLWIDGET
    QOpenGLWidget::resizeEvent(rEvent);
#endif
    _handleMouseAndKeyboardAndResizeEvents(rEvent,7);
}

void COpenglWidget::_resizeEvent(SMouseOrKeyboardOrResizeEvent e)
{
    FUNCTION_DEBUG;

    int x,y;
    _computeMousePos(e.x,e.y,x,y);

    App::mainWindow->windowResizeEvent(x,y);
}

void COpenglWidget::paintGL()
{
    FUNCTION_DEBUG;
}

void COpenglWidget::_setCtrlAndShiftKeyState(bool ctrlDown,bool shiftDown)
{
    int state=App::mainWindow->getKeyDownState()&(0xffff-1-2);
    if (ctrlDown)
        state|=1;
    if (shiftDown)
        state|=2;
    App::mainWindow->setKeyDownState(state);
}

void COpenglWidget::mouseMoveEvent(QMouseEvent* mEvent)
{
    FUNCTION_DEBUG;
    _handleMouseAndKeyboardAndResizeEvents(mEvent,3);
}

void COpenglWidget::_mouseMoveEvent(SMouseOrKeyboardOrResizeEvent e)
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    FUNCTION_DEBUG;
    int x,y;
    _computeMousePos(e.x,e.y,x,y);
    _setCtrlAndShiftKeyState(e.ctrlDown,e.shiftDown);

    if ( (disableMouseMoveFor200ms==-1)||(VDateTime::getTimeDiffInMs(disableMouseMoveFor200ms)>200) )
    { // when a mouse button was released, sometimes the mouse moves involontarily
        disableMouseMoveFor200ms=-1;
        App::mainWindow->onMouseMoveTT(x,y);
    }
}

void COpenglWidget::mousePressEvent(QMouseEvent* mEvent)
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    FUNCTION_DEBUG;
    _handleMouseAndKeyboardAndResizeEvents(mEvent,0);
}

void COpenglWidget::_mousePressEvent(SMouseOrKeyboardOrResizeEvent e)
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    FUNCTION_DEBUG;

    int x,y;
    _computeMousePos(e.x,e.y,x,y);
    _setCtrlAndShiftKeyState(e.ctrlDown,e.shiftDown);

    App::mainWindow->setMouseButtonState(App::mainWindow->getMouseButtonState()&(0xffff-16));
    if (e.button==0)
    { // Left button down
        if (!e.ctrlDown)
            App::mainWindow->setMouseButtonState(App::mainWindow->getMouseButtonState()|16);

        if ((App::mainWindow->getMouseButtonState()&12)==0)
        { // registered only if left right and middle buttons are not down
            App::mainWindow->onLeftMouseButtonDownTT(x,y);
            ignoreLeftMouseUp=false;
        }
        else
            ignoreLeftMouseUp=true;
    }
    if (!(App::isFullScreen()))
    {
        if (e.button==2)
        { // right button down
            if ((App::mainWindow->getMouseButtonState()&9)==0)
            { // registered only if left and middle buttons are not down
                if (App::userSettings->navigationBackwardCompatibility)
                {
                    _savedMouseMode=App::getMouseMode();
                    int upperMouseMode=((App::getMouseMode()&0xff00)|sim_navigation_clickselection)-sim_navigation_clickselection; // sim_navigation_clickselection because otherwise we have a problem (12/06/2011)
                    if (App::getMouseMode()&sim_navigation_camerarotaterightbutton)
                        App::setMouseMode(upperMouseMode|sim_navigation_camerarotate); // default
                    else
                        App::setMouseMode(upperMouseMode|sim_navigation_passive);
                }
                App::mainWindow->onRightMouseButtonDownTT(x,y);
                ignoreRightMouseUp=false;
            }
            else
                ignoreRightMouseUp=true;
        }
    }
    if (e.button==1)
    { // middle button down
        if ((App::mainWindow->getMouseButtonState()&5)==0)
        { // registered only if left and right buttons are not down
            if (!App::userSettings->navigationBackwardCompatibility)
            { // Regular routine
                _savedMouseMode=App::getMouseMode();
                App::mainWindow->onMiddleMouseButtonDownTT(x,y);
            }
            else
            { // Routine that supports the old navigation mode
                if ( (App::userSettings->middleMouseButtonSwitchesModes)&&(!(App::isFullScreen())) )
                {
                    bool noSelector=true;
                    if (App::mainWindow->oglSurface->isSceneSelectionActive()||App::mainWindow->oglSurface->isPageSelectionActive()||App::mainWindow->oglSurface->isViewSelectionActive())
                        noSelector=false;
                    if (noSelector)
                    {
                        if ((App::getMouseMode()&0x00ff)==sim_navigation_camerashift)
                            App::setMouseMode((App::getMouseMode()&0xff00)|sim_navigation_objectshift);
                        else
                        {
                            if ((App::getMouseMode()&0x00ff)==sim_navigation_objectshift)
                                App::setMouseMode((App::getMouseMode()&0xff00)|sim_navigation_objectrotate);
                            else
                                App::setMouseMode((App::getMouseMode()&0xff00)|sim_navigation_camerashift);
                        }
                    }
                    App::mainWindow->setMouseButtonState(App::mainWindow->getMouseButtonState()|8);
                }
            }
            ignoreMiddleMouseUp=false;
        }
        else
            ignoreMiddleMouseUp=true;
    }

    CDlgEx::doTransparencyCounter++;
}

void COpenglWidget::mouseReleaseEvent(QMouseEvent* mEvent)
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    FUNCTION_DEBUG;
    _handleMouseAndKeyboardAndResizeEvents(mEvent,1);
}

void COpenglWidget::_mouseReleaseEvent(SMouseOrKeyboardOrResizeEvent e)
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    FUNCTION_DEBUG;

    int x,y;
    _computeMousePos(e.x,e.y,x,y);
    _setCtrlAndShiftKeyState(e.ctrlDown,e.shiftDown);

    if (e.button==0)
    { // left button
        CEnvironment::setShapeTexturesTemporarilyDisabled(false);
        CEnvironment::setShapeEdgesTemporarilyDisabled(false);
        CEnvironment::setCustomUisTemporarilyDisabled(false);
        if (!ignoreLeftMouseUp)
        {
            disableMouseMoveFor200ms=VDateTime::getTimeInMs(); // when the left mouse button was released, sometimes the mouse moves involontarily
            App::mainWindow->onLeftMouseButtonUpTT(x,y);
        }
    }
    if (!(App::isFullScreen()))
    {
        if (!ignoreRightMouseUp)
        {
            if (e.button==2)
            { // right button
                disableMouseMoveFor200ms=VDateTime::getTimeInMs(); // when the right mouse button was released, sometimes the mouse moves involontarily
                App::mainWindow->onRightMouseButtonUpTT(x,y);
                if ( App::userSettings->navigationBackwardCompatibility&&(!(App::mainWindow->isFullScreen())) )
                    App::setMouseMode(_savedMouseMode);
            }
        }
    }
    if (e.button==1)
    { // middle button
        CEnvironment::setShapeTexturesTemporarilyDisabled(false);
        CEnvironment::setShapeEdgesTemporarilyDisabled(false);
        CEnvironment::setCustomUisTemporarilyDisabled(false);
        if (!ignoreMiddleMouseUp)
        {
            if (!App::userSettings->navigationBackwardCompatibility)
            {
                disableWheelRotateForOne500ms=VDateTime::getTimeInMs(); // when the middle mouse button was released, sometimes the wheel rotates involontarily
                disableMouseMoveFor200ms=VDateTime::getTimeInMs(); // when the middle mouse button was released, sometimes the mouse moves involontarily
                App::mainWindow->onMiddleMouseButtonUpTT(x,y);
                App::setMouseMode(_savedMouseMode);
            }
            App::mainWindow->setMouseButtonState(App::mainWindow->getMouseButtonState()&(0xffff-8));
        }
    }
}

void COpenglWidget::mouseDoubleClickEvent(QMouseEvent* mEvent)
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    FUNCTION_DEBUG;
    _handleMouseAndKeyboardAndResizeEvents(mEvent,2);
}

void COpenglWidget::_mouseDoubleClickEvent(SMouseOrKeyboardOrResizeEvent e)
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    FUNCTION_DEBUG;

    int x,y;
    _computeMousePos(e.x,e.y,x,y);
    _setCtrlAndShiftKeyState(e.ctrlDown,e.shiftDown);

    if (!(App::isFullScreen()))
    {
        if (e.button==0)
        { // left button
            App::mainWindow->onLeftMouseButtonDoubleClickTT(x,y);
        }
        if (e.button==1)
        { // Middle button. similar thing further up (in mouse press)
            if (App::userSettings->navigationBackwardCompatibility)
            { // to support the old navigation method
                if (App::userSettings->middleMouseButtonSwitchesModes)
                {
                    bool noSelector=true;
                    if (App::mainWindow->oglSurface->isSceneSelectionActive()||App::mainWindow->oglSurface->isPageSelectionActive()||App::mainWindow->oglSurface->isViewSelectionActive())
                        noSelector=false;
                    if (noSelector)
                    {
                        if ((App::getMouseMode()&0x00ff)==sim_navigation_camerashift)
                            App::setMouseMode((App::getMouseMode()&0xff00)|sim_navigation_objectshift);
                        else
                        {
                            if ((App::getMouseMode()&0x00ff)==sim_navigation_objectshift)
                                App::setMouseMode((App::getMouseMode()&0xff00)|sim_navigation_objectrotate);
                            else
                                App::setMouseMode((App::getMouseMode()&0xff00)|sim_navigation_camerashift);
                        }
                    }
                }
            }
        }
    }
}

void COpenglWidget::wheelEvent(QWheelEvent* wEvent)
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    FUNCTION_DEBUG;
    _handleMouseAndKeyboardAndResizeEvents(wEvent,4);
}

void COpenglWidget::_wheelEvent(SMouseOrKeyboardOrResizeEvent e)
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    FUNCTION_DEBUG;

    int x,y;
    _computeMousePos(e.x,e.y,x,y);

    if ((App::mainWindow->getMouseButtonState()&13)==0)
    { // wheel only workes when no other button is down
        if ( (disableWheelRotateForOne500ms==-1)||(VDateTime::getTimeDiffInMs(disableWheelRotateForOne500ms)>300) )
        { // when the middle mouse button was released, sometimes the wheel rotates involontarily
            disableWheelRotateForOne500ms=-1;
            if ((App::mainWindow->getMouseButtonState()&8)==0)
            {
                _setCtrlAndShiftKeyState(e.ctrlDown,e.shiftDown);
                App::mainWindow->onWheelRotateTT(e.wheelDelta,x,y);
            }
        }
    }
}

bool COpenglWidget::focusNextPrevChild(bool next)
{ // Needed so that Tab-keys are also generating a keyPressEvent!
    return(false);
}


void COpenglWidget::keyPressEvent(QKeyEvent* kEvent)
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    FUNCTION_DEBUG;
    _handleMouseAndKeyboardAndResizeEvents(kEvent,5);
}

void COpenglWidget::_keyPressEvent(SMouseOrKeyboardOrResizeEvent e)
{
    FUNCTION_DEBUG;
    App::mainWindow->onKeyPress(e);
}

void COpenglWidget::keyReleaseEvent(QKeyEvent* kEvent)
{
    FUNCTION_DEBUG;
    _handleMouseAndKeyboardAndResizeEvents(kEvent,6);
}

void COpenglWidget::_keyReleaseEvent(SMouseOrKeyboardOrResizeEvent e)
{
    FUNCTION_DEBUG;
    App::mainWindow->onKeyRelease(e);
}

void COpenglWidget::_timer100ms_fire()
{
    FUNCTION_DEBUG;
    if (!CSimAndUiThreadSync::hasUiLockedResourcesForReadOrWrite())
        _handleMouseAndKeyboardAndResizeEvents(nullptr,8);
}

void COpenglWidget::_handleMouseAndKeyboardAndResizeEvents(void* event,int t)
{
    FUNCTION_DEBUG;
    if (event!=nullptr)
    {
        SMouseOrKeyboardOrResizeEvent e;
        e.eventType=t;
        if (t<=3)
        { // mouse events
            QMouseEvent* mEvent=(QMouseEvent*)event;
            e.x=mEvent->x();
            e.y=mEvent->y();
            e.ctrlDown=((mEvent->modifiers()&Qt::ControlModifier)!=0);
            e.shiftDown=((mEvent->modifiers()&Qt::ShiftModifier)!=0);
            if (mEvent->button()==Qt::LeftButton)
                e.button=0;
            if (mEvent->button()==Qt::MiddleButton)
                e.button=1;
            if (mEvent->button()==Qt::RightButton)
                e.button=2;
        }
        if (t==4)
        { // mouse wheel events
            QWheelEvent* wEvent=(QWheelEvent*)event;
            e.x=wEvent->x();
            e.y=wEvent->y();
            e.ctrlDown=((wEvent->modifiers()&Qt::ControlModifier)!=0);
            e.shiftDown=((wEvent->modifiers()&Qt::ShiftModifier)!=0);
            e.wheelDelta=wEvent->delta();
        }
        if ((t>=5)&&(t<=6))
        { // keyboard events
            QKeyEvent* kEvent=(QKeyEvent*)event;
            if (t==5)
            { // press
                bool processed=false;
                e.key=-1;
                e.specialKey=-1;
                if (kEvent->matches(QKeySequence::Cut))
                {
                    if (App::mainWindow->getOpenGlDisplayEnabled())
                        e.specialKey=CTRL_X_KEY;
                    processed=true;
                }
                if (kEvent->matches(QKeySequence::Copy))
                {
                    if (App::mainWindow->getOpenGlDisplayEnabled())
                        e.specialKey=CTRL_C_KEY;
                    processed=true;
                }
                if (kEvent->matches(QKeySequence::Paste))
                {
                    if (App::mainWindow->getOpenGlDisplayEnabled())
                        e.specialKey=CTRL_V_KEY;
                    processed=true;
                }
                if (kEvent->matches(QKeySequence::Undo))
                {
                    if (App::mainWindow->getOpenGlDisplayEnabled())
                        e.specialKey=CTRL_Z_KEY;
                    processed=true;
                }
                if (kEvent->matches(QKeySequence::Redo))
                {
                    if (App::mainWindow->getOpenGlDisplayEnabled())
                        e.specialKey=CTRL_Y_KEY;
                    processed=true;
                }
                if (kEvent->matches(QKeySequence::Save))
                {
                    if (App::mainWindow->getOpenGlDisplayEnabled())
                        e.specialKey=CTRL_S_KEY;
                    processed=true;
                }
                if (kEvent->matches(QKeySequence::SelectAll))
                {
                    if (CMiscBase::handleVerSpec_canSelectAllObjects())
                    {
                        if (App::mainWindow->getOpenGlDisplayEnabled())
                            e.specialKey=CTRL_A_KEY;
                        processed=true;
                    }
                }
                if (kEvent->matches(QKeySequence::Open))
                {
                    if (App::mainWindow->getOpenGlDisplayEnabled())
                        e.specialKey=CTRL_O_KEY;
                    processed=true;
                }
                if (kEvent->matches(QKeySequence::Close))
                {
                    if (App::mainWindow->getOpenGlDisplayEnabled())
                        e.specialKey=CTRL_W_KEY;
                    processed=true;
                }
                if (kEvent->matches(QKeySequence::New))
                {
                    if (App::mainWindow->getOpenGlDisplayEnabled())
                        e.specialKey=CTRL_N_KEY;
                    processed=true;
                }
                if (!processed)
                {
                    e.key=kEvent->key();
                    e.unicodeText=kEvent->text();
                }
            }
            else
            { // release
                e.key=kEvent->key();
            }
        }
        if (t==7)
        { // OpenGl surface resize
            QResizeEvent* rEvent=(QResizeEvent*)event;
            e.x=rEvent->size().width();
            e.y=rEvent->size().height();
        }
        _bufferedMouseOrKeyboardOrResizeEvents.push_back(e);
    }
    else
    { // timer event
        SMouseOrKeyboardOrResizeEvent e;
        e.eventType=t;
        _bufferedMouseOrKeyboardOrResizeEvents.push_back(e);
    }

    if (_bufferedMouseOrKeyboardOrResizeEvents.size()!=0)
    {
        IF_UI_EVENT_CAN_READ_DATA_NO_WAIT
        {
            while (_bufferedMouseOrKeyboardOrResizeEvents.size()!=0)
            {
                SMouseOrKeyboardOrResizeEvent e=_bufferedMouseOrKeyboardOrResizeEvents[0];
                _bufferedMouseOrKeyboardOrResizeEvents.erase(_bufferedMouseOrKeyboardOrResizeEvents.begin());
                if (e.eventType==0)
                    _mousePressEvent(e);
                if (e.eventType==1)
                    _mouseReleaseEvent(e);
                if (e.eventType==2)
                    _mouseDoubleClickEvent(e);
                if (e.eventType==3)
                    _mouseMoveEvent(e);
                if (e.eventType==4)
                    _wheelEvent(e);
                if (e.eventType==5)
                    _keyPressEvent(e);
                if (e.eventType==6)
                    _keyReleaseEvent(e);
                if (e.eventType==7)
                    _resizeEvent(e);
                if (e.eventType==8)
                {
                    if ( (_lastGlobalMousePos[0]!=QCursor::pos().x())||(_lastGlobalMousePos[1]!=QCursor::pos().y()) )
                    {
                        _lastGlobalMousePos[0]=QCursor::pos().x();
                        _lastGlobalMousePos[1]=QCursor::pos().y();
                        if ((App::mainWindow->getMouseButtonState()&1)==0)
                            App::mainWindow->setCurrentCursor(sim_cursor_arrow);
                    }
                }
            }
        }
    }
}

void COpenglWidget::_computeMousePos(int inX,int inY,int& outX,int& outY)
{
#ifdef USING_QOPENGLWIDGET
    float sx=App::qtApp->devicePixelRatio();
    float sy=sx;
#else
    float sx=windowHandle()->devicePixelRatio();
    float sy=windowHandle()->devicePixelRatio();
#endif
    outX=int(float(inX)*sx+0.5f);
    outY=int(float(inY)*sy+0.5f);
    _lastGlobalMousePos[0]=QCursor::pos().x();
    _lastGlobalMousePos[1]=QCursor::pos().y();
}

void COpenglWidget::dragEnterEvent(QDragEnterEvent* dEvent)
{
    if (dEvent->mimeData()->hasText())
    {
        const SModelThumbnailInfo* thumbnail=App::mainWindow->modelListWidget->getThumbnailInfoFromModelName(dEvent->mimeData()->text().toStdString().c_str(),nullptr);
        if (thumbnail!=nullptr)
            dEvent->accept();
    }
}

void COpenglWidget::dragLeaveEvent(QDragLeaveEvent* dEvent)
{
    _modelDragAndDropInfo=nullptr;
}

void COpenglWidget::dropEvent(QDropEvent* dEvent)
{
    if (dEvent->mimeData()->hasText())
    {
        int x,y;
        _computeMousePos(dEvent->pos().x(),dEvent->pos().y(),x,y);
        _modelDragAndDropInfo=App::mainWindow->modelListWidget->getThumbnailInfoFromModelName(dEvent->mimeData()->text().toStdString().c_str(),nullptr);
        if (_modelDragAndDropInfo!=nullptr)
        {
            C3Vector desiredModelPosition;
            int okToDrop=App::mainWindow->modelDragMoveEvent(x,y,&desiredModelPosition);
            if (okToDrop>0)
            {
                std::string pathAndName=_modelDragAndDropInfo->modelPathAndNameWithExtension;
                if (pathAndName.length()!=0)
                {
                    SSimulationThreadCommand cmd;
                    cmd.cmdId=MODEL_BROWSER_DRAG_AND_DROP_CMD;
                    cmd.stringParams.push_back(pathAndName);
                    cmd.floatParams.push_back(desiredModelPosition(0));
                    cmd.floatParams.push_back(desiredModelPosition(1));
                    cmd.floatParams.push_back(desiredModelPosition(2));
                    App::appendSimulationThreadCommand(cmd); // that command will clear _modelDragAndDropInfo once the model was loaded
                }
                else
                    _modelDragAndDropInfo=nullptr;
            }
            else
                _modelDragAndDropInfo=nullptr;
        }
        else
            _modelDragAndDropInfo=nullptr;
    }
    else
        _modelDragAndDropInfo=nullptr;
}

void COpenglWidget::dragMoveEvent(QDragMoveEvent* dEvent)
{
    if (dEvent->mimeData()->hasText())
    {
        int x,y;
        _computeMousePos(dEvent->pos().x(),dEvent->pos().y(),x,y);
        SModelThumbnailInfo* info=App::mainWindow->modelListWidget->getThumbnailInfoFromModelName(dEvent->mimeData()->text().toStdString().c_str(),nullptr);
        if (info!=nullptr)
        {
            C3Vector desiredModelPosition;
            int okToDrop=App::mainWindow->modelDragMoveEvent(x,y,&desiredModelPosition);
            if (okToDrop>0)
            {
                dEvent->accept();
                _modelDragAndDropInfo=info;
                _modelDragAndDropInfo->desiredDropPos=desiredModelPosition;
            }
            else
            {
                dEvent->ignore();
                _modelDragAndDropInfo=nullptr;
            }
        }
        else
        {
            dEvent->ignore();
            _modelDragAndDropInfo=nullptr;
        }
    }
    else
        _modelDragAndDropInfo=nullptr;
}

SModelThumbnailInfo* COpenglWidget::getModelDragAndDropInfo()
{
    return(_modelDragAndDropInfo);
}

void COpenglWidget::clearModelDragAndDropInfo()
{
    _modelDragAndDropInfo=nullptr;
}
