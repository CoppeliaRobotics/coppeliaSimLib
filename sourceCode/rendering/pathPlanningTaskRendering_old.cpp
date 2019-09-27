
#include "pathPlanningTaskRendering_old.h"

#ifdef SIM_WITH_OPENGL

void displayPathPlanningTask(const C3Vector* corners)
{
    ogl::setMaterialColor(sim_colorcomponent_emission,ogl::colorCyan);
    ogl::buffer.clear();
    ogl::addBuffer3DPoints(corners[0].data);
    ogl::addBuffer3DPoints(corners[2].data);
    ogl::addBuffer3DPoints(corners[6].data);
    ogl::addBuffer3DPoints(corners[4].data);
    ogl::addBuffer3DPoints(corners[0].data);
    ogl::addBuffer3DPoints(corners[1].data);
    ogl::addBuffer3DPoints(corners[3].data);
    ogl::addBuffer3DPoints(corners[7].data);
    ogl::addBuffer3DPoints(corners[5].data);
    ogl::addBuffer3DPoints(corners[1].data);
    ogl::drawRandom3dLines(&ogl::buffer[0],(int)ogl::buffer.size()/3,true,nullptr);
    ogl::buffer.clear();
    ogl::addBuffer3DPoints(corners[4].data);
    ogl::addBuffer3DPoints(corners[5].data);
    ogl::addBuffer3DPoints(corners[6].data);
    ogl::addBuffer3DPoints(corners[7].data);
    ogl::addBuffer3DPoints(corners[2].data);
    ogl::addBuffer3DPoints(corners[3].data);
    ogl::drawRandom3dLines(&ogl::buffer[0],(int)ogl::buffer.size()/3,false,nullptr);
    ogl::buffer.clear();
}

#else

void displayPathPlanningTask(const C3Vector* corners)
{

}

#endif



