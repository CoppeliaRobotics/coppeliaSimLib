#include <markerRendering.h>
#include <guiApp.h>

void displayMarker(CMarker* marker, CViewableBase* renderingObject, int displayAttrib, bool overlay)
{
    // At the beginning of every scene object display routine:
    _commonStart(marker, renderingObject, true);
    C3Vector normalVectorForLinesAndPoints(marker->getFullCumulativeTransformation().Q.getInverse() * C3Vector::unitZVector);

    // Object display:
    if (marker->getShouldObjectBeDisplayed(renderingObject->getObjectHandle(), displayAttrib))
    {
        if ((GuiApp::getEditModeType() & SHAPE_OR_PATH_EDIT_MODE_OLD) == 0)
        {
            if (marker->getObjectProperty() & sim_objectproperty_selectmodelbaseinstead)
                glLoadName(marker->getModelSelectionHandle());
            else
                glLoadName(marker->getObjectHandle());
        }
        else
            glLoadName(-1);

        if ((displayAttrib & sim_displayattribute_forcewireframe) && (displayAttrib & sim_displayattribute_renderpass))
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        if ((displayAttrib & sim_displayattribute_forcewireframe) == 0)
            glEnable(GL_CULL_FACE);

        _enableAuxClippingPlanes(marker->getObjectHandle());
//        double s = marker->getMarkerSize();
//        ogl::drawReference(s);
//        marker->getMarkerColor()->makeCurrentColor((displayAttrib & sim_displayattribute_useauxcomponent) != 0);
//        ogl::drawBox(s / 2.0, s / 2.0, s / 2.0, true, normalVectorForLinesAndPoints.ptr());
        marker->drawItems(displayAttrib, normalVectorForLinesAndPoints.data, overlay);
        glDisable(GL_CULL_FACE);
        _disableAuxClippingPlanes();
    }

           // At the end of every scene object display routine:
    _commonFinish(marker, renderingObject);
}
