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

#include <pointCloudRendering.h>
#include <guiApp.h>

void displayPointCloud(CPointCloud* pointCloud, CViewableBase* renderingObject, int displayAttrib)
{
    // At the beginning of every scene object display routine:
    _commonStart(pointCloud, renderingObject);

    C3Vector normalVectorForLinesAndPoints(pointCloud->getFullCumulativeTransformation().Q.getInverse() *
                                           C3Vector::unitZVector);

    // Object display:
    if (pointCloud->getShouldObjectBeDisplayed(renderingObject->getObjectHandle(), displayAttrib))
    {
        if ((GuiApp::getEditModeType() & SHAPE_OR_PATH_EDIT_MODE_OLD) == 0)
        {
            if (pointCloud->getObjectProperty() & sim_objectproperty_selectmodelbaseinstead)
                glLoadName(pointCloud->getModelSelectionHandle());
            else
                glLoadName(pointCloud->getObjectHandle());
        }
        else
            glLoadName(-1);

        if ((displayAttrib & sim_displayattribute_forcewireframe) && (displayAttrib & sim_displayattribute_renderpass))
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        if ((displayAttrib & sim_displayattribute_forcewireframe) == 0)
            glEnable(GL_CULL_FACE);

        _enableAuxClippingPlanes(pointCloud->getObjectHandle());

        std::vector<double>& _points = pointCloud->getPoints()[0];
        if (_points.size() > 0)
        {
            pointCloud->getColor()->makeCurrentColor(false);

            if (pointCloud->getShowOctree() && (pointCloud->getPointCloudInfo() != nullptr) &&
                ((displayAttrib & sim_displayattribute_forvisionsensor) == 0))
            {
                std::vector<double> corners;
                App::worldContainer->pluginContainer->geomPlugin_getPtcloudOctreeCorners(
                    pointCloud->getPointCloudInfo(), corners);

                glBegin(GL_LINES);
                glNormal3dv(normalVectorForLinesAndPoints.data);

                for (size_t i = 0; i < corners.size() / 24; i++)
                {
                    glVertex3dv(&corners[0] + i * 8 * 3 + 0);
                    glVertex3dv(&corners[0] + i * 8 * 3 + 3);
                    glVertex3dv(&corners[0] + i * 8 * 3 + 3);
                    glVertex3dv(&corners[0] + i * 8 * 3 + 9);
                    glVertex3dv(&corners[0] + i * 8 * 3 + 0);
                    glVertex3dv(&corners[0] + i * 8 * 3 + 6);
                    glVertex3dv(&corners[0] + i * 8 * 3 + 6);
                    glVertex3dv(&corners[0] + i * 8 * 3 + 9);

                    glVertex3dv(&corners[0] + i * 8 * 3 + 12);
                    glVertex3dv(&corners[0] + i * 8 * 3 + 15);
                    glVertex3dv(&corners[0] + i * 8 * 3 + 15);
                    glVertex3dv(&corners[0] + i * 8 * 3 + 21);
                    glVertex3dv(&corners[0] + i * 8 * 3 + 12);
                    glVertex3dv(&corners[0] + i * 8 * 3 + 18);
                    glVertex3dv(&corners[0] + i * 8 * 3 + 18);
                    glVertex3dv(&corners[0] + i * 8 * 3 + 21);

                    glVertex3dv(&corners[0] + i * 8 * 3 + 0);
                    glVertex3dv(&corners[0] + i * 8 * 3 + 12);

                    glVertex3dv(&corners[0] + i * 8 * 3 + 3);
                    glVertex3dv(&corners[0] + i * 8 * 3 + 15);

                    glVertex3dv(&corners[0] + i * 8 * 3 + 6);
                    glVertex3dv(&corners[0] + i * 8 * 3 + 18);

                    glVertex3dv(&corners[0] + i * 8 * 3 + 9);
                    glVertex3dv(&corners[0] + i * 8 * 3 + 21);
                }
                glEnd();
            }

            glPointSize(float(pointCloud->getPointSize()));
            std::vector<double>* pts = &_points;
            std::vector<double>* cols = pointCloud->getColors();
            if (pointCloud->getDisplayPoints()->size() > 0)
            {
                pts = pointCloud->getDisplayPoints();
                cols = pointCloud->getDisplayColors();
            }

            if (cols->size() == 0)
            {
                glBegin(GL_POINTS);
                glNormal3dv(normalVectorForLinesAndPoints.data);
                for (size_t i = 0; i < pts->size() / 3; i++)
                    glVertex3dv(&(pts[0])[3 * i]);
                glEnd();
            }
            else
            {
                // note: glMaterialfv has some bugs in some geForce drivers, use glColor instead
                glEnable(GL_COLOR_MATERIAL);
                glColorMaterial(GL_FRONT_AND_BACK, GL_SPECULAR);
                glColor3f(0.0, 0.0, 0.0);
                glColorMaterial(GL_FRONT_AND_BACK, GL_EMISSION);
                glColor3f(0.0, 0.0, 0.0);
                glColorMaterial(GL_FRONT_AND_BACK, GL_SHININESS);
                glColor3f(0.0, 0.0, 0.0);
                glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
                glColor3f(0.0, 0.0, 0.0);
                if (pointCloud->getColorIsEmissive())
                    glColorMaterial(GL_FRONT_AND_BACK, GL_EMISSION);

                glBegin(GL_POINTS);
                glNormal3dv(normalVectorForLinesAndPoints.data);
                for (size_t i = 0; i < pts->size() / 3; i++)
                {
                    glColor4dv(&(cols[0])[4 * i]);
                    glVertex3dv(&(pts[0])[3 * i]);
                }
                glEnd();
                glDisable(GL_COLOR_MATERIAL);
            }
            glPointSize(1.0);
        }

        glDisable(GL_CULL_FACE);
        _disableAuxClippingPlanes();
    }

    // At the end of every scene object display routine:
    _commonFinish(pointCloud, renderingObject);
}
