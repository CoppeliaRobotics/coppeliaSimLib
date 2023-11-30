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

#include <pathRendering.h>
#include <guiApp.h>

void displayPath(CPath_old *path, CViewableBase *renderingObject, int displayAttrib)
{
    // At the beginning of every scene object display routine:
    _commonStart(path, renderingObject);

    C3Vector normalVectorForLinesAndPoints(path->getFullCumulativeTransformation().Q.getInverse() *
                                           C3Vector::unitZVector);

    // Object display:
    if (path->getShouldObjectBeDisplayed(renderingObject->getObjectHandle(), displayAttrib) ||
        ((GuiApp::mainWindow != nullptr) && (GuiApp::mainWindow->editModeContainer->getEditModePath_old() == path)))
    {
        if ((GuiApp::getEditModeType() & SHAPE_OR_PATH_EDIT_MODE_OLD) == 0)
        {
            if (path->getObjectProperty() & sim_objectproperty_selectmodelbaseinstead)
                glLoadName(path->getModelSelectionHandle());
            else
                glLoadName(path->getObjectHandle());
        }
        else
            glLoadName(-1);

        if ((displayAttrib & sim_displayattribute_forcewireframe) && (displayAttrib & sim_displayattribute_renderpass))
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        if ((GuiApp::mainWindow != nullptr) && (GuiApp::mainWindow->editModeContainer->getEditModePath_old() == path))
            GuiApp::mainWindow->editModeContainer->getEditModePathContainer_old()->render(true, 0, false,
                                                                                          path->getObjectHandle());
        else
        {
            _enableAuxClippingPlanes(path->getObjectHandle());
            if ((displayAttrib & sim_displayattribute_forvisionsensor) == 0)
            {
                bool isUniqueSelectedPath = false;
                if (GuiApp::mainWindow != nullptr)
                    isUniqueSelectedPath = GuiApp::mainWindow->editModeContainer->pathPointManipulation
                                               ->getUniqueSelectedPathId_nonEditMode() != -1;
                path->pathContainer->render(false, displayAttrib, isUniqueSelectedPath, path->getObjectHandle());
            }

            if (path->getShapingEnabled())
            {
                path->getShapingColor()->makeCurrentColor((displayAttrib & sim_displayattribute_useauxcomponent) != 0);
                glBegin(GL_TRIANGLES);
                for (int i = 0; i < int(path->_pathShapeIndices.size()); i++)
                {
                    glNormal3dv(&path->_pathShapeNormals[3 * i]);
                    glVertex3dv(&path->_pathShapeVertices[3 * (path->_pathShapeIndices[i])]);
                }
                glEnd();
            }
            _disableAuxClippingPlanes();
        }

        glDisable(GL_CULL_FACE);
    }

    // At the end of every scene object display routine:
    _commonFinish(path, renderingObject);
}
