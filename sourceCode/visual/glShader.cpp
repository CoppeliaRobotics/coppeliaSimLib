
#include "oGL.h"
#include "glShader.h"

CGlShader::CGlShader()
{
    _shaderProgram.addShaderFromSourceFile(QGLShader::Vertex,"vertexShaderTest.txt");
    _shaderProgram.addShaderFromSourceFile(QGLShader::Fragment,"fragmentShaderTest.txt");
    /*
    _shaderProgram.addShaderFromSourceCode(QGLShader::Vertex,
                                           "varying vec4 diffuse,ambient;\n"
                                           "varying vec3 normal,halfVector;\n"
                                           " \n"
                                           "void main()\n"
                                           "{\n"
                                           "    \n"
                                           "    normal = normalize(gl_NormalMatrix * gl_Normal);\n"
                                           " \n"
                                           "    \n"
                                           "    halfVector = gl_LightSource[0].halfVector.xyz;\n"
                                           " \n"
                                           "    \n"
                                           "    diffuse = gl_FrontMaterial.diffuse * gl_LightSource[0].diffuse;\n"
                                           "    ambient = gl_FrontMaterial.ambient * gl_LightSource[0].ambient;\n"
                                           "    ambient += gl_LightModel.ambient * gl_FrontMaterial.ambient;\n"
                                           "    gl_Position = ftransform();\n"
                                           " \n"
                                           "}");
    _shaderProgram.addShaderFromSourceCode(QGLShader::Fragment,
                                           "varying vec4 diffuse,ambient;\n"
                                           "varying vec3 normal,halfVector;\n"
                                           " \n"
                                           "void main()\n"
                                           "{\n"
                                           "    vec3 n,halfV,lightDir;\n"
                                           "    double NdotL,NdotHV;\n"
                                           " \n"
                                           "    lightDir = vec3(gl_LightSource[0].position);\n"
                                           " \n"
                                           "    \n"
                                           "    vec4 color = ambient;\n"
                                           "    \n"
                                           "    n = normalize(normal);\n"
                                           "    \n"
                                           " \n"
                                           "    NdotL = max(dot(n,lightDir),0.0);\n"
                                            "      if (NdotL > 0.0) {\n"
                                            "       color += diffuse * NdotL;\n"
                                            "       halfV = normalize(halfVector);\n"
                                            "       NdotHV = max(dot(n,halfV),0.0);\n"
                                            "       color += gl_FrontMaterial.specular *\n"
                                            "               gl_LightSource[0].specular *\n"
                                            "               pow(NdotHV, gl_FrontMaterial.shininess);\n"
                                            "   }\n"
                                            "\n"
                                            "   gl_FragColor = color;\n"
                                               "}");
*/

                                               /*
    _shaderProgram.addShaderFromSourceCode(QGLShader::Vertex,
        "attribute vec4 in_vertex;\n"
        "uniform mat4 projection;\n"
        "uniform mat4 view;\n"
        "uniform mat4 model;\n"
        "void main()\n"
        "{\n"
        "   gl_Position = projection * view * model * in_vertex;\n"
        "}");
    _shaderProgram.addShaderFromSourceCode(QGLShader::Fragment,
        "void main()\n"
        "{\n"
        "   gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
        "}");
    */
    if (_shaderProgram.link())
    {
        printf("Link successful.\n");
    }
    else
    {
        printf("Link failed: %s\n",_shaderProgram.log().toStdString().c_str());
    }
}

CGlShader::~CGlShader()
{
    _shaderProgram.release();
}

bool CGlShader::bind()
{
    bool result=_shaderProgram.bind();
    if (result)
        printf("Bind successful.\n");
    else
        printf("Bind failed.\n");
    return(result);
}
