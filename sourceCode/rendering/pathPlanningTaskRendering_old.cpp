/*
Source code based on the V-REP library source code from Coppelia
Robotics AG on September 2019

Copyright (C) 2006-2023 Coppelia Robotics AG
Copyright (C) 2019 Robot Nordic ApS

All rights reserved.

GNU GPL license:
================
The code in this file is free software: you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

See the GNU General Public License for more details.
<http://www.gnu.org/licenses/>
*/

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



