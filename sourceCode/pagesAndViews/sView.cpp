
// This file requires some serious refactoring!

#include "vrepMainHeader.h"
#include "funcDebug.h"
#include "v_rep_internal.h"
#include "sView.h"
#include "tt.h"
#include "simulation.h"
#include "sceneObjectOperations.h"
#include "simulation.h"
#include "addOperations.h"
#include "v_repStrings.h"
#include "vDateTime.h"
#include "proxSensorRoutine.h"
#include "pluginContainer.h"
#include "app.h"
#include "viewRendering.h"
#ifdef SIM_WITH_GUI
#include "oglSurface.h"
#endif

CSView::CSView(int theLinkedObjectID)
{
    setDefaultValues();
    linkedObjectID=theLinkedObjectID;
    static int nextUniqueID=10000;
    _uniqueID=nextUniqueID++;
    _initialValuesInitialized=false;
}

CSView::~CSView()
{

}
void CSView::initializeInitialValues(bool simulationIsRunning)
{ // is called at simulation start, but also after object(s) have been copied into a scene!
    _initialValuesInitialized=simulationIsRunning;
    if (simulationIsRunning)
    {
        _initialPerspectiveDisplay=perspectiveDisplay;
        _initialShowEdges=_showEdges;
        _initialThickEdges=_thickEdges;
        _initialVisualizeOnlyInertias=_visualizeOnlyInertias;
        _initialRenderingMode=_renderingMode;
        _initialGraphIsTimeGraph=graphIsTimeGraph;
    }
}

bool CSView::simulationEnded()
{ // Remember, this is not guaranteed to be run! (the object can be copied during simulation, and pasted after it ended). For thoses situations there is the initializeInitialValues routine!
    if (_initialValuesInitialized&&App::ct->simulation->getResetSceneAtSimulationEnd())
    {
        perspectiveDisplay=_initialPerspectiveDisplay;
        _showEdges=_initialShowEdges;
        _thickEdges=_initialThickEdges;
        _visualizeOnlyInertias=_initialVisualizeOnlyInertias;
        _renderingMode=_initialRenderingMode;
        graphIsTimeGraph=_initialGraphIsTimeGraph;
    }
    _initialValuesInitialized=false;
    return(_removeFloatingViewAtSimulationEnd);
}

int CSView::getUniqueID() const
{
    return(_uniqueID);
}

void CSView::setDefaultValues()
{
    dontActivatePopup=false;
    linkedObjectID=-1;
    perspectiveDisplay=true;
    _showEdges=true;
    _thickEdges=false;
    _visualizeOnlyInertias=false;
    _removeFloatingViewAtSimulationEnd=false;
    _doNotSaveFloatingView=false;
    _renderingMode=RENDERING_MODE_SOLID;
    _xyGraphInAutoModeDuringSimulation=true;
    _timeGraphXInAutoModeDuringSimulation=true;
    _timeGraphYInAutoModeDuringSimulation=true;
    _xyGraphIsOneOneProportional=false;
    _canSwapViewWithMainView=true;
    _canBeClosed=true;
    _canBeShifted=true;
    _canBeResized=true;

    _fitSceneToView=false;
    _fitSelectionToView=false;

    graphPosition[0]=-0.5f;
    graphPosition[1]=-0.5f;
    graphSize[0]=1.0f;
    graphSize[1]=1.0f;
    _relativeViewPositionOffset=0.0f;
    _trackedCurveIndex=-1;

    rightButtonIsCaught=false;
    mouseIsDown=false;
    mouseJustWentDownWasProcessed=true;
    mouseJustWentDownFlag=false;
    mouseJustWentUpFlag=false;
    selectionStatus=NOSELECTION;
    _caughtElements=0;  
    dontActivatePopup=false;
    linkedObjectID=-1;
    graphIsTimeGraph=true;
    graphPosition[0]=-0.5f;
    graphPosition[1]=-0.5f;
    graphSize[0]=1.0f;
    graphSize[1]=1.0f;
    _relativeViewPositionOffset=0.0f;

    rightButtonIsCaught=false;
    mouseIsDown=false;
    mouseJustWentDownWasProcessed=true;
    mouseJustWentDownFlag=false;
    mouseJustWentUpFlag=false;
    selectionStatus=NOSELECTION;
    _caughtElements=0;
}

void CSView::setCenterPosition(float pos[3])
{
    centerPosition[0]=pos[0];
    centerPosition[1]=pos[1];
    centerPosition[2]=pos[2];
}
void CSView::getCenterPosition(float pos[3]) const
{
    pos[0]=centerPosition[0];
    pos[1]=centerPosition[1];
    pos[2]=centerPosition[2];
}

void CSView::setViewSizeAndPosition(int sizeX,int sizeY,int posX,int posY)
{
    _viewSize[0]=sizeX;
    _viewSize[1]=sizeY;
    _viewPosition[0]=posX;
    _viewPosition[1]=posY;
}

void CSView::getViewSize(int size[2]) const
{
    size[0]=_viewSize[0];
    size[1]=_viewSize[1];
}

void CSView::getViewPosition(int pos[2]) const
{
    pos[0]=_viewPosition[0];
    pos[1]=_viewPosition[1];
}

int CSView::getLinkedObjectID() const
{
    return(linkedObjectID);
}
void CSView::setLinkedObjectID(int theNewLinkedObjectID,bool noDefaultValues)
{
    if (!noDefaultValues)
        setDefaultValues();
    linkedObjectID=theNewLinkedObjectID;
}

void CSView::setAlternativeViewName(const char* name)
{
    _alternativeViewName=name;
}

std::string CSView::getAlternativeViewName() const
{
    return(_alternativeViewName);
}

void CSView::setPerspectiveDisplay(bool perspective)
{
    perspectiveDisplay=perspective;
}

bool CSView::getPerspectiveDisplay() const
{
    return(perspectiveDisplay);
}

void CSView::setShowEdges(bool edges)
{
    _showEdges=edges;
}

bool CSView::getShowEdges() const
{
    return(_showEdges);
}

void CSView::setThickEdges(bool thickEdges)
{
    _thickEdges=thickEdges;
}

bool CSView::getThickEdges() const
{
    return(_thickEdges);
}

void CSView::setVisualizeOnlyInertias(bool inertiasOnly)
{
    _visualizeOnlyInertias=inertiasOnly;
}

bool CSView::getVisualizeOnlyInertias() const
{
    return(_visualizeOnlyInertias);
}

void CSView::setRenderingMode(int mode)
{
    _renderingMode=mode;
}

void CSView::setRemoveFloatingViewAtSimulationEnd(bool remove)
{

    _removeFloatingViewAtSimulationEnd=remove;
}

bool CSView::getRemoveFloatingViewAtSimulationEnd() const
{
    return(_removeFloatingViewAtSimulationEnd);
}

void CSView::setDoNotSaveFloatingView(bool doNotSave)
{
    _doNotSaveFloatingView=doNotSave;
}
bool CSView::getDoNotSaveFloatingView() const
{
    return(_doNotSaveFloatingView);
}


int CSView::getRenderingMode() const
{
    return(_renderingMode);
}

void CSView::setXYGraphAutoModeDuringSimulation(bool autoMode)
{
    _xyGraphInAutoModeDuringSimulation=autoMode;
}

bool CSView::getXYGraphAutoModeDuringSimulation() const
{
    return(_xyGraphInAutoModeDuringSimulation);
}

void CSView::setTimeGraphXAutoModeDuringSimulation(bool autoMode)
{
    _timeGraphXInAutoModeDuringSimulation=autoMode;
}

bool CSView::getTimeGraphXAutoModeDuringSimulation() const
{
    return(_timeGraphXInAutoModeDuringSimulation);
}

void CSView::setTimeGraphYAutoModeDuringSimulation(bool autoMode)
{
    _timeGraphYInAutoModeDuringSimulation=autoMode;
}

bool CSView::getTimeGraphYAutoModeDuringSimulation() const
{
    return(_timeGraphYInAutoModeDuringSimulation);
}

void CSView::setXYGraphIsOneOneProportional(bool autoMode)
{
    _xyGraphIsOneOneProportional=autoMode;
}

bool CSView::getXYGraphIsOneOneProportional() const
{
    return(_xyGraphIsOneOneProportional);
}

void CSView::setTimeGraph(bool timeGraph)
{
    graphIsTimeGraph=timeGraph;
}

bool CSView::getTimeGraph() const
{
    return(graphIsTimeGraph);
}

void CSView::setCanSwapViewWithMainView(bool canDoIt)
{
    _canSwapViewWithMainView=canDoIt;
}

bool CSView::getCanSwapViewWithMainView() const
{
    return(_canSwapViewWithMainView);
}

void CSView::setCanBeClosed(bool canDoIt)
{
    _canBeClosed=canDoIt;
}

bool CSView::getCanBeClosed() const
{
    return(_canBeClosed);
}

void CSView::setCanBeShifted(bool canDoIt)
{
    _canBeShifted=canDoIt;
}

bool CSView::getCanBeShifted() const
{
    return(_canBeShifted);
}

void CSView::setCanBeResized(bool canDoIt)
{
    _canBeResized=canDoIt;
}

bool CSView::getCanBeResized() const
{
    return(_canBeResized);
}

void CSView::setGraphPosition(float x,float y)
{
    graphPosition[0]=x;
    graphPosition[1]=y;
}

void CSView::getGraphPosition(float position[2]) const
{
    position[0]=graphPosition[0];
    position[1]=graphPosition[1];
}

void CSView::setGraphSize(float x,float y)
{
    graphSize[0]=x;
    graphSize[1]=y;
}

void CSView::getGraphSize(float size[2]) const
{
    size[0]=graphSize[0];
    size[1]=graphSize[1];
}

bool CSView::announceObjectWillBeErased(int objectID)
{ // Never called from copy buffer!
    if (objectID==linkedObjectID)
    {
        linkedObjectID=-1;
        return(true);
    }
    return(false);
}

void CSView::performObjectLoadingMapping(std::vector<int>* map)
{
    linkedObjectID=App::ct->objCont->getLoadingMapping(map,linkedObjectID);
}

void CSView::setTrackedGraphCurveIndex(int index)
{
    _trackedCurveIndex=index;
}

int CSView::getTrackedGraphCurveIndex() const
{
    return(_trackedCurveIndex);
}

void CSView::setFitViewToScene(bool doIt)
{
    _fitSceneToView=doIt;
    if (doIt)
        _fitSelectionToView=false;
}

bool CSView::getFitViewToScene() const
{
    return(_fitSceneToView);
}

void CSView::setFitViewToSelection(bool doIt)
{
    _fitSelectionToView=doIt;
    if (doIt)
        _fitSceneToView=false;
}

bool CSView::getFitViewToSelection() const
{
    return(_fitSelectionToView);
}


bool CSView::processCommand(int commandID,int subViewIndex)
{ // Return value is true if the command belonged to hierarchy menu and was executed
    if (commandID==VIEW_FUNCTIONS_TOGGLE_SOLID_DISPLAY_VFCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            if (_renderingMode==RENDERING_MODE_SOLID)
            {
                _renderingMode=RENDERING_MODE_WIREFRAME_TRIANGLES;
                App::addStatusbarMessage(IDSNS_NOW_IN_WIREFRAME_RENDERING_MODE);
            }
            else
            {
                _renderingMode=RENDERING_MODE_SOLID;
                App::addStatusbarMessage(IDSNS_NOW_IN_SOLID_RENDERING_MODE);
            }
            POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            cmd.intParams.push_back(subViewIndex);
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }

    if (commandID==VIEW_FUNCTIONS_PERSPECTIVE_DISPLAY_VFCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            perspectiveDisplay=!perspectiveDisplay;
            POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
            if (perspectiveDisplay)
                App::addStatusbarMessage(IDSNS_NOW_IN_PERSPECTIVE_PROJECTION_MODE);
            else
                App::addStatusbarMessage(IDSNS_NOW_IN_ORTHOGRAPHIC_PROJECTION_MODE);
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            cmd.intParams.push_back(subViewIndex);
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }
    if (commandID==VIEW_FUNCTIONS_SHOW_EDGES_VFCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            _showEdges=!_showEdges;
            POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
            if (_showEdges)
                App::addStatusbarMessage(IDSNS_NOW_SHOWING_EDGES);
            else
                App::addStatusbarMessage(IDSNS_NOW_HIDING_EDGES);
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            cmd.intParams.push_back(subViewIndex);
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }
    if (commandID==VIEW_FUNCTIONS_THICK_EDGES_VFCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            _thickEdges=!_thickEdges;
            POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
            if (_thickEdges)
                App::addStatusbarMessage(IDSNS_EDGES_ARE_NOW_THICK);
            else
                App::addStatusbarMessage(IDSNS_EDGES_ARE_NOW_THIN);
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            cmd.intParams.push_back(subViewIndex);
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }
    if (commandID==VIEW_FUNCTIONS_TEXTURED_DISPLAY_VFCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            App::ct->environment->setShapeTexturesEnabled(!App::ct->environment->getShapeTexturesEnabled());
            POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
            if (App::ct->environment->getShapeTexturesEnabled())
                App::addStatusbarMessage(IDSNS_SHAPE_TEXTURES_ENABLED);
            else
                App::addStatusbarMessage(IDSNS_SHAPE_TEXTURES_DISABLED);
            App::setFullDialogRefreshFlag(); // so that env. dlg gets refreshed
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            cmd.intParams.push_back(subViewIndex);
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }

    if (commandID==VIEW_FUNCTIONS_SHOW_INERTIAS_VFCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            _visualizeOnlyInertias=!_visualizeOnlyInertias;
            POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
            if (_visualizeOnlyInertias)
                App::addStatusbarMessage(IDSNS_SHOWING_INERTIAS);
            else
                App::addStatusbarMessage(IDSNS_NOT_SHOWING_INERTIAS);
            App::setFullDialogRefreshFlag(); // so that env. dlg gets refreshed
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            cmd.intParams.push_back(subViewIndex);
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }


    if (commandID==VIEW_FUNCTIONS_XY_GRAPH_AUTO_MODE_DURING_SIMULATION_VFCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            _xyGraphInAutoModeDuringSimulation=!_xyGraphInAutoModeDuringSimulation;
            POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
            if (_xyGraphInAutoModeDuringSimulation)
                App::addStatusbarMessage(IDSNS_NOW_IN_AUTO_MODE);
            else
                App::addStatusbarMessage(IDSNS_AUTO_MODE_DISABLED);
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            cmd.intParams.push_back(subViewIndex);
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }
    if (commandID==VIEW_FUNCTIONS_TIME_GRAPH_X_AUTO_MODE_DURING_SIMULATION_VFCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            _timeGraphXInAutoModeDuringSimulation=!_timeGraphXInAutoModeDuringSimulation;
            POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
            if (_timeGraphXInAutoModeDuringSimulation)
                App::addStatusbarMessage(IDSNS_NOW_IN_AUTO_MODE);
            else
                App::addStatusbarMessage(IDSNS_AUTO_MODE_DISABLED);
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            cmd.intParams.push_back(subViewIndex);
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }
    if (commandID==VIEW_FUNCTIONS_TIME_GRAPH_Y_AUTO_MODE_DURING_SIMULATION_VFCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            _timeGraphYInAutoModeDuringSimulation=!_timeGraphYInAutoModeDuringSimulation;
            POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
            if (_timeGraphYInAutoModeDuringSimulation)
                App::addStatusbarMessage(IDSNS_NOW_IN_AUTO_MODE);
            else
                App::addStatusbarMessage(IDSNS_AUTO_MODE_DISABLED);
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            cmd.intParams.push_back(subViewIndex);
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }
    if (commandID==VIEW_FUNCTIONS_XY_GRAPH_KEEP_PROPORTIONS_AT_ONE_ONE_VFCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            _xyGraphIsOneOneProportional=!_xyGraphIsOneOneProportional;
            POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
            if (_xyGraphIsOneOneProportional)
                App::addStatusbarMessage(IDSNS_KEEPING_PROPORTIONS_AT_1_1);
            else
                App::addStatusbarMessage(IDSNS_PROPORTIONS_NOT_CONSTRAINED_ANYMORE);
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            cmd.intParams.push_back(subViewIndex);
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }
    if (commandID==VIEW_FUNCTIONS_XY_GRAPH_DISPLAY_VFCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            graphIsTimeGraph=!graphIsTimeGraph;
            POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
            if (graphIsTimeGraph)
                App::addStatusbarMessage(IDSNS_NOW_IN_TIME_GRAPH_MODE);
            else
                App::addStatusbarMessage(IDSNS_NOW_IN_XY_GRAPH_MODE);
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            cmd.intParams.push_back(subViewIndex);
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }

    if (commandID==VIEW_FUNCTIONS_LOOK_THROUGH_CAMERA_VFCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            std::vector<int> sel;
            for (int i=0;i<App::ct->objCont->getSelSize();i++)
                sel.push_back(App::ct->objCont->getSelID(i));
            if ((sel.size()==1)&&(App::ct->objCont->getCamera(sel[0])!=nullptr))
            {
                setDefaultValues();
                linkedObjectID=sel[0];
                POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
                App::addStatusbarMessage(IDSNS_NOW_LOOKING_THROUGH_SELECTED_CAMERA);
            }
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            cmd.intParams.push_back(subViewIndex);
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }
    /*
    if (commandID==VIEW_FUNCTIONS_LOOK_AT_GRAPH_VFCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            std::vector<int> sel;
            for (int i=0;i<App::ct->objCont->getSelSize();i++)
                sel.push_back(App::ct->objCont->getSelID(i));
            if ((sel.size()==1)&&(App::ct->objCont->getGraph(sel[0])!=nullptr))
            {
                setDefaultValues();
                linkedObjectID=sel[0];
                POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
                App::addStatusbarMessage(IDSNS_NOW_LOOKING_AT_SELECTED_GRAPH);
            }
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            cmd.intParams.push_back(subViewIndex);
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }
    */
    if (commandID==VIEW_FUNCTIONS_LOOK_AT_VISION_SENSOR_VFCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            std::vector<int> sel;
            for (int i=0;i<App::ct->objCont->getSelSize();i++)
                sel.push_back(App::ct->objCont->getSelID(i));
            if ((sel.size()==1)&&(App::ct->objCont->getVisionSensor(sel[0])!=nullptr))
            {
                setDefaultValues();
                linkedObjectID=sel[0];
                POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
                App::addStatusbarMessage(IDSNS_NOW_LOOKING_AT_SELECTED_VISION_SENSOR);
            }
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            cmd.intParams.push_back(subViewIndex);
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }
    if (commandID==VIEW_FUNCTIONS_TRACK_OBJECT_VFCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            std::vector<int> sel;
            for (int i=0;i<App::ct->objCont->getSelSize();i++)
                sel.push_back(App::ct->objCont->getSelID(i));
            CCamera* camera=App::ct->objCont->getCamera(linkedObjectID);
            if ((camera!=nullptr)&&(sel.size()==1))
            {
                camera->setTrackedObjectID(sel[0]);
                POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
                App::addStatusbarMessage(IDSNS_CAMERA_NOW_TRACKING_SELECTED_OBJECT);
            }
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            cmd.intParams.push_back(subViewIndex);
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }
    if (commandID==VIEW_FUNCTIONS_DONT_TRACK_OBJECT_VFCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            CCamera* camera=App::ct->objCont->getCamera(linkedObjectID);
            if (camera!=nullptr)
            {
                camera->setTrackedObjectID(-1);
                POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
                App::addStatusbarMessage(IDSNS_CAMERA_NOW_NOT_TRACKING_ANY_OBJECT);
            }
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            cmd.intParams.push_back(subViewIndex);
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }
    return(false);
}

int CSView::getSelectionStatus() const
{
    return(selectionStatus);
}

CSView* CSView::copyYourself()
{
    CSView* newView=new CSView(linkedObjectID);
    newView->setDefaultValues();
    newView->linkedObjectID=linkedObjectID;
    newView->perspectiveDisplay=perspectiveDisplay;

    newView->_showEdges=_showEdges;
    newView->_thickEdges=_thickEdges;
    newView->_visualizeOnlyInertias=_visualizeOnlyInertias;

    newView->_fitSceneToView=_fitSceneToView;
    newView->_fitSelectionToView=_fitSelectionToView;

    newView->_renderingMode=_renderingMode;
    newView->_xyGraphInAutoModeDuringSimulation=_xyGraphInAutoModeDuringSimulation;
    newView->_timeGraphXInAutoModeDuringSimulation=_timeGraphXInAutoModeDuringSimulation;
    newView->_timeGraphYInAutoModeDuringSimulation=_timeGraphYInAutoModeDuringSimulation;
    newView->_xyGraphIsOneOneProportional=_xyGraphIsOneOneProportional;
    newView->graphIsTimeGraph=graphIsTimeGraph;
    newView->_canSwapViewWithMainView=_canSwapViewWithMainView;

    newView->_canBeClosed=_canBeClosed;
    newView->_canBeShifted=_canBeShifted;
    newView->_canBeResized=_canBeResized;

    newView->graphPosition[0]=graphPosition[0];
    newView->graphPosition[1]=graphPosition[1];
    newView->graphSize[0]=graphSize[0];
    newView->graphSize[1]=graphSize[1];

    newView->_relativeViewPosition[0]=_relativeViewPosition[0]+_relativeViewPositionOffset;
    newView->_relativeViewPosition[1]=_relativeViewPosition[1]+_relativeViewPositionOffset;
    if (newView->_relativeViewPosition[0]>1.0f)
        newView->_relativeViewPosition[0]=1.0f;
    if (newView->_relativeViewPosition[1]>1.0f)
        newView->_relativeViewPosition[1]=0.0f;
    newView->_relativeViewSize[0]=_relativeViewSize[0];
    newView->_relativeViewSize[1]=_relativeViewSize[1];
    newView->_relativeViewPositionOffset=_relativeViewPositionOffset;
    _relativeViewPositionOffset+=0.02f;

    return(newView);
}

void CSView::setViewIndex(int ind)
{
    _viewIndex=ind;
}

int CSView::getViewIndex() const
{
    return(_viewIndex);
}

void CSView::serialize(CSer& ar)
{
    if (ar.isBinary())
    {
        if (ar.isStoring())
        {
            ar.storeDataName("Oid");
            ar << linkedObjectID;
            ar.flush();

            ar.storeDataName("Gps");
            ar << graphPosition[0] << graphPosition[1];
            ar << graphSize[0] << graphSize[1];
            ar.flush();

            ar.storeDataName("Rem");
            ar << _renderingMode;
            ar.flush();

            ar.storeDataName("Va2");
            unsigned char dummy=0;
            SIM_SET_CLEAR_BIT(dummy,0,perspectiveDisplay);
            SIM_SET_CLEAR_BIT(dummy,1,!_canSwapViewWithMainView);
            SIM_SET_CLEAR_BIT(dummy,2,graphIsTimeGraph);
            SIM_SET_CLEAR_BIT(dummy,3,!_canBeClosed);
            SIM_SET_CLEAR_BIT(dummy,4,_xyGraphInAutoModeDuringSimulation);
            SIM_SET_CLEAR_BIT(dummy,5,_xyGraphIsOneOneProportional);
            SIM_SET_CLEAR_BIT(dummy,6,!_canBeShifted);
            SIM_SET_CLEAR_BIT(dummy,7,!_canBeResized);
            ar << dummy;
            ar.flush();


            ar.storeDataName("Va3");
            dummy=0;
            SIM_SET_CLEAR_BIT(dummy,0,_showEdges);
            SIM_SET_CLEAR_BIT(dummy,1,_thickEdges);
            SIM_SET_CLEAR_BIT(dummy,2,_fitSceneToView);
            SIM_SET_CLEAR_BIT(dummy,3,_fitSelectionToView);
            SIM_SET_CLEAR_BIT(dummy,4,!_timeGraphXInAutoModeDuringSimulation);
            SIM_SET_CLEAR_BIT(dummy,5,!_timeGraphYInAutoModeDuringSimulation);
            SIM_SET_CLEAR_BIT(dummy,6,_visualizeOnlyInertias);
            ar << dummy;
            ar.flush();

            ar.storeDataName(SER_END_OF_OBJECT);
        }
        else
        {
            int byteQuantity;
            std::string theName="";
            while (theName.compare(SER_END_OF_OBJECT)!=0)
            {
                theName=ar.readDataName();
                if (theName.compare(SER_END_OF_OBJECT)!=0)
                {
                    bool noHit=true;
                    if (theName.compare("Oid")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> linkedObjectID;
                    }
                    if (theName.compare("Gps")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> graphPosition[0] >> graphPosition[1];
                        ar >> graphSize[0] >> graphSize[1];
                    }
                    if (theName.compare("Rem")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _renderingMode;
                    }
                    if (theName.compare("Va2")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char dummy;
                        ar >> dummy;
                        perspectiveDisplay=SIM_IS_BIT_SET(dummy,0);
                        _canSwapViewWithMainView=!SIM_IS_BIT_SET(dummy,1);
                        graphIsTimeGraph=SIM_IS_BIT_SET(dummy,2);
                        _canBeClosed=!SIM_IS_BIT_SET(dummy,3);
                        _xyGraphInAutoModeDuringSimulation=SIM_IS_BIT_SET(dummy,4);
                        _xyGraphIsOneOneProportional=SIM_IS_BIT_SET(dummy,5);
                        _canBeShifted=!SIM_IS_BIT_SET(dummy,6);
                        _canBeResized=!SIM_IS_BIT_SET(dummy,7);
                    }
                    if (theName.compare("Va3")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char dummy;
                        ar >> dummy;
                        _showEdges=SIM_IS_BIT_SET(dummy,0);
                        _thickEdges=SIM_IS_BIT_SET(dummy,1);
                        _fitSceneToView=SIM_IS_BIT_SET(dummy,2);
                        _fitSelectionToView=SIM_IS_BIT_SET(dummy,3);
                        _timeGraphXInAutoModeDuringSimulation=!SIM_IS_BIT_SET(dummy,4);
                        _timeGraphYInAutoModeDuringSimulation=!SIM_IS_BIT_SET(dummy,5);
                        _visualizeOnlyInertias=SIM_IS_BIT_SET(dummy,6);
                    }
                    if (noHit)
                        ar.loadUnknownData();
                }
            }
        }
    }
}

void CSView::render(int mainWindowXPos,bool clipWithMainWindowXPos,bool drawText,bool passiveSubView)
{
    FUNCTION_DEBUG;
    C3DObject* it=App::ct->objCont->getObjectFromHandle(linkedObjectID);
    displayView(this,it,mainWindowXPos,clipWithMainWindowXPos,drawText,passiveSubView);
}

#ifdef SIM_WITH_GUI
void CSView::addMenu(VMenu* menu)
{
    //bool lastSelIsGraph=false;
    CGraph* graph=App::ct->objCont->getGraph(linkedObjectID);
    CCamera* camera=App::ct->objCont->getCamera(linkedObjectID);
    CVisionSensor* sensor=App::ct->objCont->getVisionSensor(linkedObjectID);
    int selSize=App::ct->objCont->getSelSize();
    bool lastSelIsCamera=false;
    bool lastSelIsRendSens=false;
    if (selSize>0)
    {
//        if (App::ct->objCont->getLastSelection_object()->getObjectType()==sim_object_graph_type)
//            lastSelIsGraph=true;
        if (App::ct->objCont->getLastSelection_object()->getObjectType()==sim_object_camera_type)
            lastSelIsCamera=true;
        if (App::ct->objCont->getLastSelection_object()->getObjectType()==sim_object_visionsensor_type)
            lastSelIsRendSens=true;
    }
    if (camera!=nullptr)
    { // The linked object is a camera:
        menu->appendMenuItem(true,_renderingMode==RENDERING_MODE_SOLID,VIEW_FUNCTIONS_TOGGLE_SOLID_DISPLAY_VFCMD,IDS_SOLID_RENDERING_MENU_ITEM,true);
        menu->appendMenuItem(true,perspectiveDisplay,VIEW_FUNCTIONS_PERSPECTIVE_DISPLAY_VFCMD,IDS_PERSPECTIVE_PROJECTION_MENU_ITEM,true);
        menu->appendMenuItem(true,_showEdges,VIEW_FUNCTIONS_SHOW_EDGES_VFCMD,IDS_SHOW_EDGES_IN_VIEW_MENU_ITEM,true);
        menu->appendMenuItem(_showEdges,_thickEdges,VIEW_FUNCTIONS_THICK_EDGES_VFCMD,IDS_THICK_EDGES_IN_VIEW_MENU_ITEM,true);
        menu->appendMenuItem(true,_visualizeOnlyInertias,VIEW_FUNCTIONS_SHOW_INERTIAS_VFCMD,IDSN_SHOW_INERTIAS,true);
        menu->appendMenuItem(true,App::ct->environment->getShapeTexturesEnabled(),VIEW_FUNCTIONS_TEXTURED_DISPLAY_VFCMD,IDSN_SHAPE_TEXTURES_ENABLED,true);
        menu->appendMenuSeparator();

//        if ( (selSize==1)&&lastSelIsGraph )
//            menu->appendMenuItem(true,false,VIEW_FUNCTIONS_LOOK_AT_GRAPH_VFCMD,IDS_LOOK_AT_SELECTED_GRAPH_MENU_ITEM);
        /*else*/ if ( (selSize==1)&&lastSelIsRendSens )
            menu->appendMenuItem(true,false,VIEW_FUNCTIONS_LOOK_AT_VISION_SENSOR_VFCMD,IDS_LOOK_AT_SELECTED_VISION_SENSOR_MENU_ITEM);
        else
            menu->appendMenuItem(lastSelIsCamera&&(selSize==1),false,VIEW_FUNCTIONS_LOOK_THROUGH_CAMERA_VFCMD,IDS_LOOK_THROUGH_SELECTED_CAMERA_MENU_ITEM);

        menu->appendMenuItem(true,false,VIEW_SELECTOR_SELECT_ANY_VSCMD,IDSN_SELECT_VIEWABLE_OBJECT);

        C3DObject* trkObj=App::ct->objCont->getObjectFromHandle(camera->getTrackedObjectID());
        if (trkObj!=nullptr)
        {
            std::string tmp(IDS_DONT_TRACK_OBJECT__MENU_ITEM);
            tmp+=trkObj->getObjectName()+"'";
            menu->appendMenuItem(true,false,VIEW_FUNCTIONS_DONT_TRACK_OBJECT_VFCMD,tmp);
        }
        else
        {
            bool illegalLoop=(selSize==1)&&(App::ct->objCont->getLastSelection_object()==camera);
            menu->appendMenuItem((selSize==1)&&(!illegalLoop),false,VIEW_FUNCTIONS_TRACK_OBJECT_VFCMD,IDS_TRACK_SELECTED_OBJECT_MENU_ITEM);
        }
    }
    if (graph!=nullptr)
    { // The linked object is a graph:
        if (graphIsTimeGraph)
        {
            menu->appendMenuItem(true,false,VIEW_FUNCTIONS_XY_GRAPH_DISPLAY_VFCMD,IDS_DISPLAY_X_Y_GRAPH_MENU_ITEM);
            menu->appendMenuItem(true,_timeGraphXInAutoModeDuringSimulation,VIEW_FUNCTIONS_TIME_GRAPH_X_AUTO_MODE_DURING_SIMULATION_VFCMD,IDS_TIME_GRAPH_X_AUTO_MODE_DURING_SIMULATION_MENU_ITEM,true);
            menu->appendMenuItem(true,_timeGraphYInAutoModeDuringSimulation,VIEW_FUNCTIONS_TIME_GRAPH_Y_AUTO_MODE_DURING_SIMULATION_VFCMD,IDS_TIME_GRAPH_Y_AUTO_MODE_DURING_SIMULATION_MENU_ITEM,true);
        }
        else
        {
            menu->appendMenuItem(true,false,VIEW_FUNCTIONS_XY_GRAPH_DISPLAY_VFCMD,IDS_DISPLAY_TIME_GRAPH_MENU_ITEM);
            menu->appendMenuItem(true,_xyGraphInAutoModeDuringSimulation,VIEW_FUNCTIONS_XY_GRAPH_AUTO_MODE_DURING_SIMULATION_VFCMD,IDS_XY_GRAPH_AUTO_MODE_DURING_SIMULATION_MENU_ITEM,true);
            menu->appendMenuItem(true,_xyGraphIsOneOneProportional,VIEW_FUNCTIONS_XY_GRAPH_KEEP_PROPORTIONS_AT_ONE_ONE_VFCMD,IDS_XY_GRAPH_ONE_ONE_PROPORTION_MENU_ITEM,true);
        }
        menu->appendMenuSeparator();

        if ( (selSize==1)&&lastSelIsCamera )
            menu->appendMenuItem(true,false,VIEW_FUNCTIONS_LOOK_THROUGH_CAMERA_VFCMD,IDS_LOOK_THROUGH_SELECTED_CAMERA_MENU_ITEM);
        else if ( (selSize==1)&&lastSelIsRendSens )
            menu->appendMenuItem(true,false,VIEW_FUNCTIONS_LOOK_AT_VISION_SENSOR_VFCMD,IDS_LOOK_AT_SELECTED_VISION_SENSOR_MENU_ITEM);
//        else
//            menu->appendMenuItem(lastSelIsGraph&&(selSize==1),false,VIEW_FUNCTIONS_LOOK_AT_GRAPH_VFCMD,IDS_LOOK_AT_SELECTED_GRAPH_MENU_ITEM);

        menu->appendMenuItem(true,false,VIEW_SELECTOR_SELECT_ANY_VSCMD,IDSN_SELECT_VIEWABLE_OBJECT);
    }
    if (sensor!=nullptr)
    { // The linked object is a vision sensor:
//        if ( (selSize==1)&&lastSelIsGraph )
//            menu->appendMenuItem(true,false,VIEW_FUNCTIONS_LOOK_AT_GRAPH_VFCMD,IDS_LOOK_AT_SELECTED_GRAPH_MENU_ITEM);
        /*else*/ if ( (selSize==1)&&lastSelIsCamera )
            menu->appendMenuItem(true,false,VIEW_FUNCTIONS_LOOK_THROUGH_CAMERA_VFCMD,IDS_LOOK_THROUGH_SELECTED_CAMERA_MENU_ITEM);
        else
            menu->appendMenuItem(lastSelIsRendSens&&(selSize==1),false,VIEW_FUNCTIONS_LOOK_AT_VISION_SENSOR_VFCMD,IDS_LOOK_AT_SELECTED_VISION_SENSOR_MENU_ITEM);

        menu->appendMenuItem(true,false,VIEW_SELECTOR_SELECT_ANY_VSCMD,IDSN_SELECT_VIEWABLE_OBJECT);
    }
    if ( (camera==nullptr)&&(graph==nullptr)&&(sensor==nullptr) )
    {
//        if ( (selSize==1)&&lastSelIsGraph )
//            menu->appendMenuItem(true,false,VIEW_FUNCTIONS_LOOK_AT_GRAPH_VFCMD,IDS_LOOK_AT_SELECTED_GRAPH_MENU_ITEM);
        /*else*/ if ( (selSize==1)&&lastSelIsRendSens )
            menu->appendMenuItem(true,false,VIEW_FUNCTIONS_LOOK_AT_VISION_SENSOR_VFCMD,IDS_LOOK_AT_SELECTED_VISION_SENSOR_MENU_ITEM);
        else
            menu->appendMenuItem(lastSelIsCamera&&(selSize==1),false,VIEW_FUNCTIONS_LOOK_THROUGH_CAMERA_VFCMD,IDS_LOOK_THROUGH_SELECTED_CAMERA_MENU_ITEM);
        menu->appendMenuItem(true,false,VIEW_SELECTOR_SELECT_ANY_VSCMD,IDSN_SELECT_VIEWABLE_OBJECT);
    }
}

int CSView::getCaughtElements() const
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    return(_caughtElements);
}

void CSView::clearCaughtElements(int keepMask)
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    _caughtElements&=keepMask;
}

void CSView::setMousePositionDepth(float depth)
{
    mousePositionDepth=depth;
}
float CSView::getMousePositionDepth() const
{
    return(mousePositionDepth);
}

void CSView::clearMouseJustWentDownAndUpFlag()
{
    if (mouseJustWentUpFlag)
        mouseIsDown=false;
    mouseJustWentDownFlag=false;
    mouseJustWentUpFlag=false;
}

bool CSView::mouseWheel(int deltaZ,int x,int y)
{
    if (x<0)
        return(false);
    if (y<0)
        return(false);
    if (x>_viewSize[0])
        return(false);
    if (y>_viewSize[1])
        return(false);
    C3DObject* it=App::ct->objCont->getObjectFromHandle(linkedObjectID);
    if (it!=nullptr)
    {
        deltaZ=int(float(deltaZ)*App::userSettings->mouseWheelZoomFactor*1.001f);
        if (it->getObjectType()==sim_object_camera_type)
        {
            C3DObject* cameraParentProxy=nullptr;
            if (((CCamera*)it)->getUseParentObjectAsManipulationProxy())
                cameraParentProxy=((CCamera*)it)->getParentObject();
            if (!perspectiveDisplay)
            {
                ((CCamera*)it)->setOrthoViewSize(((CCamera*)it)->getOrthoViewSize()*(1.0f+float(deltaZ)/1920.0f));
                if (cameraParentProxy!=nullptr)
                { // We report the same camera opening to all cameras attached to cameraParentProxy
                    for (int i=0;i<int(cameraParentProxy->childList.size());i++)
                    {
                        if (cameraParentProxy->childList[i]->getObjectType()==sim_object_camera_type)
                        {
                            ((CCamera*)cameraParentProxy->childList[i])->setOrthoViewSize(((CCamera*)it)->getOrthoViewSize());
                            POST_SCENE_CHANGED_GRADUAL_ANNOUNCEMENT(""); // **************** UNDO THINGY ****************
                        }
                    }
                }
            }
            else
            {
                int ct=VDateTime::getTimeInMs();
                static int lastTime=ct;
                int timeDiffInMs=VDateTime::getTimeDiffInMs(lastTime,ct);
                if (timeDiffInMs<100)
                    timeDiffInMs=100;
                float rl=float(deltaZ/120);
                float vel=rl/timeDiffInMs;
                static float previousRl=rl;
                static float fact=1.0f;
                if (previousRl*rl<0)
                {
                    fact=1.0f;
                }
                fact+=fabs(vel)/0.02f;
                if (fact>50.0f)
                    fact=50.0f;
                if (timeDiffInMs>800)
                {
                    fact=1.0f;
                }
                previousRl=rl;
                lastTime=ct;

                C4X4Matrix local(((CCamera*)it)->getLocalTransformation().getMatrix());
                C4X4Matrix localNew(local);
                localNew.X-=localNew.M.axis[2]*0.01f*(((CCamera*)it)->getNearClippingPlane()/0.05f)*fact*float(deltaZ)/120.0f; // Added *(((CCamera*)it)->getNearClippingPlane()/0.05f) on 23/02/2011 to make smaller displacements when near clip. plane is closer
                ((CCamera*)it)->shiftCameraInCameraManipulationMode(localNew.X);
                if (cameraParentProxy!=nullptr)
                { // We manipulate the parent object instead:
                    C7Vector local1(((CCamera*)it)->getLocalTransformation());
                    ((CCamera*)it)->setLocalTransformation(local.getTransformation()); // we reset to initial
                    cameraParentProxy->setLocalTransformation(cameraParentProxy->getLocalTransformation()*local1*(local.getInverse()).getTransformation());
                }
                POST_SCENE_CHANGED_GRADUAL_ANNOUNCEMENT(""); // **************** UNDO THINGY ****************
            }
        }
        if ( (it->getObjectType()==sim_object_graph_type)&&(!App::ct->simulation->isSimulationRunning()) )
        {
            float zoomFact=float(deltaZ/120)*0.1f;
            float centerPos[2]={graphPosition[0]+graphSize[0]/2.0f,graphPosition[1]+graphSize[1]/2.0f};
            graphSize[0]=graphSize[0]*(1.0f+zoomFact);
            graphSize[1]=graphSize[1]*(1.0f+zoomFact);
            graphPosition[0]=centerPos[0]-graphSize[0]/2.0f;
            graphPosition[1]=centerPos[1]-graphSize[1]/2.0f;
            ((CGraph*)it)->validateViewValues(_viewSize,graphPosition,graphSize,graphIsTimeGraph,false,false,false);
            POST_SCENE_CHANGED_GRADUAL_ANNOUNCEMENT(""); // **************** UNDO THINGY ****************
        }
    }
    return(true); // This event was caught
}

bool CSView::getMouseRelPosObjectAndViewSize(int x,int y,int relPos[2],int& objType,int& objID,int vSize[2],bool& viewIsPerspective) const
{ // NOT FULLY IMPLEMENTED! objType=-1 --> not supported, 0 --> hierarchy, 1 --> 3DViewable
    if ( (x<0)||(y<0)||(x>_viewSize[0])||(y>_viewSize[1]) )
        return(false);
    // The position is in this subview
    if (linkedObjectID==-1)
    {
        objType=-1;
        return(true);
    }
    relPos[0]=x;
    relPos[1]=y;
    vSize[0]=_viewSize[0];
    vSize[1]=_viewSize[1];
    objType=1; // 3D object
    objID=linkedObjectID;
    viewIsPerspective=perspectiveDisplay;
    return(true);
}

void CSView::_handleClickRayIntersection(int x,int y,bool mouseDown)
{
    if (App::mainWindow->getKeyDownState()&3)
        return; // doesn't generate any message when the ctrl or shift key is pressed
    CCamera* cam=App::ct->objCont->getCamera(linkedObjectID);
    if (cam==nullptr)
        return;

    if (mouseDown)
    {
        if (App::mainWindow->getProxSensorClickSelectDown()==0)
            return;
        if ((App::mainWindow->getMouseButtonState()&1)==0)
            return; // happens when right-clicking for rotation
    }
    else
    {
        if (App::mainWindow->getProxSensorClickSelectUp()==0)
            return;
    }

    C7Vector tr;
    tr.X.clear();
    float t[2]={(1.0f-float(x)/float(_viewSize[0]))-0.5f,(1.0f-float(y)/float(_viewSize[1]))-0.5f};
    float ratio=(float)(_viewSize[0]/(float)_viewSize[1]);
    if (perspectiveDisplay)
    {
        float va[2];
        if (ratio>1.0f)
        {
            va[0]=cam->getViewAngle();
            va[1]=2.0f*(float)atan(tan(cam->getViewAngle()/2.0f)/ratio);
        }
        else
        {
            va[0]=2.0f*(float)atan(tan(cam->getViewAngle()/2.0f)*ratio);
            va[1]=cam->getViewAngle();
        }
        float a0=atan(2.0f*t[0]*tan(va[0]*0.5f));
        float a1=atan(2.0f*t[1]*tan(va[1]*0.5f));

        C4Vector q(C3Vector(0.0f,0.0f,1.0f),C3Vector(tan(a0),-tan(a1),1.0f));
        tr.Q=q;
    }
    else
    {
        float va[2];
        if (ratio>1.0f)
        {
            va[0]=cam->getOrthoViewSize();
            va[1]=cam->getOrthoViewSize()/ratio;
        }
        else
        {
            va[0]=cam->getOrthoViewSize()*ratio;
            va[1]=cam->getOrthoViewSize();
        }
        float a0=va[0]*t[0];
        float a1=va[1]*(-t[1]);
        tr.X=C3Vector(a0,a1,0.0f);
    }

    // Process the command via the simulation thread (delayed):
    SSimulationThreadCommand cmd;
    cmd.cmdId=CLICK_RAY_INTERSECTION_CMD;
    cmd.boolParams.push_back(mouseDown);
    cmd.floatParams.push_back(cam->getNearClippingPlane());
    cmd.intParams.push_back(cam->getObjectHandle());
    cmd.transfParams.push_back(tr);
    App::appendSimulationThreadCommand(cmd);
}

int CSView::getCursor(int x,int y) const
{
    if ( (x<0)||(y<0)||(x>_viewSize[0])||(y>_viewSize[1]) )
        return(-1);
    int navigationMode=App::getMouseMode()&0x00ff;
    if ( (navigationMode==sim_navigation_objectshift)||(navigationMode==sim_navigation_objectrotate) )
    {
        CCamera* cam=App::ct->objCont->getCamera(linkedObjectID);
        if (cam!=nullptr)
        {
            if ((App::mainWindow->getMouseButtonState()&1)==0)
                return(sim_cursor_open_hand);
            return(sim_cursor_closed_hand);
//            return(sim_cursor_cross);
        }
    }
    return(-1);
}

bool CSView::leftMouseButtonDown(int x,int y,int selStatus)
{
    _caughtElements&=0xffff-sim_left_button;
    _mouseMovedWhileDownFlag=false;
    if ( (x<0)||(y<0)||(x>_viewSize[0])||(y>_viewSize[1]) )
        return(false);
    // Maybe generate an intersection coordinate+object ID with scene objects:
    _handleClickRayIntersection(x,y,true);

    // The mouse went down in this subview
    _caughtElements|=sim_left_button;
    // Was a previous mouse action processed (needs rendering pass)?
    if (mouseJustWentDownFlag||mouseJustWentUpFlag||mouseIsDown)
        return(true); // the mouse is caught but we don't process the event
    if (App::userSettings->noEdgesWhenMouseDownInCameraView)
        CEnvironment::setShapeEdgesTemporarilyDisabled(true);
    if (App::userSettings->noTexturesWhenMouseDownInCameraView)
        CEnvironment::setShapeTexturesTemporarilyDisabled(true);
    if (App::userSettings->noCustomUisWhenMouseDownInCameraView)
        CEnvironment::setCustomUisTemporarilyDisabled(true);
    selectionStatus=selStatus;
    mouseDownRelativePosition[0]=x;
    mouseDownRelativePosition[1]=y;
    mouseRelativePosition[0]=x;
    mouseRelativePosition[1]=y;
    mousePreviousRelativePosition[0]=x;
    mousePreviousRelativePosition[1]=y;
    mouseIsDown=true;
    mouseJustWentDownFlag=true;
    mouseDownTimings=VDateTime::getTimeInMs();
    mouseJustWentDownWasProcessed=false;
    mouseJustWentUpFlag=false;
    // Clear all manip mode overlay grid flags:
    for (int i=0;i<int(App::ct->objCont->objectList.size());i++)
    {
        C3DObject* it=App::ct->objCont->getObjectFromHandle(App::ct->objCont->objectList[i]);
        it->clearManipulationModeOverlayGridFlag();
    }

    return(true); // This event was caught
}

void CSView::leftMouseButtonUp(int x,int y)
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!

    // Maybe generate an intersection coordinate+object ID with scene objects:
    if (!_mouseMovedWhileDownFlag)
        _handleClickRayIntersection(x,y,false); // will happen in a delayed manner


    // Was a previous mouse action processed (needs rendering pass)?
    if ( (!mouseIsDown)||mouseJustWentUpFlag)
        return;
    mouseRelativePosition[0]=x;
    mouseRelativePosition[1]=y;
    mouseJustWentUpFlag=true;
}

void CSView::mouseMove(int x,int y,bool passiveAndFocused)
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    mouseRelativePosition[0]=x;
    mouseRelativePosition[1]=y;
    if (mouseIsDown&&(mouseDownRelativePosition[0]!=x)&&(mouseDownRelativePosition[1]!=y))
        _mouseMovedWhileDownFlag=true;

    if (App::userSettings->navigationBackwardCompatibility)
    {
        if (!passiveAndFocused)
        {
            if (_caughtElements&sim_right_button)
            {
                if ( ((abs(rightMouseDownRelativePosition[1]-mouseRelativePosition[1])>1)||
                    (abs(rightMouseDownRelativePosition[0]-mouseRelativePosition[0])>1) ) &&(!dontActivatePopup)&&(App::mainWindow->getMouseButtonState()&4))
                { // we are zooming with the right button!
                    dontActivatePopup=true;
                    if (!subviewIsPassive) // we do that only on an active view
                        leftMouseButtonDown(_rightMouseButtonDownCoord[0],_rightMouseButtonDownCoord[1],NOSELECTION); // pretend we are using the left button
                }
            }
        }
    }
    else
    {
        if (!passiveAndFocused)
        {
            if (_caughtElements&sim_middle_button)
            { // we are rotating with the middle button!
                if (!subviewIsPassive) // we do that only on an active view
                    leftMouseButtonDown(_rightMouseButtonDownCoord[0],_rightMouseButtonDownCoord[1],NOSELECTION); // pretend we are using the left button
            }
        }
    }
}

int CSView::modelDragMoveEvent(int x,int y,C3Vector* desiredModelPosition)
{
    mouseRelativePosition[0]=x;
    mouseRelativePosition[1]=y;
    if ( (x<0)||(y<0)||(x>_viewSize[0])||(y>_viewSize[1]) )
        return(-1); // mouse not in this view
    C3DObject* obj=App::ct->objCont->getObjectFromHandle(linkedObjectID);
    if ( (obj!=nullptr)&&(obj->getObjectType()==sim_object_camera_type) )
    {
        CCamera* thecam=(CCamera*)obj;

        int relPos[2]={x,y};
        int viewSize[2]={_viewSize[0],_viewSize[1]};
        bool viewIsPerspective=perspectiveDisplay;


        float pos[2]={float(relPos[0]),float(relPos[1])};
        float screenHalfSizes[2]={float(viewSize[0])/2.0f,float(viewSize[1])/2.0f};
        float halfSizes[2];
        float ratio=screenHalfSizes[0]/screenHalfSizes[1];
        if (viewIsPerspective)
        {
            if (ratio>1.0f)
            {
                float a=2.0f*(float)atan(tan(thecam->getViewAngle()/2.0f)/ratio);
                halfSizes[0]=thecam->getViewAngle()/2.0f;
                halfSizes[1]=a/2.0f;
            }
            else
            {
                float a=2.0f*(float)atan(tan(thecam->getViewAngle()/2.0f)*ratio);
                halfSizes[0]=a/2.0f;
                halfSizes[1]=thecam->getViewAngle()/2.0f;
            }
            pos[0]-=screenHalfSizes[0];
            pos[1]-=screenHalfSizes[1];
            pos[0]=atan((pos[0]/screenHalfSizes[0])*tan(halfSizes[0]));
            pos[1]=atan((pos[1]/screenHalfSizes[1])*tan(halfSizes[1]));
            screenHalfSizes[0]=halfSizes[0];
            screenHalfSizes[1]=halfSizes[1];
        }
        else
        {
            if (ratio>1.0f)
            {
                halfSizes[0]=thecam->getOrthoViewSize()*0.5f;
                halfSizes[1]=thecam->getOrthoViewSize()*0.5f/ratio;
            }
            else
            {
                halfSizes[1]=thecam->getOrthoViewSize()*0.5f;
                halfSizes[0]=thecam->getOrthoViewSize()*0.5f*ratio;
            }
            pos[0]-=screenHalfSizes[0];
            pos[1]-=screenHalfSizes[1];
        }

        C4X4Matrix objAbs;
        objAbs.setIdentity();

        C4X4Matrix originalPlane(objAbs); // x-y plane
        C4X4Matrix plane(originalPlane);
        C3Vector p; // point on the plane
        float d=-(plane.X*plane.M.axis[2]);
        float screenP[2]={pos[0],pos[1]};
        C4X4Matrix cam(thecam->getCumulativeTransformationPart1().getMatrix());
        bool singularityProblem=false;

        C3Vector pp(cam.X);
        if (!viewIsPerspective)
        {
            if (fabs(cam.M.axis[2]*plane.M.axis[2])<0.005f)
                singularityProblem=true;
            pp-=cam.M.axis[0]*halfSizes[0]*(screenP[0]/screenHalfSizes[0]);
            pp+=cam.M.axis[1]*halfSizes[1]*(screenP[1]/screenHalfSizes[1]);
            float t=(-d-(plane.M.axis[2]*pp))/(cam.M.axis[2]*plane.M.axis[2]);
            p=pp+cam.M.axis[2]*t;
        }
        else
        {
            C3Vector v(cam.M.axis[2]+cam.M.axis[0]*tan(-screenP[0])+cam.M.axis[1]*tan(screenP[1]));
            v.normalize();
            pp+=v;
            if (fabs(v*plane.M.axis[2])<0.005f)
                singularityProblem=true;
            float t=(-d-(plane.M.axis[2]*pp))/(v*plane.M.axis[2]);
            p=pp+v*t;
        }
        if (!singularityProblem)
        {
            C3Vector pRel(cam.getInverse()*p);
            singularityProblem|=(pRel(2)<thecam->getNearClippingPlane());
            singularityProblem|=(pRel(2)>thecam->getFarClippingPlane());
        }
        if (singularityProblem)
            return(0);
        if (desiredModelPosition!=nullptr)
            desiredModelPosition[0]=p;
        return(2); // Ok we can drop the model here!
    }
    return(0);
}

bool CSView::rightMouseButtonDown(int x,int y,bool _subViewIsPassive)
{ // YOU ARE ONLY ALLOWED TO MODIFY SIMPLE TYPES. NO OBJECT CREATION/DESTRUCTION HERE!!
    subviewIsPassive=_subViewIsPassive;
    if ( (x<0)||(y<0)||(x>_viewSize[0])||(y>_viewSize[1]) )
        return(false);
    _rightMouseButtonDownCoord[0]=x;
    _rightMouseButtonDownCoord[1]=y;
    _caughtElements|=sim_right_button;
    // The mouse went down in this subview
    mouseRelativePosition[0]=x;
    mouseRelativePosition[1]=y;
    rightMouseDownRelativePosition[0]=x;
    rightMouseDownRelativePosition[1]=y;
    dontActivatePopup=false;
    rightButtonIsCaught=true;
    return(true); // The right button was caught
}
bool CSView::rightMouseButtonUp(int x,int y,int absX,int absY,QWidget* mainWindow,int subViewIndex)
{ // true indicates that this view has to be removed (command "Remove view")
    bool caughtSave=rightButtonIsCaught;
    rightButtonIsCaught=false;
    mouseRelativePosition[0]=x;
    mouseRelativePosition[1]=y;
    mouseDownRelativePosition[0]=x;
    mouseDownRelativePosition[1]=y;
    rightMouseDownRelativePosition[0]=x;
    rightMouseDownRelativePosition[1]=y;

    if (dontActivatePopup||(!App::userSettings->navigationBackwardCompatibility))
        leftMouseButtonUp(x,y); // pretend we are using the left button (we were in right-button zoom mode)

    if (x<0)
        return(false);
    if (y<0)
        return(false);
    if (x>_viewSize[0])
        return(false);
    if (y>_viewSize[1])
        return(false);
    // The mouse went up in this subview
    mouseRelativePosition[0]=x;
    mouseRelativePosition[1]=y;
    C3DObject* it=App::ct->objCont->getObjectFromHandle(linkedObjectID);
    int linkedObj=-1;
    if (it!=nullptr)
    {
        if (it->getObjectType()==sim_object_camera_type)
            linkedObj=0;
        if (it->getObjectType()==sim_object_graph_type)
            linkedObj=1;
        if (it->getObjectType()==sim_object_visionsensor_type)
            linkedObj=2;
    }

    if (caughtSave&&(!dontActivatePopup))
    { // We have to activate a popup-menu:
        if (App::operationalUIParts&sim_gui_popups)
        { // Default popups
            VMenu mainMenu=VMenu();

            mainMenu.appendMenuItem(App::getEditModeType()!=BUTTON_EDIT_MODE,false,REMOVE_VIEW_CMD,IDS_REMOVE_PAGE_MENU_ITEM);
            VMenu* viewMenu=new VMenu();
            addMenu(viewMenu);
            mainMenu.appendMenuAndDetach(viewMenu,App::getEditModeType()!=BUTTON_EDIT_MODE,IDS_VIEW_MENU_ITEM);
            if ( (App::getEditModeType()==NO_EDIT_MODE) )
            {
                if (linkedObj==0)
                {
                    VMenu* objectEditMenu=new VMenu();
                    CSceneObjectOperations::addMenu(objectEditMenu);
                    mainMenu.appendMenuAndDetach(objectEditMenu,true,IDS_EDIT_MENU_ITEM);
                }

                VMenu* addMenu=new VMenu();
                CAddOperations::addMenu(addMenu,this,linkedObj==-1);
                mainMenu.appendMenuAndDetach(addMenu,true,IDS_ADD_MENU_ITEM);
            }
            else
            {
                int t=App::getEditModeType();
                if (t&SHAPE_EDIT_MODE)
                {
                    VMenu* triangleVertexEditMenu=new VMenu();
                    App::mainWindow->editModeContainer->addMenu(triangleVertexEditMenu,nullptr);
                    mainMenu.appendMenuAndDetach(triangleVertexEditMenu,true,IDS_EDIT_MENU_ITEM);
                }
                if (t&PATH_EDIT_MODE)
                {
                    VMenu* pathEditMenu=new VMenu();
                    App::mainWindow->editModeContainer->addMenu(pathEditMenu,App::ct->objCont->getObjectFromHandle(linkedObjectID));
                    mainMenu.appendMenuAndDetach(pathEditMenu,true,IDS_EDIT_MENU_ITEM);
                }
                if (t&BUTTON_EDIT_MODE)
                {
                    VMenu* buttonEditMenu=new VMenu();
                    App::mainWindow->editModeContainer->addMenu(buttonEditMenu,nullptr);
                    mainMenu.appendMenuAndDetach(buttonEditMenu,true,IDS_EDIT_MENU_ITEM);
                }
            }

            if (App::getEditModeType()==NO_EDIT_MODE)
            {
                if (linkedObj!=-1)
                {
                    VMenu* simulationMenu=new VMenu();
                    App::ct->simulation->addMenu(simulationMenu);
                    mainMenu.appendMenuAndDetach(simulationMenu,true,IDS_SIMULATION_MENU_ITEM);
                }
            }
            int command=mainMenu.trackPopupMenu();
            if (command==REMOVE_VIEW_CMD)
                return(true);
            bool processed=false;
            if (!processed)
                processed=processCommand(command,subViewIndex);
            if ( (!processed)&&(App::mainWindow!=nullptr) )
                processed=App::mainWindow->oglSurface->viewSelector->processCommand(command,subViewIndex);
            if (!processed)
                processed=CAddOperations::processCommand(command,this);
            if (App::getEditModeType()==NO_EDIT_MODE)
            {
                if (!processed)
                    processed=CSceneObjectOperations::processCommand(command);
            }
            if (App::getEditModeType()&SHAPE_EDIT_MODE)
            {
                if (!processed)
                    processed=App::mainWindow->editModeContainer->processCommand(command,nullptr);
            }
            if (App::getEditModeType()&PATH_EDIT_MODE)
            {
                if (!processed)
                    processed=App::mainWindow->editModeContainer->processCommand(command,App::ct->objCont->getObjectFromHandle(linkedObjectID));
            }
            if (App::getEditModeType()&BUTTON_EDIT_MODE)
            {
                if (!processed)
                    processed=App::mainWindow->editModeContainer->processCommand(command,nullptr);
            }
            if (!processed)
                processed=App::ct->simulation->processCommand(command);
        }
    }
    return(false);
}

bool CSView::middleMouseButtonDown(int x,int y,bool _subViewIsPassive)
{
    subviewIsPassive=_subViewIsPassive;
    if ( (x<0)||(y<0)||(x>_viewSize[0])||(y>_viewSize[1]) )
        return(false);
    if (App::userSettings->noEdgesWhenMouseDownInCameraView)
        CEnvironment::setShapeEdgesTemporarilyDisabled(true);
    if (App::userSettings->noTexturesWhenMouseDownInCameraView)
        CEnvironment::setShapeTexturesTemporarilyDisabled(true);
    if (App::userSettings->noCustomUisWhenMouseDownInCameraView)
        CEnvironment::setCustomUisTemporarilyDisabled(true);
    _rightMouseButtonDownCoord[0]=x;
    _rightMouseButtonDownCoord[1]=y;
    _caughtElements|=sim_middle_button;
    // The mouse went down in this subview
    mouseRelativePosition[0]=x;
    mouseRelativePosition[1]=y;
    rightMouseDownRelativePosition[0]=x;
    rightMouseDownRelativePosition[1]=y;
    return(true); // The middle button was caught
}

void CSView::middleMouseButtonUp(int x,int y)
{
    mouseRelativePosition[0]=x;
    mouseRelativePosition[1]=y;
    mouseDownRelativePosition[0]=x;
    mouseDownRelativePosition[1]=y;
    rightMouseDownRelativePosition[0]=x;
    rightMouseDownRelativePosition[1]=y;

    leftMouseButtonUp(x,y); // pretend we are using the left button (we were in right-button zoom mode)

    if ( (x>=0)||(y>=0)||(x<=_viewSize[0])||(y<=_viewSize[1]) )
    {
        mouseRelativePosition[0]=x;
        mouseRelativePosition[1]=y;
    }
}

bool CSView::leftMouseButtonDoubleClick(int x,int y,int selStatus)
{
    if (x<0)
        return(false);
    if (y<0)
        return(false);
    if (x>_viewSize[0])
        return(false);
    if (y>_viewSize[1])
        return(false);
    // The mouse went down in this subview
    selectionStatus=selStatus;
    mouseRelativePosition[0]=x;
    mouseRelativePosition[1]=y;
    return(false); // Not yet processed
}

void CSView::getMouseDownRelativePosition(int p[2]) const
{
    p[0]=mouseDownRelativePosition[0];
    p[1]=mouseDownRelativePosition[1];
}
void CSView::getMouseRelativePosition(int p[2]) const
{
    p[0]=mouseRelativePosition[0];
    p[1]=mouseRelativePosition[1];
}
void CSView::getPreviousMouseRelativePosition(int p[2]) const
{
    p[0]=mousePreviousRelativePosition[0];
    p[1]=mousePreviousRelativePosition[1];
}
bool CSView::isMouseDown() const
{
    return(mouseIsDown);
}
bool CSView::didMouseJustGoDown() const
{
    return(mouseJustWentDownFlag);
}
bool CSView::wasMouseJustWentDownEventProcessed() const
{
    return(mouseJustWentDownWasProcessed);
}
bool CSView::didMouseJustGoUp() const
{
    return(mouseJustWentUpFlag);
}

bool CSView::didMouseMoveWhileDown() const
{
    return(_mouseMovedWhileDownFlag);
}

void CSView::handleCameraOrGraphMotion()
{
    if ( (mouseJustWentDownFlag||mouseIsDown||mouseJustWentUpFlag) )
    {
        if (selectionStatus==NOSELECTION)
            cameraAndObjectMotion();

        graphMotion();
        // Important to reset the old mouse position to the new mouse position:
        mousePreviousRelativePosition[0]=mouseRelativePosition[0];
        mousePreviousRelativePosition[1]=mouseRelativePosition[1];
    }
    if (mouseJustWentUpFlag)
    {
        mouseIsDown=false;
        selectionStatus=NOSELECTION;
        // Following added on 2010/02/09 in order to be able to manipulate dynamic objects too
        for (int i=0;i<int(App::ct->objCont->objectList.size());i++)
        {
            C3DObject* it=App::ct->objCont->getObjectFromHandle(App::ct->objCont->objectList[i]);
            it->disableDynamicTreeForManipulation(false);
        }
        POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
    }
    mouseJustWentDownFlag=false;
    mouseJustWentUpFlag=false;
}

void CSView::graphMotion()
{
    CGraph* graph=App::ct->objCont->getGraph(linkedObjectID);
    if (graph==nullptr)
        return;

    int const zoomTrigger=30;
    int posDifference=abs(mouseDownRelativePosition[1]-mouseRelativePosition[1])+
                        abs(mouseDownRelativePosition[0]-mouseRelativePosition[0]);
    if (posDifference<zoomTrigger)
        posDifference=0;
    else
        posDifference=posDifference-zoomTrigger;
    if ((selectionStatus!=SHIFTSELECTION)&&(selectionStatus!=CTRLSELECTION))
    {
        float xShift=(mousePreviousRelativePosition[0]-mouseRelativePosition[0])*graphSize[0]/(float)_viewSize[0];
        float yShift=(mousePreviousRelativePosition[1]-mouseRelativePosition[1])*graphSize[1]/(float)_viewSize[1];
        graphPosition[0]=graphPosition[0]+xShift;
        graphPosition[1]=graphPosition[1]+yShift;
        graph->validateViewValues(_viewSize,graphPosition,graphSize,graphIsTimeGraph,true,false,false);
    }

    if (selectionStatus==SHIFTSELECTION)
    {
        float x1=graphPosition[0]+((float)mouseDownRelativePosition[0]/(float)_viewSize[0])*graphSize[0];
        float y1=graphPosition[1]+((float)mouseDownRelativePosition[1]/(float)_viewSize[1])*graphSize[1];
        float x2=graphPosition[0]+((float)mouseRelativePosition[0]/(float)_viewSize[0])*graphSize[0];
        float y2=graphPosition[1]+((float)mouseRelativePosition[1]/(float)_viewSize[1])*graphSize[1];
        tt::limitValue(graphPosition[0],graphPosition[0]+graphSize[0],x1);
        tt::limitValue(graphPosition[1],graphPosition[1]+graphSize[1],y1);
        tt::limitValue(graphPosition[0],graphPosition[0]+graphSize[0],x2);
        tt::limitValue(graphPosition[1],graphPosition[1]+graphSize[1],y2);
        if ((x1>x2)&&(y2>y1))
        { // Gradually zooming out
            float centerPos[2]={graphPosition[0]+graphSize[0]/2.0f,graphPosition[1]+graphSize[1]/2.0f};
            graphSize[0]=graphSize[0]*(1.0f+0.00005f*posDifference);
            graphSize[1]=graphSize[1]*(1.0f+0.00005f*posDifference);
            graphPosition[0]=centerPos[0]-graphSize[0]/2.0f;
            graphPosition[1]=centerPos[1]-graphSize[1]/2.0f;
            graph->validateViewValues(_viewSize,graphPosition,graphSize,graphIsTimeGraph,false,false,false);
        }
        if ((x1>x2)&&(y2<y1))
        { // Gradually zooming in
            float centerPos[2]={graphPosition[0]+graphSize[0]/2.0f,graphPosition[1]+graphSize[1]/2.0f};
            graphSize[0]=graphSize[0]*(1.0f-0.00005f*posDifference);
            graphSize[1]=graphSize[1]*(1.0f-0.00005f*posDifference);
            graphPosition[0]=centerPos[0]-graphSize[0]/2.0f;
            graphPosition[1]=centerPos[1]-graphSize[1]/2.0f;
            graph->validateViewValues(_viewSize,graphPosition,graphSize,graphIsTimeGraph,false,false,false);
        }
        if ((x2>x1)&&(y1>y2)&&mouseJustWentUpFlag)
        { // Zooming in the selected square
            graphPosition[0]=x1;
            graphPosition[1]=y2;
            graphSize[0]=x2-x1;
            graphSize[1]=y1-y2;
            graph->validateViewValues(_viewSize,graphPosition,graphSize,graphIsTimeGraph,false,false,false);
        }
        else if ((x2>x1)&&(y2>y1)&&mouseJustWentUpFlag)
        {// Setting proportions to 1:1
            graph->validateViewValues(_viewSize,graphPosition,graphSize,graphIsTimeGraph,false,true,false);
        }
    }
}


void CSView::cameraAndObjectMotion()
{
    static int eventID=0;
    static int mouseDownInitialPage=0;
    static int mouseDownInitialInstance=0;
    if (mouseJustWentDownFlag)
    {
        eventID++;
        mouseDownInitialPage=App::ct->pageContainer->getActivePageIndex();
        mouseDownInitialInstance=App::ct->getCurrentInstanceIndex();
    }
    else
    {
        if (mouseDownInitialPage!=App::ct->pageContainer->getActivePageIndex())
            eventID++;
        if (mouseDownInitialInstance!=App::ct->getCurrentInstanceIndex())
            eventID++;
    }
    CCamera* camera=App::ct->objCont->getCamera(linkedObjectID);
    if (camera==nullptr)
        return;
    C3DObject* cameraParentProxy=nullptr;
    if (camera->getUseParentObjectAsManipulationProxy())
        cameraParentProxy=camera->getParentObject();
    VPoint activeWinSize(_viewSize[0],_viewSize[1]);
    VPoint dummy;
    bool perspective=perspectiveDisplay;
    VPoint mousePosition(mouseRelativePosition[0],mouseRelativePosition[1]);
    VPoint mouseDownPosition(mouseDownRelativePosition[0],mouseDownRelativePosition[1]);
    VPoint previousMousePosition(mousePreviousRelativePosition[0],mousePreviousRelativePosition[1]);
    int navigationMode=App::getMouseMode()&0x00ff;
    float zoomSensitivity=0.000005f;

    // Needed later...
    int yPosDifference=mouseDownPosition.y-mousePosition.y;
    static int restY=0;
    static int restX=0;
    const int treshhold=20;
    const int sleep=10;
    int yDiff=mousePosition.y-mouseDownPosition.y;
    int xDiff=mousePosition.x-mouseDownPosition.x;
    if ( (yDiff>-treshhold)&&(yDiff<treshhold) )
        restY=restY+yDiff;
    if ( (restY<=-treshhold*sleep)||(restY>=treshhold*sleep) )
    {
        yDiff=restY/sleep;
        restY=0;
    }
    if ( (xDiff>-treshhold)&&(xDiff<treshhold) )
        restX=restX+xDiff;
    if ( (restX<=-treshhold*sleep)||(restX>=treshhold*sleep) )
    {
        xDiff=restX/sleep;
        restX=0;
    }
    std::vector<int> vertexSel;
    if (App::getEditModeType()&SHAPE_EDIT_MODE)
    {
        if (App::getEditModeType()&VERTEX_EDIT_MODE)
        { // We simply copy the selection buffer
            vertexSel.clear();
            vertexSel.insert(vertexSel.begin(),App::mainWindow->editModeContainer->getEditModeBuffer()->begin(),App::mainWindow->editModeContainer->getEditModeBuffer()->end());
        }
        if (App::getEditModeType()&TRIANGLE_EDIT_MODE)
            App::mainWindow->editModeContainer->getShapeEditMode()->selectionFromTriangleToVertexEditMode(&vertexSel);
        if (App::getEditModeType()&EDGE_EDIT_MODE)
            App::mainWindow->editModeContainer->getShapeEditMode()->selectionFromEdgeToVertexEditMode(&vertexSel);
    }

    // Camera/object/vertice/path point 2D rotation
    // ****************************************************************************
    if (navigationMode==sim_navigation_cameratilt)//||
    {
        // Camera 2D rotation
        //-------------------
            C7Vector camOld(camera->getLocalTransformationPart1());
            camera->tiltCameraInCameraManipulationMode(-float(previousMousePosition.x-mousePosition.x)*0.5f*degToRad_f);

            if (cameraParentProxy!=nullptr)
            { // We manipulate the parent object instead:
                C7Vector camNew(camera->getLocalTransformationPart1());
                camera->setLocalTransformation(camOld); // we reset to initial
                cameraParentProxy->setLocalTransformation(cameraParentProxy->getLocalTransformation()*camNew*camOld.getInverse());
            }
    }
    // ****************************************************************************

    // Camera/object 3D rotation..
    // ****************************************************************************
    if ( (navigationMode==sim_navigation_camerarotate)||
        (navigationMode==sim_navigation_objectrotate) )
    {
        float aroundX=(previousMousePosition.y-mousePosition.y)*0.005f;
        float aroundY=(previousMousePosition.x-mousePosition.x)*0.005f;
        C3Vector cp(centerPosition[0],centerPosition[1],centerPosition[2]);
        C7Vector cameraCTM=camera->getCumulativeTransformationPart1();
        if ( (navigationMode==sim_navigation_camerarotate)&&(mousePositionDepth>0.0f) )
        {
            C7Vector parentCTMI(camera->getParentCumulativeTransformation().getInverse());
            { // We have to keep head up
                C3X3Matrix cameraCumulTr=cameraCTM.Q.getMatrix();
                bool headUp=cameraCumulTr(2,1)>=0.0f;
                C3Vector euler=cameraCumulTr.getEulerAngles();
                euler(2)=atan2(cos(euler(0))*sin(euler(1)),sin(euler(0)));
                if (!headUp)
                    euler(2)=euler(2)+piValue_f;
                cameraCTM.Q.setEulerAngles(euler);
                if (!headUp)
                    aroundY=-aroundY;
                C7Vector rot1(aroundY,cp,C3Vector(0.0f,0.0f,1.0f));
                cameraCTM=rot1*cameraCTM;
                C7Vector rot2(aroundX,cp,cameraCTM.getAxis(0));
                cameraCTM=rot2*cameraCTM;
            }
            C7Vector local(camera->getLocalTransformation());
            camera->rotateCameraInCameraManipulationMode(parentCTMI*cameraCTM);

            if (cameraParentProxy!=nullptr)
            { // We manipulate the parent object instead:
                C7Vector local1(camera->getLocalTransformation());
                camera->setLocalTransformation(local); // we reset to initial
                if ((cameraParentProxy->getObjectManipulationModePermissions()&0x1f)==0x1f)
                    cameraParentProxy->setLocalTransformation(cameraParentProxy->getLocalTransformation()*local1*local.getInverse());
            }
        }
        if (navigationMode==sim_navigation_objectrotate)
        {
            float dX=-float(previousMousePosition.x-mousePosition.x)*degToRad_f;

            aroundX=-aroundX;
            aroundY=-aroundY;
            if ((App::getEditModeType()&SHAPE_OR_PATH_EDIT_MODE)==0)
            { // We have object rotation here:
                // Prepare the object that will be rotated, and all other objects in selection appropriately:
                // There is one master object that acts as the rotation pivot. That object needs to be carefully selected
                std::vector<C3DObject*> allSelObj;
                for (int i=0;i<App::ct->objCont->getSelSize();i++)
                {
                    C3DObject* it=App::ct->objCont->getObjectFromHandle(App::ct->objCont->getSelID(i));
                    allSelObj.push_back(it);
                }
                std::vector<C3DObject*> allSelObjects;
                std::map<C3DObject*,bool> occ;
                C3DObject* masterObj=nullptr;
                for (int i=int(allSelObj.size())-1;i>=0;i--)
                {
                    C3DObject* it=allSelObj[i]->getLastParentInSelection(&allSelObj);
                    if (it==nullptr)
                        it=allSelObj[i];
                    std::map<C3DObject*,bool>::iterator it2=occ.find(it);
                    if (it2==occ.end())
                    {
                        occ[it]=true;
                        if (masterObj==nullptr)
                            masterObj=it;
                        else
                            allSelObjects.push_back(it);
                    }
                }
                if (masterObj!=nullptr)
                {
                    bool rotatedMaster=false;
                    C7Vector oldTr(masterObj->getCumulativeTransformationPart1());
                    if (masterObj->setLocalTransformationFromObjectRotationMode(camera->getCumulativeTransformation().getMatrix(),dX*0.5f,perspective,eventID))
                    {
                        masterObj->disableDynamicTreeForManipulation(true); // so that we can also manipulate dynamic objects
                        rotatedMaster=true;
                    }
                    if (rotatedMaster)
                    {
                        C7Vector newTr(masterObj->getCumulativeTransformationPart1());
                        C7Vector shift(newTr*oldTr.getInverse());
                        for (size_t i=0;i<allSelObjects.size();i++)
                        {
                            C3DObject* obj=allSelObjects[i];
                            C7Vector oldLTr=obj->getLocalTransformationPart1();
                            C7Vector parentTr=obj->getParentCumulativeTransformation();
                            obj->setLocalTransformation(parentTr.getInverse()*shift*parentTr*oldLTr);
                            obj->disableDynamicTreeForManipulation(true); // so that we can also manipulate dynamic objects
                        }
                    }
                }
            }
        }
    }
    // ****************************************************************************

    // Camera/object/vertice shifting/zooming...
    // ****************************************************************************
    if ( ((navigationMode==sim_navigation_camerashift)||
        (navigationMode==sim_navigation_objectshift)||
        (navigationMode==sim_navigation_camerazoom) )&&(!mouseJustWentUpFlag) )
    {
        C7Vector cameraLTM(camera->getLocalTransformationPart1());
        C7Vector cameraCTM(camera->getCumulativeTransformationPart1());
        float ratio=(float)(activeWinSize.x/(float)activeWinSize.y);
        float scaleFactor=2*mousePositionDepth*(float)tan((camera->getViewAngle()*180.0f/piValTimes2_f)
            *degToRad_f)/(float)activeWinSize.y;
        if (!perspective)
            scaleFactor=camera->getOrthoViewSize()/(float)activeWinSize.y;
        if (ratio>1.0f)
            scaleFactor=scaleFactor/ratio;
        int ct=VDateTime::getTimeInMs();
        float zoomFactor=(float)(yPosDifference*yPosDifference*zoomSensitivity)*(float(VDateTime::getTimeDiffInMs(mouseDownTimings))/50.0f);
        mouseDownTimings=ct;
        if (yPosDifference>0)
            zoomFactor=-zoomFactor;
        if ( (navigationMode==sim_navigation_camerashift)||(navigationMode==sim_navigation_objectshift) )
            zoomFactor=0.0f;
        if (navigationMode==sim_navigation_camerazoom)
            scaleFactor=0.0f;

        float deltaX=-(previousMousePosition.x-mousePosition.x)*scaleFactor;
        float deltaY=(previousMousePosition.y-mousePosition.y)*scaleFactor;
        float deltaZ=-zoomFactor;

        C3Vector relativeTransl(cameraLTM.getAxis(0)*deltaX+cameraLTM.getAxis(1)*deltaY+cameraLTM.getAxis(2)*deltaZ);
        C3Vector absoluteTransl(cameraCTM.getAxis(0)*deltaX+cameraCTM.getAxis(1)*deltaY+cameraCTM.getAxis(2)*deltaZ);

        if ( (mousePositionDepth<=0.0f)&&( (navigationMode==sim_navigation_camerashift)||
            (navigationMode==sim_navigation_objectshift) ) )
        { // In case we didn't click an object
            relativeTransl.clear();
            absoluteTransl.clear();
        }

        if ( ((navigationMode==sim_navigation_camerashift)||(navigationMode==sim_navigation_camerazoom)) )
        { // Camera shifting/zooming
            C7Vector local(camera->getLocalTransformation());
            camera->shiftCameraInCameraManipulationMode(cameraLTM.X+relativeTransl);
            if (cameraParentProxy!=nullptr)
            { // We manipulate the parent object instead:
                C7Vector local1(camera->getLocalTransformation());
                camera->setLocalTransformation(local); // we reset to initial
                cameraParentProxy->setLocalTransformation(cameraParentProxy->getLocalTransformation()*local1*local.getInverse());
            }
        }
        if (navigationMode==sim_navigation_objectshift)
        { // Object/vertice shifting/zooming
            // We have to invert the translation vector:
            absoluteTransl*=-1.0f;
            if (navigationMode==sim_navigation_objectshift)
            { // We have to shift the green ball
                centerPosition[0]+=absoluteTransl(0);
                centerPosition[1]+=absoluteTransl(1);
                centerPosition[2]+=absoluteTransl(2);
            }
            if ((App::getEditModeType()&SHAPE_OR_PATH_EDIT_MODE)==0)
            { // Object shifting/zooming
                // Prepare the object that will be shifted, and all other objects in selection appropriately:
                // There is one master object that acts as the shift pivot. That object needs to be carefully selected
                std::vector<C3DObject*> allSelObj;
                for (int i=0;i<App::ct->objCont->getSelSize();i++)
                {
                    C3DObject* it=App::ct->objCont->getObjectFromHandle(App::ct->objCont->getSelID(i));
                    allSelObj.push_back(it);
                }
                std::vector<C3DObject*> allSelObjects;
                std::map<C3DObject*,bool> occ;
                C3DObject* masterObj=nullptr;
                for (int i=int(allSelObj.size())-1;i>=0;i--)
                {
                    C3DObject* it=allSelObj[i]->getLastParentInSelection(&allSelObj);
                    if (it==nullptr)
                        it=allSelObj[i];
                    std::map<C3DObject*,bool>::iterator it2=occ.find(it);
                    if (it2==occ.end())
                    {
                        occ[it]=true;
                        if (masterObj==nullptr)
                            masterObj=it;
                        else
                            allSelObjects.push_back(it);
                    }
                }
                if (masterObj!=nullptr)
                {
                    bool shiftedMaster=false;
                    C7Vector oldTr(masterObj->getCumulativeTransformationPart1());
                    float prevPos[2]={float(previousMousePosition.x),float(previousMousePosition.y)};
                    float pos[2]={float(mousePosition.x),float(mousePosition.y)};
                    float screenHalfSizes[2]={float(activeWinSize.x)/2.0f,float(activeWinSize.y)/2.0f};
                    float halfSizes[2];
                    float ratio=screenHalfSizes[0]/screenHalfSizes[1];
                    if (perspective)
                    {
                        if (ratio>1.0f)
                        {
                            float a=2.0f*(float)atan(tan(camera->getViewAngle()/2.0f)/ratio);
                            halfSizes[0]=camera->getViewAngle()/2.0f;
                            halfSizes[1]=a/2.0f;
                        }
                        else
                        {
                            float a=2.0f*(float)atan(tan(camera->getViewAngle()/2.0f)*ratio);
                            halfSizes[0]=a/2.0f;
                            halfSizes[1]=camera->getViewAngle()/2.0f;
                        }
                        prevPos[0]-=screenHalfSizes[0];
                        prevPos[1]-=screenHalfSizes[1];
                        pos[0]-=screenHalfSizes[0];
                        pos[1]-=screenHalfSizes[1];
                        prevPos[0]=atan((prevPos[0]/screenHalfSizes[0])*tan(halfSizes[0]));
                        prevPos[1]=atan((prevPos[1]/screenHalfSizes[1])*tan(halfSizes[1]));
                        pos[0]=atan((pos[0]/screenHalfSizes[0])*tan(halfSizes[0]));
                        pos[1]=atan((pos[1]/screenHalfSizes[1])*tan(halfSizes[1]));
                        screenHalfSizes[0]=halfSizes[0];
                        screenHalfSizes[1]=halfSizes[1];
                    }
                    else
                    {
                        if (ratio>1.0f)
                        {
                            halfSizes[0]=camera->getOrthoViewSize()*0.5f;
                            halfSizes[1]=camera->getOrthoViewSize()*0.5f/ratio;
                        }
                        else
                        {
                            halfSizes[1]=camera->getOrthoViewSize()*0.5f;
                            halfSizes[0]=camera->getOrthoViewSize()*0.5f*ratio;
                        }
                        prevPos[0]-=screenHalfSizes[0];
                        prevPos[1]-=screenHalfSizes[1];
                        pos[0]-=screenHalfSizes[0];
                        pos[1]-=screenHalfSizes[1];
                    }
                    C3Vector centerPos(centerPosition);
                    if ( (masterObj->getObjectType()!=sim_object_path_type)||(allSelObjects.size()!=0)||(App::mainWindow->editModeContainer->pathPointManipulation->getSelectedPathPointIndicesSize_nonEditMode()==0) )
                    { // normal object shifting:
                        if (masterObj->setLocalTransformationFromObjectTranslationMode(camera->getCumulativeTransformation().getMatrix(),centerPos,prevPos,pos,screenHalfSizes,halfSizes,perspective,eventID))
                        {
                            masterObj->disableDynamicTreeForManipulation(true); // so that we can also manipulate dynamic objects
                            shiftedMaster=true;
                        }
                    }
                    else
                    { // path point shifting (non-edit mode!):
                        ((CPath*)masterObj)->transformSelectedPathPoints(camera->getCumulativeTransformation().getMatrix(),centerPos,prevPos,pos,screenHalfSizes,halfSizes,perspective,eventID);
                    }

                    if (shiftedMaster)
                    {
                        C7Vector newTr(masterObj->getCumulativeTransformationPart1());
                        C7Vector shift(newTr*oldTr.getInverse());
                        for (size_t i=0;i<allSelObjects.size();i++)
                        {
                            C3DObject* obj=allSelObjects[i];
                            C7Vector oldLTr=obj->getLocalTransformationPart1();
                            C7Vector parentTr=obj->getParentCumulativeTransformation();
                            obj->setLocalTransformation(parentTr.getInverse()*shift*parentTr*oldLTr);
                            obj->disableDynamicTreeForManipulation(true); // so that we can also manipulate dynamic objects
                        }
                    }
                }
            }
            if (App::getEditModeType()&SHAPE_EDIT_MODE)
            { // Vertice shifting/zooming
                CShape* shape=App::mainWindow->editModeContainer->getEditModeShape();
                C7Vector objCTM(shape->getCumulativeTransformationPart1());
                C7Vector objCTMI(objCTM.getInverse());
                objCTM.X+=absoluteTransl;
                objCTM=objCTMI*objCTM;
                for (int i=0;i<int(vertexSel.size());i++)
                {
                    C3Vector v(App::mainWindow->editModeContainer->getShapeEditMode()->getEditionVertex(vertexSel[i]));
                    App::mainWindow->editModeContainer->getShapeEditMode()->setEditionVertex(vertexSel[i],objCTM*v);
                }
            }
            if (App::getEditModeType()&PATH_EDIT_MODE)
            { // Path point shifting
                float prevPos[2]={float(previousMousePosition.x),float(previousMousePosition.y)};
                float pos[2]={float(mousePosition.x),float(mousePosition.y)};
                float screenHalfSizes[2]={float(activeWinSize.x)/2.0f,float(activeWinSize.y)/2.0f};
                float halfSizes[2];
                float ratio=screenHalfSizes[0]/screenHalfSizes[1];
                if (perspective)
                {
                    if (ratio>1.0f)
                    {
                        float a=2.0f*(float)atan(tan(camera->getViewAngle()/2.0f)/ratio);
                        halfSizes[0]=camera->getViewAngle()/2.0f;
                        halfSizes[1]=a/2.0f;
                    }
                    else
                    {
                        float a=2.0f*(float)atan(tan(camera->getViewAngle()/2.0f)*ratio);
                        halfSizes[0]=a/2.0f;
                        halfSizes[1]=camera->getViewAngle()/2.0f;
                    }
                    prevPos[0]-=screenHalfSizes[0];
                    prevPos[1]-=screenHalfSizes[1];
                    pos[0]-=screenHalfSizes[0];
                    pos[1]-=screenHalfSizes[1];
                    prevPos[0]=atan((prevPos[0]/screenHalfSizes[0])*tan(halfSizes[0]));
                    prevPos[1]=atan((prevPos[1]/screenHalfSizes[1])*tan(halfSizes[1]));
                    pos[0]=atan((pos[0]/screenHalfSizes[0])*tan(halfSizes[0]));
                    pos[1]=atan((pos[1]/screenHalfSizes[1])*tan(halfSizes[1]));
                    screenHalfSizes[0]=halfSizes[0];
                    screenHalfSizes[1]=halfSizes[1];
                }
                else
                {
                    if (ratio>1.0f)
                    {
                        halfSizes[0]=camera->getOrthoViewSize()*0.5f;
                        halfSizes[1]=camera->getOrthoViewSize()*0.5f/ratio;
                    }
                    else
                    {
                        halfSizes[1]=camera->getOrthoViewSize()*0.5f;
                        halfSizes[0]=camera->getOrthoViewSize()*0.5f*ratio;
                    }
                    prevPos[0]-=screenHalfSizes[0];
                    prevPos[1]-=screenHalfSizes[1];
                    pos[0]-=screenHalfSizes[0];
                    pos[1]-=screenHalfSizes[1];
                }
                C3Vector centerPos(centerPosition);

                CPath* path=App::mainWindow->editModeContainer->getEditModePath();
                path->transformSelectedPathPoints(camera->getCumulativeTransformation().getMatrix(),centerPos,prevPos,pos,screenHalfSizes,halfSizes,perspective,eventID);
            }
        }
    }
    // ****************************************************************************

    // Camera opening angle...
    // ****************************************************************************
    if (navigationMode==sim_navigation_cameraangle)
    {
        float zoomFactor=(float)(previousMousePosition.y-mousePosition.y)*0.005f;
        if (perspective)
        {
            float newViewAngle=camera->getViewAngle()+zoomFactor;
            tt::limitValue(10.0f*degToRad_f,135.0f*degToRad_f,newViewAngle); // with 90 degrees, objects disappear!! Really??? Changed to 135 on 2010/11/12
            camera->setViewAngle(newViewAngle);
            if (cameraParentProxy!=nullptr)
            { // We report the same camera opening to all cameras attached to cameraPrentProxy
                for (int i=0;i<int(cameraParentProxy->childList.size());i++)
                {
                    if (cameraParentProxy->childList[i]->getObjectType()==sim_object_camera_type)
                        ((CCamera*)cameraParentProxy->childList[i])->setViewAngle(newViewAngle);
                }
            }
        }
        else
        {
            camera->setOrthoViewSize(camera->getOrthoViewSize()*(1.0f+zoomFactor));
            if (cameraParentProxy!=nullptr)
            { // We report the same camera opening to all cameras attached to cameraPrentProxy
                for (int i=0;i<int(cameraParentProxy->childList.size());i++)
                {
                    if (cameraParentProxy->childList[i]->getObjectType()==sim_object_camera_type)
                        ((CCamera*)cameraParentProxy->childList[i])->setOrthoViewSize(camera->getOrthoViewSize()*(1.0f+zoomFactor));
                }
            }
        }
    }
    // ****************************************************************************

    // Camera fly...
    // ****************************************************************************
    if ((navigationMode==sim_navigation_camerafly)&&(camera->getCameraManipulationModePermissions()&0x008))
    {
        int ct=VDateTime::getTimeInMs();
        static int currentEventID=-4;
        static int lastTime;
        static float velocity;
        static float rotX;
        static float rotY;
        static float rotXVel;
        static float rotYVel;
        static int downX,downY;
        static C4X4Matrix travelDir;
        static bool previousTranslateMode=false;
        if (eventID!=currentEventID)
        {
            currentEventID=eventID;
            lastTime=VDateTime::getTimeInMs();
            velocity=0.15f;
            rotX=0.0f;
            rotY=0.0f;
            rotXVel=0.0f;
            rotYVel=0.0f;
            downX=mouseDownPosition.x;
            downY=mouseDownPosition.y;
            travelDir=camera->getCumulativeTransformationPart1();
            if (App::mainWindow!=nullptr)
                App::mainWindow->setFlyModeCameraHandle(camera->getObjectHandle());
        }
        bool translateMode=((App::mainWindow!=nullptr)&&((App::mainWindow->getKeyDownState()&3)==3));
        if (translateMode!=previousTranslateMode)
        {
            downX=mousePosition.x;
            downY=mousePosition.y;
        }
        previousTranslateMode=translateMode;

        float dt=float(VDateTime::getTimeDiffInMs(lastTime))/1000.0f;
        lastTime=ct;

        if (fabs(dt)>0.0001f)
        {
            float dx=-0.08f*float(mousePosition.x-downX)/float(activeWinSize.x);
            float dy=0.08f*float(mousePosition.y-downY)/float(activeWinSize.y);
            C4X4Matrix m(travelDir);
            C3Vector xDir((C3Vector::unitZVector^m.M.axis[2]).getNormalized());
            C3Vector yDir((m.M.axis[2]^xDir));
            float accelX=0.0f;
            float accelY=0.0f;
            if ((App::mainWindow!=nullptr)&&(App::mainWindow->getKeyDownState()&16))
                accelX=0.5f;
            if ((App::mainWindow!=nullptr)&&(App::mainWindow->getKeyDownState()&32))
                accelX=-0.5f;
            if ((App::mainWindow!=nullptr)&&(App::mainWindow->getKeyDownState()&4))
                accelY=-0.5f;
            if ((App::mainWindow!=nullptr)&&(App::mainWindow->getKeyDownState()&8))
                accelY=0.5f;
            if ((accelX==0.0f)&&(rotXVel!=0.0f))
                accelX=-SIM_MIN(0.5f,fabs(rotXVel)/dt)*rotXVel/fabs(rotXVel);
            if ((accelY==0.0f)&&(rotYVel!=0.0f))
                accelY=-SIM_MIN(0.5f,fabs(rotYVel)/dt)*rotYVel/fabs(rotYVel);
            rotXVel+=dt*accelX;
            rotYVel+=dt*accelY;
            if (rotXVel!=0.0f)
                rotXVel=SIM_MIN(fabs(rotXVel),0.55f)*rotXVel/fabs(rotXVel);
            if (rotYVel!=0.0f)
                rotYVel=SIM_MIN(fabs(rotYVel),0.55f)*rotYVel/fabs(rotYVel);
            rotX+=dt*rotXVel;
            rotY+=dt*rotYVel;
            if (translateMode)
            {
                m.X+=(m.M.axis[2]*velocity*dt)+(xDir*velocity*dt*dx*50.0f)+(yDir*velocity*dt*dy*50.0f);
            }
            else
            {
                if ((App::mainWindow!=nullptr)&&(App::mainWindow->getKeyDownState()&1))
                    velocity-=dt*0.5f;
                if ((App::mainWindow!=nullptr)&&(App::mainWindow->getKeyDownState()&2))
                    velocity+=dt*0.5f;
                if ((velocity<0.0001f)&&(velocity>=0.0f))
                    velocity=0.0001f;
                if ((velocity>-0.0001f)&&(velocity<0.0f))
                    velocity=-0.0001f;
                m.X+=m.M.axis[2]*velocity*dt;
                C3Vector v(((m.M.axis[2]*fabs(velocity)*dt)+(xDir*fabs(velocity)*dt*dx)+(yDir*fabs(velocity)*dt*dy)).getNormalized());
                C3Vector w(((m.M.axis[2]*fabs(velocity)*dt)+(xDir*fabs(velocity)*dt*dx)).getNormalized());
                if (fabs(v*C3Vector::unitZVector)>0.99f)
                    v=w;
                m.M.axis[2]=v;
                m.M.axis[0]=(C3Vector::unitZVector^m.M.axis[2]).getNormalized();
                m.M.axis[1]=m.M.axis[2]^m.M.axis[0];
            }
            C4X4Matrix newCameraPos(m);
            C3X3Matrix rot;
            rot.buildXRotation(-rotY);
            newCameraPos.M*=rot;
            rot.buildZRotation(rotX);
            newCameraPos.M=rot*newCameraPos.M;

            C7Vector local(camera->getLocalTransformationPart1());
            C7Vector local1(camera->getParentCumulativeTransformation().getInverse()*newCameraPos.getTransformation());
            camera->setLocalTransformation(local1);
            if (cameraParentProxy!=nullptr)
            { // We manipulate the parent object instead:
                C7Vector local1(camera->getLocalTransformation());
                camera->setLocalTransformation(local); // we reset to initial
                cameraParentProxy->setLocalTransformation(cameraParentProxy->getLocalTransformation()*local1*local.getInverse());
            }
            travelDir=m;
        }

    }
    // ****************************************************************************

}
#endif
