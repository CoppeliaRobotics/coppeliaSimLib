
#include "bannerRendering.h"

#ifdef SIM_WITH_OPENGL

void displayBanner(int objId,int options,const float* bckColor,const C7Vector& tr,const char* label,const CVisualParam& color,float height,const C4X4Matrix& cameraCTM,const int windowSize[2],float verticalViewSizeOrAngle,bool perspective)
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
            glDepthRange(0.0f,0.0f);
        }
        ogl::setMaterialColor(ogl::colorBlack,ogl::colorBlack,ogl::colorBlack);

        if (bckColor!=nullptr)
        {
            ogl::setMaterialColor(sim_colorcomponent_emission,SIM_MAX(bckColor[0],bckColor[9]),SIM_MAX(bckColor[1],bckColor[10]),SIM_MAX(bckColor[2],bckColor[11]));
            ogl::drawBitmapTextBackgroundIntoScene(tr.X(0),tr.X(1),tr.X(2),label);
        }

        ogl::setTextColor(SIM_MAX(color.colors[0],color.colors[9]),SIM_MAX(color.colors[1],color.colors[10]),SIM_MAX(color.colors[2],color.colors[11]));
        ogl::drawBitmapTextIntoScene(tr.X(0),tr.X(1),tr.X(2),label);

        glDepthRange(0.0f,1.0f);
        glEnable(GL_DEPTH_TEST);
    }
    else
    {
        if (options&(sim_banner_fullyfacingcamera+sim_banner_facingcamera))
            ogl::drawOutlineText(label,tr,height,&cameraCTM,(options&sim_banner_overlay)!=0,(options&sim_banner_left)!=0,(options&sim_banner_right)!=0,(options&sim_banner_backfaceculling)!=0,color.colors,bckColor,(options&sim_banner_keepsamesize)!=0,windowSize[1],verticalViewSizeOrAngle,perspective,(options&sim_banner_fullyfacingcamera)!=0);
        else
            ogl::drawOutlineText(label,tr,height,nullptr,(options&sim_banner_overlay)!=0,(options&sim_banner_left)!=0,(options&sim_banner_right)!=0,(options&sim_banner_backfaceculling)!=0,color.colors,bckColor,(options&sim_banner_keepsamesize)!=0,windowSize[1],verticalViewSizeOrAngle,perspective,false);
    }

    glLoadName(-1);
}

#else

void displayBanner(int objId,int options,const float* bckColor,const C7Vector& tr,const char* label,const CVisualParam& color,float height,const C4X4Matrix& cameraCTM,const int windowSize[2],float verticalViewSizeOrAngle,bool perspective)
{

}

#endif



