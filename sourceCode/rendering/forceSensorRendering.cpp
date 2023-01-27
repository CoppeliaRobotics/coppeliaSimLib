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

#include "forceSensorRendering.h"

#ifdef SIM_WITH_OPENGL

void displayForceSensor(CForceSensor* forceSensor,CViewableBase* renderingObject,int displayAttrib)
{
    // At the beginning of every 3DObject display routine:
    _commonStart(forceSensor,renderingObject,displayAttrib);

    // Bounding box display:
    if (displayAttrib&sim_displayattribute_renderpass)
        _displayBoundingBox(forceSensor,displayAttrib,true,forceSensor->getForceSensorSize());

    // Object display:
    if (forceSensor->getShouldObjectBeDisplayed(renderingObject->getObjectHandle(),displayAttrib))
    {
        _enableAuxClippingPlanes(forceSensor->getObjectHandle());
        if (displayAttrib&sim_displayattribute_dynamiccontentonly)
            ogl::setMaterialColor(0.0f,0.6f,0.0f,0.5f,0.5f,0.5f,0.0f,0.0f,0.0f);
        else
            forceSensor->getColor(false)->makeCurrentColor((displayAttrib&sim_displayattribute_useauxcomponent)!=0);
        _displayForceSensor(forceSensor,displayAttrib,true,forceSensor->getForceSensorSize());
        if (displayAttrib&sim_displayattribute_dynamiccontentonly)
            ogl::setMaterialColor(0.2f,0.2f,0.2f,0.5f,0.5f,0.5f,0.0f,0.0f,0.0f);
        else
            forceSensor->getColor(true)->makeCurrentColor((displayAttrib&sim_displayattribute_useauxcomponent)!=0);
        _displayForceSensor(forceSensor,displayAttrib,false,forceSensor->getForceSensorSize());
        _disableAuxClippingPlanes();
    }

    // At the end of every 3DObject display routine:
    _commonFinish(forceSensor,renderingObject);
}

void _displayForceSensor(CForceSensor* forceSensor,int displayAttrib,bool partOne,double sizeParam)
{
    if ((App::getEditModeType()&SHAPE_OR_PATH_EDIT_MODE_OLD)==0)
    {
        if (forceSensor->getObjectProperty()&sim_objectproperty_selectmodelbaseinstead)
            glLoadName(forceSensor->getModelSelectionHandle());
        else
            glLoadName(forceSensor->getObjectHandle());
    }
    else
        glLoadName(-1);

    glPushAttrib(GL_POLYGON_BIT);
    if ((displayAttrib&sim_displayattribute_forcewireframe)&&(displayAttrib&sim_displayattribute_renderpass))
        glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);

    glPushMatrix();
    if (partOne)
        ogl::drawCylinder(forceSensor->getForceSensorSize(),forceSensor->getForceSensorSize()*0.5,16,0,true);
    else
    {
        C7Vector tr(forceSensor->getIntrinsicTransformation(true));
        glTranslated(tr.X(0),tr.X(1),tr.X(2));
        C4Vector axis=tr.Q.getAngleAndAxis();
        glRotated(axis(0)*radToDeg,axis(1),axis(2),axis(3));

        ogl::drawCylinder(forceSensor->getForceSensorSize()/5.0,forceSensor->getForceSensorSize(),16,0,true);
        if (displayAttrib&sim_displayattribute_selected)
            _drawReference(forceSensor,sizeParam);
    }
    glPopAttrib();
    glPopMatrix();
}

#else

void displayForceSensor(CForceSensor* forceSensor,CViewableBase* renderingObject,int displayAttrib)
{

}

#endif



