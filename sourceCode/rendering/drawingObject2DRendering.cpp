
#include "drawingObject2DRendering.h"

#ifdef SIM_WITH_OPENGL

void displayDrawingObject2D(CDrawingObject2D* obj,const int minViewPos[2],const int maxViewPos[2])
{
    float dx=float(maxViewPos[0]-minViewPos[0]);
    float dy=float(maxViewPos[1]-minViewPos[1]);
    ogl::setMaterialColor(ogl::colorBlack,ogl::colorBlack,ogl::colorBlack);
    int t=obj->_objectType&255;
    float hs=(dx+dy)*0.25f*obj->_size;
    if (obj->_objectType&sim_drawing2d_pixelsize)
        hs=obj->_size/2.0f;
    if ( (t==sim_drawing2d_square)||(t==sim_drawing2d_circle) )
    {
        int startI=1;
        if (obj->_objectType&sim_drawing2d_blackbkground)
            startI=0;
        glLineWidth(3.0f);
        int ad=0;
        for (int n=startI;n<2;n++)
        {
            if (n==1)
            {
                glLineWidth(1.0f);
                ogl::setMaterialColor(sim_colorcomponent_emission,obj->_col);
                ad=0;
            }
            else
                ad=1;
            if (t==sim_drawing2d_square)
            {
                glBegin(GL_QUADS);
                glVertex3i(minViewPos[0]+int(dx*obj->_vertices[0]-hs)-ad,minViewPos[1]+int(dy*obj->_vertices[1]-hs)-ad,0);
                glVertex3i(minViewPos[0]+int(dx*obj->_vertices[0]+hs)+ad,minViewPos[1]+int(dy*obj->_vertices[1]-hs)-ad,0);
                glVertex3i(minViewPos[0]+int(dx*obj->_vertices[0]+hs)+ad,minViewPos[1]+int(dy*obj->_vertices[1]+hs)+ad,0);
                glVertex3i(minViewPos[0]+int(dx*obj->_vertices[0]-hs)-ad,minViewPos[1]+int(dy*obj->_vertices[1]+hs)+ad,0);
                glEnd();
            }

            if (t==sim_drawing2d_circle)
            {
                glBegin(GL_LINE_STRIP);
                const float da=piValTimes2_f/16.0f;
                for (int i=0;i<17;i++)
                    glVertex3i(minViewPos[0]+int(dx*obj->_vertices[0]+hs*cos(da*float(i))+0.5f),minViewPos[1]+int(dy*obj->_vertices[1]+hs*sin(da*float(i))+0.5f),0);
                glEnd();
            }
        }
        glLineWidth(1.0f);
    }
    if (t==sim_drawing2d_text)
    {
        int startI=1;
        if (obj->_objectType&sim_drawing2d_blackbkground)
            startI=0;
        int ad=0;
        for (int n=startI;n<2;n++)
        {
            if (n==1)
            {
                ogl::setTextColor(obj->_col);
                ad=0;
            }
            else
            {
                ogl::setTextColor(ogl::colorBlack);
                ad=1;
            }
            ogl::drawTexti(minViewPos[0]+int(dx*obj->_vertices[0])-ad,minViewPos[1]+int(dy*obj->_vertices[1])-ad,0,obj->_txt);
        }
    }
    if (t==sim_drawing2d_line)
    {
        int startI=1;
        if (obj->_objectType&sim_drawing2d_blackbkground)
            startI=0;
        glLineWidth(3.0f);
        for (int n=startI;n<2;n++)
        {
            if (n==1)
            {
                glLineWidth(1.0f);
                ogl::setMaterialColor(sim_colorcomponent_emission,obj->_col);
            }
            glBegin(GL_LINES);
            glVertex3i(minViewPos[0]+int(dx*obj->_vertices[0]),minViewPos[1]+int(dy*obj->_vertices[1]),0);
            glVertex3i(minViewPos[0]+int(dx*obj->_vertices[2]),minViewPos[1]+int(dy*obj->_vertices[3]),0);
            glEnd();
        }
        glLineWidth(1.0f);
    }
}

#else

void displayDrawingObject2D(CDrawingObject2D* obj,const int minViewPos[2],const int maxViewPos[2])
{

}

#endif



