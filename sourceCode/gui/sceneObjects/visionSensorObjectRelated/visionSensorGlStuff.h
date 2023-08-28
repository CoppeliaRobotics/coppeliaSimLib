#pragma once

#include <offscreenGlContext.h>
#include <frameBufferObject.h>
#include <textureObject.h>

class CVisionSensorGlStuff : public QObject
{
    Q_OBJECT
public:

    #ifdef USES_QGLWIDGET
        CVisionSensorGlStuff(int resX,int resY,int offscreenType,bool qtFbo,QGLWidget* otherWidgetToShareResourcesWith,bool useStencilBuffer,bool destroyOffscreenContext);
    #else
        CVisionSensorGlStuff(int resX,int resY,int offscreenType,bool qtFbo,QOpenGLWidget* otherWidgetToShareResourcesWith,bool useStencilBuffer,bool destroyOffscreenContext);
    #endif
    virtual ~CVisionSensorGlStuff();

    COffscreenGlContext* offscreenContext;
    CFrameBufferObject* frameBufferObject;
    CTextureObject* textureObject;
protected:
    bool _destroyOffscreenContext;
};
