#include <simInternal.h>
#include <hierarchyElement.h>
#include <oGL.h>
#include <imgLoaderSaver.h>
#include <app.h>
#include <tt.h>
#include <utils.h>
#include <mesh.h>
#include <boost/lexical_cast.hpp>
#ifdef SIM_WITH_GUI
#include <guiApp.h>
#endif

const int CONST_VAL_6 = 6;

CHierarchyElement::CHierarchyElement(int theObjectID)
{
    children.clear();
    objectID = theObjectID;
}

CHierarchyElement::~CHierarchyElement()
{
    for (int i = 0; i < int(children.size()); i++)
        delete children[i];
    children.clear();
}

void CHierarchyElement::setSceneName(const char* sn)
{
    if (strlen(sn) == 0)
        _sceneName = "new scene";
    else
        _sceneName = sn;
}

CHierarchyElement* CHierarchyElement::getElementLinkedWithObject(int objID)
{
    if (objectID == objID)
        return (this);
    CHierarchyElement* retVal = nullptr;
    for (int i = 0; i < int(children.size()); i++)
    {
        retVal = children[i]->getElementLinkedWithObject(objID);
        if (retVal != nullptr)
            return (retVal);
    }
    return (nullptr);
}

void CHierarchyElement::addYourChildren()
{
    if (GuiApp::getEditModeType() == NO_EDIT_MODE)
    {
        if ((objectID < 0) && isLocalWorld())
        { // this is the world!
            for (size_t i = 0; i < App::currentWorld->sceneObjects->getOrphanCount(); i++)
            {
                CSceneObject* it = App::currentWorld->sceneObjects->getOrphanFromIndex(i);
                CHierarchyElement* aKid = new CHierarchyElement(it->getObjectHandle());
                aKid->addYourChildren();
                children.push_back(aKid);
            }
        }
        else
        { // this is a scene object!
            CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(objectID);
            if (it != nullptr)
            {
                for (size_t i = 0; i < it->getChildCount(); i++)
                {
                    CSceneObject* child = it->getChildFromIndex(i);
                    CHierarchyElement* aKid = new CHierarchyElement(child->getObjectHandle());
                    aKid->addYourChildren();
                    children.push_back(aKid);
                }
            }
        }
    }
}

int CHierarchyElement::getLinkedObjectID()
{
    return (objectID);
}

void CHierarchyElement::renderElement_sceneObject(CHierarchy* hier, int labelEditObjectID, bool& bright,
                                                  bool dontDisplay, int renderingSize[2], int textPos[2], int indentNb,
                                                  std::vector<int>* vertLines, int minRenderedPos[2],
                                                  int maxRenderedPos[2], bool forDragAndDrop /*=false*/,
                                                  int transparentForTreeObjects /*=-1*/, int dropID /*=-1*/,
                                                  int worldClick /*=-9999*/)
{ // transparentForTreeObjects==-1: normal, transparentForTreeObjects==-2: transparent, otherwise transparent only for
    // objID=transparentForTreeObjects
    const unsigned char horizontalShift = 13 * GuiApp::sc;
    double transparencyFactor = 0.0;
    if ((transparentForTreeObjects >= 0) && (transparentForTreeObjects == objectID))
        transparentForTreeObjects = -2; // from here on, everything is transparent
    if (transparentForTreeObjects == -2)
        transparencyFactor = 0.875;

    bool isOtherWorld = ((objectID < 0) && (!isLocalWorld()));

    if (!forDragAndDrop)
    {
        bright = !bright;
        if (textPos[0] < minRenderedPos[0])
            minRenderedPos[0] = textPos[0];
        if (textPos[1] < minRenderedPos[1])
            minRenderedPos[1] = textPos[1];
        if (textPos[0] > maxRenderedPos[0])
            maxRenderedPos[0] = textPos[0];
        if (textPos[1] > maxRenderedPos[1])
            maxRenderedPos[1] = textPos[1];

        if (textPos[1] < -HIERARCHY_INTER_LINE_SPACE * GuiApp::sc)
            dontDisplay = true; // We don't display what is outside of the view!
    }

    bool textInside = (textPos[1] < renderingSize[1] + HIERARCHY_INTER_LINE_SPACE * GuiApp::sc);
    CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(objectID);
    std::string theText;
    if (it != nullptr)
    {
        //        theText=it->getObjectName();
        theText = it->getObjectAliasAndOrderIfRequired();
        /*
        if (App::currentWorld->sceneObjects->isObjectInSelection(objectID))
        {
            theText+=" (";
            theText+=it->getObjectName_old();
            theText+=")";
        }
        */
    }
    else
    {
        theText += _sceneName;
        theText += tt::decorateString(" (scene ", utils::getIntString(false, -objectID), ")");
    }

    bool inSelection = false;
    float dummyCol[3] = {0.0, 0.0, 0.0};
    float* bgCol = dummyCol;
    if (!forDragAndDrop)
    {
        bool hasAColor = false;
        if (dropID != -9999)
        {
            if ((transparentForTreeObjects != -2) && (dropID == objectID))
            {
                bgCol = ogl::HIERARCHY_DROP_LOCATION_COLOR;
                hasAColor = true;
            }
        }
        else
        {
            if (App::currentWorld->sceneObjects->isObjectInSelection(objectID) && textInside && (!dontDisplay))
            {
                if (App::currentWorld->sceneObjects->getLastSelectionObject() == it)
                    bgCol = ogl::HIERARCHY_AND_BROWSER_LAST_SELECTION_COLOR;
                else
                {
                    if (bright)
                        bgCol = ogl::HIERARCHY_NOT_LAST_SELECTION_COLOR_BRIGHT;
                    else
                        bgCol = ogl::HIERARCHY_NOT_LAST_SELECTION_COLOR_DARK;
                }
                inSelection = true;
                hasAColor = true;
            }
        }
        if (textInside && (!dontDisplay) && (!hasAColor))
        {
            if (it == nullptr)
            { // world
                if (worldClick == objectID)
                    bgCol = ogl::HIERARCHY_WORLD_CLICK_COLOR;
                else
                {
                    if (isLocalWorld())
                    {
                        if (App::userSettings->darkMode)
                            bgCol = ogl::HIERARCHY_AND_BROWSER_NO_SELECTION_COLOR_DARK;
                        else
                            bgCol = ogl::HIERARCHY_AND_BROWSER_NO_SELECTION_COLOR_BRIGHT;
                    }
                    else
                        bgCol = ogl::HIERARCHY_UNACTIVE_WORLD_COLOR;
                }
            }
            else
            {
                int colIndex = it->getHierarchyColorIndex();
                if (colIndex == -1)
                {
                    if (App::userSettings->darkMode)
                        bgCol = ogl::HIERARCHY_AND_BROWSER_NO_SELECTION_COLOR_DARK;
                    else
                        bgCol = ogl::HIERARCHY_AND_BROWSER_NO_SELECTION_COLOR_BRIGHT;
                }
                else
                {
                    if (colIndex == 0)
                    {
                        if (!bright)
                            bgCol = ogl::HIERARCHY_NO_SELECTION_RED_DARK;
                        else
                            bgCol = ogl::HIERARCHY_NO_SELECTION_RED_BRIGHT;
                    }
                    if (colIndex == 1)
                    {
                        if (!bright)
                            bgCol = ogl::HIERARCHY_NO_SELECTION_GREEN_DARK;
                        else
                            bgCol = ogl::HIERARCHY_NO_SELECTION_GREEN_BRIGHT;
                    }
                    if (colIndex == 2)
                    {
                        if (!bright)
                            bgCol = ogl::HIERARCHY_NO_SELECTION_BLUE_DARK;
                        else
                            bgCol = ogl::HIERARCHY_NO_SELECTION_BLUE_BRIGHT;
                    }
                }
            }
        }

        if (!forDragAndDrop)
        {
            ogl::setMaterialColor(sim_colorcomponent_emission, bgCol);
            glBegin(GL_QUADS);
            glVertex3i(0, textPos[1] + (HIERARCHY_TEXT_CENTER_OFFSET - HIERARCHY_HALF_INTER_LINE_SPACE) * GuiApp::sc,
                       0);
            glVertex3i(renderingSize[0],
                       textPos[1] + (HIERARCHY_TEXT_CENTER_OFFSET - HIERARCHY_HALF_INTER_LINE_SPACE) * GuiApp::sc, 0);
            glVertex3i(renderingSize[0],
                       textPos[1] + (HIERARCHY_TEXT_CENTER_OFFSET + HIERARCHY_HALF_INTER_LINE_SPACE) * GuiApp::sc, 0);
            glVertex3i(0, textPos[1] + (HIERARCHY_TEXT_CENTER_OFFSET + HIERARCHY_HALF_INTER_LINE_SPACE) * GuiApp::sc,
                       0);
            glEnd();
        }

        if (textInside && (!dontDisplay))
        {
            hier->objectPosition.push_back(textPos[1] + HIERARCHY_TEXT_CENTER_OFFSET * GuiApp::sc);
            hier->objectPosition.push_back(objectID);
        }
    }
    else
    { // for drag and drop only:
        inSelection = App::currentWorld->sceneObjects->isObjectInSelection(objectID);
    }

    int off = 2;
    if (it != nullptr)
    {
        if (children.size() != 0) //-//
        {
            if (it->getObjectProperty() & sim_objectproperty_collapsed)
            {
                int picture = PLUS_SIGN_TREE_PICTURE;
                if (dontDisplay || (!textInside))
                    picture = NO_TREE_PICTURE;
                _drawIcon_sceneObject(hier, textPos[0] + off - 5 * GuiApp::sc,
                                      textPos[1] + HIERARCHY_TEXT_CENTER_OFFSET * GuiApp::sc, it, picture, true,
                                      transparencyFactor, forDragAndDrop);
            }
            else
            {
                int picture = MINUS_SIGN_TREE_PICTURE;
                if (dontDisplay || (!textInside))
                    picture = NO_TREE_PICTURE;
                _drawIcon_sceneObject(hier, textPos[0] + off - 5 * GuiApp::sc,
                                      textPos[1] + HIERARCHY_TEXT_CENTER_OFFSET * GuiApp::sc, it, picture, true,
                                      transparencyFactor, forDragAndDrop);
            }
        }

        off = off + _drawIcon_sceneObject(hier, textPos[0] + off + 8 * GuiApp::sc,
                                          textPos[1] + HIERARCHY_TEXT_CENTER_OFFSET * GuiApp::sc, it, -1, !dontDisplay,
                                          transparencyFactor, forDragAndDrop);
    }

    if (objectID < 0)
    { // For the world!
        off = off + _drawIcon_sceneObject(hier, textPos[0] + off - 15 * GuiApp::sc,
                                          textPos[1] + HIERARCHY_TEXT_CENTER_OFFSET * GuiApp::sc, it, objectID,
                                          !dontDisplay, transparencyFactor, forDragAndDrop);
        off -= 24 / GuiApp::sc; // tweaking
    }

    int textEndPosition = textPos[0] + off + ogl::getTextLengthInPixels(theText.c_str());
    if ((labelEditObjectID == objectID) && (objectID >= 0))
        textEndPosition = textPos[0] + off + ogl::getTextLengthInPixels(hier->editionText.c_str()) + 16 * GuiApp::sc;

    if (!forDragAndDrop)
    {
        if (objectID >= 0)
        { // world text can never be edited anyways
            hier->textPosition.push_back(textPos[0]);
            hier->textPosition.push_back(textPos[1]);
            hier->textPosition.push_back(textPos[0] + off);
            hier->textPosition.push_back(textEndPosition);
            hier->textPosition.push_back(textPos[1] + HIERARCHY_TEXT_CENTER_OFFSET * GuiApp::sc);
            hier->textPosition.push_back(objectID);
        }
    }

    if (textInside && (!dontDisplay))
    {
        float* tc;
        if (inSelection)
            tc = ogl::HIERARCHY_AND_BROWSER_TEXT_COLOR_VISIBLE;
        else
        {
            if ((it != nullptr) &&
                (((it->getVisibilityLayer() & App::currentWorld->environment->getActiveLayers()) == 0) ||
                 it->isObjectPartOfInvisibleModel()))
                tc = ogl::HIERARCHY_AND_BROWSER_TEXT_COLOR_INVISIBLE;
            else
                tc = ogl::HIERARCHY_AND_BROWSER_TEXT_COLOR_VISIBLE;
        }
        if (transparentForTreeObjects == -2)
            ogl::setTextColor((tc[0] + 7.0 * bgCol[0]) * 0.125, (tc[1] + 7.0 * bgCol[1]) * 0.125,
                              (tc[2] + 7.0 * bgCol[2]) * 0.125);
        else
        {
            if ((it == nullptr) && (!isLocalWorld()))
                ogl::setTextColor((tc[0] + 3.0 * bgCol[0]) * 0.25, (tc[1] + 3.0 * bgCol[1]) * 0.25,
                                  (tc[2] + 3.0 * bgCol[2]) * 0.25);
            else
                ogl::setTextColor(tc);
        }

        if ((labelEditObjectID != objectID) || (objectID == -1))
            ogl::drawText(textPos[0] + off, textPos[1], 0, theText, false);
        else
            hier->drawEditionLabel(textPos[0] + off, textPos[1]);
        // Following is for vertical lines that originates from one of this object's parent
        ogl::setMaterialColor(sim_colorcomponent_emission, ogl::HIERARCHY_AND_BROWSER_LINE_COLOR);
        if (!forDragAndDrop)
        {
            for (int i = 0; i < int(vertLines->size()); i++)
            {
                if (vertLines->at(i) < indentNb - 1)
                    ogl::drawSingle2dLine_i(
                        textPos[0] + 3 + horizontalShift / 2 - horizontalShift * (indentNb - vertLines->at(i)),
                        textPos[1] + (HIERARCHY_TEXT_CENTER_OFFSET + HIERARCHY_HALF_INTER_LINE_SPACE) * GuiApp::sc,
                        textPos[0] + 3 + horizontalShift / 2 - horizontalShift * (indentNb - vertLines->at(i)),
                        textPos[1] + (HIERARCHY_TEXT_CENTER_OFFSET - HIERARCHY_HALF_INTER_LINE_SPACE) * GuiApp::sc);
            }
        }
    }

    if (isOtherWorld)
    {
        textPos[1] = textPos[1] - HIERARCHY_INTER_LINE_SPACE * GuiApp::sc;
        return;
    }

    int lineLastPos = textEndPosition;

    // Show the script and script parameter icons:
    int tPosX = lineLastPos;
    int tPosY = textPos[1] + HIERARCHY_TEXT_CENTER_OFFSET * GuiApp::sc;
    int localOffset = 16;
    if (it != nullptr)
    {
        bool hasScript = false;
        // Old simulation scripts:
        CScriptObject* script = App::currentWorld->sceneObjects->embeddedScriptContainer->getScriptFromObjectAttachedTo(sim_scripttype_simulation, it->getObjectHandle());
        if (script != nullptr)
        {
            hasScript = true;
            if (!dontDisplay)
            {
                if (script->getScriptDisabledAndNoErrorRaised() || ((it->getCumulativeModelProperty() & sim_modelproperty_scripts_inactive) != 0))
                    App::worldContainer->globalGuiTextureCont->startTextureDisplay(SCRIPTDISABLED_PICTURE);
                else
                {
                    if (script->getScriptHasError())
                        App::worldContainer->globalGuiTextureCont->startTextureDisplay(SCRIPTERROR_PICTURE);
                    else
                        App::worldContainer->globalGuiTextureCont->startTextureDisplay(SCRIPT_PICTURE);
                }
                _drawTexturedIcon(tPosX + localOffset, tPosY, HIERARCHY_ICON_WIDTH * GuiApp::sc,
                                  HIERARCHY_ICON_HEIGHT * GuiApp::sc, transparencyFactor);
            }
            if (!forDragAndDrop)
            {
                hier->scriptIconPosition.push_back(tPosX + localOffset);
                hier->scriptIconPosition.push_back(tPosY);
                hier->scriptIconPosition.push_back(script->getScriptHandle());
            }
            localOffset += HIERARCHY_ICON_WIDTH * GuiApp::sc;
        }

        // Old Customization scripts:
        CScriptObject* customizationScript = App::currentWorld->sceneObjects->embeddedScriptContainer->getScriptFromObjectAttachedTo(
            sim_scripttype_customization, it->getObjectHandle());
        if (customizationScript != nullptr)
        {
            hasScript = true;
            if (!dontDisplay)
            {
                if (customizationScript->getScriptDisabledAndNoErrorRaised() || ((it->getCumulativeModelProperty() & sim_modelproperty_scripts_inactive) != 0))
                    App::worldContainer->globalGuiTextureCont->startTextureDisplay(CUSTOMIZATIONSCRIPTDISABLED_PICTURE);
                else
                {
                    if (customizationScript->getScriptHasError())
                        App::worldContainer->globalGuiTextureCont->startTextureDisplay(CUSTOMIZATIONSCRIPTERROR_PICTURE);
                    else
                        App::worldContainer->globalGuiTextureCont->startTextureDisplay(CUSTOMIZATIONSCRIPT_PICTURE);
                }
                _drawTexturedIcon(tPosX + localOffset, tPosY, HIERARCHY_ICON_WIDTH * GuiApp::sc,
                                  HIERARCHY_ICON_HEIGHT * GuiApp::sc, transparencyFactor);
            }
            if (!forDragAndDrop)
            {
                hier->scriptIconPosition.push_back(tPosX + localOffset);
                hier->scriptIconPosition.push_back(tPosY);
                hier->scriptIconPosition.push_back(customizationScript->getScriptHandle());
            }
            localOffset += HIERARCHY_ICON_WIDTH * GuiApp::sc;
        }

        //if (hasScript)
        { // User params:
            CScriptObject* newScript = nullptr;
            if (it->getObjectType() == sim_sceneobject_script)
            {
                CScript* so = (CScript*)it;
                if (so->scriptObject->getScriptType() == sim_scripttype_customization)
                {
                    if ((!so->scriptObject->getScriptIsDisabled()) && (!so->scriptObject->getScriptHasError()) && so->scriptObject->hasSystemFunctionOrHook(sim_syscb_userconfig))
                        newScript = so->scriptObject;
                }
            }
            else
            {
                for (size_t i = 0; i < it->getChildCount(); i++)
                {
                    CSceneObject* ch = it->getChildFromIndex(i);
                    if (ch->getObjectType() == sim_sceneobject_script)
                    {
                        CScript* so = (CScript*)ch;
                        if (so->scriptObject->getScriptType() == sim_scripttype_customization)
                        {
                            if ((!so->scriptObject->getScriptIsDisabled()) && (!so->scriptObject->getScriptHasError()) && so->scriptObject->hasSystemFunctionOrHook(sim_syscb_userconfig))
                            {
                                newScript = so->scriptObject;
                                break;
                            }
                        }
                    }
                }
            }
            if (newScript != nullptr)
            {
                if (!dontDisplay)
                {
                    App::worldContainer->globalGuiTextureCont->startTextureDisplay(USER_PARAMETERS_PICTURE);
                    _drawTexturedIcon(tPosX + localOffset, tPosY, HIERARCHY_ICON_WIDTH * GuiApp::sc,
                                      HIERARCHY_ICON_HEIGHT * GuiApp::sc, transparencyFactor);
                }
                if (!forDragAndDrop)
                {
                    hier->scriptParametersIconPosition.push_back(tPosX + localOffset);
                    hier->scriptParametersIconPosition.push_back(tPosY);
                    hier->scriptParametersIconPosition.push_back(newScript->getScriptHandle()); // which is same as object handle with new scripts
                }
                localOffset += (HIERARCHY_ICON_WIDTH + HIERARCHY_INTER_ICON_SPACING) * GuiApp::sc;
            }
            else
            {
                if (it->getObjectType() != sim_sceneobject_script)
                {
                    CUserParameters* params = it->getUserScriptParameterObject();
                    if (((params != nullptr) && (params->userParamEntries.size() > 0)) ||
                        ((customizationScript != nullptr) &&
                         customizationScript->hasSystemFunctionOrHook(sim_syscb_userconfig)))
                    {
                        //                printf("Object %s, param1: %s,
                        //                %i\n",it->getObjectAlias().c_str(),params->userParamEntries[0].name.c_str(),params->userParamEntries.size());
                        if (!dontDisplay)
                        {
                            App::worldContainer->globalGuiTextureCont->startTextureDisplay(USER_PARAMETERS_PICTURE);
                            _drawTexturedIcon(tPosX + localOffset, tPosY, HIERARCHY_ICON_WIDTH * GuiApp::sc,
                                              HIERARCHY_ICON_HEIGHT * GuiApp::sc, transparencyFactor);
                        }
                        if (!forDragAndDrop)
                        {
                            hier->scriptParametersIconPosition.push_back(tPosX + localOffset);
                            hier->scriptParametersIconPosition.push_back(tPosY);
                            hier->scriptParametersIconPosition.push_back(objectID);
                        }
                        localOffset += (HIERARCHY_ICON_WIDTH + HIERARCHY_INTER_ICON_SPACING) * GuiApp::sc;
                    }
                }
            }
        }
    }
    else
    { // This is for the main script (pseudo object "world"):
        CScriptObject* script = App::currentWorld->sceneObjects->embeddedScriptContainer->getMainScript();
        if (script != nullptr)
        {
            if (!dontDisplay)
            {
                App::worldContainer->globalGuiTextureCont->startTextureDisplay(MAIN_SCRIPT_PICTURE);
                _drawTexturedIcon(tPosX + localOffset, tPosY, HIERARCHY_ICON_WIDTH * GuiApp::sc,
                                  HIERARCHY_ICON_HEIGHT * GuiApp::sc, transparencyFactor);
            }
            if (!forDragAndDrop)
            {
                hier->scriptIconPosition.push_back(tPosX + localOffset);
                hier->scriptIconPosition.push_back(tPosY);
                hier->scriptIconPosition.push_back(script->getScriptHandle());
            }
            localOffset += HIERARCHY_ICON_WIDTH * GuiApp::sc;
            /*
            if (!dontDisplay)
            {
                if (script->getScriptParametersObject()->userParamEntries.size()!=0)
                    App::wc->globalGuiTextureCont->startTextureDisplay(USER_PARAMETERS_PICTURE);
                else
                    App::wc->globalGuiTextureCont->startTextureDisplay(SCRIPT_PARAMETERS_PICTURE);
                _drawTexturedIcon(tPosX+localOffset,tPosY,HIERARCHY_ICON_WIDTH*GuiApp::sc,HIERARCHY_ICON_HEIGHT*GuiApp::sc,transparencyFactor);
            }
            if (!forDragAndDrop)
            {
                hier->scriptParametersIconPosition.push_back(tPosX+localOffset);
                hier->scriptParametersIconPosition.push_back(tPosY);
                hier->scriptParametersIconPosition.push_back(script->getScriptHandle());
            }
            localOffset+=(HIERARCHY_ICON_WIDTH+HIERARCHY_INTER_ICON_SPACING)*GuiApp::sc;
            */
        }
    }
    lineLastPos += localOffset;

    if (lineLastPos > maxRenderedPos[0])
        maxRenderedPos[0] = lineLastPos;

    // Show the dynamic icons:
    if ((it != nullptr) && (!forDragAndDrop))
    {
        int simIconCode = it->getDynamicSimulationIconCode();
        if (simIconCode != sim_dynamicsimicon_none)
        {
            int ic;
            if (simIconCode == sim_dynamicsimicon_objectisdynamicallysimulated)
                ic = DYNAMICS_PICTURE;
            if (simIconCode == sim_dynamicsimicon_objectisnotdynamicallyenabled)
                ic = WARNING_PICTURE;
            lineLastPos +=
                16 * GuiApp::sc + _drawIcon_sceneObject(hier, lineLastPos + 24 * GuiApp::sc,
                                                        textPos[1] + HIERARCHY_TEXT_CENTER_OFFSET * GuiApp::sc, it, ic,
                                                        !dontDisplay, transparencyFactor, forDragAndDrop);
            int mmm = std::max<int>(lineLastPos, textEndPosition);
            //          if (textEndPosition+lineLastPos>maxRenderedPos[0])
            //              maxRenderedPos[0]=textEndPosition+lineLastPos;
            if (mmm > maxRenderedPos[0])
                maxRenderedPos[0] = mmm;
        }
    }

    if (!forDragAndDrop)
    {
        hier->lineLastPosition.push_back(lineLastPos);
        hier->lineLastPosition.push_back(textPos[1] + HIERARCHY_TEXT_CENTER_OFFSET * GuiApp::sc);
        if (it != nullptr)
            hier->lineLastPosition.push_back(it->getObjectHandle());
        else
            hier->lineLastPosition.push_back(-1);
    }

    if ((!dontDisplay) && (!forDragAndDrop) && (it != nullptr))
    { // Following for the line element(s) just left of scene object icons:
        if (it->getModelBase())
        { //-//
            ogl::drawSingle2dLine_i(
                textPos[0] + 16 * GuiApp::sc, textPos[1] + HIERARCHY_TEXT_CENTER_OFFSET * GuiApp::sc,
                textPos[0] + 20 * GuiApp::sc, textPos[1] + HIERARCHY_TEXT_CENTER_OFFSET * GuiApp::sc);
            if ((children.size() != 0) && ((it->getObjectProperty() & sim_objectproperty_collapsed) == 0))
                ogl::drawSingle2dLine_i(textPos[0] + 10 * GuiApp::sc,
                                        textPos[1] + (HIERARCHY_TEXT_CENTER_OFFSET - CONST_VAL_6) * GuiApp::sc,
                                        textPos[0] + 10 * GuiApp::sc,
                                        textPos[1] +
                                            (HIERARCHY_TEXT_CENTER_OFFSET - HIERARCHY_INTER_LINE_SPACE) * GuiApp::sc);
        }
        else
        { //-//
            ogl::drawSingle2dLine_i(textPos[0] + 4 * GuiApp::sc, textPos[1] + HIERARCHY_TEXT_CENTER_OFFSET * GuiApp::sc,
                                    textPos[0] + 20 * GuiApp::sc,
                                    textPos[1] + HIERARCHY_TEXT_CENTER_OFFSET * GuiApp::sc);
            if ((children.size() != 0) && ((it->getObjectProperty() & sim_objectproperty_collapsed) == 0))
                ogl::drawSingle2dLine_i(
                    textPos[0] + 10 * GuiApp::sc, textPos[1] + HIERARCHY_TEXT_CENTER_OFFSET * GuiApp::sc,
                    textPos[0] + 10 * GuiApp::sc,
                    textPos[1] + (HIERARCHY_TEXT_CENTER_OFFSET - HIERARCHY_INTER_LINE_SPACE) * GuiApp::sc);
        }
    }

    textPos[1] = textPos[1] - HIERARCHY_INTER_LINE_SPACE * GuiApp::sc;

    if (((it != nullptr) && ((it->getObjectProperty() & sim_objectproperty_collapsed) == 0)) || (objectID < 0))
    {
        int xPosCopy = textPos[0];
        textPos[0] = textPos[0] + horizontalShift;
        int indentCopy = indentNb + 1;

        ogl::setMaterialColor(sim_colorcomponent_emission, ogl::HIERARCHY_AND_BROWSER_LINE_COLOR);

        for (size_t i = 0; i < children.size(); i++)
        {
            if (i != children.size() - 1)
            { // Vertical line going through (a T turned counter-clockwise)
                vertLines->push_back(indentNb);
                int txtYTmp = textPos[1];
                children[i]->renderElement_sceneObject(hier, labelEditObjectID, bright, dontDisplay, renderingSize,
                                                       textPos, indentCopy, vertLines, minRenderedPos, maxRenderedPos,
                                                       forDragAndDrop, transparentForTreeObjects, dropID);
                vertLines->erase(vertLines->end() - 1);
                if ((!dontDisplay) && (!forDragAndDrop))
                {
                    if (children[i]->children.size() == 0) //-//
                    {
                        ogl::drawSingle2dLine_i(
                            xPosCopy + 10 * GuiApp::sc,
                            txtYTmp + (HIERARCHY_TEXT_CENTER_OFFSET + HIERARCHY_HALF_INTER_LINE_SPACE) * GuiApp::sc,
                            xPosCopy + 10 * GuiApp::sc,
                            txtYTmp + (HIERARCHY_TEXT_CENTER_OFFSET - HIERARCHY_HALF_INTER_LINE_SPACE) * GuiApp::sc);
                        ogl::drawSingle2dLine_i(
                            xPosCopy + 10 * GuiApp::sc, txtYTmp + HIERARCHY_TEXT_CENTER_OFFSET * GuiApp::sc,
                            xPosCopy + 17 * GuiApp::sc, txtYTmp + HIERARCHY_TEXT_CENTER_OFFSET * GuiApp::sc);
                    }
                }
            }
            else
            { // Vertical line stopping in the middle (L)
                int txtYTmp = textPos[1];
                children[i]->renderElement_sceneObject(hier, labelEditObjectID, bright, dontDisplay, renderingSize,
                                                       textPos, indentCopy, vertLines, minRenderedPos, maxRenderedPos,
                                                       forDragAndDrop, transparentForTreeObjects, dropID);
                if ((!dontDisplay) && (!forDragAndDrop))
                {
                    if (children[i]->children.size() == 0) //-//
                    {
                        ogl::drawSingle2dLine_i(
                            xPosCopy + 10 * GuiApp::sc,
                            txtYTmp + (HIERARCHY_TEXT_CENTER_OFFSET + HIERARCHY_HALF_INTER_LINE_SPACE) * GuiApp::sc,
                            xPosCopy + 10 * GuiApp::sc, txtYTmp + HIERARCHY_TEXT_CENTER_OFFSET * GuiApp::sc);
                        ogl::drawSingle2dLine_i(
                            xPosCopy + 10 * GuiApp::sc, txtYTmp + HIERARCHY_TEXT_CENTER_OFFSET * GuiApp::sc,
                            xPosCopy + 17 * GuiApp::sc, txtYTmp + HIERARCHY_TEXT_CENTER_OFFSET * GuiApp::sc);
                    }
                }
            }
        }
        textPos[0] = xPosCopy;
    }
}

int CHierarchyElement::_drawIcon_sceneObject(CHierarchy* hier, int tPosX, int tPosY, CSceneObject* it, int pictureID,
                                             bool drawIt, double transparencyFactor, bool forDragAndDrop)
{ // pictureID is -1 by default. It is then ignored. The size of the icon is 16x16
    int retVal = 0;
    if (pictureID != NO_TREE_PICTURE)
    {
        if (pictureID >= 0)
        {
            if (drawIt)
            {
                App::worldContainer->globalGuiTextureCont->startTextureDisplay(pictureID);
                if ((it != nullptr) && (!forDragAndDrop))
                {
                    if (((pictureID == PLUS_SIGN_TREE_PICTURE) || (pictureID == MINUS_SIGN_TREE_PICTURE)))
                    {
                        hier->inflateIconPosition.push_back(tPosX);
                        hier->inflateIconPosition.push_back(tPosY);
                        hier->inflateIconPosition.push_back(it->getObjectHandle());
                    }
                    else
                    {
                        if (it->getDynamicSimulationIconCode() != sim_dynamicsimicon_none)
                        {
                            hier->simulationIconPosition.push_back(tPosX);
                            hier->simulationIconPosition.push_back(tPosY);
                            hier->simulationIconPosition.push_back(it->getObjectHandle());
                        }
                    }
                }
                _drawTexturedIcon(tPosX, tPosY, HIERARCHY_ICON_WIDTH * GuiApp::sc, HIERARCHY_ICON_HEIGHT * GuiApp::sc,
                                  transparencyFactor);
            }
            retVal += (HIERARCHY_ICON_WIDTH + HIERARCHY_INTER_ICON_SPACING) * GuiApp::sc;
        }
        else
        {
            int type = -1; // World
            if (it != nullptr)
                type = it->getObjectType();
            int objectOrWorldIconID = -1;
            if (drawIt)
            {
                if (type == -1)
                {
                    objectOrWorldIconID = WORLD_TREE_PICTURE;
                    if (!isLocalWorld())
                        transparencyFactor = 0.6;
                }
                if (type == sim_sceneobject_shape)
                {
                    if (((CShape*)it)->isCompound())
                    {
                        if (((CShape*)it)->getMesh()->isPure())
                            objectOrWorldIconID = PURE_MULTISHAPE_TREE_PICTURE;
                        else if (((CShape*)it)->getMesh()->isConvex())
                            objectOrWorldIconID = CONVEX_MULTISHAPE_TREE_PICTURE;
                        else
                            objectOrWorldIconID = MULTI_SHAPE_TREE_PICTURE;
                    }
                    else
                    {
                        if (((CShape*)it)->getMesh()->isPure())
                        {
                            if (((CShape*)it)->getMesh()->getPurePrimitiveType() == sim_primitiveshape_heightfield)
                                objectOrWorldIconID = HEIGHTFIELD_SHAPE_TREE_PICTURE;
                            else
                                objectOrWorldIconID = PURE_SHAPE_TREE_PICTURE;
                        }
                        else if (((CShape*)it)->getMesh()->isConvex())
                            objectOrWorldIconID = CONVEX_SHAPE_TREE_PICTURE;
                        else
                            objectOrWorldIconID = SIMPLE_SHAPE_TREE_PICTURE;
                    }
                }
                if (type == sim_sceneobject_joint)
                {
                    if (((CJoint*)it)->getJointType() == sim_joint_revolute)
                        objectOrWorldIconID = REVOLUTE_JOINT_TREE_PICTURE;
                    if (((CJoint*)it)->getJointType() == sim_joint_prismatic)
                        objectOrWorldIconID = PRISMATIC_JOINT_TREE_PICTURE;
                    if (((CJoint*)it)->getJointType() == sim_joint_spherical)
                        objectOrWorldIconID = SPHERICAL_JOINT_TREE_PICTURE;
                }
                if (type == sim_sceneobject_camera)
                    objectOrWorldIconID = CAMERA_TREE_PICTURE;
                if (type == sim_sceneobject_mirror)
                {
                    if (((CMirror*)it)->getIsMirror())
                        objectOrWorldIconID = MIRROR_TREE_PICTURE;
                    else
                        objectOrWorldIconID = CLIPPLANE_TREE_PICTURE;
                }
                if (type == sim_sceneobject_light)
                {
                    int subType = ((CLight*)it)->getLightType();
                    if (subType == sim_light_omnidirectional)
                    {
                        if (((CLight*)it)->getLightActive())
                            objectOrWorldIconID = OMNI_LIGHT_ON_TREE_PICTURE;
                        else
                            objectOrWorldIconID = OMNI_LIGHT_OFF_TREE_PICTURE;
                    }
                    if (subType == sim_light_spot)
                    {
                        if (((CLight*)it)->getLightActive())
                            objectOrWorldIconID = SPOT_LIGHT_ON_TREE_PICTURE;
                        else
                            objectOrWorldIconID = SPOT_LIGHT_OFF_TREE_PICTURE;
                    }
                    if (subType == sim_light_directional)
                    {
                        if (((CLight*)it)->getLightActive())
                            objectOrWorldIconID = DIR_LIGHT_ON_TREE_PICTURE;
                        else
                            objectOrWorldIconID = DIR_LIGHT_OFF_TREE_PICTURE;
                    }
                }
                if (type == sim_sceneobject_graph)
                    objectOrWorldIconID = GRAPH_TREE_PICTURE;
                if (type == sim_sceneobject_dummy)
                    objectOrWorldIconID = DUMMY_TREE_PICTURE;
                if (type == sim_sceneobject_script)
                {
                    CScriptObject* script = ((CScript*)it)->scriptObject;
                    if (script->getScriptType() == sim_scripttype_simulation)
                    {
                        if (script->getScriptDisabledAndNoErrorRaised() || ((it->getCumulativeModelProperty() & sim_modelproperty_scripts_inactive) != 0))
                            objectOrWorldIconID = SCRIPTDISABLED_PICTURE;
                        else
                        {
                            if (script->getScriptHasError())
                                objectOrWorldIconID = SCRIPTERROR_PICTURE;
                            else
                                objectOrWorldIconID = SCRIPT_PICTURE;
                        }
                    }
                    else
                    {
                        if (script->getScriptDisabledAndNoErrorRaised() || ((it->getCumulativeModelProperty() & sim_modelproperty_scripts_inactive) != 0))
                            objectOrWorldIconID = CUSTOMIZATIONSCRIPTDISABLED_PICTURE;
                        else
                        {
                            if (script->getScriptHasError())
                                objectOrWorldIconID = CUSTOMIZATIONSCRIPTERROR_PICTURE;
                            else
                                objectOrWorldIconID = CUSTOMIZATIONSCRIPT_PICTURE;
                        }
                    }
                }
                if (type == sim_sceneobject_pointcloud)
                    objectOrWorldIconID = POINTCLOUD_TREE_PICTURE;
                if (type == sim_sceneobject_octree)
                    objectOrWorldIconID = OCTREE_TREE_PICTURE;
                if (type == sim_sceneobject_proximitysensor)
                    objectOrWorldIconID = PROXIMITYSENSOR_TREE_PICTURE;
                if (type == sim_sceneobject_visionsensor)
                    objectOrWorldIconID = VISIONSENSOR_TREE_PICTURE;
                if (type == sim_sceneobject_path)
                    objectOrWorldIconID = PATH_TREE_PICTURE;
                if (type == sim_sceneobject_mill)
                    objectOrWorldIconID = MILL_TREE_PICTURE;
                if (type == sim_sceneobject_forcesensor)
                    objectOrWorldIconID = FORCE_SENSOR_TREE_PICTURE;
            }
            if (drawIt)
            {
                if ((it != nullptr) && it->getModelBase())
                { // We have to draw a model icon before the object icon:
                    App::worldContainer->globalGuiTextureCont->startTextureDisplay(MODEL_TREE_PICTURE);
                    _drawTexturedIcon(tPosX + retVal, tPosY, HIERARCHY_ICON_WIDTH * GuiApp::sc,
                                      HIERARCHY_ICON_HEIGHT * GuiApp::sc, transparencyFactor);
                    if (!forDragAndDrop)
                    {
                        hier->modelIconPosition.push_back(tPosX + retVal);
                        hier->modelIconPosition.push_back(tPosY);
                        hier->modelIconPosition.push_back(it->getObjectHandle());
                    }
                }
                retVal += (HIERARCHY_ICON_WIDTH + HIERARCHY_INTER_ICON_SPACING) * GuiApp::sc;

                App::worldContainer->globalGuiTextureCont->startTextureDisplay(objectOrWorldIconID);
                _drawTexturedIcon(tPosX + retVal, tPosY, HIERARCHY_ICON_WIDTH * GuiApp::sc,
                                  HIERARCHY_ICON_HEIGHT * GuiApp::sc, transparencyFactor);
                if (!forDragAndDrop)
                {
                    if ((it != nullptr) || ((pictureID < 0)))
                    {
                        hier->objectIconPosition.push_back(tPosX + retVal);
                        hier->objectIconPosition.push_back(tPosY);
                        if (it == nullptr)
                            hier->objectIconPosition.push_back(objectID);
                        else
                            hier->objectIconPosition.push_back(it->getObjectHandle());
                    }
                }
                retVal += (HIERARCHY_ICON_WIDTH + HIERARCHY_INTER_ICON_SPACING) * GuiApp::sc;
            }
            else
            {
                if ((it != nullptr) && it->getModelBase())
                { // We have to consider a model icon before the object icon:
                    if (!forDragAndDrop)
                    {
                        hier->modelIconPosition.push_back(tPosX + retVal);
                        hier->modelIconPosition.push_back(tPosY);
                        hier->modelIconPosition.push_back(it->getObjectHandle());
                    }
                    retVal += (HIERARCHY_ICON_WIDTH + HIERARCHY_INTER_ICON_SPACING) * GuiApp::sc;
                }
                if (!forDragAndDrop)
                {
                    hier->objectIconPosition.push_back(tPosX + retVal);
                    hier->objectIconPosition.push_back(tPosY);
                    if (it == nullptr)
                        hier->objectIconPosition.push_back(-1); // World icon
                    else
                        hier->objectIconPosition.push_back(it->getObjectHandle());
                }
                retVal += (HIERARCHY_ICON_WIDTH + HIERARCHY_INTER_ICON_SPACING) * GuiApp::sc;
            }
        }
    }
    else
        retVal += (HIERARCHY_ICON_WIDTH + HIERARCHY_INTER_ICON_SPACING) * GuiApp::sc;
    return (retVal);
}

void CHierarchyElement::renderElement_editModeList(CHierarchy* hier, int labelEditObjectID, bool& bright,
                                                   bool dontDisplay, int renderingSize[2], int textPos[2], int indentNb,
                                                   int minRenderedPos[2], int maxRenderedPos[2], int selectedState,
                                                   int editModeType)
{
    bright = !bright;
    if (textPos[0] < minRenderedPos[0])
        minRenderedPos[0] = textPos[0];
    if (textPos[1] < minRenderedPos[1])
        minRenderedPos[1] = textPos[1];
    if (textPos[1] > maxRenderedPos[1])
        maxRenderedPos[1] = textPos[1];

    if (textPos[1] < -HIERARCHY_INTER_LINE_SPACE * GuiApp::sc)
        dontDisplay = true; // We don't display what is outside of the view!
    bool textInside = (textPos[1] < renderingSize[1] + HIERARCHY_INTER_LINE_SPACE * GuiApp::sc);
    std::string theText;
    if (editModeType == VERTEX_EDIT_MODE)
    {
        theText = "Vertex ";
        theText += boost::lexical_cast<std::string>(objectID + 1);
    }
    if (editModeType == TRIANGLE_EDIT_MODE)
    {
        theText = "Triangle ";
        theText += boost::lexical_cast<std::string>(objectID + 1);
    }
    if (editModeType == EDGE_EDIT_MODE)
    {
        theText = "Edge ";
        theText += boost::lexical_cast<std::string>(objectID + 1);
    }
    if (editModeType == PATH_EDIT_MODE_OLD)
    {
        theText = "Path point ";
        theText += boost::lexical_cast<std::string>(objectID + 1);
    }
    if (editModeType == MULTISHAPE_EDIT_MODE)
    {
        theText = "Shape component ";
        theText += boost::lexical_cast<std::string>(objectID + 1);
        if (objectID ==
            GuiApp::mainWindow->editModeContainer->getMultishapeEditMode()->getMultishapeGeometricComponentIndex())
            selectedState = 2;
    }
    if ((selectedState != 0) && textInside && (!dontDisplay))
    {
        if (selectedState == 2)
            ogl::setMaterialColor(sim_colorcomponent_emission, ogl::HIERARCHY_AND_BROWSER_LAST_SELECTION_COLOR);
        else
        {
            if (bright)
                ogl::setMaterialColor(sim_colorcomponent_emission, ogl::HIERARCHY_NOT_LAST_SELECTION_COLOR_BRIGHT);
            else
                ogl::setMaterialColor(sim_colorcomponent_emission, ogl::HIERARCHY_NOT_LAST_SELECTION_COLOR_DARK);
        }
        glBegin(GL_QUADS);
        glVertex3i(0, textPos[1] + (HIERARCHY_TEXT_CENTER_OFFSET - HIERARCHY_HALF_INTER_LINE_SPACE) * GuiApp::sc, 0);
        glVertex3i(renderingSize[0],
                   textPos[1] + (HIERARCHY_TEXT_CENTER_OFFSET - HIERARCHY_HALF_INTER_LINE_SPACE) * GuiApp::sc, 0);
        glVertex3i(renderingSize[0],
                   textPos[1] + (HIERARCHY_TEXT_CENTER_OFFSET + HIERARCHY_HALF_INTER_LINE_SPACE) * GuiApp::sc, 0);
        glVertex3i(0, textPos[1] + (HIERARCHY_TEXT_CENTER_OFFSET + HIERARCHY_HALF_INTER_LINE_SPACE) * GuiApp::sc, 0);
        glEnd();
    }
    else if (textInside && (!dontDisplay))
    {
        if (App::userSettings->darkMode)
            ogl::setMaterialColor(sim_colorcomponent_emission, ogl::HIERARCHY_AND_BROWSER_NO_SELECTION_COLOR_DARK);
        else
            ogl::setMaterialColor(sim_colorcomponent_emission, ogl::HIERARCHY_AND_BROWSER_NO_SELECTION_COLOR_BRIGHT);
        glBegin(GL_QUADS);
        glVertex3i(0, textPos[1] + (HIERARCHY_TEXT_CENTER_OFFSET - HIERARCHY_HALF_INTER_LINE_SPACE) * GuiApp::sc, 0);
        glVertex3i(renderingSize[0],
                   textPos[1] + (HIERARCHY_TEXT_CENTER_OFFSET - HIERARCHY_HALF_INTER_LINE_SPACE) * GuiApp::sc, 0);
        glVertex3i(renderingSize[0],
                   textPos[1] + (HIERARCHY_TEXT_CENTER_OFFSET + HIERARCHY_HALF_INTER_LINE_SPACE) * GuiApp::sc, 0);
        glVertex3i(0, textPos[1] + (HIERARCHY_TEXT_CENTER_OFFSET + HIERARCHY_HALF_INTER_LINE_SPACE) * GuiApp::sc, 0);
        glEnd();
    }

    if (textInside && (!dontDisplay))
    {
        hier->objectPosition.push_back(textPos[1] + HIERARCHY_TEXT_CENTER_OFFSET * GuiApp::sc);
        hier->objectPosition.push_back(objectID);
    }
    int off = 2;

    if (editModeType == VERTEX_EDIT_MODE)
        off = off + _drawIcon_editModeList(hier, textPos[0] + off + 8 * GuiApp::sc,
                                           textPos[1] + HIERARCHY_TEXT_CENTER_OFFSET * GuiApp::sc, VERTEX_PICTURE,
                                           !dontDisplay);
    if (editModeType == TRIANGLE_EDIT_MODE)
        off = off + _drawIcon_editModeList(hier, textPos[0] + off + 8 * GuiApp::sc,
                                           textPos[1] + HIERARCHY_TEXT_CENTER_OFFSET * GuiApp::sc, TRIANGLE_PICTURE,
                                           !dontDisplay);
    if (editModeType == EDGE_EDIT_MODE)
        off = off + _drawIcon_editModeList(hier, textPos[0] + off + 8 * GuiApp::sc,
                                           textPos[1] + HIERARCHY_TEXT_CENTER_OFFSET * GuiApp::sc, EDGE_PICTURE,
                                           !dontDisplay);
    if (editModeType == PATH_EDIT_MODE_OLD)
        off = off + _drawIcon_editModeList(hier, textPos[0] + off + 8 * GuiApp::sc,
                                           textPos[1] + HIERARCHY_TEXT_CENTER_OFFSET * GuiApp::sc, PATH_POINT_PICTURE,
                                           !dontDisplay);
    if (editModeType == MULTISHAPE_EDIT_MODE)
    {
        if ((objectID >= 0) &&
            (objectID <
             GuiApp::mainWindow->editModeContainer->getMultishapeEditMode()->getMultishapeGeometricComponentsSize()))
        {
            CMesh* geom =
                GuiApp::mainWindow->editModeContainer->getMultishapeEditMode()->getMultishapeGeometricComponentAtIndex(
                    objectID);
            int pic = 0;
            if (geom->isPure())
                pic = PURE_SHAPE_TREE_PICTURE;
            else
            {
                if (geom->isConvex())
                    pic = CONVEX_SHAPE_TREE_PICTURE;
                else
                    pic = SIMPLE_SHAPE_TREE_PICTURE;
            }
            off =
                off + _drawIcon_editModeList(hier, textPos[0] + off + 8 * GuiApp::sc,
                                             textPos[1] + HIERARCHY_TEXT_CENTER_OFFSET * GuiApp::sc, pic, !dontDisplay);
        }
    }

    int textEndPosition = textPos[0] + off + ogl::getTextLengthInPixels(theText.c_str());
    hier->textPosition.push_back(textPos[0]);
    hier->textPosition.push_back(textPos[1]);
    hier->textPosition.push_back(textPos[0] + off);
    hier->textPosition.push_back(textEndPosition);
    hier->textPosition.push_back(textPos[1] + HIERARCHY_TEXT_CENTER_OFFSET * GuiApp::sc);
    hier->textPosition.push_back(objectID);

    if (textEndPosition > maxRenderedPos[0])
        maxRenderedPos[0] = textEndPosition;

    if (textInside && (!dontDisplay))
    {
        ogl::setTextColor(ogl::colorBlack);
        if (labelEditObjectID != objectID)
            ogl::drawText(textPos[0] + off, textPos[1], 0, theText);
        else
            hier->drawEditionLabel(textPos[0] + off, textPos[1]);
        ogl::setMaterialColor(sim_colorcomponent_emission, ogl::colorBlack);
    }
    textPos[1] = textPos[1] - HIERARCHY_INTER_LINE_SPACE * GuiApp::sc;
}

int CHierarchyElement::_drawIcon_editModeList(CHierarchy* hier, int tPosX, int tPosY, int pictureID, bool drawIt)
{ // The size of the icon is 16x16
    const int sizeX = 16 * GuiApp::sc;
    const int sizeY = 16 * GuiApp::sc;
    const int interIconSpacing = 4 * GuiApp::sc;
    int retVal = 0;
    if (pictureID != NO_TREE_PICTURE)
    {
        if (pictureID != -1)
        {
            if (drawIt)
            {
                App::worldContainer->globalGuiTextureCont->startTextureDisplay(pictureID);
                _drawTexturedIcon(tPosX, tPosY, sizeX, sizeY, 0.0);
            }
            hier->objectIconPosition.push_back(tPosX);
            hier->objectIconPosition.push_back(tPosY);
            hier->objectIconPosition.push_back(objectID);
        }
    }
    retVal += sizeX + interIconSpacing;
    return (retVal);
}

void CHierarchyElement::_drawTexturedIcon(int tPosX, int tPosY, int sizeX, int sizeY, double transparencyFactor)
{
    ogl::setBlending(true, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // We turn blending on!

    if (transparencyFactor > 0.01)
        glColor4f(1.0, 1.0, 1.0, float(1.0 - transparencyFactor));
    else
        glColor4f(1.0, 1.0, 1.0, 1.0); // opaque

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glEnable(GL_TEXTURE_2D);
    ogl::disableLighting_useWithCare(); // only temporarily
    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 0.0);
    glVertex3i(tPosX - sizeX / 2, tPosY - sizeY / 2, 0);
    glTexCoord2f(0.625, 0.0); // icons themselves are 20x16, but since some gfx cards don't support that non-power of 2
                              // res, we save them as 32x16!
    glVertex3i(tPosX + sizeX / 2, tPosY - sizeY / 2, 0);
    glTexCoord2f(0.625, 1.0); // icons themselves are 20x16, but since some gfx cards don't support that non-power of 2
                              // res, we save them as 32x16!
    glVertex3i(tPosX + sizeX / 2, tPosY + sizeY / 2, 0);
    glTexCoord2f(0.0, 1.0);
    glVertex3i(tPosX - sizeX / 2, tPosY + sizeY / 2, 0);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    ogl::enableLighting_useWithCare();
    ogl::setBlending(false);
    glDisable(GL_ALPHA_TEST);
    App::worldContainer->globalGuiTextureCont->endTextureDisplay();
}

bool CHierarchyElement::renderDummyElement(bool& bright, int renderingSize[2], int textPos[2])
{ // Just renders the background and returns true when inside the visible zone
    bright = !bright;
    if (textPos[1] < -HIERARCHY_INTER_LINE_SPACE * GuiApp::sc)
        return (false);
    if (textPos[1] < renderingSize[1] + HIERARCHY_INTER_LINE_SPACE * GuiApp::sc)
    {
        if (App::userSettings->darkMode)
            ogl::setMaterialColor(sim_colorcomponent_emission, ogl::HIERARCHY_AND_BROWSER_NO_SELECTION_COLOR_DARK);
        else
            ogl::setMaterialColor(sim_colorcomponent_emission, ogl::HIERARCHY_AND_BROWSER_NO_SELECTION_COLOR_BRIGHT);
        glBegin(GL_QUADS);
        glVertex3i(0, textPos[1] + (HIERARCHY_TEXT_CENTER_OFFSET - HIERARCHY_HALF_INTER_LINE_SPACE) * GuiApp::sc, 0);
        glVertex3i(renderingSize[0],
                   textPos[1] + (HIERARCHY_TEXT_CENTER_OFFSET - HIERARCHY_HALF_INTER_LINE_SPACE) * GuiApp::sc, 0);
        glVertex3i(renderingSize[0],
                   textPos[1] + (HIERARCHY_TEXT_CENTER_OFFSET + HIERARCHY_HALF_INTER_LINE_SPACE) * GuiApp::sc, 0);
        glVertex3i(0, textPos[1] + (HIERARCHY_TEXT_CENTER_OFFSET + HIERARCHY_HALF_INTER_LINE_SPACE) * GuiApp::sc, 0);
        glEnd();
        textPos[1] = textPos[1] - HIERARCHY_INTER_LINE_SPACE * GuiApp::sc;
        return (true);
    }
    return (false);
}

bool CHierarchyElement::isLocalWorld()
{
    if (objectID < 0)
        return (-App::worldContainer->getCurrentWorldIndex() - 1 == objectID);
    return (false);
}
