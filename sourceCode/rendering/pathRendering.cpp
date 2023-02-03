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

#ifdef SIM_WITH_OPENGL

void displayPath(CPath_old* path,CViewableBase* renderingObject,int displayAttrib)
{
    // At the beginning of every 3DObject display routine:
    _commonStart(path,renderingObject,displayAttrib);

    // Bounding box display:
    if (displayAttrib&sim_displayattribute_renderpass)
        _displayBoundingBox(path,displayAttrib,true,path->pathContainer->getSquareSize()*2.0);

    C3Vector normalVectorForLinesAndPoints(path->getFullCumulativeTransformation().Q.getInverse()*C3Vector::unitZVector);

    // Object display:
#ifdef SIM_WITH_GUI
    if ( path->getShouldObjectBeDisplayed(renderingObject->getObjectHandle(),displayAttrib)||( (App::mainWindow!=nullptr)&&(App::mainWindow->editModeContainer->getEditModePath_old()==path) ) )
#else
    if (path->getShouldObjectBeDisplayed(renderingObject->getObjectHandle(),displayAttrib))
#endif
    {
        if ((App::getEditModeType()&SHAPE_OR_PATH_EDIT_MODE_OLD)==0)
        {
            if (path->getObjectProperty()&sim_objectproperty_selectmodelbaseinstead)
                glLoadName(path->getModelSelectionHandle());
            else
                glLoadName(path->getObjectHandle());
        }
        else
            glLoadName(-1);

        if ( (displayAttrib&sim_displayattribute_forcewireframe)&&(displayAttrib&sim_displayattribute_renderpass) )
            glPolygonMode (GL_FRONT_AND_BACK,GL_LINE);

#ifdef SIM_WITH_GUI
        if ( (App::mainWindow!=nullptr)&&(App::mainWindow->editModeContainer->getEditModePath_old()==path) )
            App::mainWindow->editModeContainer->getEditModePathContainer_old()->render(true,0,false,path->getObjectHandle());
        else
#endif
        {
            _enableAuxClippingPlanes(path->getObjectHandle());
            if ((displayAttrib&sim_displayattribute_forvisionsensor)==0)
            {
                bool isUniqueSelectedPath=false;
#ifdef SIM_WITH_GUI
                if (App::mainWindow!=nullptr)
                    isUniqueSelectedPath=App::mainWindow->editModeContainer->pathPointManipulation->getUniqueSelectedPathId_nonEditMode()!=-1;
#endif
                path->pathContainer->render(false,displayAttrib,isUniqueSelectedPath,path->getObjectHandle());
            }

            if (path->getShapingEnabled())
            {
                path->getShapingColor()->makeCurrentColor((displayAttrib&sim_displayattribute_useauxcomponent)!=0);
                glBegin(GL_TRIANGLES);
                for (int i=0;i<int(path->_pathShapeIndices.size());i++)
                {
                    glNormal3dv(&path->_pathShapeNormals[3*i]);
                    glVertex3dv(&path->_pathShapeVertices[3*(path->_pathShapeIndices[i])]);
                }
                glEnd();
            }
            _disableAuxClippingPlanes();
        }

        glDisable(GL_CULL_FACE);
    }

    // At the end of every 3DObject display routine:
    _commonFinish(path,renderingObject);
}

#else

void displayPath(CPath_old* path,CViewableBase* renderingObject,int displayAttrib)
{

}

#endif
