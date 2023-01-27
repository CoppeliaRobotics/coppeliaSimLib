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

#include "bannerRendering.h"

#ifdef SIM_WITH_OPENGL

void displayBanner(int objId,int options,const float* bckColor,const C7Vector& tr,const char* label,const CColorObject& color,double height,const C4X4Matrix& cameraCTM,const int windowSize[2],double verticalViewSizeOrAngle,bool perspective)
{
    if (options&(sim_banner_clickselectsparent+sim_banner_clicktriggersevent))
        glLoadName(objId+NON_OBJECT_PICKING_ID_BANNER_START); // But bitmap fonts cannot be picked!
    else
        glLoadName(-1);

    if (options&sim_banner_bitmapfont)
    {
        if (options&sim_banner_overlay)
        {
            glDisable(GL_DEPTH_TEST);
            glDepthRange(0.0,0.0);
        }
        ogl::setMaterialColor(ogl::colorBlack,ogl::colorBlack,ogl::colorBlack);

        if (bckColor!=nullptr)
        {
            ogl::setMaterialColor(sim_colorcomponent_emission,std::max<float>(bckColor[0],bckColor[9]),std::max<float>(bckColor[1],bckColor[10]),std::max<float>(bckColor[2],bckColor[11]));
            ogl::drawBitmapTextBackgroundIntoScene(tr.X(0),tr.X(1),tr.X(2),label);
        }

        ogl::setTextColor(std::max<double>(color.getColorsPtr()[0],color.getColorsPtr()[9]),std::max<double>(color.getColorsPtr()[1],color.getColorsPtr()[10]),std::max<double>(color.getColorsPtr()[2],color.getColorsPtr()[11]));
        ogl::drawBitmapTextIntoScene(tr.X(0),tr.X(1),tr.X(2),label);

        glDepthRange(0.0,1.0);
        glEnable(GL_DEPTH_TEST);
    }
    else
    {
        if (options&(sim_banner_fullyfacingcamera+sim_banner_facingcamera))
            ogl::drawOutlineText(label,tr,height,&cameraCTM,(options&sim_banner_overlay)!=0,(options&sim_banner_left)!=0,(options&sim_banner_right)!=0,(options&sim_banner_backfaceculling)!=0,color.getColorsPtr(),bckColor,(options&sim_banner_keepsamesize)!=0,windowSize[1],verticalViewSizeOrAngle,perspective,(options&sim_banner_fullyfacingcamera)!=0);
        else
            ogl::drawOutlineText(label,tr,height,nullptr,(options&sim_banner_overlay)!=0,(options&sim_banner_left)!=0,(options&sim_banner_right)!=0,(options&sim_banner_backfaceculling)!=0,color.getColorsPtr(),bckColor,(options&sim_banner_keepsamesize)!=0,windowSize[1],verticalViewSizeOrAngle,perspective,false);
    }

    glLoadName(-1);
}

#else

void displayBanner(int objId,int options,const float* bckColor,const C7Vector& tr,const char* label,const CColorObject& color,double height,const C4X4Matrix& cameraCTM,const int windowSize[2],double verticalViewSizeOrAngle,bool perspective)
{

}

#endif



