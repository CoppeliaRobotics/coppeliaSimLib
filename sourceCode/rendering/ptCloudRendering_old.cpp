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

#include "ptCloudRendering_old.h"

#ifdef SIM_WITH_OPENGL
void displayPtCloud_old(CPtCloud_old* ptCloud,CSceneObject* it)
{
    glPushMatrix();

    C3Vector normalVectorForLinesAndPoints=C3Vector::unitZVector;

    if (it!=nullptr)
    {
        C7Vector tr=it->getCumulativeTransformation();
        glTranslated(tr.X(0),tr.X(1),tr.X(2));
        C4Vector axis=tr.Q.getAngleAndAxis();
        glRotated(axis(0)*radToDeg,axis(1),axis(2),axis(3));
        normalVectorForLinesAndPoints=(tr.Q.getInverse()*C3Vector::unitZVector);
    }

    glPointSize((float)ptCloud->_pointSize);

    if (ptCloud->_normals.size()==0)
        ogl::setMaterialColor(ptCloud->_defaultColors,ogl::colorBlack,ptCloud->_defaultColors+12);
    else
        ogl::setMaterialColor(ptCloud->_defaultColors,ptCloud->_defaultColors+8,ptCloud->_defaultColors+12);

    if (ptCloud->_colors.size()==0)
    {
        if (ptCloud->_normals.size()==0)
            ogl::drawRandom3dPointsEx(&ptCloud->_vertices[0],(int)ptCloud->_vertices.size()/3,nullptr,nullptr,nullptr,false,normalVectorForLinesAndPoints.data,3);
        else
            ogl::drawRandom3dPointsEx(&ptCloud->_vertices[0],(int)ptCloud->_vertices.size()/3,&ptCloud->_normals[0],nullptr,nullptr,false,normalVectorForLinesAndPoints.data,3);
    }
    else
    {
        if (ptCloud->_normals.size()==0)
            ogl::drawRandom3dPointsEx(&ptCloud->_vertices[0],(int)ptCloud->_vertices.size()/3,nullptr,&ptCloud->_colors[0],nullptr,(ptCloud->_options&4)!=0,normalVectorForLinesAndPoints.data,3);
        else
            ogl::drawRandom3dPointsEx(&ptCloud->_vertices[0],(int)ptCloud->_vertices.size()/3,&ptCloud->_normals[0],&ptCloud->_colors[0],nullptr,(ptCloud->_options&4)!=0,normalVectorForLinesAndPoints.data,3);
    }
    glPointSize(1.0);

    glPopMatrix();
}

#else

void displayPtCloud_old(CPtCloud_old* ptCloud,CSceneObject* it)
{

}

#endif



