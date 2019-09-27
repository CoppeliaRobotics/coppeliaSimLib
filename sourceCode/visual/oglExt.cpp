
#include "oglExt.h"

#ifndef MAC_VREP
PFNGLGENFRAMEBUFFERSEXTPROC oglExt::_glGenFramebuffers=nullptr;
PFNGLDELETEFRAMEBUFFERSEXTPROC oglExt::_glDeleteFramebuffers=nullptr;
PFNGLBINDFRAMEBUFFEREXTPROC oglExt::_glBindFramebuffer=nullptr;
PFNGLGENRENDERBUFFERSEXTPROC oglExt::_glGenRenderbuffers=nullptr;
PFNGLDELETERENDERBUFFERSEXTPROC oglExt::_glDeleteRenderbuffers=nullptr;
PFNGLBINDRENDERBUFFEREXTPROC oglExt::_glBindRenderbuffer=nullptr;
PFNGLRENDERBUFFERSTORAGEEXTPROC oglExt::_glRenderbufferStorage=nullptr;
PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC oglExt::_glFramebufferRenderbuffer=nullptr;
PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC oglExt::_glCheckFramebufferStatus=nullptr;
PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC oglExt::_glGetRenderbufferParameteriv=nullptr;
#endif

GLenum oglExt::DEPTH24_STENCIL8=GL_DEPTH24_STENCIL8_EXT;
GLenum oglExt::FRAMEBUFFER=GL_FRAMEBUFFER_EXT;
GLenum oglExt::RENDERBUFFER=GL_RENDERBUFFER_EXT;
GLenum oglExt::DEPTH_ATTACHMENT=GL_DEPTH_ATTACHMENT_EXT;
GLenum oglExt::DEPTH_STENCIL_ATTACHMENT=GL_DEPTH_STENCIL_ATTACHMENT; // GL_DEPTH_STENCIL_ATTACHMENT_EXT doesn't exist
GLenum oglExt::COLOR_ATTACHMENT0=GL_COLOR_ATTACHMENT0_EXT;

bool oglExt::_usingExt=false;

void oglExt::GenFramebuffers(GLsizei a,GLuint* b)
{
#ifndef MAC_VREP
    _glGenFramebuffers(a,b);
#else
    if (_usingExt)
        glDeleteFramebuffersEXT(a,b);
    else
        glDeleteFramebuffers(a,b);
#endif
}

void oglExt::DeleteFramebuffers(GLsizei a,const GLuint* b)
{
#ifndef MAC_VREP
    _glDeleteFramebuffers(a,b);
#else
    if (_usingExt)
        glDeleteFramebuffersEXT(a,b);
    else
        glDeleteFramebuffers(a,b);
#endif
}

void oglExt::BindFramebuffer(GLenum a,GLuint b)
{
#ifndef MAC_VREP
    _glBindFramebuffer(a,b);
#else
    if (_usingExt)
        glBindFramebufferEXT(a,b);
    else
        glBindFramebuffer(a,b);
#endif
}

void oglExt::GenRenderbuffers(GLsizei a,GLuint* b)
{
#ifndef MAC_VREP
    _glGenRenderbuffers(a,b);
#else
    if (_usingExt)
        glGenRenderbuffersEXT(a,b);
    else
        glGenRenderbuffers(a,b);
#endif
}

void oglExt::DeleteRenderbuffers(GLsizei a,const GLuint* b)
{
#ifndef MAC_VREP
    _glDeleteRenderbuffers(a,b);
#else
    if (_usingExt)
        glDeleteRenderbuffersEXT(a,b);
    else
        glDeleteRenderbuffers(a,b);
#endif
}

void oglExt::BindRenderbuffer(GLenum a,GLuint b)
{
#ifndef MAC_VREP
    _glBindRenderbuffer(a,b);
#else
    if (_usingExt)
        glBindRenderbufferEXT(a,b);
    else
        glBindRenderbuffer(a,b);
#endif
}

void oglExt::RenderbufferStorage(GLenum a,GLenum b,GLsizei c,GLsizei d)
{
#ifndef MAC_VREP
    _glRenderbufferStorage(a,b,c,d);
#else
    if (_usingExt)
        glRenderbufferStorageEXT(a,b,c,d);
    else
        glRenderbufferStorage(a,b,c,d);
#endif
}

void oglExt::FramebufferRenderbuffer(GLenum a,GLenum b,GLenum c,GLuint d)
{
#ifndef MAC_VREP
    _glFramebufferRenderbuffer(a,b,c,d);
#else
    if (_usingExt)
        glFramebufferRenderbufferEXT(a,b,c,d);
    else
        glFramebufferRenderbuffer(a,b,c,d);
#endif
}

void oglExt::CheckFramebufferStatus(GLenum a)
{
#ifndef MAC_VREP
    _glCheckFramebufferStatus(a);
#else
    if (_usingExt)
        glCheckFramebufferStatusEXT(a);
    else
        glCheckFramebufferStatus(a);
#endif
}

void oglExt::GetRenderbufferParameteriv(GLenum a,GLenum b,GLint* c)
{
#ifndef MAC_VREP
    _glGetRenderbufferParameteriv(a,b,c);
#else
    if (_usingExt)
        glGetRenderbufferParameterivEXT(a,b,c);
    else
        glGetRenderbufferParameteriv(a,b,c);
#endif
}

void oglExt::prepareExtensionFunctions(bool forceFboToUseExt)
{
    _usingExt=false;
    _isFboAvailable(_usingExt);
#ifdef WIN_VREP
    if (_usingExt||forceFboToUseExt)
    {
        _glGenFramebuffers=(PFNGLGENFRAMEBUFFERSEXTPROC)wglGetProcAddress("glGenFramebuffersEXT");
        _glDeleteFramebuffers=(PFNGLDELETEFRAMEBUFFERSEXTPROC)wglGetProcAddress("glDeleteFramebuffersEXT");
        _glBindFramebuffer=(PFNGLBINDFRAMEBUFFEREXTPROC)wglGetProcAddress("glBindFramebufferEXT");
        _glGenRenderbuffers=(PFNGLGENRENDERBUFFERSEXTPROC)wglGetProcAddress("glGenRenderbuffersEXT");
        _glDeleteRenderbuffers=(PFNGLDELETERENDERBUFFERSEXTPROC)wglGetProcAddress("glDeleteRenderbuffersEXT");
        _glBindRenderbuffer=(PFNGLBINDRENDERBUFFEREXTPROC)wglGetProcAddress("glBindRenderbufferEXT");
        _glRenderbufferStorage=(PFNGLRENDERBUFFERSTORAGEEXTPROC)wglGetProcAddress("glRenderbufferStorageEXT");
        _glFramebufferRenderbuffer=(PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC)wglGetProcAddress("glFramebufferRenderbufferEXT");
        _glCheckFramebufferStatus=(PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC)wglGetProcAddress("glCheckFramebufferStatusEXT");
        _glGetRenderbufferParameteriv=(PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC)wglGetProcAddress("glGetRenderbufferParameterivEXT");
    }
    else
    {
        _glGenFramebuffers=(PFNGLGENFRAMEBUFFERSEXTPROC)wglGetProcAddress("glGenFramebuffers");
        _glDeleteFramebuffers=(PFNGLDELETEFRAMEBUFFERSEXTPROC)wglGetProcAddress("glDeleteFramebuffers");
        _glBindFramebuffer=(PFNGLBINDFRAMEBUFFEREXTPROC)wglGetProcAddress("glBindFramebuffer");
        _glGenRenderbuffers=(PFNGLGENRENDERBUFFERSEXTPROC)wglGetProcAddress("glGenRenderbuffers");
        _glDeleteRenderbuffers=(PFNGLDELETERENDERBUFFERSEXTPROC)wglGetProcAddress("glDeleteRenderbuffers");
        _glBindRenderbuffer=(PFNGLBINDRENDERBUFFEREXTPROC)wglGetProcAddress("glBindRenderbuffer");
        _glRenderbufferStorage=(PFNGLRENDERBUFFERSTORAGEEXTPROC)wglGetProcAddress("glRenderbufferStorage");
        _glFramebufferRenderbuffer=(PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC)wglGetProcAddress("glFramebufferRenderbuffer");
        _glCheckFramebufferStatus=(PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC)wglGetProcAddress("glCheckFramebufferStatus");
        _glGetRenderbufferParameteriv=(PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC)wglGetProcAddress("glGetRenderbufferParameteriv");

        DEPTH24_STENCIL8=GL_DEPTH24_STENCIL8;
        FRAMEBUFFER=GL_FRAMEBUFFER;
        RENDERBUFFER=GL_RENDERBUFFER;
        DEPTH_ATTACHMENT=GL_DEPTH_ATTACHMENT;
        COLOR_ATTACHMENT0=GL_COLOR_ATTACHMENT0;
        DEPTH_STENCIL_ATTACHMENT=GL_DEPTH_STENCIL_ATTACHMENT;
    }
#endif

#ifdef LIN_VREP
    if (_usingExt||forceFboToUseExt)
    {
        _glGenFramebuffers=(PFNGLGENFRAMEBUFFERSEXTPROC)glXGetProcAddress((GLubyte*)"glGenFramebuffersEXT");
        _glDeleteFramebuffers=(PFNGLDELETEFRAMEBUFFERSEXTPROC)glXGetProcAddress((GLubyte*)"glDeleteFramebuffersEXT");
        _glBindFramebuffer=(PFNGLBINDFRAMEBUFFEREXTPROC)glXGetProcAddress((GLubyte*)"glBindFramebufferEXT");
        _glGenRenderbuffers=(PFNGLGENRENDERBUFFERSEXTPROC)glXGetProcAddress((GLubyte*)"glGenRenderbuffersEXT");
        _glDeleteRenderbuffers=(PFNGLDELETERENDERBUFFERSEXTPROC)glXGetProcAddress((GLubyte*)"glDeleteRenderbuffersEXT");
        _glBindRenderbuffer=(PFNGLBINDRENDERBUFFEREXTPROC)glXGetProcAddress((GLubyte*)"glBindRenderbufferEXT");
        _glRenderbufferStorage=(PFNGLRENDERBUFFERSTORAGEEXTPROC)glXGetProcAddress((GLubyte*)"glRenderbufferStorageEXT");
        _glFramebufferRenderbuffer=(PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC)glXGetProcAddress((GLubyte*)"glFramebufferRenderbufferEXT");
        _glCheckFramebufferStatus=(PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC)glXGetProcAddress((GLubyte*)"glCheckFramebufferStatusEXT");
        _glGetRenderbufferParameteriv=(PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC)glXGetProcAddress((GLubyte*)"glGetRenderbufferParameterivEXT");
    }
    else
    {
        _glGenFramebuffers=(PFNGLGENFRAMEBUFFERSEXTPROC)glXGetProcAddress((GLubyte*)"glGenFramebuffers");
        _glDeleteFramebuffers=(PFNGLDELETEFRAMEBUFFERSEXTPROC)glXGetProcAddress((GLubyte*)"glDeleteFramebuffers");
        _glBindFramebuffer=(PFNGLBINDFRAMEBUFFEREXTPROC)glXGetProcAddress((GLubyte*)"glBindFramebuffer");
        _glGenRenderbuffers=(PFNGLGENRENDERBUFFERSEXTPROC)glXGetProcAddress((GLubyte*)"glGenRenderbuffers");
        _glDeleteRenderbuffers=(PFNGLDELETERENDERBUFFERSEXTPROC)glXGetProcAddress((GLubyte*)"glDeleteRenderbuffers");
        _glBindRenderbuffer=(PFNGLBINDRENDERBUFFEREXTPROC)glXGetProcAddress((GLubyte*)"glBindRenderbuffer");
        _glRenderbufferStorage=(PFNGLRENDERBUFFERSTORAGEEXTPROC)glXGetProcAddress((GLubyte*)"glRenderbufferStorage");
        _glFramebufferRenderbuffer=(PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC)glXGetProcAddress((GLubyte*)"glFramebufferRenderbuffer");
        _glCheckFramebufferStatus=(PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC)glXGetProcAddress((GLubyte*)"glCheckFramebufferStatus");
        _glGetRenderbufferParameteriv=(PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC)glXGetProcAddress((GLubyte*)"glGetRenderbufferParameteriv");

        DEPTH24_STENCIL8=GL_DEPTH24_STENCIL8;
        FRAMEBUFFER=GL_FRAMEBUFFER;
        RENDERBUFFER=GL_RENDERBUFFER;
        DEPTH_ATTACHMENT=GL_DEPTH_ATTACHMENT;
        COLOR_ATTACHMENT0=GL_COLOR_ATTACHMENT0;
        DEPTH_STENCIL_ATTACHMENT=GL_DEPTH_STENCIL_ATTACHMENT;
    }
#endif

#ifdef MAC_VREP
    if ((!_usingExt)&&(!forceFboToUseExt))
    {
        DEPTH24_STENCIL8=GL_DEPTH24_STENCIL8;
        FRAMEBUFFER=GL_FRAMEBUFFER;
        RENDERBUFFER=GL_RENDERBUFFER;
        DEPTH_ATTACHMENT=GL_DEPTH_ATTACHMENT;
        DEPTH_STENCIL_ATTACHMENT=GL_DEPTH_STENCIL_ATTACHMENT;
        COLOR_ATTACHMENT0=GL_COLOR_ATTACHMENT0;
    }
#endif

    if (forceFboToUseExt)
        _usingExt=true;
}

void oglExt::turnOffVSync(int vsync)
{
#ifdef WIN_VREP
    typedef BOOL (WINAPI *PFNWGLSWAPINTERVALEXTPROC)(int interval);
    PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT=(PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
    if (wglSwapIntervalEXT)
        wglSwapIntervalEXT(vsync);
#endif

#ifdef MAC_VREP
    int swap_interval=vsync;
    CGLContextObj cgl_context=CGLGetCurrentContext();
    CGLSetParameter(cgl_context,kCGLCPSwapInterval,&swap_interval);
#endif

#ifdef LIN_VREP
    typedef int (APIENTRY * AAAAGLSWAPINTERVALEXTPROC)(int);
    AAAAGLSWAPINTERVALEXTPROC wglSwapIntervalEXT=(AAAAGLSWAPINTERVALEXTPROC)glXGetProcAddress((const GLubyte*)"glXSwapIntervalSGI");
    if (wglSwapIntervalEXT)
        wglSwapIntervalEXT(vsync);
//Yes, WGL extensions are windows only, seems like they have put WGL_EXT_swap_control instead of GLX_SGI_swap_control into the extension string. With my nvidia card I get GLX_SGI_swap_control under linux and WGL_EXT_swap_control under windows.
//  With GLX (X11), uses extension GLX_SGI_video_sync
#endif
}


bool oglExt::isFboAvailable()
{
    bool dummyViaExt;
    return(_isFboAvailable(dummyViaExt));
}

bool oglExt::_isFboAvailable(bool& viaExt)
{ // prepareExtensionFunctions() should have been called previously!
#ifndef MAC_VREP
    const char* gl_version=(const char*)(glGetString(GL_VERSION));
//  printf("glVersion: %s\n",gl_version);
/*
    printf("glVersion: %s\n",gl_version);

    if (App::mainWindow->openglWidget->context()->isOpenGLES())
        printf("Is ES\n");
    else
        printf("Is NOT ES\n");
    //*/
/*
    QOpenGLFunctions* f=App::mainWindow->openglWidget->context()->functions();
    gl_version=(const char*)(f->glGetString(GL_VERSION));
    printf("glVersion2: %s\n",gl_version);
*/

    if (atof(gl_version)>=3.0)
    {
        viaExt=false;
        return(true);
    }
    else
    {
        const char* gl_extensions=(const char*)(glGetString(GL_EXTENSIONS));
        if (strstr(gl_extensions,"ARB_framebuffer_object")!=0)
            viaExt=false;
        else if (strstr(gl_extensions,"EXT_framebuffer_object")!=0)
            viaExt=true;
        else
        {
            viaExt=true;
            return(false);
        }
        return(true);
    }
    return(false);
#else
    const char* gl_version((const char*)glGetString(GL_VERSION));
    if (atof(gl_version)>=3.0)
    {
        viaExt=false;
        return(true);
    }
    else
    {
        const char* gl_extensions((const char*)glGetString(GL_EXTENSIONS));
        if ( (strstr(gl_extensions,"GL_ARB_framebuffer_object")!=0) || (strstr(gl_extensions,"ARB_framebuffer_object")!=0) )
            viaExt=false;
        else if ( (strstr(gl_extensions,"GL_EXT_framebuffer_object")!=0) || (strstr(gl_extensions,"EXT_framebuffer_object")!=0) )
            viaExt=true;
        else
        {
            viaExt=true;
            return(false);
        }
        return(true);
    }
    return(false);
//  std::string extString((const char*)glGetString(GL_EXTENSIONS));
//  return(extString.find("GL_EXT_framebuffer_object")!=std::string::npos);
#endif
}

bool oglExt::areNonPowerOfTwoTexturesAvailable()
{
    std::string extString((const char*)glGetString(GL_EXTENSIONS));
    return(extString.find("GL_ARB_texture_non_power_of_two")!=std::string::npos);
}

void oglExt::initDefaultGlValues()
{
    glClearDepth(1.0f);
    glDepthFunc(GL_LEQUAL); // Maybe useful with glPolygonOffset?
    glClearColor(0.0f,0.0f,0.0f,1.0f);
    glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
    glCullFace(GL_BACK);
    glDisable(GL_CULL_FACE);
    glBlendFunc(GL_ONE_MINUS_DST_COLOR,GL_ZERO);
    glLineStipple(1,3855);
    // The following is very important for the readPixels command and similar:
    // Default byte alignement in openGL is 4, but we want it to be 1! Make sure to keep GL_UNPACK_ALIGNMENT to 4 (default). Really? Why?
    glPixelStorei(GL_PACK_ALIGNMENT,1);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_DITHER);
    glEnable(GL_LIGHTING); // keep lighting on for everything, except for temporary operations.
    glShadeModel(GL_SMOOTH);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,1); // Important in order to have both sides affected by lights!
}
