
#ifndef QVREP_OPENGLWIDGET_H
#define QVREP_OPENGLWIDGET_H

#ifdef USING_QOPENGLWIDGET
#include <QOpenGLWidget>
#else
#include <QGLWidget>
#endif
#include <QString>

struct SModelThumbnailInfo;

struct SMouseOrKeyboardOrResizeEvent
{
    int eventType;  // 0=mouse press, 1=mouse release, 2=mouse dbl click,
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

#ifdef USING_QOPENGLWIDGET
class COpenglWidget : public QOpenGLWidget
#else
class COpenglWidget : public QGLWidget
#endif
{
    Q_OBJECT
public:
    explicit COpenglWidget(QWidget *parent = 0);
    ~COpenglWidget();

    void makeContextCurrent();

    void dragEnterEvent(QDragEnterEvent* dEvent);
    void dragLeaveEvent(QDragLeaveEvent* dEvent);
    void dragMoveEvent(QDragMoveEvent* dEvent);
    void dropEvent(QDropEvent* dEvent);

    SModelThumbnailInfo* getModelDragAndDropInfo();
    void clearModelDragAndDropInfo();


protected:
    void _setCtrlAndShiftKeyState(bool ctrlDown,bool shiftDown);
    void _computeMousePos(int inX,int inY,int& outX,int& outY);
    void _handleMouseAndKeyboardAndResizeEvents(void* event,int t);

    void mousePressEvent(QMouseEvent* mEvent);
    void _mousePressEvent(SMouseOrKeyboardOrResizeEvent e);
    void mouseReleaseEvent(QMouseEvent* mEvent);
    void _mouseReleaseEvent(SMouseOrKeyboardOrResizeEvent e);
    void mouseDoubleClickEvent(QMouseEvent* mEvent);
    void _mouseDoubleClickEvent(SMouseOrKeyboardOrResizeEvent e);
    void mouseMoveEvent(QMouseEvent* mEvent);
    void _mouseMoveEvent(SMouseOrKeyboardOrResizeEvent e);

    void wheelEvent(QWheelEvent* wEvent);
    void _wheelEvent(SMouseOrKeyboardOrResizeEvent e);

    bool focusNextPrevChild(bool next);
    void keyPressEvent(QKeyEvent* kEvent);
    void _keyPressEvent(SMouseOrKeyboardOrResizeEvent e);
    void keyReleaseEvent(QKeyEvent* kEvent);
    void _keyReleaseEvent(SMouseOrKeyboardOrResizeEvent e);

    void resizeEvent(QResizeEvent* rEvent);
    void _resizeEvent(SMouseOrKeyboardOrResizeEvent e);


    void initializeGL();
    void paintGL();

    void paintEvent(QPaintEvent* event);

    int _lastGlobalMousePos[2];
    SModelThumbnailInfo* _modelDragAndDropInfo;
signals:

private slots:
    void _timer100ms_fire();

};
#endif // QVREP_OPENGLWIDGET_H
