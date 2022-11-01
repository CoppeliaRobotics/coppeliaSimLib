#include "ghostRendering.h"

#ifdef SIM_WITH_OPENGL

void displayGhost(CShape* shape,const C7Vector& tr,int displayAttributes,int options,float transparencyFactor,const float* color)
{
    glPushMatrix();
    glPushAttrib(GL_POLYGON_BIT);

    glTranslatef(tr.X(0),tr.X(1),tr.X(2));
    C4Vector axis=tr.Q.getAngleAndAxis();
    glRotatef(axis(0)*radToDeg_f,axis(1),axis(2),axis(3));

    shape->getMeshWrapper()->displayGhost(shape,displayAttributes,(options&4)!=0,(options&32)!=0,transparencyFactor,color);

    glPopAttrib();
    glPopMatrix();
    ogl::setBlending(false);
    glDisable(GL_CULL_FACE);
}

#else

void displayGhost(CShape* shape,const C7Vector& tr,int displayAttributes,int options,float transparencyFactor,const float* color)
{

}

#endif



