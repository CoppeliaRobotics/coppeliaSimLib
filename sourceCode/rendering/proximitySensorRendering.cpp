#include "proximitySensorRendering.h"

#ifdef SIM_WITH_OPENGL

void displayProximitySensor(CProxSensor* proxSensor,CViewableBase* renderingObject,int displayAttrib)
{
    // At the beginning of every 3DObject display routine:
    _commonStart(proxSensor,renderingObject,displayAttrib);

    // Display the bounding box:
    if (displayAttrib&sim_displayattribute_renderpass)
        _displayBoundingBox(proxSensor,displayAttrib,true,0.0);

    C3Vector normalVectorForLinesAndPoints(proxSensor->getFullCumulativeTransformation().Q.getInverse()*C3Vector::unitZVector);

    // Display the object:
    if (proxSensor->getShouldObjectBeDisplayed(renderingObject->getObjectHandle(),displayAttrib))
    {
        _enableAuxClippingPlanes(proxSensor->getObjectHandle());
        if ((App::getEditModeType()&SHAPE_OR_PATH_EDIT_MODE_OLD)==0)
        {
            if (proxSensor->getObjectProperty()&sim_objectproperty_selectmodelbaseinstead)
                glLoadName(proxSensor->getModelSelectionHandle());
            else
                glLoadName(proxSensor->getObjectHandle());
        }
        else
            glLoadName(-1);

        if ( (displayAttrib&sim_displayattribute_forcewireframe)&&(displayAttrib&sim_displayattribute_renderpass) )
            glPolygonMode (GL_FRONT_AND_BACK,GL_LINE);

        ogl::setMaterialColor(ogl::colorBlack,ogl::colorBlack,ogl::colorBlack);
        if (proxSensor->getIsDetectedPointValid())
        {
            if (!proxSensor->getHideDetectionRay())
            {
                proxSensor->getColor(1)->makeCurrentColor((displayAttrib&sim_displayattribute_useauxcomponent)!=0);
                glLineWidth(3.0);
                ogl::buffer.clear();
                ogl::addBuffer3DPoints(0.0,0.0,0.0);
                ogl::addBuffer3DPoints(proxSensor->getDetectedPoint().data);
                ogl::drawRandom3dLines(&ogl::buffer[0],2,false,normalVectorForLinesAndPoints.data);
                ogl::buffer.clear();
                glLineWidth(1.0);

                if ( (proxSensor->getSensorType()==sim_proximitysensor_ray_subtype)&&proxSensor->getRandomizedDetection() )
                {
                    ogl::buffer.clear();
                    const std::vector<C3Vector>& _randomizedVectors=proxSensor->getPointerToRandomizedRays()[0];
                    const std::vector<double>& _randomizedVectorDetectionStates=proxSensor->getPointerToRandomizedRayDetectionStates()[0];
                    for (size_t i=0;i<_randomizedVectors.size();i++)
                    {
                        if (_randomizedVectorDetectionStates[i]!=0.0)
                        {
                            ogl::addBuffer3DPoints(0.0,0.0,0.0);
                            ogl::addBuffer3DPoints(_randomizedVectors[i](0)*_randomizedVectorDetectionStates[i],_randomizedVectors[i](1)*_randomizedVectorDetectionStates[i],_randomizedVectors[i](2)*_randomizedVectorDetectionStates[i]);
                        }
                    }
                    ogl::drawRandom3dLines(&ogl::buffer[0],2,false,normalVectorForLinesAndPoints.data);
                    ogl::buffer.clear();
                }
            }
        }
        proxSensor->getColor(0)->makeCurrentColor((displayAttrib&sim_displayattribute_useauxcomponent)!=0);

        ogl::drawSphere(proxSensor->getProxSensorSize()/2.0,10,5,true);

        if (proxSensor->getShowVolume())
        {
            if (proxSensor->convexVolume->volumeEdges.size()!=0)
                ogl::drawRandom3dLines(&proxSensor->convexVolume->volumeEdges[0],(int)proxSensor->convexVolume->volumeEdges.size()/3,false,normalVectorForLinesAndPoints.data);
            float ccc[3]={0.2,0.2,0.2};
            ogl::setMaterialColor(ccc,ccc,ccc);
            if (proxSensor->convexVolume->nonDetectingVolumeEdges.size()!=0)
                ogl::drawRandom3dLines(&proxSensor->convexVolume->nonDetectingVolumeEdges[0],(int)proxSensor->convexVolume->nonDetectingVolumeEdges.size()/3,false,normalVectorForLinesAndPoints.data);
        }


        ogl::setMaterialColor(ogl::colorBlack,ogl::colorBlack,ogl::colorRed);
        if (proxSensor->convexVolume->normalsInside.size()!=0)
            ogl::drawRandom3dLines(&proxSensor->convexVolume->normalsInside[0],(int)proxSensor->convexVolume->normalsInside.size()/3,false,normalVectorForLinesAndPoints.data);

        ogl::setMaterialColor(sim_colorcomponent_emission,ogl::colorBlue);
        if (proxSensor->convexVolume->normalsOutside.size()!=0)
            ogl::drawRandom3dLines(&proxSensor->convexVolume->normalsOutside[0],(int)proxSensor->convexVolume->normalsOutside.size()/3,false,normalVectorForLinesAndPoints.data);

        _disableAuxClippingPlanes();
    }

    // At the end of every 3DObject display routine:
    _commonFinish(proxSensor,renderingObject);
}

#else

void displayProximitySensor(CProxSensor* proxSensor,CViewableBase* renderingObject,int displayAttrib)
{

}

#endif



