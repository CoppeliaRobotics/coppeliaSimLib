#pragma once

#include <QOpenGLContext>
#include <QOffscreenSurface>
#ifdef USES_QGLWIDGET
#include <QGLWidget>
#else
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#endif
#include <vThread.h>

#ifdef USES_QGLWIDGET
class COffscreenGlContext : public QObject
#else
class COffscreenGlContext : public QObject, protected QOpenGLFunctions
#endif
{
    Q_OBJECT
  public:
#ifdef USES_QGLWIDGET
    COffscreenGlContext(int offscreenType, int resX, int resY, QGLWidget* otherWidgetToShareResourcesWith);
#else
    COffscreenGlContext(int offscreenType, int resX, int resY, QOpenGLWidget* otherWidgetToShareResourcesWith);
#endif
    virtual ~COffscreenGlContext();

    bool makeCurrent();
    bool doneCurrent();

    enum OFFSCREEN_TYPE
    {
        QT_OFFSCREEN_TP = 0,
        QT_WINDOW_SHOW_TP = 1,
        QT_WINDOW_HIDE_TP = 2
    };

  protected:
    int _offscreenType;
    bool _isHeadless;

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
