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

#include <ghostRendering.h>

#ifdef SIM_WITH_OPENGL

void displayGhost(CShape* shape,const C7Vector& tr,int displayAttributes,int options,double transparencyFactor,const float* color)
{
    glPushMatrix();
    glPushAttrib(GL_POLYGON_BIT);

    glTranslated(tr.X(0),tr.X(1),tr.X(2));
    C4Vector axis=tr.Q.getAngleAndAxis();
    glRotated(axis(0)*radToDeg,axis(1),axis(2),axis(3));

    shape->getMeshWrapper()->displayGhost(shape,displayAttributes,(options&4)!=0,(options&32)!=0,transparencyFactor,color);

    glPopAttrib();
    glPopMatrix();
    ogl::setBlending(false);
    glDisable(GL_CULL_FACE);
}

#else

void displayGhost(CShape* shape,const C7Vector& tr,int displayAttributes,int options,double transparencyFactor,const float* color)
{

}

#endif



