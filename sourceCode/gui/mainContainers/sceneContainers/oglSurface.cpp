
#include "vrepMainHeader.h"
#include "funcDebug.h"
#include "v_rep_internal.h"
#include "oglSurface.h"
#include "tt.h"
#include "oGL.h"
#include "app.h"
#include "simulation.h"
#include "dlgCont.h"
#include "imgLoaderSaver.h"
#include "vDateTime.h"
#include "pluginContainer.h"
#include "v_repStrings.h"
#include "miscBase.h"

enum {NO_ONE=0,HIERARCHY_PART,VIEW_SELECTION_PART,PAGE_PART,
        HIERARCHY_RESIZING_PART,HIERARCHY_CLOSING_PART,BROWSER_PART,
        BROWSER_RESIZING_PART,BROWSER_CLOSING_PART};

#define HIERARCHY_WIDTH 300
#define HIERARCHY_MIN_WIDTH 150

bool COglSurface::_hierarchyEnabled=false;
int COglSurface::_hierarchyWidth=HIERARCHY_WIDTH;
int COglSurface::_hierarchyMinWidth=HIERARCHY_MIN_WIDTH;

COglSurface::COglSurface()
{
    FUNCTION_DEBUG;

    _hierarchyWidth=HIERARCHY_WIDTH*App::sc;
    _hierarchyMinWidth=HIERARCHY_MIN_WIDTH*App::sc;

    viewSelector=new CViewSelector();
    pageSelector=new CPageSelector();
    sceneSelector=new CSceneSelector();

    hierarchy=new CHierarchy();

    setUpDefaultValues();
}

COglSurface::~COglSurface()
{
    delete hierarchy;
    delete sceneSelector;
    delete pageSelector;
    delete viewSelector;
}

void COglSurface::adjustBrowserAndHierarchySizesToDefault()
{
    _hierarchyWidth=HIERARCHY_WIDTH*App::sc;
    _hierarchyMinWidth=HIERARCHY_MIN_WIDTH*App::sc;
}

void COglSurface::setSurfaceSizeAndPosition(int sizeX,int sizeY,int posX,int posY)
{
    surfaceSize[0]=sizeX;
    surfaceSize[1]=sizeY;
    surfacePosition[0]=posX;
    surfacePosition[1]=posY;
    actualizeAllSurfacesSizeAndPosition();
    _readyToRender=true;
}

bool COglSurface::getMouseRelPosObjectAndViewSize(int x,int y,int relPos[2],int& objType,int& objID,int vSize[2],bool& viewIsPerspective)
{ // NOT FULLY IMPLEMENTED! objType=-1 --> not supported, 0 --> hierarchy, 1 --> 3DViewable
    int offx=0;
    int offy=0;
    if (sceneSelectionActive)
        return(false);
    if (pageSelectionActive)
        return(false);
    if (viewSelectionActive)
        return(false);
    if (_hierarchyEnabled)
    {
        VPoint btl(offx+_hierarchyWidth-BROWSER_HIERARCHY_TITLE_BAR_CLOSING_BUTTON_WIDTH*App::sc-BROWSER_HIERARCHY_MAIN_RENDERING_WINDOW_SEPARATION_WIDTH,surfaceSize[1]-BROWSER_HIERARCHY_TITLE_BAR_HEIGHT*App::sc);
        VPoint btr(offx+_hierarchyWidth-BROWSER_HIERARCHY_MAIN_RENDERING_WINDOW_SEPARATION_WIDTH,surfaceSize[1]);
        if ( (mouseRelativePosition[0]>=btl.x)&&(mouseRelativePosition[0]<=btr.x)&&
            (mouseRelativePosition[1]>=btl.y)&&(mouseRelativePosition[1]<=btr.y) )
        { // We are on the closing button
            objType=-1;
            return(true); 
        }
        if ( (x>=offx)&&(x<offx+_hierarchyWidth)&&(y>=offy)&&(y<offy+surfaceSize[1]) )
        {
            objType=0;
            relPos[0]=x-offx;
            relPos[1]=y-offy;
            return(true); // We are in the hierarchy window
        }
    }
    if (_hierarchyEnabled)
        offx+=_hierarchyWidth;
    if (App::ct->pageContainer->getMouseRelPosObjectAndViewSize(x-offx,y-offy,relPos,objType,objID,vSize,viewIsPerspective))
        return(true); // We are in the views windows and have an object
    return(false);
}

bool COglSurface::leftMouseButtonDown(int x,int y,int selectionStatus)
{
    int offx=0;
    int offy=0;
    mouseRelativePosition[0]=x;
    mouseRelativePosition[1]=y;
    mousePreviousRelativePosition[0]=mouseRelativePosition[0];
    mousePreviousRelativePosition[1]=mouseRelativePosition[1];
    sceneSelector->clearCaughtElements(0xffff-sim_left_button);
    pageSelector->clearCaughtElements(0xffff-sim_left_button);
    viewSelector->clearCaughtElements(0xffff-sim_left_button);
    hierarchy->clearCaughtElements(0xffff-sim_left_button);
    App::ct->pageContainer->clearCaughtElements(0xffff-sim_left_button);
    _caughtElements&=0xffff-sim_left_button;
    if (sceneSelectionActive)
    { 
        sceneSelector->leftMouseButtonDown(mouseRelativePosition[0],mouseRelativePosition[1],selectionStatus);
        return(true); // We want the mouse captured!
    }
    if (pageSelectionActive)
    { 
        pageSelector->leftMouseButtonDown(mouseRelativePosition[0],mouseRelativePosition[1],selectionStatus);
        return(true); // We want the mouse captured!
    }
    if (viewSelectionActive)
    { 
        viewSelector->leftMouseButtonDown(mouseRelativePosition[0],mouseRelativePosition[1],selectionStatus);
        return(true); // We want the mouse captured!
    }
    if (_hierarchyEnabled)
    {
        VPoint btl(offx+_hierarchyWidth-BROWSER_HIERARCHY_TITLE_BAR_CLOSING_BUTTON_WIDTH*App::sc-BROWSER_HIERARCHY_MAIN_RENDERING_WINDOW_SEPARATION_WIDTH,surfaceSize[1]-BROWSER_HIERARCHY_TITLE_BAR_HEIGHT*App::sc);
        VPoint btr(offx+_hierarchyWidth-BROWSER_HIERARCHY_MAIN_RENDERING_WINDOW_SEPARATION_WIDTH,surfaceSize[1]);
        if ( (mouseRelativePosition[0]>=btl.x)&&(mouseRelativePosition[0]<=btr.x)&&
            (mouseRelativePosition[1]>=btl.y)&&(mouseRelativePosition[1]<=btr.y) )
        { // We pressed the hierarchy closing button:
            _caughtElements|=sim_left_button|sim_focus;
            _hierarchyClosingButtonDown=true;
            return(true); // We want the mouse captured!
        }
        if (_hierarchyResizingMousePosition(mouseRelativePosition[0],mouseRelativePosition[1]))
        { // We start a resizing action of the hierarchy view:
            _caughtElements|=sim_left_button|sim_focus;
            _hierarchyResizingAction=true;
            return(true); // We want the mouse captured!
        }
        if ( hierarchy->leftMouseDown(mouseRelativePosition[0]-offx,mouseRelativePosition[1],selectionStatus) )
        { // Mouse went down on the hierarchy window:
            setFocusObject(FOCUS_ON_HIERARCHY);
            return(true); // We want the mouse captured!
        }
    }
    if (_hierarchyEnabled)
        offx+=_hierarchyWidth;
    if (App::ct->pageContainer->leftMouseButtonDown(mouseRelativePosition[0]-offx,mouseRelativePosition[1]-offy,selectionStatus))
    { // Mouse went down on the views window:
        setFocusObject(App::ct->pageContainer->getFocusObject());
        return(true); // We want the mouse captured!
    }
    return(false); // Nothing caught that action
}

bool COglSurface::_hierarchyResizingMousePosition(int x,int y)
{
    int b=0;
    return( (x>b+_hierarchyWidth-4)&&(x<b+_hierarchyWidth+4)&&
            (y>=0)&&(y<=surfaceSize[1]) );
}

void COglSurface::leftMouseButtonUp(int x,int y)
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    bool hierarchyCloseCopy=_hierarchyClosingButtonDown;
    _hierarchyClosingButtonDown=false;
    _hierarchyResizingAction=false;
    int offx=0;
    int offy=0;
    mouseRelativePosition[0]=x;
    mouseRelativePosition[1]=y;
    mousePreviousRelativePosition[0]=mouseRelativePosition[0];
    mousePreviousRelativePosition[1]=mouseRelativePosition[1];

    if (viewSelectionActive&&(viewSelector->getCaughtElements()&sim_left_button))
        viewSelector->leftMouseButtonUp(mouseRelativePosition[0],mouseRelativePosition[1]);
    if (pageSelectionActive&&(pageSelector->getCaughtElements()&sim_left_button))
        pageSelector->leftMouseButtonUp(mouseRelativePosition[0],mouseRelativePosition[1]);
    if (sceneSelectionActive&&(sceneSelector->getCaughtElements()&sim_left_button))
        sceneSelector->leftMouseButtonUp(mouseRelativePosition[0],mouseRelativePosition[1]);
    if (_hierarchyEnabled)
    {
        if (_caughtElements&sim_left_button)
        {
            VPoint btl(offx+_hierarchyWidth-BROWSER_HIERARCHY_TITLE_BAR_CLOSING_BUTTON_WIDTH*App::sc-BROWSER_HIERARCHY_MAIN_RENDERING_WINDOW_SEPARATION_WIDTH,surfaceSize[1]-BROWSER_HIERARCHY_TITLE_BAR_HEIGHT*App::sc);
            VPoint btr(offx+_hierarchyWidth-BROWSER_HIERARCHY_MAIN_RENDERING_WINDOW_SEPARATION_WIDTH,surfaceSize[1]);
            if ( hierarchyCloseCopy&&App::mainWindow->getHierarchyToggleViaGuiEnabled() )
            {
                if ( (mouseRelativePosition[0]>=btl.x)&&(mouseRelativePosition[0]<=btr.x)&&
                    (mouseRelativePosition[1]>=btl.y)&&(mouseRelativePosition[1]<=btr.y) )
                { // We have to close the hierarchy!!
                    if (App::getEditModeType()==NO_EDIT_MODE) // In edit mode, we disable the close button (needs to stay open)
                        setHierarchyEnabled(false);
                }
            }
        }
        if (hierarchy->getCaughtElements()&sim_left_button)
            hierarchy->leftMouseUp(mouseRelativePosition[0]-offx,mouseRelativePosition[1]);
    }
    if (_hierarchyEnabled)
        offx+=_hierarchyWidth;
    if (App::ct->pageContainer->getCaughtElements()&sim_left_button)
        App::ct->pageContainer->leftMouseButtonUp(mouseRelativePosition[0]-offx,mouseRelativePosition[1]-offy);
}
void COglSurface::mouseWheel(int deltaZ,int x,int y)
{
    int offx=0;
    int offy=0;
    if (sceneSelectionActive)
        return;
    if (pageSelectionActive)
        return;
    if (viewSelectionActive)
        return;
    if (_hierarchyEnabled)
    {
        hierarchy->mouseWheel(deltaZ,x-offx,y-offy);
        offx+=_hierarchyWidth;
    }
#ifdef MAC_VREP
        deltaZ=-deltaZ; // on Mac the mouse wheel appears inverted for that
#endif
    App::ct->pageContainer->mouseWheel(deltaZ,x-offx,y-offy);
}

void COglSurface::mouseMove(int x,int y,bool passiveAndFocused)
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    int bts=sim_right_button|sim_middle_button|sim_left_button;
    if (App::userSettings->navigationBackwardCompatibility)
        bts=sim_right_button|sim_left_button;

    int offx=0;
    int offy=0;
    mouseRelativePosition[0]=x;
    mouseRelativePosition[1]=y;

    if (sceneSelectionActive)
    {
        if (!passiveAndFocused)
        {
            if (sceneSelector->getCaughtElements()&bts)
                sceneSelector->mouseMove(mouseRelativePosition[0],mouseRelativePosition[1],passiveAndFocused);
        }
        else
            sceneSelector->mouseMove(mouseRelativePosition[0],mouseRelativePosition[1],passiveAndFocused);
    }
    if (pageSelectionActive)
    {
        if (!passiveAndFocused)
        {
            if (pageSelector->getCaughtElements()&bts)
                pageSelector->mouseMove(mouseRelativePosition[0],mouseRelativePosition[1],passiveAndFocused);
        }
        else
            pageSelector->mouseMove(mouseRelativePosition[0],mouseRelativePosition[1],passiveAndFocused);
    }
    if (viewSelectionActive)
    {
        if (!passiveAndFocused)
        {
            if (viewSelector->getCaughtElements()&bts)
                viewSelector->mouseMove(mouseRelativePosition[0],mouseRelativePosition[1],passiveAndFocused);
        }
        else
            viewSelector->mouseMove(mouseRelativePosition[0],mouseRelativePosition[1],passiveAndFocused);
    }
    if (_hierarchyEnabled)
    {
        int b=0;
        VPoint btl(b+_hierarchyWidth-BROWSER_HIERARCHY_TITLE_BAR_CLOSING_BUTTON_WIDTH*App::sc-BROWSER_HIERARCHY_MAIN_RENDERING_WINDOW_SEPARATION_WIDTH,surfaceSize[1]-BROWSER_HIERARCHY_TITLE_BAR_HEIGHT*App::sc);
        VPoint btr(b+_hierarchyWidth-BROWSER_HIERARCHY_MAIN_RENDERING_WINDOW_SEPARATION_WIDTH,surfaceSize[1]);
        if (!passiveAndFocused)
        {
            if (_caughtElements&bts)
            {
                if (_hierarchyClosingButtonDown)
                {
                    if ( (mouseRelativePosition[0]<btl.x)||(mouseRelativePosition[0]>btr.x)||
                        (mouseRelativePosition[1]<btl.y)||(mouseRelativePosition[1]>btr.y) )
                        _hierarchyClosingButtonDown=false; // We moved away from the button
                }
                if (_hierarchyResizingAction)
                {
                    _hierarchyWidth=mouseRelativePosition[0]-offx;
                    actualizeAllSurfacesSizeAndPosition();
                }

            }
            if (hierarchy->getCaughtElements()&bts)
                hierarchy->mouseMove(mouseRelativePosition[0]-offx,mouseRelativePosition[1],passiveAndFocused);
        }
        else
            hierarchy->mouseMove(mouseRelativePosition[0]-offx,mouseRelativePosition[1],passiveAndFocused);
    }
    if (_hierarchyEnabled)
        offx+=_hierarchyWidth;
    if (!passiveAndFocused)
    {
        if (App::ct->pageContainer->getCaughtElements()&bts)
            App::ct->pageContainer->mouseMove(mouseRelativePosition[0]-offx,mouseRelativePosition[1]-offy,passiveAndFocused);
    }
    else
        App::ct->pageContainer->mouseMove(mouseRelativePosition[0]-offx,mouseRelativePosition[1]-offy,passiveAndFocused);

    mousePreviousRelativePosition[0]=mouseRelativePosition[0];
    mousePreviousRelativePosition[1]=mouseRelativePosition[1];
}

int COglSurface::modelDragMoveEvent(int xPos,int yPos,C3Vector* desiredModelPosition)
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    int offx=0;
    int offy=0;
    mouseRelativePosition[0]=xPos;
    mouseRelativePosition[1]=yPos;

    if (sceneSelectionActive||pageSelectionActive||viewSelectionActive)
        return(0);
    if (_hierarchyEnabled)
    {
        offx+=_hierarchyWidth;
        if (xPos-offx<=0)
        {
            if (desiredModelPosition!=nullptr)
                desiredModelPosition->clear();
            return(1);
        }
    }
    mousePreviousRelativePosition[0]=mouseRelativePosition[0];
    mousePreviousRelativePosition[1]=mouseRelativePosition[1];
    return(App::ct->pageContainer->modelDragMoveEvent(mouseRelativePosition[0]-offx,mouseRelativePosition[1]-offy,desiredModelPosition));
}


int COglSurface::getCursor(int x,int y)
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    int offx=0;
    int offy=0;
    if (_hierarchyEnabled)
        offx+=_hierarchyWidth;
    if (sceneSelectionActive)
        return(sceneSelector->getCursor(x,y));
    if (pageSelectionActive)
        return(pageSelector->getCursor(x,y));
    if (viewSelectionActive)
        return(viewSelector->getCursor(x,y));
    if (_hierarchyEnabled&&_hierarchyResizingMousePosition(x,y))
        return(sim_cursor_horizontal_directions);
    return(App::ct->pageContainer->getCursor(x-offx,y-offy));
}

bool COglSurface::rightMouseButtonDown(int x,int y)
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    int offx=0;
    int offy=0;
    mouseRelativePosition[0]=x;
    mouseRelativePosition[1]=y;
    mousePreviousRelativePosition[0]=mouseRelativePosition[0];
    mousePreviousRelativePosition[1]=mouseRelativePosition[1];
    sceneSelector->clearCaughtElements(0xffff-sim_right_button);
    pageSelector->clearCaughtElements(0xffff-sim_right_button);
    viewSelector->clearCaughtElements(0xffff-sim_right_button);
    hierarchy->clearCaughtElements(0xffff-sim_right_button);
    App::ct->pageContainer->clearCaughtElements(0xffff-sim_right_button);
    if (sceneSelectionActive)
    { // Mouse went down on scene selector
        setFocusObject(FOCUS_ON_SCENE_SELECTION_WINDOW);
        return(sceneSelector->rightMouseButtonDown(mouseRelativePosition[0],mouseRelativePosition[1]));
    }
    if (pageSelectionActive)
    { // Mouse went down on page selector
        setFocusObject(FOCUS_ON_PAGE_SELECTION_WINDOW);
        return(pageSelector->rightMouseButtonDown(mouseRelativePosition[0],mouseRelativePosition[1]));
    }
    if (viewSelectionActive)
    { // Mouse went down on view selector
        setFocusObject(FOCUS_ON_VIEW_SELECTION_WINDOW);
        return(viewSelector->rightMouseButtonDown(mouseRelativePosition[0],mouseRelativePosition[1]));
    }
    if ( _hierarchyEnabled&&hierarchy->rightMouseDown(mouseRelativePosition[0]-offx,mouseRelativePosition[1]) )
    { // Mouse went down on hierarchy window
        setFocusObject(FOCUS_ON_HIERARCHY);
        return(true); // We want the mouse captured!
    }
    if (_hierarchyEnabled)
        offx+=_hierarchyWidth;
    if (App::ct->pageContainer->rightMouseButtonDown(mouseRelativePosition[0]-offx,mouseRelativePosition[1]-offy))
    { // Mouse went down on views window
        setFocusObject(App::ct->pageContainer->getFocusObject());
        return(true); // We want the mouse captured!
    }
    return(false); // Nothing caught that action
}
void COglSurface::rightMouseButtonUp(int x,int y,int absX,int absY,QWidget* mainWindow)
{
    int offx=0;
    int offy=0;
    mouseRelativePosition[0]=x;
    mouseRelativePosition[1]=y;
    mousePreviousRelativePosition[0]=mouseRelativePosition[0];
    mousePreviousRelativePosition[1]=mouseRelativePosition[1];
    if (CMiscBase::handleVerSpec_hasPopupDlg())
    {
        if (sceneSelectionActive&&(sceneSelector->getCaughtElements()&sim_right_button))
            sceneSelector->rightMouseButtonUp(mouseRelativePosition[0],mouseRelativePosition[1],absX,absY,mainWindow);
        if (pageSelectionActive&&(pageSelector->getCaughtElements()&sim_right_button))
            pageSelector->rightMouseButtonUp(mouseRelativePosition[0],mouseRelativePosition[1],absX,absY,mainWindow);
        if (viewSelectionActive&&(viewSelector->getCaughtElements()&sim_right_button))
            viewSelector->rightMouseButtonUp(mouseRelativePosition[0],mouseRelativePosition[1],absX,absY,mainWindow);
        if (_hierarchyEnabled&&(hierarchy->getCaughtElements()&sim_right_button))
            hierarchy->rightMouseUp(mouseRelativePosition[0]-offx,mouseRelativePosition[1],absX,absY,mainWindow);
        if (_hierarchyEnabled)
            offx+=_hierarchyWidth;
        if (App::ct->pageContainer->getCaughtElements()&sim_right_button)
            App::ct->pageContainer->rightMouseButtonUp(mouseRelativePosition[0]-offx,mouseRelativePosition[1]-offy,absX,absY,mainWindow);
    }
    sceneSelector->clearCaughtElements(0xffff-sim_right_button);
    pageSelector->clearCaughtElements(0xffff-sim_right_button);
    viewSelector->clearCaughtElements(0xffff-sim_right_button);
    hierarchy->clearCaughtElements(0xffff-sim_right_button);
    App::ct->pageContainer->clearCaughtElements(0xffff-sim_right_button);
}

bool COglSurface::middleMouseButtonDown(int x,int y)
{
    int offx=0;
    int offy=0;
    mouseRelativePosition[0]=x;
    mouseRelativePosition[1]=y;
    mousePreviousRelativePosition[0]=mouseRelativePosition[0];
    mousePreviousRelativePosition[1]=mouseRelativePosition[1];
    sceneSelector->clearCaughtElements(0xffff-sim_middle_button);
    pageSelector->clearCaughtElements(0xffff-sim_middle_button);
    viewSelector->clearCaughtElements(0xffff-sim_middle_button);
    hierarchy->clearCaughtElements(0xffff-sim_middle_button);
    App::ct->pageContainer->clearCaughtElements(0xffff-sim_middle_button);
    if (_hierarchyEnabled)
        offx+=_hierarchyWidth;
    if ( (!sceneSelectionActive)&&(!pageSelectionActive)&&(!viewSelectionActive) )
    {
        if (App::ct->pageContainer->middleMouseButtonDown(mouseRelativePosition[0]-offx,mouseRelativePosition[1]-offy))
        { // Mouse went down on views window
            setFocusObject(App::ct->pageContainer->getFocusObject());
            return(true); // We want the mouse captured!
        }
    }
    return(false); // Nothing caught that action
}

void COglSurface::middleMouseButtonUp(int x,int y)
{
    int offx=0;
    int offy=0;
    mouseRelativePosition[0]=x;
    mouseRelativePosition[1]=y;
    mousePreviousRelativePosition[0]=mouseRelativePosition[0];
    mousePreviousRelativePosition[1]=mouseRelativePosition[1];
    if (_hierarchyEnabled)
        offx+=_hierarchyWidth;
    if ( (!sceneSelectionActive)&&(!pageSelectionActive)&&(!viewSelectionActive) )
    {
        if (App::ct->pageContainer->getCaughtElements()&sim_middle_button)
            App::ct->pageContainer->middleMouseButtonUp(mouseRelativePosition[0]-offx,mouseRelativePosition[1]-offy);
    }
    sceneSelector->clearCaughtElements(0xffff-sim_middle_button);
    pageSelector->clearCaughtElements(0xffff-sim_middle_button);
    viewSelector->clearCaughtElements(0xffff-sim_middle_button);
    hierarchy->clearCaughtElements(0xffff-sim_middle_button);
    App::ct->pageContainer->clearCaughtElements(0xffff-sim_middle_button);
}

bool COglSurface::leftMouseButtonDoubleClick(int x,int y,int selectionStatus)
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    int offx=0;
    int offy=0;
    mouseRelativePosition[0]=x;
    mouseRelativePosition[1]=y;
    mousePreviousRelativePosition[0]=mouseRelativePosition[0];
    mousePreviousRelativePosition[1]=mouseRelativePosition[1];
    if ( _hierarchyEnabled&&(hierarchy->leftMouseDblClick(mouseRelativePosition[0]-offx,mouseRelativePosition[1],selectionStatus)))
    {
        setFocusObject(FOCUS_ON_HIERARCHY);
        return(true); // We inform that this action was processed
    }
    if (_hierarchyEnabled)
        offx+=_hierarchyWidth;
    if (App::ct->pageContainer->leftMouseButtonDoubleClick(mouseRelativePosition[0]-offx,mouseRelativePosition[1]-offy,selectionStatus))
    {
        setFocusObject(App::ct->pageContainer->getFocusObject());
        return(true); // We inform that this action was processed
    }
    return(false);  // Nothing caught that action
}
bool COglSurface::isScenePageOrViewSelectionActive()
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    return(sceneSelectionActive || pageSelectionActive || viewSelectionActive);
}


bool COglSurface::isSceneSelectionActive()
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    return(sceneSelectionActive);
}

bool COglSurface::isPageSelectionActive()
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    return(pageSelectionActive);
}

bool COglSurface::isViewSelectionActive()
{
    return(viewSelectionActive);
}

unsigned char* COglSurface::render(int currentCursor,int mouseButtonState,int mousePos[2],int* frameResol)
{
    FUNCTION_DEBUG;
    if (!_readyToRender)
        return(nullptr);

    if (pageSelectionActive)
        pageSelector->render();
    else if (sceneSelectionActive)
        sceneSelector->render();
    else if (viewSelectionActive)
        viewSelector->render();
    else
    {
        // We render from right to left for the hierarchy and object browser (since both will render larger on their left)
        bool hierarchyWasRendered=false;
        if (_hierarchyEnabled)
        {
            int b=0;
            hierarchyWasRendered=hierarchy->render();

            // We now display the hierarchy title:
            ogl::setMaterialColor(ogl::colorBlack,ogl::colorBlack,ogl::colorBlack);
            glViewport(surfacePosition[0],surfacePosition[1],surfaceSize[0],surfaceSize[1]);
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            glOrtho(0.0f,surfaceSize[0],0.0f,surfaceSize[1],-100.0f,100.0f);
            glMatrixMode (GL_MODELVIEW);
            glLoadIdentity();
            glDisable(GL_DEPTH_TEST);

            std::string hierarchyTitle("  Scene hierarchy");
            int t=App::getEditModeType();
            if (t&SHAPE_EDIT_MODE)
            {
                std::string objName("ERROR");
                CShape* theShape=App::mainWindow->editModeContainer->getEditModeShape();
                if (theShape!=nullptr)
                    objName=theShape->getObjectName();
                if (t&VERTEX_EDIT_MODE)
                    hierarchyTitle="  Vertices (";
                if (t&TRIANGLE_EDIT_MODE)
                    hierarchyTitle="  Triangles (";
                if (t&EDGE_EDIT_MODE)
                    hierarchyTitle="  Edges (";
                hierarchyTitle+=objName+")";
            }
            if (t&PATH_EDIT_MODE)
            {
                std::string objName("  ERROR");
                CPath* thePath=App::mainWindow->editModeContainer->getEditModePath();
                if (thePath!=nullptr)
                    objName=thePath->getObjectName();
                hierarchyTitle="  Control points (";
                hierarchyTitle+=objName+")";
            }
            if (t&BUTTON_EDIT_MODE)
                hierarchyTitle="  OpenGl-based custom UIs";

            float txtCol[3]={0.2f,0.2f,0.2f};
            float* bkgrndCol=ogl::TITLE_BAR_COLOR;
            VPoint size(_hierarchyWidth-BROWSER_HIERARCHY_MAIN_RENDERING_WINDOW_SEPARATION_WIDTH,BROWSER_HIERARCHY_TITLE_BAR_HEIGHT*App::sc);
            VPoint pos(b+size.x/2,surfaceSize[1]-size.y/2);
            int buttonAttrib=sim_buttonproperty_label|sim_buttonproperty_enabled|sim_buttonproperty_verticallycentered;
            ogl::drawButton(pos,size,txtCol,bkgrndCol,bkgrndCol,hierarchyTitle,buttonAttrib,false,0,0.0f,false,0,nullptr,nullptr,nullptr,nullptr,nullptr);
            pos.x=b+_hierarchyWidth+(-BROWSER_HIERARCHY_TITLE_BAR_CLOSING_BUTTON_WIDTH/2)*App::sc-BROWSER_HIERARCHY_MAIN_RENDERING_WINDOW_SEPARATION_WIDTH;
            size.x=BROWSER_HIERARCHY_TITLE_BAR_CLOSING_BUTTON_WIDTH*App::sc;
            float* bkgrndCol2=ogl::TITLE_BAR_BUTTON_COLOR;
            buttonAttrib=sim_buttonproperty_button|sim_buttonproperty_enabled|sim_buttonproperty_horizontallycentered|sim_buttonproperty_verticallycentered;
            if (_hierarchyClosingButtonDown)
                buttonAttrib|=sim_buttonproperty_isdown;
            ogl::drawButton(pos,size,txtCol,bkgrndCol2,bkgrndCol2,"&&Check",buttonAttrib,false,0,0.0f,false,0,nullptr,nullptr,nullptr,nullptr,nullptr);

            ogl::setMaterialColor(sim_colorcomponent_emission,ogl::SEPARATION_LINE_COLOR);
            glLineWidth(1.0f);
            ogl::drawSingle2dLine_i(b,surfaceSize[1]+(-BROWSER_HIERARCHY_TITLE_BAR_HEIGHT-1)*App::sc,b+_hierarchyWidth-BROWSER_HIERARCHY_MAIN_RENDERING_WINDOW_SEPARATION_WIDTH,surfaceSize[1]+(-BROWSER_HIERARCHY_TITLE_BAR_HEIGHT-1)*App::sc);
            glLineWidth(1.0f);

            glEnable(GL_DEPTH_TEST);
        }
        App::ct->pageContainer->renderCurrentPage(frameResol!=nullptr);
        // We now have to draw separations between the different parts:
        if (_hierarchyEnabled)
        {
            int b=0;
            ogl::setMaterialColor(ogl::colorBlack,ogl::colorBlack,ogl::colorBlack);
            glViewport(surfacePosition[0],surfacePosition[1],surfaceSize[0],surfaceSize[1]);
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            glOrtho(0.0f,surfaceSize[0],0.0f,surfaceSize[1],-100.0f,100.0f);
            glDisable(GL_DEPTH_TEST);
            ogl::setMaterialColor(sim_colorcomponent_emission,ogl::SEPARATION_LINE_COLOR);
            glLineWidth(2.0f);
            ogl::drawSingle2dLine_i(b+_hierarchyWidth-1,0,b+_hierarchyWidth-1,surfaceSize[1]);
            glLineWidth(1.0f);
            glEnable(GL_DEPTH_TEST);
        }
    }

    if ((App::mainWindow!=nullptr)&&App::mainWindow->simulationRecorder->getIsRecording()&&App::mainWindow->simulationRecorder->getShowCursor())
    { // OpenGL cursor:
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0,surfaceSize[0],0,surfaceSize[1],-1,1);
        glDisable(GL_SCISSOR_TEST); 
        glViewport(surfacePosition[0],surfacePosition[1],surfaceSize[0],surfaceSize[1]);
        glDisable(GL_DEPTH_TEST);
        int pc[2]={1,-1};

        if (currentCursor==sim_cursor_arrow)
        {
            App::ct->globalGuiTextureCont->startTextureDisplay(CURSOR_ARROW);
            pc[0]=0;
            pc[1]=-16;
        }
        if (currentCursor==sim_cursor_finger)
        {
            App::ct->globalGuiTextureCont->startTextureDisplay(CURSOR_FINGER);
            pc[0]=1;
            pc[1]=-15;
        }
        if (currentCursor==sim_cursor_all_directions)
            App::ct->globalGuiTextureCont->startTextureDisplay(CURSOR_ALL_DIR);
        if (currentCursor==sim_cursor_horizontal_directions)
            App::ct->globalGuiTextureCont->startTextureDisplay(CURSOR_HORIZONTAL_DIR);
        if (currentCursor==sim_cursor_vertical_directions)
            App::ct->globalGuiTextureCont->startTextureDisplay(CURSOR_VERTICAL_DIR);
        if (currentCursor==sim_cursor_slash_directions)
            App::ct->globalGuiTextureCont->startTextureDisplay(CURSOR_SLASH_DIR);
        if (currentCursor==sim_cursor_backslash_directions)
            App::ct->globalGuiTextureCont->startTextureDisplay(CURSOR_BACKSLASH_DIR);

        // Added following on 2011/01/26 to remove the annoying green borders that appear on some graphic cards:
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);

        glEnable(GL_ALPHA_TEST);
        glAlphaFunc(GL_GREATER,0.0f);
        ogl::disableLighting_useWithCare(); // only temporarily
        glColor3f(1.0f, 1.0f, 1.0f);
        glBegin(GL_QUADS);
        glTexCoord2f(0.0f,0.0f);
        glVertex3i(mousePos[0]+pc[0]-16,mousePos[1]+pc[1]-16,0);
        glTexCoord2f(1.0f,0.0f);
        glVertex3i(mousePos[0]+pc[0]+16,mousePos[1]+pc[1]-16,0);
        glTexCoord2f(1.0f,1.0f);
        glVertex3i(mousePos[0]+pc[0]+16,mousePos[1]+pc[1]+16,0);
        glTexCoord2f(0.0f,1.0f);
        glVertex3i(mousePos[0]+pc[0]-16,mousePos[1]+pc[1]+16,0);
        glEnd();
        App::ct->globalGuiTextureCont->endTextureDisplay();

        if ((App::mainWindow!=nullptr)&&App::mainWindow->simulationRecorder->getShowButtonStates())
        {       
            if (mouseButtonState&0x0f)
            {
                if (mouseButtonState&8)
                    App::ct->globalGuiTextureCont->startTextureDisplay(CURSOR_MIDDLE_BUTTON);
                else
                {
                    if (mouseButtonState&4)
                        App::ct->globalGuiTextureCont->startTextureDisplay(CURSOR_RIGHT_BUTTON);
                    else
                    {
                        if (mouseButtonState&1)
                            App::ct->globalGuiTextureCont->startTextureDisplay(CURSOR_LEFT_BUTTON);
                        else
                        {
                            if (mouseButtonState&2)
                                App::ct->globalGuiTextureCont->startTextureDisplay(CURSOR_WHEEL_BUTTON);
                        }
                    }
                }

                // Added following on 2011/01/26 to remove the annoying green borders that appear on some graphic cards:
                glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);

                int off[2]={+24,-16};
                glBegin(GL_QUADS);
                glTexCoord2f(0.0f,0.0f);
                glVertex3i(mousePos[0]-16+off[0],mousePos[1]-16+off[1],0);
                glTexCoord2f(1.0f,0.0f);
                glVertex3i(mousePos[0]+16+off[0],mousePos[1]-16+off[1],0);
                glTexCoord2f(1.0f,1.0f);
                glVertex3i(mousePos[0]+16+off[0],mousePos[1]+16+off[1],0);
                glTexCoord2f(0.0f,1.0f);
                glVertex3i(mousePos[0]-16+off[0],mousePos[1]+16+off[1],0);
                glEnd();
                App::ct->globalGuiTextureCont->endTextureDisplay();

                if ((App::mainWindow!=nullptr)&&(App::mainWindow->getKeyDownState()&2))
                {
                    App::ct->globalGuiTextureCont->startTextureDisplay(CURSOR_SHIFT_BUTTON);

                    // Added following on 2011/01/26 to remove the annoying green borders that appear on some graphic cards:
                    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
                    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);

                    int off[2]={+24+24,-16};
                    glBegin(GL_QUADS);
                    glTexCoord2f(0.0f,0.0f);
                    glVertex3i(mousePos[0]-16+off[0],mousePos[1]-16+off[1],0);
                    glTexCoord2f(1.0f,0.0f);
                    glVertex3i(mousePos[0]+16+off[0],mousePos[1]-16+off[1],0);
                    glTexCoord2f(1.0f,1.0f);
                    glVertex3i(mousePos[0]+16+off[0],mousePos[1]+16+off[1],0);
                    glTexCoord2f(0.0f,1.0f);
                    glVertex3i(mousePos[0]-16+off[0],mousePos[1]+16+off[1],0);
                    glEnd();
                    App::ct->globalGuiTextureCont->endTextureDisplay();
                }
                if ((App::mainWindow!=nullptr)&&(App::mainWindow->getKeyDownState()&1))
                {
                    App::ct->globalGuiTextureCont->startTextureDisplay(CURSOR_CTRL_BUTTON);

                    // Added following on 2011/01/26 to remove the annoying green borders that appear on some graphic cards:
                    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
                    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);

                    int off[2]={+24+24,-16};
                    glBegin(GL_QUADS);
                    glTexCoord2f(0.0f,0.0f);
                    glVertex3i(mousePos[0]-16+off[0],mousePos[1]-16+off[1],0);
                    glTexCoord2f(1.0f,0.0f);
                    glVertex3i(mousePos[0]+16+off[0],mousePos[1]-16+off[1],0);
                    glTexCoord2f(1.0f,1.0f);
                    glVertex3i(mousePos[0]+16+off[0],mousePos[1]+16+off[1],0);
                    glTexCoord2f(0.0f,1.0f);
                    glVertex3i(mousePos[0]-16+off[0],mousePos[1]+16+off[1],0);
                    glEnd();
                    App::ct->globalGuiTextureCont->endTextureDisplay();
                }
            }
        }
        ogl::enableLighting_useWithCare();
        ogl::setBlending(false);
        glDisable(GL_ALPHA_TEST);

        glEnable(GL_DEPTH_TEST);
        int b=0;
        if ( (_hierarchyEnabled)&&(mousePos[0]<=b+_hierarchyWidth+6)&&(mousePos[0]>=b-36) ) // 36 because the cursor can be very wide (when the mouse button states are displayed)
            App::setRefreshHierarchyViewFlag();
    }

//*************************************************
// Following allows to hand the full rendered frame to a plugin, that might modify it and return it!
//*************************************************
    static int prevResX=-1;
    static int prevResY=-1;
    static char* buff=nullptr;
    if (CPluginContainer::shouldSend_openglframe_msg())
    {
        if ((prevResX!=surfaceSize[0])||(prevResY!=surfaceSize[1]))
        {
            delete[] buff;
            buff=new char[surfaceSize[0]*surfaceSize[1]*3];
        }
        glPixelStorei(GL_PACK_ALIGNMENT,1);
        glReadPixels(0, 0, surfaceSize[0], surfaceSize[1], GL_RGB, GL_UNSIGNED_BYTE, buff);
        glPixelStorei(GL_PACK_ALIGNMENT,4); // important to restore! Really?

        int auxVals[4];
        int retVals[4];
        auxVals[0]=surfaceSize[0];
        auxVals[1]=surfaceSize[1];
        auxVals[2]=0;
        auxVals[3]=0;
        CPluginContainer::sendSpecialEventCallbackMessageToSomePlugins(sim_message_eventcallback_openglframe,auxVals,buff,retVals);
        if (auxVals[3]!=0)
        { // we want to apply a new image!
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            glOrtho(0,surfaceSize[0],0,surfaceSize[1],-100,100);
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity ();
            glDisable(GL_DEPTH_TEST);
            ogl::setMaterialColor(sim_colorcomponent_emission,ogl::colorWhite);

            GLuint _oglTextureName;
            glGenTextures(1,&_oglTextureName);
            glBindTexture(GL_TEXTURE_2D,_oglTextureName);
            glPixelStorei(GL_UNPACK_ALIGNMENT,1);
            glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,surfaceSize[0],surfaceSize[1],0,GL_RGB,GL_UNSIGNED_BYTE,buff);
            glPixelStorei(GL_UNPACK_ALIGNMENT,4); // important to restore! Really?
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); // keep to GL_LINEAR here!!
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
            glTexParameteri (GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
            glTexParameteri (GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
            glTexEnvi (GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_DECAL);
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D,_oglTextureName);

            glColor3f(1.0f,1.0f,1.0f);

            glBegin(GL_QUADS);
            glTexCoord2f(0.0f,0.0f);
            glVertex3i(0,0,0);
            glTexCoord2f(0.0f,1.0f);
            glVertex3i(0,surfaceSize[1],0);
            glTexCoord2f(1.0f,1.0f);
            glVertex3i(surfaceSize[0],surfaceSize[1],0);
            glTexCoord2f(1.0f,0.0f);
            glVertex3i(surfaceSize[0],0,0);
            glEnd();

            glDisable(GL_TEXTURE_2D);
            glDeleteTextures(1,&_oglTextureName);
            glEnable(GL_DEPTH_TEST);
        }
    }
    else
    {
        delete[] buff;
        buff=nullptr;
        prevResX=-1;
    }
//*************************************************

    if (frameResol!=nullptr)
    {
        frameResol[0]=surfaceSize[0];
        frameResol[1]=surfaceSize[1];
        unsigned char* img=new unsigned char[frameResol[0]*frameResol[1]*3];
        glPixelStorei(GL_PACK_ALIGNMENT,1);
        glReadPixels(0,0,surfaceSize[0],surfaceSize[1],GL_RGB,GL_UNSIGNED_BYTE,img);
        glPixelStorei(GL_PACK_ALIGNMENT,4); // important to restore! Really?
        return(img);
    }
    return(nullptr);
}

void COglSurface::setUpDefaultValues()
{
    _hierarchyClosingButtonDown=false;
    _hierarchyResizingAction=false;
    surfaceSize[0]=2000;
    surfaceSize[1]=2000;
    surfacePosition[0]=0;
    surfacePosition[1]=0;
    _readyToRender=false;

    viewSelectionActive=false;
    pageSelectionActive=false;
    sceneSelectionActive=false;
    focusObject=FOCUS_ON_PAGE;
    hierarchy->setUpDefaultValues();
    _caughtElements=0;

    actualizeAllSurfacesSizeAndPosition();  // Important because the newly created views
                                            // don't have their position/size set!
    App::setToolbarRefreshFlag();
}

int COglSurface::getCaughtElements()
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    int retVal=0;
    if (sceneSelectionActive)
        retVal|=sceneSelector->getCaughtElements();
    if (pageSelectionActive)
        retVal|=pageSelector->getCaughtElements();
    if (viewSelectionActive)
        retVal|=viewSelector->getCaughtElements();
    if (_hierarchyEnabled)
        retVal|=hierarchy->getCaughtElements();
    retVal|=App::ct->pageContainer->getCaughtElements();
    retVal|=_caughtElements;
    return(retVal);
}

void COglSurface::clearCaughtElements(int keepMask)
{
    sceneSelector->clearCaughtElements(keepMask);
    pageSelector->clearCaughtElements(keepMask);
    viewSelector->clearCaughtElements(keepMask);
    hierarchy->clearCaughtElements(keepMask);
    App::ct->pageContainer->clearCaughtElements(keepMask);
    _caughtElements&=keepMask;
}

void COglSurface::setFocusObject(int obj)
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    focusObject=obj;
    if (obj==FOCUS_ON_PAGE)
    {
        setSceneSelectionActive(false);
        setPageSelectionActive(false);
        setViewSelectionActive(false);
        hierarchy->looseFocus();
        App::ct->pageContainer->setFocusObject(obj);
    }
    if (obj==FOCUS_ON_BROWSER)
    {
        setSceneSelectionActive(false);
        setPageSelectionActive(false);
        setViewSelectionActive(false);
        hierarchy->looseFocus();
        App::ct->pageContainer->looseFocus();
    }
    if (obj==FOCUS_ON_HIERARCHY)
    {
        setSceneSelectionActive(false);
        setPageSelectionActive(false);
        setViewSelectionActive(false);
        App::ct->pageContainer->looseFocus();
    }
    if (obj==FOCUS_ON_SCENE_SELECTION_WINDOW)
    {
        setPageSelectionActive(false);
        setViewSelectionActive(false);
        hierarchy->looseFocus();
        App::ct->pageContainer->looseFocus();
    }
    if (obj==FOCUS_ON_PAGE_SELECTION_WINDOW)
    {
        setSceneSelectionActive(false);
        setViewSelectionActive(false);
        hierarchy->looseFocus();
        App::ct->pageContainer->looseFocus();
    }
    if (obj==FOCUS_ON_VIEW_SELECTION_WINDOW)
    {
        setSceneSelectionActive(false);
        setPageSelectionActive(false);
        hierarchy->looseFocus();
        App::ct->pageContainer->looseFocus();
    }
    if (obj==FOCUS_ON_SOFT_DIALOG)
    {
        setSceneSelectionActive(false);
        setPageSelectionActive(false);
        setViewSelectionActive(false);
        hierarchy->looseFocus();
        App::ct->pageContainer->setFocusObject(obj);
    }
}
int COglSurface::getFocusObject()
{
    return(focusObject);
}

void COglSurface::setHierarchyEnabled(bool isEnabled)
{
    _hierarchyEnabled=isEnabled;
    if (!isEnabled)
        setFocusObject(FOCUS_ON_PAGE);
    actualizeAllSurfacesSizeAndPosition();
    App::setToolbarRefreshFlag();
}


void COglSurface::setSceneSelectionActive(bool isActive)
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    if (sceneSelectionActive!=isActive)
    {
        sceneSelectionActive=isActive;
        if (isActive)
        {
            App::mainWindow->closeTemporarilyDialogsForSceneSelector();
            setFocusObject(FOCUS_ON_SCENE_SELECTION_WINDOW);
            sceneSelector->markAsFirstRender();
        }
        else
            App::mainWindow->reopenTemporarilyClosedDialogsForSceneSelector();

        actualizeAllSurfacesSizeAndPosition();
        App::setToolbarRefreshFlag();

        SUIThreadCommand cmdIn;
        SUIThreadCommand cmdOut;
        cmdIn.cmdId=CREATE_DEFAULT_MENU_BAR_UITHREADCMD;
        App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
    }
}

void COglSurface::setPageSelectionActive(bool isActive)
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    if (pageSelectionActive!=isActive)
    {
        pageSelectionActive=isActive;
        if (isActive)
        {
            App::mainWindow->closeTemporarilyDialogsForPageSelector();
            setFocusObject(FOCUS_ON_PAGE_SELECTION_WINDOW);
        }
        else
            App::mainWindow->reopenTemporarilyClosedDialogsForPageSelector();

        actualizeAllSurfacesSizeAndPosition();
        App::setToolbarRefreshFlag();

        SUIThreadCommand cmdIn;
        SUIThreadCommand cmdOut;
        cmdIn.cmdId=CREATE_DEFAULT_MENU_BAR_UITHREADCMD;
        App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
    }
}

void COglSurface::setViewSelectionActive(bool isActive)
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    if (viewSelectionActive!=isActive)
    {
        viewSelectionActive=isActive;
        if (isActive)
        {
            App::mainWindow->closeTemporarilyDialogsForViewSelector();
            setFocusObject(FOCUS_ON_VIEW_SELECTION_WINDOW);
        }
        else
            App::mainWindow->reopenTemporarilyClosedDialogsForViewSelector();

        actualizeAllSurfacesSizeAndPosition();
        App::setToolbarRefreshFlag();

        SUIThreadCommand cmdIn;
        SUIThreadCommand cmdOut;
        cmdIn.cmdId=CREATE_DEFAULT_MENU_BAR_UITHREADCMD;
        App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
    }
}

void COglSurface::setHierarchySurfaceSizeAndPosition()
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    FUNCTION_DEBUG;
    int b=0;
    if (_hierarchyEnabled)
    {
        if (_hierarchyWidth>surfaceSize[0]-b)
            _hierarchyWidth=surfaceSize[0]-b;
        if (_hierarchyWidth<_hierarchyMinWidth)
            _hierarchyWidth=_hierarchyMinWidth;
        hierarchy->setRenderingSizeAndPosition(_hierarchyWidth-BROWSER_HIERARCHY_MAIN_RENDERING_WINDOW_SEPARATION_WIDTH+1,surfaceSize[1]+(-BROWSER_HIERARCHY_TITLE_BAR_HEIGHT-1-1)*App::sc,surfacePosition[0]+b,surfacePosition[1]);
    }
}

void COglSurface::setViewSurfaceSizeAndPosition()
{
    int h=0;
    int b=0;
    if (_hierarchyEnabled)
        h=_hierarchyWidth;
    App::ct->pageContainer->setPageSizeAndPosition(surfaceSize[0]-h-b,surfaceSize[1],surfacePosition[0]+h+b,surfacePosition[1]);
}

void COglSurface::actualizeAllSurfacesSizeAndPosition()
{
    FUNCTION_DEBUG;
    setHierarchySurfaceSizeAndPosition();
    setViewSurfaceSizeAndPosition();
    sceneSelector->setViewSizeAndPosition(surfaceSize[0],surfaceSize[1],surfacePosition[0],surfacePosition[1]);
    pageSelector->setViewSizeAndPosition(surfaceSize[0],surfaceSize[1],surfacePosition[0],surfacePosition[1]);
    viewSelector->setViewSizeAndPosition(surfaceSize[0],surfaceSize[1],surfacePosition[0],surfacePosition[1]);
}

bool COglSurface::isHierarchyEnabled()
{
    return(_hierarchyEnabled);
}

void COglSurface::keyPress(int key,QWidget* mainWindow)
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    if (key==ESC_KEY)
        App::ct->pageContainer->clearAllLastMouseDownViewIndex();

    if (focusObject==FOCUS_ON_HIERARCHY)
    {
        hierarchy->keyPress(key);
        return;
    }
    if ((focusObject==FOCUS_ON_PAGE)||(focusObject==FOCUS_ON_SOFT_DIALOG))
    {
        App::ct->pageContainer->keyPress(key,mainWindow);
        return;
    }

    if (focusObject==FOCUS_ON_SCENE_SELECTION_WINDOW)
    {
        sceneSelector->keyPress(key);
        return;
    }
    if (focusObject==FOCUS_ON_PAGE_SELECTION_WINDOW)
    {
        pageSelector->keyPress(key);
        return;
    }
    if (focusObject==FOCUS_ON_VIEW_SELECTION_WINDOW)
    {
        viewSelector->keyPress(key);
        return;
    }
}

void COglSurface::startViewSelection(int objectType,int subViewIndex)
{
    viewSelector->setViewSelectionInfo(objectType,App::ct->pageContainer->getActivePageIndex(),subViewIndex);
    setViewSelectionActive(true);
}

int COglSurface::getHierarchyWidth()
{
    return(_hierarchyWidth);
}
void COglSurface::setHierarchyWidth(int w)
{
    _hierarchyWidth=w;
    actualizeAllSurfacesSizeAndPosition();
}
