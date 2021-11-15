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
        glPushMatrix();
        glTranslatef(0.0f,0.0f,cameraSize/6.0f);
        ogl::drawCone(cameraSize,5.0f*cameraSize/3.0f,20,true,true);
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
