
// This file requires some serious refactoring!!

#include "vrepMainHeader.h"
#include "funcDebug.h"
#include "v_rep_internal.h"
#include "hierarchy.h"
#include "oGL.h"
#include "imgLoaderSaver.h"
#include "sceneObjectOperations.h"
#include "vMessageBox.h"
#include "addOperations.h"
#include "fileOperations.h"
#include "tt.h"
#include "pluginContainer.h"
#include "v_repStrings.h"
#include "app.h"
#include "vVarious.h"
#include "vDateTime.h"
#include "miscBase.h"

const int SAFETY_BORDER_SIZE=20;
const int CONST_VAL_40=40;
const int CONST_VAL_8=8;
const int CONST_VAL_6=6;
const int CONST_VAL_4=4;
//const int SAFETY_BORDER_SIZE=40;

CHierarchy::CHierarchy()
{
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
    for (int i=0;i<int(rootElements.size());i++)
        delete rootElements[i];
    rootElements.clear();
    refreshViewFlag=App::userSettings->hierarchyRefreshCnt;
    rebuildHierarchyFlag=true;
    resetViewFlag=true;
}

void CHierarchy::setUpDefaultValues()
{
    viewPosition[0]=0;
    viewPosition[1]=0;
    refreshViewFlag=App::userSettings->hierarchyRefreshCnt;
    rebuildHierarchyFlag=true;
    resetViewFlag=true;
    shiftingAllowed=false;
    shiftSelectionStarted=false;
    labelEditObjectID=-1;
    editionTextEditPos=-1;
    horizontalScrollbarHeight=0;
    verticalScrollbarWidth=0;
    sliderMoveMode=0;
    _caughtElements=0;
    _mouseDownDragObjectID=-1;
    _worldSelectID_down=-9999;
    _worldSelectID_moving=-9999;
}

int CHierarchy::getCaughtElements()
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    return(_caughtElements);
}

void CHierarchy::clearCaughtElements(int keepMask)
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    _caughtElements&=keepMask;
}

void CHierarchy::setRenderingSizeAndPosition(int xSize,int ySize,int xPos,int yPos)
{
    renderingSize[0]=xSize+SAFETY_BORDER_SIZE*App::sc;
    renderingSize[1]=ySize+SAFETY_BORDER_SIZE*App::sc;
    renderingPosition[0]=xPos-SAFETY_BORDER_SIZE*App::sc;
    renderingPosition[1]=yPos-SAFETY_BORDER_SIZE*App::sc;
    refreshViewFlag=App::userSettings->hierarchyRefreshCnt; // Important, even if the size and position didn't change!
}
void CHierarchy::rebuildHierarchy()
{
    for (size_t i=0;i<rootElements.size();i++)
        delete rootElements[i];
    rootElements.clear();

    if (App::getEditModeType()==NO_EDIT_MODE)
    {
        /*
        std::vector<std::string> sceneNames;
        App::ct->getAllSceneNames(sceneNames);
        for (int i=0;i<int(sceneNames.size());i++)
        {
            if (i!=App::ct->getCurrentInstanceIndex())
            {
                CHierarchyElement* el=new CHierarchyElement(-i-1);
                el->setSceneName(sceneNames[i]);
                rootElements.push_back(el);
            }
        }
        */
        CHierarchyElement* newEl=new CHierarchyElement(-App::ct->getCurrentInstanceIndex()-1);
        newEl->addYourChildren();
        std::string sceneName=App::ct->mainSettings->getSceneName();
        newEl->setSceneName(sceneName);
        rootElements.push_back(newEl);
    }
    if (App::getEditModeType()&VERTEX_EDIT_MODE)
    {
        for (int i=0;i<App::mainWindow->editModeContainer->getShapeEditMode()->getEditionVerticesSize()/3;i++)
            rootElements.push_back(new CHierarchyElement(i));
    }
    if (App::getEditModeType()&TRIANGLE_EDIT_MODE)
    {
        for (int i=0;i<App::mainWindow->editModeContainer->getShapeEditMode()->getEditionIndicesSize()/3;i++)
            rootElements.push_back(new CHierarchyElement(i));
    }
    if (App::getEditModeType()&EDGE_EDIT_MODE)
    {
        for (int i=0;i<App::mainWindow->editModeContainer->getShapeEditMode()->getEditionEdgesSize()/2;i++)
            rootElements.push_back(new CHierarchyElement(i));
    }
    if (App::getEditModeType()&PATH_EDIT_MODE)
    {
        for (int i=0;i<int(App::mainWindow->editModeContainer->getEditModePathContainer()->getSimplePathPointCount());i++)
            rootElements.push_back(new CHierarchyElement(i));
    }
    if (App::getEditModeType()&BUTTON_EDIT_MODE)
    {
        for (int i=0;i<int(App::ct->buttonBlockContainer->allBlocks.size());i++)
        {
            CButtonBlock* it=App::ct->buttonBlockContainer->allBlocks[i];
            if ( ((it->getAttributes()&sim_ui_property_systemblock)==0) )
                rootElements.push_back(new CHierarchyElement(it->getBlockID()));
        }
    }
    if (App::getEditModeType()&MULTISHAPE_EDIT_MODE)
    {
        for (int i=0;i<App::mainWindow->editModeContainer->getMultishapeEditMode()->getMultishapeGeometricComponentsSize();i++)
            rootElements.push_back(new CHierarchyElement(i));
    }
    for (size_t i=0;i<rootElements.size();i++)
        rootElements[i]->computeNumberOfElements();
    rebuildHierarchyFlag=false;
    refreshViewFlag=App::userSettings->hierarchyRefreshCnt;
}

void CHierarchy::looseFocus()
{
    refreshViewFlag=App::userSettings->hierarchyRefreshCnt;
    labelEditObjectID=-1;
}

void CHierarchy::keyPress(int key)
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    if (App::ct->objCont==nullptr)
        return;
    refreshViewFlag=App::userSettings->hierarchyRefreshCnt;

    if (labelEditObjectID==-1)
    { // Normal mode (no label edition)
        if ( (key==UP_KEY)||(key==DOWN_KEY)||(key==LEFT_KEY)||(key==RIGHT_KEY) )
        {
            if (key==UP_KEY)
                viewPosition[1]=viewPosition[1]+HIERARCHY_INTER_LINE_SPACE*App::sc;
            if (key==DOWN_KEY)
                viewPosition[1]=viewPosition[1]-HIERARCHY_INTER_LINE_SPACE*App::sc;
            if (key==LEFT_KEY)
                viewPosition[0]=viewPosition[0]+HIERARCHY_INTER_LINE_SPACE*App::sc;
            if (key==RIGHT_KEY)
                viewPosition[0]=viewPosition[0]-HIERARCHY_INTER_LINE_SPACE*App::sc;
            return;
        }

        if ( (key==CTRL_V_KEY)||(key==ESC_KEY)||(key==DELETE_KEY)||(key==BACKSPACE_KEY)||(key==CTRL_X_KEY)||(key==CTRL_C_KEY)||(key==CTRL_A_KEY)||(key==CTRL_Y_KEY)||(key==CTRL_Z_KEY) )
        {
            if (!App::mainWindow->editModeContainer->keyPress(key))
                CSceneObjectOperations::keyPress(key); // the key press was not for the edit mode
            return;
        }

        if ( (key==CTRL_S_KEY)||(key==CTRL_O_KEY)||(key==CTRL_W_KEY)||(key==CTRL_Q_KEY)||(key==CTRL_N_KEY) )
        {
            CFileOperations::keyPress(key);
            return;
        }

        if (key==CTRL_SPACE_KEY)
        {
            App::ct->simulation->keyPress(key);
            return;
        }

        if (key==CTRL_E_KEY)
        {
            App::ct->keyPress(key);
            return;
        }

        if ( (key==CTRL_D_KEY)||(key==CTRL_G_KEY) )
        {
            App::mainWindow->dlgCont->keyPress(key);
            return;
        }
    }
    else
    { // Label edition mode
        int em=App::getEditModeType();
        if ( (em==NO_EDIT_MODE)||(em==BUTTON_EDIT_MODE) )
        {
            C3DObject* it=App::ct->objCont->getObjectFromHandle(labelEditObjectID);
            CButtonBlock* blk=App::ct->buttonBlockContainer->getBlockWithID(labelEditObjectID);
            if ( (key==ENTER_KEY)||(key==TAB_KEY) )
            {
                tt::removeIllegalCharacters(editionText,true);
                if (editionText!="")
                {
                    if ( (em==NO_EDIT_MODE)&&(it!=nullptr) )
                    {
                        if (App::ct->objCont->getObjectFromName(editionText.c_str())==nullptr)
                        {
                            if ( (VREP_LOWCASE_STRING_COMPARE("world",editionText.c_str())!=0)&&(VREP_LOWCASE_STRING_COMPARE("none",editionText.c_str())!=0) )
                            {
                                App::ct->objCont->renameObject(it->getObjectHandle(),editionText.c_str());
                                POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
                            }
                        }
                        App::setFullDialogRefreshFlag();
                    }
                    if ( (em==BUTTON_EDIT_MODE)&&(blk!=nullptr) )
                    {
                        App::appendSimulationThreadCommand(RENAME_UI_OPENGLUIBLOCKGUITRIGGEREDCMD,blk->getBlockID(),-1,0.0,0.0,editionText.c_str());
                        // App::appendSimulationThreadCommand(POST_SCENE_CHANGED_ANNOUNCEMENT_GUITRIGGEREDCMD);
                        App::appendSimulationThreadCommand(FULLREFRESH_ALL_DIALOGS_GUITRIGGEREDCMD);
                    }
                }
                labelEditObjectID=-1;
            }
            if ( (key==LEFT_KEY)||(key==RIGHT_KEY) )
            {
                if (editionTextEditPos==-1)
                    editionTextEditPos=(int)editionText.length();
                if ( (key==LEFT_KEY)&&(editionTextEditPos>0) )
                    editionTextEditPos--;
                if ( (key==RIGHT_KEY)&&(editionTextEditPos<int(editionText.length())) )
                    editionTextEditPos++;
            }
            if (key==BACKSPACE_KEY)
            {
                if (editionTextEditPos==-1)
                {
                    editionText="";
                    editionTextEditPos=0;
                }
                else
                {
                    if (editionText.length()!=0)
                    {
                        if (editionTextEditPos>0)
                        {
                            editionText.erase(editionText.begin()+editionTextEditPos-1);
                            editionTextEditPos--;
                        }
                    }
                }
            }
            if (key==DELETE_KEY)
            {
                if (editionTextEditPos==-1)
                {
                    editionText="";
                    editionTextEditPos=0;
                }
                else
                {
                    if (editionText.length()!=0)
                    {
                        if (editionTextEditPos<int(editionText.length()))
                            editionText.erase(editionText.begin()+editionTextEditPos);
                    }
                }
            }
            if ( (key>=32)&&(key<123) )
            {
                if (editionTextEditPos==-1)
                {
                    editionTextEditPos=0;
                    editionText="";
                }
                editionText.insert(editionText.begin()+editionTextEditPos,(char)key);
                editionTextEditPos++;
            }
        }
    }
}

bool CHierarchy::render()
{ // return value true means the hierarchy was refreshed
    FUNCTION_DEBUG;
    if (viewPosition[0]<-20000) // From -2000 to -20000 on 3/4/2011 // somehow there is a bug I can't put the finger on right now (2009/12/16)
        viewPosition[0]=0;
    if (rebuildHierarchyFlag)
    {
        rebuildHierarchy();
        if (App::mainWindow->sceneHierarchyWidget->isVisible())
            App::mainWindow->sceneHierarchyWidget->rebuild();
    }
    if (refreshViewFlag)
    {
    if (App::mainWindow->sceneHierarchyWidget->isVisible())
        App::mainWindow->sceneHierarchyWidget->refresh();
    }

    if (resetViewFlag)
    {
        viewPosition[0]=0;
        viewPosition[1]=0;
        resetViewFlag=false;
    }
    if (App::userSettings->hierarchyRefreshCnt>0)
    {
        if (refreshViewFlag<=0)
            return(false);
        refreshViewFlag--;
    }

    int editModeType=App::getEditModeType();
    bool hierarchDragUnderway=false;
    int dx=0;
    int dy=0;
    int dropID=-9999;
    if (editModeType==NO_EDIT_MODE)
    {
        if (_worldSelectID_down!=-9999)
            refreshViewFlag=App::userSettings->hierarchyRefreshCnt;

        if (_mouseDownDragObjectID!=-1)
        {
            if (App::ct->objCont->getLastSelectionID()!=_mouseDownDragObjectID)
                _mouseDownDragObjectID=-1; // we probably pressed esc
            if (_mouseDownDragObjectID!=-1)
            {
                refreshViewFlag=App::userSettings->hierarchyRefreshCnt;
                dx=mouseRelativePosition[0]-mouseDownRelativePosition[0];
                dy=mouseRelativePosition[1]-mouseDownRelativePosition[1];
                hierarchDragUnderway=((abs(dx)>8)||(abs(dy)>8));
                if (hierarchDragUnderway)
                {
                    dropID=objectIDWhereTheMouseCurrentlyIs_minus9999ForNone;
                    if (dropID==-9999)
                        dropID=-App::ct->getCurrentInstanceIndex()-1; // world
                    else
                    {
                        if (dropID<0)
                        {
                            if (dropID!=-App::ct->getCurrentInstanceIndex()-1)
                                dropID=-9999;
                        }
                    }
                }
            }
        }

        static bool lastHierarchDragUnderway=false;
        static int lastDragUnderwayTime=0;
        static int dragStartTime=0;
        if (hierarchDragUnderway)
        {
            int ct=VDateTime::getTimeInMs();
            if (!lastHierarchDragUnderway)
                dragStartTime=ct;
            if (ct-dragStartTime>1000)
            {
                if (ct-lastDragUnderwayTime>50)
                {
                    if (mouseRelativePosition[1]<renderingPosition[1]+(BROWSER_HIERARCHY_TITLE_BAR_HEIGHT+CONST_VAL_40+HIERARCHY_INTER_LINE_SPACE*4)*App::sc) // Bottom border
                        viewPosition[1]=viewPosition[1]+HIERARCHY_INTER_LINE_SPACE*App::sc;
                    if (mouseRelativePosition[1]<renderingPosition[1]+(BROWSER_HIERARCHY_TITLE_BAR_HEIGHT+CONST_VAL_40+HIERARCHY_INTER_LINE_SPACE*2)*App::sc) // Bottom border
                        viewPosition[1]=viewPosition[1]+HIERARCHY_INTER_LINE_SPACE*App::sc;
                    if (mouseRelativePosition[1]>renderingPosition[1]+BROWSER_HIERARCHY_TITLE_BAR_HEIGHT*App::sc+renderingSize[1]-HIERARCHY_INTER_LINE_SPACE*4*App::sc) // top border
                        viewPosition[1]=viewPosition[1]-HIERARCHY_INTER_LINE_SPACE*App::sc;
                    if (mouseRelativePosition[1]>renderingPosition[1]+BROWSER_HIERARCHY_TITLE_BAR_HEIGHT*App::sc+renderingSize[1]-HIERARCHY_INTER_LINE_SPACE*2*App::sc) // top border
                        viewPosition[1]=viewPosition[1]-HIERARCHY_INTER_LINE_SPACE*App::sc;

                    if (mouseRelativePosition[0]<HIERARCHY_INTER_LINE_SPACE*2*App::sc) // left border
                        viewPosition[0]=viewPosition[0]+HIERARCHY_INTER_LINE_SPACE*App::sc;
                    if (mouseRelativePosition[0]>renderingSize[0]-HIERARCHY_INTER_LINE_SPACE*2*App::sc) // right border
                        viewPosition[0]=viewPosition[0]-HIERARCHY_INTER_LINE_SPACE*App::sc;
                    objectIDWhereTheMouseCurrentlyIs_minus9999ForNone=getActionObjectID_icon(0,mouseRelativePosition[1],true);
                    lastDragUnderwayTime=ct;
                }
            }
        }
        lastHierarchDragUnderway=hierarchDragUnderway;

    }

    // We draw a black background so that the separation between the hierarchy and scene is rendered correctly on ALL graphic cards:
    glEnable(GL_SCISSOR_TEST);
    glDisable(GL_DEPTH_TEST);
    glScissor(renderingPosition[0]+SAFETY_BORDER_SIZE*App::sc,renderingPosition[1]+BROWSER_HIERARCHY_TITLE_BAR_HEIGHT*App::sc,renderingSize[0]-SAFETY_BORDER_SIZE*App::sc,renderingSize[1]);
    glClearColor(ogl::SEPARATION_LINE_COLOR[0],ogl::SEPARATION_LINE_COLOR[1],ogl::SEPARATION_LINE_COLOR[2],1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    ogl::setMaterialColor(ogl::colorBlack,ogl::colorBlack,ogl::colorBlack);
    glEnable(GL_SCISSOR_TEST);  
    glViewport(renderingPosition[0],renderingPosition[1],renderingSize[0],renderingSize[1]);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0f,renderingSize[0],0.0f,renderingSize[1],-1.0f,1.0f);
    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity ();
    glDisable(GL_DEPTH_TEST);
    glScissor(renderingPosition[0]+SAFETY_BORDER_SIZE*App::sc,renderingPosition[1],renderingSize[0]-SAFETY_BORDER_SIZE*App::sc,renderingSize[1]);
    glClearColor(1.0f,1.0f,1.0f,1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // There are 2 passes: one where we don't display anything but retrieve information
    // about min and max positions:
    validateViewPosition();
    int textPos[2]={viewPosition[0],viewPosition[1]+renderingSize[1]};
    minRenderedPosition[0]=100000;
    minRenderedPosition[1]=100000;
    maxRenderedPosition[0]=-100000;
    maxRenderedPosition[1]=-100000;
    bool bright=false;

    inflateIconPosition.clear();
    scriptIconPosition.clear();
    scriptParametersIconPosition.clear();
    objectPosition.clear();
    objectIconPosition.clear();
    modelIconPosition.clear();
    textPosition.clear();
    lineLastPosition.clear();
    simulationIconPosition.clear();
    if (editModeType==NO_EDIT_MODE)
    {
        for (int i=0;i<int(rootElements.size());i++)
        {
            std::vector<int> vertLines;
            rootElements[i]->renderElement_3DObject(this,labelEditObjectID,bright,true,renderingSize,
                textPos,0,&vertLines,minRenderedPosition,maxRenderedPosition);
        }
        _drawLinesLinkingDummies(maxRenderedPosition);
    }
    std::vector<char> editModeSelectionStateList;
    if ((editModeType&SHAPE_EDIT_MODE)||(editModeType&PATH_EDIT_MODE))
    {
        if (editModeType&VERTEX_EDIT_MODE)
            editModeSelectionStateList.resize(App::mainWindow->editModeContainer->getShapeEditMode()->getEditionVerticesSize()/3,0);
        if (editModeType&TRIANGLE_EDIT_MODE)
            editModeSelectionStateList.resize(App::mainWindow->editModeContainer->getShapeEditMode()->getEditionIndicesSize()/3,0);
        if (editModeType&EDGE_EDIT_MODE)
            editModeSelectionStateList.resize(App::mainWindow->editModeContainer->getShapeEditMode()->getEditionEdgesSize()/2,0);
        if (editModeType&PATH_EDIT_MODE)
            editModeSelectionStateList.resize(App::mainWindow->editModeContainer->getEditModePathContainer()->getSimplePathPointCount(),0);
        for (int i=0;i<App::mainWindow->editModeContainer->getEditModeBufferSize();i++)
        {
            int ind=App::mainWindow->editModeContainer->getEditModeBufferValue(i);
            if (i==App::mainWindow->editModeContainer->getEditModeBufferSize()-1)
                editModeSelectionStateList[ind]=2; // last selection
            else
                editModeSelectionStateList[ind]=1;
        }
        for (int i=0;i<int(rootElements.size());i++)
        {
            rootElements[i]->renderElement_editModeList(this,labelEditObjectID,bright,true,renderingSize,
                textPos,0,minRenderedPosition,maxRenderedPosition,editModeSelectionStateList[i],editModeType);
        }
    }
    if (editModeType&BUTTON_EDIT_MODE)
    {
        for (int i=0;i<int(rootElements.size());i++)
        {
            rootElements[i]->renderElement_editModeList(this,labelEditObjectID,bright,true,renderingSize,
                textPos,0,minRenderedPosition,maxRenderedPosition,0,editModeType);
        }
    }
    if (editModeType&MULTISHAPE_EDIT_MODE)
    {
        for (int i=0;i<int(rootElements.size());i++)
        {
            rootElements[i]->renderElement_editModeList(this,-1,bright,true,renderingSize,
                textPos,0,minRenderedPosition,maxRenderedPosition,0,editModeType);
        }
    }
    maxRenderedPosition[0]+=8*App::sc;
    maxRenderedPosition[0]=maxRenderedPosition[0]-viewPosition[0];
    maxRenderedPosition[1]=maxRenderedPosition[1]-viewPosition[1];
    minRenderedPosition[0]=minRenderedPosition[0]-viewPosition[0];
    minRenderedPosition[1]=minRenderedPosition[1]-viewPosition[1];

    // Second pass where we display:    
    validateViewPosition();

    textPos[0]=viewPosition[0];
    textPos[1]=viewPosition[1]+renderingSize[1];
    minRenderedPosition[0]=100000;
    minRenderedPosition[1]=100000;
    maxRenderedPosition[0]=-100000;
    maxRenderedPosition[1]=-100000;
    bright=false;
    inflateIconPosition.clear();
    scriptIconPosition.clear();
    scriptParametersIconPosition.clear();
    objectPosition.clear();
    objectIconPosition.clear();
    modelIconPosition.clear();
    textPosition.clear();
    lineLastPosition.clear();
    simulationIconPosition.clear();
    if (editModeType==NO_EDIT_MODE)
    {

        std::vector<CHierarchyElement*> el;
        std::vector<int> elNb;
        for (int i=0;i<int(rootElements.size());i++)
        { // This is needed to order the elements from most sub-elements to least sub-elements
            int els=rootElements[i]->getNumberOfElements();
            int j;
            for (j=0;j<int(elNb.size());j++)
            {
                if (els>elNb[j])
                    break;
            }
            elNb.insert(elNb.begin()+j,els);
            el.insert(el.begin()+j,rootElements[i]);
        }   

        int objFromHalf=-1;
        if (hierarchDragUnderway)
            objFromHalf=_mouseDownDragObjectID;

        int worldClickThing=-9999;
        if (_worldSelectID_down==_worldSelectID_moving)
            worldClickThing=_worldSelectID_down;

        for (int i=0;i<int(el.size());i++)
        {
            std::vector<int> vertLines;
            el[i]->renderElement_3DObject(this,labelEditObjectID,bright,false,renderingSize,
                textPos,0,&vertLines,minRenderedPosition,maxRenderedPosition,false,objFromHalf,dropID,worldClickThing);
        }
        while (CHierarchyElement::renderDummyElement(bright,renderingSize,textPos));
        _drawLinesLinkingDummies(maxRenderedPosition);

        if (hierarchDragUnderway)
        {
            textPos[0]=dx+_mouseDownDragOffset[0];
            textPos[1]=dy+_mouseDownDragOffset[1];

            CHierarchyElement* it=nullptr;
            for (int i=0;i<int(el.size());i++)
            {
                it=el[i]->getElementLinkedWithObject(_mouseDownDragObjectID);
                if (it!=nullptr)
                    break;
            }
            if (it!=nullptr)
            { // should normally never be nullptr
                std::vector<int> vertLines;
                int renderingSizeCopy[2]={renderingSize[0],renderingSize[1]};
                int minRenderedPositionCopy[2]={minRenderedPosition[0],minRenderedPosition[1]};
                int maxRenderedPositionCopy[2]={maxRenderedPosition[0],maxRenderedPosition[1]};
                it->renderElement_3DObject(this,labelEditObjectID,bright,false,renderingSizeCopy,
                    textPos,0,&vertLines,minRenderedPositionCopy,maxRenderedPositionCopy,true);
            }
        }

    }
    if ((editModeType&SHAPE_EDIT_MODE)||(editModeType&PATH_EDIT_MODE))
    {
        for (int i=0;i<int(rootElements.size());i++)
        {
            rootElements[i]->renderElement_editModeList(this,labelEditObjectID,bright,false,renderingSize,
                textPos,0,minRenderedPosition,maxRenderedPosition,editModeSelectionStateList[i],editModeType);
        }
        while (CHierarchyElement::renderDummyElement(bright,renderingSize,textPos));
    }
    if (editModeType&BUTTON_EDIT_MODE)
    {
        for (int i=0;i<int(rootElements.size());i++)
        {
            rootElements[i]->renderElement_editModeList(this,labelEditObjectID,bright,false,renderingSize,
                textPos,0,minRenderedPosition,maxRenderedPosition,0,editModeType);
        }
        while (CHierarchyElement::renderDummyElement(bright,renderingSize,textPos));
    }
    if (editModeType&MULTISHAPE_EDIT_MODE)
    {
        for (int i=0;i<int(rootElements.size());i++)
        {
            rootElements[i]->renderElement_editModeList(this,-1,bright,false,renderingSize,
                textPos,0,minRenderedPosition,maxRenderedPosition,0,editModeType);
        }
        while (CHierarchyElement::renderDummyElement(bright,renderingSize,textPos));
    }
    maxRenderedPosition[0]+=8*App::sc;
    maxRenderedPosition[0]=maxRenderedPosition[0]-viewPosition[0];
    maxRenderedPosition[1]=maxRenderedPosition[1]-viewPosition[1];
    minRenderedPosition[0]=minRenderedPosition[0]-viewPosition[0];
    minRenderedPosition[1]=minRenderedPosition[1]-viewPosition[1];
    
    // We now draw the selection square:
    if (shiftSelectionStarted)
    {
        int limitedPos[2]={mouseRelativePosition[0],mouseRelativePosition[1]};
        if (limitedPos[0]>renderingSize[0])
            limitedPos[0]=renderingSize[0];
        if (limitedPos[1]>renderingSize[1])
            limitedPos[1]=renderingSize[1];
        if (limitedPos[0]<SAFETY_BORDER_SIZE*App::sc)
            limitedPos[0]=SAFETY_BORDER_SIZE*App::sc;
        if (limitedPos[1]<SAFETY_BORDER_SIZE*App::sc)
            limitedPos[1]=SAFETY_BORDER_SIZE*App::sc;
        ogl::setAlpha(0.2f);
        ogl::setMaterialColor(sim_colorcomponent_emission,ogl::colorYellow);
        ogl::setBlending(true,GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        glBegin(GL_QUADS);
        glVertex3i(mouseDownRelativePosition[0],mouseDownRelativePosition[1],0);
        glVertex3i(mouseDownRelativePosition[0],limitedPos[1],0);
        glVertex3i(limitedPos[0],limitedPos[1],0);
        glVertex3i(limitedPos[0],mouseDownRelativePosition[1],0);
        glEnd();
        ogl::setBlending(false);            
        ogl::setMaterialColor(sim_colorcomponent_emission,ogl::colorBlack);
        glBegin(GL_LINE_STRIP);
        glVertex3i(mouseDownRelativePosition[0],mouseDownRelativePosition[1],0);
        glVertex3i(mouseDownRelativePosition[0],limitedPos[1],0);
        glVertex3i(limitedPos[0],limitedPos[1],0);
        glVertex3i(limitedPos[0],mouseDownRelativePosition[1],0);
        glVertex3i(mouseDownRelativePosition[0],mouseDownRelativePosition[1],0);
        glEnd();
    }
    int vtl[2];
    int vbr[2];
    int htl[2];
    int hbr[2];
    int slidersEnable=getSliderPositions(vtl,vbr,htl,hbr,nullptr);
//  float black[3]={0.0f,0.0f,0.0f};

    if (slidersEnable&1)
    { // here we draw the vertical slider:
        ogl::setMaterialColor(sim_colorcomponent_emission,ogl::HIERARCHY_AND_BROWSER_SCROLLBAR_BACK_COLOR);
        int leftTop[2]={renderingSize[0]-verticalScrollbarWidth,renderingSize[1]};
        int rightBottom[2]={renderingSize[0],0};
        glBegin(GL_QUADS);
        glVertex3i(leftTop[0],leftTop[1],0);
        glVertex3i(rightBottom[0],leftTop[1],0);
        glVertex3i(rightBottom[0],rightBottom[1],0);
        glVertex3i(leftTop[0],rightBottom[1],0);
        glEnd();

        ogl::setMaterialColor(sim_colorcomponent_emission,ogl::HIERARCHY_AND_BROWSER_SCROLLBAR_COLOR);
        glBegin(GL_QUADS);
        glVertex3i(vtl[0],vtl[1],0);
        glVertex3i(vbr[0],vtl[1],0);
        glVertex3i(vbr[0],vbr[1],0);
        glVertex3i(vtl[0],vbr[1],0);
        glEnd();

        ogl::setMaterialColor(sim_colorcomponent_emission,ogl::SEPARATION_LINE_COLOR);
        ogl::drawSingle2dLine_i(leftTop[0],leftTop[1],leftTop[0],rightBottom[1]);
    }
    if (slidersEnable&2)
    { // here we draw the horizontal slider:
        ogl::setMaterialColor(sim_colorcomponent_emission,ogl::HIERARCHY_AND_BROWSER_SCROLLBAR_BACK_COLOR);
        int leftTop[2]={0,horizontalScrollbarHeight+SAFETY_BORDER_SIZE*App::sc};
        int rightBottom[2]={renderingSize[0],0};
        glBegin(GL_QUADS);
        glVertex3i(leftTop[0],leftTop[1],0);
        glVertex3i(rightBottom[0],leftTop[1],0);
        glVertex3i(rightBottom[0],rightBottom[1],0);
        glVertex3i(leftTop[0],rightBottom[1],0);
        glEnd();
        ogl::setMaterialColor(sim_colorcomponent_emission,ogl::HIERARCHY_AND_BROWSER_SCROLLBAR_COLOR);
        glBegin(GL_QUADS);
        glVertex3i(htl[0],htl[1],0);
        glVertex3i(hbr[0],htl[1],0);
        glVertex3i(hbr[0],hbr[1],0);
        glVertex3i(htl[0],hbr[1],0);
        glEnd();

        ogl::setMaterialColor(sim_colorcomponent_emission,ogl::SEPARATION_LINE_COLOR);
        ogl::drawSingle2dLine_i(leftTop[0],leftTop[1],rightBottom[0]-verticalScrollbarWidth,leftTop[1]);
    }
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_SCISSOR_TEST);
    return(true);
}

int CHierarchy::getSliderPositions(int vSliderTopLeft[2],int vSliderBottomRight[2],int hSliderTopLeft[2],int hSliderBottomRight[2],float prop[2])
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    // we check if we need the sliders:
    verticalScrollbarWidth=0;
    horizontalScrollbarHeight=0;
    int minMaxViewPosition[2];
    int minViewPosition[2];
    for (int i=0;i<3;i++)
    { // we need to do it 3 times because one added slider might mean we have to add the other one!
        int savedViewPosition[2]={viewPosition[0],viewPosition[1]};
        viewPosition[0]=-99999999;
        viewPosition[1]=-99999999;
        validateViewPosition();
        minViewPosition[0]=viewPosition[0];
        minViewPosition[1]=viewPosition[1];
        viewPosition[0]=+99999999;
        viewPosition[1]=+99999999;
        validateViewPosition();
        int maxViewPosition[2]={viewPosition[0],viewPosition[1]};
        minMaxViewPosition[0]=maxViewPosition[0]-minViewPosition[0];
        minMaxViewPosition[1]=maxViewPosition[1]-minViewPosition[1];
        viewPosition[0]=savedViewPosition[0];
        viewPosition[1]=savedViewPosition[1];
        if (minMaxViewPosition[0]!=0)
            horizontalScrollbarHeight=HIERARCHY_SCROLLBAR_WIDTH*App::sc;
        if (minMaxViewPosition[1]!=0)
            verticalScrollbarWidth=HIERARCHY_SCROLLBAR_WIDTH*App::sc;
    }
    int retVal=0;
    if (verticalScrollbarWidth>0)
    { // here we compute the vertical slider:
        retVal|=1;
        int effDr=renderingSize[1]-horizontalScrollbarHeight-SAFETY_BORDER_SIZE*App::sc;//-4;
        int sliderS=effDr*effDr/(effDr+minMaxViewPosition[1]);
        if (sliderS<HIERARCHY_SCROLLBAR_WIDTH*App::sc)
            sliderS=HIERARCHY_SCROLLBAR_WIDTH*App::sc;
        int sliderP=int(float(effDr-sliderS)*float(viewPosition[1]-minViewPosition[1])/float(minMaxViewPosition[1]));
        vSliderTopLeft[0]=renderingSize[0]-verticalScrollbarWidth;
        vSliderBottomRight[0]=renderingSize[0];
        vSliderTopLeft[1]=renderingSize[1]-sliderP;
        vSliderBottomRight[1]=vSliderTopLeft[1]-sliderS;
        if (prop!=nullptr)
            prop[0]=float(minMaxViewPosition[1])/float(effDr-sliderS);
    }
    if (horizontalScrollbarHeight>0)
    { // here we compute the horizontal slider:
        retVal|=2;
        int effDr=renderingSize[0]-verticalScrollbarWidth+(-SAFETY_BORDER_SIZE-CONST_VAL_4)*App::sc;
        int sliderS=effDr*effDr/(effDr+minMaxViewPosition[0]);
        if (sliderS<HIERARCHY_SCROLLBAR_WIDTH*App::sc)
            sliderS=HIERARCHY_SCROLLBAR_WIDTH*App::sc;
        int sliderP=int(float(effDr-sliderS)*float(viewPosition[0]-minViewPosition[0])/float(minMaxViewPosition[0]));
        hSliderTopLeft[1]=horizontalScrollbarHeight+SAFETY_BORDER_SIZE*App::sc;
        hSliderBottomRight[1]=0;
        hSliderTopLeft[0]=effDr-sliderS-sliderP+SAFETY_BORDER_SIZE*App::sc;
        hSliderBottomRight[0]=hSliderTopLeft[0]+sliderS;
        if (prop!=nullptr)
            prop[1]=float(minMaxViewPosition[0])/float(effDr-sliderS);
    }
    return(retVal);
}

bool CHierarchy::leftMouseDown(int x,int y,int selectionStatus)
{
    _mouseDownDragObjectID=-1;
    _worldSelectID_down=-9999;
    if (x<0)
        return(false);
    if (y<0)
        return(false);
    if (x>renderingSize[0]-SAFETY_BORDER_SIZE*App::sc)
        return(false);
    if (y>renderingSize[1]-SAFETY_BORDER_SIZE*App::sc)
        return(false);
    // The mouse went down on the hierarchy window!
    _caughtElements|=sim_left_button;

    refreshViewFlag=App::userSettings->hierarchyRefreshCnt;
    mouseDownRelativePosition[0]=x+SAFETY_BORDER_SIZE*App::sc;
    mouseDownRelativePosition[1]=y+SAFETY_BORDER_SIZE*App::sc;
    mouseRelativePosition[0]=x+SAFETY_BORDER_SIZE*App::sc;
    mouseRelativePosition[1]=y+SAFETY_BORDER_SIZE*App::sc;
    previousMouseRelativePosition[0]=x+SAFETY_BORDER_SIZE*App::sc;
    previousMouseRelativePosition[1]=y+SAFETY_BORDER_SIZE*App::sc;
    shiftSelectionStarted=false;
    labelEditObjectID=-1;

    int vtl[2];
    int vbr[2];
    int htl[2];
    int hbr[2];
    int slidersEnable=getSliderPositions(vtl,vbr,htl,hbr,nullptr);
    sliderMoveMode=0;
    if ( (slidersEnable&1)&&(mouseDownRelativePosition[0]>vtl[0])&&(mouseDownRelativePosition[0]<vbr[0]) )
    {
        if ( (mouseDownRelativePosition[1]<vtl[1])&&(mouseDownRelativePosition[1]>vbr[1]) )
        { // Vertical slider action
            sliderMoveMode=1;
            viewPosWhenMouseOnSliderDown[0]=viewPosition[0];
            viewPosWhenMouseOnSliderDown[1]=viewPosition[1];
            return(true);
        }
        else
        { // We clicked the slider, but not the mobile part
            return(true);
        }
    }
    if ( (slidersEnable&2)&&(mouseDownRelativePosition[1]<htl[1])&&(mouseDownRelativePosition[1]>hbr[1]) )
    {
        if ( (mouseDownRelativePosition[0]>htl[0])&&(mouseDownRelativePosition[0]<hbr[0]) )
        { // Horizontal slider action
            sliderMoveMode=2;
            viewPosWhenMouseOnSliderDown[0]=viewPosition[0];
            viewPosWhenMouseOnSliderDown[1]=viewPosition[1];
            return(true);
        }
        else
        { // We clicked the slider, but not the mobile part
            return(true);
        }
    }
    bool clickSelection=false;
    if ( (selectionStatus!=CTRLSELECTION)&&(selectionStatus!=SHIFTSELECTION) )
    {
        clickSelection=true;
    }

    shiftingAllowed=( (selectionStatus!=CTRLSELECTION)&&(selectionStatus!=SHIFTSELECTION) );
    int objID=getInflateActionObjectID(mouseDownRelativePosition[0],mouseDownRelativePosition[1]);
    bool canSelect=true;
    if ((objID>=0)&&(selectionStatus!=CTRLSELECTION)&&(selectionStatus!=SHIFTSELECTION))
    { // Expansion/collapse
        C3DObject* it=App::ct->objCont->getObjectFromHandle(objID);
        if (it!=nullptr)
        {
            SSimulationThreadCommand cmd;
            cmd.cmdId=TOGGLE_EXPAND_COLLAPSE_HIERARCHY_OBJECT_CMD;
            cmd.intParams.push_back(it->getObjectHandle());
            App::appendSimulationThreadCommand(cmd);
            shiftingAllowed=false;
            canSelect=false;
        }
    }
    if ( (App::getEditModeType()==NO_EDIT_MODE)&&canSelect )
    { // NO EDIT MODE
        if ( (selectionStatus==CTRLSELECTION)||clickSelection )
        {
            int objID=getActionObjectID(mouseDownRelativePosition[1],HIERARCHY_HALF_INTER_LINE_SPACE*App::sc);
            if (objID>=0)
            {
                C3DObject* obj=App::ct->objCont->getObjectFromHandle(objID);
                if (obj!=nullptr) // just in case
                {
                    if (selectionStatus==CTRLSELECTION)
                    {
                        App::ct->objCont->xorAddObjectToSelection(objID); // Normal selection/deselection
                    }
                    else
                    {
                        App::ct->objCont->deselectObjects();
                        App::ct->objCont->addObjectToSelection(objID); // Normal selection
#ifndef KEYWORD__NOT_DEFINED_FORMELY_BR
                        int dxv[2];
                        if (getLineObjectID(mouseDownRelativePosition[1],dxv)==objID)
                        {
                            _mouseDownDragObjectID=objID;
                            _mouseDownDragOffset[0]=dxv[0];
                            _mouseDownDragOffset[1]=dxv[1];
                        }
#endif
                    }
                }
            }
            else
            {
                if (objID==-9999)
                    App::ct->objCont->deselectObjects();
                else
                {
                    _worldSelectID_down=objID;
                    _worldSelectID_moving=objID;
                }
            }
        }
        if (selectionStatus==SHIFTSELECTION)
            shiftSelectionStarted=true;
    }
    if ( ((App::getEditModeType()&SHAPE_EDIT_MODE)||(App::getEditModeType()&PATH_EDIT_MODE))&&canSelect )
    { // SHAPE OR PATH EDIT MODE
        if ( (selectionStatus==CTRLSELECTION)||clickSelection )
        {
            int objID=getActionObjectID(mouseDownRelativePosition[1],HIERARCHY_HALF_INTER_LINE_SPACE*App::sc);
            if (objID>=0)
            {
                if (selectionStatus==CTRLSELECTION)
                    App::mainWindow->editModeContainer->xorAddItemToEditModeBuffer(objID,true);
                else
                {
                    App::mainWindow->editModeContainer->deselectEditModeBuffer();
                    App::mainWindow->editModeContainer->addItemToEditModeBuffer(objID,true);
                }
            }
            else
                App::mainWindow->editModeContainer->deselectEditModeBuffer();
        }
        if (selectionStatus==SHIFTSELECTION)
            shiftSelectionStarted=true;
    }
    if ( (App::getEditModeType()&BUTTON_EDIT_MODE)&&canSelect )
    { // BUTTON EDIT MODE
        if ( (selectionStatus==CTRLSELECTION)||clickSelection )
        {
            int objID=getActionObjectID(mouseDownRelativePosition[1],HIERARCHY_HALF_INTER_LINE_SPACE*App::sc);
            App::ct->buttonBlockContainer->setBlockInEdition(objID);
            App::setLightDialogRefreshFlag();
        }
    }
    if ( (App::getEditModeType()&MULTISHAPE_EDIT_MODE)&&canSelect )
    { // MULTISHAPE EDIT MODE
        if ( (selectionStatus==CTRLSELECTION)||clickSelection )
        {
            int objID=getActionObjectID(mouseDownRelativePosition[1],HIERARCHY_HALF_INTER_LINE_SPACE*App::sc);
            App::mainWindow->editModeContainer->getMultishapeEditMode()->setMultishapeGeometricComponentIndex(objID);
            App::setLightDialogRefreshFlag();
        }
    }
    return(true);
}

void CHierarchy::leftMouseUp(int x,int y)
{
    int dx=(x+SAFETY_BORDER_SIZE*App::sc)-mouseDownRelativePosition[0];
    int dy=(y+SAFETY_BORDER_SIZE*App::sc)-mouseDownRelativePosition[1];
    bool hierarchDragUnderway=((abs(dx)>8)||(abs(dy)>8));
    if ((_mouseDownDragObjectID!=-1)&&hierarchDragUnderway)
    {
        if (App::ct->objCont->getLastSelectionID()==_mouseDownDragObjectID)
        {

            if (objectIDWhereTheMouseCurrentlyIs_minus9999ForNone==-9999)
                objectIDWhereTheMouseCurrentlyIs_minus9999ForNone=-App::ct->getCurrentInstanceIndex()-1; // world
            else
            {
                if (objectIDWhereTheMouseCurrentlyIs_minus9999ForNone<0)
                {
                    if (objectIDWhereTheMouseCurrentlyIs_minus9999ForNone!=-App::ct->getCurrentInstanceIndex()-1)
                        objectIDWhereTheMouseCurrentlyIs_minus9999ForNone=-9999;
                }
            }

            if ((objectIDWhereTheMouseCurrentlyIs_minus9999ForNone!=_mouseDownDragObjectID)&&(objectIDWhereTheMouseCurrentlyIs_minus9999ForNone!=-9999))
            {
                SSimulationThreadCommand cmd;
                cmd.cmdId=MAKE_OBJECT_CHILD_OF_CMD;
                cmd.intParams.push_back(_mouseDownDragObjectID);
                cmd.intParams.push_back(objectIDWhereTheMouseCurrentlyIs_minus9999ForNone);
                App::appendSimulationThreadCommand(cmd);
            }
        }
    }

    _mouseDownDragObjectID=-1;
    sliderMoveMode=0;
    refreshViewFlag=App::userSettings->hierarchyRefreshCnt;
    mouseRelativePosition[0]=x+SAFETY_BORDER_SIZE*App::sc;
    mouseRelativePosition[1]=y+SAFETY_BORDER_SIZE*App::sc;


    if (shiftSelectionStarted&&(_caughtElements&sim_left_button))
    { // SHIFT SELECTION
        int limitedPos=mouseRelativePosition[1];
        if (limitedPos>renderingSize[1])
            limitedPos=renderingSize[1];
        if (limitedPos<SAFETY_BORDER_SIZE*App::sc)
            limitedPos=SAFETY_BORDER_SIZE*App::sc;
        std::vector<int> objToBeSelected;
        getActionObjectIDs(mouseDownRelativePosition[1],limitedPos,HIERARCHY_HALF_INTER_LINE_SPACE*App::sc,&objToBeSelected);
        if (App::getEditModeType()==NO_EDIT_MODE)
        {
            for (int i=0;i<int(objToBeSelected.size());i++)
            {
                C3DObject* obj=App::ct->objCont->getObjectFromHandle(objToBeSelected[i]);
                if (obj!=nullptr) // Just in case
                {
                    App::ct->objCont->addObjectToSelection(objToBeSelected[i]); // Normal selection
                }
            }
        }
        if ((App::getEditModeType()&SHAPE_EDIT_MODE)||(App::getEditModeType()&PATH_EDIT_MODE))
        {
            for (int i=0;i<int(objToBeSelected.size());i++)
                App::mainWindow->editModeContainer->addItemToEditModeBuffer(objToBeSelected[i],true);
        }
    }
    _caughtElements&=0xffff-sim_left_button;
    shiftingAllowed=false;
    shiftSelectionStarted=false;

    // We have to do this at the very end of the routine since we are switching instances:
    if ((_worldSelectID_down!=-9999)&&(_worldSelectID_down==getActionObjectID(mouseRelativePosition[1],HIERARCHY_HALF_INTER_LINE_SPACE*App::sc)))
    {
        int nii=(-_worldSelectID_down)-1;
        _worldSelectID_down=-9999;
        App::ct->setInstanceIndexWithThumbnails(nii);
    }

}
bool CHierarchy::rightMouseDown(int x,int y)
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    if (x<0)
        return(false);
    if (y<0)
        return(false);
    if (x>renderingSize[0]-SAFETY_BORDER_SIZE*App::sc)
        return(false);
    if (y>renderingSize[1]-SAFETY_BORDER_SIZE*App::sc)
        return(false);
    // The mouse went down on the hierarchy window
    _caughtElements|=sim_right_button;
    refreshViewFlag=App::userSettings->hierarchyRefreshCnt;
    mouseDownRelativePosition[0]=x+SAFETY_BORDER_SIZE*App::sc;
    mouseDownRelativePosition[1]=y+SAFETY_BORDER_SIZE*App::sc;
    mouseRelativePosition[0]=x+SAFETY_BORDER_SIZE*App::sc;
    mouseRelativePosition[1]=y+SAFETY_BORDER_SIZE*App::sc;
    previousMouseRelativePosition[0]=x+SAFETY_BORDER_SIZE*App::sc;
    previousMouseRelativePosition[1]=y+SAFETY_BORDER_SIZE*App::sc;
    objectIDWhereTheMouseCurrentlyIs_minus9999ForNone=getActionObjectID_icon(0,mouseRelativePosition[1],true);
    labelEditObjectID=-1;
    return(true); // We catch this event to display a popup-menu when the mouse comes up
}
void CHierarchy::rightMouseUp(int x,int y,int absX,int absY,QWidget* mainWindow)
{ // Only caught if right button was caught by the hierarchy!
    _caughtElements&=0xffff-sim_right_button;
    if ( (x<0)||(y<0)||(x>renderingSize[0]-SAFETY_BORDER_SIZE*App::sc)||(y>renderingSize[1]-SAFETY_BORDER_SIZE*App::sc) )
        return;

    // The mouse went up on the hierarchy window
    refreshViewFlag=App::userSettings->hierarchyRefreshCnt;
    mouseRelativePosition[0]=x+SAFETY_BORDER_SIZE*App::sc;
    mouseRelativePosition[1]=y+SAFETY_BORDER_SIZE*App::sc;

    if (App::operationalUIParts&sim_gui_popups)
    { // Default popups
        if (App::getEditModeType()==NO_EDIT_MODE)
        {
            VMenu mainMenu=VMenu();
            addMenu(&mainMenu);

            VMenu* objectEditionMenu=new VMenu();
            CSceneObjectOperations::addMenu(objectEditionMenu);
            mainMenu.appendMenuAndDetach(objectEditionMenu,true,IDS_EDIT_MENU_ITEM);

            VMenu* addMenu=new VMenu();
            CAddOperations::addMenu(addMenu,nullptr,false);
            mainMenu.appendMenuAndDetach(addMenu,true,IDS_ADD_MENU_ITEM);

            int selSize=App::ct->objCont->getSelSize();
            VMenu* hierarchyColoringMenu=new VMenu();
            bool cols[4]={false,false,false,false};
            if (selSize>0)
            {
                for (int i=0;i<selSize;i++)
                {
                    C3DObject* anO=App::ct->objCont->getObjectFromHandle(App::ct->objCont->getSelID(i));
                    int colInd=anO->getHierarchyColorIndex();
                    if (colInd==-1)
                        cols[0]=true;
                    if (colInd==0)
                        cols[1]=true;
                    if (colInd==1)
                        cols[2]=true;
                    if (colInd==2)
                        cols[3]=true;
                }
                hierarchyColoringMenu->appendMenuItem(true,cols[0],HIERARCHY_COLORING_NONE_CMD,IDSN_HIERARCHY_COLORING_NONE_MENU_ITEM,true);
                hierarchyColoringMenu->appendMenuItem(true,cols[1],HIERARCHY_COLORING_RED_CMD,IDSN_HIERARCHY_COLORING_RED_MENU_ITEM,true);
                hierarchyColoringMenu->appendMenuItem(true,cols[2],HIERARCHY_COLORING_GREEN_CMD,IDSN_HIERARCHY_COLORING_GREEN_MENU_ITEM,true);
                hierarchyColoringMenu->appendMenuItem(true,cols[3],HIERARCHY_COLORING_BLUE_CMD,IDSN_HIERARCHY_COLORING_BLUE_MENU_ITEM,true);
            }
            mainMenu.appendMenuAndDetach(hierarchyColoringMenu,selSize>0,IDSN_HIERARCHY_COLORING_MENU_ITEM);


            int command=mainMenu.trackPopupMenu();
            bool processed=false;
            if (!processed)
                processed=processCommand(command);
            if (!processed)
                processed=CSceneObjectOperations::processCommand(command);
            if (!processed)
                processed=CAddOperations::processCommand(command,nullptr);
        } 
        else
        {
            VMenu mainMenu=VMenu();

            if (App::getEditModeType()&SHAPE_EDIT_MODE)
                App::mainWindow->editModeContainer->addMenu(&mainMenu,nullptr);
            if (App::getEditModeType()&PATH_EDIT_MODE)
                App::mainWindow->editModeContainer->addMenu(&mainMenu,nullptr);
            if (App::getEditModeType()&BUTTON_EDIT_MODE)
                App::mainWindow->editModeContainer->addMenu(&mainMenu,nullptr);

            int command=mainMenu.trackPopupMenu();

            if (App::getEditModeType()&SHAPE_EDIT_MODE)
                App::mainWindow->editModeContainer->processCommand(command,nullptr);
            if (App::getEditModeType()&PATH_EDIT_MODE)
                App::mainWindow->editModeContainer->processCommand(command,nullptr);
            if (App::getEditModeType()&BUTTON_EDIT_MODE)
                App::mainWindow->editModeContainer->processCommand(command,nullptr);
        }
    }
}

bool CHierarchy::mouseWheel(int deltaZ,int x,int y)
{
    if ( (x<0)||(y<0)||(x>renderingSize[0]-SAFETY_BORDER_SIZE*App::sc)||(y>renderingSize[1]-SAFETY_BORDER_SIZE*App::sc) )
        return(false);
    refreshViewFlag=App::userSettings->hierarchyRefreshCnt;
    viewPosition[1]=viewPosition[1]-(deltaZ/120)*HIERARCHY_INTER_LINE_SPACE*App::sc;
    return(true);
}

void CHierarchy::mouseMove(int x,int y,bool passiveAndFocused)
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    _worldSelectID_moving=-9999;
    mouseRelativePosition[0]=x+SAFETY_BORDER_SIZE*App::sc;
    mouseRelativePosition[1]=y+SAFETY_BORDER_SIZE*App::sc;
    objectIDWhereTheMouseCurrentlyIs_minus9999ForNone=getActionObjectID_icon(0,mouseRelativePosition[1],true);
    if (!passiveAndFocused)
    {
        int vtl[2];
        int vbr[2];
        int htl[2];
        int hbr[2];
        float prop[2];
        if (sliderMoveMode&1)
        { // we are moving the vertical slider
            getSliderPositions(vtl,vbr,htl,hbr,prop);
            viewPosition[1]=viewPosWhenMouseOnSliderDown[1]-int(prop[0]*float(mouseRelativePosition[1]-mouseDownRelativePosition[1]));
            refreshViewFlag=App::userSettings->hierarchyRefreshCnt;
        } 
        else if (sliderMoveMode&2)
        { // we are moving the horizontal slider
            getSliderPositions(vtl,vbr,htl,hbr,prop);
            viewPosition[0]=viewPosWhenMouseOnSliderDown[0]-int(prop[1]*float(mouseRelativePosition[0]-mouseDownRelativePosition[0]));
            refreshViewFlag=App::userSettings->hierarchyRefreshCnt;
        }
        else if (shiftingAllowed)
        {
            refreshViewFlag=App::userSettings->hierarchyRefreshCnt;
            int objID=getActionObjectID(mouseRelativePosition[1],HIERARCHY_HALF_INTER_LINE_SPACE*App::sc);
            if (objID<0)
                _worldSelectID_moving=objID;
        }
        else if (shiftSelectionStarted)
            refreshViewFlag=App::userSettings->hierarchyRefreshCnt;
    }
    previousMouseRelativePosition[0]=mouseRelativePosition[0];
    previousMouseRelativePosition[1]=mouseRelativePosition[1];
}

bool CHierarchy::leftMouseDblClick(int x,int y,int selectionStatus)
{
    // Following few on 23/9/2013, because somehow with Qt5 there is no mouse up event following:
    _mouseDownDragObjectID=-1;
    _worldSelectID_down=-9999;
    sliderMoveMode=0;
    _caughtElements&=0xffff-sim_left_button;


    if ( (x<0)||(y<0)||(x>renderingSize[0]-SAFETY_BORDER_SIZE*App::sc)||(y>renderingSize[1]-SAFETY_BORDER_SIZE*App::sc) )
        return(false);
    // The mouse went down on the hierarchy window
    refreshViewFlag=App::userSettings->hierarchyRefreshCnt;
    mouseDownRelativePosition[0]=x+SAFETY_BORDER_SIZE*App::sc;
    mouseDownRelativePosition[1]=y+SAFETY_BORDER_SIZE*App::sc;
    mouseRelativePosition[0]=x+SAFETY_BORDER_SIZE*App::sc;
    mouseRelativePosition[1]=y+SAFETY_BORDER_SIZE*App::sc;
    previousMouseRelativePosition[0]=x+SAFETY_BORDER_SIZE*App::sc;
    previousMouseRelativePosition[1]=y+SAFETY_BORDER_SIZE*App::sc;
    shiftSelectionStarted=false;
    shiftingAllowed=false;

#ifndef KEYWORD__NOT_DEFINED_FORMELY_BR
    // We check if we have to launch a script editor window:
    int scriptID=getScriptActionObjectID(mouseDownRelativePosition[0],mouseDownRelativePosition[1]);
    if (scriptID!=-1)
    {
        CLuaScriptObject* it=App::ct->luaScriptContainer->getScriptFromID_noAddOnsNorSandbox(scriptID);
        if (it!=nullptr)
        {
            bool openScriptEditor=true;
            int auxData[4]={-1,-1,-1,-1};
            if (it->getScriptType()==sim_scripttype_childscript)
                auxData[0]=it->getObjectIDThatScriptIsAttachedTo_child();
            if (it->getScriptType()==sim_scripttype_customizationscript)
                auxData[0]=it->getObjectIDThatScriptIsAttachedTo_customization();
            int retVals[4]={-1,-1,-1,-1};
            void* returnVal=CPluginContainer::sendEventCallbackMessageToAllPlugins(sim_message_eventcallback_scripticondblclick,auxData,nullptr,retVals);
            delete[] (char*)returnVal;
            openScriptEditor=(retVals[0]!=1);
            if (openScriptEditor)
            {
                // Process the command via the simulation thread (delayed):
                SSimulationThreadCommand cmd;
                cmd.cmdId=OPEN_SCRIPT_EDITOR_CMD;
                cmd.intParams.push_back(it->getScriptID());
                App::appendSimulationThreadCommand(cmd);
            }
        }
        return(true);
    }
    scriptID=getScriptParameterActionObjectID(mouseDownRelativePosition[0],mouseDownRelativePosition[1]);
    if (scriptID!=-1)
    {
        CLuaScriptObject* it=App::ct->luaScriptContainer->getScriptFromID_noAddOnsNorSandbox(scriptID);
        if ( (it!=nullptr)&&((App::operationalUIParts&sim_gui_scriptsimulationparameters)!=0) )
        {
            // Process the command via the simulation thread (delayed):
            SSimulationThreadCommand cmd;
            cmd.cmdId=OPEN_MODAL_SCRIPT_SIMULATION_PARAMETERS_CMD;
            cmd.intParams.push_back(it->getScriptID());
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }
#endif
    if (App::getEditModeType()==NO_EDIT_MODE)
    {
        int objID=-1;
#ifndef KEYWORD__NOT_DEFINED_FORMELY_BR
        // Do we need to open an object property dialog?
        objID=getActionObjectID_icon(mouseDownRelativePosition[0],mouseDownRelativePosition[1]);
        if (objID!=-9999) // minus numbers are for the world(s)
        { // yes!
            if (App::mainWindow->getObjPropToggleViaGuiEnabled())
            {
                if (objID>=0)
                { // Regular object
                    App::ct->objCont->deselectObjects();
                    App::ct->objCont->addObjectToSelection(objID);
                    App::setFullDialogRefreshFlag();
                    App::mainWindow->dlgCont->processCommand(OPEN_OBJECT_DLG_OBJECT_SPECIFIC_PART_CMD);
                }
                else
                { // World object!
                    App::mainWindow->dlgCont->processCommand(OPEN_ENVIRONMENT_DLG_CMD);
                }
            }
            return(true);
        }

        // Do we need to open a model property dialog?
        objID=getActionModelID_icon(mouseDownRelativePosition[0],mouseDownRelativePosition[1]);
        if (objID>=0)
        { // yes!
            C3DObject* it=App::ct->objCont->getObjectFromHandle(objID);
            if (it!=nullptr)
            {
                // Process the command via the simulation thread (delayed):
                SSimulationThreadCommand cmd;
                cmd.cmdId=OPEN_MODAL_MODEL_PROPERTIES_CMD;
                cmd.intParams.push_back(it->getObjectHandle());
                App::appendSimulationThreadCommand(cmd);
            }
            return(true);
        }

        // Do we need to open a warning message?
        objID=getSimulationActionObjectID(mouseDownRelativePosition[0],mouseDownRelativePosition[1]);
        if (objID!=-1)
        { // yes!
            C3DObject* it=App::ct->objCont->getObjectFromHandle(objID);
            if (it!=nullptr)
            {
                std::string txt;
                if (it->getDynamicSimulationIconCode()==sim_dynamicsimicon_objectisnotdynamicallyenabled)
                {
                    txt=IDS_OBJECT;
                    txt+=" '"+it->getObjectName()+"' ";
                    txt+=IDS_IS_NOT_DYNAMICALLY_ENABLED_WARNING;
                }
                if (it->getDynamicSimulationIconCode()==sim_dynamicsimicon_objectisdynamicallysimulated)
                {
                    txt=IDS_OBJECT;
                    txt+=" '"+it->getObjectName()+"' ";
                    txt+=std::string(IDS_IS_DYNAMICALLY_SIMULATED)+"\n";
                    if (it->getObjectType()==sim_object_shape_type)
                    {
                        CShape* so=(CShape*)it;
                        if (so->getShapeIsDynamicallyStatic())
                        { // static
                            if (so->getRespondable())
                                txt+=IDS_SHAPE_IS_STATIC_AND_RESPONDABLE;
                            else
                                txt+=IDS_SHAPE_IS_STATIC_AND_NON_RESPONDABLE;
                        }
                        else
                        { // non-static
                            if (so->getRespondable())
                                txt+=IDS_SHAPE_IS_NON_STATIC_AND_RESPONDABLE;
                            else
                                txt+=IDS_SHAPE_IS_NON_STATIC_AND_NON_RESPONDABLE;
                        }
                    }
                    if (it->getObjectType()==sim_object_joint_type)
                    {
                        CJoint* so=(CJoint*)it;
                        if (so->getJointMode()==sim_jointmode_force)
                            txt+=IDS_JOINT_OPERATES_IN_FORCE_TORQUE_MODE;
                        else
                            txt+=IDS_JOINT_OPERATES_IN_HYBRID_FASHION;
                        txt+=' ';
                        if (so->getEnableDynamicMotor())
                        {
                            if (so->getEnableDynamicMotorControlLoop())
                                txt+=IDS__CONTROL_LOOP_ENABLED_;
                            else
                                txt+=IDS__CONTROL_LOOP_DISABLED_;
                        }
                        else
                            txt+=IDS__MOTOR_DISABLED_;
                    }
                    if (it->getObjectType()==sim_object_forcesensor_type)
                    {
                        CForceSensor* so=(CForceSensor*)it;
                        if (so->getEnableForceThreshold()||so->getEnableTorqueThreshold())
                            txt+=IDS_FORCE_SENSOR_IS_BREAKABLE;
                        else
                            txt+=IDS_FORCE_SENSOR_IS_NOT_BREAKABLE;
                    }
                }
                if (txt!="")
                    App::uiThread->messageBox_information(App::mainWindow,strTranslate("Dynamic property"),txt,VMESSAGEBOX_OKELI);
            }
            return(true);
        }
#endif
        // Do we need to do some label editing?
        objID=getTextActionObjectID(mouseDownRelativePosition[0],mouseDownRelativePosition[1]);
        if (objID==-9999)
            objID=-1;
        labelEditObjectID=objID;
        C3DObject* it=App::ct->objCont->getObjectFromHandle(objID);
        if (it!=nullptr)
        {
            editionText=it->getObjectName();
            editionTextEditPos=(int)editionText.length();
        }
        return(true);
    }

    if ( CMiscBase::handleVerSpec_supportsOpenglBasedCustomUiEdition()&&(App::getEditModeType()&BUTTON_EDIT_MODE) )
    {
        // Did we double-click the icon?
        int objID=getActionObjectID_icon(mouseDownRelativePosition[0],mouseDownRelativePosition[1]);
        if (objID!=-9999)
        { // yes!
            App::ct->buttonBlockContainer->setBlockInEdition(objID);
            App::setFullDialogRefreshFlag();
            App::mainWindow->dlgCont->processCommand(OPEN_BUTTON_DLG_CMD);
            return(true);
        }
        objID=getTextActionObjectID(mouseDownRelativePosition[0],mouseDownRelativePosition[1]);
        if (objID==-9999)
            objID=-1;
        labelEditObjectID=objID;
        CButtonBlock* it=App::ct->buttonBlockContainer->getBlockWithID(objID);
        if (it!=nullptr)
        {
            editionText=it->getBlockName();
            editionTextEditPos=(int)editionText.length();
        }
        return(true);
    }
    if ( CMiscBase::handleVerSpec_supportsOpenglBasedCustomUiEdition()&&(App::getEditModeType()&(VERTEX_EDIT_MODE|PATH_EDIT_MODE)) )
    {
        // Did we double-click the icon?
        int objID=getActionObjectID_icon(mouseDownRelativePosition[0],mouseDownRelativePosition[1]);
        if (objID!=-9999)
        { // yes!
            App::mainWindow->editModeContainer->deselectEditModeBuffer();
            App::mainWindow->editModeContainer->addItemToEditModeBuffer(objID,true);
            App::setFullDialogRefreshFlag();
            return(true);
        }
    }
    return(false);
}

void CHierarchy::validateViewPosition()
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    int tmp=renderingSize[0]-maxRenderedPosition[0]-viewPosition[0]-SAFETY_BORDER_SIZE*App::sc-verticalScrollbarWidth;
    if (tmp>0)
        viewPosition[0]=renderingSize[0]-(maxRenderedPosition[0]-minRenderedPosition[0])-SAFETY_BORDER_SIZE*App::sc-verticalScrollbarWidth;
    if (viewPosition[0]>SAFETY_BORDER_SIZE*App::sc)
        viewPosition[0]=SAFETY_BORDER_SIZE*App::sc;
    tmp=renderingSize[1]-(maxRenderedPosition[1]-minRenderedPosition[1])+viewPosition[1]+(-SAFETY_BORDER_SIZE-CONST_VAL_4)*App::sc-horizontalScrollbarHeight;
    if (tmp>0)
        viewPosition[1]=maxRenderedPosition[1]-minRenderedPosition[1]-renderingSize[1]+(SAFETY_BORDER_SIZE+CONST_VAL_6)*App::sc+horizontalScrollbarHeight;
    if (viewPosition[1]<(-SAFETY_BORDER_SIZE+CONST_VAL_8)*App::sc)
        viewPosition[1]=(-SAFETY_BORDER_SIZE+CONST_VAL_8)*App::sc;
}

void CHierarchy::setRebuildHierarchyFlag()
{
    rebuildHierarchyFlag=true;
    refreshViewFlag=App::userSettings->hierarchyRefreshCnt;
}

void CHierarchy::setResetViewFlag()
{
    resetViewFlag=true;
}

void CHierarchy::setRefreshViewFlag()
{
    refreshViewFlag=App::userSettings->hierarchyRefreshCnt;
}

int CHierarchy::getInflateActionObjectID(int mousePositionX,int mousePositionY)
{
    for (int i=0;i<int(inflateIconPosition.size())/3;i++)
    {
        if ((mousePositionX>=inflateIconPosition[3*i+0]-HIERARCHY_ICON_QUARTER_WIDTH*App::sc)&&(mousePositionX<=inflateIconPosition[3*i+0]+HIERARCHY_ICON_QUARTER_WIDTH*App::sc) )
        {
            if ((mousePositionY>=inflateIconPosition[3*i+1]-HIERARCHY_ICON_QUARTER_HEIGHT*App::sc)&&(mousePositionY<=inflateIconPosition[3*i+1]+HIERARCHY_ICON_QUARTER_HEIGHT*App::sc) )
                return(inflateIconPosition[3*i+2]);
        }
    }
    return(-1);
}

int CHierarchy::getScriptActionObjectID(int mousePositionX,int mousePositionY)
{
    for (int i=0;i<int(scriptIconPosition.size())/3;i++)
    {
        if ((mousePositionX>=scriptIconPosition[3*i+0]-HIERARCHY_ICON_HALF_WIDTH*App::sc)&&(mousePositionX<=scriptIconPosition[3*i+0]+HIERARCHY_ICON_HALF_WIDTH*App::sc) )
        {
            if ((mousePositionY>=scriptIconPosition[3*i+1]-HIERARCHY_ICON_HALF_HEIGHT*App::sc)&&(mousePositionY<=scriptIconPosition[3*i+1]+HIERARCHY_ICON_HALF_HEIGHT*App::sc) )
                return(scriptIconPosition[3*i+2]);
        }
    }
    return(-1);
}
int CHierarchy::getScriptParameterActionObjectID(int mousePositionX,int mousePositionY)
{
    for (int i=0;i<int(scriptParametersIconPosition.size())/3;i++)
    {
        if ((mousePositionX>=scriptParametersIconPosition[3*i+0]-HIERARCHY_ICON_HALF_WIDTH*App::sc)&&(mousePositionX<=scriptParametersIconPosition[3*i+0]+HIERARCHY_ICON_HALF_WIDTH*App::sc) )
        {
            if ((mousePositionY>=scriptParametersIconPosition[3*i+1]-HIERARCHY_ICON_HALF_HEIGHT*App::sc)&&(mousePositionY<=scriptParametersIconPosition[3*i+1]+HIERARCHY_ICON_HALF_HEIGHT*App::sc) )
                return(scriptParametersIconPosition[3*i+2]);
        }
    }
    return(-1);
}

int CHierarchy::getTextActionObjectID(int mousePositionX,int mousePositionY)
{
    for (int i=0;i<int(textPosition.size())/6;i++)
    {
        if ((mousePositionX>=textPosition[6*i+2])&&(mousePositionX<=textPosition[6*i+3]) )
        {
            if ((mousePositionY>=textPosition[6*i+4]-HIERARCHY_HALF_INTER_LINE_SPACE*App::sc)&&(mousePositionY<=textPosition[6*i+4]+HIERARCHY_HALF_INTER_LINE_SPACE*App::sc) )
                return(textPosition[6*i+5]);
        }
    }
    return(-9999); // minus number are for worlds (also in different instances)
}

int CHierarchy::getLineObjectID(int mousePositionY,int textPosStart[2])
{
    for (int i=0;i<int(textPosition.size())/6;i++)
    {
        if ((mousePositionY>=textPosition[6*i+4]-HIERARCHY_HALF_INTER_LINE_SPACE*App::sc)&&(mousePositionY<=textPosition[6*i+4]+HIERARCHY_HALF_INTER_LINE_SPACE*App::sc) )
        {
            if (textPosStart!=nullptr)
            {
                textPosStart[0]=textPosition[6*i+0];
                textPosStart[1]=textPosition[6*i+1];
            }
            return(textPosition[6*i+5]);
        }
    }
    return(-9999); // minus number are for worlds (also in different instances)
}

int CHierarchy::getActionObjectID_icon(int mousePositionX,int mousePositionY,bool ignoreXPositionAndCheckLineInstead/*=false*/)
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    if (!ignoreXPositionAndCheckLineInstead)
    {
        for (int i=0;i<int(objectIconPosition.size())/3;i++)
        {
            if ( (mousePositionX>=objectIconPosition[3*i+0]-HIERARCHY_ICON_HALF_WIDTH*App::sc)&&(mousePositionX<=objectIconPosition[3*i+0]+HIERARCHY_ICON_HALF_WIDTH*App::sc) )
            {
                if ((mousePositionY>=objectIconPosition[3*i+1]-HIERARCHY_ICON_HALF_HEIGHT*App::sc)&&(mousePositionY<=objectIconPosition[3*i+1]+HIERARCHY_ICON_HALF_HEIGHT*App::sc) )
                    return(objectIconPosition[3*i+2]);
            }
        }
    }
    else
    {
        for (int i=0;i<int(objectIconPosition.size())/3;i++)
        {
            if ((mousePositionY>=objectIconPosition[3*i+1]-HIERARCHY_HALF_INTER_LINE_SPACE*App::sc)&&(mousePositionY<=objectIconPosition[3*i+1]+HIERARCHY_HALF_INTER_LINE_SPACE*App::sc) )
                return(objectIconPosition[3*i+2]);
        }
    }
    return(-9999); // minus number are for worlds (also in different instances)
}

int CHierarchy::getActionModelID_icon(int mousePositionX,int mousePositionY)
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    for (int i=0;i<int(modelIconPosition.size())/3;i++)
    {
        if ((mousePositionX>=modelIconPosition[3*i+0]-HIERARCHY_ICON_QUARTER_WIDTH*App::sc)&&(mousePositionX<=modelIconPosition[3*i+0]+HIERARCHY_ICON_QUARTER_WIDTH*App::sc) )
        {
            if ((mousePositionY>=modelIconPosition[3*i+1]-HIERARCHY_ICON_QUARTER_HEIGHT*App::sc)&&(mousePositionY<=modelIconPosition[3*i+1]+HIERARCHY_ICON_QUARTER_HEIGHT*App::sc) )
                return(modelIconPosition[3*i+2]);
        }
    }
    return(-9999); // minus number are for worlds (also in different instances)
}

int CHierarchy::getSimulationActionObjectID(int mousePositionX,int mousePositionY)
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    for (int i=0;i<int(simulationIconPosition.size())/3;i++)
    {
        if ((mousePositionX>=simulationIconPosition[3*i+0]-HIERARCHY_ICON_HALF_WIDTH*App::sc)&&(mousePositionX<=simulationIconPosition[3*i+0]+HIERARCHY_ICON_HALF_WIDTH*App::sc) )
        {
            if ((mousePositionY>=simulationIconPosition[3*i+1]-HIERARCHY_ICON_HALF_HEIGHT*App::sc)&&(mousePositionY<=simulationIconPosition[3*i+1]+HIERARCHY_ICON_HALF_HEIGHT*App::sc) )
                return(simulationIconPosition[3*i+2]);
        }
    }
    return(-1);
}

int CHierarchy::getActionObjectID(int mousePositionY,int tolerance)
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    for (int i=0;i<int(objectPosition.size())/2;i++)
    {
        if ((mousePositionY<=objectPosition[2*i+0]+tolerance)&&(mousePositionY>=objectPosition[2*i+0]-tolerance) )
            return(objectPosition[2*i+1]);
    }
    return(-9999);
}

void CHierarchy::getActionObjectIDs(int mouseDownPositionY,int mouseUpPositionY,int tolerance,std::vector<int>* toBeSelected)
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    int minV=mouseUpPositionY-tolerance;
    int maxV=mouseDownPositionY+tolerance;
    if (mouseDownPositionY<mouseUpPositionY)
    {
        minV=mouseDownPositionY-tolerance;
        maxV=mouseUpPositionY+tolerance;
    }
    for (int i=0;i<int(objectPosition.size())/2;i++)
    {
        if ((maxV>=objectPosition[2*i+0])&&(minV<=objectPosition[2*i+0]) )
            toBeSelected->push_back(objectPosition[2*i+1]);
    }
}

std::string CHierarchy::getEditedLabel()
{
    return(editionText);
}

void CHierarchy::setEditionLabel(std::string txt)
{
    editionText=txt;
}

void CHierarchy::drawEditionLabel(int textPosX,int textPosY)
{
    int buttonWidth=20+ogl::getTextLengthInPixels(editionText);
    VPoint p(textPosX-2+buttonWidth/2,textPosY+HIERARCHY_TEXT_CENTER_OFFSET*App::sc);
    VPoint s(buttonWidth,HIERARCHY_INTER_LINE_SPACE*App::sc);
    float txtCol[3]={0.0f,0.0f,0.0f};
    float backCol[3]={1.0f,1.0f,0.0f};
    int buttonAttrib=sim_buttonproperty_editbox|sim_buttonproperty_enabled|sim_buttonproperty_verticallycentered;
    ogl::drawButton(p,s,txtCol,backCol,backCol,editionText,buttonAttrib,true,editionTextEditPos,0.0f,false,VDateTime::getTimeInMs(),nullptr,nullptr,nullptr,nullptr,nullptr);
    refreshViewFlag=App::userSettings->hierarchyRefreshCnt;
}

void CHierarchy::addMenu(VMenu* menu)
{ 
    bool selection=App::ct->objCont->getSelSize()>0;
    menu->appendMenuItem(true,false,EXPAND_HIERARCHY_CMD,IDS_EXPAND_ALL_MENU_ITEM);
    menu->appendMenuItem(true,false,COLLAPSE_HIERARCHY_CMD,IDS_COLLAPSE_ALL_MENU_ITEM);
    menu->appendMenuItem(selection,false,EXPAND_SELECTED_HIERARCHY_CMD,IDS_EXPAND_SELECTED_TREE_MENU_ITEM);
    menu->appendMenuItem(selection,false,COLLAPSE_SELECTED_HIERARCHY_CMD,IDS_COLLAPSE_SELECTED_TREE_MENU_ITEM);
}

bool CHierarchy::processCommand(int commandID)
{ // Return value is true if the command belonged to Add menu and was executed
    if ( (commandID==EXPAND_HIERARCHY_CMD)||(commandID==COLLAPSE_HIERARCHY_CMD) )
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            for (int i=0;i<int(App::ct->objCont->objectList.size());i++)
            {
                C3DObject* it=App::ct->objCont->getObjectFromHandle(App::ct->objCont->objectList[i]);
                it->setLocalObjectProperty(it->getLocalObjectProperty()|sim_objectproperty_collapsed);
                if (commandID==EXPAND_HIERARCHY_CMD)
                    it->setLocalObjectProperty(it->getLocalObjectProperty()-sim_objectproperty_collapsed);
            }
            if (commandID==EXPAND_HIERARCHY_CMD)
                App::addStatusbarMessage(IDSNS_HIERARCHY_EXPANDED);
            else
                App::addStatusbarMessage(IDSNS_HIERARCHY_COLLAPSED);
            POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }
    if ( (commandID==EXPAND_SELECTED_HIERARCHY_CMD)||(commandID==COLLAPSE_SELECTED_HIERARCHY_CMD) )
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            for (int i=0;i<int(App::ct->objCont->getSelSize());i++)
            {
                C3DObject* it=App::ct->objCont->getObjectFromHandle(App::ct->objCont->getSelID(i));
                std::vector<C3DObject*> toExplore;
                toExplore.push_back(it);
                while (toExplore.size()!=0)
                {
                    it=toExplore[0];
                    it->setLocalObjectProperty(it->getLocalObjectProperty()|sim_objectproperty_collapsed);
                    if (commandID==EXPAND_SELECTED_HIERARCHY_CMD)
                        it->setLocalObjectProperty(it->getLocalObjectProperty()-sim_objectproperty_collapsed);
                    toExplore.erase(toExplore.begin());
                    for (int j=0;j<int(it->childList.size());j++)
                        toExplore.push_back(it->childList[j]);
                }
            }
            if (commandID==EXPAND_HIERARCHY_CMD)
                App::addStatusbarMessage(IDSNS_HIERARCHY_TREES_EXPANDED);
            else
                App::addStatusbarMessage(IDSNS_HIERARCHY_TREES_COLLAPSED);
            POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }
    if ( (commandID>=HIERARCHY_COLORING_NONE_CMD)&&(commandID<=HIERARCHY_COLORING_BLUE_CMD) )
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            for (int i=0;i<int(App::ct->objCont->getSelSize());i++)
            {
                C3DObject* it=App::ct->objCont->getObjectFromHandle(App::ct->objCont->getSelID(i));
                it->setHierarchyColorIndex(commandID-HIERARCHY_COLORING_NONE_CMD-1);
            }
            POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }
    
    return(false);
}

void CHierarchy::_drawLinesLinkingDummies(int maxRenderedPos[2])
{
    std::vector<int> positions; // contains only objects that have a dummy linking to another, as child (or the dummy itself)
    for (int i=0;i<int(App::ct->objCont->dummyList.size());i++)
    {
        CDummy* dummy=App::ct->objCont->getDummy(App::ct->objCont->dummyList[i]);
        if (dummy->getLinkedDummyID()!=-1)
        {
            int dummyID=dummy->getObjectHandle();
            int linkedDummyID=dummy->getLinkedDummyID();
            C3DObject* obj=dummy;
            bool found=false;
            while ( (!found)&&(obj!=nullptr) )
            {
                int idToSearch=obj->getObjectHandle();
                for (int j=0;j<int(lineLastPosition.size()/3);j++)
                {
                    if (idToSearch==lineLastPosition[3*j+2])
                    { // we found a pos
                        positions.push_back(lineLastPosition[3*j+0]);
                        positions.push_back(lineLastPosition[3*j+1]);
                        positions.push_back(dummyID);
                        positions.push_back(linkedDummyID);
                        positions.push_back(j); // index in the lineLastPosition list
                        int wv=0;
                        if (dummyID==idToSearch)
                            wv|=1; // the dummy is visible (otherwise it is not visible (built on a collapsed item))
                        if (App::ct->objCont->isObjectSelected(dummyID)||App::ct->objCont->isObjectSelected(linkedDummyID))
                            wv|=2; // one of the dummies is selected
                        positions.push_back(wv);
                        positions.push_back(dummy->getLinkType());
                        found=true;
                        break;
                    }
                }
                if (!found)
                    obj=obj->getParentObject();
            }
        }
    }
    glLineStipple(1,0x5555);
    glEnable(GL_LINE_STIPPLE);
    std::vector<int> linesPresent; // minY, maxY, X
    const int segmentOffset=5;
    const int segmentWidth=10;
    for (int i=0;i<int(positions.size())/7;i++)
    {
        if (positions[7*i+5]!=-1)
        { // not yet processed
            int firstIndex=positions[7*i+4];
            for (int j=i+1;j<int(positions.size())/7;j++)
            {
                int secondDummyID=positions[7*j+2];
                if (positions[7*j+5]!=-1)
                { // not yet processed
                    if (secondDummyID==positions[7*i+3])
                    { // this is the one!
                        int secondIndex=positions[7*j+4];
                        if (secondIndex!=firstIndex)
                        { // make sure the two dummies are not linking to the same line!
                            int maxX=SIM_MAX(positions[7*i+0],positions[7*j+0]);
                            int overallOffset=0;
                            // a. we don't want to intersect any text/icon in-between. We search for the overallOffset needed:
                            int low=SIM_MIN(firstIndex,secondIndex);
                            int high=SIM_MAX(firstIndex,secondIndex);
                            int maxOtherX=0;
                            for (int k=low+1;k<high;k++)
                            {
                                if (lineLastPosition[3*k+0]>maxOtherX)
                                    maxOtherX=lineLastPosition[3*k+0];
                            }
                            if (maxOtherX>maxX)
                                overallOffset+=maxOtherX-maxX;
                            // b. we don't want to overlap other dummy-dummy link lines
                            // i.e. maxX+overallOffset should no lie within other lines maxX+overallOffset
                            int minY=SIM_MIN(positions[7*i+1],positions[7*j+1]);
                            int maxY=SIM_MAX(positions[7*i+1],positions[7*j+1]);
                            for (int k=0;k<int(linesPresent.size())/3;k++)
                            {
                                if ((maxY>=linesPresent[3*k+0])&&(minY<=linesPresent[3*k+1]))
                                {
                                    bool restart=false;
                                    while (abs(maxX+overallOffset-linesPresent[3*k+2])<segmentWidth+4)
                                    {
                                        overallOffset+=2;
                                        restart=true;
                                    }
                                    if (restart)
                                        k=-1;
                                }
                            }

                            if (positions[7*i+5]&2)
                            {
                                glLineWidth(3.0);
                                glDisable(GL_LINE_STIPPLE);
                            }
                            if ( (positions[7*i+6]==sim_dummy_linktype_dynamics_loop_closure)||(positions[7*i+6]==sim_dummy_linktype_dynamics_force_constraint) )
                                ogl::setMaterialColor(sim_colorcomponent_emission,ogl::colorBlue);
                            if ( (positions[7*i+6]==sim_dummy_linktype_gcs_loop_closure)||(positions[7*i+6]==sim_dummy_linktype_gcs_tip)||(positions[7*i+6]==sim_dummy_linktype_gcs_target) )
                                ogl::setMaterialColor(sim_colorcomponent_emission,0.0f,0.6f,0.0f);
                            if (positions[7*i+6]==sim_dummy_linktype_ik_tip_target)
                                ogl::setMaterialColor(sim_colorcomponent_emission,ogl::colorRed);
                            ogl::drawSingle2dLine_i(maxX+segmentOffset+overallOffset,positions[7*i+1],maxX+segmentOffset+segmentWidth+overallOffset,positions[7*i+1]);
                            if (positions[7*i+5]&1)
                            {
                                glDisable(GL_LINE_STIPPLE);
                                ogl::drawSingle2dLine_i(maxX+segmentOffset+overallOffset,positions[7*i+1],maxX+segmentOffset+4+overallOffset,positions[7*i+1]+4);
                                ogl::drawSingle2dLine_i(maxX+segmentOffset+overallOffset,positions[7*i+1],maxX+segmentOffset+4+overallOffset,positions[7*i+1]-4);
                                if ((positions[7*i+5]&2)==0)
                                    glEnable(GL_LINE_STIPPLE);
                            }
                            ogl::drawSingle2dLine_i(maxX+segmentOffset+overallOffset,positions[7*j+1],maxX+segmentOffset+segmentWidth+overallOffset,positions[7*j+1]);
                            if (positions[7*j+5]&1)
                            {
                                glDisable(GL_LINE_STIPPLE);
                                ogl::drawSingle2dLine_i(maxX+segmentOffset+overallOffset,positions[7*j+1],maxX+segmentOffset+4+overallOffset,positions[7*j+1]+4);
                                ogl::drawSingle2dLine_i(maxX+segmentOffset+overallOffset,positions[7*j+1],maxX+segmentOffset+4+overallOffset,positions[7*j+1]-4);
                                if ((positions[7*i+5]&2)==0)
                                    glEnable(GL_LINE_STIPPLE);
                            }
                            ogl::drawSingle2dLine_i(maxX+segmentOffset+segmentWidth+overallOffset,positions[7*i+1],maxX+segmentOffset+segmentWidth+overallOffset,positions[7*j+1]);
                            glLineWidth(1.0);
                            glEnable(GL_LINE_STIPPLE);
                            linesPresent.push_back(minY);
                            linesPresent.push_back(maxY);
                            linesPresent.push_back(maxX+overallOffset);
                            if (maxRenderedPos[0]<maxX+overallOffset+segmentOffset+segmentWidth)
                                maxRenderedPos[0]=maxX+overallOffset+segmentOffset+segmentWidth;
                        }
                        positions[7*j+5]=-1; // do not process this position anymore
                        break;
                    }
                }   
            }
        }
    }
    glDisable(GL_LINE_STIPPLE);
}

