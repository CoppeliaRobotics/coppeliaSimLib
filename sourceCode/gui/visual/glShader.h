
#pragma once
#include <QGLShaderProgram>

class CGlShader
{
public:

    CGlShader();
    virtual ~CGlShader();

    bool bind();

protected:
    QGLShaderProgram _shaderProgram;
};
