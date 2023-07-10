#pragma once

#include <QOpenGLFramebufferObject>
#include <QThread>

class CFrameBufferObject : public QObject
{
    Q_OBJECT
public:

    CFrameBufferObject(bool native,int resX,int resY,bool useStencilBuffer);
    virtual ~CFrameBufferObject();

    void switchToFbo();
    void switchToNonFbo();

    bool getUsingStencilBuffer();
    bool canBeDeleted();
protected:
    bool _native;
    QThread* _initialThread;

    // Data for direct (i.e. native) handling of FBO's:
    unsigned int _fbo;
    unsigned int _fboDepthBuffer;
    unsigned int _fboPictureBuffer;
    bool _usingStencilBuffer;

    // Data for handling FBO's via Qt's QOpenGLFramebufferObject:
    QOpenGLFramebufferObject* _frameBufferObject;
};
