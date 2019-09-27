
#include "millRendering.h"

#ifdef SIM_WITH_OPENGL

void displayMill(CMill* mill,CViewableBase* renderingObject,int displayAttrib)
{
    // At the beginning of every 3DObject display routine:
    _commonStart(mill,renderingObject,displayAttrib);

    // Display the bounding box:
    if (displayAttrib&sim_displayattribute_renderpass)
        _displayBoundingBox(mill,displayAttrib,true,0.0);

    C3Vector normalVectorForLinesAndPoints(mill->getCumulativeTransformation().Q.getInverse()*C3Vector::unitZVector);

    // Display the object:
    if (mill->getShouldObjectBeDisplayed(renderingObject->getObjectHandle(),displayAttrib))
    {
        if ((App::getEditModeType()&SHAPE_OR_PATH_EDIT_MODE)==0)
        {
            if (mill->getLocalObjectProperty()&sim_objectproperty_selectmodelbaseinstead)
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
        float _size=mill->getSize();
        ogl::drawBox(_size/2.0f,_size/2.0f,_size/2.0f,!wire,normalVectorForLinesAndPoints.data);

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



