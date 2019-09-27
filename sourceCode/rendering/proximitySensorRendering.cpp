
#include "proximitySensorRendering.h"

#ifdef SIM_WITH_OPENGL

void displayProximitySensor(CProxSensor* proxSensor,CViewableBase* renderingObject,int displayAttrib)
{
    // At the beginning of every 3DObject display routine:
    _commonStart(proxSensor,renderingObject,displayAttrib);

    // Display the bounding box:
    if (displayAttrib&sim_displayattribute_renderpass)
        _displayBoundingBox(proxSensor,displayAttrib,true,0.0);

    C3Vector normalVectorForLinesAndPoints(proxSensor->getCumulativeTransformation().Q.getInverse()*C3Vector::unitZVector);

    // Display the object:
    if (proxSensor->getShouldObjectBeDisplayed(renderingObject->getObjectHandle(),displayAttrib))
    {
        _enableAuxClippingPlanes(proxSensor->getObjectHandle());
        if ((App::getEditModeType()&SHAPE_OR_PATH_EDIT_MODE)==0)
        {
            if (proxSensor->getLocalObjectProperty()&sim_objectproperty_selectmodelbaseinstead)
                glLoadName(proxSensor->getModelSelectionHandle());
            else
                glLoadName(proxSensor->getObjectHandle());
        }
        else
            glLoadName(-1);

        if ( (displayAttrib&sim_displayattribute_forcewireframe)&&(displayAttrib&sim_displayattribute_renderpass) )
            glPolygonMode (GL_FRONT_AND_BACK,GL_LINE);

        ogl::setMaterialColor(ogl::colorBlack,ogl::colorBlack,ogl::colorBlack);
/*
        if (CIloIlo::debugSensorCutting&&(sensorType!=sim_proximitysensor_ray_subtype))
        {
            ogl::setMaterialColor(sim_colorcomponent_emission,ogl::colorWhite);
            ogl::setBlending(true);
            glDepthMask(GL_FALSE);
            ogl::buffer.clear();
            for (int klm=0;klm<int(cutEdges.size())/3;klm++)
                ogl::addBuffer3DPoints(&cutEdges[3*klm]);
            if (ogl::buffer.size()!=0)
                ogl::drawRandom3dLines(&ogl::buffer[0],ogl::buffer.size(),true,normalVectorForLinesAndPoints.data);
            ogl::buffer.clear();
            ogl::setBlending(false);
            glDepthMask(GL_TRUE);
        }
*/
        if (proxSensor->getIsDetectedPointValid())
        {
            if (!proxSensor->getHideDetectionRay())
            {
                proxSensor->getColor(2)->makeCurrentColor((displayAttrib&sim_displayattribute_useauxcomponent)!=0);
                glLineWidth(3.0f);
                ogl::buffer.clear();
                ogl::addBuffer3DPoints(0.0f,0.0f,0.0f);
                ogl::addBuffer3DPoints(proxSensor->getDetectedPoint().data);
                ogl::drawRandom3dLines(&ogl::buffer[0],2,false,normalVectorForLinesAndPoints.data);
                ogl::buffer.clear();
                glLineWidth(1.0f);

                if ( (proxSensor->getSensorType()==sim_proximitysensor_ray_subtype)&&proxSensor->getRandomizedDetection() )
                {
                    ogl::buffer.clear();
                    const std::vector<C3Vector>& _randomizedVectors=proxSensor->getPointerToRandomizedRays()[0];
                    const std::vector<float>& _randomizedVectorDetectionStates=proxSensor->getPointerToRandomizedRayDetectionStates()[0];
                    for (size_t i=0;i<_randomizedVectors.size();i++)
                    {
                        if (_randomizedVectorDetectionStates[i]!=0.0f)
                        {
                            ogl::addBuffer3DPoints(0.0f,0.0f,0.0f);
                            ogl::addBuffer3DPoints(_randomizedVectors[i](0)*_randomizedVectorDetectionStates[i],_randomizedVectors[i](1)*_randomizedVectorDetectionStates[i],_randomizedVectors[i](2)*_randomizedVectorDetectionStates[i]);
                        }
                    }
                    ogl::drawRandom3dLines(&ogl::buffer[0],2,false,normalVectorForLinesAndPoints.data);
                    ogl::buffer.clear();
                }
            }
            proxSensor->getColor(1)->makeCurrentColor((displayAttrib&sim_displayattribute_useauxcomponent)!=0);
        }
        else
            proxSensor->getColor(0)->makeCurrentColor((displayAttrib&sim_displayattribute_useauxcomponent)!=0);

        if ((proxSensor->getShowVolumeWhenDetecting()&&proxSensor->getIsDetectedPointValid())||(proxSensor->getShowVolumeWhenNotDetecting()&&(!proxSensor->getIsDetectedPointValid())))
        {
            ogl::drawSphere(proxSensor->getSize()/2.0f,10,5,true);
            if (proxSensor->convexVolume->volumeEdges.size()!=0)
                ogl::drawRandom3dLines(&proxSensor->convexVolume->volumeEdges[0],(int)proxSensor->convexVolume->volumeEdges.size()/3,false,normalVectorForLinesAndPoints.data);
            proxSensor->getColor(3)->makeCurrentColor((displayAttrib&sim_displayattribute_useauxcomponent)!=0);
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



