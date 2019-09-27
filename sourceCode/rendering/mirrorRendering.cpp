
#include "mirrorRendering.h"

#ifdef SIM_WITH_OPENGL

void displayMirror(CMirror* mirror,CViewableBase* renderingObject,int displayAttrib)
{
    // At the beginning of every 3DObject display routine:
    _commonStart(mirror,renderingObject,displayAttrib);

    // Bounding box display:
    if (displayAttrib&sim_displayattribute_renderpass)
        _displayBoundingBox(mirror,displayAttrib,true,0.0);

    // Object display:
    if (mirror->getShouldObjectBeDisplayed(renderingObject->getObjectHandle(),displayAttrib))
    {
        if ((App::getEditModeType()&SHAPE_OR_PATH_EDIT_MODE)==0)
        {
            if (mirror->getLocalObjectProperty()&sim_objectproperty_selectmodelbaseinstead)
                glLoadName(mirror->getModelSelectionHandle());
            else
                glLoadName(mirror->getObjectHandle());
        }
        else
            glLoadName(-1);

        if ( (displayAttrib&sim_displayattribute_forcewireframe)&&(displayAttrib&sim_displayattribute_renderpass) )
            glPolygonMode (GL_FRONT_AND_BACK,GL_LINE);

        _enableAuxClippingPlanes(mirror->getObjectHandle());
        if (((displayAttrib&sim_displayattribute_pickpass)||((mirror->currentMirrorContentBeingRendered!=mirror->getObjectHandle())&&(mirror->currentMirrorContentBeingRendered!=-1)))&&mirror->getIsMirror())
        {
            ogl::disableLighting_useWithCare(); // only temporarily
            ogl::setMaterialColor(mirror->mirrorColor,ogl::colorBlack,ogl::colorBlack);
            ogl::drawPlane(mirror->getMirrorWidth(),mirror->getMirrorHeight());
            ogl::enableLighting_useWithCare();
        }
        _disableAuxClippingPlanes();
        // For this part we want the clipping planes disabled:
        if (!mirror->getIsMirror())
        {
            mirror->getClipPlaneColor()->makeCurrentColor((displayAttrib&sim_displayattribute_useauxcomponent)!=0);
            ogl::drawPlane(mirror->getMirrorWidth(),mirror->getMirrorHeight());
        }
    }

    // At the end of every 3DObject display routine:
    _commonFinish(mirror,renderingObject);
}

#else

void displayMirror(CMirror* mirror,CViewableBase* renderingObject,int displayAttrib)
{

}

#endif



