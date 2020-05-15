
#pragma once

#include "offscreenGlContext.h"
#include "frameBufferObject.h"
#include "textureObject.h"

class CVisionSensorGlStuff : public QObject
{
    Q_OBJECT
public:

#ifdef USING_QOPENGLWIDGET
    CVisionSensorGlStuff(int resX,int resY,int offscreenType,bool qtFbo,QOpenGLWidget* otherWidgetToShareResourcesWith,bool useStencilBuffer,bool destroyOffscreenContext,int majorOpenGl,int minorOpenGl);
#else
    CVisionSensorGlStuff(int resX,int resY,int offscreenType,bool qtFbo,QGLWidget* otherWidgetToShareResourcesWith,bool useStencilBuffer,bool destroyOffscreenContext,int majorOpenGl,int minorOpenGl);
#endif
    virtual ~CVisionSensorGlStuff();

    bool canDeleteNow();

    COffscreenGlContext* offscreenContext;
    CFrameBufferObject* frameBufferObject;
    CTextureObject* textureObject;
protected:
    bool _destroyOffscreenContext;
};
