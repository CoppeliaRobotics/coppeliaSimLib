
// This file includes many many very ugly things and requires some serious refactoring!!!!

#include "vrepMainHeader.h"
#include "oGL.h"
#include "global.h"
#include "v_rep_internal.h"
#include "4X4Matrix.h"
#include "app.h"
#include "rendering.h"

SOglFont ogl::oglFonts[]= 
    {
        // For future: "Arial" size 12 is good (for UIs)
        //             "Arial" size 16 and size 32 are good too
        {"MS Sans Serif",16,0,{}},
        {"courier new",16,0,{}},
        {"",0,0,{}}
    };

bool ogl::colorsLocked=false;

int ogl::fontIndex=0;
char ogl::opacBitmap[10000];

unsigned int ogl::outlineFontBase;
float ogl::outlineFontSizes[256];
float ogl::outlineFontHeight;
float ogl::outlineFontCenter;

float ogl::currentRichTextColor[3];
float ogl::defaultRichTextColor[3];

float ogl::HIERARCHY_AND_BROWSER_LAST_SELECTION_COLOR[3]={1.0f,0.82f,0.38f};

float ogl::HIERARCHY_NOT_LAST_SELECTION_COLOR_BRIGHT[3]={1.0f,0.89f,0.54f};
float ogl::HIERARCHY_NOT_LAST_SELECTION_COLOR_DARK[3]={1.0f,0.89f,0.54f};

float ogl::HIERARCHY_COLLECTION_SELECTION_COLOR_BRIGHT[3]={0.8f,0.56f,0.71f};
float ogl::HIERARCHY_COLLECTION_SELECTION_COLOR_DARK[3]={0.8f,0.56f,0.71f};

float ogl::HIERARCHY_DROP_LOCATION_COLOR[3]={1.0f,0.7f,0.0f};
float ogl::HIERARCHY_WORLD_CLICK_COLOR[3]={1.0f,0.0f,0.0f};

float ogl::HIERARCHY_UNACTIVE_WORLD_COLOR[3]={0.9f,0.9f,0.9f};

float ogl::HIERARCHY_AND_BROWSER_NO_SELECTION_COLOR_BRIGHT[3]={0.975f,0.975f,0.975f};
float ogl::HIERARCHY_AND_BROWSER_NO_SELECTION_COLOR_DARK[3]={0.975f,0.975f,0.975f};

float ogl::HIERARCHY_NO_SELECTION_RED_BRIGHT[3]={1.0f,0.855f,0.855f};
float ogl::HIERARCHY_NO_SELECTION_RED_DARK[3]={1.0f,0.855f,0.855f};

float ogl::HIERARCHY_NO_SELECTION_GREEN_BRIGHT[3]={0.8f,1.0f,0.8f};
float ogl::HIERARCHY_NO_SELECTION_GREEN_DARK[3]={0.8f,1.0f,0.8f};

float ogl::HIERARCHY_NO_SELECTION_BLUE_BRIGHT[3]={0.855f,0.855f,1.0f};
float ogl::HIERARCHY_NO_SELECTION_BLUE_DARK[3]={0.855f,0.855f,1.0f};

float ogl::HIERARCHY_AND_BROWSER_TEXT_COLOR_VISIBLE[3]={0.0f,0.0f,0.0f};
float ogl::HIERARCHY_AND_BROWSER_TEXT_COLOR_INVISIBLE[3]={0.65f,0.65f,0.65f};

float ogl::HIERARCHY_AND_BROWSER_LINE_COLOR[3]={0.75f,0.75f,0.75f};

float ogl::HIERARCHY_AND_BROWSER_SCROLLBAR_COLOR[3]={0.9f,0.9f,0.9f};
float ogl::HIERARCHY_AND_BROWSER_SCROLLBAR_BACK_COLOR[3]={0.975f,0.975f,0.975f};

float ogl::TITLE_BAR_COLOR[3]={0.94f,0.94f,0.94f};
float ogl::TITLE_BAR_BUTTON_COLOR[3]={0.94f,0.94f,0.94f};
float ogl::SEPARATION_LINE_COLOR[3]={0.75f,0.75f,0.75f};

float ogl::MANIPULATION_MODE_OVERLAY_COLOR[4]={0.0f,0.85f,0.75f,0.25f};
float ogl::MANIPULATION_MODE_OVERLAY_GRID_COLOR[4]={0.3f,0.3f,0.3f,0.25f};

float ogl::colorBlack[3]={0.0f,0.0f,0.0f};
float ogl::colorDarkGrey[3]={0.2f,0.2f,0.2f};
float ogl::colorGrey[3]={0.5f,0.5f,0.5f};
float ogl::colorLightGrey[3]={0.8f,0.8f,0.8f};
float ogl::colorWhite[3]={1.0f,1.0f,1.0f};

float ogl::colorRed[3]={1.0f,0.0f,0.0f};
float ogl::colorGreen[3]={0.0f,1.0f,0.0f};
float ogl::colorBlue[3]={0.0f,0.0f,1.0f};
float ogl::colorYellow[3]={1.0f,1.0f,0.0f};
float ogl::colorCyan[3]={0.0f,1.0f,1.0f};
float ogl::colorPurple[3]={1.0f,0.0f,1.0f};

float ogl::_lastAmbientDiffuseAlpha[4];


std::vector<float> ogl::buffer;
std::vector<int> ogl::buffer_i;

void ogl::setTextColor(float r,float g,float b)
{
    float buff[4]={r,g,b,1.0f};
    glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,buff);
//      if (App::userSettings->specificGpuTweak)
    { // Following needed with some graphic card to see text when there is a background! This is probably a bug in the driver
        glBegin(GL_QUADS);
        glEnd();
        glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,buff);
    }
}

void ogl::setTextColor(float rgb[3])
{
    setTextColor(rgb[0],rgb[1],rgb[2]);
}

void ogl::setMaterialColor(const float ambientCol[3],const float specularCol[3],const float emissiveCol[3])
{
    float buff[4]={0.0f,0.0f,0.0f,1.0f};
    if (ambientCol!=nullptr)
    {
        _lastAmbientDiffuseAlpha[0]=ambientCol[0];
        _lastAmbientDiffuseAlpha[1]=ambientCol[1];
        _lastAmbientDiffuseAlpha[2]=ambientCol[2];
        glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,_lastAmbientDiffuseAlpha);
        if (App::userSettings->specificGpuTweak)
        { // Following needed with some graphic cards to see text when there is a background! This is probably a bug in the driver
            glBegin(GL_QUADS);
            glEnd();
            glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,_lastAmbientDiffuseAlpha);
        }
    }
    if (specularCol!=nullptr)
    {
        buff[0]=specularCol[0];
        buff[1]=specularCol[1];
        buff[2]=specularCol[2];
        glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,buff);
        if (App::userSettings->specificGpuTweak)
        { // Following needed with some graphic cards to see text when there is a background! This is probably a bug in the driver
            glBegin(GL_QUADS);
            glEnd();
            glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,buff);
        }
    }
    if (emissiveCol!=nullptr)
    {
        buff[0]=emissiveCol[0];
        buff[1]=emissiveCol[1];
        buff[2]=emissiveCol[2];
        glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,buff);
        if (App::userSettings->specificGpuTweak)
        { // Following needed with some graphic cards to see text when there is a background! This is probably a bug in the driver
            glBegin(GL_QUADS);
            glEnd();
            glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,buff);
        }
    }
}

void ogl::setMaterialColor(float amb_r,float amb_g,float amb_b,float spec_r,float spec_g,float spec_b,float em_r,float em_g,float em_b)
{
    setMaterialColor(sim_colorcomponent_ambient_diffuse,amb_r,amb_g,amb_b);
    setMaterialColor(sim_colorcomponent_specular,spec_r,spec_g,spec_b);
    setMaterialColor(sim_colorcomponent_emission,em_r,em_g,em_b);
}

void ogl::setMaterialColor(int colorMode,const float col[3])
{
    if (colorMode==sim_colorcomponent_ambient_diffuse)
        setMaterialColor(col,nullptr,nullptr);
    if (colorMode==sim_colorcomponent_specular)
        setMaterialColor(nullptr,col,nullptr);
    if (colorMode==sim_colorcomponent_emission)
        setMaterialColor(nullptr,nullptr,col);
}

void ogl::setMaterialColor(int colorMode,float r,float g,float b)
{
    if (colorMode==sim_colorcomponent_ambient_diffuse)
    {
        _lastAmbientDiffuseAlpha[0]=r;
        _lastAmbientDiffuseAlpha[1]=g;
        _lastAmbientDiffuseAlpha[2]=b;
        glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,_lastAmbientDiffuseAlpha);
        if (App::userSettings->specificGpuTweak)
        { // Following needed with some graphic cards to see text when there is a background! This is probably a bug in the driver
            glBegin(GL_QUADS);
            glEnd();
            glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,_lastAmbientDiffuseAlpha);
        }
    }
    if (colorMode==sim_colorcomponent_specular)
    {
        float buff[4]={r,g,b,1.0f};
        glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,buff);
        if (App::userSettings->specificGpuTweak)
        { // Following needed with some graphic cards to see text when there is a background! This is probably a bug in the driver
            glBegin(GL_QUADS);
            glEnd();
            glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,buff);
        }
    }
    if (colorMode==sim_colorcomponent_emission)
    {
        float buff[4]={r,g,b,1.0f};
        glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,buff);
        if (App::userSettings->specificGpuTweak)
        { // Following needed with some graphic card to see text when there is a background! This is probably a bug in the driver
            glBegin(GL_QUADS);
            glEnd();
            glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,buff);
        }
    }
}

void ogl::setBlendedMaterialColor(int colorMode,float rgb[3],float weight,float rgb2[3])
{   // Color rgb is merged with color rgb2 according to weight (0.0=no merging, 1.0=full merging)
    setMaterialColor(colorMode,rgb[0]+(rgb2[0]-rgb[0])*weight,rgb[1]+(rgb2[1]-rgb[1])*weight,rgb[2]+(rgb2[2]-rgb[2])*weight);
}

void ogl::setBlendedMaterialColor(int colorMode,float r,float g,float b,float weight,float r2,float g2,float b2)
{   // Color rgb is merged with color r2g2b2 according to weight (0.0=no merging, 1.0=full merging)
    float rgb[3]={r,g,b};
    float rgb2[3]={r2,g2,b2};
    setBlendedMaterialColor(colorMode,rgb,weight,rgb2);
}

void ogl::setBlending(bool on,int param1,int param2)
{   // param1 and param2 are 0 by default
    if (colorsLocked)
        return;
    if (on)
    {
        glEnable(GL_BLEND);
        glBlendFunc(param1,param2);
    }
    else
        glDisable(GL_BLEND);
}

void ogl::setAlpha(float a)
{
    _lastAmbientDiffuseAlpha[3]=a;
    glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,_lastAmbientDiffuseAlpha);
}

void ogl::setShininess(int shin)
{
    glMateriali(GL_FRONT_AND_BACK,GL_SHININESS,shin);
}

void ogl::disableLighting_useWithCare()
{ // use with care: we keep lighting enabled all the time, except for special operations
    glDisable(GL_LIGHTING);
}

void ogl::enableLighting_useWithCare()
{ // use with care: we keep lighting enabled all the time, except for special operations
    glEnable(GL_LIGHTING);
}

void ogl::drawRandom3dLines(const float* pts,int ptsCnt,bool connected,const float normalVectorForDiffuseComp[3])
{
    if (connected)
        glBegin(GL_LINE_STRIP);
    else
        glBegin(GL_LINES);
    if (normalVectorForDiffuseComp!=nullptr)
        glNormal3fv(normalVectorForDiffuseComp);
    else
        glNormal3f(0.0f,0.0f,1.0f);
    for (int i=0;i<ptsCnt;i++)
        glVertex3fv(pts+i*3);
    glEnd();
}

void ogl::drawRandom3dPoints(const float* pts,int ptsCnt,const float normalVectorForDiffuseComp[3])
{
    drawRandom3dPointsEx(pts,ptsCnt,nullptr,nullptr,nullptr,false,normalVectorForDiffuseComp);
}

void ogl::drawRandom3dPointsEx(const float* pts,int ptsCnt,const float* normals,const float* cols,const float* sizes,bool colsAreEmission,const float normalVectorForDiffuseComp[3])
{
    if (normals==nullptr)
    {
        if (cols==nullptr)
        {
            if (sizes==nullptr)
            {
                glBegin(GL_POINTS);
                if (normalVectorForDiffuseComp!=nullptr)
                    glNormal3fv(normalVectorForDiffuseComp);
                else
                    glNormal3f(0.0f,0.0f,1.0f);
                for (int i=0;i<ptsCnt;i++)
                    glVertex3fv(pts+i*3);
                glEnd();
            }
            else
            {
                if (normalVectorForDiffuseComp!=nullptr)
                    glNormal3fv(normalVectorForDiffuseComp);
                else
                    glNormal3f(0.0f,0.0f,1.0f);
                for (int i=0;i<ptsCnt;i++)
                {
                    glPointSize(sizes[i]); // cannot be called between glBegin and glEnd!
                    glBegin(GL_POINTS);
                    glVertex3fv(pts+i*3);
                    glEnd();
                }
            }
        }
        else
        {
            if (sizes==nullptr)
            {
                glBegin(GL_POINTS);
                if (normalVectorForDiffuseComp!=nullptr)
                    glNormal3fv(normalVectorForDiffuseComp);
                else
                    glNormal3f(0.0f,0.0f,1.0f);
                GLenum comp=GL_AMBIENT_AND_DIFFUSE;
                if (colsAreEmission)
                    comp=GL_EMISSION;
                for (int i=0;i<ptsCnt;i++)
                {
                    glMaterialfv(GL_FRONT_AND_BACK,comp,&cols[4*i]);
                    glVertex3fv(pts+i*3);
                }
                glEnd();
            }
            else
            {
                if (normalVectorForDiffuseComp!=nullptr)
                    glNormal3fv(normalVectorForDiffuseComp);
                else
                    glNormal3f(0.0f,0.0f,1.0f);
                GLenum comp=GL_AMBIENT_AND_DIFFUSE;
                if (colsAreEmission)
                    comp=GL_EMISSION;
                for (int i=0;i<ptsCnt;i++)
                {
                    glPointSize(sizes[i]); // cannot be called between glBegin and glEnd!
                    glBegin(GL_POINTS);
                    glMaterialfv(GL_FRONT_AND_BACK,comp,&cols[4*i]);
                    glVertex3fv(pts+i*3);
                    glEnd();
                }
            }
        }
    }
    else
    {
        if (cols==nullptr)
        {
            if (sizes==nullptr)
            {
                glBegin(GL_POINTS);
                for (int i=0;i<ptsCnt;i++)
                {
                    glNormal3fv(normals+i*3);
                    glVertex3fv(pts+i*3);
                }
                glEnd();
            }
            else
            {
                for (int i=0;i<ptsCnt;i++)
                {
                    glPointSize(sizes[i]); // cannot be called between glBegin and glEnd!
                    glBegin(GL_POINTS);
                    glNormal3fv(normals+i*3);
                    glVertex3fv(pts+i*3);
                    glEnd();
                }
            }
        }
        else
        {
            if (sizes==nullptr)
            {
                glBegin(GL_POINTS);
                GLenum comp=GL_AMBIENT_AND_DIFFUSE;
                if (colsAreEmission)
                    comp=GL_EMISSION;
                for (int i=0;i<ptsCnt;i++)
                {
                    glMaterialfv(GL_FRONT_AND_BACK,comp,&cols[4*i]);
                    glNormal3fv(normals+i*3);
                    glVertex3fv(pts+i*3);
                }
                glEnd();
            }
            else
            {
                GLenum comp=GL_AMBIENT_AND_DIFFUSE;
                if (colsAreEmission)
                    comp=GL_EMISSION;
                for (int i=0;i<ptsCnt;i++)
                {
                    glPointSize(sizes[i]); // cannot be called between glBegin and glEnd!
                    glBegin(GL_POINTS);
                    glMaterialfv(GL_FRONT_AND_BACK,comp,&cols[4*i]);
                    glNormal3fv(normals+i*3);
                    glVertex3fv(pts+i*3);
                    glEnd();
                }
            }
        }
    }
}

void ogl::drawRandom2dLines(const float* pts,int ptsCnt,bool connected,float zCoord)
{
    if (connected)
        glBegin(GL_LINE_STRIP);
    else
        glBegin(GL_LINES);
    for (int i=0;i<ptsCnt;i++)
        glVertex3f(pts[i*2+0],pts[i*2+1],zCoord);
    glEnd();
}

void ogl::drawRandom2dLines_i(const int* pts,int ptsCnt,bool connected,int zCoord)
{
    if (connected)
        glBegin(GL_LINE_STRIP);
    else
        glBegin(GL_LINES);
    for (int i=0;i<ptsCnt;i++)
        glVertex3i(pts[i*2+0],pts[i*2+1],zCoord);
    glEnd();
}

void ogl::drawRandom2dPoints(const float* pts,int ptsCnt,float zCoord)
{
    glBegin(GL_POINTS);
    for (int i=0;i<ptsCnt;i++)
        glVertex3f(pts[i*2+0],pts[i*2+1],zCoord);
    glEnd();
}

void ogl::drawBitmapTextTo3dPosition(const float pos[3],const std::string& txt,const float normalVectorForDiffuseComp[3])
{
    drawBitmapTextTo3dPosition(pos[0],pos[1],pos[2],txt,normalVectorForDiffuseComp);
}

void ogl::drawBitmapTextTo3dPosition(float x,float y,float z,const std::string& txt,const float normalVectorForDiffuseComp[3])
{
    if (oglFonts[fontIndex].fontBase==0)
        return;
    if (txt.length()==0)
        return;
    if (normalVectorForDiffuseComp!=nullptr)
        glNormal3fv(normalVectorForDiffuseComp);
    else
        glNormal3f(0.0f,0.0f,1.0f);
    glRasterPos3f(x,y,z);
    glPushAttrib(GL_LIST_BIT);
    glListBase(oglFonts[fontIndex].fontBase);
    glCallLists((GLsizei)txt.length(),GL_UNSIGNED_BYTE,txt.c_str());
    glPopAttrib();
}

void ogl::drawBitmapTextTo2dPosition(float posX,float posY,const std::string& txt)
{
    if (oglFonts[fontIndex].fontBase==0)
        return;
    if (txt.length()==0)
        return;
    glRasterPos3f(posX,posY,0.0f);
    glPushAttrib(GL_LIST_BIT);
    glListBase(oglFonts[fontIndex].fontBase);
    glCallLists((GLsizei)txt.length(),GL_UNSIGNED_BYTE,txt.c_str());
    glPopAttrib();
}

void ogl::drawPlane(float sizeX,float sizeY)
{
    float x=sizeX/2.0f;
    float y=sizeY/2.0f;
    glBegin(GL_QUADS);
    glNormal3f(0.0f,0.0f,1.0f);
    glVertex3f(-x,-y,0.0f);
    glVertex3f(-x,+y,0.0f);
    glVertex3f(+x,+y,0.0f);
    glVertex3f(+x,-y,0.0f);
    glEnd();
}

void ogl::drawBox(float x,float y,float z,bool solid,const float normalVectorForDiffuseComp[3])
{
    float dx=x/2.0f;
    float dy=y/2.0f;
    float dz=z/2.0f;
    if (solid)
    {
        glBegin(GL_QUADS);
        glNormal3f(-1.0f,0.0f,0.0f);
        glVertex3f(-dx,-dy,dz);
        glVertex3f(-dx,dy,dz);
        glVertex3f(-dx,dy,-dz);
        glVertex3f(-dx,-dy,-dz);
        glNormal3f(0.0f,0.0f,-1.0f);
        glVertex3f(-dx,-dy,-dz);
        glVertex3f(-dx,dy,-dz);
        glVertex3f(dx,dy,-dz);
        glVertex3f(dx,-dy,-dz);
        glNormal3f(1.0f,0.0f,0.0f);
        glVertex3f(dx,-dy,-dz);
        glVertex3f(dx,dy,-dz);
        glVertex3f(dx,dy,dz);
        glVertex3f(dx,-dy,dz);
        glNormal3f(0.0f,0.0f,1.0f);
        glVertex3f(dx,-dy,dz);
        glVertex3f(dx,dy,dz);
        glVertex3f(-dx,dy,dz);
        glVertex3f(-dx,-dy,dz);
        glNormal3f(0.0f,-1.0f,0.0f);
        glVertex3f(-dx,-dy,dz);
        glVertex3f(-dx,-dy,-dz);
        glVertex3f(dx,-dy,-dz);
        glVertex3f(dx,-dy,dz);
        glNormal3f(0.0f,1.0f,0.0f);
        glVertex3f(-dx,dy,dz);
        glVertex3f(dx,dy,dz);
        glVertex3f(dx,dy,-dz);
        glVertex3f(-dx,dy,-dz);
        glEnd();
    }
    else
    {
        if (normalVectorForDiffuseComp!=nullptr)
            glNormal3f(normalVectorForDiffuseComp[0],normalVectorForDiffuseComp[1],normalVectorForDiffuseComp[2]);
        else
            glNormal3f(0.0f,0.0f,1.0f);
        glBegin(GL_LINE_STRIP);
        glVertex3f(-dx,-dy,-dz);
        glVertex3f(-dx,+dy,-dz);
        glVertex3f(-dx,+dy,+dz);
        glVertex3f(-dx,-dy,+dz);
        glVertex3f(-dx,-dy,-dz);
        glVertex3f(+dx,-dy,-dz);
        glVertex3f(+dx,+dy,-dz);
        glVertex3f(+dx,+dy,+dz);
        glVertex3f(+dx,-dy,+dz);
        glVertex3f(+dx,-dy,-dz);
        glEnd();
        glBegin(GL_LINES);
        glVertex3f(+dx,+dy,-dz);
        glVertex3f(-dx,+dy,-dz);
        glVertex3f(+dx,+dy,+dz);
        glVertex3f(-dx,+dy,+dz);
        glVertex3f(+dx,-dy,+dz);
        glVertex3f(-dx,-dy,+dz);
        glEnd();
    }
}

void ogl::drawCylinder(float d,float l,int div,int openEnds,bool smooth)
{   // openEnds is 0 by default, smooth is true by default
    //  openEnds=1: end1 is open
    //  openEnds=2: end2 is open
    //  openEnds=3: end1&end2 are open
    float angle=0.0f;
    float r=d/2.0f;
    float halfL=l/2.0f;
    float xVal1,yVal1,xVal2,yVal2;
    float incr=piValue_f*2.0f/((float)div);
    for (int i=0;i<div;i++)
    {
        xVal1=r*(float)cos(angle);
        yVal1=r*(float)sin(angle);
        xVal2=r*(float)cos(angle+incr);
        yVal2=r*(float)sin(angle+incr);
        glBegin(GL_QUADS);
        if (smooth)
        {
            glNormal3f(xVal1/r,yVal1/r,0.0f);
            glVertex3f(xVal1,yVal1,halfL);
            glVertex3f(xVal1,yVal1,-halfL);
            glNormal3f(xVal2/r,yVal2/r,0.0f);
            glVertex3f(xVal2,yVal2,-halfL);
            glVertex3f(xVal2,yVal2,halfL);
        }
        else
        {
            glNormal3f((float)cos(angle+incr/2.0f),(float)sin(angle+incr/2.0f),0.0f);
            glVertex3f(xVal1,yVal1,halfL);
            glVertex3f(xVal1,yVal1,-halfL);
            glVertex3f(xVal2,yVal2,-halfL);
            glVertex3f(xVal2,yVal2,halfL);
        }
        glEnd();
        glBegin(GL_TRIANGLES);
        if ((openEnds&1)==0)
        {
            glNormal3f(0.0f,0.0f,1.0f);
            glVertex3f(0.0f,0.0f,halfL);
            glVertex3f(xVal1,yVal1,halfL);
            glVertex3f(xVal2,yVal2,halfL);
        }
        if ((openEnds&2)==0)
        {
            glNormal3f(0.0f,0.0f,-1.0f);
            glVertex3f(0.0f,0.0f,-halfL);
            glVertex3f(xVal2,yVal2,-halfL);
            glVertex3f(xVal1,yVal1,-halfL);
        }
        glEnd();
        angle=angle+incr;
    }
}

void ogl::drawCircle(float d,int div)
{
    float angle=0.0f;
    float r=d/2.0f;
    float xVal1,yVal1,xVal2,yVal2;
    float incr=piValue_f*2.0f/((float)div);
    glBegin(GL_LINES);
    for (int i=0;i<div;i++)
    {
        xVal1=r*(float)cos(angle);
        yVal1=r*(float)sin(angle);
        xVal2=r*(float)cos(angle+incr);
        yVal2=r*(float)sin(angle+incr);
        glNormal3f((float)cos(angle+incr/2.0f),(float)sin(angle+incr/2.0f),0.0f);
        glVertex3f(xVal1,yVal1,0.0);
        glVertex3f(xVal2,yVal2,0.0);
        angle=angle+incr;
    }
    glEnd();
}

void ogl::drawDisk(float d,int div)
{   // The normal is in the direction of positive z
    float angle=0.0f;
    float r=d/2.0f;
    float xVal1,yVal1,xVal2,yVal2;
    float incr=piValue_f*2.0f/((float)div);
    for (int i=0;i<div;i++)
    {
        xVal1=r*(float)cos(angle);
        yVal1=r*(float)sin(angle);
        xVal2=r*(float)cos(angle+incr);
        yVal2=r*(float)sin(angle+incr);
        glBegin(GL_TRIANGLES);
        glNormal3f(0.0f,0.0f,1.0f);
        glVertex3f(0.0f,0.0f,0.0f);
        glVertex3f(xVal1,yVal1,0.0f);
        glVertex3f(xVal2,yVal2,0.0f);
        glEnd();
        angle=angle+incr;
    }
}

void ogl::drawCone(float d,float l,int div,bool openEnds,bool smooth)
{
    float angle=0.0f;
    float r=d/2.0f;
    float halfL=l/2.0f;
    float xVal1,yVal1,xVal2,yVal2;
    float incr=piValue_f*2.0f/((float)div);
    float t2=(float)sin(atan(r/l));
    float t1=sqrtf(1.0f-t2*t2);
    for (int i=0;i<div;i++)
    {
        xVal1=r*(float)cos(angle);
        yVal1=r*(float)sin(angle);
        xVal2=r*(float)cos(angle+incr);
        yVal2=r*(float)sin(angle+incr);
        glBegin(GL_TRIANGLES);
        if (smooth)
        {
            glNormal3f(t1*xVal2/r,t1*yVal2/r,-t2);
            glVertex3f(xVal2,yVal2,halfL);
            glNormal3f(t1*xVal1/r,t1*yVal1/r,-t2);
            glVertex3f(xVal1,yVal1,halfL);
            glNormal3f(0.0f,0.0f,-1.0f);
            glVertex3f(0.0f,0.0f,-halfL);
        }
        else
        {
            glNormal3f(t1*(float)cos(angle+incr/2.0f),t1*(float)sin(angle+incr/2.0f),-t2);
            glVertex3f(xVal2,yVal2,halfL);
            glVertex3f(xVal1,yVal1,halfL);
            glVertex3f(0.0f,0.0f,-halfL);
        }
        if (!openEnds)
        {
            glNormal3f(0.0f,0.0f,1.0f);
            glVertex3f(0.0f,0.0f,halfL);
            glVertex3f(xVal1,yVal1,halfL);
            glVertex3f(xVal2,yVal2,halfL);
        }
        glEnd();
        angle=angle+incr;
    }
}

void ogl::drawSphere(float r,int sides,int faces,bool smooth)
{
    if (sides<3) sides=3;
    if (faces<2) faces=2;
    float sa=2.0f*piValue_f/((float)sides);
    float fa=piValue_f/((float)faces);
    float p0[3],p1[3],p2[3],p3[3];
    for (int i=0;i<sides;i++)
    {
        for (int j=0;j<faces;j++)
        {
            p0[0]=r*(float)(sin(j*fa)*cos(i*sa));
            p0[1]=r*(float)cos(j*fa);
            p0[2]=r*(float)(sin(j*fa)*sin(i*sa));
            p1[0]=r*(float)(sin(j*fa)*cos((i+1)*sa));
            p1[1]=r*(float)cos(j*fa);
            p1[2]=r*(float)(sin(j*fa)*sin((i+1)*sa));
            p2[0]=r*(float)(sin((j+1)*fa)*cos((i+1)*sa));
            p2[1]=r*(float)cos((j+1)*fa);
            p2[2]=r*(float)(sin((j+1)*fa)*sin((i+1)*sa));
            p3[0]=r*(float)(sin((j+1)*fa)*cos(i*sa));
            p3[1]=r*(float)cos((j+1)*fa);
            p3[2]=r*(float)(sin((j+1)*fa)*sin(i*sa));
            glBegin(GL_QUADS);
            if (smooth)
            {
                glNormal3f(p0[0]/r,p0[1]/r,p0[2]/r);
                glVertex3f(p0[0],p0[1],p0[2]);
                glNormal3f(p1[0]/r,p1[1]/r,p1[2]/r);
                glVertex3f(p1[0],p1[1],p1[2]);
                glNormal3f(p2[0]/r,p2[1]/r,p2[2]/r);
                glVertex3f(p2[0],p2[1],p2[2]);
                glNormal3f(p3[0]/r,p3[1]/r,p3[2]/r);
                glVertex3f(p3[0],p3[1],p3[2]);
            }
            else
            {
                glNormal3f((float)(sin((j+0.5)*fa)*cos((i+0.5)*sa)),
                    (float)cos((j+0.5)*fa),
                    (float)(sin((j+0.5)*fa)*sin((i+0.5)*sa)));
                glVertex3f(p0[0],p0[1],p0[2]);
                glVertex3f(p1[0],p1[1],p1[2]);
                glVertex3f(p2[0],p2[1],p2[2]);
                glVertex3f(p3[0],p3[1],p3[2]);
            }
            glEnd();
        }
    }
}

void ogl::drawReference(float size,bool line,bool setColors,bool emissiveColor,const float normalVectorForDiffuseComp[3])
{
    float subSize=size/8.0f;
    float subSubSize=subSize/2.0f;
    setMaterialColor(ogl::colorBlack,ogl::colorBlack,ogl::colorBlack);
    int colComp=sim_colorcomponent_ambient_diffuse;
    if (emissiveColor)
        colComp=sim_colorcomponent_emission;

    if (line)
    {
        if (normalVectorForDiffuseComp!=nullptr)
            glNormal3fv(normalVectorForDiffuseComp);
        else
            glNormal3f(0.0f,0.0f,1.0f);
        if (setColors)
            setMaterialColor(colComp,ogl::colorRed);
        glBegin(GL_LINES);
        glVertex3f(0.0f,0.0f,0.0f);
        glVertex3f(size,0.0f,0.0f);
        glVertex3f(size-subSize,0.0f,subSubSize);
        glVertex3f(size,0.0f,0.0f);
        glVertex3f(size-subSize,0.0f,-subSubSize);
        glVertex3f(size,0.0f,0.0f);
        glEnd();
    }
    else
    {
        if (setColors)
            setMaterialColor(colComp,ogl::colorRed);
        glPushMatrix();
        glRotatef(90,0,1,0);
        glTranslatef(0.0f,0.0f,size/2.0f);
        drawCylinder(size/20.0f,size,8,3,true);
        glTranslatef(0.0f,0.0f,size*3.0f/5.0f);
        drawCone(size/10.0f,size/5.0f,8,true,true);
        glPopMatrix();
    }
    if (line)
    {
        if (setColors)
            setMaterialColor(colComp,ogl::colorGreen);
        glBegin(GL_LINES);
        glVertex3f(0.0f,0.0f,0.0f);
        glVertex3f(0.0f,size,0.0f);
        glVertex3f(0.0f,size-subSize,subSubSize);
        glVertex3f(0.0f,size,0.0f);
        glVertex3f(0.0f,size-subSize,-subSubSize);
        glVertex3f(0.0f,size,0.0f);
        glEnd();
    }
    else
    {
        if (setColors)
            setMaterialColor(colComp,ogl::colorGreen);
        glPushMatrix();
        glRotatef(-90,1,0,0);
        glTranslatef(0.0f,0.0f,size/2.0f);
        drawCylinder(size/20.0f,size,8,3,true);
        glTranslatef(0.0f,0.0f,size*3.0f/5.0f);
        drawCone(size/10.0f,size/5.0f,8,true,true);
        glPopMatrix();
    }
    if (line)
    {
        if (setColors)
            setMaterialColor(colComp,ogl::colorBlue);
        glBegin(GL_LINES);
        glVertex3f(0.0f,0.0f,0.0f);
        glVertex3f(0.0f,0.0f,size);
        glVertex3f(0.0f,subSubSize,size-subSize);
        glVertex3f(0.0f,0.0f,size);
        glVertex3f(0.0f,-subSubSize,size-subSize);
        glVertex3f(0.0f,0.0f,size);
        glEnd();
    }
    else
    {
        if (setColors)
            setMaterialColor(colComp,ogl::colorBlue);
        glPushMatrix();
        glTranslatef(0.0f,0.0f,size/2.0f);
        drawCylinder(size/20.0f,size,8,3,true);
        glTranslatef(0.0f,0.0f,size*3.0f/5.0f);
        drawCone(size/10.0f,size/5.0f,8,true,true);
        glPopMatrix();
    }
}

void ogl::perspectiveSpecial(float fovy,float aspect,float zNear,float zFar)
{   // Same function as gluPerspective
   float ymax=zNear*(float)tan(fovy*piValue_f/360.0f);
   float ymin=-ymax;
   float xmin=ymin*aspect;
   float xmax=ymax*aspect;
   _frustumSpecial(xmin,xmax,ymin,ymax,zNear,zFar);
}

void ogl::_frustumSpecial(float left,float right,float bottom,float top,float nearval,float farval)
{
   float x, y, a, b, c, d;
   float m[16];
   x=(2.0f*nearval)/(right-left);
   y=(2.0f*nearval)/(top-bottom);
   a=(right+left)/(right-left);
   b=(top+bottom)/(top-bottom);
   c=-(farval+nearval)/(farval-nearval);
   d=-(2.0f*farval*nearval)/(farval-nearval);
#define M(row,col)  m[col*4+row]
   M(0,0)=x;     M(0,1)=0.0f;  M(0,2)=a;      M(0,3)=0.0f;
   M(1,0)=0.0f;  M(1,1)=y;     M(1,2)=b;      M(1,3)=0.0f;
   M(2,0)=0.0f;  M(2,1)=0.0f;  M(2,2)=c;      M(2,3)=d;
   M(3,0)=0.0f;  M(3,1)=0.0f;  M(3,2)=-1.0f;  M(3,3)=0.0f;
#undef M
   glMultMatrixf(m);
}

void ogl::pickMatrixSpecial(int x,int y,int width,int height,int viewport[4])
{   // Same function as gluPickMatrix()
   GLfloat m[16];
   GLfloat sx,sy;
   GLfloat tx,ty;
   sx=((float)viewport[2])/((float)width);
   sy=((float)viewport[3])/((float)height);
   tx=(((float)viewport[2])+2.0f*((float)(viewport[0]-x)))/((float)width);
   ty=(((float)viewport[3])+2.0f*((float)(viewport[1]-y)))/((float)height);
#define M(row,col)  m[col*4+row]
   M(0,0)=sx;
   M(0,1)=0.0f;
   M(0,2)=0.0f;
   M(0,3)=tx;
   M(1,0)=0.0f;
   M(1,1)=sy;
   M(1,2)=0.0f;
   M(1,3)=ty;
   M(2,0)=0.0f;
   M(2,1)=0.0f;
   M(2,2)=1.0f;
   M(2,3)=0.0f;
   M(3,0)=0.0f;
   M(3,1)=0.0f;
   M(3,2)=0.0f;
   M(3,3)=1.0f;
#undef M
   glMultMatrixf(m);
}
















/*
void ogl::drawCenteredText(int posX,int posY,int posZ,std::string txt,bool verticallyCentered)
{   // verticallyCentered is false by default
    if (txt.length()==0) 
        return;
    int width=getTextLengthInPixels(txt);
    drawText(posX-width/2,posY,posZ,txt,verticallyCentered);
}
*/
void ogl::drawText(int posX,int posY,int posZ,std::string txt,bool verticallyCentered)
{   // verticallyCentered is false by default
    if (oglFonts[fontIndex].fontBase==0)
        return;
    while ( (posX<0)&&(txt.length()!=0) )
    {
        posX=posX+oglFonts[fontIndex].fontWidths[int(txt[0])];
        txt.erase(txt.begin());
    }
    if (txt.length()==0)
        return;
    int vCorr=0;
    if (verticallyCentered)
        vCorr=-oglFonts[fontIndex].fontHeight/4;
    glRasterPos3i(posX,posY+vCorr,posZ);    
    glPushAttrib(GL_LIST_BIT);
    glListBase(oglFonts[fontIndex].fontBase);
    glCallLists((GLsizei)txt.length(),GL_UNSIGNED_BYTE,txt.c_str());
    glPopAttrib();
}
void ogl::drawBitmapTextBackgroundIntoScene(float posX,float posY,float posZ,std::string txt)
{
    int l=ogl::getTextLengthInPixels(txt.c_str());
    if (l!=0)
    {
        glRasterPos3f(posX,posY,posZ);
        glBitmap(l,oglFonts[fontIndex].fontHeight,0.0f,float(oglFonts[fontIndex].fontHeight/4),0.0f,0.0f,(GLubyte*)opacBitmap);
    }
}

void ogl::drawBitmapTextIntoScene(float posX,float posY,float posZ,const std::string& txt)
{
    if (oglFonts[fontIndex].fontBase==0) 
        return;
    if (txt.length()==0)
        return;
    glRasterPos3f(posX,posY,posZ);
    glPushAttrib(GL_LIST_BIT);
    glListBase(oglFonts[fontIndex].fontBase);
    glCallLists((GLsizei)txt.length(),GL_UNSIGNED_BYTE,txt.c_str());
    glPopAttrib();
}

void ogl::drawTexti(int posX,int posY,int posZ,std::string txt)
{
    if (oglFonts[fontIndex].fontBase==0) 
        return;
    if (txt.length()==0)
        return;
    glRasterPos3i(posX,posY,posZ);  
    glPushAttrib(GL_LIST_BIT);
    glListBase(oglFonts[fontIndex].fontBase);
    glCallLists((GLsizei)txt.length(),GL_UNSIGNED_BYTE,txt.c_str());
    glPopAttrib();
}

int ogl::getTextLengthInPixels(const std::string& txt)
{
    if (oglFonts[fontIndex].fontBase==0)
        return(0);
    int width=0;
    for (int i=0;i<int(txt.length());i++)
        width=width+oglFonts[fontIndex].fontWidths[(unsigned char)txt[i]];
    return(width);
}

int ogl::getSeparateWords(const char* txt,std::vector<std::string>& separateWords)
{
    std::string aWord;
    size_t l=strlen(txt);
    for (size_t i=0;i<l;i++)
    {
        if (txt[i]==' ')
        {
            if (aWord.length()!=0)
            {
                separateWords.push_back(aWord);
                aWord="";
            }
        }
        else
            aWord+=txt[i];
    }
    if (aWord.length()!=0)
        separateWords.push_back(aWord);
    return(int(separateWords.size()));
}

int ogl::getSeparateWordWidths(const std::vector<std::string>& separateWords,std::vector<int>& widths)
{
    if (oglFonts[fontIndex].fontBase==0)
        return(0);
    for (int i=0;i<int(separateWords.size());i++)
    {
        int l=0;
        for (int j=0;j<int(separateWords[i].length());j++)
            l+=oglFonts[fontIndex].fontWidths[(unsigned char)separateWords[i][j]];
        widths.push_back(l);
    }
    return(oglFonts[fontIndex].fontWidths[32]); // We return the width of the space!
}


std::string ogl::getTextThatFitIntoPixelWidth(std::vector<std::string>& separateWords,std::vector<int>& widths,int spaceWidth,int pixelWidth,int& textCharCount_pixelWidth,bool allowWordCut)
{
    if (separateWords.size()==0)
        return("");

    // 1. Get the first word at first (special case):
    std::string retVal;
    int width=0;

    if (widths[0]<=pixelWidth)
    { // The whole first word fits!
        retVal=separateWords[0];
        width=widths[0];
        separateWords.erase(separateWords.begin());
        widths.erase(widths.begin());
    }
    else
    { // Only part of the first word fit!
        int countToErase=0;
        for (int i=0;i<int(separateWords[0].length());i++)
        {
            int s=oglFonts[fontIndex].fontWidths[(unsigned char)separateWords[0][i]];
            width+=s;
            if (width<=pixelWidth)
            {
                retVal+=separateWords[0][i];
                countToErase++;
            }
            else
            {
                width-=s;
                break;
            }
        }
        separateWords[0].erase(separateWords[0].begin(),separateWords[0].begin()+countToErase);
        widths[0]=getTextLengthInPixels(separateWords[0]);
        textCharCount_pixelWidth=width;
        return(retVal);
    }

    // 2. Now get additional words if possible (or part of words if allowWordCut is true):
    while (separateWords.size()!=0)
    {
        if (!allowWordCut)
        { // We are not allowed to cut words
            if (width+spaceWidth+widths[0]<=pixelWidth)
            { // The whole word fits!
                width+=spaceWidth+widths[0];
                retVal+=' ';
                retVal+=separateWords[0];
                separateWords.erase(separateWords.begin());
                widths.erase(widths.begin());
            }
            else
            { // The whole word doesn't fit, we leave here
                textCharCount_pixelWidth=width;
                return(retVal);
            }
        }
        else
        { // We are allowed to cut words
            if (width+spaceWidth+widths[0]<=pixelWidth)
            { // The whole word fits!
                width+=spaceWidth+widths[0];
                retVal+=' ';
                retVal+=separateWords[0];
                separateWords.erase(separateWords.begin());
                widths.erase(widths.begin());
            }
            else
            { // The whole word doesn't fit
                if (width+spaceWidth+oglFonts[fontIndex].fontWidths[(unsigned char)separateWords[0][0]]<=pixelWidth)
                { // Part of the word fits
                    retVal+=' ';
                    width+=spaceWidth;
                    while (separateWords[0].length()!=0)
                    {
                        int s=oglFonts[fontIndex].fontWidths[(unsigned char)separateWords[0][0]];
                        if (width+s<=pixelWidth)
                        {
                            width+=s;
                            retVal+=separateWords[0][0];
                            separateWords[0].erase(separateWords[0].begin());
                            widths[0]-=s;
                        }
                        else
                        { // Nothing can be appended anymore
                            textCharCount_pixelWidth=width;
                            return(retVal);
                        }
                    }
                }
                else
                { // Nothing can be appended
                    textCharCount_pixelWidth=width;
                    return(retVal);
                }
            }
        }
    }

    textCharCount_pixelWidth=width;
    return(retVal);
}

void ogl::loadBitmapFont(const unsigned char* fontData,int fontHeight,int theFontIndex)
{
    oglFonts[theFontIndex].fontHeight=fontHeight;
    oglFonts[theFontIndex].fontBase=glGenLists(256);
    for (int i=0;i<256;i++)
        oglFonts[theFontIndex].fontWidths[i]=0;
    unsigned char temp[2000];
    int letter=32;
    int p=0;
    while (letter<32+96)
    {
        for (int i=0;i<2000;i++)
            temp[i]=0;
        int sizeX=fontData[p++];
        int sizeY=fontData[p++];
        int originX=(char)fontData[p++];
        int originY=(char)fontData[p++];
        int charWidth=fontData[p++];
//      if (letter==int('e'))
    //      int aaa=0;
        oglFonts[theFontIndex].fontWidths[letter]=charWidth;
        int byteCntX=((sizeX-1)/8)+1;
        for (int y=0;y<sizeY;y++)
        {
            for (int x=0;x<byteCntX;x++)
            {
                temp[y*4+x]=fontData[p++]; // we use a 4 byte alignement because of MS error in the openGl implementation
            }
        }
        glNewList(oglFonts[theFontIndex].fontBase+letter,GL_COMPILE);
        glBitmap(sizeX,sizeY,float(originX),float(originY),float(charWidth),0.0f,temp);
        glEndList();
        letter++;
    }

    // Following needed for the bitmap font background:
    for (int i=0;i<10000;i++)
        opacBitmap[i]=char(255);
}

void ogl::selectBitmapFont(int index)
{
    fontIndex=index;
}

void ogl::freeBitmapFonts()
{
    int lfontIndex=0;
    while (oglFonts[lfontIndex].fontHeight!=0)
    {
        glDeleteLists(oglFonts[lfontIndex].fontBase+32,96);
        lfontIndex++;
    }
}

int ogl::getFontHeight()
{
    if (oglFonts[fontIndex].fontBase==0)
        return(0);
    return(oglFonts[fontIndex].fontHeight);
}

int ogl::getFontCharWidth(unsigned char c)
{
    if (oglFonts[fontIndex].fontBase==0)
        return(0);
    return(oglFonts[fontIndex].fontWidths[c]);
}

int ogl::getInterline()
{
    if (oglFonts[fontIndex].fontBase==0)
        return(0);
    return(oglFonts[fontIndex].fontHeight-4);
}

void ogl::loadOutlineFont(const int* fontData,const float* fontVertices)
{
    outlineFontBase=glGenLists(256);                                // Storage For 96 Characters
    for (int i=0;i<256;i++)
        outlineFontSizes[i]=0.0f;
    int p=0;
    for (int letter=32;letter<32+96;letter++)
    {
//      int vertCnt=fontData[p++];
        p++;

        int indCnt=fontData[p++];
        outlineFontSizes[letter]=(float(fontData[p++])/1000.0f)+0.154f*0.6f; // 0.0.154 corresponds to the width of the "I"

        glNewList(outlineFontBase+letter,GL_COMPILE);
        glBegin(GL_TRIANGLES);
        glNormal3f(0.0f,0.0f,1.0f);
        for (int i=0;i<indCnt*3;i++)
        {
            glVertex2fv(fontVertices+2*fontData[p++]+0);
        }
        glEnd();
        glTranslatef(outlineFontSizes[letter],0.0f,0.0f);
        glEndList();
    }
    outlineFontHeight=1.0f;
    outlineFontCenter=0.275f;
}

void ogl::freeOutlineFont()
{
    glDeleteLists(outlineFontBase+32,96);
}

void ogl::drawOutlineText(const char* txt,const C7Vector& trOrig,float textHeight,const C4X4Matrix* cameraAbsConfig,bool overlay,bool left,bool right,bool culling,const float* textColor,const float* backColor/*=nullptr*/,bool sizeInPixels/*=false*/,int windowHeight/*=0*/,float verticalViewSizeOrAngle/*=0.0f*/,bool perspective/*=true*/,bool fullyFacingCamera/*=true*/)
{ // cameraAbsConfig can be nullptr. If not, the text is always facing the camera
    // If backColor is nullptr, there is no background
    float length=0; 

    const char* text=txt;

    int l=int(strlen(text));
    for (int loop=0;loop<l;loop++)
        length+=outlineFontSizes[int(text[loop])];
    if (left&&right)
    {
        left=false;
        right=false;
    }

    C7Vector tr(trOrig);
    if (cameraAbsConfig!=nullptr)
    { // the banner has always to face the camera
        C3X3Matrix m;
        if (fullyFacingCamera)
        {
            m.axis[2]=cameraAbsConfig->M.axis[2]*-1.0f;
            m.axis[0]=cameraAbsConfig->M.axis[0]*-1.0f;
            m.axis[1]=cameraAbsConfig->M.axis[1];
            tr.Q=m.getQuaternion();
        }
        else
        { // Rotate around the banner's y axis to make it "relatively" face the camera
            m=tr.Q.getMatrix();
            C3Vector lll(cameraAbsConfig->X-tr.X);
            if (lll.getLength()!=0.0f)
            {
                lll.normalize();
                if (fabs(lll*m.axis[1])<0.999f)
                {
                    m.axis[0]=(m.axis[1]^lll).getNormalized();
                    m.axis[2]=(m.axis[0]^m.axis[1]).getNormalized();
                }
            }
            tr.Q=m.getQuaternion();
        }


        if (sizeInPixels)
        {
            if (!perspective)
                textHeight=float(textHeight)*verticalViewSizeOrAngle/float(windowHeight);
            else
            {
                C3Vector v(tr.X-cameraAbsConfig->X);
                float d=fabs(v*cameraAbsConfig->M.axis[2]);
                textHeight=2.0f*float(textHeight)*tan(verticalViewSizeOrAngle*0.5f)*d/float(windowHeight);
            }
        }
    }


    float ld2=(length+outlineFontHeight*0.1f)*textHeight/2.0f;
    float hd2=outlineFontHeight*1.1f*textHeight/2.0f;
    float ldo=0.0f;

    if (right)
        ldo=(length)*textHeight/2.0f;
    if (left)
        ldo=-(length)*textHeight/2.0f;

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    for (int i=0;i<2;i++)
    {
        if (i==1)
        {
            C3X3Matrix m(tr.Q.getMatrix());
            m.axis[0]*=-1.0f;
            m.axis[2]*=-1.0f;
            tr.Q=m.getQuaternion();
        }

        if (overlay)
        {
            glDisable(GL_DEPTH_TEST);
            glDepthRange(0.0f,0.0f);
        }
        else
        {
            if (backColor!=nullptr)
                glDepthMask(GL_FALSE);
        }

        if (backColor!=nullptr)
        {
            glPushMatrix();
            glTranslatef(tr.X(0),tr.X(1),tr.X(2));
            C4Vector axis=tr.Q.getAngleAndAxisNoChecking();
            glRotatef(axis(0)*radToDeg_f,axis(1),axis(2),axis(3));
            setMaterialColor(backColor,backColor+6,backColor+9);
            glBegin(GL_QUADS);
            glNormal3f(0.0f,0.0f,1.0f);
            glVertex3f(ldo-ld2,+hd2,0.0f);
            glVertex3f(ldo-ld2,-hd2,0.0f);
            glVertex3f(ldo+ld2,-hd2,0.0f);
            glVertex3f(ldo+ld2,+hd2,0.0f);
            glEnd();
            glPopMatrix();
        }

        glPushMatrix();
        C3X3Matrix m(tr.Q);
        float lq=length;
        if (right)
            lq=0.0f;
        if (left)
            lq*=2.0f;
        C3Vector x(tr.X+m.axis[0]*(-lq/2.0f)*textHeight+m.axis[1]*(-outlineFontCenter)*textHeight);
        glTranslatef(x(0),x(1),x(2));
        C4Vector axis=tr.Q.getAngleAndAxisNoChecking();
        glRotatef(axis(0)*radToDeg_f,axis(1),axis(2),axis(3));
        glEnable(GL_NORMALIZE); // Might be important since normals are also scaled (bad lighting)
        glScalef(textHeight,textHeight,textHeight);
        setMaterialColor(textColor,textColor+6,textColor+9);
        glPushAttrib(GL_LIST_BIT);
        glListBase(outlineFontBase);    
        glCallLists(l,GL_UNSIGNED_BYTE,text);
        glPopAttrib();

        glScalef(1.0f,1.0f,1.0f); // Reset scaling
        glDisable(GL_NORMALIZE); // Set default again (faster)

        glPopMatrix();

        glDepthMask(GL_TRUE);

        if (backColor!=nullptr)
        {
            glColorMask(GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE);
            glPushMatrix();
            glTranslatef(tr.X(0),tr.X(1),tr.X(2));
            glRotatef(axis(0)*radToDeg_f,axis(1),axis(2),axis(3));
            setMaterialColor(backColor,backColor+6,backColor+9);
            glBegin(GL_QUADS);
            glNormal3f(0.0f,0.0f,1.0f);
            glVertex3f(ldo-ld2,+hd2,0.0f);
            glVertex3f(ldo-ld2,-hd2,0.0f);
            glVertex3f(ldo+ld2,-hd2,0.0f);
            glVertex3f(ldo+ld2,+hd2,0.0f);
            glEnd();
            glPopMatrix();
            glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
        }

        if (overlay)
            glEnable(GL_DEPTH_TEST);

        if ( (cameraAbsConfig!=nullptr)||culling )
            break; // we don't have to render twice here!
    }
    glDepthRange(0.0f,1.0f);
    glDisable(GL_CULL_FACE);
}


void ogl::drawQuad(int posX,int posY,int sizeX,int sizeY,float col[3])
{ // Draws a simple quad
    setMaterialColor(sim_colorcomponent_emission,col);
    glBegin(GL_QUADS);
    glVertex3i(posX,posY,0);
    glVertex3i(posX+sizeX,posY,0);
    glVertex3i(posX+sizeX,posY+sizeY,0);
    glVertex3i(posX,posY+sizeY,0);
    glEnd();
}

void ogl::drawBorder(int posX,int posY,int sizeX,int sizeY,float col[3],int type,int borderSize)
{   // Draws a frame. borderSize is 3 by default
    // type: -1: nothing,0=button, 1=pressed button, 2=2D borders, 3=3D borders, 4=filled
    if (type==-1)
        return;
    if (type<3)
    {
        float ul=1.0f;
        float lr=1.0f;
        if (type==0)
        {
            ul=1.5f;
            lr=0.5f;
        }
        if (type==1)
        {
            ul=0.5f;
            lr=1.5f;
        }
        if (type==2)
        {
            ul=0.5f;
            lr=0.5f;
        }
        glBegin(GL_LINES);
        setMaterialColor(sim_colorcomponent_emission,col[0]*ul,col[1]*ul,col[2]*ul);
        glVertex3i(posX,posY,0);
        glVertex3i(posX,posY+sizeY,0);
        glVertex3i(posX,posY+sizeY,0);
        glVertex3i(posX+sizeX,posY+sizeY,0);
        setMaterialColor(sim_colorcomponent_emission,col[0]*lr,col[1]*lr,col[2]*lr);
        glVertex3i(posX,posY,0);
        glVertex3i(posX+sizeX,posY,0);
        glVertex3i(posX+sizeX,posY,0);
        glVertex3i(posX+sizeX,posY+sizeY,0);
        glEnd();
    }
    else
    {
        setMaterialColor(sim_colorcomponent_emission,col);
        glBegin(GL_QUADS);
        glVertex3i(posX,posY,0);
        glVertex3i(posX+sizeX,posY,0);
        glVertex3i(posX+sizeX,posY+borderSize,0);
        glVertex3i(posX,posY+borderSize,0);
        glVertex3i(posX,posY,0);
        glEnd();
        glBegin(GL_QUADS);
        glVertex3i(posX+sizeX-borderSize,posY,0);
        glVertex3i(posX+sizeX,posY,0);
        glVertex3i(posX+sizeX,posY+sizeY,0);
        glVertex3i(posX+sizeX-borderSize,posY+sizeY,0);
        glVertex3i(posX+sizeX-borderSize,posY,0);
        glEnd();
        glBegin(GL_QUADS);
        glVertex3i(posX,posY+sizeY-borderSize,0);
        glVertex3i(posX+sizeX,posY+sizeY-borderSize,0);
        glVertex3i(posX+sizeX,posY+sizeY,0);
        glVertex3i(posX,posY+sizeY,0);
        glVertex3i(posX,posY+sizeY-borderSize,0);
        glEnd();
        glBegin(GL_QUADS);
        glVertex3i(posX,posY,0);
        glVertex3i(posX+borderSize,posY,0);
        glVertex3i(posX+borderSize,posY+sizeY,0);
        glVertex3i(posX,posY+sizeY,0);
        glVertex3i(posX,posY,0);
        glEnd();
        if (type==3)
        {
            drawBorder(posX,posY,sizeX,sizeY,col,0,borderSize);
            drawBorder(posX+borderSize,posY+borderSize,sizeX-2*borderSize,sizeY-2*borderSize,col,1,borderSize);
        }
    }
}

void ogl::drawRichTextIcon(int centerPosX,int centerPosY,int size,int icon)
{
    size=size*8/10;
    int lineW=size/10;
    if (lineW<1)
        lineW=1;
    size=(size-2)/2;
    if (size<1)
        size=1;
    if (icon==BOX_RICH_TEXT)
    {
        glBegin(GL_QUADS);
        glVertex3i(centerPosX-size,centerPosY-size,0);
        glVertex3i(centerPosX+size,centerPosY-size,0);
        glVertex3i(centerPosX+size,centerPosY+size,0);
        glVertex3i(centerPosX-size,centerPosY+size,0);
        glEnd();
    }
    if (icon==PAUSE_RICH_TEXT)
    {
        glBegin(GL_QUADS);
        glVertex3i(centerPosX-size,centerPosY-size,0);
        glVertex3i(centerPosX-size*3/10,centerPosY-size,0);
        glVertex3i(centerPosX-size*3/10,centerPosY+size,0);
        glVertex3i(centerPosX-size,centerPosY+size,0);
        glVertex3i(centerPosX+size,centerPosY-size,0);
        glVertex3i(centerPosX+size,centerPosY+size,0);
        glVertex3i(centerPosX+size*3/10,centerPosY+size,0);
        glVertex3i(centerPosX+size*3/10,centerPosY-size,0);
        glEnd();
    }           
    if (icon==START_RICH_TEXT)
    {
        glBegin(GL_QUADS);
        glVertex3i(centerPosX-size,centerPosY-size,0);
        glVertex3i(centerPosX-size*3/10,centerPosY-size,0);
        glVertex3i(centerPosX-size*3/10,centerPosY+size,0);
        glVertex3i(centerPosX-size,centerPosY+size,0);
        glEnd();
        glBegin(GL_TRIANGLES);
        glVertex3i(centerPosX,centerPosY,0);
        glVertex3i(centerPosX+size,centerPosY-size,0);
        glVertex3i(centerPosX+size,centerPosY+size,0);
        glEnd();
    }
    if (icon==END_RICH_TEXT)
    {
        glBegin(GL_QUADS);
        glVertex3i(centerPosX+size,centerPosY-size,0);
        glVertex3i(centerPosX+size,centerPosY+size,0);
        glVertex3i(centerPosX+size*3/10,centerPosY+size,0);
        glVertex3i(centerPosX+size*3/10,centerPosY-size,0);
        glEnd();
        glBegin(GL_TRIANGLES);
        glVertex3i(centerPosX,centerPosY,0);
        glVertex3i(centerPosX-size,centerPosY+size,0);
        glVertex3i(centerPosX-size,centerPosY-size,0);
        glEnd();
    }
    if (icon==ARROW_LEFT_RICH_TEXT)
    {
        glBegin(GL_TRIANGLES);
        glVertex3i(centerPosX-size,centerPosY,0);
        glVertex3i(centerPosX+size,centerPosY-size,0);
        glVertex3i(centerPosX+size,centerPosY+size,0);
        glEnd();
    }
    if (icon==ARROW_RIGHT_RICH_TEXT)
    {
        glBegin(GL_TRIANGLES);
        glVertex3i(centerPosX+size,centerPosY,0);
        glVertex3i(centerPosX-size,centerPosY+size,0);
        glVertex3i(centerPosX-size,centerPosY-size,0);
        glEnd();
    }
    if (icon==ARROW_UP_RICH_TEXT)
    {
        glBegin(GL_TRIANGLES);
        glVertex3i(centerPosX,centerPosY+size,0);
        glVertex3i(centerPosX-size,centerPosY-size,0);
        glVertex3i(centerPosX+size,centerPosY-size,0);
        glEnd();
    }
    if (icon==ARROW_DOWN_RICH_TEXT)
    {
        glBegin(GL_TRIANGLES);
        glVertex3i(centerPosX,centerPosY-size,0);
        glVertex3i(centerPosX+size,centerPosY+size,0);
        glVertex3i(centerPosX-size,centerPosY+size,0);
        glEnd();
    }
    if (icon==CHECK_RICH_TEXT)
    {
        if (size>8) // size is now half size!!
        {
            glPushMatrix();
            glTranslatef(float(centerPosX),float(centerPosY),0.0f);
            glRotatef(45.0f,0.0f,0.0f,1.0f);
            glBegin(GL_QUADS);
            glVertex3i(-size,+lineW/2-lineW,0);
            glVertex3i(+size,+lineW/2-lineW,0);
            glVertex3i(+size,+lineW/2,0);
            glVertex3i(-size,+lineW/2,0);
            glVertex3i(+lineW/2-lineW,-size,0);
            glVertex3i(+lineW/2,-size,0);
            glVertex3i(+lineW/2,+size,0);
            glVertex3i(+lineW/2-lineW,+size,0);
            glEnd();
            glPopMatrix();
        }
        else
        {
            glBegin(GL_LINES);
            glVertex3i(centerPosX-size,centerPosY-size,0);
            glVertex3i(centerPosX+size,centerPosY+size,0);
            glVertex3i(centerPosX-size,centerPosY+size,0);
            glVertex3i(centerPosX+size,centerPosY-size,0);
            glEnd();
        }
    }
    if (icon==SQUARE_RICH_TEXT)
    {
        glBegin(GL_QUADS);
        glVertex3i(centerPosX-size,centerPosY-size,0);
        glVertex3i(centerPosX+size,centerPosY-size,0);
        glVertex3i(centerPosX+size,centerPosY-size+lineW,0);
        glVertex3i(centerPosX-size,centerPosY-size+lineW,0);

        glVertex3i(centerPosX+size-lineW,centerPosY-size,0);
        glVertex3i(centerPosX+size,centerPosY-size,0);
        glVertex3i(centerPosX+size,centerPosY+size,0);
        glVertex3i(centerPosX+size-lineW,centerPosY+size,0);

        glVertex3i(centerPosX-size,centerPosY+size-lineW,0);
        glVertex3i(centerPosX+size,centerPosY+size-lineW,0);
        glVertex3i(centerPosX+size,centerPosY+size,0);
        glVertex3i(centerPosX-size,centerPosY+size,0);
    
        glVertex3i(centerPosX-size,centerPosY-size,0);
        glVertex3i(centerPosX-size+lineW,centerPosY-size,0);
        glVertex3i(centerPosX-size+lineW,centerPosY+size,0);
        glVertex3i(centerPosX-size,centerPosY+size,0);
        glEnd();
    }
    if (icon==MINIMIZE_RICH_TEXT)
    {
        glBegin(GL_QUADS);
        glVertex3i(centerPosX-size+lineW,centerPosY-size+lineW,0);
        glVertex3i(centerPosX+size-lineW,centerPosY-size+lineW,0);
        glVertex3i(centerPosX+size-lineW,centerPosY-size+3*lineW,0);
        glVertex3i(centerPosX-size+lineW,centerPosY-size+3*lineW,0);
        glEnd();
    }
}

void ogl::drawRichText(int posX,int posY,int size,std::string& text,std::vector<int>& iconsAndPos,int alignement,float backgroundColor[3],float secondTextColor[3])
{   // secondTextColor can be nullptr in which case the text is not displayed double!
    if (oglFonts[fontIndex].fontBase==0)
        return;
    int width=0;
    for (int i=0;i<int(text.length());i++)
        width+=oglFonts[fontIndex].fontWidths[int(text[i])];
    int iconSize=oglFonts[fontIndex].fontHeight; // with text, icons have to be small
    bool onlyIcons=false;
    if (text.length()==0)
    {
        iconSize=size; // without text, the icons can be big
        onlyIcons=true;
        for (int i=0;i<int(iconsAndPos.size())/2;i++)
            text+='0'; // to keep below loop alive until all icons have been printed!
    }
    int iconCnt=0;
    int ind=0;
    while (ind<int(iconsAndPos.size()))
    {
        if (iconsAndPos[ind+1]==COLOR_AND_COLOR_RESET_RICH_TEXT)
            ind+=5;
        else
        {
            ind+=2;
            iconCnt++;
        }
    }
    width+=iconCnt*iconSize;

    int pos[2]={posX,posY}; // Left aligned
    if (alignement==1) // Centered
        pos[0]=pos[0]-width/2;
    if (alignement==2) // Right aligned
        pos[0]=pos[0]-width;
//  int startPos[2]={pos[0],pos[1]};
    int iconIndex=0;
    int textP=pos[0];
    std::string bufferedText;
    for (int i=0;i<int(text.length()+1);i++)
    {
        bool ignoreNextChar=false;
        // Do we need an icon here?
        if (int(iconsAndPos.size())>iconIndex)
        {
            if (iconsAndPos[iconIndex]==i)
            { // Yes!
                // We have to draw the buffered text:
                if (bufferedText.length()!=0)
                {
                    if (secondTextColor!=nullptr)
                    { // We have to draw the text twice!
                        float tc[3];
                        getRichTextColor(tc);
                        setRichTextColor(secondTextColor);
                        glRasterPos3i(textP+1,-1+pos[1]-oglFonts[fontIndex].fontHeight/4,0);    // we have a small vertical correction
                        glPushAttrib(GL_LIST_BIT);
                        glListBase(oglFonts[fontIndex].fontBase);
                        glCallLists((GLsizei)bufferedText.length(),GL_UNSIGNED_BYTE,bufferedText.c_str());
                        glPopAttrib();
                        setRichTextColor(tc); // We restore previous color
                    }
                    glRasterPos3i(textP,pos[1]-oglFonts[fontIndex].fontHeight/4,0); // we have a small vertical correction
                    glPushAttrib(GL_LIST_BIT);
                    glListBase(oglFonts[fontIndex].fontBase);
                    glCallLists((GLsizei)bufferedText.length(),GL_UNSIGNED_BYTE,bufferedText.c_str());
                    glPopAttrib();
                    bufferedText.clear();
                }
                // Now the icon:
                if (iconsAndPos[iconIndex+1]==COLOR_AND_COLOR_RESET_RICH_TEXT)
                { // This is a pseudo-icon: we just wanna change color!
                    if (iconsAndPos[iconIndex+2]==16) // this indicates a color reset
                    {
                        float tc[3];
                        getRichTextDefaultColor(tc);
                        setRichTextColor(tc);
                    }
                    else
                    {
                        float tc[3]={float(iconsAndPos[iconIndex+2])/9.0f,float(iconsAndPos[iconIndex+3])/9.0f,float(iconsAndPos[iconIndex+4])/9.0f};
                        setRichTextColor(tc);
                    }
                    iconIndex+=5;
                }
                else
                { // this is a real icon
                    if (secondTextColor!=nullptr)
                    { // We have to draw the icon twice!
                        float tc[3];
                        getRichTextColor(tc);
                        setRichTextColor(secondTextColor);
                        drawRichTextIcon(1+pos[0]+iconSize/2,-1+pos[1],iconSize,iconsAndPos[iconIndex+1]);
                        setRichTextColor(tc); // We restore previous color
                    }
                    drawRichTextIcon(pos[0]+iconSize/2,pos[1],iconSize,iconsAndPos[iconIndex+1]);
                    pos[0]+=iconSize;
                    iconIndex+=2;
                }
                if (int(iconsAndPos.size())>iconIndex)
                {
                    if (iconsAndPos[iconIndex]==i)
                    { // next icon follows directly
                        i--;
                        ignoreNextChar=true;

                    }
                }
                textP=pos[0];
            }
        }
        if ( (!onlyIcons)&&(i<int(text.length()))&&(!ignoreNextChar) )
        {
            if (pos[0]>=0)
            {
                bufferedText+=text[i];
                pos[0]+=oglFonts[fontIndex].fontWidths[int(text[i])];
            }
            else
            {
                pos[0]+=oglFonts[fontIndex].fontWidths[int(text[i])];
                textP=pos[0];
            }
        }
    }
    if (onlyIcons)
        text.clear();
    if (bufferedText.length()!=0)
    {   // We have to draw the buffered text:
        if (secondTextColor!=nullptr)
        { // We have to draw the text twice!
            float tc[3];
            getRichTextColor(tc);
            setRichTextColor(secondTextColor);
            glRasterPos3i(textP+1,-1+pos[1]-oglFonts[fontIndex].fontHeight/4,0);    // we have a small vertical correction
            glPushAttrib(GL_LIST_BIT);
            glListBase(oglFonts[fontIndex].fontBase);
            glCallLists((GLsizei)bufferedText.length(),GL_UNSIGNED_BYTE,bufferedText.c_str());
            glPopAttrib();
            setRichTextColor(tc); // We restore previous color
        }
        glRasterPos3i(textP,pos[1]-oglFonts[fontIndex].fontHeight/4,0); // we have a small vertical correction
        glPushAttrib(GL_LIST_BIT);
        glListBase(oglFonts[fontIndex].fontBase);
        glCallLists((GLsizei)bufferedText.length(),GL_UNSIGNED_BYTE,bufferedText.c_str());
        glPopAttrib();
    }
}

void ogl::setRichTextColor(float col[3])
{
    currentRichTextColor[0]=col[0]; 
    currentRichTextColor[1]=col[1]; 
    currentRichTextColor[2]=col[2]; 
    setTextColor(col);
}

void ogl::getRichTextColor(float col[3])
{
    col[0]=currentRichTextColor[0]; 
    col[1]=currentRichTextColor[1]; 
    col[2]=currentRichTextColor[2]; 
}

void ogl::getRichTextDefaultColor(float col[3])
{
    col[0]=defaultRichTextColor[0]; 
    col[1]=defaultRichTextColor[1]; 
    col[2]=defaultRichTextColor[2]; 
}

void ogl::setRichTextDefaultColor(float col[3])
{
    defaultRichTextColor[0]=col[0]; 
    defaultRichTextColor[1]=col[1]; 
    defaultRichTextColor[2]=col[2]; 
    setTextColor(defaultRichTextColor);
}

void ogl::drawButtonEdit(VPoint p,VPoint s,bool selected,bool mainSel)
{ // Draws an overlay for a button in edit-mode
    // Before calling this function, make sure that viewSize==viewportSize!!
    // We work in screen pixel coordinates!
    glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    if (mainSel)
        setMaterialColor(sim_colorcomponent_emission,ogl::colorWhite);
    else if (selected)
        setMaterialColor(sim_colorcomponent_emission,ogl::colorYellow);
    else
        setMaterialColor(sim_colorcomponent_emission,1.0f,0.5f,0.3f);

    int pos[2]={p.x,p.y};
    int size[2]={s.x,s.y};

    ogl::buffer_i.clear();
    addBuffer2DPoints_i(pos[0]-size[0]/2,pos[1]-size[1]/2);
    addBuffer2DPoints_i(pos[0]-size[0]/2,pos[1]+size[1]/2);
    addBuffer2DPoints_i(pos[0]+size[0]/2,pos[1]+size[1]/2);
    addBuffer2DPoints_i(pos[0]+size[0]/2,pos[1]-size[1]/2);
    addBuffer2DPoints_i(pos[0]-size[0]/2,pos[1]-size[1]/2);
    ogl::drawRandom2dLines_i(&ogl::buffer_i[0],(int)ogl::buffer_i.size()/2,true,0);
    ogl::buffer_i.clear();
    glLineWidth(1.0f);
    glEnable(GL_DEPTH_TEST);
}

int ogl::getRichTextInfo(std::string& text,std::vector<int>& iconsAndPos)
{
    iconsAndPos.clear();
    int iconCount=0;
    size_t i=text.find("&&",0);
    while (i!=std::string::npos)
    {
        bool contin=true;
        if (contin&&(text.compare(i,4,"&&fg")==0))
        {
            iconsAndPos.push_back((int)i); // pos
            iconsAndPos.push_back(COLOR_AND_COLOR_RESET_RICH_TEXT); // exception: this indicates color change, or color reset (fb@@@)!
            iconsAndPos.push_back(text[i+4]-48); // rgb
            iconsAndPos.push_back(text[i+5]-48); // rgb
            iconsAndPos.push_back(text[i+6]-48); // rgb
            text.erase(i,7);
            iconCount--; // will be anyway increased further down (we compensate here)
            contin=false;
        }
        if (contin&&(text.compare(i,5,"&&Box")==0))
        {
            iconsAndPos.push_back((int)i); // pos
            iconsAndPos.push_back(BOX_RICH_TEXT); // icon ID
            text.erase(i,5);
            contin=false;
        }
        if (contin&&(text.compare(i,7,"&&Pause")==0))
        {
            iconsAndPos.push_back((int)i); // pos
            iconsAndPos.push_back(PAUSE_RICH_TEXT); // icon ID
            text.erase(i,7);
            contin=false;
        }
        if (contin&&(text.compare(i,7,"&&Start")==0))
        {
            iconsAndPos.push_back((int)i); // pos
            iconsAndPos.push_back(START_RICH_TEXT); // icon ID
            text.erase(i,7);
            contin=false;
        }
        if (contin&&(text.compare(i,5,"&&End")==0))
        {
            iconsAndPos.push_back((int)i); // pos
            iconsAndPos.push_back(END_RICH_TEXT); // icon ID
            text.erase(i,5);
            contin=false;
        }
        if (contin&&(text.compare(i,11,"&&ArrowLeft")==0))
        {
            iconsAndPos.push_back((int)i); // pos
            iconsAndPos.push_back(ARROW_LEFT_RICH_TEXT); // icon ID
            text.erase(i,11);
            contin=false;
        }
        if (contin&&(text.compare(i,12,"&&ArrowRight")==0))
        {
            iconsAndPos.push_back((int)i); // pos
            iconsAndPos.push_back(ARROW_RIGHT_RICH_TEXT); // icon ID
            text.erase(i,12);
            contin=false;
        }
        if (contin&&(text.compare(i,9,"&&ArrowUp")==0))
        {
            iconsAndPos.push_back((int)i); // pos
            iconsAndPos.push_back(ARROW_UP_RICH_TEXT); // icon ID
            text.erase(i,9);
            contin=false;
        }
        if (contin&&(text.compare(i,11,"&&ArrowDown")==0))
        {
            iconsAndPos.push_back((int)i); // pos
            iconsAndPos.push_back(ARROW_DOWN_RICH_TEXT); // icon ID
            text.erase(i,11);
            contin=false;
        }
        if (contin&&(text.compare(i,7,"&&Check")==0))
        {
            iconsAndPos.push_back((int)i); // pos
            iconsAndPos.push_back(CHECK_RICH_TEXT); // icon ID
            text.erase(i,7);
            contin=false;
        }
        if (contin&&(text.compare(i,8,"&&Square")==0))
        {
            iconsAndPos.push_back((int)i); // pos
            iconsAndPos.push_back(SQUARE_RICH_TEXT); // icon ID
            text.erase(i,8);
            contin=false;
        }
        if (contin&&(text.compare(i,10,"&&Minimize")==0))
        {
            iconsAndPos.push_back((int)i); // pos
            iconsAndPos.push_back(MINIMIZE_RICH_TEXT); // icon ID
            text.erase(i,10);
            contin=false;
        }
        if (contin)
            i+=2;
        else
            iconCount++;
        i=text.find("&&",i);
    }
    return(iconCount);
}

int ogl::getMultilineTextInfo(const std::string& text,std::vector<std::string>& lines,int* textMaxWidth,int* textHeight,int* charHeight)
{
    if (oglFonts[fontIndex].fontBase==0)
        return(0);
    int fontHeight=oglFonts[fontIndex].fontHeight;
    // How many lines?
    lines.clear();
    size_t i=text.find("&&n",0);
    int lastFoundPos=-3;

    int maxLineWidth=0;
    std::vector<int> iconsAndPos;
    while (i!=std::string::npos)
    {
        std::string lineTxt(text,lastFoundPos+3,i-(lastFoundPos+3));
        std::string t(lineTxt);
        int iconCount=getRichTextInfo(t,iconsAndPos);
        int tWidth=iconCount*fontHeight;
        for (int j=0;j<int(t.size());j++)
            tWidth+=oglFonts[fontIndex].fontWidths[int(t[j])];
        if (tWidth>maxLineWidth)
            maxLineWidth=tWidth;
        lines.push_back(lineTxt);
        lastFoundPos=(int)i;
        i=text.find("&&n",i+3);
    }
    std::string lineTxt;
    if (lastFoundPos<0)
        lineTxt=text; // Just one line
    else
        lineTxt=std::string(text,lastFoundPos+3,999999);
    std::string t(lineTxt);
    int iconCount=getRichTextInfo(t,iconsAndPos);
    int tWidth=iconCount*fontHeight;
    for (int j=0;j<int(t.size());j++)
        tWidth+=oglFonts[fontIndex].fontWidths[int(t[j])];
    if (tWidth>maxLineWidth)
        maxLineWidth=tWidth;
    lines.push_back(lineTxt);

    if (textMaxWidth!=nullptr)
        (*textMaxWidth)=maxLineWidth;
    if (textHeight!=nullptr)
        (*textHeight)=fontHeight*(int)lines.size();
    if (charHeight!=nullptr)
        (*charHeight)=fontHeight;
    return((int)lines.size());
}


void ogl::drawButton(VPoint p,VPoint s,float txtColor[3],float _bckgrndColor1[3],
                       float _bckgrndColor2[3],std::string text,int buttonTypeAndAttributes,
                       bool editing,int editionPos,float sliderPos,bool vertical,int timeInMs,float secondTextColor[3],
                       CTextureProperty* backgroundTexture,VPoint* backgroundPos,VPoint* backgroundSize,CTextureProperty* foregroundTexture)
{  // If sliderPos is >2.0, there's no slider. Else, sliderPos should be between -1.0 and 1.0
    // Before calling this function, make sure that viewSize==viewportSize!!
    // Make sure also that viewportPos is set correctly!!!
    // We work in screen pixel coordinates!
    // secondTextColor can be nullptr in wich case the text is only displayed once (no double)
//  float zeroCol[3]={0.0f,0.0f,0.0f};
//  ogl::setColor(zeroCol,zeroCol,zeroCol,zeroCol,12,0.5f);
    ogl::setAlpha(0.5f);
    float bckgrndColor1[3]={_bckgrndColor1[0],_bckgrndColor1[1],_bckgrndColor1[2]};
    float bckgrndColor2[3]={_bckgrndColor2[0],_bckgrndColor2[1],_bckgrndColor2[2]};
    if ( (buttonTypeAndAttributes&sim_buttonproperty_transparent)&&(backgroundTexture==nullptr)&&(foregroundTexture==nullptr) )
        ogl::setBlending(true,GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

    glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    if ((buttonTypeAndAttributes&0x0007)==sim_buttonproperty_button)
    {
        if (buttonTypeAndAttributes&sim_buttonproperty_isdown)
            setMaterialColor(sim_colorcomponent_emission,bckgrndColor2);
        else
            setMaterialColor(sim_colorcomponent_emission,bckgrndColor1);
    }
    if ( ((buttonTypeAndAttributes&0x0007)==sim_buttonproperty_label)||((buttonTypeAndAttributes&0x0007)==sim_buttonproperty_slider) )
        setMaterialColor(sim_colorcomponent_emission,bckgrndColor1);
    if ((buttonTypeAndAttributes&0x0007)==sim_buttonproperty_editbox)
    {
        if (buttonTypeAndAttributes&sim_buttonproperty_enabled)
        {
            if (editing)
            {
                if (editionPos==-1)
                    setMaterialColor(sim_colorcomponent_emission,ogl::colorBlue);
                else
                    setMaterialColor(sim_colorcomponent_emission,ogl::colorYellow);
            }
            else
                setMaterialColor(sim_colorcomponent_emission,bckgrndColor2);
        }
        else
            setMaterialColor(sim_colorcomponent_emission,bckgrndColor2[0]*0.6f,bckgrndColor2[1]*0.6f,bckgrndColor2[2]*0.6f);
    }

    int pos[2]={p.x,p.y};
    int size[2]={s.x+2,s.y+2};

    glEnable(GL_SCISSOR_TEST); // To avoid the label to pass the button's size
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT,viewport);
    glScissor(viewport[0]+p.x-(s.x/2)-1,viewport[1]+p.y-(s.y/2)-1,s.x+2,s.y+2);
    if ((buttonTypeAndAttributes&sim_buttonproperty_nobackgroundcolor)==0)
    {
        int a=pos[0]-size[0]/2;
        int b=pos[0]+size[0]/2;
        int c=pos[1]-size[1]/2;
        int d=pos[1]+size[1]/2;
        if ( ((backgroundTexture==nullptr)||((buttonTypeAndAttributes&sim_buttonproperty_transparent)==0))&&
            (foregroundTexture==nullptr) )
        {
            glBegin(GL_QUADS);
            glVertex3i(a,c,0);
            glVertex3i(b,c,0);
            glVertex3i(b,d,0);
            glVertex3i(a,d,0);
            glEnd();
        }
        if ((backgroundTexture!=nullptr)&&(buttonTypeAndAttributes&sim_buttonproperty_transparent))
        { // we have to display the background texture first
            float aq=float(a-backgroundPos->x)/float(backgroundSize->x);
            float bq=float(b-backgroundPos->x)/float(backgroundSize->x);
            float cq=1.0f-(float(backgroundPos->y-c)/float(backgroundSize->y));
            float dq=1.0f-(float(backgroundPos->y-d)/float(backgroundSize->y));
            _start2DTextureDisplay(backgroundTexture);
            glBegin(GL_QUADS);
            glTexCoord2f(aq,cq);
            glVertex3i(a,c,0);
            glTexCoord2f(bq,cq);
            glVertex3i(b,c,0);
            glTexCoord2f(bq,dq);
            glVertex3i(b,d,0);
            glTexCoord2f(aq,dq);
            glVertex3i(a,d,0);
            glEnd();
            _end2DTextureDisplay(backgroundTexture);
        }
        if (foregroundTexture!=nullptr)
        { // we have to display the foreground texture

            int off=0;
            if (buttonTypeAndAttributes&sim_buttonproperty_isdown)
                off=1;
            _start2DTextureDisplay(foregroundTexture);
            glBegin(GL_QUADS);
            glTexCoord2f(0.0f,0.0f);
            glVertex3i(a+off,c-off,0);
            glTexCoord2f(1.0f,0.0f);
            glVertex3i(b+off,c-off,0);
            glTexCoord2f(1.0f,1.0f);
            glVertex3i(b+off,d-off,0);
            glTexCoord2f(0.0f,1.0f);
            glVertex3i(a+off,d-off,0);
            glEnd();
            _end2DTextureDisplay(foregroundTexture);
        }
    }
    setBlending(false);
    if ( ((buttonTypeAndAttributes&0x0007)==sim_buttonproperty_button)&&((buttonTypeAndAttributes&sim_buttonproperty_borderless)==0)&&((buttonTypeAndAttributes&sim_buttonproperty_nobackgroundcolor)==0) )
    {
        glLineWidth(2.0f);
        if (buttonTypeAndAttributes&sim_buttonproperty_isdown)
            setMaterialColor(sim_colorcomponent_emission,bckgrndColor2[0]*0.66f,bckgrndColor2[1]*0.66f,bckgrndColor2[2]*0.66f);
        else
            setMaterialColor(sim_colorcomponent_emission,bckgrndColor1[0]*1.5f,bckgrndColor1[1]*1.5f,bckgrndColor1[2]*1.5f);
        glBegin(GL_LINE_STRIP);
        glVertex3i(pos[0]-size[0]/2,pos[1]-size[1]/2,0);
        glVertex3i(pos[0]-size[0]/2,pos[1]+size[1]/2,0);
        glVertex3i(pos[0]+size[0]/2,pos[1]+size[1]/2,0);
        glEnd();
        if (buttonTypeAndAttributes&sim_buttonproperty_isdown)
            setMaterialColor(sim_colorcomponent_emission,bckgrndColor2[0]*1.5f,bckgrndColor2[1]*1.5f,bckgrndColor2[2]*1.5f);
        else
            setMaterialColor(sim_colorcomponent_emission,bckgrndColor1[0]*0.66f,bckgrndColor1[1]*0.66f,bckgrndColor1[2]*0.66f);
        glBegin(GL_LINE_STRIP);
        glVertex3i(pos[0]+size[0]/2,pos[1]+size[1]/2,0);
        glVertex3i(pos[0]+size[0]/2,pos[1]-size[1]/2,0);
        glVertex3i(pos[0]-size[0]/2,pos[1]-size[1]/2,0);
        glEnd();
        glLineWidth(1.0f);
    }
    if ( ((buttonTypeAndAttributes&0x0007)==sim_buttonproperty_button)||((buttonTypeAndAttributes&0x0007)==sim_buttonproperty_label) )
    {
        float* secondTextCol=nullptr;
        float secondTextColRef[3]={0.0f,0.0f,0.0f};
        if (secondTextColor)
        {
            secondTextColRef[0]=secondTextColor[0];
            secondTextColRef[1]=secondTextColor[1];
            secondTextColRef[2]=secondTextColor[2];
            secondTextCol=secondTextColRef;
        }
        if (buttonTypeAndAttributes&sim_buttonproperty_enabled)
            setRichTextDefaultColor(txtColor);
        else
        {
            float disabledCol[3]={txtColor[0]-0.8f*(txtColor[0]-bckgrndColor1[0]),
                txtColor[1]-0.8f*(txtColor[1]-bckgrndColor1[1]),
                txtColor[2]-0.8f*(txtColor[2]-bckgrndColor1[2])};
            if (secondTextColor)
            {
                secondTextCol[0]=secondTextCol[0]*0.2f+bckgrndColor1[0]*0.8f;
                secondTextCol[1]=secondTextCol[1]*0.2f+bckgrndColor1[1]*0.8f;
                secondTextCol[2]=secondTextCol[2]*0.2f+bckgrndColor1[2]*0.8f;
            }
            setRichTextDefaultColor(disabledCol);
        }

        { // Normal text
            int fontHeight=10;
            if (oglFonts[fontIndex].fontBase!=0)
                fontHeight=oglFonts[fontIndex].fontHeight;
            // How many lines?
            std::vector<std::string> lines;
            size_t i=text.find("&&n",0);
            int lastFoundPos=-3;
            while (i!=std::string::npos)
            {
                lines.push_back(std::string(text,lastFoundPos+3,i-(lastFoundPos+3)));
                lastFoundPos=(int)i;
                i=text.find("&&n",i+3);
            }
            if (lastFoundPos==int(std::string::npos))
                lines.push_back(text); // Just one line
            else
                lines.push_back(std::string(text,lastFoundPos+3,999999));

            int xSt=pos[0];
            int ySt=pos[1];
            if (buttonTypeAndAttributes&sim_buttonproperty_verticallycentered)
                ySt+=((int)lines.size()-1)*fontHeight/2;
            else
                ySt+=s.y/2-fontHeight/2;
            if (buttonTypeAndAttributes&sim_buttonproperty_isdown)
            {
                xSt=xSt+1;
                ySt=ySt-1;
            }
            int alignment=0;
            if (buttonTypeAndAttributes&sim_buttonproperty_horizontallycentered)
                alignment=1;
            else
                xSt-=size[0]/2-2;
            // Following 3 lines added on 2009/05/15
            int ssi=s.y;//8*s.y/10;
            if (s.y>s.x)
                ssi=s.x;//8*s.x/10;
            if ((lines.size()>1)||((buttonTypeAndAttributes&sim_buttonproperty_horizontallycentered)==0)||((buttonTypeAndAttributes&sim_buttonproperty_verticallycentered)==0))
                ssi=oglFonts[fontIndex].fontHeight; // We have more than one line of text or the line is aligned at the top or left. We switch to small icons
            for (int i=0;i<int(lines.size());i++)
            {
                std::vector<int> iconInfo;
                getRichTextInfo(lines[i],iconInfo);
                if (buttonTypeAndAttributes&sim_buttonproperty_isdown)
                    drawRichText(xSt,ySt,ssi,lines[i],iconInfo,alignment,bckgrndColor2,secondTextCol);
                else
                    drawRichText(xSt,ySt,ssi,lines[i],iconInfo,alignment,bckgrndColor1,secondTextCol);
                ySt-=fontHeight;
            }
        }

    }
    if ((buttonTypeAndAttributes&0x0007)==sim_buttonproperty_slider)
    {   // Here we have to draw a slider. The slider button has to be 20% of the
        // button x-size and 80% of the button y-size (so that mouse-dragging
        // is correct)
        // First we draw the middle line:
        if (buttonTypeAndAttributes&sim_buttonproperty_enabled)
            setMaterialColor(sim_colorcomponent_emission,ogl::colorBlack);
        else
            setMaterialColor(sim_colorcomponent_emission,ogl::colorGrey);
        float xFact=2.0f/(1.0f-(float)BUTTON_SLIDER_X_SIZE);
        float yFact=20.0f;
        if (vertical)
        {
            xFact=20.0f;
            yFact=2.0f/(1.0f-(float)BUTTON_SLIDER_X_SIZE);
        }
        glBegin(GL_QUADS);
        glVertex3i(int(pos[0]-size[0]/xFact),int(pos[1]-size[1]/yFact),0);
        glVertex3i(int(pos[0]+size[0]/xFact),int(pos[1]-size[1]/yFact),0);
        glVertex3i(int(pos[0]+size[0]/xFact),int(pos[1]+size[1]/yFact),0);
        glVertex3i(int(pos[0]-size[0]/xFact),int(pos[1]+size[1]/yFact),0);
        glEnd();
        // Now the dragging button:
        float c=0.7f;
        if (buttonTypeAndAttributes&sim_buttonproperty_enabled)
            c=1.0f;
        setMaterialColor(sim_colorcomponent_emission,bckgrndColor1[0]*c,bckgrndColor1[1]*c,bckgrndColor1[2]*c);
        float specialPos=float(p.x);
        float specialSize=float(s.x+1);
        int dbs[2]={int(float(size[0])*BUTTON_SLIDER_X_SIZE/2.0f),int(float(size[1])*BUTTON_SLIDER_Y_SIZE/2.0f)};
        if (vertical)
        {
            specialPos=float(p.y);
            specialSize=float(s.y+1);
            pos[1]=int(specialPos-sliderPos*specialSize*(1.0f-BUTTON_SLIDER_X_SIZE)/2.0f);
            dbs[0]=int(float(size[0])*BUTTON_SLIDER_Y_SIZE/2.0f);
            dbs[1]=int(float(size[1])*BUTTON_SLIDER_X_SIZE/2.0f);
        }
        else
            pos[0]=int(specialPos+sliderPos*specialSize*(1.0f-BUTTON_SLIDER_X_SIZE)/2.0f);
        glBegin(GL_QUADS);
        glVertex3i(pos[0]-dbs[0],pos[1]-dbs[1],0);
        glVertex3i(pos[0]+dbs[0],pos[1]-dbs[1],0);
        glVertex3i(pos[0]+dbs[0],pos[1]+dbs[1],0);
        glVertex3i(pos[0]-dbs[0],pos[1]+dbs[1],0);
        glEnd();
        // Now the borders
        if ((buttonTypeAndAttributes&sim_buttonproperty_nobackgroundcolor)==0)
        {
            setMaterialColor(sim_colorcomponent_emission,bckgrndColor1[0]*1.5f,bckgrndColor1[1]*1.5f,bckgrndColor1[2]*1.5f);
            glBegin(GL_LINE_STRIP);
            glVertex3i(pos[0]-dbs[0],pos[1]-dbs[1],0);
            glVertex3i(pos[0]-dbs[0],pos[1]+dbs[1],0);
            glVertex3i(pos[0]+dbs[0],pos[1]+dbs[1],0);
            glEnd();
            setMaterialColor(sim_colorcomponent_emission,bckgrndColor1[0]*0.66f,bckgrndColor1[1]*0.66f,bckgrndColor1[2]*0.66f);
            glBegin(GL_LINE_STRIP);
            glVertex3i(pos[0]+dbs[0],pos[1]+dbs[1],0);
            glVertex3i(pos[0]+dbs[0],pos[1]-dbs[1],0);
            glVertex3i(pos[0]-dbs[0],pos[1]-dbs[1],0);
            glEnd();
        }
    }
    if ((buttonTypeAndAttributes&0x0007)==sim_buttonproperty_editbox)
    {   // Here we have to draw an edit-box:
        size[0]=s.x+2;
        size[1]=s.y+2;
        if (((buttonTypeAndAttributes&sim_buttonproperty_borderless)==0)&&((buttonTypeAndAttributes&sim_buttonproperty_nobackgroundcolor)==0))
        {
            glLineWidth(2.0f);
            setMaterialColor(sim_colorcomponent_emission,bckgrndColor1[0]*0.33f,bckgrndColor1[1]*0.33f,bckgrndColor1[2]*0.33f);
            glBegin(GL_LINE_STRIP);
            glVertex3i(pos[0]-size[0]/2,pos[1]-size[1]/2,0);
            glVertex3i(pos[0]-size[0]/2,pos[1]+size[1]/2,0);
            glVertex3i(pos[0]+size[0]/2,pos[1]+size[1]/2,0);
            glEnd();
            setMaterialColor(sim_colorcomponent_emission,bckgrndColor1[0]*1.1f,bckgrndColor1[1]*1.1f,bckgrndColor1[2]*1.1f);
            glBegin(GL_LINE_STRIP);
            glVertex3i(pos[0]+size[0]/2,pos[1]+size[1]/2,0);
            glVertex3i(pos[0]+size[0]/2,pos[1]-size[1]/2,0);
            glVertex3i(pos[0]-size[0]/2,pos[1]-size[1]/2,0);
            glEnd();
            glLineWidth(1.0f);
        }

        // Now the text-part:
        glScissor(viewport[0]+p.x-(s.x/2)+1,viewport[1]+p.y-(s.y/2)+1,s.x-3,s.y-2);

        int heightCorrection=int((0.3f*float(oglFonts[fontIndex].fontHeight))); // from 0.2 to 0.3 on 22/12/2011
        if (editing&&(editionPos==-1))
            setTextColor(ogl::colorWhite);
        else
            setTextColor(txtColor);
        size[0]=s.x-4;


        int width=0;
        for (int i=0;i<int(text.length());i++)
            width=width+oglFonts[fontIndex].fontWidths[int(text[i])];
        int xx=pos[0]-size[0]/2;
        if (buttonTypeAndAttributes&sim_buttonproperty_horizontallycentered)
            xx=pos[0]-width/2;
        if (editing)
        {
            xx=pos[0]+size[0]/2-width;
            if (xx>(pos[0]-size[0]/2))
                xx=pos[0]-size[0]/2;
        }
        drawText(xx,pos[1]-heightCorrection,0,text,false);

        // Now we have to draw the little cursor under the current editionPos:
        if (editing&&(editionPos!=-1))
        {
            width=0;
            for (int i=0;i<editionPos;i++)
                width=width+oglFonts[fontIndex].fontWidths[int(text[i])];
            setTextColor(ogl::colorRed);
            if (timeInMs&512)
                drawText(xx+width-1,pos[1]-heightCorrection,0,"|",false);
        }
    }
    glDisable(GL_SCISSOR_TEST);
    ogl::setBlending(false);
}

