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

#include <environmentRendering.h>

#ifdef SIM_WITH_OPENGL

void displayBackground(const int* viewSize,bool fogEnabled,const float* fogBackgroundColor,const float* backGroundColorDown,const float* backGroundColor)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0,viewSize[0],0,viewSize[1],-100,100);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    ogl::setMaterialColor(ogl::colorBlack,ogl::colorBlack,ogl::colorBlack);
    glDisable(GL_DEPTH_TEST);
    glColorMaterial(GL_FRONT_AND_BACK,GL_EMISSION);
    glEnable(GL_COLOR_MATERIAL);
    glBegin(GL_QUADS);
    if (fogEnabled)
        glColor3f(fogBackgroundColor[0],fogBackgroundColor[1],fogBackgroundColor[2]);
    else
        glColor3f(backGroundColorDown[0],backGroundColorDown[1],backGroundColorDown[2]);
    glVertex3i(0,0,0);
    glVertex3i(viewSize[0],0,0);
    if (fogEnabled)
        glColor3f(fogBackgroundColor[0],fogBackgroundColor[1],fogBackgroundColor[2]);
    else
        glColor3f(backGroundColor[0],backGroundColor[1],backGroundColor[2]);
    glVertex3i(viewSize[0],viewSize[1],0);
    glVertex3i(0,viewSize[1],0);
    glEnd();
    glDisable(GL_COLOR_MATERIAL);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_DEPTH_BUFFER_BIT);
}

void enableAmbientLight(bool on,const float* ambientLightColor)
{
    if (on)
    {
        float ambient[]={ambientLightColor[0],ambientLightColor[1],ambientLightColor[2],1.0};
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT,ambient);
    }
    else
    {
        float ambient[]={0.0,0.0,0.0,1.0};
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT,ambient);
    }
}

void activateFog(const float* fogBackgroundColor,int fogType,double dd,double farClipp,double fogStart,double fogEnd,double fogDensity)
{
    float fog_color[4]={fogBackgroundColor[0],fogBackgroundColor[1],fogBackgroundColor[2],1.0};
    GLenum fogTypeEnum[3]={GL_LINEAR,GL_EXP,GL_EXP2};
    glFogfv(GL_FOG_COLOR,fog_color);
    glFogi(GL_FOG_MODE,fogTypeEnum[fogType]);
    glFogf(GL_FOG_START,float(fogStart*dd+(1.0-dd)*farClipp));
    glFogf(GL_FOG_END,float(fogEnd*dd+(1.0-dd)*farClipp));
    glFogf(GL_FOG_DENSITY,float(fogDensity*dd));
    glEnable(GL_FOG);
}

void enableFog(bool on)
{
    if (on)
        glEnable(GL_FOG);
    else
        glDisable(GL_FOG);
}

#else

void displayBackground(const int* viewSize,bool fogEnabled,const float* fogBackgroundColor,const float* backGroundColorDown,const float* backGroundColor)
{

}

void enableAmbientLight(bool on,const float* ambientLightColor)
{

}

void activateFog(const float* fogBackgroundColor,int fogType,double dd,double farClipp,double fogStart,double fogEnd,double fogDensity)
{

}

void enableFog(bool on)
{

}

#endif



