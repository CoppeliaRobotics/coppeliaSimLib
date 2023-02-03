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

#include <collisionContourRendering.h>

#ifdef SIM_WITH_OPENGL

void displayContour(CCollisionObject_old* coll,int countourWidth)
{
    const std::vector<double>* intersections=coll->getIntersections();

    glDisable(GL_DEPTH_TEST);
    coll->getContourColor()->makeCurrentColor(false);
    for (size_t i=0;i<intersections->size()/6;i++)
    {
        if ( (intersections->at(6*i+0)==intersections->at(6*i+3))&&
            (intersections->at(6*i+1)==intersections->at(6*i+4))&&
            (intersections->at(6*i+2)==intersections->at(6*i+5)) )
        {
            glPointSize(3.0);
            ogl::buffer.clear();
            ogl::addBuffer3DPoints(&intersections->at(6*i));
            ogl::drawRandom3dPoints(&ogl::buffer[0],1,nullptr);
            ogl::buffer.clear();
            glPointSize(1.0);
        }
        else
        {
            glLineWidth(float(countourWidth));
            ogl::buffer.clear();
            ogl::addBuffer3DPoints(&intersections->at(6*i+0));
            ogl::addBuffer3DPoints(&intersections->at(6*i+3));
            ogl::drawRandom3dLines(&ogl::buffer[0],2,false,nullptr);
            ogl::buffer.clear();
            glLineWidth(1.0);
        }
    }
    glEnable(GL_DEPTH_TEST);
}

#else

void displayContour(CCollisionObject_old* coll,int countourWidth)
{

}

#endif



