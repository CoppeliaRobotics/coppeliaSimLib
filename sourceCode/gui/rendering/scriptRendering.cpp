#include <scriptRendering.h>
#include <guiApp.h>

void displayScript(CScript *script, CViewableBase *renderingObject, int displayAttrib)
{
    // At the beginning of every scene object display routine:
    _commonStart(script, renderingObject);

    C3Vector normalVectorForLinesAndPoints(script->getFullCumulativeTransformation().Q.getInverse() *
                                           C3Vector::unitZVector);

    // Object display:
    if (script->getShouldObjectBeDisplayed(renderingObject->getObjectHandle(), displayAttrib))
    {
        if ((GuiApp::getEditModeType() & SHAPE_OR_PATH_EDIT_MODE_OLD) == 0)
        {
            if (script->getObjectProperty() & sim_objectproperty_selectmodelbaseinstead)
                glLoadName(script->getModelSelectionHandle());
            else
                glLoadName(script->getObjectHandle());
        }
        else
            glLoadName(-1);

        if ((displayAttrib & sim_displayattribute_forcewireframe) && (displayAttrib & sim_displayattribute_renderpass))
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        if ((displayAttrib & sim_displayattribute_forcewireframe) == 0)
            glEnable(GL_CULL_FACE);

        _enableAuxClippingPlanes(script->getObjectHandle());
        ogl::drawReference(script->getScriptSize());
        script->getScriptColor()->makeCurrentColor((displayAttrib & sim_displayattribute_useauxcomponent) != 0);
        ogl::drawBox(script->getScriptSize() / 2.0, script->getScriptSize() / 2.0, script->getScriptSize() / 2.0, true, normalVectorForLinesAndPoints.ptr());
        glDisable(GL_CULL_FACE);
        _disableAuxClippingPlanes();
    }

    // At the end of every scene object display routine:
    _commonFinish(script, renderingObject);
}
