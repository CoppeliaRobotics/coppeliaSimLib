
#include "vrepMainHeader.h"
#include "funcDebug.h"
#include "visionSensorGlStuff.h"
#include "glShader.h"

#ifdef USING_QOPENGLWIDGET
CVisionSensorGlStuff::CVisionSensorGlStuff(int resX,int resY,int offscreenType,bool qtFbo,QOpenGLWidget* otherWidgetToShareResourcesWith,bool useStencilBuffer,bool destroyOffscreenContext,int majorOpenGl,int minorOpenGl) : QObject()
#else
CVisionSensorGlStuff::CVisionSensorGlStuff(int resX,int resY,int offscreenType,bool qtFbo,QGLWidget* otherWidgetToShareResourcesWith,bool useStencilBuffer,bool destroyOffscreenContext,int majorOpenGl,int minorOpenGl) : QObject()
#endif
{
    _destroyOffscreenContext=destroyOffscreenContext;

    // 1. We need an offscreen context:
    offscreenContext=new COffscreenGlContext(offscreenType,resX,resY,otherWidgetToShareResourcesWith,majorOpenGl,minorOpenGl);

    // 2. We need an FBO:
    frameBufferObject=new CFrameBufferObject(!qtFbo,resX,resY,useStencilBuffer);

    // 3. We need a texture object:
    textureObject=new CTextureObject(resX,resY);

//  CGlShader* a=new CGlShader();

    offscreenContext->doneCurrent();
}

CVisionSensorGlStuff::~CVisionSensorGlStuff()
{
    FUNCTION_DEBUG;
    offscreenContext->makeCurrent();
    delete textureObject;
    delete frameBufferObject;
    offscreenContext->doneCurrent();

    // On some buggy GPU drivers, destroying the context here might hinder further renderings in other contexts:
    if (_destroyOffscreenContext)
        delete offscreenContext;
}

bool CVisionSensorGlStuff::canDeleteNow()
{
    return(offscreenContext->canBeDeleted());
}
