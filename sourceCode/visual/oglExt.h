
#pragma once
#include <oGL.h>

#ifdef WIN_SIM
//  #include <qgl.h>
//  #include <GL/gl.h>
//  #include "glext.h"
//  #define GL_DEPTH24_STENCIL8_EXT 0x88F0
#endif

#ifdef LIN_SIM
    #include <GL/glext.h>
    #include <GL/gl.h>
    #include <GL/glx.h>
    #include <GL/glxext.h>
#endif

#ifdef MAC_SIM
//  #define GL_GLEXT_FUNCTION_POINTERS 1
    #include <OpenGL/OpenGL.h>
    #include <OpenGL/gl.h>
    #include <OpenGL/gl3.h>
    #include <OpenGL/glext.h>
    #include <OpenGL/gl3ext.h>
#endif

#ifndef MAC_SIM
    extern PFNGLGENFRAMEBUFFERSEXTPROC _glGenFramebuffers;
    extern PFNGLDELETEFRAMEBUFFERSEXTPROC _glDeleteFramebuffers;
    extern PFNGLBINDFRAMEBUFFEREXTPROC _glBindFramebuffer;
    extern PFNGLGENRENDERBUFFERSEXTPROC _glGenRenderbuffers;
    extern PFNGLDELETERENDERBUFFERSEXTPROC _glDeleteRenderbuffers;
    extern PFNGLBINDRENDERBUFFEREXTPROC _glBindRenderbuffer;
    extern PFNGLRENDERBUFFERSTORAGEEXTPROC _glRenderbufferStorage;
    extern PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC _glFramebufferRenderbuffer;
    extern PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC _glCheckFramebufferStatus;
    extern PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC _glGetRenderbufferParameteriv;
#endif

//FULLY STATIC CLASS
class oglExt 
{
public:
    static void prepareExtensionFunctions(bool forceFboToUseExt);
    static void turnOffVSync(int vsync);
    static bool isFboAvailable();
    static bool _isFboAvailable(bool &viaExt);
    static bool areNonPowerOfTwoTexturesAvailable();
    static void initDefaultGlValues();


    static void GenFramebuffers(GLsizei a,GLuint* b);
    static void DeleteFramebuffers(GLsizei a,const GLuint* b);
    static void BindFramebuffer(GLenum a,GLuint b);
    static void GenRenderbuffers(GLsizei a,GLuint* b);
    static void DeleteRenderbuffers(GLsizei a,const GLuint* b);
    static void BindRenderbuffer(GLenum a,GLuint b);
    static void RenderbufferStorage(GLenum a,GLenum b,GLsizei c,GLsizei d);
    static void FramebufferRenderbuffer(GLenum a,GLenum b,GLenum c,GLuint d);
    static void CheckFramebufferStatus(GLenum a);
    static void GetRenderbufferParameteriv(GLenum a,GLenum b,GLint* c);

#ifndef MAC_SIM
    static PFNGLGENFRAMEBUFFERSEXTPROC _glGenFramebuffers;
    static PFNGLDELETEFRAMEBUFFERSEXTPROC _glDeleteFramebuffers;
    static PFNGLBINDFRAMEBUFFEREXTPROC _glBindFramebuffer;
    static PFNGLGENRENDERBUFFERSEXTPROC _glGenRenderbuffers;
    static PFNGLDELETERENDERBUFFERSEXTPROC _glDeleteRenderbuffers;
    static PFNGLBINDRENDERBUFFEREXTPROC _glBindRenderbuffer;
    static PFNGLRENDERBUFFERSTORAGEEXTPROC _glRenderbufferStorage;
    static PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC _glFramebufferRenderbuffer;
    static PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC _glCheckFramebufferStatus;
    static PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC _glGetRenderbufferParameteriv;
#endif

    static GLenum DEPTH24_STENCIL8;
    static GLenum FRAMEBUFFER;
    static GLenum RENDERBUFFER;
    static GLenum DEPTH_ATTACHMENT;
    static GLenum DEPTH_STENCIL_ATTACHMENT;
    static GLenum COLOR_ATTACHMENT0;

    static bool _usingExt;
};
