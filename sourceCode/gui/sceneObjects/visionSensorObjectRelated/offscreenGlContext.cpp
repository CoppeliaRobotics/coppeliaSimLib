#include <offscreenGlContext.h>
#include <app.h>
#include <rendering.h>
#include <oglExt.h>
#ifdef SIM_WITH_GUI
    #include <guiApp.h>
#endif

std::vector<QOpenGLContext*> COffscreenGlContext::_allQtContexts;
#ifdef USES_QGLWIDGET
    std::vector<QGLWidget*> COffscreenGlContext::_allQtWidgets;
#else
    std::vector<QOpenGLWidget*> COffscreenGlContext::_allQtWidgets;
#endif

#ifdef WIN_SIM
std::vector<HGLRC> COffscreenGlContext::_allNativeContexts;
/*
LRESULT CALLBACK WndProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
    return DefWindowProc(hWnd, Message, wParam, lParam);
}
*/
#endif

#ifdef USES_QGLWIDGET
COffscreenGlContext::COffscreenGlContext(int offscreenType,int resX,int resY,QGLWidget *otherWidgetToShareResourcesWith,int majorOpenGl,int minorOpenGl) : QObject()
#else
COffscreenGlContext::COffscreenGlContext(int offscreenType,int resX,int resY,QOpenGLWidget *otherWidgetToShareResourcesWith,int majorOpenGl,int minorOpenGl) : QObject()
#endif
{
    // 1. QT_OFFSCREEN_TP can be started from any thread, also when running in headless mode. However:
    // QOpenGLContext somehow requires that the thread that created it also processes its events on
    // a regular basis, otherwise the UI thread will hang out of the blue somehow.
    //
    // 2. QT_WINDOW_SHOW_TP and QT_WINDOW_HIDE_TP can only be started from the UI thread
    // (also when running in headless mode), otherwise we have crashes. So in that situation
    // if we notice we are not running the UI thread, we ask the UI thread to do the
    // QGLWidget/QOpenGLWidget initialization and rendering.

#ifndef SIM_WITH_GUI
    offscreenType=QT_OFFSCREEN_TP;
#endif

    TRACE_INTERNAL;
    _offscreenType=offscreenType;
    _isHeadless=(otherWidgetToShareResourcesWith==nullptr);
    _initialThread=QThread::currentThread();
    if (_offscreenType==QT_OFFSCREEN_TP)
    {
        _qOffscreenSurface=new QOffscreenSurface();
        #ifdef USES_QGLWIDGET
            QSurfaceFormat f;
            f.setSwapBehavior(QSurfaceFormat::SingleBuffer);
            f.setRenderableType(QSurfaceFormat::OpenGL);
            f.setRedBufferSize(8);
            f.setGreenBufferSize(8);
            f.setBlueBufferSize(8);
            f.setAlphaBufferSize(0);
            f.setStencilBufferSize(8);
            f.setDepthBufferSize(24);
            if (majorOpenGl!=-1)
                f.setMajorVersion(majorOpenGl);
            if (minorOpenGl!=-1)
                f.setMinorVersion(minorOpenGl);
            _qOffscreenSurface->setFormat(f);
        #endif
        _qOffscreenSurface->create();
        if (_qOffscreenSurface->isValid())
        {
            _qContext=new QOpenGLContext();
            #ifdef USES_QGLWIDGET
                _qContext->setFormat(_qOffscreenSurface->format());
            #endif
            if (otherWidgetToShareResourcesWith!=nullptr)
                #ifdef USES_QGLWIDGET
                    _qContext->setShareContext(otherWidgetToShareResourcesWith->context()->contextHandle());
                #else
                    _qContext->setShareContext(otherWidgetToShareResourcesWith->context());
                #endif
            else
            {
                if (_allQtContexts.size()!=0)
                    _qContext->setShareContext(_allQtContexts[0]);
            }
            _qContext->create();
            if (_qContext->isValid())
                _allQtContexts.push_back(_qContext);
            else
            { // context creation failed. Use an invisible window instead:
                delete _qContext;
                _qOffscreenSurface->destroy();
                delete _qOffscreenSurface;
                _offscreenType=QT_WINDOW_HIDE_TP;
                #ifdef USES_QGLWIDGET
                    App::logMsg(sim_verbosity_errors,"failed creating an offscreen QOpenGLContext. Switching to a hidden QGLWidget.");
                #else
                    App::logMsg(sim_verbosity_errors,"failed creating an offscreen QOpenGLContext. Switching to a hidden QOpenGLWidget.");
                #endif
            }
        }
        else
        { // surface creation failed. Use an invisible window instead:
            delete _qOffscreenSurface;
            _offscreenType=QT_WINDOW_HIDE_TP;
            #ifdef USES_QGLWIDGET
                App::logMsg(sim_verbosity_errors,"failed creating a QOffscreenSurface. Switching to a hidden QGLWidget.");
            #else
                App::logMsg(sim_verbosity_errors,"failed creating a QOffscreenSurface. Switching to a hidden QOpenGLWidget.");
            #endif
        }
    }

#ifdef SIM_WITH_GUI
    if ((_offscreenType==QT_WINDOW_SHOW_TP)||(_offscreenType==QT_WINDOW_HIDE_TP))
    {
        if ((otherWidgetToShareResourcesWith==nullptr)&&(_allQtWidgets.size()!=0))
            otherWidgetToShareResourcesWith=_allQtWidgets[0];

#ifdef USES_QGLWIDGET
        if ((majorOpenGl!=-1)||(minorOpenGl!=-1))
        {
            QGLFormat fmt;
            fmt.setDoubleBuffer(false);
            // fmt.setDirectRendering(false);   // default is true
            fmt.setRedBufferSize(8);
            fmt.setGreenBufferSize(8);
            fmt.setBlueBufferSize(8);
            fmt.setAlphaBufferSize(0);
            fmt.setStencilBufferSize(8);
            fmt.setDepthBufferSize(24);
            if ((majorOpenGl!=-1)||(minorOpenGl!=-1))
            {
                if (majorOpenGl==-1)
                    majorOpenGl=2;
                if (minorOpenGl==-1)
                    minorOpenGl=0;
                fmt.setVersion(majorOpenGl,minorOpenGl);
            }
            _hiddenWindow=new QGLWidget(fmt,GuiApp::mainWindow,otherWidgetToShareResourcesWith,Qt::Tool);
        }
        else
#endif
        {
            #ifdef USES_QGLWIDGET
                _hiddenWindow=new QGLWidget(GuiApp::mainWindow,otherWidgetToShareResourcesWith,Qt::Tool);
            #else
                _hiddenWindow=new QOpenGLWidget(GuiApp::mainWindow,Qt::Tool);
            #endif
            _hiddenWindow->setFixedWidth(resX);
            _hiddenWindow->setFixedHeight(resY);

            if (_offscreenType==QT_WINDOW_SHOW_TP)
                _hiddenWindow->show();
        }
        _allQtWidgets.push_back(_hiddenWindow);
    }
#endif

    makeCurrent();
    initGl_ifNeeded();
    oglExt::initDefaultGlValues(); // important to call it here, in case the context was already created
}

COffscreenGlContext::~COffscreenGlContext()
{
    TRACE_INTERNAL;
    if (_offscreenType==QT_OFFSCREEN_TP)
    {
        if ( (!_isHeadless)||(_allQtContexts.size()>1) ) // otherwise we have a problem with context sharing somehow, in headless mode... we probably can live with this for the time being
        {
            for (size_t i=0;i<_allQtContexts.size();i++)
            {
                if (_allQtContexts[i]==_qContext)
                {
                    _allQtContexts.erase(_allQtContexts.begin()+i);
                    break;
                }
            }
            delete _qContext;
            _qOffscreenSurface->destroy();
            delete _qOffscreenSurface;
        }
    }

    if ((_offscreenType==QT_WINDOW_SHOW_TP)||(_offscreenType==QT_WINDOW_HIDE_TP))
    {
        if ( (!_isHeadless)||(_allQtContexts.size()>1) ) // otherwise we have a problem with context sharing somehow, in headless mode... we probably can live with this for the time being
        {
            for (size_t i=0;i<_allQtWidgets.size();i++)
            {
                if (_allQtWidgets[i]==_hiddenWindow)
                {
                    _allQtWidgets.erase(_allQtWidgets.begin()+i);
                    break;
                }
            }
            _hiddenWindow->hide(); // required, otherwise crash at simulator exit when in headless mode
            _hiddenWindow->deleteLater(); // delete _hiddenWindow works also (with hide first) under Windows
        }
    }
}

bool COffscreenGlContext::canBeDeleted()
{
    return(QThread::currentThread()==_initialThread);
}

bool COffscreenGlContext::makeCurrent()
{
    TRACE_INTERNAL;
    if (_offscreenType==QT_OFFSCREEN_TP)
    {
        if (QThread::currentThread()==_initialThread)
        {
            _qContext->makeCurrent(_qOffscreenSurface);
            return(true);
        }
        else
            return(false); // we cannot pull a Qt context from another thread!
    }

    if ((_offscreenType==QT_WINDOW_SHOW_TP)||(_offscreenType==QT_WINDOW_HIDE_TP))
    {
        if (QThread::currentThread()==_initialThread)
        {
            _hiddenWindow->makeCurrent();
            return(true);
        }
        else
            return(false); // we cannot pull a Qt context from another thread!
    }

    return(false);
}

bool COffscreenGlContext::doneCurrent()
{
    TRACE_INTERNAL;
    if (_offscreenType==QT_OFFSCREEN_TP)
    {
        if (QThread::currentThread()==_initialThread)
        {
            _qContext->doneCurrent();
            return(true);
        }
    }

    if ((_offscreenType==QT_WINDOW_SHOW_TP)||(_offscreenType==QT_WINDOW_HIDE_TP))
    {
        if (QThread::currentThread()==_initialThread)
        {
            _hiddenWindow->doneCurrent();
            return(true);
        }
    }

    if (QThread::currentThread()!=_initialThread)
    {
        App::logMsg(sim_verbosity_errors,"do not call COffscreenGlContext::doneCurrent() from a non-initial thread.");
        App::beep();
    }
    return(false);
}

void COffscreenGlContext::moveGlContextToThread(QThread* otherThread)
{ // not used!
    TRACE_INTERNAL;
    if (_offscreenType==QT_OFFSCREEN_TP)
        _qContext->moveToThread(otherThread);

    if ((_offscreenType==QT_WINDOW_SHOW_TP)||(_offscreenType==QT_WINDOW_HIDE_TP))
        _hiddenWindow->context()->moveToThread(otherThread);

    _initialThread=otherThread;
}

