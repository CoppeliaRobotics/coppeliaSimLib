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

#include <distanceRendering.h>
#include <tt.h>
#include <gV.h>

#ifdef SIM_WITH_OPENGL

void displayDistance(CDistanceObject_old* dist,int segWidth,const double* distResult)
{
    dist->getSegmentColor()->makeCurrentColor(false);
    glLineWidth(float(segWidth));
    ogl::drawSingle3dLine(distResult+0,distResult+3,nullptr);
    glLineWidth(1.0);
    std::string txt("d=");
    txt+=tt::FNb(0,distResult[6],6,false)+" ";
    txt+=gv::getSizeUnitStr();
    ogl::setTextColor(dist->getSegmentColor()->getColorsPtr());
    ogl::drawBitmapTextIntoScene((distResult[0]+distResult[3])/2.0,(distResult[1]+distResult[4])/2.0,(distResult[2]+distResult[5])/2.0,txt.c_str());
}

#else

void displayDistance(CDistanceObject_old* dist,int segWidth,const double* distResult)
{

}

#endif



