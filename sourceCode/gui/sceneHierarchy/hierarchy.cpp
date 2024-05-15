#include <simInternal.h>
#include <hierarchy.h>
#include <oGL.h>
#include <imgLoaderSaver.h>
#include <sceneObjectOperations.h>
#include <vMessageBox.h>
#include <addOperations.h>
#include <fileOperations.h>
#include <tt.h>
#include <simStrings.h>
#include <app.h>
#include <vVarious.h>
#include <vDateTime.h>
#include <simFlavor.h>
#ifdef SIM_WITH_GUI
#include <guiApp.h>
#endif

const int SAFETY_BORDER_SIZE = 20;
const int CONST_VAL_40 = 40;
const int CONST_VAL_8 = 8;
const int CONST_VAL_6 = 6;
const int CONST_VAL_4 = 4;
// const int SAFETY_BORDER_SIZE=40;

CHierarchy::CHierarchy()
{
    renderingSize[0] = 0;
    renderingSize[1] = 0;
    minRenderedPosition[0] = 0;
    minRenderedPosition[1] = 0;
    maxRenderedPosition[0] = 0;
    maxRenderedPosition[1] = 0;
    setUpDefaultValues();
}

CHierarchy::~CHierarchy()
{
    removeAll();
}

void CHierarchy::newSceneProcedure()
{
    setUpDefaultValues();
}

void CHierarchy::removeAll()
{
    for (int i = 0; i < int(rootElements.size()); i++)
        delete rootElements[i];
    rootElements.clear();
    refreshViewFlag = App::userSettings->hierarchyRefreshCnt;
    rebuildHierarchyFlag = true;
    resetViewFlag = true;
}

void CHierarchy::setUpDefaultValues()
{
    viewPosition[0] = 0;
    viewPosition[1] = 0;
    refreshViewFlag = App::userSettings->hierarchyRefreshCnt;
    rebuildHierarchyFlag = true;
    resetViewFlag = true;
    shiftingAllowed = false;
    shiftSelectionStarted = false;
    labelEditObjectID = -1;
    editionTextEditPos = -1;
    horizontalScrollbarHeight = 0;
    verticalScrollbarWidth = 0;
    sliderMoveMode = 0;
    _caughtElements = 0;
    _mouseDownDragObjectID = -1;
    _worldSelectID_down = -9999;
    _worldSelectID_moving = -9999;
}

int CHierarchy::getCaughtElements()
{
    return (_caughtElements);
}

void CHierarchy::clearCaughtElements(int keepMask)
{
    _caughtElements &= keepMask;
}

void CHierarchy::setRenderingSizeAndPosition(int xSize, int ySize, int xPos, int yPos)
{
    renderingSize[0] = xSize + SAFETY_BORDER_SIZE * GuiApp::sc;
    renderingSize[1] = ySize + SAFETY_BORDER_SIZE * GuiApp::sc;
    renderingPosition[0] = xPos - SAFETY_BORDER_SIZE * GuiApp::sc;
    renderingPosition[1] = yPos - SAFETY_BORDER_SIZE * GuiApp::sc;
    refreshViewFlag = App::userSettings->hierarchyRefreshCnt; // Important, even if the size and position didn't change!
}

void CHierarchy::rebuildHierarchy()
{
    for (size_t i = 0; i < rootElements.size(); i++)
        delete rootElements[i];
    rootElements.clear();

    if (GuiApp::getEditModeType() == NO_EDIT_MODE)
    {
        CHierarchyElement *newEl = new CHierarchyElement(-App::worldContainer->getCurrentWorldIndex() - 1);
        newEl->addYourChildren();
        std::string sceneName = App::currentWorld->mainSettings->getSceneName();
        newEl->setSceneName(sceneName.c_str());
        rootElements.push_back(newEl);
    }
    if (GuiApp::getEditModeType() & VERTEX_EDIT_MODE)
    {
        for (int i = 0; i < GuiApp::mainWindow->editModeContainer->getShapeEditMode()->getEditionVerticesSize() / 3;
             i++)
            rootElements.push_back(new CHierarchyElement(i));
    }
    if (GuiApp::getEditModeType() & TRIANGLE_EDIT_MODE)
    {
        for (int i = 0; i < GuiApp::mainWindow->editModeContainer->getShapeEditMode()->getEditionIndicesSize() / 3; i++)
            rootElements.push_back(new CHierarchyElement(i));
    }
    if (GuiApp::getEditModeType() & EDGE_EDIT_MODE)
    {
        for (int i = 0; i < GuiApp::mainWindow->editModeContainer->getShapeEditMode()->getEditionEdgesSize() / 2; i++)
            rootElements.push_back(new CHierarchyElement(i));
    }
    if (GuiApp::getEditModeType() & PATH_EDIT_MODE_OLD)
    {
        for (int i = 0;
             i < int(GuiApp::mainWindow->editModeContainer->getEditModePathContainer_old()->getSimplePathPointCount());
             i++)
            rootElements.push_back(new CHierarchyElement(i));
    }
    if (GuiApp::getEditModeType() & MULTISHAPE_EDIT_MODE)
    {
        for (int i = 0;
             i < GuiApp::mainWindow->editModeContainer->getMultishapeEditMode()->getMultishapeGeometricComponentsSize();
             i++)
            rootElements.push_back(new CHierarchyElement(i));
    }
    rebuildHierarchyFlag = false;
    refreshViewFlag = App::userSettings->hierarchyRefreshCnt;
}

void CHierarchy::looseFocus()
{
    refreshViewFlag = App::userSettings->hierarchyRefreshCnt;
    labelEditObjectID = -1;
}

void CHierarchy::keyPress(int key)
{
    if (App::currentWorld->sceneObjects == nullptr)
        return;
    refreshViewFlag = App::userSettings->hierarchyRefreshCnt;

    if (labelEditObjectID == -1)
    { // Normal mode (no label edition)
        if ((key == UP_KEY) || (key == DOWN_KEY) || (key == LEFT_KEY) || (key == RIGHT_KEY))
        {
            if (key == UP_KEY)
                viewPosition[1] = viewPosition[1] + HIERARCHY_INTER_LINE_SPACE * GuiApp::sc;
            if (key == DOWN_KEY)
                viewPosition[1] = viewPosition[1] - HIERARCHY_INTER_LINE_SPACE * GuiApp::sc;
            if (key == LEFT_KEY)
                viewPosition[0] = viewPosition[0] + HIERARCHY_INTER_LINE_SPACE * GuiApp::sc;
            if (key == RIGHT_KEY)
                viewPosition[0] = viewPosition[0] - HIERARCHY_INTER_LINE_SPACE * GuiApp::sc;
            return;
        }

        if ((key == CTRL_V_KEY) || (key == ESC_KEY) || (key == DELETE_KEY) || (key == BACKSPACE_KEY) ||
            (key == CTRL_X_KEY) || (key == CTRL_C_KEY) || (key == CTRL_A_KEY) || (key == CTRL_Y_KEY) ||
            (key == CTRL_Z_KEY))
        {
            if (!GuiApp::mainWindow->editModeContainer->keyPress(key))
                CSceneObjectOperations::keyPress(key); // the key press was not for the edit mode
            return;
        }

        if ((key == CTRL_S_KEY) || (key == CTRL_O_KEY) || (key == CTRL_W_KEY) || (key == CTRL_Q_KEY) ||
            (key == CTRL_N_KEY))
        {
            CFileOperations::keyPress(key);
            return;
        }

        if (key == CTRL_SPACE_KEY)
        {
            App::currentWorld->simulation->keyPress(key);
            return;
        }

        if (key == CTRL_E_KEY)
        {
            App::worldContainer->keyPress(key);
            return;
        }

        if ((key == CTRL_D_KEY) || (key == CTRL_G_KEY))
        {
            GuiApp::mainWindow->dlgCont->keyPress(key);
            return;
        }
    }
    else
    { // Label edition mode
        int em = GuiApp::getEditModeType();
        if (em == NO_EDIT_MODE)
        {
            CSceneObject *it = App::currentWorld->sceneObjects->getObjectFromHandle(labelEditObjectID);
            CButtonBlock *blk = App::currentWorld->buttonBlockContainer->getBlockWithID(labelEditObjectID);
            if ((key == ENTER_KEY) || (key == TAB_KEY))
            {
                if ((em == NO_EDIT_MODE) && (it != nullptr))
                {
                    if (editionText.find("#") != std::string::npos)
                    {
                        if (editionText[editionText.size() - 1] == '#')
                            editionText.pop_back();
                        if (App::currentWorld->sceneObjects->setObjectName_old(it, editionText.c_str(), true))
                            App::undoRedo_sceneChanged("");
                    }
                    else
                    {
                        if (App::currentWorld->sceneObjects->setObjectAlias(it, editionText.c_str(), true))
                            App::undoRedo_sceneChanged("");
                    }
                    GuiApp::setFullDialogRefreshFlag();
                }
                labelEditObjectID = -1;
            }
            if ((key == LEFT_KEY) || (key == RIGHT_KEY))
            {
                if (editionTextEditPos == -1)
                    editionTextEditPos = (int)editionText.length();
                if ((key == LEFT_KEY) && (editionTextEditPos > 0))
                    editionTextEditPos--;
                if ((key == RIGHT_KEY) && (editionTextEditPos < int(editionText.length())))
                    editionTextEditPos++;
            }
            if (key == BACKSPACE_KEY)
            {
                if (editionTextEditPos == -1)
                {
                    editionText = "";
                    editionTextEditPos = 0;
                }
                else
                {
                    if (editionText.length() != 0)
                    {
                        if (editionTextEditPos > 0)
                        {
                            editionText.erase(editionText.begin() + editionTextEditPos - 1);
                            editionTextEditPos--;
                        }
                    }
                }
            }
            if (key == DELETE_KEY)
            {
                if (editionTextEditPos == -1)
                {
                    editionText = "";
                    editionTextEditPos = 0;
                }
                else
                {
                    if (editionText.length() != 0)
                    {
                        if (editionTextEditPos < int(editionText.length()))
                            editionText.erase(editionText.begin() + editionTextEditPos);
                    }
                }
            }
            if ((key >= 32) && (key < 123))
            {
                if (editionTextEditPos == -1)
                {
                    editionTextEditPos = 0;
                    editionText = "";
                }
                editionText.insert(editionText.begin() + editionTextEditPos, (char)key);
                editionTextEditPos++;
            }
        }
    }
}

bool CHierarchy::render()
{ // return value true means the hierarchy was refreshed
    TRACE_INTERNAL;
    if (viewPosition[0] < -20000) // From -2000 to -20000 on 3/4/2011 // somehow there is a bug I can't put the finger
                                  // on right now (2009/12/16)
        viewPosition[0] = 0;
    if (rebuildHierarchyFlag)
        rebuildHierarchy();

    if (resetViewFlag)
    {
        viewPosition[0] = 0;
        viewPosition[1] = 0;
        resetViewFlag = false;
    }
    if (App::userSettings->hierarchyRefreshCnt > 0)
    {
        if (refreshViewFlag <= 0)
            return (false);
        refreshViewFlag--;
    }

    int editModeType = GuiApp::getEditModeType();
    bool hierarchDragUnderway = false;
    int dx = 0;
    int dy = 0;
    int dropID = -9999;
    if (editModeType == NO_EDIT_MODE)
    {
        if (_worldSelectID_down != -9999)
            refreshViewFlag = App::userSettings->hierarchyRefreshCnt;

        if (_mouseDownDragObjectID != -1)
        {
            if (App::currentWorld->sceneObjects->getLastSelectionHandle() != _mouseDownDragObjectID)
                _mouseDownDragObjectID = -1; // we probably pressed esc
            if (_mouseDownDragObjectID != -1)
            {
                refreshViewFlag = App::userSettings->hierarchyRefreshCnt;
                dx = mouseRelativePosition[0] - mouseDownRelativePosition[0];
                dy = mouseRelativePosition[1] - mouseDownRelativePosition[1];
                hierarchDragUnderway = ((abs(dx) > 8) || (abs(dy) > 8));
                if (hierarchDragUnderway)
                {
                    dropID = objectIDWhereTheMouseCurrentlyIs_minus9999ForNone;
                    if (dropID == -9999)
                        dropID = -App::worldContainer->getCurrentWorldIndex() - 1; // world
                    else
                    {
                        if (dropID < 0)
                        {
                            if (dropID != -App::worldContainer->getCurrentWorldIndex() - 1)
                                dropID = -9999;
                        }
                    }
                }
            }
        }

        static bool lastHierarchDragUnderway = false;
        static int lastDragUnderwayTime = 0;
        static int dragStartTime = 0;
        if (hierarchDragUnderway)
        {
            int ct = (int)VDateTime::getTimeInMs();
            if (!lastHierarchDragUnderway)
                dragStartTime = ct;
            if (ct - dragStartTime > 1000)
            {
                if (ct - lastDragUnderwayTime > 50)
                {
                    if (mouseRelativePosition[1] <
                        renderingPosition[1] +
                            (BROWSER_HIERARCHY_TITLE_BAR_HEIGHT + CONST_VAL_40 + HIERARCHY_INTER_LINE_SPACE * 4) *
                                GuiApp::sc) // Bottom border
                        viewPosition[1] = viewPosition[1] + HIERARCHY_INTER_LINE_SPACE * GuiApp::sc;
                    if (mouseRelativePosition[1] <
                        renderingPosition[1] +
                            (BROWSER_HIERARCHY_TITLE_BAR_HEIGHT + CONST_VAL_40 + HIERARCHY_INTER_LINE_SPACE * 2) *
                                GuiApp::sc) // Bottom border
                        viewPosition[1] = viewPosition[1] + HIERARCHY_INTER_LINE_SPACE * GuiApp::sc;
                    if (mouseRelativePosition[1] >
                        renderingPosition[1] + BROWSER_HIERARCHY_TITLE_BAR_HEIGHT * GuiApp::sc + renderingSize[1] -
                            HIERARCHY_INTER_LINE_SPACE * 4 * GuiApp::sc) // top border
                        viewPosition[1] = viewPosition[1] - HIERARCHY_INTER_LINE_SPACE * GuiApp::sc;
                    if (mouseRelativePosition[1] >
                        renderingPosition[1] + BROWSER_HIERARCHY_TITLE_BAR_HEIGHT * GuiApp::sc + renderingSize[1] -
                            HIERARCHY_INTER_LINE_SPACE * 2 * GuiApp::sc) // top border
                        viewPosition[1] = viewPosition[1] - HIERARCHY_INTER_LINE_SPACE * GuiApp::sc;

                    if (mouseRelativePosition[0] < HIERARCHY_INTER_LINE_SPACE * 2 * GuiApp::sc) // left border
                        viewPosition[0] = viewPosition[0] + HIERARCHY_INTER_LINE_SPACE * GuiApp::sc;
                    if (mouseRelativePosition[0] >
                        renderingSize[0] - HIERARCHY_INTER_LINE_SPACE * 2 * GuiApp::sc) // right border
                        viewPosition[0] = viewPosition[0] - HIERARCHY_INTER_LINE_SPACE * GuiApp::sc;
                    objectIDWhereTheMouseCurrentlyIs_minus9999ForNone =
                        getActionObjectID_icon(0, mouseRelativePosition[1], true);
                    lastDragUnderwayTime = ct;
                }
            }
        }
        lastHierarchDragUnderway = hierarchDragUnderway;
    }

    // We draw a black background so that the separation between the hierarchy and scene is rendered correctly on ALL
    // graphic cards:
    glEnable(GL_SCISSOR_TEST);
    glDisable(GL_DEPTH_TEST);
    glScissor(renderingPosition[0] + SAFETY_BORDER_SIZE * GuiApp::sc,
              renderingPosition[1] + BROWSER_HIERARCHY_TITLE_BAR_HEIGHT * GuiApp::sc,
              renderingSize[0] - SAFETY_BORDER_SIZE * GuiApp::sc, renderingSize[1]);
    glClearColor(ogl::SEPARATION_LINE_COLOR[0], ogl::SEPARATION_LINE_COLOR[1], ogl::SEPARATION_LINE_COLOR[2], 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    ogl::setMaterialColor(ogl::colorBlack, ogl::colorBlack, ogl::colorBlack);
    glEnable(GL_SCISSOR_TEST);
    glViewport(renderingPosition[0], renderingPosition[1], renderingSize[0], renderingSize[1]);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, renderingSize[0], 0.0, renderingSize[1], -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);
    glScissor(renderingPosition[0] + SAFETY_BORDER_SIZE * GuiApp::sc, renderingPosition[1],
              renderingSize[0] - SAFETY_BORDER_SIZE * GuiApp::sc, renderingSize[1]);
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    // There are 2 passes: one where we don't display anything but retrieve information
    // about min and max positions:
    validateViewPosition();
    int textPos[2] = {viewPosition[0], viewPosition[1] + renderingSize[1]};
    minRenderedPosition[0] = 100000;
    minRenderedPosition[1] = 100000;
    maxRenderedPosition[0] = -100000;
    maxRenderedPosition[1] = -100000;
    bool bright = false;

    inflateIconPosition.clear();
    scriptIconPosition.clear();
    scriptParametersIconPosition.clear();
    objectPosition.clear();
    objectIconPosition.clear();
    modelIconPosition.clear();
    textPosition.clear();
    lineLastPosition.clear();
    simulationIconPosition.clear();
    if (editModeType == NO_EDIT_MODE)
    {
        for (int i = 0; i < int(rootElements.size()); i++)
        {
            std::vector<int> vertLines;
            rootElements[i]->renderElement_sceneObject(this, labelEditObjectID, bright, true, renderingSize, textPos, 0,
                                                       &vertLines, minRenderedPosition, maxRenderedPosition);
        }
        _drawLinesLinkingDummies(maxRenderedPosition);
    }
    std::vector<char> editModeSelectionStateList;
    if ((editModeType & SHAPE_EDIT_MODE) || (editModeType & PATH_EDIT_MODE_OLD))
    {
        if (editModeType & VERTEX_EDIT_MODE)
            editModeSelectionStateList.resize(
                GuiApp::mainWindow->editModeContainer->getShapeEditMode()->getEditionVerticesSize() / 3, 0);
        if (editModeType & TRIANGLE_EDIT_MODE)
            editModeSelectionStateList.resize(
                GuiApp::mainWindow->editModeContainer->getShapeEditMode()->getEditionIndicesSize() / 3, 0);
        if (editModeType & EDGE_EDIT_MODE)
            editModeSelectionStateList.resize(
                GuiApp::mainWindow->editModeContainer->getShapeEditMode()->getEditionEdgesSize() / 2, 0);
        if (editModeType & PATH_EDIT_MODE_OLD)
            editModeSelectionStateList.resize(
                GuiApp::mainWindow->editModeContainer->getEditModePathContainer_old()->getSimplePathPointCount(), 0);
        for (int i = 0; i < GuiApp::mainWindow->editModeContainer->getEditModeBufferSize(); i++)
        {
            int ind = GuiApp::mainWindow->editModeContainer->getEditModeBufferValue(i);
            if (i == GuiApp::mainWindow->editModeContainer->getEditModeBufferSize() - 1)
                editModeSelectionStateList[ind] = 2; // last selection
            else
                editModeSelectionStateList[ind] = 1;
        }
        for (int i = 0; i < int(rootElements.size()); i++)
        {
            rootElements[i]->renderElement_editModeList(this, labelEditObjectID, bright, true, renderingSize, textPos,
                                                        0, minRenderedPosition, maxRenderedPosition,
                                                        editModeSelectionStateList[i], editModeType);
        }
    }
    if (editModeType & MULTISHAPE_EDIT_MODE)
    {
        for (int i = 0; i < int(rootElements.size()); i++)
        {
            rootElements[i]->renderElement_editModeList(this, -1, bright, true, renderingSize, textPos, 0,
                                                        minRenderedPosition, maxRenderedPosition, 0, editModeType);
        }
    }
    maxRenderedPosition[0] += 8 * GuiApp::sc;
    maxRenderedPosition[0] = maxRenderedPosition[0] - viewPosition[0];
    maxRenderedPosition[1] = maxRenderedPosition[1] - viewPosition[1];
    minRenderedPosition[0] = minRenderedPosition[0] - viewPosition[0];
    minRenderedPosition[1] = minRenderedPosition[1] - viewPosition[1];

    // Second pass where we display:
    validateViewPosition();

    textPos[0] = viewPosition[0];
    textPos[1] = viewPosition[1] + renderingSize[1];
    minRenderedPosition[0] = 100000;
    minRenderedPosition[1] = 100000;
    maxRenderedPosition[0] = -100000;
    maxRenderedPosition[1] = -100000;
    bright = false;
    inflateIconPosition.clear();
    scriptIconPosition.clear();
    scriptParametersIconPosition.clear();
    objectPosition.clear();
    objectIconPosition.clear();
    modelIconPosition.clear();
    textPosition.clear();
    lineLastPosition.clear();
    simulationIconPosition.clear();
    if (editModeType == NO_EDIT_MODE)
    {
        int objFromHalf = -1;
        if (hierarchDragUnderway)
            objFromHalf = _mouseDownDragObjectID;

        int worldClickThing = -9999;
        if (_worldSelectID_down == _worldSelectID_moving)
            worldClickThing = _worldSelectID_down;

        for (size_t i = 0; i < rootElements.size(); i++)
        {
            std::vector<int> vertLines;
            rootElements[i]->renderElement_sceneObject(this, labelEditObjectID, bright, false, renderingSize, textPos,
                                                       0, &vertLines, minRenderedPosition, maxRenderedPosition, false,
                                                       objFromHalf, dropID, worldClickThing);
        }
        while (CHierarchyElement::renderDummyElement(bright, renderingSize, textPos))
            ;
        _drawLinesLinkingDummies(maxRenderedPosition);

        if (hierarchDragUnderway)
        {
            textPos[0] = dx + _mouseDownDragOffset[0];
            textPos[1] = dy + _mouseDownDragOffset[1];

            CHierarchyElement *it = nullptr;
            for (size_t i = 0; i < rootElements.size(); i++)
            {
                it = rootElements[i]->getElementLinkedWithObject(_mouseDownDragObjectID);
                if (it != nullptr)
                    break;
            }
            if (it != nullptr)
            { // should normally never be nullptr
                std::vector<int> vertLines;
                int renderingSizeCopy[2] = {renderingSize[0], renderingSize[1]};
                int minRenderedPositionCopy[2] = {minRenderedPosition[0], minRenderedPosition[1]};
                int maxRenderedPositionCopy[2] = {maxRenderedPosition[0], maxRenderedPosition[1]};
                it->renderElement_sceneObject(this, labelEditObjectID, bright, false, renderingSizeCopy, textPos, 0,
                                              &vertLines, minRenderedPositionCopy, maxRenderedPositionCopy, true);
            }
        }
    }
    if ((editModeType & SHAPE_EDIT_MODE) || (editModeType & PATH_EDIT_MODE_OLD))
    {
        for (int i = 0; i < int(rootElements.size()); i++)
        {
            rootElements[i]->renderElement_editModeList(this, labelEditObjectID, bright, false, renderingSize, textPos,
                                                        0, minRenderedPosition, maxRenderedPosition,
                                                        editModeSelectionStateList[i], editModeType);
        }
        while (CHierarchyElement::renderDummyElement(bright, renderingSize, textPos))
            ;
    }
    if (editModeType & MULTISHAPE_EDIT_MODE)
    {
        for (int i = 0; i < int(rootElements.size()); i++)
        {
            rootElements[i]->renderElement_editModeList(this, -1, bright, false, renderingSize, textPos, 0,
                                                        minRenderedPosition, maxRenderedPosition, 0, editModeType);
        }
        while (CHierarchyElement::renderDummyElement(bright, renderingSize, textPos))
            ;
    }
    maxRenderedPosition[0] += 8 * GuiApp::sc;
    maxRenderedPosition[0] = maxRenderedPosition[0] - viewPosition[0];
    maxRenderedPosition[1] = maxRenderedPosition[1] - viewPosition[1];
    minRenderedPosition[0] = minRenderedPosition[0] - viewPosition[0];
    minRenderedPosition[1] = minRenderedPosition[1] - viewPosition[1];

    // We now draw the selection square:
    if (shiftSelectionStarted)
    {
        int limitedPos[2] = {mouseRelativePosition[0], mouseRelativePosition[1]};
        if (limitedPos[0] > renderingSize[0])
            limitedPos[0] = renderingSize[0];
        if (limitedPos[1] > renderingSize[1])
            limitedPos[1] = renderingSize[1];
        if (limitedPos[0] < SAFETY_BORDER_SIZE * GuiApp::sc)
            limitedPos[0] = SAFETY_BORDER_SIZE * GuiApp::sc;
        if (limitedPos[1] < SAFETY_BORDER_SIZE * GuiApp::sc)
            limitedPos[1] = SAFETY_BORDER_SIZE * GuiApp::sc;
        ogl::setAlpha(0.2);
        ogl::setMaterialColor(sim_colorcomponent_emission, ogl::colorYellow);
        ogl::setBlending(true, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBegin(GL_QUADS);
        glVertex3i(mouseDownRelativePosition[0], mouseDownRelativePosition[1], 0);
        glVertex3i(mouseDownRelativePosition[0], limitedPos[1], 0);
        glVertex3i(limitedPos[0], limitedPos[1], 0);
        glVertex3i(limitedPos[0], mouseDownRelativePosition[1], 0);
        glEnd();
        ogl::setBlending(false);
        ogl::setMaterialColor(sim_colorcomponent_emission, ogl::colorBlack);
        glBegin(GL_LINE_STRIP);
        glVertex3i(mouseDownRelativePosition[0], mouseDownRelativePosition[1], 0);
        glVertex3i(mouseDownRelativePosition[0], limitedPos[1], 0);
        glVertex3i(limitedPos[0], limitedPos[1], 0);
        glVertex3i(limitedPos[0], mouseDownRelativePosition[1], 0);
        glVertex3i(mouseDownRelativePosition[0], mouseDownRelativePosition[1], 0);
        glEnd();
    }
    int vtl[2];
    int vbr[2];
    int htl[2];
    int hbr[2];
    int slidersEnable = getSliderPositions(vtl, vbr, htl, hbr, nullptr);
    //  double black[3]={0.0,0.0,0.0};

    if (slidersEnable & 1)
    { // here we draw the vertical slider:
        ogl::setMaterialColor(sim_colorcomponent_emission, ogl::HIERARCHY_AND_BROWSER_SCROLLBAR_BACK_COLOR);
        int leftTop[2] = {renderingSize[0] - verticalScrollbarWidth, renderingSize[1]};
        int rightBottom[2] = {renderingSize[0], 0};
        glBegin(GL_QUADS);
        glVertex3i(leftTop[0], leftTop[1], 0);
        glVertex3i(rightBottom[0], leftTop[1], 0);
        glVertex3i(rightBottom[0], rightBottom[1], 0);
        glVertex3i(leftTop[0], rightBottom[1], 0);
        glEnd();

        ogl::setMaterialColor(sim_colorcomponent_emission, ogl::HIERARCHY_AND_BROWSER_SCROLLBAR_COLOR);
        glBegin(GL_QUADS);
        glVertex3i(vtl[0], vtl[1], 0);
        glVertex3i(vbr[0], vtl[1], 0);
        glVertex3i(vbr[0], vbr[1], 0);
        glVertex3i(vtl[0], vbr[1], 0);
        glEnd();

        ogl::setMaterialColor(sim_colorcomponent_emission, ogl::SEPARATION_LINE_COLOR);
        ogl::drawSingle2dLine_i(leftTop[0], leftTop[1], leftTop[0], rightBottom[1]);
    }
    if (slidersEnable & 2)
    { // here we draw the horizontal slider:
        ogl::setMaterialColor(sim_colorcomponent_emission, ogl::HIERARCHY_AND_BROWSER_SCROLLBAR_BACK_COLOR);
        int leftTop[2] = {0, horizontalScrollbarHeight + SAFETY_BORDER_SIZE * GuiApp::sc};
        int rightBottom[2] = {renderingSize[0], 0};
        glBegin(GL_QUADS);
        glVertex3i(leftTop[0], leftTop[1], 0);
        glVertex3i(rightBottom[0], leftTop[1], 0);
        glVertex3i(rightBottom[0], rightBottom[1], 0);
        glVertex3i(leftTop[0], rightBottom[1], 0);
        glEnd();
        ogl::setMaterialColor(sim_colorcomponent_emission, ogl::HIERARCHY_AND_BROWSER_SCROLLBAR_COLOR);
        glBegin(GL_QUADS);
        glVertex3i(htl[0], htl[1], 0);
        glVertex3i(hbr[0], htl[1], 0);
        glVertex3i(hbr[0], hbr[1], 0);
        glVertex3i(htl[0], hbr[1], 0);
        glEnd();

        ogl::setMaterialColor(sim_colorcomponent_emission, ogl::SEPARATION_LINE_COLOR);
        ogl::drawSingle2dLine_i(leftTop[0], leftTop[1], rightBottom[0] - verticalScrollbarWidth, leftTop[1]);
    }
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_SCISSOR_TEST);
    return (true);
}

int CHierarchy::getSliderPositions(int vSliderTopLeft[2], int vSliderBottomRight[2], int hSliderTopLeft[2],
                                   int hSliderBottomRight[2], double prop[2])
{
    // we check if we need the sliders:
    verticalScrollbarWidth = 0;
    horizontalScrollbarHeight = 0;
    int minMaxViewPosition[2];
    int minViewPosition[2];
    for (int i = 0; i < 3; i++)
    { // we need to do it 3 times because one added slider might mean we have to add the other one!
        int savedViewPosition[2] = {viewPosition[0], viewPosition[1]};
        viewPosition[0] = -99999999;
        viewPosition[1] = -99999999;
        validateViewPosition();
        minViewPosition[0] = viewPosition[0];
        minViewPosition[1] = viewPosition[1];
        viewPosition[0] = +99999999;
        viewPosition[1] = +99999999;
        validateViewPosition();
        int maxViewPosition[2] = {viewPosition[0], viewPosition[1]};
        minMaxViewPosition[0] = maxViewPosition[0] - minViewPosition[0];
        minMaxViewPosition[1] = maxViewPosition[1] - minViewPosition[1];
        viewPosition[0] = savedViewPosition[0];
        viewPosition[1] = savedViewPosition[1];
        if (minMaxViewPosition[0] != 0)
            horizontalScrollbarHeight = HIERARCHY_SCROLLBAR_WIDTH * GuiApp::sc;
        if (minMaxViewPosition[1] != 0)
            verticalScrollbarWidth = HIERARCHY_SCROLLBAR_WIDTH * GuiApp::sc;
    }
    int retVal = 0;
    if (verticalScrollbarWidth > 0)
    { // here we compute the vertical slider:
        retVal |= 1;
        int effDr = renderingSize[1] - horizontalScrollbarHeight - SAFETY_BORDER_SIZE * GuiApp::sc; //-4;
        int sliderS = effDr * effDr / (effDr + minMaxViewPosition[1]);
        if (sliderS < HIERARCHY_SCROLLBAR_WIDTH * GuiApp::sc)
            sliderS = HIERARCHY_SCROLLBAR_WIDTH * GuiApp::sc;
        int sliderP =
            int(double(effDr - sliderS) * double(viewPosition[1] - minViewPosition[1]) / double(minMaxViewPosition[1]));
        vSliderTopLeft[0] = renderingSize[0] - verticalScrollbarWidth;
        vSliderBottomRight[0] = renderingSize[0];
        vSliderTopLeft[1] = renderingSize[1] - sliderP;
        vSliderBottomRight[1] = vSliderTopLeft[1] - sliderS;
        if (prop != nullptr)
            prop[0] = double(minMaxViewPosition[1]) / double(effDr - sliderS);
    }
    if (horizontalScrollbarHeight > 0)
    { // here we compute the horizontal slider:
        retVal |= 2;
        int effDr = renderingSize[0] - verticalScrollbarWidth + (-SAFETY_BORDER_SIZE - CONST_VAL_4) * GuiApp::sc;
        int sliderS = effDr * effDr / (effDr + minMaxViewPosition[0]);
        if (sliderS < HIERARCHY_SCROLLBAR_WIDTH * GuiApp::sc)
            sliderS = HIERARCHY_SCROLLBAR_WIDTH * GuiApp::sc;
        int sliderP =
            int(double(effDr - sliderS) * double(viewPosition[0] - minViewPosition[0]) / double(minMaxViewPosition[0]));
        hSliderTopLeft[1] = horizontalScrollbarHeight + SAFETY_BORDER_SIZE * GuiApp::sc;
        hSliderBottomRight[1] = 0;
        hSliderTopLeft[0] = effDr - sliderS - sliderP + SAFETY_BORDER_SIZE * GuiApp::sc;
        hSliderBottomRight[0] = hSliderTopLeft[0] + sliderS;
        if (prop != nullptr)
            prop[1] = double(minMaxViewPosition[0]) / double(effDr - sliderS);
    }
    return (retVal);
}

bool CHierarchy::leftMouseDown(int x, int y, int selectionStatus)
{
    _mouseDownDragObjectID = -1;
    _worldSelectID_down = -9999;
    if (x < 0)
        return (false);
    if (y < 0)
        return (false);
    if (x > renderingSize[0] - SAFETY_BORDER_SIZE * GuiApp::sc)
        return (false);
    if (y > renderingSize[1] - SAFETY_BORDER_SIZE * GuiApp::sc)
        return (false);
    // The mouse went down on the hierarchy window!
    _caughtElements |= sim_left_button;

    refreshViewFlag = App::userSettings->hierarchyRefreshCnt;
    mouseDownRelativePosition[0] = x + SAFETY_BORDER_SIZE * GuiApp::sc;
    mouseDownRelativePosition[1] = y + SAFETY_BORDER_SIZE * GuiApp::sc;
    mouseRelativePosition[0] = x + SAFETY_BORDER_SIZE * GuiApp::sc;
    mouseRelativePosition[1] = y + SAFETY_BORDER_SIZE * GuiApp::sc;
    previousMouseRelativePosition[0] = x + SAFETY_BORDER_SIZE * GuiApp::sc;
    previousMouseRelativePosition[1] = y + SAFETY_BORDER_SIZE * GuiApp::sc;
    shiftSelectionStarted = false;
    labelEditObjectID = -1;

    int vtl[2];
    int vbr[2];
    int htl[2];
    int hbr[2];
    int slidersEnable = getSliderPositions(vtl, vbr, htl, hbr, nullptr);
    sliderMoveMode = 0;
    if ((slidersEnable & 1) && (mouseDownRelativePosition[0] > vtl[0]) && (mouseDownRelativePosition[0] < vbr[0]))
    {
        if ((mouseDownRelativePosition[1] < vtl[1]) && (mouseDownRelativePosition[1] > vbr[1]))
        { // Vertical slider action
            sliderMoveMode = 1;
            viewPosWhenMouseOnSliderDown[0] = viewPosition[0];
            viewPosWhenMouseOnSliderDown[1] = viewPosition[1];
            return (true);
        }
        else
        { // We clicked the slider, but not the mobile part
            return (true);
        }
    }
    if ((slidersEnable & 2) && (mouseDownRelativePosition[1] < htl[1]) && (mouseDownRelativePosition[1] > hbr[1]))
    {
        if ((mouseDownRelativePosition[0] > htl[0]) && (mouseDownRelativePosition[0] < hbr[0]))
        { // Horizontal slider action
            sliderMoveMode = 2;
            viewPosWhenMouseOnSliderDown[0] = viewPosition[0];
            viewPosWhenMouseOnSliderDown[1] = viewPosition[1];
            return (true);
        }
        else
        { // We clicked the slider, but not the mobile part
            return (true);
        }
    }
    bool clickSelection = false;
    if ((selectionStatus != CTRLSELECTION) && (selectionStatus != SHIFTSELECTION))
    {
        clickSelection = true;
    }

    shiftingAllowed = ((selectionStatus != CTRLSELECTION) && (selectionStatus != SHIFTSELECTION));
    int objID = getInflateActionObjectID(mouseDownRelativePosition[0], mouseDownRelativePosition[1]);
    bool canSelect = true;
    if ((objID >= 0) && (selectionStatus != CTRLSELECTION) && (selectionStatus != SHIFTSELECTION))
    { // Expansion/collapse
        CSceneObject *it = App::currentWorld->sceneObjects->getObjectFromHandle(objID);
        if (it != nullptr)
        {
            SSimulationThreadCommand cmd;
            cmd.cmdId = TOGGLE_EXPAND_COLLAPSE_HIERARCHY_OBJECT_CMD;
            cmd.intParams.push_back(it->getObjectHandle());
            App::appendSimulationThreadCommand(cmd);
            shiftingAllowed = false;
            canSelect = false;
        }
    }
    if ((GuiApp::getEditModeType() == NO_EDIT_MODE) && canSelect)
    { // NO EDIT MODE
        if ((selectionStatus == CTRLSELECTION) || clickSelection)
        {
            int objID = getActionObjectID(mouseDownRelativePosition[1], HIERARCHY_HALF_INTER_LINE_SPACE * GuiApp::sc);
            if (objID >= 0)
            {
                CSceneObject *obj = App::currentWorld->sceneObjects->getObjectFromHandle(objID);
                if (obj != nullptr) // just in case
                {
                    if (selectionStatus == CTRLSELECTION)
                    {
                        App::currentWorld->sceneObjects->xorAddObjectToSelection(objID); // Normal selection/deselection
                    }
                    else
                    {
                        App::currentWorld->sceneObjects->deselectObjects();
                        App::currentWorld->sceneObjects->addObjectToSelection(objID); // Normal selection
                        int dxv[2];
                        if (getLineObjectID(mouseDownRelativePosition[1], dxv) == objID)
                        {
                            _mouseDownDragObjectID = objID;
                            _mouseDownDragOffset[0] = dxv[0];
                            _mouseDownDragOffset[1] = dxv[1];
                        }
                    }
                }
            }
            else
            {
                if (objID == -9999)
                    App::currentWorld->sceneObjects->deselectObjects();
                else
                {
                    _worldSelectID_down = objID;
                    _worldSelectID_moving = objID;
                }
            }
        }
        if (selectionStatus == SHIFTSELECTION)
            shiftSelectionStarted = true;
    }
    if (((GuiApp::getEditModeType() & SHAPE_EDIT_MODE) || (GuiApp::getEditModeType() & PATH_EDIT_MODE_OLD)) &&
        canSelect)
    { // SHAPE OR PATH EDIT MODE
        if ((selectionStatus == CTRLSELECTION) || clickSelection)
        {
            int objID = getActionObjectID(mouseDownRelativePosition[1], HIERARCHY_HALF_INTER_LINE_SPACE * GuiApp::sc);
            if (objID >= 0)
            {
                if (selectionStatus == CTRLSELECTION)
                    GuiApp::mainWindow->editModeContainer->xorAddItemToEditModeBuffer(objID, true);
                else
                {
                    GuiApp::mainWindow->editModeContainer->deselectEditModeBuffer();
                    GuiApp::mainWindow->editModeContainer->addItemToEditModeBuffer(objID, true);
                }
            }
            else
                GuiApp::mainWindow->editModeContainer->deselectEditModeBuffer();
        }
        if (selectionStatus == SHIFTSELECTION)
            shiftSelectionStarted = true;
    }
    if ((GuiApp::getEditModeType() & MULTISHAPE_EDIT_MODE) && canSelect)
    { // MULTISHAPE EDIT MODE
        if ((selectionStatus == CTRLSELECTION) || clickSelection)
        {
            int objID = getActionObjectID(mouseDownRelativePosition[1], HIERARCHY_HALF_INTER_LINE_SPACE * GuiApp::sc);
            GuiApp::mainWindow->editModeContainer->getMultishapeEditMode()->setMultishapeGeometricComponentIndex(objID);
            GuiApp::setLightDialogRefreshFlag();
        }
    }
    return (true);
}

void CHierarchy::leftMouseUp(int x, int y)
{
    int dx = (x + SAFETY_BORDER_SIZE * GuiApp::sc) - mouseDownRelativePosition[0];
    int dy = (y + SAFETY_BORDER_SIZE * GuiApp::sc) - mouseDownRelativePosition[1];
    bool hierarchDragUnderway = ((abs(dx) > 8) || (abs(dy) > 8));
    if ((_mouseDownDragObjectID != -1) && hierarchDragUnderway)
    {
        if (App::currentWorld->sceneObjects->getLastSelectionHandle() == _mouseDownDragObjectID)
        {

            if (objectIDWhereTheMouseCurrentlyIs_minus9999ForNone == -9999)
                objectIDWhereTheMouseCurrentlyIs_minus9999ForNone =
                    -App::worldContainer->getCurrentWorldIndex() - 1; // world
            else
            {
                if (objectIDWhereTheMouseCurrentlyIs_minus9999ForNone < 0)
                {
                    if (objectIDWhereTheMouseCurrentlyIs_minus9999ForNone !=
                        -App::worldContainer->getCurrentWorldIndex() - 1)
                        objectIDWhereTheMouseCurrentlyIs_minus9999ForNone = -9999;
                }
            }

            if ((objectIDWhereTheMouseCurrentlyIs_minus9999ForNone != _mouseDownDragObjectID) &&
                (objectIDWhereTheMouseCurrentlyIs_minus9999ForNone != -9999))
            {
                SSimulationThreadCommand cmd;
                cmd.cmdId = MAKE_OBJECT_CHILD_OF_CMD;
                cmd.intParams.push_back(_mouseDownDragObjectID);
                cmd.intParams.push_back(objectIDWhereTheMouseCurrentlyIs_minus9999ForNone);
                App::appendSimulationThreadCommand(cmd);
            }
        }
    }

    _mouseDownDragObjectID = -1;
    sliderMoveMode = 0;
    refreshViewFlag = App::userSettings->hierarchyRefreshCnt;
    mouseRelativePosition[0] = x + SAFETY_BORDER_SIZE * GuiApp::sc;
    mouseRelativePosition[1] = y + SAFETY_BORDER_SIZE * GuiApp::sc;

    if (shiftSelectionStarted && (_caughtElements & sim_left_button))
    { // SHIFT SELECTION
        int limitedPos = mouseRelativePosition[1];
        if (limitedPos > renderingSize[1])
            limitedPos = renderingSize[1];
        if (limitedPos < SAFETY_BORDER_SIZE * GuiApp::sc)
            limitedPos = SAFETY_BORDER_SIZE * GuiApp::sc;
        std::vector<int> objToBeSelected;
        getActionObjectIDs(mouseDownRelativePosition[1], limitedPos, HIERARCHY_HALF_INTER_LINE_SPACE * GuiApp::sc,
                           &objToBeSelected);
        if (GuiApp::getEditModeType() == NO_EDIT_MODE)
        {
            for (int i = 0; i < int(objToBeSelected.size()); i++)
            {
                CSceneObject *obj = App::currentWorld->sceneObjects->getObjectFromHandle(objToBeSelected[i]);
                if (obj != nullptr) // Just in case
                {
                    App::currentWorld->sceneObjects->addObjectToSelection(objToBeSelected[i]); // Normal selection
                }
            }
        }
        if ((GuiApp::getEditModeType() & SHAPE_EDIT_MODE) || (GuiApp::getEditModeType() & PATH_EDIT_MODE_OLD))
        {
            for (int i = 0; i < int(objToBeSelected.size()); i++)
                GuiApp::mainWindow->editModeContainer->addItemToEditModeBuffer(objToBeSelected[i], true);
        }
    }
    _caughtElements &= 0xffff - sim_left_button;
    shiftingAllowed = false;
    shiftSelectionStarted = false;

    // We have to do this at the very end of the routine since we are switching instances:
    if ((_worldSelectID_down != -9999) &&
        (_worldSelectID_down ==
         getActionObjectID(mouseRelativePosition[1], HIERARCHY_HALF_INTER_LINE_SPACE * GuiApp::sc)))
    {
        int nii = (-_worldSelectID_down) - 1;
        _worldSelectID_down = -9999;
        App::worldContainer->setInstanceIndexWithThumbnails(nii);
    }
}
bool CHierarchy::rightMouseDown(int x, int y)
{
    if (x < 0)
        return (false);
    if (y < 0)
        return (false);
    if (x > renderingSize[0] - SAFETY_BORDER_SIZE * GuiApp::sc)
        return (false);
    if (y > renderingSize[1] - SAFETY_BORDER_SIZE * GuiApp::sc)
        return (false);
    // The mouse went down on the hierarchy window
    _caughtElements |= sim_right_button;
    refreshViewFlag = App::userSettings->hierarchyRefreshCnt;
    mouseDownRelativePosition[0] = x + SAFETY_BORDER_SIZE * GuiApp::sc;
    mouseDownRelativePosition[1] = y + SAFETY_BORDER_SIZE * GuiApp::sc;
    mouseRelativePosition[0] = x + SAFETY_BORDER_SIZE * GuiApp::sc;
    mouseRelativePosition[1] = y + SAFETY_BORDER_SIZE * GuiApp::sc;
    previousMouseRelativePosition[0] = x + SAFETY_BORDER_SIZE * GuiApp::sc;
    previousMouseRelativePosition[1] = y + SAFETY_BORDER_SIZE * GuiApp::sc;
    objectIDWhereTheMouseCurrentlyIs_minus9999ForNone = getActionObjectID_icon(0, mouseRelativePosition[1], true);
    labelEditObjectID = -1;
    return (true); // We catch this event to display a popup-menu when the mouse comes up
}
void CHierarchy::rightMouseUp(int x, int y, int absX, int absY, QWidget *mainWindow)
{ // Only caught if right button was caught by the hierarchy!
    _caughtElements &= 0xffff - sim_right_button;
    if ((x < 0) || (y < 0) || (x > renderingSize[0] - SAFETY_BORDER_SIZE * GuiApp::sc) ||
        (y > renderingSize[1] - SAFETY_BORDER_SIZE * GuiApp::sc))
        return;

    // The mouse went up on the hierarchy window
    refreshViewFlag = App::userSettings->hierarchyRefreshCnt;
    mouseRelativePosition[0] = x + SAFETY_BORDER_SIZE * GuiApp::sc;
    mouseRelativePosition[1] = y + SAFETY_BORDER_SIZE * GuiApp::sc;

    if (GuiApp::operationalUIParts & sim_gui_popups)
    { // Default popups
        if (GuiApp::getEditModeType() == NO_EDIT_MODE)
        {
            VMenu mainMenu = VMenu();
            addMenu(&mainMenu);

            VMenu *objectEditionMenu = new VMenu();
            CSceneObjectOperations::addMenu(objectEditionMenu);
            mainMenu.appendMenuAndDetach(objectEditionMenu, true, IDS_EDIT_MENU_ITEM);

            VMenu *addMenu = new VMenu();
            CAddOperations::addMenu(addMenu, nullptr, false);
            mainMenu.appendMenuAndDetach(addMenu, true, IDS_ADD_MENU_ITEM);

            size_t selSize = App::currentWorld->sceneObjects->getSelectionCount();
            VMenu *hierarchyColoringMenu = new VMenu();
            bool cols[4] = {false, false, false, false};
            if (selSize > 0)
            {
                for (size_t i = 0; i < selSize; i++)
                {
                    CSceneObject *anO = App::currentWorld->sceneObjects->getObjectFromHandle(
                        App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
                    int colInd = anO->getHierarchyColorIndex();
                    if (colInd == -1)
                        cols[0] = true;
                    if (colInd == 0)
                        cols[1] = true;
                    if (colInd == 1)
                        cols[2] = true;
                    if (colInd == 2)
                        cols[3] = true;
                }
                hierarchyColoringMenu->appendMenuItem(true, cols[0], HIERARCHY_COLORING_NONE_CMD, "None", true);
                hierarchyColoringMenu->appendMenuItem(true, cols[1], HIERARCHY_COLORING_RED_CMD, "Red", true);
                hierarchyColoringMenu->appendMenuItem(true, cols[2], HIERARCHY_COLORING_GREEN_CMD, "Green", true);
                hierarchyColoringMenu->appendMenuItem(true, cols[3], HIERARCHY_COLORING_BLUE_CMD, "Blue", true);
            }
            mainMenu.appendMenuAndDetach(hierarchyColoringMenu, selSize > 0, "Hierarchy coloring");

            int command = mainMenu.trackPopupMenu();
            bool processed = false;
            if (!processed)
                processed = processCommand(command);
            if (!processed)
                processed = CSceneObjectOperations::processCommand(command);
            if (!processed)
                processed = CAddOperations::processCommand(command, nullptr);
        }
        else
        {
            VMenu mainMenu = VMenu();

            if (GuiApp::getEditModeType() & SHAPE_EDIT_MODE)
                GuiApp::mainWindow->editModeContainer->addMenu(&mainMenu, nullptr);
            if (GuiApp::getEditModeType() & PATH_EDIT_MODE_OLD)
                GuiApp::mainWindow->editModeContainer->addMenu(&mainMenu, nullptr);

            int command = mainMenu.trackPopupMenu();

            if (GuiApp::getEditModeType() & SHAPE_EDIT_MODE)
                GuiApp::mainWindow->editModeContainer->processCommand(command, nullptr);
            if (GuiApp::getEditModeType() & PATH_EDIT_MODE_OLD)
                GuiApp::mainWindow->editModeContainer->processCommand(command, nullptr);
        }
    }
}

bool CHierarchy::mouseWheel(int deltaZ, int x, int y)
{
    if ((x < 0) || (y < 0) || (x > renderingSize[0] - SAFETY_BORDER_SIZE * GuiApp::sc) ||
        (y > renderingSize[1] - SAFETY_BORDER_SIZE * GuiApp::sc))
        return (false);
    refreshViewFlag = App::userSettings->hierarchyRefreshCnt;
    viewPosition[1] = viewPosition[1] - (deltaZ / 120) * HIERARCHY_INTER_LINE_SPACE * GuiApp::sc;
    return (true);
}

void CHierarchy::mouseMove(int x, int y, bool passiveAndFocused)
{
    _worldSelectID_moving = -9999;
    mouseRelativePosition[0] = x + SAFETY_BORDER_SIZE * GuiApp::sc;
    mouseRelativePosition[1] = y + SAFETY_BORDER_SIZE * GuiApp::sc;
    objectIDWhereTheMouseCurrentlyIs_minus9999ForNone = getActionObjectID_icon(0, mouseRelativePosition[1], true);
    if (!passiveAndFocused)
    {
        int vtl[2];
        int vbr[2];
        int htl[2];
        int hbr[2];
        double prop[2];
        if (sliderMoveMode & 1)
        { // we are moving the vertical slider
            getSliderPositions(vtl, vbr, htl, hbr, prop);
            viewPosition[1] = viewPosWhenMouseOnSliderDown[1] -
                              int(prop[0] * double(mouseRelativePosition[1] - mouseDownRelativePosition[1]));
            refreshViewFlag = App::userSettings->hierarchyRefreshCnt;
        }
        else if (sliderMoveMode & 2)
        { // we are moving the horizontal slider
            getSliderPositions(vtl, vbr, htl, hbr, prop);
            viewPosition[0] = viewPosWhenMouseOnSliderDown[0] -
                              int(prop[1] * double(mouseRelativePosition[0] - mouseDownRelativePosition[0]));
            refreshViewFlag = App::userSettings->hierarchyRefreshCnt;
        }
        else if (shiftingAllowed)
        {
            refreshViewFlag = App::userSettings->hierarchyRefreshCnt;
            int objID = getActionObjectID(mouseRelativePosition[1], HIERARCHY_HALF_INTER_LINE_SPACE * GuiApp::sc);
            if (objID < 0)
                _worldSelectID_moving = objID;
        }
        else if (shiftSelectionStarted)
            refreshViewFlag = App::userSettings->hierarchyRefreshCnt;
    }
    previousMouseRelativePosition[0] = mouseRelativePosition[0];
    previousMouseRelativePosition[1] = mouseRelativePosition[1];
}

bool CHierarchy::leftMouseDblClick(int x, int y, int selectionStatus)
{
    // Following few on 23/9/2013, because somehow with Qt5 there is no mouse up event following:
    _mouseDownDragObjectID = -1;
    _worldSelectID_down = -9999;
    sliderMoveMode = 0;
    _caughtElements &= 0xffff - sim_left_button;

    if ((x < 0) || (y < 0) || (x > renderingSize[0] - SAFETY_BORDER_SIZE * GuiApp::sc) ||
        (y > renderingSize[1] - SAFETY_BORDER_SIZE * GuiApp::sc))
        return (false);
    // The mouse went down on the hierarchy window
    refreshViewFlag = App::userSettings->hierarchyRefreshCnt;
    mouseDownRelativePosition[0] = x + SAFETY_BORDER_SIZE * GuiApp::sc;
    mouseDownRelativePosition[1] = y + SAFETY_BORDER_SIZE * GuiApp::sc;
    mouseRelativePosition[0] = x + SAFETY_BORDER_SIZE * GuiApp::sc;
    mouseRelativePosition[1] = y + SAFETY_BORDER_SIZE * GuiApp::sc;
    previousMouseRelativePosition[0] = x + SAFETY_BORDER_SIZE * GuiApp::sc;
    previousMouseRelativePosition[1] = y + SAFETY_BORDER_SIZE * GuiApp::sc;
    shiftSelectionStarted = false;
    shiftingAllowed = false;

    // We check if we have to launch a script editor window:
    int scriptID = getScriptActionObjectID(mouseDownRelativePosition[0], mouseDownRelativePosition[1]);
    if (scriptID != -1)
    {
        CScriptObject *it = App::currentWorld->sceneObjects->embeddedScriptContainer->getScriptObjectFromHandle(scriptID);
        if (it != nullptr)
        {
            // Process the command via the simulation thread (delayed):
            SSimulationThreadCommand cmd;
            cmd.cmdId = OPEN_SCRIPT_EDITOR_CMD;
            cmd.intParams.push_back(it->getScriptHandle());
            App::appendSimulationThreadCommand(cmd);
        }
        return (true);
    }
    int objID = getScriptParameterActionObjectID(mouseDownRelativePosition[0], mouseDownRelativePosition[1]);
    if (objID != -1)
    {
        if ((GuiApp::operationalUIParts & sim_gui_scriptsimulationparameters) != 0)
        {
            // Process the command via the simulation thread (delayed):
            SSimulationThreadCommand cmd;
            cmd.cmdId = CALL_USER_CONFIG_CALLBACK_CMD;
            cmd.intParams.push_back(objID);
            App::appendSimulationThreadCommand(cmd);
        }
        return (true);
    }
    if (GuiApp::getEditModeType() == NO_EDIT_MODE)
    {
        int objID = -1;
        // Do we need to open an object property dialog?
        objID = getActionObjectID_icon(mouseDownRelativePosition[0], mouseDownRelativePosition[1]);
        if (objID != -9999) // minus numbers are for the world(s)
        {                   // yes!
            if (GuiApp::mainWindow->getObjPropToggleViaGuiEnabled())
            {
                if (objID >= 0)
                { // Regular object
                    App::currentWorld->sceneObjects->deselectObjects();
                    App::currentWorld->sceneObjects->addObjectToSelection(objID);
                    GuiApp::setFullDialogRefreshFlag();
                    GuiApp::mainWindow->dlgCont->processCommand(OPEN_OBJECT_DLG_OBJECT_SPECIFIC_PART_CMD);

                    CScript* it = App::currentWorld->sceneObjects->getScriptFromHandle(objID);
                    if (it != nullptr)
                    {
                        // Process the command via the simulation thread (delayed):
                        SSimulationThreadCommand cmd;
                        cmd.cmdId = OPEN_SCRIPT_EDITOR_CMD;
                        cmd.intParams.push_back(objID);
                        App::appendSimulationThreadCommand(cmd);
                    }

                }
                else
                { // World object!
                    GuiApp::mainWindow->dlgCont->processCommand(OPEN_ENVIRONMENT_DLG_CMD);
                }
            }
            return (true);
        }

        // Do we need to open a model property dialog?
        objID = getActionModelID_icon(mouseDownRelativePosition[0], mouseDownRelativePosition[1]);
        if (objID >= 0)
        { // yes!
            CSceneObject *it = App::currentWorld->sceneObjects->getObjectFromHandle(objID);
            if (it != nullptr)
            {
                // Process the command via the simulation thread (delayed):
                SSimulationThreadCommand cmd;
                cmd.cmdId = OPEN_MODAL_MODEL_PROPERTIES_CMD;
                cmd.intParams.push_back(it->getObjectHandle());
                App::appendSimulationThreadCommand(cmd);
            }
            return (true);
        }

        // Do we need to open a warning message?
        objID = getSimulationActionObjectID(mouseDownRelativePosition[0], mouseDownRelativePosition[1]);
        if (objID != -1)
        { // yes!
            CSceneObject *it = App::currentWorld->sceneObjects->getObjectFromHandle(objID);
            if (it != nullptr)
            {
                std::string txt;
                if (it->getDynamicSimulationIconCode() == sim_dynamicsimicon_objectisnotdynamicallyenabled)
                {
                    txt = IDS_OBJECT;
                    txt += " '" + it->getObjectAlias_printPath() + "' ";
                    txt += IDS_IS_NOT_DYNAMICALLY_ENABLED_WARNING;
                }
                if (it->getDynamicSimulationIconCode() == sim_dynamicsimicon_objectisdynamicallysimulated)
                {
                    txt = IDS_OBJECT;
                    txt += " '" + it->getObjectAlias_printPath() + "' ";
                    txt += std::string(IDS_IS_DYNAMICALLY_SIMULATED) + "\n";
                    if (it->getObjectType() == sim_object_shape_type)
                    {
                        CShape *so = (CShape *)it;
                        if (so->getStatic())
                        { // static
                            if (so->getRespondable())
                                txt += IDS_SHAPE_IS_STATIC_AND_RESPONDABLE;
                            else
                                txt += IDS_SHAPE_IS_STATIC_AND_NON_RESPONDABLE;
                        }
                        else
                        { // non-static
                            if (so->getRespondable())
                                txt += IDS_SHAPE_IS_NON_STATIC_AND_RESPONDABLE;
                            else
                                txt += IDS_SHAPE_IS_NON_STATIC_AND_NON_RESPONDABLE;
                        }
                    }
                    if (it->getObjectType() == sim_object_joint_type)
                    {
                        CJoint *so = (CJoint *)it;
                        if (so->getJointMode() == sim_jointmode_dynamic)
                            txt += IDS_JOINT_OPERATES_IN_DYNAMIC_MODE;
                        else
                            txt += IDS_JOINT_OPERATES_IN_HYBRID_FASHION;
                    }
                    if (it->getObjectType() == sim_object_forcesensor_type)
                    {
                        CForceSensor *so = (CForceSensor *)it;
                        if (so->getEnableForceThreshold() || so->getEnableTorqueThreshold())
                            txt += IDS_FORCE_SENSOR_IS_BREAKABLE;
                        else
                            txt += IDS_FORCE_SENSOR_IS_NOT_BREAKABLE;
                    }
                }
                if (txt != "")
                    GuiApp::uiThread->messageBox_information(GuiApp::mainWindow, "Dynamic property", txt.c_str(),
                                                             VMESSAGEBOX_OKELI, VMESSAGEBOX_REPLY_OK);
            }
            return (true);
        }
        // Do we need to do some label editing?
        objID = getTextActionObjectID(mouseDownRelativePosition[0], mouseDownRelativePosition[1]);
        if (objID == -9999)
            objID = -1;
        labelEditObjectID = objID;
        CSceneObject *it = App::currentWorld->sceneObjects->getObjectFromHandle(objID);
        if (it != nullptr)
        {
            editionText = it->getObjectAlias();
            editionTextEditPos = (int)editionText.length();
        }
        return (true);
    }

    if (CSimFlavor::getBoolVal(6) && (GuiApp::getEditModeType() & (VERTEX_EDIT_MODE | PATH_EDIT_MODE_OLD)))
    {
        // Did we double-click the icon?
        int objID = getActionObjectID_icon(mouseDownRelativePosition[0], mouseDownRelativePosition[1]);
        if (objID != -9999)
        { // yes!
            GuiApp::mainWindow->editModeContainer->deselectEditModeBuffer();
            GuiApp::mainWindow->editModeContainer->addItemToEditModeBuffer(objID, true);
            GuiApp::setFullDialogRefreshFlag();
            return (true);
        }
    }
    return (false);
}

void CHierarchy::validateViewPosition()
{
    int tmp = renderingSize[0] - maxRenderedPosition[0] - viewPosition[0] - SAFETY_BORDER_SIZE * GuiApp::sc -
              verticalScrollbarWidth;
    if (tmp > 0)
        viewPosition[0] = renderingSize[0] - (maxRenderedPosition[0] - minRenderedPosition[0]) -
                          SAFETY_BORDER_SIZE * GuiApp::sc - verticalScrollbarWidth;
    if (viewPosition[0] > SAFETY_BORDER_SIZE * GuiApp::sc)
        viewPosition[0] = SAFETY_BORDER_SIZE * GuiApp::sc;
    tmp = renderingSize[1] - (maxRenderedPosition[1] - minRenderedPosition[1]) + viewPosition[1] +
          (-SAFETY_BORDER_SIZE - CONST_VAL_4) * GuiApp::sc - horizontalScrollbarHeight;
    if (tmp > 0)
        viewPosition[1] = maxRenderedPosition[1] - minRenderedPosition[1] - renderingSize[1] +
                          (SAFETY_BORDER_SIZE + CONST_VAL_6) * GuiApp::sc + horizontalScrollbarHeight;
    if (viewPosition[1] < (-SAFETY_BORDER_SIZE + CONST_VAL_8) * GuiApp::sc)
        viewPosition[1] = (-SAFETY_BORDER_SIZE + CONST_VAL_8) * GuiApp::sc;
}

void CHierarchy::setRebuildHierarchyFlag()
{
    rebuildHierarchyFlag = true;
    refreshViewFlag = App::userSettings->hierarchyRefreshCnt;
}

void CHierarchy::setResetViewFlag()
{
    resetViewFlag = true;
}

void CHierarchy::setRefreshViewFlag()
{
    refreshViewFlag = App::userSettings->hierarchyRefreshCnt;
}

int CHierarchy::getInflateActionObjectID(int mousePositionX, int mousePositionY)
{
    for (int i = 0; i < int(inflateIconPosition.size()) / 3; i++)
    {
        if ((mousePositionX >= inflateIconPosition[3 * i + 0] - HIERARCHY_ICON_QUARTER_WIDTH * GuiApp::sc) &&
            (mousePositionX <= inflateIconPosition[3 * i + 0] + HIERARCHY_ICON_QUARTER_WIDTH * GuiApp::sc))
        {
            if ((mousePositionY >= inflateIconPosition[3 * i + 1] - HIERARCHY_ICON_QUARTER_HEIGHT * GuiApp::sc) &&
                (mousePositionY <= inflateIconPosition[3 * i + 1] + HIERARCHY_ICON_QUARTER_HEIGHT * GuiApp::sc))
                return (inflateIconPosition[3 * i + 2]);
        }
    }
    return (-1);
}

int CHierarchy::getScriptActionObjectID(int mousePositionX, int mousePositionY)
{
    for (int i = 0; i < int(scriptIconPosition.size()) / 3; i++)
    {
        if ((mousePositionX >= scriptIconPosition[3 * i + 0] - HIERARCHY_ICON_HALF_WIDTH * GuiApp::sc) &&
            (mousePositionX <= scriptIconPosition[3 * i + 0] + HIERARCHY_ICON_HALF_WIDTH * GuiApp::sc))
        {
            if ((mousePositionY >= scriptIconPosition[3 * i + 1] - HIERARCHY_ICON_HALF_HEIGHT * GuiApp::sc) &&
                (mousePositionY <= scriptIconPosition[3 * i + 1] + HIERARCHY_ICON_HALF_HEIGHT * GuiApp::sc))
                return (scriptIconPosition[3 * i + 2]);
        }
    }
    return (-1);
}
int CHierarchy::getScriptParameterActionObjectID(int mousePositionX, int mousePositionY)
{
    for (int i = 0; i < int(scriptParametersIconPosition.size()) / 3; i++)
    {
        if ((mousePositionX >= scriptParametersIconPosition[3 * i + 0] - HIERARCHY_ICON_HALF_WIDTH * GuiApp::sc) &&
            (mousePositionX <= scriptParametersIconPosition[3 * i + 0] + HIERARCHY_ICON_HALF_WIDTH * GuiApp::sc))
        {
            if ((mousePositionY >= scriptParametersIconPosition[3 * i + 1] - HIERARCHY_ICON_HALF_HEIGHT * GuiApp::sc) &&
                (mousePositionY <= scriptParametersIconPosition[3 * i + 1] + HIERARCHY_ICON_HALF_HEIGHT * GuiApp::sc))
                return (scriptParametersIconPosition[3 * i + 2]);
        }
    }
    return (-1);
}

int CHierarchy::getTextActionObjectID(int mousePositionX, int mousePositionY)
{
    for (int i = 0; i < int(textPosition.size()) / 6; i++)
    {
        if ((mousePositionX >= textPosition[6 * i + 2]) && (mousePositionX <= textPosition[6 * i + 3]))
        {
            if ((mousePositionY >= textPosition[6 * i + 4] - HIERARCHY_HALF_INTER_LINE_SPACE * GuiApp::sc) &&
                (mousePositionY <= textPosition[6 * i + 4] + HIERARCHY_HALF_INTER_LINE_SPACE * GuiApp::sc))
                return (textPosition[6 * i + 5]);
        }
    }
    return (-9999); // minus number are for worlds (also in different instances)
}

int CHierarchy::getLineObjectID(int mousePositionY, int textPosStart[2])
{
    for (int i = 0; i < int(textPosition.size()) / 6; i++)
    {
        if ((mousePositionY >= textPosition[6 * i + 4] - HIERARCHY_HALF_INTER_LINE_SPACE * GuiApp::sc) &&
            (mousePositionY <= textPosition[6 * i + 4] + HIERARCHY_HALF_INTER_LINE_SPACE * GuiApp::sc))
        {
            if (textPosStart != nullptr)
            {
                textPosStart[0] = textPosition[6 * i + 0];
                textPosStart[1] = textPosition[6 * i + 1];
            }
            return (textPosition[6 * i + 5]);
        }
    }
    return (-9999); // minus number are for worlds (also in different instances)
}

int CHierarchy::getActionObjectID_icon(int mousePositionX, int mousePositionY,
                                       bool ignoreXPositionAndCheckLineInstead /*=false*/)
{
    if (!ignoreXPositionAndCheckLineInstead)
    {
        for (int i = 0; i < int(objectIconPosition.size()) / 3; i++)
        {
            if ((mousePositionX >= objectIconPosition[3 * i + 0] - HIERARCHY_ICON_HALF_WIDTH * GuiApp::sc) &&
                (mousePositionX <= objectIconPosition[3 * i + 0] + HIERARCHY_ICON_HALF_WIDTH * GuiApp::sc))
            {
                if ((mousePositionY >= objectIconPosition[3 * i + 1] - HIERARCHY_ICON_HALF_HEIGHT * GuiApp::sc) &&
                    (mousePositionY <= objectIconPosition[3 * i + 1] + HIERARCHY_ICON_HALF_HEIGHT * GuiApp::sc))
                    return (objectIconPosition[3 * i + 2]);
            }
        }
    }
    else
    {
        for (int i = 0; i < int(objectIconPosition.size()) / 3; i++)
        {
            if ((mousePositionY >= objectIconPosition[3 * i + 1] - HIERARCHY_HALF_INTER_LINE_SPACE * GuiApp::sc) &&
                (mousePositionY <= objectIconPosition[3 * i + 1] + HIERARCHY_HALF_INTER_LINE_SPACE * GuiApp::sc))
                return (objectIconPosition[3 * i + 2]);
        }
    }
    return (-9999); // minus number are for worlds (also in different instances)
}

int CHierarchy::getActionModelID_icon(int mousePositionX, int mousePositionY)
{
    for (int i = 0; i < int(modelIconPosition.size()) / 3; i++)
    {
        if ((mousePositionX >= modelIconPosition[3 * i + 0] - HIERARCHY_ICON_QUARTER_WIDTH * GuiApp::sc) &&
            (mousePositionX <= modelIconPosition[3 * i + 0] + HIERARCHY_ICON_QUARTER_WIDTH * GuiApp::sc))
        {
            if ((mousePositionY >= modelIconPosition[3 * i + 1] - HIERARCHY_ICON_QUARTER_HEIGHT * GuiApp::sc) &&
                (mousePositionY <= modelIconPosition[3 * i + 1] + HIERARCHY_ICON_QUARTER_HEIGHT * GuiApp::sc))
                return (modelIconPosition[3 * i + 2]);
        }
    }
    return (-9999); // minus number are for worlds (also in different instances)
}

int CHierarchy::getSimulationActionObjectID(int mousePositionX, int mousePositionY)
{
    for (int i = 0; i < int(simulationIconPosition.size()) / 3; i++)
    {
        if ((mousePositionX >= simulationIconPosition[3 * i + 0] - HIERARCHY_ICON_HALF_WIDTH * GuiApp::sc) &&
            (mousePositionX <= simulationIconPosition[3 * i + 0] + HIERARCHY_ICON_HALF_WIDTH * GuiApp::sc))
        {
            if ((mousePositionY >= simulationIconPosition[3 * i + 1] - HIERARCHY_ICON_HALF_HEIGHT * GuiApp::sc) &&
                (mousePositionY <= simulationIconPosition[3 * i + 1] + HIERARCHY_ICON_HALF_HEIGHT * GuiApp::sc))
                return (simulationIconPosition[3 * i + 2]);
        }
    }
    return (-1);
}

int CHierarchy::getActionObjectID(int mousePositionY, int tolerance)
{
    for (int i = 0; i < int(objectPosition.size()) / 2; i++)
    {
        if ((mousePositionY <= objectPosition[2 * i + 0] + tolerance) &&
            (mousePositionY >= objectPosition[2 * i + 0] - tolerance))
            return (objectPosition[2 * i + 1]);
    }
    return (-9999);
}

void CHierarchy::getActionObjectIDs(int mouseDownPositionY, int mouseUpPositionY, int tolerance,
                                    std::vector<int> *toBeSelected)
{
    int minV = mouseUpPositionY - tolerance;
    int maxV = mouseDownPositionY + tolerance;
    if (mouseDownPositionY < mouseUpPositionY)
    {
        minV = mouseDownPositionY - tolerance;
        maxV = mouseUpPositionY + tolerance;
    }
    for (int i = 0; i < int(objectPosition.size()) / 2; i++)
    {
        if ((maxV >= objectPosition[2 * i + 0]) && (minV <= objectPosition[2 * i + 0]))
            toBeSelected->push_back(objectPosition[2 * i + 1]);
    }
}

std::string CHierarchy::getEditedLabel()
{
    return (editionText);
}

void CHierarchy::setEditionLabel(std::string txt)
{
    editionText = txt;
}

void CHierarchy::drawEditionLabel(int textPosX, int textPosY)
{
    int buttonWidth = 20 + ogl::getTextLengthInPixels(editionText.c_str());
    VPoint p(textPosX - 2 + buttonWidth / 2, textPosY + HIERARCHY_TEXT_CENTER_OFFSET * GuiApp::sc);
    VPoint s(buttonWidth, HIERARCHY_INTER_LINE_SPACE * GuiApp::sc);
    float txtCol[3] = {0.0, 0.0, 0.0};
    float backCol[3] = {1.0, 1.0, 0.0};
    int buttonAttrib = sim_buttonproperty_editbox | sim_buttonproperty_enabled | sim_buttonproperty_verticallycentered;
    ogl::drawButton(p, s, txtCol, backCol, backCol, editionText, buttonAttrib, true, editionTextEditPos, 0.0, false,
                    (int)VDateTime::getTimeInMs(), nullptr, nullptr, nullptr, nullptr, nullptr);
    refreshViewFlag = App::userSettings->hierarchyRefreshCnt;
}

void CHierarchy::addMenu(VMenu *menu)
{
    int selCnt = App::currentWorld->sceneObjects->getSelectionCount();
    bool selection = selCnt > 0;
    menu->appendMenuItem(true, false, EXPAND_HIERARCHY_CMD, "Expand all");
    menu->appendMenuItem(true, false, COLLAPSE_HIERARCHY_CMD, "Collapse all");
    menu->appendMenuItem(selection, false, EXPAND_SELECTED_HIERARCHY_CMD, IDS_EXPAND_SELECTED_TREE_MENU_ITEM);
    menu->appendMenuItem(selection, false, COLLAPSE_SELECTED_HIERARCHY_CMD, IDS_COLLAPSE_SELECTED_TREE_MENU_ITEM);
    menu->appendMenuSeparator();
    menu->appendMenuItem(selCnt == 1, false, MOVE_UP_HIERARCHY_CMD, "Move selected object up");
    menu->appendMenuItem(selCnt == 1, false, MOVE_DOWN_HIERARCHY_CMD, "Move selected object down");
    menu->appendMenuSeparator();

    if ((selCnt == 1) &&
        (App::userSettings->externalScriptEditor.size() > 0))
    {
        int h = App::currentWorld->sceneObjects->getLastSelectionHandle();
        CScriptObject *childS =
            App::currentWorld->sceneObjects->embeddedScriptContainer->getScriptFromObjectAttachedTo(sim_scripttype_childscript, h);
        CScriptObject *custS = App::currentWorld->sceneObjects->embeddedScriptContainer->getScriptFromObjectAttachedTo(
            sim_scripttype_customizationscript, h);
        if ((childS != nullptr) || (custS != nullptr))
        {
            menu->appendMenuSeparator();
            if (childS != nullptr)
                menu->appendMenuItem(App::currentWorld->simulation->getSimulationState() != sim_simulation_stopped,
                                     false, RESTART_CHILD_SCRIPT_CMD, "Restart child script");
            if (custS != nullptr)
                menu->appendMenuItem(true, false, RESTART_CUSTOMIZATION_SCRIPT_CMD, "Restart customization script");
            menu->appendMenuSeparator();
        }
    }
}

bool CHierarchy::processCommand(int commandID)
{ // Return value is true if the command belonged to Add menu and was executed
    if ((commandID == EXPAND_HIERARCHY_CMD) || (commandID == COLLAPSE_HIERARCHY_CMD))
    {
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            for (size_t i = 0; i < App::currentWorld->sceneObjects->getObjectCount(); i++)
            {
                CSceneObject *it = App::currentWorld->sceneObjects->getObjectFromIndex(i);
                it->setObjectProperty(it->getObjectProperty() | sim_objectproperty_collapsed);
                if (commandID == EXPAND_HIERARCHY_CMD)
                    it->setObjectProperty(it->getObjectProperty() - sim_objectproperty_collapsed);
            }
            if (commandID == EXPAND_HIERARCHY_CMD)
                App::logMsg(sim_verbosity_msgs, "Hierarchy expanded.");
            else
                App::logMsg(sim_verbosity_msgs, "Hierarchy collapsed.");
            App::undoRedo_sceneChanged("");
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId = commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return (true);
    }
    if ((commandID == MOVE_UP_HIERARCHY_CMD) || (commandID == MOVE_DOWN_HIERARCHY_CMD))
    {
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            CSceneObject* it = App::currentWorld->sceneObjects->getLastSelectionObject();

            int order = App::currentWorld->sceneObjects->getObjectSequence(it);
            if (commandID == MOVE_UP_HIERARCHY_CMD)
                order--;
            else
                order++;
            if ((order >= 0) && (App::currentWorld->sceneObjects->setObjectSequence(it, order)))
            {
                if (commandID == MOVE_UP_HIERARCHY_CMD)
                    App::logMsg(sim_verbosity_msgs, "Object moved up in scene hierarchy.");
                else
                    App::logMsg(sim_verbosity_msgs, "Object moved down in scene hierarchy.");
                App::undoRedo_sceneChanged("");
            }
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId = commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return (true);
    }


    if ((commandID == EXPAND_SELECTED_HIERARCHY_CMD) || (commandID == COLLAPSE_SELECTED_HIERARCHY_CMD))
    {
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            for (size_t i = 0; i < App::currentWorld->sceneObjects->getSelectionCount(); i++)
            {
                CSceneObject *it = App::currentWorld->sceneObjects->getObjectFromHandle(
                    App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
                std::vector<CSceneObject *> toExplore;
                toExplore.push_back(it);
                while (toExplore.size() != 0)
                {
                    it = toExplore[0];
                    it->setObjectProperty(it->getObjectProperty() | sim_objectproperty_collapsed);
                    if (commandID == EXPAND_SELECTED_HIERARCHY_CMD)
                        it->setObjectProperty(it->getObjectProperty() - sim_objectproperty_collapsed);
                    toExplore.erase(toExplore.begin());
                    for (size_t j = 0; j < it->getChildCount(); j++)
                        toExplore.push_back(it->getChildFromIndex(j));
                }
            }
            if (commandID == EXPAND_HIERARCHY_CMD)
                App::logMsg(sim_verbosity_msgs, "Hierarchy tree(s) expanded.");
            else
                App::logMsg(sim_verbosity_msgs, "Hierarchy tree(s) collapsed.");
            App::undoRedo_sceneChanged("");
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId = commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return (true);
    }
    if ((commandID >= HIERARCHY_COLORING_NONE_CMD) && (commandID <= HIERARCHY_COLORING_BLUE_CMD))
    {
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            for (size_t i = 0; i < App::currentWorld->sceneObjects->getSelectionCount(); i++)
            {
                CSceneObject *it = App::currentWorld->sceneObjects->getObjectFromHandle(
                    App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
                it->setHierarchyColorIndex(commandID - HIERARCHY_COLORING_NONE_CMD - 1);
            }
            App::undoRedo_sceneChanged("");
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId = commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return (true);
    }

    if ((commandID == RESTART_CHILD_SCRIPT_CMD) || (commandID == RESTART_CUSTOMIZATION_SCRIPT_CMD))
    {
        SSimulationThreadCommand cmd;
        cmd.cmdId = RESTART_SCRIPT_CMD;
        int h = App::currentWorld->sceneObjects->getLastSelectionHandle();
        CScriptObject *s = nullptr;
        if (commandID == RESTART_CHILD_SCRIPT_CMD)
            s = App::currentWorld->sceneObjects->embeddedScriptContainer->getScriptFromObjectAttachedTo(sim_scripttype_childscript,
                                                                                          h);
        else
            s = App::currentWorld->sceneObjects->embeddedScriptContainer->getScriptFromObjectAttachedTo(
                sim_scripttype_customizationscript, h);
        cmd.intParams.push_back(s->getScriptHandle());
        App::appendSimulationThreadCommand(cmd);
        return (true);
    }

    return (false);
}

void CHierarchy::_drawLinesLinkingDummies(int maxRenderedPos[2])
{
    std::vector<int>
        positions; // contains only objects that have a dummy linking to another, as child (or the dummy itself)
    for (size_t i = 0; i < App::currentWorld->sceneObjects->getObjectCount(sim_object_dummy_type); i++)
    {
        CDummy *dummy = App::currentWorld->sceneObjects->getDummyFromIndex(i);
        if ((dummy->getLinkedDummyHandle() != -1) && (dummy->getDummyType() != sim_dummytype_default) &&
            (dummy->getDummyType() != sim_dummytype_assembly))
        {
            int dummyID = dummy->getObjectHandle();
            int linkedDummyID = dummy->getLinkedDummyHandle();
            CSceneObject *obj = dummy;
            bool found = false;
            while ((!found) && (obj != nullptr))
            {
                int idToSearch = obj->getObjectHandle();
                for (size_t j = 0; j < lineLastPosition.size() / 3; j++)
                {
                    if (idToSearch == lineLastPosition[3 * j + 2])
                    { // we found a pos
                        positions.push_back(lineLastPosition[3 * j + 0]);
                        positions.push_back(lineLastPosition[3 * j + 1]);
                        positions.push_back(dummyID);
                        positions.push_back(linkedDummyID);
                        positions.push_back(int(j)); // index in the lineLastPosition list
                        int wv = 0;
                        if (dummyID == idToSearch)
                            wv |= 1; // the dummy is visible (otherwise it is not visible (built on a collapsed item))
                        if (App::currentWorld->sceneObjects->isObjectSelected(dummyID) ||
                            App::currentWorld->sceneObjects->isObjectSelected(linkedDummyID))
                            wv |= 2; // one of the dummies is selected
                        positions.push_back(wv);
                        positions.push_back(dummy->getDummyType());
                        found = true;
                        break;
                    }
                }
                if (!found)
                    obj = obj->getParent();
            }
        }
    }
    glLineStipple(1, 0x5555);
    glEnable(GL_LINE_STIPPLE);
    std::vector<int> linesPresent; // minY, maxY, X
    const int segmentOffset = 5;
    const int segmentWidth = 10;
    for (int i = 0; i < int(positions.size()) / 7; i++)
    {
        if (positions[7 * i + 5] != -1)
        { // not yet processed
            int firstIndex = positions[7 * i + 4];
            for (int j = i + 1; j < int(positions.size()) / 7; j++)
            {
                int secondDummyID = positions[7 * j + 2];
                if (positions[7 * j + 5] != -1)
                { // not yet processed
                    if (secondDummyID == positions[7 * i + 3])
                    { // this is the one!
                        int secondIndex = positions[7 * j + 4];
                        if (secondIndex != firstIndex)
                        { // make sure the two dummies are not linking to the same line!
                            int maxX = std::max<int>(positions[7 * i + 0], positions[7 * j + 0]);
                            int overallOffset = 0;
                            // a. we don't want to intersect any text/icon in-between. We search for the overallOffset
                            // needed:
                            int low = std::min<int>(firstIndex, secondIndex);
                            int high = std::max<int>(firstIndex, secondIndex);
                            int maxOtherX = 0;
                            for (int k = low + 1; k < high; k++)
                            {
                                if (lineLastPosition[3 * k + 0] > maxOtherX)
                                    maxOtherX = lineLastPosition[3 * k + 0];
                            }
                            if (maxOtherX > maxX)
                                overallOffset += maxOtherX - maxX;
                            // b. we don't want to overlap other dummy-dummy link lines
                            // i.e. maxX+overallOffset should no lie within other lines maxX+overallOffset
                            int minY = std::min<int>(positions[7 * i + 1], positions[7 * j + 1]);
                            int maxY = std::max<int>(positions[7 * i + 1], positions[7 * j + 1]);
                            for (int k = 0; k < int(linesPresent.size()) / 3; k++)
                            {
                                if ((maxY >= linesPresent[3 * k + 0]) && (minY <= linesPresent[3 * k + 1]))
                                {
                                    bool restart = false;
                                    while (abs(maxX + overallOffset - linesPresent[3 * k + 2]) < segmentWidth + 4)
                                    {
                                        overallOffset += 2;
                                        restart = true;
                                    }
                                    if (restart)
                                        k = -1;
                                }
                            }

                            if (positions[7 * i + 5] & 2)
                            {
                                glLineWidth(3.0);
                                glDisable(GL_LINE_STIPPLE);
                            }
                            if ((positions[7 * i + 6] == sim_dummytype_dynloopclosure) ||
                                (positions[7 * i + 6] == sim_dummy_linktype_dynamics_force_constraint) ||
                                (positions[7 * i + 6] == sim_dummytype_dyntendon))
                                ogl::setMaterialColor(sim_colorcomponent_emission, ogl::colorBlue);
                            if ((positions[7 * i + 6] == sim_dummy_linktype_gcs_loop_closure) ||
                                (positions[7 * i + 6] == sim_dummy_linktype_gcs_tip) ||
                                (positions[7 * i + 6] == sim_dummy_linktype_gcs_target))
                                ogl::setMaterialColor(sim_colorcomponent_emission, 0.0f, 0.6f, 0.0f);
                            if (positions[7 * i + 6] == sim_dummy_linktype_ik_tip_target)
                                ogl::setMaterialColor(sim_colorcomponent_emission, ogl::colorRed);
                            ogl::drawSingle2dLine_i(maxX + segmentOffset + overallOffset, positions[7 * i + 1],
                                                    maxX + segmentOffset + segmentWidth + overallOffset,
                                                    positions[7 * i + 1]);
                            if (positions[7 * i + 5] & 1)
                            {
                                glDisable(GL_LINE_STIPPLE);
                                ogl::drawSingle2dLine_i(maxX + segmentOffset + overallOffset, positions[7 * i + 1],
                                                        maxX + segmentOffset + 4 + overallOffset,
                                                        positions[7 * i + 1] + 4);
                                ogl::drawSingle2dLine_i(maxX + segmentOffset + overallOffset, positions[7 * i + 1],
                                                        maxX + segmentOffset + 4 + overallOffset,
                                                        positions[7 * i + 1] - 4);
                                if ((positions[7 * i + 5] & 2) == 0)
                                    glEnable(GL_LINE_STIPPLE);
                            }
                            ogl::drawSingle2dLine_i(maxX + segmentOffset + overallOffset, positions[7 * j + 1],
                                                    maxX + segmentOffset + segmentWidth + overallOffset,
                                                    positions[7 * j + 1]);
                            if (positions[7 * j + 5] & 1)
                            {
                                glDisable(GL_LINE_STIPPLE);
                                ogl::drawSingle2dLine_i(maxX + segmentOffset + overallOffset, positions[7 * j + 1],
                                                        maxX + segmentOffset + 4 + overallOffset,
                                                        positions[7 * j + 1] + 4);
                                ogl::drawSingle2dLine_i(maxX + segmentOffset + overallOffset, positions[7 * j + 1],
                                                        maxX + segmentOffset + 4 + overallOffset,
                                                        positions[7 * j + 1] - 4);
                                if ((positions[7 * i + 5] & 2) == 0)
                                    glEnable(GL_LINE_STIPPLE);
                            }
                            ogl::drawSingle2dLine_i(
                                maxX + segmentOffset + segmentWidth + overallOffset, positions[7 * i + 1],
                                maxX + segmentOffset + segmentWidth + overallOffset, positions[7 * j + 1]);
                            glLineWidth(1.0);
                            glEnable(GL_LINE_STIPPLE);
                            linesPresent.push_back(minY);
                            linesPresent.push_back(maxY);
                            linesPresent.push_back(maxX + overallOffset);
                            if (maxRenderedPos[0] < maxX + overallOffset + segmentOffset + segmentWidth)
                                maxRenderedPos[0] = maxX + overallOffset + segmentOffset + segmentWidth;
                        }
                        positions[7 * j + 5] = -1; // do not process this position anymore
                        break;
                    }
                }
            }
        }
    }
    glDisable(GL_LINE_STIPPLE);
}
