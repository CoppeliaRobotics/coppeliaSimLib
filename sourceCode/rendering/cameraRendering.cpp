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

#include <cameraRendering.h>

#ifdef SIM_WITH_OPENGL
#include <pluginContainer.h>

void displayCamera(CCamera* camera,CViewableBase* renderingObject,int displayAttrib)
{
    // At the beginning of every 3DObject display routine:
    _commonStart(camera,renderingObject,displayAttrib);

    // Bounding box display:
    if (displayAttrib&sim_displayattribute_renderpass)
        _displayBoundingBox(camera,displayAttrib,true,0.0);

    C3Vector normalVectorForLinesAndPoints(camera->getFullCumulativeTransformation().Q.getInverse()*C3Vector::unitZVector);

    // Object display:
    if (camera->getShouldObjectBeDisplayed(renderingObject->getObjectHandle(),displayAttrib))
    {
        if ((App::getEditModeType()&SHAPE_OR_PATH_EDIT_MODE_OLD)==0)
        {
            if (camera->getObjectProperty()&sim_objectproperty_selectmodelbaseinstead)
                glLoadName(camera->getModelSelectionHandle());
            else
                glLoadName(camera->getObjectHandle());
        }
        else
            glLoadName(-1);

        if ( (displayAttrib&sim_displayattribute_forcewireframe)&&(displayAttrib&sim_displayattribute_renderpass) )
            glPolygonMode (GL_FRONT_AND_BACK,GL_LINE);

        _enableAuxClippingPlanes(camera->getObjectHandle());
        camera->getColor(false)->makeCurrentColor((displayAttrib&sim_displayattribute_useauxcomponent)!=0);
        double cameraSize=camera->getCameraSize();
        glPushMatrix();
        glTranslated(0.0,0.0,-cameraSize);
        ogl::drawBox(0.4*cameraSize,cameraSize,2.0*cameraSize,true,nullptr);
        glPopMatrix();
        glPushMatrix();
        camera->getColor(true)->makeCurrentColor((displayAttrib&sim_displayattribute_useauxcomponent)!=0);
        glTranslated(0.0,1.3*cameraSize,-0.25*cameraSize);
        glRotatef(90.0,0,1,0);
        ogl::drawCylinder(2.0*cameraSize,cameraSize/2.0,20,0,true);
        glTranslated(1.5*cameraSize,0.0,0.0);
        ogl::drawCylinder(2.0*cameraSize,cameraSize/2.0,20,0,true);
        glPopMatrix();
        glPushMatrix();
        glTranslated(0.0,0.0,cameraSize/6.0);
        ogl::drawCone(cameraSize,5.0*cameraSize/3.0,20,true,true);
        glPopMatrix();
        if (camera->getShowVolume())
        {
            C3Vector c,f;
            camera->getVolumeVectors(c,f);
            ogl::buffer.clear();
            ogl::addBuffer3DPoints(-f(0),-f(1),f(2));
            ogl::addBuffer3DPoints(-f(0),+f(1),f(2));
            ogl::addBuffer3DPoints(+f(0),+f(1),f(2));
            ogl::addBuffer3DPoints(+f(0),-f(1),f(2));
            ogl::addBuffer3DPoints(-f(0),-f(1),f(2));
            ogl::drawRandom3dLines(&ogl::buffer[0],(int)ogl::buffer.size()/3,true,normalVectorForLinesAndPoints.data);
            ogl::buffer.clear();
            ogl::addBuffer3DPoints(-c(0),-c(1),c(2));
            ogl::addBuffer3DPoints(-c(0),+c(1),c(2));
            ogl::addBuffer3DPoints(+c(0),+c(1),c(2));
            ogl::addBuffer3DPoints(+c(0),-c(1),c(2));
            ogl::addBuffer3DPoints(-c(0),-c(1),c(2));
            ogl::drawRandom3dLines(&ogl::buffer[0],(int)ogl::buffer.size()/3,true,normalVectorForLinesAndPoints.data);
            ogl::buffer.clear();
            ogl::addBuffer3DPoints(-c(0),-c(1),c(2));
            ogl::addBuffer3DPoints(-f(0),-f(1),f(2));
            ogl::addBuffer3DPoints(-c(0),+c(1),c(2));
            ogl::addBuffer3DPoints(-f(0),+f(1),f(2));
            ogl::addBuffer3DPoints(+c(0),-c(1),c(2));
            ogl::addBuffer3DPoints(+f(0),-f(1),f(2));
            ogl::addBuffer3DPoints(+c(0),+c(1),c(2));
            ogl::addBuffer3DPoints(+f(0),+f(1),f(2));
            ogl::drawRandom3dLines(&ogl::buffer[0],(int)ogl::buffer.size()/3,false,normalVectorForLinesAndPoints.data);
            ogl::buffer.clear();
        }

        _disableAuxClippingPlanes();
    }

    // At the end of every 3DObject display routine:
    _commonFinish(camera,renderingObject);
}

#else

void displayCamera(CCamera* camera,CViewableBase* renderingObject,int displayAttrib)
{

}

#endif
