
#include "dynamicsRendering.h"

#ifdef SIM_WITH_OPENGL
#include "pluginContainer.h"

const float SPHEREVERTICES[24*3]={
-0.4142f,-1.0000f,-0.4142f,
+0.4142f,-1.0000f,-0.4142f,
+0.4142f,-1.0000f,+0.4142f,
-0.4142f,-1.0000f,+0.4142f,

-0.4142f,-0.4142f,-1.0000f,
-0.4142f,+0.4142f,-1.0000f,
+0.4142f,+0.4142f,-1.0000f,
+0.4142f,-0.4142f,-1.0000f,

+0.4142f,+1.0000f,-0.4142f,
-0.4142f,+1.0000f,-0.4142f,
-0.4142f,+1.0000f,+0.4142f,
+0.4142f,+1.0000f,+0.4142f,

+0.4142f,-0.4142f,+1.0000f,
+0.4142f,+0.4142f,+1.0000f,
-0.4142f,+0.4142f,+1.0000f,
-0.4142f,-0.4142f,+1.0000f,

+1.0000f,-0.4142f,-0.4142f,
+1.0000f,+0.4142f,-0.4142f,
+1.0000f,+0.4142f,+0.4142f,
+1.0000f,-0.4142f,+0.4142f,

-1.0000f,-0.4142f,+0.4142f,
-1.0000f,+0.4142f,+0.4142f,
-1.0000f,+0.4142f,-0.4142f,
-1.0000f,-0.4142f,-0.4142f
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

const float SPHEREQUADNORMALS[18*3]={
+0.0f,-1.0f,+0.0f,
+0.0f,+0.0f,-1.0f,
+0.0f,+1.0f,+0.0f,
+0.0f,+0.0f,+1.0f,
+1.0f,+0.0f,+0.0f,
-1.0f,+0.0f,+0.0f,

+0.0f,-0.7071f,-0.7071f,
+0.7071f,-0.7071f,+0.0f,
+0.0f,-0.7071f,+0.7071f,
-0.7071f,-0.7071f,+0.0f,

+0.0f,+0.7071f,-0.7071f,
+0.7071f,+0.7071f,+0.0f,
+0.0f,+0.7071f,+0.7071f,
-0.7071f,+0.7071f,+0.0f,

+0.7071f,0.0f,-0.7071f,
+0.7071f,0.0f,+0.7071f,
-0.7071f,0.0f,+0.7071f,
-0.7071f,0.0f,-0.7071f
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

const float SPHERETRIANGLENORMALS[8*3]={
+0.5773f,-0.5773f,-0.5773f,
+0.5773f,-0.5773f,+0.5773f,
-0.5773f,-0.5773f,+0.5773f,
-0.5773f,-0.5773f,-0.5773f,

+0.5773f,+0.5773f,-0.5773f,
+0.5773f,+0.5773f,+0.5773f,
-0.5773f,+0.5773f,+0.5773f,
-0.5773f,+0.5773f,-0.5773f,
};

void displayParticles(void** particlesPointer,int particlesCount,int displayAttrib,const C4X4Matrix& cameraCTM,const float* cols,int objectType)
{
    ogl::setMaterialColor(cols,cols+6,cols+9);
    C3Vector pos;
    float size;
    int particleType;
    float* additionalColor;
    for (int i=0;i<particlesCount;i++)
    {
        if (particlesPointer[i]!=nullptr)
        {
            if (CPluginContainer::dyn_getParticleData(particlesPointer[i],pos.data,&size,&particleType,&additionalColor))
            {
            //  displayParticle(displayAttrib,m,pos,size,particleType,additionalColor);
                glPushMatrix();
                glPushAttrib(GL_POLYGON_BIT);
                glLoadName(-1);

                bool wire=( (objectType&sim_drawing_wireframe)||(displayAttrib&sim_displayattribute_forcewireframe))&&((displayAttrib&sim_displayattribute_depthpass)==0);
                if (displayAttrib&sim_displayattribute_forbidwireframe)
                    wire=false;
                if (wire)
                    glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
                glEnable(GL_CULL_FACE);


                int tmp=objectType&0x001f;
                if ((tmp==sim_particle_points1)||(tmp==sim_particle_points2)||(tmp==sim_particle_points4))
                    _drawPoints(displayAttrib,cameraCTM,pos,size,objectType,additionalColor);
                if (tmp==sim_particle_roughspheres)
                    _drawRoughSphere(displayAttrib,pos,size,objectType,additionalColor);
                if (tmp==sim_particle_spheres)
                    _drawSphere(displayAttrib,pos,size,objectType,additionalColor);

                glDisable(GL_CULL_FACE);
                glPopAttrib();
                glPopMatrix();
            }
        }
    }
}

void displayContactPoints(int displayAttrib,const CVisualParam& contactPointColor,const float* pts,int cnt)
{
    glDisable(GL_DEPTH_TEST);
    glLoadName(-1);
    glPointSize(4.0f);
    contactPointColor.makeCurrentColor((displayAttrib&sim_displayattribute_useauxcomponent)!=0);
    ogl::buffer.clear();
    for (int i=0;i<cnt;i++)
        ogl::addBuffer3DPoints(pts+3*i); // Contact points come in pairs!
    if (cnt!=0)
        ogl::drawRandom3dPoints(&ogl::buffer[0],cnt,nullptr);
    ogl::buffer.clear();

    glLineWidth(1.0f);
    glPointSize(1.0f);
    glEnable(GL_DEPTH_TEST);
}

void _drawPoints(int displayAttrib,const C4X4Matrix& cameraRTM,const C3Vector& _currentPosition,float _size,int _objectType,const float* _additionalColor)
{
    C3Vector n(cameraRTM.M.axis[2]*-1.0f);
    int tmp=_objectType&0x001f;
    if (tmp==sim_particle_points1)
        glPointSize(1.0f);
    if (tmp==sim_particle_points2)
        glPointSize(2.0f);
    if (tmp==sim_particle_points4)
        glPointSize(4.0f);

    C3Vector v;
    float rgbaAmb[4]={0.0f,0.0f,0.0f,1.0f};
    if (_objectType&sim_particle_itemcolors)
    {
        rgbaAmb[0]=_additionalColor[0];
        rgbaAmb[1]=_additionalColor[1];
        rgbaAmb[2]=_additionalColor[2];
    }
    ogl::drawRandom3dPointsEx(_currentPosition.data,1,nullptr,rgbaAmb,nullptr,(_objectType&sim_particle_emissioncolor)!=0,n.data);
    glPointSize(1.0f);
}

void _drawRoughSphere(int displayAttrib,const C3Vector& _currentPosition,float _size,int _objectType,const float* _additionalColor)
{
    if (_objectType&sim_particle_itemcolors)
    {
        if (_objectType&sim_particle_emissioncolor)
            ogl::setMaterialColor(sim_colorcomponent_emission,_additionalColor);
        else
            ogl::setMaterialColor(sim_colorcomponent_ambient_diffuse,_additionalColor);
    }
    glBegin(GL_QUADS);
    for (int j=0;j<18;j++)
    {
        C3Vector x0(SPHEREVERTICES+3*SPHEREQUADINDICES[4*j+0]);
        C3Vector x1(SPHEREVERTICES+3*SPHEREQUADINDICES[4*j+1]);
        C3Vector x2(SPHEREVERTICES+3*SPHEREQUADINDICES[4*j+2]);
        C3Vector x3(SPHEREVERTICES+3*SPHEREQUADINDICES[4*j+3]);
        x0=_currentPosition+x0*_size*0.5f;
        x1=_currentPosition+x1*_size*0.5f;
        x2=_currentPosition+x2*_size*0.5f;
        x3=_currentPosition+x3*_size*0.5f;
        glNormal3fv(SPHEREQUADNORMALS+3*j);
        glVertex3fv(x0.data);
        glVertex3fv(x1.data);
        glVertex3fv(x2.data);
        glVertex3fv(x3.data);
    }
    glEnd();
    glBegin(GL_TRIANGLES);
    for (int j=0;j<8;j++)
    {
        C3Vector x0(SPHEREVERTICES+3*SPHERETRIANGLEINDICES[3*j+0]);
        C3Vector x1(SPHEREVERTICES+3*SPHERETRIANGLEINDICES[3*j+1]);
        C3Vector x2(SPHEREVERTICES+3*SPHERETRIANGLEINDICES[3*j+2]);
        x0=_currentPosition+x0*_size*0.5f;
        x1=_currentPosition+x1*_size*0.5f;
        x2=_currentPosition+x2*_size*0.5f;
        glNormal3fv(SPHERETRIANGLENORMALS+3*j);
        glVertex3fv(x0.data);
        glVertex3fv(x1.data);
        glVertex3fv(x2.data);
    }
    glEnd();
}

void _drawSphere(int displayAttrib,const C3Vector& _currentPosition,float _size,int _objectType,const float* _additionalColor)
{
    glPushMatrix();
    glTranslatef(_currentPosition(0),_currentPosition(1),_currentPosition(2));

    if (_objectType&sim_particle_itemcolors)
    {
        if (_objectType&sim_particle_emissioncolor)
            ogl::setMaterialColor(sim_colorcomponent_emission,_additionalColor);
        else
            ogl::setMaterialColor(sim_colorcomponent_ambient_diffuse,_additionalColor);
    }
    ogl::drawSphere(_size*0.5f,10,5,true);
    glPopMatrix();
}

#else

void displayParticles(void** particlesPointer,int particlesCount,int displayAttrib,const C4X4Matrix& cameraCTM,const float* cols,int objectType)
{

}

void displayContactPoints(int displayAttrib,const CVisualParam& contactPointColor,const float* pts,int cnt)
{

}

#endif



