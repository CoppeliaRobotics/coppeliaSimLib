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

#include <millRendering.h>

#ifdef SIM_WITH_OPENGL

void displayMill(CMill* mill,CViewableBase* renderingObject,int displayAttrib)
{
    // At the beginning of every 3DObject display routine:
    _commonStart(mill,renderingObject,displayAttrib);

    // Display the bounding box:
    if (displayAttrib&sim_displayattribute_renderpass)
        _displayBoundingBox(mill,displayAttrib,true,0.0);

    C3Vector normalVectorForLinesAndPoints(mill->getFullCumulativeTransformation().Q.getInverse()*C3Vector::unitZVector);

    // Display the object:
    if (mill->getShouldObjectBeDisplayed(renderingObject->getObjectHandle(),displayAttrib))
    {
        if ((App::getEditModeType()&SHAPE_OR_PATH_EDIT_MODE_OLD)==0)
        {
            if (mill->getObjectProperty()&sim_objectproperty_selectmodelbaseinstead)
                glLoadName(mill->getModelSelectionHandle());
            else
                glLoadName(mill->getObjectHandle());
        }
        else
            glLoadName(-1);

        _enableAuxClippingPlanes(mill->getObjectHandle());
        bool wire=false;
        if ( (displayAttrib&sim_displayattribute_forcewireframe)&&(displayAttrib&sim_displayattribute_renderpass) )
        {
            wire=true;
            glPolygonMode (GL_FRONT_AND_BACK,GL_LINE);
        }

        int _milledObjectCount;
        if (mill->getMilledCount(_milledObjectCount)&&(_milledObjectCount>0))
            mill->getColor(true)->makeCurrentColor((displayAttrib&sim_displayattribute_useauxcomponent)!=0);
        else
            mill->getColor(false)->makeCurrentColor((displayAttrib&sim_displayattribute_useauxcomponent)!=0);
        double _size=mill->getSize();
        ogl::drawBox(_size/2.0,_size/2.0,_size/2.0,!wire,normalVectorForLinesAndPoints.data);

        if (mill->convexVolume->volumeEdges.size()!=0)
            ogl::drawRandom3dLines(&mill->convexVolume->volumeEdges[0],(int)mill->convexVolume->volumeEdges.size()/3,false,normalVectorForLinesAndPoints.data);

        ogl::setMaterialColor(ogl::colorBlack,ogl::colorBlack,ogl::colorRed);

        if (mill->convexVolume->normalsInside.size()!=0)
            ogl::drawRandom3dLines(&mill->convexVolume->normalsInside[0],(int)mill->convexVolume->normalsInside.size()/3,false,normalVectorForLinesAndPoints.data);

        _disableAuxClippingPlanes();
    }

    // At the end of every 3DObject display routine:
    _commonFinish(mill,renderingObject);
}

#else

void displayMill(CMill* mill,CViewableBase* renderingObject,int displayAttrib)
{

}

#endif



