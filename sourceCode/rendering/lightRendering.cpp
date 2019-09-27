
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
        if ((App::getEditModeType()&SHAPE_OR_PATH_EDIT_MODE)==0)
        {
            if (light->getLocalObjectProperty()&sim_objectproperty_selectmodelbaseinstead)
                glLoadName(light->getModelSelectionHandle());
            else
                glLoadName(light->getObjectHandle());
        }
        else
            glLoadName(-1);

        if ( (displayAttrib&sim_displayattribute_forcewireframe)&&(displayAttrib&sim_displayattribute_renderpass) )
            glPolygonMode (GL_FRONT_AND_BACK,GL_LINE);

        _enableAuxClippingPlanes(light->getObjectHandle());
        C3Vector normalizedAmbientColor(light->getColor(true)->colors);
        float m=SIM_MAX(SIM_MAX(normalizedAmbientColor(0),normalizedAmbientColor(1)),normalizedAmbientColor(2));
        if (m>0.00001f)
            normalizedAmbientColor/=m;
        C3Vector normalizedDiffuseColor(light->getColor(true)->colors+3);
        m=SIM_MAX(SIM_MAX(normalizedDiffuseColor(0),normalizedDiffuseColor(1)),normalizedDiffuseColor(2));
        if (m>0.00001f)
            normalizedDiffuseColor/=m;
        C3Vector normalizedSpecularColor(light->getColor(true)->colors+6);
        m=SIM_MAX(SIM_MAX(normalizedSpecularColor(0),normalizedSpecularColor(1)),normalizedSpecularColor(2));
        if (m>0.00001f)
            normalizedSpecularColor/=m;
        normalizedAmbientColor+=normalizedDiffuseColor*0.2f+normalizedSpecularColor*0.1f;
        m=SIM_MAX(SIM_MAX(normalizedAmbientColor(0),normalizedAmbientColor(1)),normalizedAmbientColor(2));
        if (m>0.00001f)
            normalizedAmbientColor/=m;
//      float black[3]={0.0f,0.0f,0.0f};
//      float grey[3]={0.2f,0.2f,0.2f};
        float lightEmission[3]={0.0f,0.0f,0.0f};
        for (int i=0;i<3;i++)
        {
            if ((displayAttrib&sim_displayattribute_useauxcomponent)!=0)
                lightEmission[i]=light->getColor(true)->colors[12+i];
            else
                lightEmission[i]=normalizedAmbientColor(i);
        }
        if (!light->getLightActive())
        {
            if ((displayAttrib&sim_displayattribute_useauxcomponent)!=0)
            { // we automatically adjust the temperature and light channels:
                lightEmission[0]=0.5f;
                lightEmission[2]=0.0f;
            }
            else
            {
                lightEmission[0]=0.0f;
                lightEmission[1]=0.0f;
                lightEmission[2]=0.0f;
            }
        }
        float _lightSize=light->getLightSize();
        if (light->getLightType()==sim_light_omnidirectional_subtype)
        {   // Omnidirectional light
            if (light->getLightActive())
                ogl::setMaterialColor(ogl::colorBlack,ogl::colorBlack,lightEmission);
            else
                ogl::setMaterialColor(ogl::colorDarkGrey,ogl::colorDarkGrey,lightEmission);
            ogl::drawSphere(0.5f*_lightSize,20,10,true);
        }
        if (light->getLightType()==sim_light_spot_subtype)
        {   //spotLight
            light->getColor(false)->makeCurrentColor((displayAttrib&sim_displayattribute_useauxcomponent)!=0);
            glPushMatrix();
            glTranslatef(0.0f,0.0f,-0.5f*_lightSize);
            ogl::drawCone(1.6f*_lightSize,2.0f*_lightSize,20,true,true);
            glTranslatef(0.0f,0.0f,0.5f*_lightSize);
            if (light->getLightActive())
                ogl::setMaterialColor(ogl::colorBlack,ogl::colorBlack,lightEmission);
            else
                ogl::setMaterialColor(ogl::colorDarkGrey,ogl::colorDarkGrey,lightEmission);
            ogl::drawSphere(0.5f*_lightSize,20,10,true);
            glPopMatrix();
        }
        if (light->getLightType()==sim_light_directional_subtype)
        {   // Directional light
            glPushMatrix();
                glTranslatef(0.0f,0.0f,0.5f*_lightSize);
                if (light->getLightActive())
                    ogl::setMaterialColor(ogl::colorBlack,ogl::colorBlack,lightEmission);
                else
                    ogl::setMaterialColor(ogl::colorDarkGrey,ogl::colorDarkGrey,lightEmission);
                ogl::drawDisk(_lightSize,20);
                light->getColor(false)->makeCurrentColor((displayAttrib&sim_displayattribute_useauxcomponent)!=0);
                glTranslatef(0.0f,0.0f,-_lightSize/2.0f);
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



