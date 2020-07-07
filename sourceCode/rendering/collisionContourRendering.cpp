#include "collisionContourRendering.h"

#ifdef SIM_WITH_OPENGL

void displayContour(CCollisionObject* coll,int countourWidth)
{
    const std::vector<float>* intersections=coll->getIntersections();

    glDisable(GL_DEPTH_TEST);
    coll->getContourColor()->makeCurrentColor(false);
    for (size_t i=0;i<intersections->size()/6;i++)
    {
        if ( (intersections->at(6*i+0)==intersections->at(6*i+3))&&
            (intersections->at(6*i+1)==intersections->at(6*i+4))&&
            (intersections->at(6*i+2)==intersections->at(6*i+5)) )
        {
            glPointSize(3.0f);
            ogl::buffer.clear();
            ogl::addBuffer3DPoints(&intersections->at(6*i));
            ogl::drawRandom3dPoints(&ogl::buffer[0],1,nullptr);
            ogl::buffer.clear();
            glPointSize(1.0f);
        }
        else
        {
            glLineWidth(float(countourWidth));
            ogl::buffer.clear();
            ogl::addBuffer3DPoints(&intersections->at(6*i+0));
            ogl::addBuffer3DPoints(&intersections->at(6*i+3));
            ogl::drawRandom3dLines(&ogl::buffer[0],2,false,nullptr);
            ogl::buffer.clear();
            glLineWidth(1.0f);
        }
    }
    glEnable(GL_DEPTH_TEST);
}

#else

void displayContour(CCollisionObject* coll,int countourWidth)
{

}

#endif



