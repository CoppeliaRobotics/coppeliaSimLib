#include <scriptRendering.h>
#include <guiApp.h>

void displayScript(CScriptObject* scriptObj, CViewableBase* renderingObject, int displayAttrib)
{
    // At the beginning of every scene object display routine:
    _commonStart(scriptObj, renderingObject);

    C3Vector normalVectorForLinesAndPoints(scriptObj->getFullCumulativeTransformation().Q.getInverse() *
                                           C3Vector::unitZVector);

    // Object display:
    if (scriptObj->getShouldObjectBeDisplayed(renderingObject->getObjectHandle(), displayAttrib))
    {
        if ((GuiApp::getEditModeType() & SHAPE_OR_PATH_EDIT_MODE_OLD) == 0)
        {
            if (scriptObj->getObjectProperty() & sim_objectproperty_selectmodelbaseinstead)
                glLoadName(scriptObj->getModelSelectionHandle());
            else
                glLoadName(scriptObj->getObjectHandle());
        }
        else
            glLoadName(-1);

        if ((displayAttrib & sim_displayattribute_forcewireframe) && (displayAttrib & sim_displayattribute_renderpass))
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        if ((displayAttrib & sim_displayattribute_forcewireframe) == 0)
            glEnable(GL_CULL_FACE);

        _enableAuxClippingPlanes(scriptObj->getObjectHandle());
        ogl::drawReference(scriptObj->getScriptSize());
        scriptObj->getScriptColor()->makeCurrentColor((displayAttrib & sim_displayattribute_useauxcomponent) != 0);
        ogl::drawBox(scriptObj->getScriptSize() / 2.0, scriptObj->getScriptSize() / 2.0, scriptObj->getScriptSize() / 2.0, true, normalVectorForLinesAndPoints.ptr());
        glDisable(GL_CULL_FACE);
        _disableAuxClippingPlanes();
    }

    // At the end of every scene object display routine:
    _commonFinish(scriptObj, renderingObject);
}
