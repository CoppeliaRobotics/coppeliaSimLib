
#ifndef QSIM_OPENGLWIDGET_H
#define QSIM_OPENGLWIDGET_H

#ifdef USES_QGLWIDGET
#include <QGLWidget>
#else
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#endif
#include <QString>

struct SModelThumbnailInfo;

struct SMouseOrKeyboardOrResizeEvent
{
    int eventType; // 0=mouse press, 1=mouse release, 2=mouse dbl click,
                   // 3=mouse move, 4=mouse wheel, 5=key press,
                   // 6=key release, 7=resize, 8=timer
    int button;
    int x;
    int y;
    bool ctrlDown;
    bool shiftDown;
    int wheelDelta;
    int key;
    int specialKey;
    QString unicodeText;
};

#ifdef USES_QGLWIDGET
class COpenglWidget : public QGLWidget
#else
class COpenglWidget : public QOpenGLWidget, protected QOpenGLFunctions
#endif
{
    Q_OBJECT
  public:
    explicit COpenglWidget(QWidget* parent = 0);
    ~COpenglWidget();

    void makeContextCurrent();

    void dragEnterEvent(QDragEnterEvent* dEvent) override;
    void dragLeaveEvent(QDragLeaveEvent* dEvent) override;
    void dragMoveEvent(QDragMoveEvent* dEvent) override;
    void dropEvent(QDropEvent* dEvent) override;

    SModelThumbnailInfo* getModelDragAndDropInfo();
    void clearModelDragAndDropInfo();

  protected:
    void _setCtrlAndShiftKeyState(bool ctrlDown, bool shiftDown);
    void _computeMousePos(int inX, int inY, int& outX, int& outY);
    void _handleMouseAndKeyboardAndResizeEvents(void* event, int t);

    void mousePressEvent(QMouseEvent* mEvent) override;
    void _mousePressEvent(SMouseOrKeyboardOrResizeEvent e);
    void mouseReleaseEvent(QMouseEvent* mEvent) override;
    void _mouseReleaseEvent(SMouseOrKeyboardOrResizeEvent e);
    void mouseDoubleClickEvent(QMouseEvent* mEvent) override;
    void _mouseDoubleClickEvent(SMouseOrKeyboardOrResizeEvent e);
    void mouseMoveEvent(QMouseEvent* mEvent) override;
    void _mouseMoveEvent(SMouseOrKeyboardOrResizeEvent e);

    void wheelEvent(QWheelEvent* wEvent) override;
    void _wheelEvent(SMouseOrKeyboardOrResizeEvent e);

    bool focusNextPrevChild(bool next) override;
    void keyPressEvent(QKeyEvent* kEvent) override;
    void _keyPressEvent(SMouseOrKeyboardOrResizeEvent e);
    void keyReleaseEvent(QKeyEvent* kEvent) override;
    void _keyReleaseEvent(SMouseOrKeyboardOrResizeEvent e);

    void resizeEvent(QResizeEvent* rEvent) override;
    void _resizeEvent(SMouseOrKeyboardOrResizeEvent e);

    void initializeGL() override;
    void paintGL() override;

    int _lastGlobalMousePos[2];
    SModelThumbnailInfo* _modelDragAndDropInfo;
  signals:

  private slots:
    void _timer100ms_fire();
};
#endif // QSIM_OPENGLWIDGET_H
