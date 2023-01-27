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

#include "drawingObjectRendering.h"

#ifdef SIM_WITH_OPENGL

const double SPHEREVERTICES[24*3]={
-0.4142,-1.0000,-0.4142,
+0.4142,-1.0000,-0.4142,
+0.4142,-1.0000,+0.4142,
-0.4142,-1.0000,+0.4142,

-0.4142,-0.4142,-1.0000,
-0.4142,+0.4142,-1.0000,
+0.4142,+0.4142,-1.0000,
+0.4142,-0.4142,-1.0000,

+0.4142,+1.0000,-0.4142,
-0.4142,+1.0000,-0.4142,
-0.4142,+1.0000,+0.4142,
+0.4142,+1.0000,+0.4142,

+0.4142,-0.4142,+1.0000,
+0.4142,+0.4142,+1.0000,
-0.4142,+0.4142,+1.0000,
-0.4142,-0.4142,+1.0000,

+1.0000,-0.4142,-0.4142,
+1.0000,+0.4142,-0.4142,
+1.0000,+0.4142,+0.4142,
+1.0000,-0.4142,+0.4142,

-1.0000,-0.4142,+0.4142,
-1.0000,+0.4142,+0.4142,
-1.0000,+0.4142,-0.4142,
-1.0000,-0.4142,-0.4142
};

const int SPHEREQUADINDICES[18*4]={
0,1,2,3,
4,5,6,7,
8,9,10,11,
12,13,14,15,
16,17,18,19,
20,21,22,23,

0,4,7,1,
1,16,19,2,
3,2,12,15,
0,3,20,23,

6,5,9,8,
18,17,8,11,
14,13,11,10,
10,9,22,21,

7,6,17,16,
19,18,13,12,
15,14,21,20,
23,22,5,4
};

const double SPHEREQUADNORMALS[18*3]={
+0.0,-1.0,+0.0,
+0.0,+0.0,-1.0,
+0.0,+1.0,+0.0,
+0.0,+0.0,+1.0,
+1.0,+0.0,+0.0,
-1.0,+0.0,+0.0,

+0.0,-0.7071,-0.7071,
+0.7071,-0.7071,+0.0,
+0.0,-0.7071,+0.7071,
-0.7071,-0.7071,+0.0,

+0.0,+0.7071,-0.7071,
+0.7071,+0.7071,+0.0,
+0.0,+0.7071,+0.7071,
-0.7071,+0.7071,+0.0,

+0.7071,0.0,-0.7071,
+0.7071,0.0,+0.7071,
-0.7071,0.0,+0.7071,
-0.7071,0.0,-0.7071
};

const int SPHERETRIANGLEINDICES[8*3]={
1,7,16,
2,19,12,
3,15,20,
0,23,4,

17,6,8,
18,11,13,
14,10,21,
22,9,5
};

const double SPHERETRIANGLENORMALS[8*3]={
+0.5773,-0.5773,-0.5773,
+0.5773,-0.5773,+0.5773,
-0.5773,-0.5773,+0.5773,
-0.5773,-0.5773,-0.5773,

+0.5773,+0.5773,-0.5773,
+0.5773,+0.5773,+0.5773,
-0.5773,+0.5773,+0.5773,
-0.5773,+0.5773,-0.5773,
};

void displayDrawingObject(CDrawingObject* drawingObject,C7Vector& tr,bool overlay,bool transparentObject,int displayAttrib,const C4X4Matrix& cameraCTM)
{
    int _objectType=drawingObject->getObjectType();
    C3Vector normalVectorForLinesAndPoints(tr.Q.getInverse()*C3Vector::unitZVector);

    glPushMatrix();
    glPushAttrib(GL_POLYGON_BIT);
    glLoadName(-1);
    drawingObject->color.makeCurrentColor((displayAttrib&sim_displayattribute_useauxcomponent)!=0);
    C4X4Matrix cameraRTM(cameraCTM);
    if (drawingObject->getSceneObjectId()>=0)
    {
        glTranslated(tr.X(0),tr.X(1),tr.X(2));
        C4Vector axis=tr.Q.getAngleAndAxis();
        glRotated(axis(0)*radToDeg,axis(1),axis(2),axis(3));
        cameraRTM.setMultResult(tr.getInverse().getMatrix(),cameraCTM);
    }

    bool wire=( (drawingObject->getObjectType()&sim_drawing_wireframe)||(displayAttrib&sim_displayattribute_forcewireframe))&&((displayAttrib&sim_displayattribute_depthpass)==0);
    if (displayAttrib&sim_displayattribute_forbidwireframe)
        wire=false;
    if (wire)
        glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
    if ((_objectType&sim_drawing_backfaceculling)&&(!wire))
        glEnable(GL_CULL_FACE);

    if (overlay)
        glDisable(GL_DEPTH_TEST);


    int tmp=_objectType&0x001f;
    if (tmp==sim_drawing_points)
        _drawPoints(drawingObject,displayAttrib,cameraRTM,normalVectorForLinesAndPoints.data);
    if (tmp==sim_drawing_lines)
        _drawLines(drawingObject,displayAttrib,cameraRTM,normalVectorForLinesAndPoints.data);
    if (tmp==sim_drawing_linestrip)
        _drawLineStrip(drawingObject,displayAttrib,cameraRTM,normalVectorForLinesAndPoints.data);
    if (tmp==sim_drawing_triangles)
        _drawTriangles(drawingObject,displayAttrib);
    if ( (tmp==sim_drawing_trianglepoints)||(tmp==sim_drawing_trianglepts) )
        _drawTrianglePoints(drawingObject,displayAttrib,cameraRTM);
    if ( (tmp==sim_drawing_quadpoints)||(tmp==sim_drawing_quadpts) )
        _drawQuadPoints(drawingObject,displayAttrib,cameraRTM);
    if ( (tmp==sim_drawing_discpoints)||(tmp==sim_drawing_discpts) )
        _drawDiscPoints(drawingObject,displayAttrib,cameraRTM);
    if ( (tmp==sim_drawing_cubepoints)||(tmp==sim_drawing_cubepts) )
        _drawCubePoints(drawingObject,displayAttrib,cameraRTM);
    if (tmp==sim_drawing_spherepts)
        _drawSpherePoints(drawingObject,displayAttrib);

    if (overlay)
        glEnable(GL_DEPTH_TEST);


    glDisable(GL_CULL_FACE);
    glPopAttrib();
    glPopMatrix();

    if (transparentObject)
        ogl::setBlending(false);
}

void _drawPoints(CDrawingObject* drawingObject,int displayAttrib,const C4X4Matrix& cameraRTM,const double normalVectorForLinesAndPoints[3])
{
    bool auxCmp=(displayAttrib&sim_displayattribute_useauxcomponent)!=0;
    C3Vector n(cameraRTM.M.axis[2]*-1.0);
    int _objectType=drawingObject->getObjectType();
    double _size=drawingObject->getSize();
    int _maxItemCount=drawingObject->getMaxItemCount();
    int _startItem=drawingObject->getStartItem();
    std::vector<double>& _data=drawingObject->getDataPtr()[0];
    // Following 2 new since introduction of sim_drawing_itemtransparency:
    if ( (_objectType&sim_drawing_itemtransparency)&&(!auxCmp) )
        ogl::setBlending(true,GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA); // We turn blending on!

    if (_objectType&sim_drawing_itemsizes)
    { // This is a special routine in case we want various point sizes (change not possible between glBegin and glEnd)
        C3Vector v;

        ogl::buffer.clear();
        std::vector<float> glCols;
        std::vector<double> glSizes;
        bool cols=false;
        for (int i=0;i<int(_data.size())/drawingObject->floatsPerItem;i++)
        {
            int off=0;
            int p=_startItem+i;
            if (p>=_maxItemCount)
                p-=_maxItemCount;
            if ( (_objectType&sim_drawing_itemcolors) && ((!auxCmp)||(_objectType&sim_drawing_auxchannelcolor2)) )
            {
                glCols.push_back(_data[drawingObject->floatsPerItem*p+3+0]);
                glCols.push_back(_data[drawingObject->floatsPerItem*p+3+1]);
                glCols.push_back(_data[drawingObject->floatsPerItem*p+3+2]);
                off+=3;
                cols=true;
            }

            glSizes.push_back(_data[drawingObject->floatsPerItem*p+3+off]);
            off++;

            if (_objectType&sim_drawing_itemtransparency)
            {
                if (!cols)
                {
                    glCols.push_back(drawingObject->color.getColorsPtr()[0]);
                    glCols.push_back(drawingObject->color.getColorsPtr()[1]);
                    glCols.push_back(drawingObject->color.getColorsPtr()[2]);
                }
                glCols.push_back(1.0-_data[drawingObject->floatsPerItem*p+3+off]);
                off++;
            }
            else
            {
                if (cols)
                    glCols.push_back(1.0);
            }

            v.setData(&_data[drawingObject->floatsPerItem*p+0]);
            ogl::addBuffer3DPoints(v.data);
        }
        if (ogl::buffer.size()!=0)
        {
            if (glCols.size()==0)
                ogl::drawRandom3dPointsEx(&ogl::buffer[0],(int)ogl::buffer.size()/3,nullptr,nullptr,&glSizes[0],_objectType&(sim_drawing_emissioncolor|sim_drawing_auxchannelcolor2),normalVectorForLinesAndPoints);
            else
                ogl::drawRandom3dPointsEx(&ogl::buffer[0],(int)ogl::buffer.size()/3,nullptr,&glCols[0],&glSizes[0],_objectType&(sim_drawing_emissioncolor|sim_drawing_auxchannelcolor2),normalVectorForLinesAndPoints);
        }
        ogl::buffer.clear();
        glPointSize(1.0);
    }
    else
    { // fixed point size
        glPointSize((float)_size);

        ogl::buffer.clear();
        std::vector<float> glCols;

        C3Vector v;
        bool cols=false;
        for (int i=0;i<int(_data.size())/drawingObject->floatsPerItem;i++)
        {
            int off=0;
            int p=_startItem+i;
            if (p>=_maxItemCount)
                p-=_maxItemCount;
            v.setData(&_data[drawingObject->floatsPerItem*p+0]);
            if ( (_objectType&sim_drawing_itemcolors) && ((!auxCmp)||(_objectType&sim_drawing_auxchannelcolor2)) )
            {
                glCols.push_back(_data[drawingObject->floatsPerItem*p+3+0]);
                glCols.push_back(_data[drawingObject->floatsPerItem*p+3+1]);
                glCols.push_back(_data[drawingObject->floatsPerItem*p+3+2]);
                cols=true;
                off+=3;
            }
            if (_objectType&sim_drawing_itemtransparency)
            {
                if (!cols)
                {
                    glCols.push_back(drawingObject->color.getColorsPtr()[0]);
                    glCols.push_back(drawingObject->color.getColorsPtr()[1]);
                    glCols.push_back(drawingObject->color.getColorsPtr()[2]);
                }
                glCols.push_back(1.0-_data[drawingObject->floatsPerItem*p+3+off]);
                off++;
            }
            else
            {
                if (cols)
                    glCols.push_back(1.0);
            }

            ogl::addBuffer3DPoints(v.data);
        }
        if (ogl::buffer.size()!=0)
        {
            if (glCols.size()==0)
                ogl::drawRandom3dPointsEx(&ogl::buffer[0],(int)ogl::buffer.size()/3,nullptr,nullptr,nullptr,_objectType&(sim_drawing_emissioncolor|sim_drawing_auxchannelcolor2),normalVectorForLinesAndPoints);
            else
                ogl::drawRandom3dPointsEx(&ogl::buffer[0],(int)ogl::buffer.size()/3,nullptr,&glCols[0],nullptr,_objectType&(sim_drawing_emissioncolor|sim_drawing_auxchannelcolor2),normalVectorForLinesAndPoints);
        }
        ogl::buffer.clear();
        glPointSize(1.0);
    }

    // Following 2 new since introduction of sim_drawing_itemtransparency:
    if ( (_objectType&sim_drawing_itemtransparency)&&(!auxCmp) )
        ogl::setBlending(false); // make sure we turn blending off!
}

void _drawTrianglePoints(CDrawingObject* drawingObject,int displayAttrib,const C4X4Matrix& cameraRTM)
{
    bool auxCmp=(displayAttrib&sim_displayattribute_useauxcomponent)!=0;
    int _objectType=drawingObject->getObjectType();
    double _size=drawingObject->getSize();
    int _maxItemCount=drawingObject->getMaxItemCount();
    int _startItem=drawingObject->getStartItem();
    std::vector<double>& _data=drawingObject->getDataPtr()[0];
    // Following 2 new since introduction of sim_drawing_itemtransparency:
    if ( (_objectType&sim_drawing_itemtransparency)&&(!auxCmp) )
        ogl::setBlending(true,GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA); // We turn blending on!

    C3Vector v,n,x,y;
    double s=_size;
    double sxc=0.866;
    double syc=0.5;
    for (int i=0;i<int(_data.size())/drawingObject->floatsPerItem;i++)
    {
        int off=0;
        int p=_startItem+i;
        if (p>=_maxItemCount)
            p-=_maxItemCount;
        v.setData(&_data[drawingObject->floatsPerItem*p+off]);
        off+=3;
        if (_objectType&sim_drawing_facingcamera)
        {
            n=cameraRTM.M.axis[2]*-1.0;
            if (n(2)<0.8)
                x=(n^C3Vector::unitZVector).getNormalized();
            else
                x=(n^C3Vector::unitXVector).getNormalized();
            y=n^x;
        }
        else
        {
            C4Vector q(&_data[0]+drawingObject->floatsPerItem*p+off);
            C3X3Matrix m=q.getMatrix();
            x=m.axis[0];
            y=m.axis[1];
            n=m.axis[2];
            off+=4;
        }
        if ( (_objectType&sim_drawing_itemcolors) && ((!auxCmp)||(_objectType&sim_drawing_auxchannelcolor2)) )
        {
            float ab[3]={(float)_data[drawingObject->floatsPerItem*p+off+0],(float)_data[drawingObject->floatsPerItem*p+off+1],(float)_data[drawingObject->floatsPerItem*p+off+2]};
            if (_objectType&(sim_drawing_emissioncolor|sim_drawing_auxchannelcolor2))
                ogl::setMaterialColor(sim_colorcomponent_emission,ab);
            else
                ogl::setMaterialColor(sim_colorcomponent_ambient_diffuse,ab);
        }
        if (_objectType&sim_drawing_itemcolors)
            off+=3;
        if (_objectType&sim_drawing_itemsizes)
        {
            s=_data[drawingObject->floatsPerItem*p+off];
            off++;
        }

        if ( (_objectType&sim_drawing_itemtransparency)&&(!auxCmp) )
        { // Following part new since introduction of sim_drawing_itemtransparency:
            ogl::setAlpha(1.0-_data[drawingObject->floatsPerItem*p+off]);
            off++;
        }

        glBegin(GL_TRIANGLES);
        glNormal3dv(n.data);
        glVertex3dv((v+(y*s)).data);
        glVertex3dv((v-(x*(s*sxc))-(y*(s*syc))).data);
        glVertex3dv((v+(x*(s*sxc))-(y*(s*syc))).data);
        glEnd();
    }

    // Following 2 new since introduction of sim_drawing_itemtransparency:
    if ( (_objectType&sim_drawing_itemtransparency)&&(!auxCmp) )
        ogl::setBlending(false); // make sure we turn blending off!
}

void _drawQuadPoints(CDrawingObject* drawingObject,int displayAttrib,const C4X4Matrix& cameraRTM)
{
    bool auxCmp=(displayAttrib&sim_displayattribute_useauxcomponent)!=0;
    int _objectType=drawingObject->getObjectType();
    double _size=drawingObject->getSize();
    int _maxItemCount=drawingObject->getMaxItemCount();
    int _startItem=drawingObject->getStartItem();
    std::vector<double>& _data=drawingObject->getDataPtr()[0];
    // Following 2 new since introduction of sim_drawing_itemtransparency:
    if ( (_objectType&sim_drawing_itemtransparency)&&(!auxCmp) )
        ogl::setBlending(true,GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA); // We turn blending on!

    C3Vector v,n,x,y;
    double s=_size;
    for (int i=0;i<int(_data.size())/drawingObject->floatsPerItem;i++)
    {
        int off=0;
        int p=_startItem+i;
        if (p>=_maxItemCount)
            p-=_maxItemCount;
        v.setData(&_data[drawingObject->floatsPerItem*p+off]);
        off+=3;
        if (_objectType&sim_drawing_facingcamera)
        {
            n=cameraRTM.M.axis[2]*-1.0;
            if (n(2)<0.8)
                x=(n^C3Vector::unitZVector).getNormalized();
            else
                x=(n^C3Vector::unitXVector).getNormalized();
            y=n^x;
        }
        else
        {
            C4Vector q(&_data[0]+drawingObject->floatsPerItem*p+off);
            C3X3Matrix m=q.getMatrix();
            x=m.axis[0];
            y=m.axis[1];
            n=m.axis[2];
            off+=4;
        }
        if ( (_objectType&sim_drawing_itemcolors) && ((!auxCmp)||(_objectType&sim_drawing_auxchannelcolor2)) )
        {
            float ab[3]={(float)_data[drawingObject->floatsPerItem*p+off+0],(float)_data[drawingObject->floatsPerItem*p+off+1],(float)_data[drawingObject->floatsPerItem*p+off+2]};
            if (_objectType&(sim_drawing_emissioncolor|sim_drawing_auxchannelcolor2))
                ogl::setMaterialColor(sim_colorcomponent_emission,ab);
            else
                ogl::setMaterialColor(sim_colorcomponent_ambient_diffuse,ab);
        }
        if (_objectType&sim_drawing_itemcolors)
            off+=3;
        if (_objectType&sim_drawing_itemsizes)
        {
            s=_data[drawingObject->floatsPerItem*p+off];
            off++;
        }

        if ( (_objectType&sim_drawing_itemtransparency)&&(!auxCmp) )
        { // Following part new since introduction of sim_drawing_itemtransparency:
            ogl::setAlpha(1.0-_data[drawingObject->floatsPerItem*p+off]);
            off++;
        }

        glBegin(GL_QUADS);
        glNormal3dv(n.data);
        glVertex3dv((v+(y*s)+(x*s)).data);
        glVertex3dv((v+(y*s)-(x*s)).data);
        glVertex3dv((v-(y*s)-(x*s)).data);
        glVertex3dv((v-(y*s)+(x*s)).data);
        glEnd();
    }

    // Following 2 new since introduction of sim_drawing_itemtransparency:
    if ( (_objectType&sim_drawing_itemtransparency)&&(!auxCmp) )
        ogl::setBlending(false); // make sure we turn blending off!
}

void _drawDiscPoints(CDrawingObject* drawingObject,int displayAttrib,const C4X4Matrix& cameraRTM)
{
    bool auxCmp=(displayAttrib&sim_displayattribute_useauxcomponent)!=0;
    int _objectType=drawingObject->getObjectType();
    double _size=drawingObject->getSize();
    int _maxItemCount=drawingObject->getMaxItemCount();
    int _startItem=drawingObject->getStartItem();
    std::vector<double>& _data=drawingObject->getDataPtr()[0];
    // Following 2 new since introduction of sim_drawing_itemtransparency:
    if ( (_objectType&sim_drawing_itemtransparency)&&(!auxCmp) )
        ogl::setBlending(true,GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA); // We turn blending on!

    C3Vector v,n,x,y;
    double s=_size;
    double ssc=0.4142;
    for (int i=0;i<int(_data.size())/drawingObject->floatsPerItem;i++)
    {
        int off=0;
        int p=_startItem+i;
        if (p>=_maxItemCount)
            p-=_maxItemCount;
        v.setData(&_data[drawingObject->floatsPerItem*p+off]);
        off+=3;
        if (_objectType&sim_drawing_facingcamera)
        {
            n=cameraRTM.M.axis[2]*-1.0;
            if (n(2)<0.8)
                x=(n^C3Vector::unitZVector).getNormalized();
            else
                x=(n^C3Vector::unitXVector).getNormalized();
            y=n^x;
        }
        else
        {
            C4Vector q(&_data[0]+drawingObject->floatsPerItem*p+off);
            C3X3Matrix m=q.getMatrix();
            x=m.axis[0];
            y=m.axis[1];
            n=m.axis[2];
            off+=4;
        }
        if ( (_objectType&sim_drawing_itemcolors) && ((!auxCmp)||(_objectType&sim_drawing_auxchannelcolor2)) )
        {
            float ab[3]={(float)_data[drawingObject->floatsPerItem*p+off+0],(float)_data[drawingObject->floatsPerItem*p+off+1],(float)_data[drawingObject->floatsPerItem*p+off+2]};
            if (_objectType&(sim_drawing_emissioncolor|sim_drawing_auxchannelcolor2))
                ogl::setMaterialColor(sim_colorcomponent_emission,ab);
            else
                ogl::setMaterialColor(sim_colorcomponent_ambient_diffuse,ab);
        }
        if (_objectType&sim_drawing_itemcolors)
            off+=3;
        if (_objectType&sim_drawing_itemsizes)
        {
            s=_data[drawingObject->floatsPerItem*p+off];
            off++;
        }

        if ( (_objectType&sim_drawing_itemtransparency)&&(!auxCmp) )
        { // Following part new since introduction of sim_drawing_itemtransparency:
            ogl::setAlpha(1.0-_data[drawingObject->floatsPerItem*p+off]);
            off++;
        }

        glBegin(GL_TRIANGLE_FAN); // GL_POLYGON is problematic on certain graphic cards!
        glNormal3dv(n.data);
        glVertex3dv((v+(x*s)+(y*(s*ssc))).data);
        glVertex3dv((v+(x*(s*ssc))+(y*s)).data);
        glVertex3dv((v-(x*(s*ssc))+(y*s)).data);
        glVertex3dv((v-(x*s)+(y*(s*ssc))).data);
        glVertex3dv((v-(x*s)-(y*(s*ssc))).data);
        glVertex3dv((v-(x*(s*ssc))-(y*s)).data);
        glVertex3dv((v+(x*(s*ssc))-(y*s)).data);
        glVertex3dv((v+(x*s)-(y*(s*ssc))).data);
        glEnd();
    }

    // Following 2 new since introduction of sim_drawing_itemtransparency:
    if ( (_objectType&sim_drawing_itemtransparency)&&(!auxCmp) )
        ogl::setBlending(false); // make sure we turn blending off!
}

void _drawCubePoints(CDrawingObject* drawingObject,int displayAttrib,const C4X4Matrix& cameraRTM)
{
    bool auxCmp=(displayAttrib&sim_displayattribute_useauxcomponent)!=0;
    int _objectType=drawingObject->getObjectType();
    double _size=drawingObject->getSize();
    int _maxItemCount=drawingObject->getMaxItemCount();
    int _startItem=drawingObject->getStartItem();
    std::vector<double>& _data=drawingObject->getDataPtr()[0];
    // Following 2 new since introduction of sim_drawing_itemtransparency:
    if ( (_objectType&sim_drawing_itemtransparency)&&(!auxCmp) )
        ogl::setBlending(true,GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA); // We turn blending on!

    C3Vector v,n,x,y,x0,y0,z0;
    double s=_size;
    for (int i=0;i<int(_data.size())/drawingObject->floatsPerItem;i++)
    {
        int off=0;
        int p=_startItem+i;
        if (p>=_maxItemCount)
            p-=_maxItemCount;
        v.setData(&_data[drawingObject->floatsPerItem*p+off]);
        off+=3;
        if (_objectType&sim_drawing_facingcamera)
        {
            n=cameraRTM.M.axis[2]*-1.0;
            if (n(2)<0.8)
                x=(n^C3Vector::unitZVector).getNormalized();
            else
                x=(n^C3Vector::unitXVector).getNormalized();
            y=n^x;
        }
        else
        {
            C4Vector q(&_data[0]+drawingObject->floatsPerItem*p+off);
            C3X3Matrix m=q.getMatrix();
            x=m.axis[0];
            y=m.axis[1];
            n=m.axis[2];
            off+=4;
        }
        if ( (_objectType&sim_drawing_itemcolors) && ((!auxCmp)||(_objectType&sim_drawing_auxchannelcolor2)) )
        {
            float ab[3]={(float)_data[drawingObject->floatsPerItem*p+off+0],(float)_data[drawingObject->floatsPerItem*p+off+1],(float)_data[drawingObject->floatsPerItem*p+off+2]};
            if (_objectType&(sim_drawing_emissioncolor|sim_drawing_auxchannelcolor2))
                ogl::setMaterialColor(sim_colorcomponent_emission,ab);
            else
                ogl::setMaterialColor(sim_colorcomponent_ambient_diffuse,ab);
        }
        if (_objectType&sim_drawing_itemcolors)
            off+=3;
        if (_objectType&sim_drawing_itemsizes)
        {
            s=_data[drawingObject->floatsPerItem*p+off];
            off++;
        }

        if ( (_objectType&sim_drawing_itemtransparency)&&(!auxCmp) )
        { // Following part new since introduction of sim_drawing_itemtransparency:
            ogl::setAlpha(1.0-_data[drawingObject->floatsPerItem*p+off]);
            off++;
        }

        x0=x*s;
        y0=y*s;
        z0=n*s;
        glBegin(GL_QUADS);
        glNormal3dv(x.data);
        glVertex3dv((v+x0+y0+z0).data);
        glVertex3dv((v+x0-y0+z0).data);
        glVertex3dv((v+x0-y0-z0).data);
        glVertex3dv((v+x0+y0-z0).data);

        glNormal3dv(y.data);
        glVertex3dv((v+x0+y0+z0).data);
        glVertex3dv((v+x0+y0-z0).data);
        glVertex3dv((v-x0+y0-z0).data);
        glVertex3dv((v-x0+y0+z0).data);

        glNormal3dv((x*-1.0).data);
        glVertex3dv((v-x0-y0+z0).data);
        glVertex3dv((v-x0+y0+z0).data);
        glVertex3dv((v-x0+y0-z0).data);
        glVertex3dv((v-x0-y0-z0).data);

        glNormal3dv((y*-1.0).data);
        glVertex3dv((v-x0-y0+z0).data);
        glVertex3dv((v-x0-y0-z0).data);
        glVertex3dv((v+x0-y0-z0).data);
        glVertex3dv((v+x0-y0+z0).data);

        glNormal3dv(n.data);
        glVertex3dv((v+x0+y0+z0).data);
        glVertex3dv((v-x0+y0+z0).data);
        glVertex3dv((v-x0-y0+z0).data);
        glVertex3dv((v+x0-y0+z0).data);

        glNormal3dv((n*-1.0).data);
        glVertex3dv((v-x0-y0-z0).data);
        glVertex3dv((v-x0+y0-z0).data);
        glVertex3dv((v+x0+y0-z0).data);
        glVertex3dv((v+x0-y0-z0).data);

        glEnd();
    }

    // Following 2 new since introduction of sim_drawing_itemtransparency:
    if ( (_objectType&sim_drawing_itemtransparency)&&(!auxCmp) )
        ogl::setBlending(false); // make sure we turn blending off!
}

void _drawSpherePoints(CDrawingObject* drawingObject,int displayAttrib)
{
    bool auxCmp=(displayAttrib&sim_displayattribute_useauxcomponent)!=0;
    int _objectType=drawingObject->getObjectType();
    double _size=drawingObject->getSize();
    int _maxItemCount=drawingObject->getMaxItemCount();
    int _startItem=drawingObject->getStartItem();
    std::vector<double>& _data=drawingObject->getDataPtr()[0];
    // Following 2 new since introduction of sim_drawing_itemtransparency:
    if ( (_objectType&sim_drawing_itemtransparency)&&(!auxCmp) )
        ogl::setBlending(true,GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA); // We turn blending on!

    C3Vector v;
    double s=_size;
    for (int i=0;i<int(_data.size())/drawingObject->floatsPerItem;i++)
    {
        int off=0;
        int p=_startItem+i;
        if (p>=_maxItemCount)
            p-=_maxItemCount;
        v.setData(&_data[drawingObject->floatsPerItem*p+0]);
        if ( (_objectType&sim_drawing_itemcolors) && ((!auxCmp)||(_objectType&sim_drawing_auxchannelcolor2)) )
        {
            float ab[3]={(float)_data[drawingObject->floatsPerItem*p+3],(float)_data[drawingObject->floatsPerItem*p+4],(float)_data[drawingObject->floatsPerItem*p+5]};
            if (_objectType&(sim_drawing_emissioncolor|sim_drawing_auxchannelcolor2))
                ogl::setMaterialColor(sim_colorcomponent_emission,ab);
            else
                ogl::setMaterialColor(sim_colorcomponent_ambient_diffuse,ab);
        }
        if (_objectType&sim_drawing_itemcolors)
            off+=3;
        if (_objectType&sim_drawing_itemsizes)
        {
            s=_data[drawingObject->floatsPerItem*p+3+off];
            off++;
        }

        if ( (_objectType&sim_drawing_itemtransparency)&&(!auxCmp) )
        { // Following part new since introduction of sim_drawing_itemtransparency:
            ogl::setAlpha(1.0-_data[drawingObject->floatsPerItem*p+3+off]);
            off++;
        }

        glBegin(GL_QUADS);
        for (int j=0;j<18;j++)
        {
            C3Vector x0(SPHEREVERTICES+3*SPHEREQUADINDICES[4*j+0]);
            C3Vector x1(SPHEREVERTICES+3*SPHEREQUADINDICES[4*j+1]);
            C3Vector x2(SPHEREVERTICES+3*SPHEREQUADINDICES[4*j+2]);
            C3Vector x3(SPHEREVERTICES+3*SPHEREQUADINDICES[4*j+3]);
            x0=v+x0*s;
            x1=v+x1*s;
            x2=v+x2*s;
            x3=v+x3*s;
            glNormal3dv(SPHEREQUADNORMALS+3*j);
            glVertex3dv(x0.data);
            glVertex3dv(x1.data);
            glVertex3dv(x2.data);
            glVertex3dv(x3.data);
        }
        glEnd();
        glBegin(GL_TRIANGLES);
        for (int j=0;j<8;j++)
        {
            C3Vector x0(SPHEREVERTICES+3*SPHERETRIANGLEINDICES[3*j+0]);
            C3Vector x1(SPHEREVERTICES+3*SPHERETRIANGLEINDICES[3*j+1]);
            C3Vector x2(SPHEREVERTICES+3*SPHERETRIANGLEINDICES[3*j+2]);
            x0=v+x0*s;
            x1=v+x1*s;
            x2=v+x2*s;
            glNormal3dv(SPHERETRIANGLENORMALS+3*j);
            glVertex3dv(x0.data);
            glVertex3dv(x1.data);
            glVertex3dv(x2.data);
        }
        glEnd();
    }

    // Following 2 new since introduction of sim_drawing_itemtransparency:
    if ( (_objectType&sim_drawing_itemtransparency)&&(!auxCmp) )
        ogl::setBlending(false); // make sure we turn blending off!
}

void _drawLines(CDrawingObject* drawingObject,int displayAttrib,const C4X4Matrix& cameraRTM,const double normalVectorForLinesAndPoints[3])
{
    bool auxCmp=(displayAttrib&sim_displayattribute_useauxcomponent)!=0;
    int _objectType=drawingObject->getObjectType();
    double _size=drawingObject->getSize();
    int _maxItemCount=drawingObject->getMaxItemCount();
    int _startItem=drawingObject->getStartItem();
    std::vector<double>& _data=drawingObject->getDataPtr()[0];
    // Following 2 new since introduction of sim_drawing_itemtransparency:
    if ( (_objectType&sim_drawing_itemtransparency)&&(!auxCmp) )
        ogl::setBlending(true,GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA); // We turn blending on!

    C3Vector n(cameraRTM.M.axis[2]*-1.0);

    if (_objectType&sim_drawing_itemsizes)
    { // This is a special routine in case we want various line sizes (change not possible between glBegin and glEnd)
        float rgbaAmb[3]={0.0,0.0,0.0};
        float rgbaAmb2[3]={0.0,0.0,0.0};
        int off=0;
        for (int i=0;i<int(_data.size())/drawingObject->floatsPerItem;i++)
        {
            int p=_startItem+i;
            if (p>=_maxItemCount)
                p-=_maxItemCount;
            if ( (_objectType&(sim_drawing_itemcolors|sim_drawing_vertexcolors)) && ((!auxCmp)||(_objectType&sim_drawing_auxchannelcolor2)) )
            {
                rgbaAmb[0]=_data[drawingObject->floatsPerItem*p+6];
                rgbaAmb[1]=_data[drawingObject->floatsPerItem*p+7];
                rgbaAmb[2]=_data[drawingObject->floatsPerItem*p+8];
                if (_objectType&(sim_drawing_emissioncolor|sim_drawing_auxchannelcolor2))
                    ogl::setMaterialColor(sim_colorcomponent_emission,rgbaAmb);
                else
                    ogl::setMaterialColor(sim_colorcomponent_ambient_diffuse,rgbaAmb);
            }
            if (_objectType&(sim_drawing_itemcolors|sim_drawing_vertexcolors))
                off+=3;
            if (_objectType&sim_drawing_vertexcolors)
            {
                rgbaAmb2[0]=_data[drawingObject->floatsPerItem*p+6+off];
                rgbaAmb2[1]=_data[drawingObject->floatsPerItem*p+7+off];
                rgbaAmb2[2]=_data[drawingObject->floatsPerItem*p+8+off];
                off+=3;
            }
            glLineWidth((float)_data[drawingObject->floatsPerItem*p+6+off]);
            double alpha=1.0;
            if ( (_objectType&sim_drawing_itemtransparency)&&(!auxCmp) )
            {
                alpha=1.0-_data[drawingObject->floatsPerItem*p+6+off+1];
                ogl::setAlpha(alpha);
                ogl::setMaterialColor(sim_colorcomponent_ambient_diffuse,drawingObject->color.getColorsPtr()+3);
            }

            glBegin(GL_LINES);
            C3Vector v,w;
            v.setData(&_data[drawingObject->floatsPerItem*p+0]);
            w.setData(&_data[drawingObject->floatsPerItem*p+3]);
            glNormal3dv(n.data);
            glVertex3dv(v.data);
            if ( (_objectType&sim_drawing_vertexcolors) && ((!auxCmp)||(_objectType&sim_drawing_auxchannelcolor2)) )
            {
                if (_objectType&(sim_drawing_emissioncolor|sim_drawing_auxchannelcolor2))
                    ogl::setMaterialColor(sim_colorcomponent_emission,rgbaAmb2);
                else
                {
                    ogl::setAlpha(alpha);
                    ogl::setMaterialColor(sim_colorcomponent_ambient_diffuse,rgbaAmb2);
                }
            }
            glVertex3dv(w.data);
            glEnd();
        }
        glLineWidth(1.0);
    }
    else
    { // fixed point size
        glLineWidth((float)_size);
        glBegin(GL_LINES);
        C3Vector v,w;
        float rgbaAmb[3]={0.0,0.0,0.0};
        float rgbaAmb2[3]={0.0,0.0,0.0};
        for (int i=0;i<int(_data.size())/drawingObject->floatsPerItem;i++)
        {
            int off=0;
            int p=_startItem+i;
            if (p>=_maxItemCount)
                p-=_maxItemCount;
            v.setData(&_data[drawingObject->floatsPerItem*p+0]);
            w.setData(&_data[drawingObject->floatsPerItem*p+3]);
            if ( (_objectType&(sim_drawing_itemcolors|sim_drawing_vertexcolors)) && ((!auxCmp)||(_objectType&sim_drawing_auxchannelcolor2)) )
            {
                rgbaAmb[0]=_data[drawingObject->floatsPerItem*p+6];
                rgbaAmb[1]=_data[drawingObject->floatsPerItem*p+7];
                rgbaAmb[2]=_data[drawingObject->floatsPerItem*p+8];
                if (_objectType&(sim_drawing_emissioncolor|sim_drawing_auxchannelcolor2))
                    ogl::setMaterialColor(sim_colorcomponent_emission,rgbaAmb);
                else
                    ogl::setMaterialColor(sim_colorcomponent_ambient_diffuse,rgbaAmb);
            }
            if (_objectType&(sim_drawing_itemcolors|sim_drawing_vertexcolors))
                off+=3;
            if (_objectType&sim_drawing_vertexcolors)
            {
                rgbaAmb2[0]=_data[drawingObject->floatsPerItem*p+6+off];
                rgbaAmb2[1]=_data[drawingObject->floatsPerItem*p+7+off];
                rgbaAmb2[2]=_data[drawingObject->floatsPerItem*p+8+off];
                off+=3;
            }

            double alpha=1.0;
            if ( (_objectType&sim_drawing_itemtransparency)&&(!auxCmp) )
            {
                alpha=1.0-_data[drawingObject->floatsPerItem*p+6+off];
                ogl::setAlpha(alpha);
                ogl::setMaterialColor(sim_colorcomponent_ambient_diffuse,drawingObject->color.getColorsPtr()+3);
            }

            glNormal3dv(n.data);
            glVertex3dv(v.data);
            if ( (_objectType&sim_drawing_vertexcolors) && ((!auxCmp)||(_objectType&sim_drawing_auxchannelcolor2)) )
            {
                if (_objectType&(sim_drawing_emissioncolor|sim_drawing_auxchannelcolor2))
                    ogl::setMaterialColor(sim_colorcomponent_emission,rgbaAmb2);
                else
                {
                    ogl::setAlpha(alpha);
                    ogl::setMaterialColor(sim_colorcomponent_ambient_diffuse,rgbaAmb2);
                }
            }
            glVertex3dv(w.data);
        }
        glEnd();
        glLineWidth(1.0);
    }

    // Following 2 new since introduction of sim_drawing_itemtransparency:
    if ( (_objectType&sim_drawing_itemtransparency)&&(!auxCmp) )
        ogl::setBlending(false); // make sure we turn blending off!
}

void _drawLineStrip(CDrawingObject* drawingObject,int displayAttrib,const C4X4Matrix& cameraRTM,const double normalVectorForLinesAndPoints[3])
{
    bool auxCmp=(displayAttrib&sim_displayattribute_useauxcomponent)!=0;
    int _objectType=drawingObject->getObjectType();
    double _size=drawingObject->getSize();
    int _maxItemCount=drawingObject->getMaxItemCount();
    int _startItem=drawingObject->getStartItem();
    std::vector<double>& _data=drawingObject->getDataPtr()[0];
    // Following 2 new since introduction of sim_drawing_itemtransparency:
    if ( (_objectType&sim_drawing_itemtransparency)&&(!auxCmp) )
        ogl::setBlending(true,GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA); // We turn blending on!

    C3Vector n(cameraRTM.M.axis[2]*-1.0);

    glLineWidth((float)_size);
    glBegin(GL_LINE_STRIP);
    C3Vector v;
    float rgbaAmb[3]={0.0,0.0,0.0};
    float rgbaAmb2[3]={0.0,0.0,0.0};
    int off=0;
    for (int i=0;i<int(_data.size())/drawingObject->floatsPerItem;i++)
    {
        int p=_startItem+i;
        if (p>=_maxItemCount)
            p-=_maxItemCount;
        v.setData(&_data[drawingObject->floatsPerItem*p+0]);
        if ( (_objectType&(sim_drawing_itemcolors|sim_drawing_vertexcolors)) && ((!auxCmp)||(_objectType&sim_drawing_auxchannelcolor2)) )
        {
            rgbaAmb[0]=_data[drawingObject->floatsPerItem*p+3];
            rgbaAmb[1]=_data[drawingObject->floatsPerItem*p+4];
            rgbaAmb[2]=_data[drawingObject->floatsPerItem*p+5];
            if (_objectType&(sim_drawing_emissioncolor|sim_drawing_auxchannelcolor2))
                ogl::setMaterialColor(sim_colorcomponent_emission,rgbaAmb);
            else
                ogl::setMaterialColor(sim_colorcomponent_ambient_diffuse,rgbaAmb);
        }

        glNormal3dv(n.data);
        glVertex3dv(v.data);
    }
    glEnd();
    glLineWidth(1.0);

    // Following 2 new since introduction of sim_drawing_itemtransparency:
    if ( (_objectType&sim_drawing_itemtransparency)&&(!auxCmp) )
        ogl::setBlending(false); // make sure we turn blending off!
}

void _drawTriangles(CDrawingObject* drawingObject,int displayAttrib)
{
    bool auxCmp=(displayAttrib&sim_displayattribute_useauxcomponent)!=0;
    int _objectType=drawingObject->getObjectType();
    //double _size=drawingObject->getSize();
    int _maxItemCount=drawingObject->getMaxItemCount();
    int _startItem=drawingObject->getStartItem();
    std::vector<double>& _data=drawingObject->getDataPtr()[0];
    // Following 2 new since introduction of sim_drawing_itemtransparency:
    if ( (_objectType&sim_drawing_itemtransparency)&&(!auxCmp) )
        ogl::setBlending(true,GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA); // We turn blending on!

    glBegin(GL_TRIANGLES);
    C3Vector v,w,x,n;
    int off=0;
    for (int i=0;i<int(_data.size())/drawingObject->floatsPerItem;i++)
    {
        int p=_startItem+i;
        if (p>=_maxItemCount)
            p-=_maxItemCount;
        v.setData(&_data[drawingObject->floatsPerItem*p+0]);
        w.setData(&_data[drawingObject->floatsPerItem*p+3]);
        x.setData(&_data[drawingObject->floatsPerItem*p+6]);
        if ( (_objectType&(sim_drawing_itemcolors|sim_drawing_vertexcolors)) && ((!auxCmp)||(_objectType&sim_drawing_auxchannelcolor2)) )
        {
            float ab[3]={(float)_data[drawingObject->floatsPerItem*p+9],(float)_data[drawingObject->floatsPerItem*p+10],(float)_data[drawingObject->floatsPerItem*p+11]};
            if (_objectType&(sim_drawing_emissioncolor|sim_drawing_auxchannelcolor2))
                ogl::setMaterialColor(sim_colorcomponent_emission,ab);
            else
                ogl::setMaterialColor(sim_colorcomponent_ambient_diffuse,ab);
        }
        if (_objectType&(sim_drawing_itemcolors|sim_drawing_vertexcolors))
            off+=3;
        float* vertex2Col=nullptr;
        float* vertex3Col=nullptr;
        float vertex2Col_[3];
        float vertex3Col_[3];
        if (_objectType&sim_drawing_vertexcolors)
        {
            vertex2Col_[0]=_data[drawingObject->floatsPerItem*p+9+off+0];
            vertex2Col_[1]=_data[drawingObject->floatsPerItem*p+9+off+1];
            vertex2Col_[2]=_data[drawingObject->floatsPerItem*p+9+off+2];
            vertex2Col=vertex2Col_;
            off+=3;
            vertex3Col_[0]=_data[drawingObject->floatsPerItem*p+9+off+0];
            vertex3Col_[1]=_data[drawingObject->floatsPerItem*p+9+off+1];
            vertex3Col_[2]=_data[drawingObject->floatsPerItem*p+9+off+2];
            vertex3Col=vertex3Col_;
            off+=3;
        }

        if ( (_objectType&sim_drawing_itemtransparency)&&(!auxCmp) )
        { // Following part new since introduction of sim_drawing_itemtransparency:
            ogl::setAlpha(1.0-_data[drawingObject->floatsPerItem*p+9+off]);
            off++;
        }

        n=(w-v)^(x-v);
        double l=n.getLength();
        if (l!=0.0)
            n/=l;
        glNormal3dv(n.data);
        glVertex3dv(v.data);
        if ( (_objectType&sim_drawing_vertexcolors) && ((!auxCmp)||(_objectType&sim_drawing_auxchannelcolor2)) )
        {
            if (_objectType&(sim_drawing_emissioncolor|sim_drawing_auxchannelcolor2))
                ogl::setMaterialColor(sim_colorcomponent_emission,vertex2Col);
            else
                ogl::setMaterialColor(sim_colorcomponent_ambient_diffuse,vertex2Col);
        }
        glVertex3dv(w.data);
        if ( (_objectType&sim_drawing_vertexcolors) && ((!auxCmp)||(_objectType&sim_drawing_auxchannelcolor2)) )
        {
            if (_objectType&(sim_drawing_emissioncolor|sim_drawing_auxchannelcolor2))
                ogl::setMaterialColor(sim_colorcomponent_emission,vertex3Col);
            else
                ogl::setMaterialColor(sim_colorcomponent_ambient_diffuse,vertex3Col);
        }
        glVertex3dv(x.data);
    }
    glEnd();

    // Following 2 new since introduction of sim_drawing_itemtransparency:
    if ( (_objectType&sim_drawing_itemtransparency)&&(!auxCmp) )
        ogl::setBlending(false); // make sure we turn blending off!
}

#else

void displayDrawingObject(CDrawingObject* drawingObject,C7Vector& tr,bool overlay,bool transparentObject,int displayAttrib,const C4X4Matrix& cameraCTM)
{

}

#endif



