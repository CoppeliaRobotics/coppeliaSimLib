
#include "cameraRendering.h"

#ifdef SIM_WITH_OPENGL
#include "pluginContainer.h"

void displayCamera(CCamera* camera,CViewableBase* renderingObject,int displayAttrib)
{
    // At the beginning of every 3DObject display routine:
    _commonStart(camera,renderingObject,displayAttrib);

    // Bounding box display:
    if (displayAttrib&sim_displayattribute_renderpass)
        _displayBoundingBox(camera,displayAttrib,true,0.0);

    // Object display:
    if (camera->getShouldObjectBeDisplayed(renderingObject->getObjectHandle(),displayAttrib))
    {
        if ((App::getEditModeType()&SHAPE_OR_PATH_EDIT_MODE)==0)
        {
            if (camera->getLocalObjectProperty()&sim_objectproperty_selectmodelbaseinstead)
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
        float cameraSize=camera->getCameraSize();
        glPushMatrix();
        glTranslatef(0.0f,0.0f,-cameraSize);
        ogl::drawBox(0.4f*cameraSize,cameraSize,2.0f*cameraSize,true,nullptr);
        glPopMatrix();
        glPushMatrix();
        camera->getColor(true)->makeCurrentColor((displayAttrib&sim_displayattribute_useauxcomponent)!=0);
        glTranslatef(0.0f,1.3f*cameraSize,-0.25f*cameraSize);
        glRotatef(90.0f,0,1,0);
        ogl::drawCylinder(2.0f*cameraSize,cameraSize/2.0f,20,0,true);
        glTranslatef(1.5f*cameraSize,0.0f,0.0f);
        ogl::drawCylinder(2.0f*cameraSize,cameraSize/2.0f,20,0,true);
        glPopMatrix();
        glTranslatef(0.0f,0.0f,cameraSize/6.0f);
        ogl::drawCone(cameraSize,5.0f*cameraSize/3.0f,20,true,true);
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
