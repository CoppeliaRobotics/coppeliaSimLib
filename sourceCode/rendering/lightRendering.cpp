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

#include "lightRendering.h"

#ifdef SIM_WITH_OPENGL

void displayLight(CLight* light,CViewableBase* renderingObject,int displayAttrib)
{
    // At the beginning of every 3DObject display routine:
    _commonStart(light,renderingObject,displayAttrib);

    // Bounding box display:
    if (displayAttrib&sim_displayattribute_renderpass)
        _displayBoundingBox(light,displayAttrib,true,0.0);

    // Object display:
    if (light->getShouldObjectBeDisplayed(renderingObject->getObjectHandle(),displayAttrib))
    {
        if ((App::getEditModeType()&SHAPE_OR_PATH_EDIT_MODE_OLD)==0)
        {
            if (light->getObjectProperty()&sim_objectproperty_selectmodelbaseinstead)
                glLoadName(light->getModelSelectionHandle());
            else
                glLoadName(light->getObjectHandle());
        }
        else
            glLoadName(-1);

        if ( (displayAttrib&sim_displayattribute_forcewireframe)&&(displayAttrib&sim_displayattribute_renderpass) )
            glPolygonMode (GL_FRONT_AND_BACK,GL_LINE);

        _enableAuxClippingPlanes(light->getObjectHandle());
        C3Vector normalizedAmbientColor;
        normalizedAmbientColor.setData(light->getColor(true)->getColorsPtr());
        float m=std::max<float>(std::max<float>(normalizedAmbientColor(0),normalizedAmbientColor(1)),normalizedAmbientColor(2));
        if (m>0.00001)
            normalizedAmbientColor/=m;
        C3Vector normalizedDiffuseColor;
        normalizedDiffuseColor.setData(light->getColor(true)->getColorsPtr()+3);
        m=std::max<float>(std::max<float>(normalizedDiffuseColor(0),normalizedDiffuseColor(1)),normalizedDiffuseColor(2));
        if (m>0.00001)
            normalizedDiffuseColor/=m;
        C3Vector normalizedSpecularColor;
        normalizedSpecularColor.setData(light->getColor(true)->getColorsPtr()+6);
        m=std::max<float>(std::max<float>(normalizedSpecularColor(0),normalizedSpecularColor(1)),normalizedSpecularColor(2));
        if (m>0.00001)
            normalizedSpecularColor/=m;
        normalizedAmbientColor+=normalizedDiffuseColor*0.2+normalizedSpecularColor*0.1;
        m=std::max<float>(std::max<float>(normalizedAmbientColor(0),normalizedAmbientColor(1)),normalizedAmbientColor(2));
        if (m>0.00001)
            normalizedAmbientColor/=m;
        float lightEmission[3]={0.0,0.0,0.0};
        for (int i=0;i<3;i++)
        {
            if ((displayAttrib&sim_displayattribute_useauxcomponent)!=0)
                lightEmission[i]=light->getColor(true)->getColorsPtr()[12+i];
            else
                lightEmission[i]=normalizedAmbientColor(i);
        }
        if (!light->getLightActive())
        {
            if ((displayAttrib&sim_displayattribute_useauxcomponent)!=0)
            { // we automatically adjust the temperature and light channels:
                lightEmission[0]=0.5;
                lightEmission[2]=0.0;
            }
            else
            {
                lightEmission[0]=0.0;
                lightEmission[1]=0.0;
                lightEmission[2]=0.0;
            }
        }
        double _lightSize=light->getLightSize();
        if (light->getLightType()==sim_light_omnidirectional_subtype)
        {   // Omnidirectional light
            if (light->getLightActive())
                ogl::setMaterialColor(ogl::colorBlack,ogl::colorBlack,lightEmission);
            else
                ogl::setMaterialColor(ogl::colorDarkGrey,ogl::colorDarkGrey,lightEmission);
            ogl::drawSphere(0.5*_lightSize,20,10,true);
        }
        if (light->getLightType()==sim_light_spot_subtype)
        {   //spotLight
            light->getColor(false)->makeCurrentColor((displayAttrib&sim_displayattribute_useauxcomponent)!=0);
            glPushMatrix();
            glTranslated(0.0,0.0,-0.5*_lightSize);
            ogl::drawCone(1.6*_lightSize,2.0*_lightSize,20,true,true);
            glTranslated(0.0,0.0,0.5*_lightSize);
            if (light->getLightActive())
                ogl::setMaterialColor(ogl::colorBlack,ogl::colorBlack,lightEmission);
            else
                ogl::setMaterialColor(ogl::colorDarkGrey,ogl::colorDarkGrey,lightEmission);
            ogl::drawSphere(0.5*_lightSize,20,10,true);
            glPopMatrix();
        }
        if (light->getLightType()==sim_light_directional_subtype)
        {   // Directional light
            glPushMatrix();
                glTranslated(0.0,0.0,0.5*_lightSize);
                if (light->getLightActive())
                    ogl::setMaterialColor(ogl::colorBlack,ogl::colorBlack,lightEmission);
                else
                    ogl::setMaterialColor(ogl::colorDarkGrey,ogl::colorDarkGrey,lightEmission);
                ogl::drawDisk(_lightSize,20);
                light->getColor(false)->makeCurrentColor((displayAttrib&sim_displayattribute_useauxcomponent)!=0);
                glTranslated(0.0,0.0,-_lightSize/2.0);
                ogl::drawCylinder(_lightSize,_lightSize,20,1,true);

            glPopMatrix();
        }
        _disableAuxClippingPlanes();
    }

    // At the end of every 3DObject display routine:
    _commonFinish(light,renderingObject);
}

#else

void displayLight(CLight* light,CViewableBase* renderingObject,int displayAttrib)
{

}

#endif



