#pragma once

#include <QOpenGLContext>
#include <QOffscreenSurface>
#include <QGLWidget>
#include <vThread.h>

class COffscreenGlContext : public QObject
{
    Q_OBJECT
public:

    COffscreenGlContext(int offscreenType,int resX,int resY,QGLWidget *otherWidgetToShareResourcesWith,int majorOpenGl,int minorOpenGl);
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

    // Qt window vars:
    QGLWidget* _hiddenWindow;

    static std::vector<QOpenGLContext*> _allQtContexts;
    static std::vector<QGLWidget*> _allQtWidgets;
};
