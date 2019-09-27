
#include "collisionContourRendering.h"

#ifdef SIM_WITH_OPENGL

void displayContour(CRegCollision* coll,int countourWidth)
{
    std::vector<float>& intersections=coll->getIntersectionsPtr()[0];

    glDisable(GL_DEPTH_TEST);
    coll->contourColor.makeCurrentColor(false);
    for (size_t i=0;i<intersections.size()/6;i++)
    {
        if ( (intersections[6*i+0]==intersections[6*i+3])&&
            (intersections[6*i+1]==intersections[6*i+4])&&
            (intersections[6*i+2]==intersections[6*i+5]) )
        {
            glPointSize(3.0f);
            ogl::buffer.clear();
            ogl::addBuffer3DPoints(&intersections[6*i]);
            ogl::drawRandom3dPoints(&ogl::buffer[0],1,nullptr);
            ogl::buffer.clear();
            glPointSize(1.0f);
        }
        else
        {
            glLineWidth(float(countourWidth));
            ogl::buffer.clear();
            ogl::addBuffer3DPoints(&intersections[6*i+0]);
            ogl::addBuffer3DPoints(&intersections[6*i+3]);
            ogl::drawRandom3dLines(&ogl::buffer[0],2,false,nullptr);
            ogl::buffer.clear();
            glLineWidth(1.0f);
        }
    }
    glEnable(GL_DEPTH_TEST);
}

#else

void displayContour(CRegCollision* coll,int countourWidth)
{

}

#endif



