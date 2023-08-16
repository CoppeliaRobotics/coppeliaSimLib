#pragma once

#include <QOpenGLContext>
#include <QOffscreenSurface>
#ifdef USES_QGLWIDGET
    #include <QGLWidget>
#else
    #include <QOpenGLWidget>
#endif
#include <vThread.h>

class COffscreenGlContext : public QObject
{
    Q_OBJECT
public:

    #ifdef USES_QGLWIDGET
        COffscreenGlContext(int offscreenType,int resX,int resY,QGLWidget *otherWidgetToShareResourcesWith,int majorOpenGl,int minorOpenGl);
    #else
        COffscreenGlContext(int offscreenType,int resX,int resY,QOpenGLWidget *otherWidgetToShareResourcesWith,int majorOpenGl,int minorOpenGl);
    #endif
    virtual ~COffscreenGlContext();

    bool makeCurrent();
    bool doneCurrent();

    bool canBeDeleted();
    void moveGlContextToThread(QThread* otherThread);

    enum OFFSCREEN_TYPE
    {
        QT_OFFSCREEN_TP = 0,
        QT_WINDOW_SHOW_TP =1,
        QT_WINDOW_HIDE_TP =2
    };

protected:
    int _offscreenType;
    bool _isHeadless;
    QThread* _initialThread;

    // Native window vars:
#ifdef WIN_SIM
    HWND _windowHandle;
    HDC _hdc;
    HGLRC _nativeContext;
    static std::vector<HGLRC> _allNativeContexts;
#endif

    // Qt offscreen vars:
    QOpenGLContext* _qContext;
    QOffscreenSurface* _qOffscreenSurface;

    #ifdef USES_QGLWIDGET
        QGLWidget* _hiddenWindow;
        static std::vector<QGLWidget*> _allQtWidgets;
    #else
        QOpenGLWidget* _hiddenWindow;
        static std::vector<QOpenGLWidget*> _allQtWidgets;
    #endif

    static std::vector<QOpenGLContext*> _allQtContexts;
};
