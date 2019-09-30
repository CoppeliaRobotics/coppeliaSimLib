#include "funcDebug.h"
#include "frameBufferObject.h"
#include "oglExt.h"

CFrameBufferObject::CFrameBufferObject(bool native,int resX,int resY,bool useStencilBuffer) : QObject()
{
    FUNCTION_DEBUG;

    if (!oglExt::isFboAvailable())
        native=false;

    _native=native;
    _usingStencilBuffer=useStencilBuffer;

    _initialThread=QThread::currentThread();

    if (_native)
    {
        unsigned int theMode=GL_DEPTH_COMPONENT;
        unsigned int attachment=oglExt::DEPTH_ATTACHMENT;
        if (_usingStencilBuffer)
        {
            theMode=oglExt::DEPTH24_STENCIL8;
            attachment=oglExt::DEPTH_STENCIL_ATTACHMENT;
        }

        oglExt::GenFramebuffers(1,&_fbo);
        oglExt::BindFramebuffer(oglExt::FRAMEBUFFER,_fbo);

        oglExt::GenRenderbuffers(1,&_fboDepthBuffer);
        oglExt::BindRenderbuffer(oglExt::RENDERBUFFER,_fboDepthBuffer);

        oglExt::RenderbufferStorage(oglExt::RENDERBUFFER,theMode,resX,resY);
        oglExt::FramebufferRenderbuffer(oglExt::FRAMEBUFFER,attachment,oglExt::RENDERBUFFER,_fboDepthBuffer);

        oglExt::GenRenderbuffers(1,&_fboPictureBuffer);
        oglExt::BindRenderbuffer(oglExt::RENDERBUFFER,_fboPictureBuffer);

        oglExt::RenderbufferStorage(oglExt::RENDERBUFFER,GL_RGB,resX,resY);
        oglExt::FramebufferRenderbuffer(oglExt::FRAMEBUFFER,oglExt::COLOR_ATTACHMENT0,oglExt::RENDERBUFFER,_fboPictureBuffer);
    }
    else
    {
        QOpenGLFramebufferObject::Attachment attachment=QOpenGLFramebufferObject::Depth;
        if (_usingStencilBuffer)
            attachment=QOpenGLFramebufferObject::CombinedDepthStencil;
        _frameBufferObject = new QOpenGLFramebufferObject(resX,resY,attachment,GL_TEXTURE_2D,GL_RGBA8); // GL_RGB);
    }
}

CFrameBufferObject::~CFrameBufferObject()
{
    FUNCTION_DEBUG;
    switchToNonFbo();
    if (_native)
    {
        oglExt::DeleteRenderbuffers(1,&_fboPictureBuffer);
        oglExt::DeleteRenderbuffers(1,&_fboDepthBuffer);
        oglExt::DeleteFramebuffers(1,&_fbo);
    }
    else
        delete _frameBufferObject;
}

bool CFrameBufferObject::canBeDeleted()
{
    return(_initialThread==QThread::currentThread());
}

void CFrameBufferObject::switchToFbo()
{
    FUNCTION_DEBUG;
    if (_native)
        oglExt::BindFramebuffer(oglExt::FRAMEBUFFER,_fbo);
    else
        _frameBufferObject->bind();
}

void CFrameBufferObject::switchToNonFbo()
{
    FUNCTION_DEBUG;
    if (_native)
        oglExt::BindFramebuffer(oglExt::FRAMEBUFFER,0);
    else
        _frameBufferObject->release();
}

bool CFrameBufferObject::getUsingStencilBuffer()
{
    return(_usingStencilBuffer);
}
