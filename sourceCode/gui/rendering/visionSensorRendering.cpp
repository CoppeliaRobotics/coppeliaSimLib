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

#include <visionSensorRendering.h>
#include <guiApp.h>

void displayVisionSensor(CVisionSensor* visionSensor, CViewableBase* renderingObject, int displayAttrib)
{
    // At the beginning of every scene object display routine:
    _commonStart(visionSensor, renderingObject);

    C3Vector normalVectorForLinesAndPoints(visionSensor->getFullCumulativeTransformation().Q.getInverse() *
                                           C3Vector::unitZVector);

    // Object display:
    if (visionSensor->getShouldObjectBeDisplayed(renderingObject->getObjectHandle(), displayAttrib))
    {
        if ((GuiApp::getEditModeType() & SHAPE_OR_PATH_EDIT_MODE_OLD) == 0)
        {
            if (visionSensor->getObjectProperty() & sim_objectproperty_selectmodelbaseinstead)
                glLoadName(visionSensor->getModelSelectionHandle());
            else
                glLoadName(visionSensor->getObjectHandle());
        }
        else
            glLoadName(-1);

        if ((displayAttrib & sim_displayattribute_forcewireframe) && (displayAttrib & sim_displayattribute_renderpass))
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        glPushMatrix();

        if (visionSensor->getShowVolume())
        {
            visionSensor->getColor()->makeCurrentColor((displayAttrib & sim_displayattribute_useauxcomponent) != 0);
            C3Vector c, f;
            visionSensor->getVolumeVectors(c, f);
            ogl::buffer.clear();
            ogl::addBuffer3DPoints(-f(0), -f(1), f(2));
            ogl::addBuffer3DPoints(-f(0), +f(1), f(2));
            ogl::addBuffer3DPoints(+f(0), +f(1), f(2));
            ogl::addBuffer3DPoints(+f(0), -f(1), f(2));
            ogl::addBuffer3DPoints(-f(0), -f(1), f(2));
            ogl::drawRandom3dLines(&ogl::buffer[0], (int)ogl::buffer.size() / 3, true,
                                   normalVectorForLinesAndPoints.data);
            ogl::buffer.clear();
            ogl::addBuffer3DPoints(-c(0), -c(1), c(2));
            ogl::addBuffer3DPoints(-c(0), +c(1), c(2));
            ogl::addBuffer3DPoints(+c(0), +c(1), c(2));
            ogl::addBuffer3DPoints(+c(0), -c(1), c(2));
            ogl::addBuffer3DPoints(-c(0), -c(1), c(2));
            ogl::drawRandom3dLines(&ogl::buffer[0], (int)ogl::buffer.size() / 3, true,
                                   normalVectorForLinesAndPoints.data);
            ogl::buffer.clear();
            ogl::addBuffer3DPoints(-c(0), -c(1), c(2));
            ogl::addBuffer3DPoints(-f(0), -f(1), f(2));
            ogl::addBuffer3DPoints(-c(0), +c(1), c(2));
            ogl::addBuffer3DPoints(-f(0), +f(1), f(2));
            ogl::addBuffer3DPoints(+c(0), -c(1), c(2));
            ogl::addBuffer3DPoints(+f(0), -f(1), f(2));
            ogl::addBuffer3DPoints(+c(0), +c(1), c(2));
            ogl::addBuffer3DPoints(+f(0), +f(1), f(2));
            ogl::drawRandom3dLines(&ogl::buffer[0], (int)ogl::buffer.size() / 3, false,
                                   normalVectorForLinesAndPoints.data);
            ogl::buffer.clear();
        }

        visionSensor->getColor()->makeCurrentColor((displayAttrib & sim_displayattribute_useauxcomponent) != 0);
        float _size(visionSensor->getVisionSensorSize());
        float dx = _size * 0.5;
        float dy = _size * 0.5;
        glBegin(GL_QUADS);
        glNormal3f(-1.0, 0.0, 0.0);
        glVertex3f(-dx, -dy, 0.0);
        glVertex3f(-dx, dy, 0.0);
        glVertex3f(-dx, dy, -_size * 2.0);
        glVertex3f(-dx, -dy, -_size * 2.0);
        glNormal3f(0.0, 0.0, -1.0);
        glVertex3f(-dx, -dy, -_size * 2.0);
        glVertex3f(-dx, dy, -_size * 2.0);
        glVertex3f(dx, dy, -_size * 2.0);
        glVertex3f(dx, -dy, -_size * 2.0);
        glNormal3f(1.0, 0.0, 0.0);
        glVertex3f(dx, -dy, -_size * 2.0);
        glVertex3f(dx, dy, -_size * 2.0);
        glVertex3f(dx, dy, 0.0);
        glVertex3f(dx, -dy, 0.0);
        glNormal3f(0.0, -1.0, 0.0);
        glVertex3f(-dx, -dy, 0.0);
        glVertex3f(-dx, -dy, -_size * 2.0);
        glVertex3f(dx, -dy, -_size * 2.0);
        glVertex3f(dx, -dy, 0.0);
        glNormal3f(0.0, 1.0, 0.0);
        glVertex3f(-dx, dy, 0.0);
        glVertex3f(dx, dy, 0.0);
        glVertex3f(dx, dy, -_size * 2.0);
        glVertex3f(-dx, dy, -_size * 2.0);
        ogl::setMaterialColor(ogl::colorBlack, ogl::colorBlack, ogl::colorBlack);
        glNormal3f(0.0, 0.0, 1.0);
        glVertex3f(dx, -dy, 0.0);
        glVertex3f(dx, dy, 0.0);
        glVertex3f(-dx, dy, 0.0);
        glVertex3f(-dx, -dy, 0.0);
        glEnd();
        glPopMatrix();

        _disableAuxClippingPlanes();
    }

    // At the end of every scene object display routine:
    _commonFinish(visionSensor, renderingObject);
}
