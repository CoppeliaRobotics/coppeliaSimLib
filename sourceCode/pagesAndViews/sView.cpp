#include <simInternal.h>
#include <sView.h>
#include <tt.h>
#include <simulation.h>
#include <sceneObjectOperations.h>
#include <simulation.h>
#include <addOperations.h>
#include <simStrings.h>
#include <vDateTime.h>
#include <proxSensorRoutine.h>
#include <app.h>
#ifdef SIM_WITH_GUI
#include <viewRendering.h>
#include <oglSurface.h>
#include <guiApp.h>
#endif

CSView::CSView(int theLinkedObjectID)
{
    setDefaultValues();
    linkedObjectID = theLinkedObjectID;
    static int nextUniqueID = 10000;
    _uniqueID = nextUniqueID++;
    _initialValuesInitialized = false;
}

CSView::~CSView()
{
}
void CSView::initializeInitialValues(bool simulationAlreadyRunning)
{ // is called at simulation start, but also after object(s) have been copied into a scene!
    _initialValuesInitialized = true;
    _initialPerspectiveDisplay = perspectiveDisplay;
    _initialShowEdges = _showEdges;
    _initialThickEdges = _thickEdges;
    _initialRenderingMode = _renderingMode;
    _initialGraphIsTimeGraph = graphIsTimeGraph;
}

bool CSView::simulationEnded()
{ // Remember, this is not guaranteed to be run! (the object can be copied during simulation, and pasted after it
    // ended). For thoses situations there is the initializeInitialValues routine!
    if (_initialValuesInitialized)
    {
        perspectiveDisplay = _initialPerspectiveDisplay;
        _showEdges = _initialShowEdges;
        _thickEdges = _initialThickEdges;
        _renderingMode = _initialRenderingMode;
        graphIsTimeGraph = _initialGraphIsTimeGraph;
    }
    _initialValuesInitialized = false;
    return (_removeFloatingViewAtSimulationEnd);
}

int CSView::getUniqueID() const
{
    return (_uniqueID);
}

void CSView::setDefaultValues()
{
    dontActivatePopup = false;
    linkedObjectID = -1;
    perspectiveDisplay = true;
    _showEdges = true;
    _thickEdges = false;
    _removeFloatingViewAtSimulationEnd = false;
    _doNotSaveFloatingView = false;
    _renderingMode = RENDERING_MODE_SOLID;
    _xyGraphInAutoModeDuringSimulation = true;
    _timeGraphXInAutoModeDuringSimulation = true;
    _timeGraphYInAutoModeDuringSimulation = true;
    _xyGraphIsOneOneProportional = false;
    _canSwapViewWithMainView = true;
    _canBeClosed = true;
    _canBeShifted = true;
    _canBeResized = true;

    _fitSceneToView = false;
    _fitSelectionToView = false;

    graphPosition[0] = -0.5;
    graphPosition[1] = -0.5;
    graphSize[0] = 1.0;
    graphSize[1] = 1.0;
    _relativeViewPositionOffset = 0.0;
    _trackedCurveIndex = -1;

    rightButtonIsCaught = false;
    mouseIsDown = false;
    mouseJustWentDownWasProcessed = true;
    mouseJustWentDownFlag = false;
    mouseJustWentUpFlag = false;
    selectionStatus = NOSELECTION;
    _caughtElements = 0;
    dontActivatePopup = false;
    linkedObjectID = -1;
    graphIsTimeGraph = true;
    graphPosition[0] = -0.5;
    graphPosition[1] = -0.5;
    graphSize[0] = 1.0;
    graphSize[1] = 1.0;
    _relativeViewPositionOffset = 0.0;

    rightButtonIsCaught = false;
    mouseIsDown = false;
    mouseJustWentDownWasProcessed = true;
    mouseJustWentDownFlag = false;
    mouseJustWentUpFlag = false;
    selectionStatus = NOSELECTION;
    _caughtElements = 0;
}

void CSView::setCenterPosition(double pos[3])
{
    centerPosition[0] = pos[0];
    centerPosition[1] = pos[1];
    centerPosition[2] = pos[2];
}
void CSView::getCenterPosition(double pos[3]) const
{
    pos[0] = centerPosition[0];
    pos[1] = centerPosition[1];
    pos[2] = centerPosition[2];
}

void CSView::setViewSizeAndPosition(int sizeX, int sizeY, int posX, int posY)
{
    _viewSize[0] = sizeX;
    _viewSize[1] = sizeY;
    _viewPosition[0] = posX;
    _viewPosition[1] = posY;
}

void CSView::getViewSize(int size[2]) const
{
    size[0] = _viewSize[0];
    size[1] = _viewSize[1];
}

void CSView::getViewPosition(int pos[2]) const
{
    pos[0] = _viewPosition[0];
    pos[1] = _viewPosition[1];
}

int CSView::getLinkedObjectID() const
{
    return (linkedObjectID);
}
void CSView::setLinkedObjectID(int theNewLinkedObjectID, bool noDefaultValues)
{
    if (!noDefaultValues)
        setDefaultValues();
    linkedObjectID = theNewLinkedObjectID;
}

void CSView::setAlternativeViewName(const char* name)
{
    _alternativeViewName = name;
}

std::string CSView::getAlternativeViewName() const
{
    return (_alternativeViewName);
}

void CSView::setPerspectiveDisplay(bool perspective)
{
    perspectiveDisplay = perspective;
}

bool CSView::getPerspectiveDisplay() const
{
    return (perspectiveDisplay);
}

void CSView::setShowEdges(bool edges)
{
    _showEdges = edges;
}

bool CSView::getShowEdges() const
{
    return (_showEdges);
}

void CSView::setThickEdges(bool thickEdges)
{
    _thickEdges = thickEdges;
}

bool CSView::getThickEdges() const
{
    return (_thickEdges);
}

void CSView::setRenderingMode(int mode)
{
    _renderingMode = mode;
}

void CSView::setRemoveFloatingViewAtSimulationEnd(bool remove)
{

    _removeFloatingViewAtSimulationEnd = remove;
}

bool CSView::getRemoveFloatingViewAtSimulationEnd() const
{
    return (_removeFloatingViewAtSimulationEnd);
}

void CSView::setDoNotSaveFloatingView(bool doNotSave)
{
    _doNotSaveFloatingView = doNotSave;
}
bool CSView::getDoNotSaveFloatingView() const
{
    return (_doNotSaveFloatingView);
}

int CSView::getRenderingMode() const
{
    return (_renderingMode);
}

void CSView::setXYGraphAutoModeDuringSimulation(bool autoMode)
{
    _xyGraphInAutoModeDuringSimulation = autoMode;
}

bool CSView::getXYGraphAutoModeDuringSimulation() const
{
    return (_xyGraphInAutoModeDuringSimulation);
}

void CSView::setTimeGraphXAutoModeDuringSimulation(bool autoMode)
{
    _timeGraphXInAutoModeDuringSimulation = autoMode;
}

bool CSView::getTimeGraphXAutoModeDuringSimulation() const
{
    return (_timeGraphXInAutoModeDuringSimulation);
}

void CSView::setTimeGraphYAutoModeDuringSimulation(bool autoMode)
{
    _timeGraphYInAutoModeDuringSimulation = autoMode;
}

bool CSView::getTimeGraphYAutoModeDuringSimulation() const
{
    return (_timeGraphYInAutoModeDuringSimulation);
}

void CSView::setXYGraphIsOneOneProportional(bool autoMode)
{
    _xyGraphIsOneOneProportional = autoMode;
}

bool CSView::getXYGraphIsOneOneProportional() const
{
    return (_xyGraphIsOneOneProportional);
}

void CSView::setTimeGraph(bool timeGraph)
{
    graphIsTimeGraph = timeGraph;
}

bool CSView::getTimeGraph() const
{
    return (graphIsTimeGraph);
}

void CSView::setCanSwapViewWithMainView(bool canDoIt)
{
    _canSwapViewWithMainView = canDoIt;
}

bool CSView::getCanSwapViewWithMainView() const
{
    return (_canSwapViewWithMainView);
}

void CSView::setCanBeClosed(bool canDoIt)
{
    _canBeClosed = canDoIt;
}

bool CSView::getCanBeClosed() const
{
    return (_canBeClosed);
}

void CSView::setCanBeShifted(bool canDoIt)
{
    _canBeShifted = canDoIt;
}

bool CSView::getCanBeShifted() const
{
    return (_canBeShifted);
}

void CSView::setCanBeResized(bool canDoIt)
{
    _canBeResized = canDoIt;
}

bool CSView::getCanBeResized() const
{
    return (_canBeResized);
}

void CSView::setGraphPosition(double x, double y)
{
    graphPosition[0] = x;
    graphPosition[1] = y;
}

void CSView::getGraphPosition(double position[2]) const
{
    position[0] = graphPosition[0];
    position[1] = graphPosition[1];
}

void CSView::setGraphSize(double x, double y)
{
    graphSize[0] = x;
    graphSize[1] = y;
}

void CSView::getGraphSize(double size[2]) const
{
    size[0] = graphSize[0];
    size[1] = graphSize[1];
}

bool CSView::announceObjectWillBeErased(int objectID)
{ // Never called from copy buffer!
    if (objectID == linkedObjectID)
    {
        linkedObjectID = -1;
        return (true);
    }
    return (false);
}

void CSView::performObjectLoadingMapping(const std::map<int, int>* map)
{
    linkedObjectID = CWorld::getLoadingMapping(map, linkedObjectID);
}

void CSView::setTrackedGraphCurveIndex(int index)
{
    _trackedCurveIndex = index;
}

int CSView::getTrackedGraphCurveIndex() const
{
    return (_trackedCurveIndex);
}

void CSView::setFitViewToScene(bool doIt)
{
    _fitSceneToView = doIt;
    if (doIt)
        _fitSelectionToView = false;
}

bool CSView::getFitViewToScene() const
{
    return (_fitSceneToView);
}

void CSView::setFitViewToSelection(bool doIt)
{
    _fitSelectionToView = doIt;
    if (doIt)
        _fitSceneToView = false;
}

bool CSView::getFitViewToSelection() const
{
    return (_fitSelectionToView);
}

int CSView::getSelectionStatus() const
{
    return (selectionStatus);
}

CSView* CSView::copyYourself()
{
    CSView* newView = new CSView(linkedObjectID);
    newView->setDefaultValues();
    newView->linkedObjectID = linkedObjectID;
    newView->perspectiveDisplay = perspectiveDisplay;

    newView->_showEdges = _showEdges;
    newView->_thickEdges = _thickEdges;

    newView->_fitSceneToView = _fitSceneToView;
    newView->_fitSelectionToView = _fitSelectionToView;

    newView->_renderingMode = _renderingMode;
    newView->_xyGraphInAutoModeDuringSimulation = _xyGraphInAutoModeDuringSimulation;
    newView->_timeGraphXInAutoModeDuringSimulation = _timeGraphXInAutoModeDuringSimulation;
    newView->_timeGraphYInAutoModeDuringSimulation = _timeGraphYInAutoModeDuringSimulation;
    newView->_xyGraphIsOneOneProportional = _xyGraphIsOneOneProportional;
    newView->graphIsTimeGraph = graphIsTimeGraph;
    newView->_canSwapViewWithMainView = _canSwapViewWithMainView;

    newView->_canBeClosed = _canBeClosed;
    newView->_canBeShifted = _canBeShifted;
    newView->_canBeResized = _canBeResized;

    newView->graphPosition[0] = graphPosition[0];
    newView->graphPosition[1] = graphPosition[1];
    newView->graphSize[0] = graphSize[0];
    newView->graphSize[1] = graphSize[1];

    newView->_relativeViewPosition[0] = _relativeViewPosition[0] + _relativeViewPositionOffset;
    newView->_relativeViewPosition[1] = _relativeViewPosition[1] + _relativeViewPositionOffset;
    if (newView->_relativeViewPosition[0] > 1.0)
        newView->_relativeViewPosition[0] = 1.0;
    if (newView->_relativeViewPosition[1] > 1.0)
        newView->_relativeViewPosition[1] = 0.0;
    newView->_relativeViewSize[0] = _relativeViewSize[0];
    newView->_relativeViewSize[1] = _relativeViewSize[1];
    newView->_relativeViewPositionOffset = _relativeViewPositionOffset;
    _relativeViewPositionOffset += 0.02;

    return (newView);
}

void CSView::setViewIndex(size_t ind)
{
    _viewIndex = ind;
}

size_t CSView::getViewIndex() const
{
    return (_viewIndex);
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

            ar.storeDataName("_ps");
            ar << graphPosition[0] << graphPosition[1];
            ar << graphSize[0] << graphSize[1];
            ar.flush();

            ar.storeDataName("Rem");
            ar << _renderingMode;
            ar.flush();

            ar.storeDataName("Va2");
            unsigned char dummy = 0;
            SIM_SET_CLEAR_BIT(dummy, 0, perspectiveDisplay);
            SIM_SET_CLEAR_BIT(dummy, 1, !_canSwapViewWithMainView);
            SIM_SET_CLEAR_BIT(dummy, 2, graphIsTimeGraph);
            SIM_SET_CLEAR_BIT(dummy, 3, !_canBeClosed);
            SIM_SET_CLEAR_BIT(dummy, 4, _xyGraphInAutoModeDuringSimulation);
            SIM_SET_CLEAR_BIT(dummy, 5, _xyGraphIsOneOneProportional);
            SIM_SET_CLEAR_BIT(dummy, 6, !_canBeShifted);
            SIM_SET_CLEAR_BIT(dummy, 7, !_canBeResized);
            ar << dummy;
            ar.flush();

            ar.storeDataName("Va3");
            dummy = 0;
            SIM_SET_CLEAR_BIT(dummy, 0, _showEdges);
            SIM_SET_CLEAR_BIT(dummy, 1, _thickEdges);
            SIM_SET_CLEAR_BIT(dummy, 2, _fitSceneToView);
            SIM_SET_CLEAR_BIT(dummy, 3, _fitSelectionToView);
            SIM_SET_CLEAR_BIT(dummy, 4, !_timeGraphXInAutoModeDuringSimulation);
            SIM_SET_CLEAR_BIT(dummy, 5, !_timeGraphYInAutoModeDuringSimulation);
            SIM_SET_CLEAR_BIT(dummy, 6, false); // _visualizeOnlyInertias
            ar << dummy;
            ar.flush();

            ar.storeDataName(SER_END_OF_OBJECT);
        }
        else
        {
            int byteQuantity;
            std::string theName = "";
            while (theName.compare(SER_END_OF_OBJECT) != 0)
            {
                theName = ar.readDataName();
                if (theName.compare(SER_END_OF_OBJECT) != 0)
                {
                    bool noHit = true;
                    if (theName.compare("Oid") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> linkedObjectID;
                    }
                    if (theName.compare("Gps") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        float bla, bli;
                        ar >> bla >> bli;
                        graphPosition[0] = (double)bla;
                        graphPosition[1] = (double)bli;
                        ar >> bla >> bli;
                        graphSize[0] = (double)bla;
                        graphSize[1] = (double)bli;
                    }

                    if (theName.compare("_ps") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> graphPosition[0] >> graphPosition[1];
                        ar >> graphSize[0] >> graphSize[1];
                    }

                    if (theName.compare("Rem") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        ar >> _renderingMode;
                    }
                    if (theName.compare("Va2") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        unsigned char dummy;
                        ar >> dummy;
                        perspectiveDisplay = SIM_IS_BIT_SET(dummy, 0);
                        _canSwapViewWithMainView = !SIM_IS_BIT_SET(dummy, 1);
                        graphIsTimeGraph = SIM_IS_BIT_SET(dummy, 2);
                        _canBeClosed = !SIM_IS_BIT_SET(dummy, 3);
                        _xyGraphInAutoModeDuringSimulation = SIM_IS_BIT_SET(dummy, 4);
                        _xyGraphIsOneOneProportional = SIM_IS_BIT_SET(dummy, 5);
                        _canBeShifted = !SIM_IS_BIT_SET(dummy, 6);
                        _canBeResized = !SIM_IS_BIT_SET(dummy, 7);
                    }
                    if (theName.compare("Va3") == 0)
                    {
                        noHit = false;
                        ar >> byteQuantity;
                        unsigned char dummy;
                        ar >> dummy;
                        _showEdges = SIM_IS_BIT_SET(dummy, 0);
                        _thickEdges = SIM_IS_BIT_SET(dummy, 1);
                        _fitSceneToView = SIM_IS_BIT_SET(dummy, 2);
                        _fitSelectionToView = SIM_IS_BIT_SET(dummy, 3);
                        _timeGraphXInAutoModeDuringSimulation = !SIM_IS_BIT_SET(dummy, 4);
                        _timeGraphYInAutoModeDuringSimulation = !SIM_IS_BIT_SET(dummy, 5);
                        // _visualizeOnlyInertias=SIM_IS_BIT_SET(dummy,6);
                    }
                    if (noHit)
                        ar.loadUnknownData();
                }
            }
        }
    }
    else
    {
        if (ar.isStoring())
        {
            ar.xmlAddNode_int("linkedObjectHandle", linkedObjectID);

            ar.xmlAddNode_int("renderMode", _renderingMode);

            ar.xmlPushNewNode("switches");
            ar.xmlAddNode_bool("perspective", perspectiveDisplay);
            ar.xmlAddNode_bool("viewCanBeSwapped", _canSwapViewWithMainView);
            ar.xmlAddNode_bool("viewCanBeClosed", _canBeClosed);
            ar.xmlAddNode_bool("canBeShifted", _canBeShifted);
            ar.xmlAddNode_bool("canBeResized", _canBeResized);
            ar.xmlAddNode_bool("showEdges", _showEdges);
            ar.xmlAddNode_bool("thickEdges", _thickEdges);
            ar.xmlAddNode_bool("fitSceneToView", _fitSceneToView);
            ar.xmlAddNode_bool("fitSelectionToView", _fitSelectionToView);
            ar.xmlPopNode();
        }
        else
        {
            ar.xmlGetNode_int("linkedObjectHandle", linkedObjectID);

            ar.xmlGetNode_int("renderMode", _renderingMode);

            if (ar.xmlPushChildNode("switches"))
            {
                ar.xmlGetNode_bool("perspective", perspectiveDisplay);
                ar.xmlGetNode_bool("viewCanBeSwapped", _canSwapViewWithMainView);
                ar.xmlGetNode_bool("viewCanBeClosed", _canBeClosed);
                ar.xmlGetNode_bool("canBeShifted", _canBeShifted);
                ar.xmlGetNode_bool("canBeResized", _canBeResized);
                ar.xmlGetNode_bool("showEdges", _showEdges);
                ar.xmlGetNode_bool("thickEdges", _thickEdges);
                ar.xmlGetNode_bool("fitSceneToView", _fitSceneToView);
                ar.xmlGetNode_bool("fitSelectionToView", _fitSelectionToView);
                ar.xmlPopNode();
            }
        }
    }
}

#ifdef SIM_WITH_GUI
void CSView::render(int mainWindowXPos, bool clipWithMainWindowXPos, bool drawText, bool passiveSubView)
{
    TRACE_INTERNAL;
    CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(linkedObjectID);
    displayView(this, it, mainWindowXPos, clipWithMainWindowXPos, drawText, passiveSubView);
}

bool CSView::processCommand(int commandID, int subViewIndex)
{ // Return value is true if the command belonged to hierarchy menu and was executed
    if (commandID == VIEW_FUNCTIONS_XY_GRAPH_AUTO_MODE_DURING_SIMULATION_VFCMD)
    {
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            _xyGraphInAutoModeDuringSimulation = !_xyGraphInAutoModeDuringSimulation;
            App::undoRedo_sceneChanged("");
            if (_xyGraphInAutoModeDuringSimulation)
                App::logMsg(sim_verbosity_msgs, IDSNS_NOW_IN_AUTO_MODE);
            else
                App::logMsg(sim_verbosity_msgs, IDSNS_AUTO_MODE_DISABLED);
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId = commandID;
            cmd.intParams.push_back(subViewIndex);
            App::appendSimulationThreadCommand(cmd);
        }
        return (true);
    }
    if (commandID == VIEW_FUNCTIONS_TIME_GRAPH_X_AUTO_MODE_DURING_SIMULATION_VFCMD)
    {
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            _timeGraphXInAutoModeDuringSimulation = !_timeGraphXInAutoModeDuringSimulation;
            App::undoRedo_sceneChanged("");
            if (_timeGraphXInAutoModeDuringSimulation)
                App::logMsg(sim_verbosity_msgs, IDSNS_NOW_IN_AUTO_MODE);
            else
                App::logMsg(sim_verbosity_msgs, IDSNS_AUTO_MODE_DISABLED);
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId = commandID;
            cmd.intParams.push_back(subViewIndex);
            App::appendSimulationThreadCommand(cmd);
        }
        return (true);
    }
    if (commandID == VIEW_FUNCTIONS_TIME_GRAPH_Y_AUTO_MODE_DURING_SIMULATION_VFCMD)
    {
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            _timeGraphYInAutoModeDuringSimulation = !_timeGraphYInAutoModeDuringSimulation;
            App::undoRedo_sceneChanged("");
            if (_timeGraphYInAutoModeDuringSimulation)
                App::logMsg(sim_verbosity_msgs, IDSNS_NOW_IN_AUTO_MODE);
            else
                App::logMsg(sim_verbosity_msgs, IDSNS_AUTO_MODE_DISABLED);
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId = commandID;
            cmd.intParams.push_back(subViewIndex);
            App::appendSimulationThreadCommand(cmd);
        }
        return (true);
    }
    if (commandID == VIEW_FUNCTIONS_XY_GRAPH_KEEP_PROPORTIONS_AT_ONE_ONE_VFCMD)
    {
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            _xyGraphIsOneOneProportional = !_xyGraphIsOneOneProportional;
            App::undoRedo_sceneChanged("");
            if (_xyGraphIsOneOneProportional)
                App::logMsg(sim_verbosity_msgs, IDSNS_KEEPING_PROPORTIONS_AT_1_1);
            else
                App::logMsg(sim_verbosity_msgs, IDSNS_PROPORTIONS_NOT_CONSTRAINED_ANYMORE);
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId = commandID;
            cmd.intParams.push_back(subViewIndex);
            App::appendSimulationThreadCommand(cmd);
        }
        return (true);
    }
    if (commandID == VIEW_FUNCTIONS_XY_GRAPH_DISPLAY_VFCMD)
    {
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            graphIsTimeGraph = !graphIsTimeGraph;
            App::undoRedo_sceneChanged("");
            if (graphIsTimeGraph)
                App::logMsg(sim_verbosity_msgs, IDSNS_NOW_IN_TIME_GRAPH_MODE);
            else
                App::logMsg(sim_verbosity_msgs, IDSNS_NOW_IN_XY_GRAPH_MODE);
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId = commandID;
            cmd.intParams.push_back(subViewIndex);
            App::appendSimulationThreadCommand(cmd);
        }
        return (true);
    }

    if (commandID == VIEW_FUNCTIONS_LOOK_THROUGH_CAMERA_VFCMD)
    {
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            std::vector<int> sel;
            for (size_t i = 0; i < App::currentWorld->sceneObjects->getSelectionCount(); i++)
                sel.push_back(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
            if ((sel.size() == 1) && (App::currentWorld->sceneObjects->getCameraFromHandle(sel[0]) != nullptr))
            {
                setDefaultValues();
                linkedObjectID = sel[0];
                App::undoRedo_sceneChanged("");
                App::logMsg(sim_verbosity_msgs, IDSNS_NOW_LOOKING_THROUGH_SELECTED_CAMERA);
            }
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId = commandID;
            cmd.intParams.push_back(subViewIndex);
            App::appendSimulationThreadCommand(cmd);
        }
        return (true);
    }
    /*
    if (commandID==VIEW_FUNCTIONS_LOOK_AT_GRAPH_VFCMD)
    {
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            std::vector<int> sel;
            for (int i=0;i<App::currentWorld->objCont->getSelSize();i++)
                sel.push_back(App::currentWorld->objCont->getSelID(i));
            if ((sel.size()==1)&&(App::currentWorld->objCont->getGraph(sel[0])!=nullptr))
            {
                setDefaultValues();
                linkedObjectID=sel[0];
                App::undoRedo_sceneChanged("");
                App::logMsg(sim_verbosity_msgs,IDSNS_NOW_LOOKING_AT_SELECTED_GRAPH);
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
    if (commandID == VIEW_FUNCTIONS_LOOK_AT_VISION_SENSOR_VFCMD)
    {
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            std::vector<int> sel;
            for (size_t i = 0; i < App::currentWorld->sceneObjects->getSelectionCount(); i++)
                sel.push_back(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
            if ((sel.size() == 1) && (App::currentWorld->sceneObjects->getVisionSensorFromHandle(sel[0]) != nullptr))
            {
                setDefaultValues();
                linkedObjectID = sel[0];
                App::undoRedo_sceneChanged("");
                App::logMsg(sim_verbosity_msgs, IDSNS_NOW_LOOKING_AT_SELECTED_VISION_SENSOR);
            }
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId = commandID;
            cmd.intParams.push_back(subViewIndex);
            App::appendSimulationThreadCommand(cmd);
        }
        return (true);
    }
    if (commandID == VIEW_FUNCTIONS_TRACK_OBJECT_VFCMD)
    {
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            std::vector<int> sel;
            for (size_t i = 0; i < App::currentWorld->sceneObjects->getSelectionCount(); i++)
                sel.push_back(App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
            CCamera* camera = App::currentWorld->sceneObjects->getCameraFromHandle(linkedObjectID);
            if ((camera != nullptr) && (sel.size() == 1))
            {
                camera->setTrackedObjectHandle(sel[0]);
                App::undoRedo_sceneChanged("");
                App::logMsg(sim_verbosity_msgs, IDSNS_CAMERA_NOW_TRACKING_SELECTED_OBJECT);
            }
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId = commandID;
            cmd.intParams.push_back(subViewIndex);
            App::appendSimulationThreadCommand(cmd);
        }
        return (true);
    }
    if (commandID == VIEW_FUNCTIONS_DONT_TRACK_OBJECT_VFCMD)
    {
        if (!VThread::isUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            CCamera* camera = App::currentWorld->sceneObjects->getCameraFromHandle(linkedObjectID);
            if (camera != nullptr)
            {
                camera->setTrackedObjectHandle(-1);
                App::undoRedo_sceneChanged("");
                App::logMsg(sim_verbosity_msgs, IDSNS_CAMERA_NOW_NOT_TRACKING_ANY_OBJECT);
            }
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId = commandID;
            cmd.intParams.push_back(subViewIndex);
            App::appendSimulationThreadCommand(cmd);
        }
        return (true);
    }
    return (false);
}

void CSView::addMenu(VMenu* menu)
{
    // bool lastSelIsGraph=false;
    CGraph* graph = App::currentWorld->sceneObjects->getGraphFromHandle(linkedObjectID);
    CCamera* camera = App::currentWorld->sceneObjects->getCameraFromHandle(linkedObjectID);
    CVisionSensor* sensor = App::currentWorld->sceneObjects->getVisionSensorFromHandle(linkedObjectID);
    size_t selSize = App::currentWorld->sceneObjects->getSelectionCount();
    bool lastSelIsCamera = false;
    bool lastSelIsRendSens = false;
    if (selSize > 0)
    {
        if (App::currentWorld->sceneObjects->getLastSelectionObject()->getObjectType() == sim_sceneobject_camera)
            lastSelIsCamera = true;
        if (App::currentWorld->sceneObjects->getLastSelectionObject()->getObjectType() == sim_sceneobject_visionsensor)
            lastSelIsRendSens = true;
    }
    if (camera != nullptr)
    { // The linked object is a camera:
        if ((selSize == 1) && lastSelIsRendSens)
            menu->appendMenuItem(true, false, VIEW_FUNCTIONS_LOOK_AT_VISION_SENSOR_VFCMD,
                                 IDS_LOOK_AT_SELECTED_VISION_SENSOR_MENU_ITEM);
        else
            menu->appendMenuItem(lastSelIsCamera && (selSize == 1), false, VIEW_FUNCTIONS_LOOK_THROUGH_CAMERA_VFCMD,
                                 IDS_LOOK_THROUGH_SELECTED_CAMERA_MENU_ITEM);

        menu->appendMenuItem(true, false, VIEW_SELECTOR_SELECT_ANY_VSCMD, IDSN_SELECT_VIEWABLE_OBJECT);

        CSceneObject* trkObj = App::currentWorld->sceneObjects->getObjectFromHandle(camera->getTrackedObjectHandle());
        if (trkObj != nullptr)
        {
            std::string tmp(IDS_DONT_TRACK_OBJECT__MENU_ITEM);
            tmp += trkObj->getObjectAlias_printPath() + "'";
            menu->appendMenuItem(true, false, VIEW_FUNCTIONS_DONT_TRACK_OBJECT_VFCMD, tmp.c_str());
        }
        else
        {
            bool illegalLoop = (selSize == 1) && (App::currentWorld->sceneObjects->getLastSelectionObject() == camera);
            menu->appendMenuItem((selSize == 1) && (!illegalLoop), false, VIEW_FUNCTIONS_TRACK_OBJECT_VFCMD,
                                 IDS_TRACK_SELECTED_OBJECT_MENU_ITEM);
        }
    }
    if (graph != nullptr)
    { // The linked object is a graph:
        if (graphIsTimeGraph)
        {
            menu->appendMenuItem(true, false, VIEW_FUNCTIONS_XY_GRAPH_DISPLAY_VFCMD, IDS_DISPLAY_X_Y_GRAPH_MENU_ITEM);
            menu->appendMenuItem(true, _timeGraphXInAutoModeDuringSimulation,
                                 VIEW_FUNCTIONS_TIME_GRAPH_X_AUTO_MODE_DURING_SIMULATION_VFCMD,
                                 IDS_TIME_GRAPH_X_AUTO_MODE_DURING_SIMULATION_MENU_ITEM, true);
            menu->appendMenuItem(true, _timeGraphYInAutoModeDuringSimulation,
                                 VIEW_FUNCTIONS_TIME_GRAPH_Y_AUTO_MODE_DURING_SIMULATION_VFCMD,
                                 IDS_TIME_GRAPH_Y_AUTO_MODE_DURING_SIMULATION_MENU_ITEM, true);
        }
        else
        {
            menu->appendMenuItem(true, false, VIEW_FUNCTIONS_XY_GRAPH_DISPLAY_VFCMD, IDS_DISPLAY_TIME_GRAPH_MENU_ITEM);
            menu->appendMenuItem(true, _xyGraphInAutoModeDuringSimulation,
                                 VIEW_FUNCTIONS_XY_GRAPH_AUTO_MODE_DURING_SIMULATION_VFCMD,
                                 IDS_XY_GRAPH_AUTO_MODE_DURING_SIMULATION_MENU_ITEM, true);
            menu->appendMenuItem(true, _xyGraphIsOneOneProportional,
                                 VIEW_FUNCTIONS_XY_GRAPH_KEEP_PROPORTIONS_AT_ONE_ONE_VFCMD,
                                 IDS_XY_GRAPH_ONE_ONE_PROPORTION_MENU_ITEM, true);
        }
        menu->appendMenuSeparator();

        if ((selSize == 1) && lastSelIsCamera)
            menu->appendMenuItem(true, false, VIEW_FUNCTIONS_LOOK_THROUGH_CAMERA_VFCMD,
                                 IDS_LOOK_THROUGH_SELECTED_CAMERA_MENU_ITEM);
        else if ((selSize == 1) && lastSelIsRendSens)
            menu->appendMenuItem(true, false, VIEW_FUNCTIONS_LOOK_AT_VISION_SENSOR_VFCMD,
                                 IDS_LOOK_AT_SELECTED_VISION_SENSOR_MENU_ITEM);
        //        else
        //            menu->appendMenuItem(lastSelIsGraph&&(selSize==1),false,VIEW_FUNCTIONS_LOOK_AT_GRAPH_VFCMD,IDS_LOOK_AT_SELECTED_GRAPH_MENU_ITEM);

        menu->appendMenuItem(true, false, VIEW_SELECTOR_SELECT_ANY_VSCMD, IDSN_SELECT_VIEWABLE_OBJECT);
    }
    if (sensor != nullptr)
    {   // The linked object is a vision sensor:
        //        if ( (selSize==1)&&lastSelIsGraph )
        //            menu->appendMenuItem(true,false,VIEW_FUNCTIONS_LOOK_AT_GRAPH_VFCMD,IDS_LOOK_AT_SELECTED_GRAPH_MENU_ITEM);
        /*else*/ if ((selSize == 1) && lastSelIsCamera)
            menu->appendMenuItem(true, false, VIEW_FUNCTIONS_LOOK_THROUGH_CAMERA_VFCMD,
                                 IDS_LOOK_THROUGH_SELECTED_CAMERA_MENU_ITEM);
        else
            menu->appendMenuItem(lastSelIsRendSens && (selSize == 1), false, VIEW_FUNCTIONS_LOOK_AT_VISION_SENSOR_VFCMD,
                                 IDS_LOOK_AT_SELECTED_VISION_SENSOR_MENU_ITEM);

        menu->appendMenuItem(true, false, VIEW_SELECTOR_SELECT_ANY_VSCMD, IDSN_SELECT_VIEWABLE_OBJECT);
    }
    if ((camera == nullptr) && (graph == nullptr) && (sensor == nullptr))
    {
        //        if ( (selSize==1)&&lastSelIsGraph )
        //            menu->appendMenuItem(true,false,VIEW_FUNCTIONS_LOOK_AT_GRAPH_VFCMD,IDS_LOOK_AT_SELECTED_GRAPH_MENU_ITEM);
        /*else*/ if ((selSize == 1) && lastSelIsRendSens)
            menu->appendMenuItem(true, false, VIEW_FUNCTIONS_LOOK_AT_VISION_SENSOR_VFCMD,
                                 IDS_LOOK_AT_SELECTED_VISION_SENSOR_MENU_ITEM);
        else
            menu->appendMenuItem(lastSelIsCamera && (selSize == 1), false, VIEW_FUNCTIONS_LOOK_THROUGH_CAMERA_VFCMD,
                                 IDS_LOOK_THROUGH_SELECTED_CAMERA_MENU_ITEM);
        menu->appendMenuItem(true, false, VIEW_SELECTOR_SELECT_ANY_VSCMD, IDSN_SELECT_VIEWABLE_OBJECT);
    }
}

int CSView::getCaughtElements() const
{
    return (_caughtElements);
}

void CSView::clearCaughtElements(int keepMask)
{
    _caughtElements &= keepMask;
}

void CSView::setMousePositionDepth(double depth)
{
    mousePositionDepth = depth;
}
double CSView::getMousePositionDepth() const
{
    return (mousePositionDepth);
}

void CSView::clearMouseJustWentDownAndUpFlag()
{
    if (mouseJustWentUpFlag)
        mouseIsDown = false;
    mouseJustWentDownFlag = false;
    mouseJustWentUpFlag = false;
}

bool CSView::mouseWheel(int deltaZ, int x, int y)
{
    if (x < 0)
        return (false);
    if (y < 0)
        return (false);
    if (x > _viewSize[0])
        return (false);
    if (y > _viewSize[1])
        return (false);
    CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(linkedObjectID);
    if (it != nullptr)
    {
        deltaZ = int(double(deltaZ) * App::userSettings->mouseWheelZoomFactor * 1.001);
        if (it->getObjectType() == sim_sceneobject_camera)
        {
            CSceneObject* cameraParentProxy = nullptr;
            if (((CCamera*)it)->getUseParentObjectAsManipulationProxy())
                cameraParentProxy = ((CCamera*)it)->getParent();
            if (!perspectiveDisplay)
            {
                ((CCamera*)it)
                    ->setOrthoViewSize(((CCamera*)it)->getOrthoViewSize() * (1.0 + double(deltaZ) / 1920.0));
                if (cameraParentProxy != nullptr)
                { // We report the same camera opening to all cameras attached to cameraParentProxy
                    for (size_t i = 0; i < cameraParentProxy->getChildCount(); i++)
                    {
                        CSceneObject* child = cameraParentProxy->getChildFromIndex(i);
                        if (child->getObjectType() == sim_sceneobject_camera)
                        {
                            ((CCamera*)child)->setOrthoViewSize(((CCamera*)it)->getOrthoViewSize());
                            App::undoRedo_sceneChangedGradual(""); // **************** UNDO THINGY ****************
                        }
                    }
                }
            }
            else
            {
                int ct = (int)VDateTime::getTimeInMs();
                static int lastTime = ct;
                int timeDiffInMs = VDateTime::getTimeDiffInMs(lastTime, ct);
                if (timeDiffInMs < 100)
                    timeDiffInMs = 100;
                double rl = double(deltaZ / 120);
                double vel = rl / timeDiffInMs;
                static double previousRl = rl;
                static double fact = 1.0;
                if (previousRl * rl < 0)
                    fact = 1.0;
                fact += fabs(vel) / 0.02;
                if (fact > 50.0)
                    fact = 50.0;
                if (timeDiffInMs > 800)
                    fact = 1.0;
                previousRl = rl;
                lastTime = ct;
                C4X4Matrix local(((CCamera*)it)->getFullLocalTransformation().getMatrix());
                C4X4Matrix localNew(local);
                double np, fp;
                ((CCamera*)it)->getClippingPlanes(np, fp);
                localNew.X -= localNew.M.axis[2] * 0.01 * (np / 0.05) * fact *
                              double(deltaZ) /
                              120.0;
                ((CCamera*)it)->shiftCameraInCameraManipulationMode(localNew.X);
                if (cameraParentProxy != nullptr)
                { // We manipulate the parent object instead:
                    C7Vector local1(((CCamera*)it)->getFullLocalTransformation());
                    ((CCamera*)it)->setLocalTransformation(local.getTransformation()); // we reset to initial
                    cameraParentProxy->setLocalTransformation(cameraParentProxy->getFullLocalTransformation() * local1 *
                                                              (local.getInverse()).getTransformation());
                }
                App::undoRedo_sceneChangedGradual(""); // **************** UNDO THINGY ****************
            }
        }
        if ((it->getObjectType() == sim_sceneobject_graph) && (!App::currentWorld->simulation->isSimulationRunning()))
        {
            double zoomFact = double(deltaZ / 120) * 0.1;
            double centerPos[2] = {graphPosition[0] + graphSize[0] / 2.0, graphPosition[1] + graphSize[1] / 2.0};
            graphSize[0] = graphSize[0] * (1.0 + zoomFact);
            graphSize[1] = graphSize[1] * (1.0 + zoomFact);
            graphPosition[0] = centerPos[0] - graphSize[0] / 2.0;
            graphPosition[1] = centerPos[1] - graphSize[1] / 2.0;
            ((CGraph*)it)
                ->validateViewValues(_viewSize, graphPosition, graphSize, graphIsTimeGraph, false, false, false);
            App::undoRedo_sceneChangedGradual(""); // **************** UNDO THINGY ****************
        }
    }
    return (true); // This event was caught
}

bool CSView::getMouseRelPosObjectAndViewSize(int x, int y, int relPos[2], int& objType, int& objID, int vSize[2],
                                             bool& viewIsPerspective) const
{ // NOT FULLY IMPLEMENTED! objType=-1 --> not supported, 0 --> hierarchy, 1 --> 3DViewable
    if ((x < 0) || (y < 0) || (x > _viewSize[0]) || (y > _viewSize[1]))
        return (false);
    // The position is in this subview
    if (linkedObjectID == -1)
    {
        objType = -1;
        return (true);
    }
    relPos[0] = x;
    relPos[1] = y;
    vSize[0] = _viewSize[0];
    vSize[1] = _viewSize[1];
    objType = 1; // 3D object
    objID = linkedObjectID;
    viewIsPerspective = perspectiveDisplay;
    return (true);
}

void CSView::_handleClickRayIntersection_old(int x, int y, bool mouseDown)
{
    if (GuiApp::mainWindow->getKeyDownState() & 3)
        return; // doesn't generate any message when the ctrl or shift key is pressed
    CCamera* cam = App::currentWorld->sceneObjects->getCameraFromHandle(linkedObjectID);
    if (cam == nullptr)
        return;

    if (mouseDown)
    {
        if (GuiApp::mainWindow->getProxSensorClickSelectDown() == 0)
            return;
        if ((GuiApp::mainWindow->getMouseButtonState() & 1) == 0)
            return; // happens when right-clicking for rotation
    }
    else
    {
        if (GuiApp::mainWindow->getProxSensorClickSelectUp() == 0)
            return;
    }

    C7Vector tr;
    tr.X.clear();
    double t[2] = {(1.0 - double(x) / double(_viewSize[0])) - 0.5, (1.0 - double(y) / double(_viewSize[1])) - 0.5};
    double ratio = (double)(_viewSize[0] / (double)_viewSize[1]);
    if (perspectiveDisplay)
    {
        double va[2];
        if (ratio > 1.0)
        {
            va[0] = cam->getViewAngle();
            va[1] = 2.0 * atan(tan(cam->getViewAngle() / 2.0) / ratio);
        }
        else
        {
            va[0] = 2.0 * atan(tan(cam->getViewAngle() / 2.0) * ratio);
            va[1] = cam->getViewAngle();
        }
        double a0 = atan(2.0 * t[0] * tan(va[0] * 0.5));
        double a1 = atan(2.0 * t[1] * tan(va[1] * 0.5));

        C4Vector q(C3Vector(0.0, 0.0, 1.0), C3Vector(tan(a0), -tan(a1), 1.0));
        tr.Q = q;
    }
    else
    {
        double va[2];
        if (ratio > 1.0)
        {
            va[0] = cam->getOrthoViewSize();
            va[1] = cam->getOrthoViewSize() / ratio;
        }
        else
        {
            va[0] = cam->getOrthoViewSize() * ratio;
            va[1] = cam->getOrthoViewSize();
        }
        double a0 = va[0] * t[0];
        double a1 = va[1] * (-t[1]);
        tr.X = C3Vector(a0, a1, 0.0);
    }

    // Process the command via the simulation thread (delayed):
    SSimulationThreadCommand cmd;
    cmd.cmdId = CLICK_RAY_INTERSECTION_CMD_OLD;
    cmd.boolParams.push_back(mouseDown);
    double np, fp;
    cam->getClippingPlanes(np, fp);
    cmd.doubleParams.push_back(np);
    cmd.intParams.push_back(cam->getObjectHandle());
    cmd.transfParams.push_back(tr);
    App::appendSimulationThreadCommand(cmd);
}

int CSView::getCursor(int x, int y) const
{
    if ((x < 0) || (y < 0) || (x > _viewSize[0]) || (y > _viewSize[1]))
        return (-1);
    int navigationMode = GuiApp::getMouseMode() & 0x00ff;
    if ((navigationMode == sim_navigation_objectshift) || (navigationMode == sim_navigation_objectrotate))
    {
        CCamera* cam = App::currentWorld->sceneObjects->getCameraFromHandle(linkedObjectID);
        if (cam != nullptr)
        {
            if ((GuiApp::mainWindow->getMouseButtonState() & 1) == 0)
                return (sim_cursor_open_hand);
            return (sim_cursor_closed_hand);
            //            return(sim_cursor_cross);
        }
    }
    return (-1);
}

bool CSView::leftMouseButtonDown(int x, int y, int selStatus)
{
    _caughtElements &= 0xffff - sim_left_button;
    _mouseMovedWhileDownFlag = false;
    if ((x < 0) || (y < 0) || (x > _viewSize[0]) || (y > _viewSize[1]))
        return (false);
    _handleClickRayIntersection_old(x, y, true);

    // The mouse went down in this subview
    _caughtElements |= sim_left_button;
    // Was a previous mouse action processed (needs rendering pass)?
    if (mouseJustWentDownFlag || mouseJustWentUpFlag || mouseIsDown)
        return (true); // the mouse is caught but we don't process the event
    if (App::userSettings->noEdgesWhenMouseDownInCameraView)
        CEnvironment::setShapeEdgesTemporarilyDisabled(true);
    if (App::userSettings->noTexturesWhenMouseDownInCameraView)
        CEnvironment::setShapeTexturesTemporarilyDisabled(true);
    if (App::userSettings->noCustomUisWhenMouseDownInCameraView)
        CEnvironment::setCustomUisTemporarilyDisabled(true);
    selectionStatus = selStatus;
    mouseDownRelativePosition[0] = x;
    mouseDownRelativePosition[1] = y;
    mouseRelativePosition[0] = x;
    mouseRelativePosition[1] = y;
    mousePreviousRelativePosition[0] = x;
    mousePreviousRelativePosition[1] = y;
    mouseIsDown = true;
    mouseJustWentDownFlag = true;
    mouseDownTimings = (int)VDateTime::getTimeInMs();
    mouseJustWentDownWasProcessed = false;
    mouseJustWentUpFlag = false;
    // Clear all manip mode overlay grid flags:
    for (size_t i = 0; i < App::currentWorld->sceneObjects->getObjectCount(); i++)
    {
        CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromIndex(i);
        it->clearManipulationModeOverlayGridFlag();
    }

    return (true); // This event was caught
}

void CSView::leftMouseButtonUp(int x, int y)
{

    if (!_mouseMovedWhileDownFlag)
        _handleClickRayIntersection_old(x, y, false); // will happen in a delayed manner

    // Was a previous mouse action processed (needs rendering pass)?
    if ((!mouseIsDown) || mouseJustWentUpFlag)
        return;
    mouseRelativePosition[0] = x;
    mouseRelativePosition[1] = y;
    mouseJustWentUpFlag = true;
}

void CSView::mouseMove(int x, int y, bool passiveAndFocused)
{
    mouseRelativePosition[0] = x;
    mouseRelativePosition[1] = y;
    if (mouseIsDown && (mouseDownRelativePosition[0] != x) && (mouseDownRelativePosition[1] != y))
        _mouseMovedWhileDownFlag = true;
    static int a = 0;
    if (App::userSettings->navigationBackwardCompatibility)
    {
        if (!passiveAndFocused)
        {
            if (_caughtElements & sim_right_button)
            {
                if (((abs(rightMouseDownRelativePosition[1] - mouseRelativePosition[1]) > 1) ||
                     (abs(rightMouseDownRelativePosition[0] - mouseRelativePosition[0]) > 1)) &&
                    (!dontActivatePopup) && (GuiApp::mainWindow->getMouseButtonState() & 4))
                { // we are zooming with the right button!
                    dontActivatePopup = true;
                    if (!subviewIsPassive) // we do that only on an active view
                        leftMouseButtonDown(_rightMouseButtonDownCoord[0], _rightMouseButtonDownCoord[1],
                                            NOSELECTION); // pretend we are using the left button
                }
            }
        }
    }
    else
    {
        if ((x >= 0) && (y >= 0) && (x < _viewSize[0]) && (y < _viewSize[1]) && (linkedObjectID != -1))
        {
            CCamera* cam = App::currentWorld->sceneObjects->getCameraFromHandle(linkedObjectID);
            if (cam != nullptr)
            {
                C4X4Matrix tr;
                tr.setIdentity();
                double t[2] = {(1.0 - double(x) / double(_viewSize[0])) - 0.5,
                               (1.0 - double(y) / double(_viewSize[1])) - 0.5};
                double ratio = (double)(_viewSize[0] / (double)_viewSize[1]);
                if (perspectiveDisplay)
                {
                    double va[2];
                    if (ratio > 1.0)
                    {
                        va[0] = cam->getViewAngle();
                        va[1] = 2.0 * atan(tan(cam->getViewAngle() / 2.0) / ratio);
                    }
                    else
                    {
                        va[0] = 2.0 * atan(tan(cam->getViewAngle() / 2.0) * ratio);
                        va[1] = cam->getViewAngle();
                    }
                    double a0 = atan(2.0 * t[0] * tan(va[0] * 0.5));
                    double a1 = atan(2.0 * t[1] * tan(va[1] * 0.5));

                    C4Vector q(C3Vector(0.0, 0.0, 1.0), C3Vector(tan(a0), -tan(a1), 1.0));
                    tr.M = q.getMatrix();
                }
                else
                {
                    double va[2];
                    if (ratio > 1.0)
                    {
                        va[0] = cam->getOrthoViewSize();
                        va[1] = cam->getOrthoViewSize() / ratio;
                    }
                    else
                    {
                        va[0] = cam->getOrthoViewSize() * ratio;
                        va[1] = cam->getOrthoViewSize();
                    }
                    double a0 = va[0] * t[0];
                    double a1 = va[1] * (-t[1]);
                    tr.X = C3Vector(a0, a1, 0.0);
                }
                tr = cam->getFullCumulativeTransformation().getMatrix() * tr;
                double np, fp;
                cam->getClippingPlanes(np, fp);
                tr.X += tr.M.axis[2] * np;
                GuiApp::mainWindow->setMouseRay(&tr.X, &tr.M.axis[2]);
            }
        }
        if (!passiveAndFocused)
        {
            if (_caughtElements & sim_middle_button)
            {                          // we are rotating with the middle button!
                if (!subviewIsPassive) // we do that only on an active view
                    leftMouseButtonDown(_rightMouseButtonDownCoord[0], _rightMouseButtonDownCoord[1],
                                        NOSELECTION); // pretend we are using the left button
            }
        }
    }
}

int CSView::modelDragMoveEvent(int x, int y, C3Vector* desiredModelPosition)
{
    mouseRelativePosition[0] = x;
    mouseRelativePosition[1] = y;
    if ((x < 0) || (y < 0) || (x > _viewSize[0]) || (y > _viewSize[1]))
        return (-2); // mouse not in this view
    CSceneObject* obj = App::currentWorld->sceneObjects->getObjectFromHandle(linkedObjectID);
    if ((obj != nullptr) && (obj->getObjectType() == sim_sceneobject_camera))
    {
        CCamera* thecam = (CCamera*)obj;

        int relPos[2] = {x, y};
        int viewSize[2] = {_viewSize[0], _viewSize[1]};
        bool viewIsPerspective = perspectiveDisplay;

        double pos[2] = {double(relPos[0]), double(relPos[1])};
        double screenHalfSizes[2] = {double(viewSize[0]) / 2.0, double(viewSize[1]) / 2.0};
        double halfSizes[2];
        double ratio = screenHalfSizes[0] / screenHalfSizes[1];
        if (viewIsPerspective)
        {
            if (ratio > 1.0)
            {
                double a = 2.0 * atan(tan(thecam->getViewAngle() / 2.0) / ratio);
                halfSizes[0] = thecam->getViewAngle() / 2.0;
                halfSizes[1] = a / 2.0;
            }
            else
            {
                double a = 2.0 * atan(tan(thecam->getViewAngle() / 2.0) * ratio);
                halfSizes[0] = a / 2.0;
                halfSizes[1] = thecam->getViewAngle() / 2.0;
            }
            pos[0] -= screenHalfSizes[0];
            pos[1] -= screenHalfSizes[1];
            pos[0] = atan((pos[0] / screenHalfSizes[0]) * tan(halfSizes[0]));
            pos[1] = atan((pos[1] / screenHalfSizes[1]) * tan(halfSizes[1]));
            screenHalfSizes[0] = halfSizes[0];
            screenHalfSizes[1] = halfSizes[1];
        }
        else
        {
            if (ratio > 1.0)
            {
                halfSizes[0] = thecam->getOrthoViewSize() * 0.5;
                halfSizes[1] = thecam->getOrthoViewSize() * 0.5 / ratio;
            }
            else
            {
                halfSizes[1] = thecam->getOrthoViewSize() * 0.5;
                halfSizes[0] = thecam->getOrthoViewSize() * 0.5 * ratio;
            }
            pos[0] -= screenHalfSizes[0];
            pos[1] -= screenHalfSizes[1];
        }

        C4X4Matrix objAbs;
        objAbs.setIdentity();

        C4X4Matrix originalPlane(objAbs); // x-y plane
        C4X4Matrix plane(originalPlane);
        C3Vector p; // point on the plane
        double d = -(plane.X * plane.M.axis[2]);
        double screenP[2] = {pos[0], pos[1]};
        C4X4Matrix cam(thecam->getCumulativeTransformation().getMatrix());
        bool singularityProblem = false;

        C3Vector pp(cam.X);
        if (!viewIsPerspective)
        {
            if (fabs(cam.M.axis[2] * plane.M.axis[2]) < 0.005)
                singularityProblem = true;
            pp -= cam.M.axis[0] * halfSizes[0] * (screenP[0] / screenHalfSizes[0]);
            pp += cam.M.axis[1] * halfSizes[1] * (screenP[1] / screenHalfSizes[1]);
            double t = (-d - (plane.M.axis[2] * pp)) / (cam.M.axis[2] * plane.M.axis[2]);
            p = pp + cam.M.axis[2] * t;
        }
        else
        {
            C3Vector v(cam.M.axis[2] + cam.M.axis[0] * tan(-screenP[0]) + cam.M.axis[1] * tan(screenP[1]));
            v.normalize();
            pp += v;
            if (fabs(v * plane.M.axis[2]) < 0.005)
                singularityProblem = true;
            double t = (-d - (plane.M.axis[2] * pp)) / (v * plane.M.axis[2]);
            p = pp + v * t;
        }
        if (!singularityProblem)
        {
            double np, fp;
            thecam->getClippingPlanes(np, fp);
            C3Vector pRel(cam.getInverse() * p);
            singularityProblem |= (pRel(2) < np);
            singularityProblem |= (pRel(2) > fp);
        }
        if (singularityProblem)
            return (-2);
        if (desiredModelPosition != nullptr)
            desiredModelPosition[0] = p;
        return (-1); // Ok we can drop the model here!
    }
    return (-2);
}

bool CSView::rightMouseButtonDown(int x, int y, bool _subViewIsPassive)
{
    subviewIsPassive = _subViewIsPassive;
    if ((x < 0) || (y < 0) || (x > _viewSize[0]) || (y > _viewSize[1]))
        return (false);
    _rightMouseButtonDownCoord[0] = x;
    _rightMouseButtonDownCoord[1] = y;
    _caughtElements |= sim_right_button;
    // The mouse went down in this subview
    mouseRelativePosition[0] = x;
    mouseRelativePosition[1] = y;
    rightMouseDownRelativePosition[0] = x;
    rightMouseDownRelativePosition[1] = y;
    dontActivatePopup = false;
    rightButtonIsCaught = true;
    return (true); // The right button was caught
}
bool CSView::rightMouseButtonUp(int x, int y, int absX, int absY, QWidget* mainWindow, int subViewIndex)
{ // true indicates that this view has to be removed (command "Remove view")
    bool caughtSave = rightButtonIsCaught;
    rightButtonIsCaught = false;
    mouseRelativePosition[0] = x;
    mouseRelativePosition[1] = y;
    mouseDownRelativePosition[0] = x;
    mouseDownRelativePosition[1] = y;
    rightMouseDownRelativePosition[0] = x;
    rightMouseDownRelativePosition[1] = y;

    if (dontActivatePopup || (!App::userSettings->navigationBackwardCompatibility))
        leftMouseButtonUp(x, y); // pretend we are using the left button (we were in right-button zoom mode)

    if (x < 0)
        return (false);
    if (y < 0)
        return (false);
    if (x > _viewSize[0])
        return (false);
    if (y > _viewSize[1])
        return (false);
    // The mouse went up in this subview
    mouseRelativePosition[0] = x;
    mouseRelativePosition[1] = y;
    CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(linkedObjectID);
    int linkedObj = -1;
    if (it != nullptr)
    {
        if (it->getObjectType() == sim_sceneobject_camera)
            linkedObj = 0;
        if (it->getObjectType() == sim_sceneobject_graph)
            linkedObj = 1;
        if (it->getObjectType() == sim_sceneobject_visionsensor)
            linkedObj = 2;
    }

    if (caughtSave && (!dontActivatePopup))
    { // We have to activate a popup-menu:
        if (GuiApp::operationalUIParts & sim_gui_popups)
        { // Default popups
            VMenu mainMenu = VMenu();

            mainMenu.appendMenuItem(true, false, REMOVE_VIEW_CMD, IDS_REMOVE_PAGE_MENU_ITEM);
            VMenu* viewMenu = new VMenu();
            addMenu(viewMenu);
            mainMenu.appendMenuAndDetach(viewMenu, true, IDS_VIEW_MENU_ITEM);
            if ((GuiApp::getEditModeType() == NO_EDIT_MODE))
            {
                if (linkedObj == 0)
                {
                    VMenu* objectEditMenu = new VMenu();
                    CSceneObjectOperations::addMenu(objectEditMenu);
                    mainMenu.appendMenuAndDetach(objectEditMenu, true, IDS_EDIT_MENU_ITEM);
                }

                VMenu* addMenu = new VMenu();
                CAddOperations::addMenu(addMenu, this, linkedObj == -1, -1);
                mainMenu.appendMenuAndDetach(addMenu, true, IDS_ADD_MENU_ITEM);
            }
            else
            {
                int t = GuiApp::getEditModeType();
                if (t & SHAPE_EDIT_MODE)
                {
                    VMenu* triangleVertexEditMenu = new VMenu();
                    GuiApp::mainWindow->editModeContainer->addMenu(triangleVertexEditMenu, nullptr);
                    mainMenu.appendMenuAndDetach(triangleVertexEditMenu, true, IDS_EDIT_MENU_ITEM);
                }
                if (t & PATH_EDIT_MODE_OLD)
                {
                    VMenu* pathEditMenu = new VMenu();
                    GuiApp::mainWindow->editModeContainer->addMenu(
                        pathEditMenu, App::currentWorld->sceneObjects->getObjectFromHandle(linkedObjectID));
                    mainMenu.appendMenuAndDetach(pathEditMenu, true, IDS_EDIT_MENU_ITEM);
                }
            }

            if (GuiApp::getEditModeType() == NO_EDIT_MODE)
            {
                if (linkedObj != -1)
                {
                    VMenu* simulationMenu = new VMenu();
                    App::currentWorld->simulation->addMenu(simulationMenu);
                    mainMenu.appendMenuAndDetach(simulationMenu, true, IDS_SIMULATION_MENU_ITEM);
                }
            }
            int command = mainMenu.trackPopupMenu();
            if (command == REMOVE_VIEW_CMD)
                return (true);
            bool processed = false;
            if (!processed)
                processed = processCommand(command, subViewIndex);
            if ((!processed) && (GuiApp::mainWindow != nullptr))
                processed = GuiApp::mainWindow->oglSurface->viewSelector->processCommand(command, subViewIndex);
            if (!processed)
                processed = CAddOperations::processCommand(command, this);
            if (GuiApp::getEditModeType() == NO_EDIT_MODE)
            {
                if (!processed)
                    processed = CSceneObjectOperations::processCommand(command);
            }
            if (GuiApp::getEditModeType() & SHAPE_EDIT_MODE)
            {
                if (!processed)
                    processed = GuiApp::mainWindow->editModeContainer->processCommand(command, nullptr);
            }
            if (GuiApp::getEditModeType() & PATH_EDIT_MODE_OLD)
            {
                if (!processed)
                    processed = GuiApp::mainWindow->editModeContainer->processCommand(
                        command, App::currentWorld->sceneObjects->getObjectFromHandle(linkedObjectID));
            }
            if (!processed)
                processed = App::currentWorld->simulation->processCommand(command);
        }
    }
    return (false);
}

bool CSView::middleMouseButtonDown(int x, int y, bool _subViewIsPassive)
{
    subviewIsPassive = _subViewIsPassive;
    if ((x < 0) || (y < 0) || (x > _viewSize[0]) || (y > _viewSize[1]))
        return (false);
    if (App::userSettings->noEdgesWhenMouseDownInCameraView)
        CEnvironment::setShapeEdgesTemporarilyDisabled(true);
    if (App::userSettings->noTexturesWhenMouseDownInCameraView)
        CEnvironment::setShapeTexturesTemporarilyDisabled(true);
    if (App::userSettings->noCustomUisWhenMouseDownInCameraView)
        CEnvironment::setCustomUisTemporarilyDisabled(true);
    _rightMouseButtonDownCoord[0] = x;
    _rightMouseButtonDownCoord[1] = y;
    _caughtElements |= sim_middle_button;
    // The mouse went down in this subview
    mouseRelativePosition[0] = x;
    mouseRelativePosition[1] = y;
    rightMouseDownRelativePosition[0] = x;
    rightMouseDownRelativePosition[1] = y;
    return (true); // The middle button was caught
}

void CSView::middleMouseButtonUp(int x, int y)
{
    mouseRelativePosition[0] = x;
    mouseRelativePosition[1] = y;
    mouseDownRelativePosition[0] = x;
    mouseDownRelativePosition[1] = y;
    rightMouseDownRelativePosition[0] = x;
    rightMouseDownRelativePosition[1] = y;

    leftMouseButtonUp(x, y); // pretend we are using the left button (we were in right-button zoom mode)

    if ((x >= 0) || (y >= 0) || (x <= _viewSize[0]) || (y <= _viewSize[1]))
    {
        mouseRelativePosition[0] = x;
        mouseRelativePosition[1] = y;
    }
}

bool CSView::leftMouseButtonDoubleClick(int x, int y, int selStatus)
{
    if (x < 0)
        return (false);
    if (y < 0)
        return (false);
    if (x > _viewSize[0])
        return (false);
    if (y > _viewSize[1])
        return (false);
    // The mouse went down in this subview
    selectionStatus = selStatus;
    mouseRelativePosition[0] = x;
    mouseRelativePosition[1] = y;
    return (false); // Not yet processed
}

void CSView::getMouseDownRelativePosition(int p[2]) const
{
    p[0] = mouseDownRelativePosition[0];
    p[1] = mouseDownRelativePosition[1];
}
void CSView::getMouseRelativePosition(int p[2]) const
{
    p[0] = mouseRelativePosition[0];
    p[1] = mouseRelativePosition[1];
}
void CSView::getPreviousMouseRelativePosition(int p[2]) const
{
    p[0] = mousePreviousRelativePosition[0];
    p[1] = mousePreviousRelativePosition[1];
}
bool CSView::isMouseDown() const
{
    return (mouseIsDown);
}
bool CSView::didMouseJustGoDown() const
{
    return (mouseJustWentDownFlag);
}
bool CSView::wasMouseJustWentDownEventProcessed() const
{
    return (mouseJustWentDownWasProcessed);
}
bool CSView::didMouseJustGoUp() const
{
    return (mouseJustWentUpFlag);
}

bool CSView::didMouseMoveWhileDown() const
{
    return (_mouseMovedWhileDownFlag);
}

void CSView::handleCameraOrGraphMotion()
{
    if ((mouseJustWentDownFlag || mouseIsDown || mouseJustWentUpFlag))
    {
        if (selectionStatus == NOSELECTION)
            cameraAndObjectMotion();

        graphMotion();
        // Important to reset the old mouse position to the new mouse position:
        mousePreviousRelativePosition[0] = mouseRelativePosition[0];
        mousePreviousRelativePosition[1] = mouseRelativePosition[1];
    }
    if (mouseJustWentUpFlag)
    {
        mouseIsDown = false;
        selectionStatus = NOSELECTION;
        App::undoRedo_sceneChanged("");
    }
    mouseJustWentDownFlag = false;
    mouseJustWentUpFlag = false;
}

void CSView::graphMotion()
{
    CGraph* graph = App::currentWorld->sceneObjects->getGraphFromHandle(linkedObjectID);
    if (graph == nullptr)
        return;

    int const zoomTrigger = 30;
    int posDifference = abs(mouseDownRelativePosition[1] - mouseRelativePosition[1]) +
                        abs(mouseDownRelativePosition[0] - mouseRelativePosition[0]);
    if (posDifference < zoomTrigger)
        posDifference = 0;
    else
        posDifference = posDifference - zoomTrigger;
    if ((selectionStatus != SHIFTSELECTION) && (selectionStatus != CTRLSELECTION))
    {
        double xShift =
            (mousePreviousRelativePosition[0] - mouseRelativePosition[0]) * graphSize[0] / (double)_viewSize[0];
        double yShift =
            (mousePreviousRelativePosition[1] - mouseRelativePosition[1]) * graphSize[1] / (double)_viewSize[1];
        graphPosition[0] = graphPosition[0] + xShift;
        graphPosition[1] = graphPosition[1] + yShift;
        graph->validateViewValues(_viewSize, graphPosition, graphSize, graphIsTimeGraph, true, false, false);
    }

    if (selectionStatus == SHIFTSELECTION)
    {
        double x1 = graphPosition[0] + ((double)mouseDownRelativePosition[0] / (double)_viewSize[0]) * graphSize[0];
        double y1 = graphPosition[1] + ((double)mouseDownRelativePosition[1] / (double)_viewSize[1]) * graphSize[1];
        double x2 = graphPosition[0] + ((double)mouseRelativePosition[0] / (double)_viewSize[0]) * graphSize[0];
        double y2 = graphPosition[1] + ((double)mouseRelativePosition[1] / (double)_viewSize[1]) * graphSize[1];
        tt::limitValue(graphPosition[0], graphPosition[0] + graphSize[0], x1);
        tt::limitValue(graphPosition[1], graphPosition[1] + graphSize[1], y1);
        tt::limitValue(graphPosition[0], graphPosition[0] + graphSize[0], x2);
        tt::limitValue(graphPosition[1], graphPosition[1] + graphSize[1], y2);
        if ((x1 > x2) && (y2 > y1))
        { // Gradually zooming out
            double centerPos[2] = {graphPosition[0] + graphSize[0] / 2.0, graphPosition[1] + graphSize[1] / 2.0};
            graphSize[0] = graphSize[0] * (1.0 + 0.00005 * posDifference);
            graphSize[1] = graphSize[1] * (1.0 + 0.00005 * posDifference);
            graphPosition[0] = centerPos[0] - graphSize[0] / 2.0;
            graphPosition[1] = centerPos[1] - graphSize[1] / 2.0;
            graph->validateViewValues(_viewSize, graphPosition, graphSize, graphIsTimeGraph, false, false, false);
        }
        if ((x1 > x2) && (y2 < y1))
        { // Gradually zooming in
            double centerPos[2] = {graphPosition[0] + graphSize[0] / 2.0, graphPosition[1] + graphSize[1] / 2.0};
            graphSize[0] = graphSize[0] * (1.0 - 0.00005 * posDifference);
            graphSize[1] = graphSize[1] * (1.0 - 0.00005 * posDifference);
            graphPosition[0] = centerPos[0] - graphSize[0] / 2.0;
            graphPosition[1] = centerPos[1] - graphSize[1] / 2.0;
            graph->validateViewValues(_viewSize, graphPosition, graphSize, graphIsTimeGraph, false, false, false);
        }
        if ((x2 > x1) && (y1 > y2) && mouseJustWentUpFlag)
        { // Zooming in the selected square
            graphPosition[0] = x1;
            graphPosition[1] = y2;
            graphSize[0] = x2 - x1;
            graphSize[1] = y1 - y2;
            graph->validateViewValues(_viewSize, graphPosition, graphSize, graphIsTimeGraph, false, false, false);
        }
        else if ((x2 > x1) && (y2 > y1) && mouseJustWentUpFlag)
        { // Setting proportions to 1:1
            graph->validateViewValues(_viewSize, graphPosition, graphSize, graphIsTimeGraph, false, true, false);
        }
    }
}

void CSView::cameraAndObjectMotion()
{
    static int eventID = 0;
    static int mouseDownInitialPage = 0;
    static int mouseDownInitialInstance = 0;
    if (mouseJustWentDownFlag)
    {
        eventID++;
        mouseDownInitialPage = App::currentWorld->pageContainer->getActivePageIndex();
        mouseDownInitialInstance = App::worldContainer->getCurrentWorldIndex();
    }
    else
    {
        if (mouseDownInitialPage != App::currentWorld->pageContainer->getActivePageIndex())
            eventID++;
        if (mouseDownInitialInstance != App::worldContainer->getCurrentWorldIndex())
            eventID++;
    }
    CCamera* camera = App::currentWorld->sceneObjects->getCameraFromHandle(linkedObjectID);
    if (camera == nullptr)
        return;
    CSceneObject* cameraParentProxy = nullptr;
    if (camera->getUseParentObjectAsManipulationProxy())
        cameraParentProxy = camera->getParent();
    VPoint activeWinSize(_viewSize[0], _viewSize[1]);
    VPoint dummy;
    bool perspective = perspectiveDisplay;
    VPoint mousePosition(mouseRelativePosition[0], mouseRelativePosition[1]);
    VPoint mouseDownPosition(mouseDownRelativePosition[0], mouseDownRelativePosition[1]);
    VPoint previousMousePosition(mousePreviousRelativePosition[0], mousePreviousRelativePosition[1]);
    int navigationMode = GuiApp::getMouseMode() & 0x00ff;
    double zoomSensitivity = 0.000005;

    // Needed later...
    int yPosDifference = mouseDownPosition.y - mousePosition.y;
    static int restY = 0;
    static int restX = 0;
    const int treshhold = 20;
    const int sleep = 10;
    int yDiff = mousePosition.y - mouseDownPosition.y;
    int xDiff = mousePosition.x - mouseDownPosition.x;
    if ((yDiff > -treshhold) && (yDiff < treshhold))
        restY = restY + yDiff;
    if ((restY <= -treshhold * sleep) || (restY >= treshhold * sleep))
    {
        yDiff = restY / sleep;
        restY = 0;
    }
    if ((xDiff > -treshhold) && (xDiff < treshhold))
        restX = restX + xDiff;
    if ((restX <= -treshhold * sleep) || (restX >= treshhold * sleep))
    {
        xDiff = restX / sleep;
        restX = 0;
    }
    std::vector<int> vertexSel;
    if (GuiApp::getEditModeType() & SHAPE_EDIT_MODE)
    {
        if (GuiApp::getEditModeType() & VERTEX_EDIT_MODE)
        { // We simply copy the selection buffer
            vertexSel.clear();
            vertexSel.insert(vertexSel.begin(), GuiApp::mainWindow->editModeContainer->getEditModeBuffer()->begin(),
                             GuiApp::mainWindow->editModeContainer->getEditModeBuffer()->end());
        }
        if (GuiApp::getEditModeType() & TRIANGLE_EDIT_MODE)
            GuiApp::mainWindow->editModeContainer->getShapeEditMode()->selectionFromTriangleToVertexEditMode(
                &vertexSel);
        if (GuiApp::getEditModeType() & EDGE_EDIT_MODE)
            GuiApp::mainWindow->editModeContainer->getShapeEditMode()->selectionFromEdgeToVertexEditMode(&vertexSel);
    }

    // Camera/object 3D rotation..
    // ****************************************************************************
    if ((navigationMode == sim_navigation_camerarotate) || (navigationMode == sim_navigation_objectrotate))
    {
        double aroundX = (previousMousePosition.y - mousePosition.y) * 0.005;
        double aroundY = (previousMousePosition.x - mousePosition.x) * 0.005;
        C3Vector cp(centerPosition[0], centerPosition[1], centerPosition[2]);
        C7Vector cameraCTM = camera->getCumulativeTransformation();
        if ((navigationMode == sim_navigation_camerarotate) && (mousePositionDepth > 0.0))
        {
            C7Vector parentCTMI(camera->getFullParentCumulativeTransformation().getInverse());
            { // We have to keep head up
                C3X3Matrix cameraCumulTr = cameraCTM.Q.getMatrix();
                bool headUp = cameraCumulTr(2, 1) >= 0.0;
                C3Vector euler = cameraCumulTr.getEulerAngles();
                euler(2) = atan2(cos(euler(0)) * sin(euler(1)), sin(euler(0)));
                if (!headUp)
                    euler(2) = euler(2) + piValue;
                cameraCTM.Q.setEulerAngles(euler);
                if (!headUp)
                    aroundY = -aroundY;
                C7Vector rot1(aroundY, cp, C3Vector(0.0, 0.0, 1.0));
                cameraCTM = rot1 * cameraCTM;
                C7Vector rot2(aroundX, cp, cameraCTM.getAxis(0));
                cameraCTM = rot2 * cameraCTM;
            }
            C7Vector local(camera->getFullLocalTransformation());
            camera->rotateCameraInCameraManipulationMode(parentCTMI * cameraCTM);

            if (cameraParentProxy != nullptr)
            { // We manipulate the parent object instead:
                C7Vector local1(camera->getFullLocalTransformation());
                camera->setLocalTransformation(local); // we reset to initial
                if ((cameraParentProxy->getObjectMovementPreferredAxes() & 0x1f) == 0x1f)
                    cameraParentProxy->setLocalTransformation(cameraParentProxy->getFullLocalTransformation() * local1 *
                                                              local.getInverse());
            }
        }
        if (navigationMode == sim_navigation_objectrotate)
        {
            double dX = -double(previousMousePosition.x - mousePosition.x) * degToRad;

            aroundX = -aroundX;
            aroundY = -aroundY;
            if ((GuiApp::getEditModeType() & SHAPE_OR_PATH_EDIT_MODE_OLD) == 0)
            { // We have object rotation here:
                // Prepare the object that will be rotated, and all other objects in selection appropriately:
                // There is one master object that acts as the rotation pivot. That object needs to be carefully
                // selected
                std::vector<CSceneObject*> allSelObj;
                for (size_t i = 0; i < App::currentWorld->sceneObjects->getSelectionCount(); i++)
                {
                    CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(
                        App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
                    allSelObj.push_back(it);
                }
                std::vector<CSceneObject*> allSelObjects;
                std::map<CSceneObject*, bool> occ;
                CSceneObject* masterObj = nullptr;
                for (int i = int(allSelObj.size()) - 1; i >= 0; i--)
                {
                    CSceneObject* it = allSelObj[i]->getLastParentInSelection(&allSelObj);
                    if (it == nullptr)
                        it = allSelObj[i];
                    std::map<CSceneObject*, bool>::iterator it2 = occ.find(it);
                    if (it2 == occ.end())
                    {
                        occ[it] = true;
                        if (masterObj == nullptr)
                            masterObj = it;
                        else
                            allSelObjects.push_back(it);
                    }
                }
                if (masterObj != nullptr)
                {
                    bool rotatedMaster = false;
                    C7Vector oldTr(masterObj->getCumulativeTransformation());
                    if (masterObj->setLocalTransformationFromObjectRotationMode(
                            camera->getFullCumulativeTransformation().getMatrix(), dX * 0.5, perspective, eventID))
                        rotatedMaster = true;
                    if (rotatedMaster)
                    {
                        C7Vector newTr(masterObj->getCumulativeTransformation());
                        C7Vector shift(newTr * oldTr.getInverse());
                        for (size_t i = 0; i < allSelObjects.size(); i++)
                        {
                            CSceneObject* obj = allSelObjects[i];
                            C7Vector oldLTr = obj->getLocalTransformation();
                            C7Vector parentTr = obj->getFullParentCumulativeTransformation();
                            obj->setLocalTransformation(parentTr.getInverse() * shift * parentTr * oldLTr);
                        }
                    }
                }
            }
        }
    }
    // ****************************************************************************

    // Camera/object/vertice shifting/zooming...
    // ****************************************************************************
    if (((navigationMode == sim_navigation_camerashift) || (navigationMode == sim_navigation_objectshift) ||
         (navigationMode == sim_navigation_camerazoom)) &&
        (!mouseJustWentUpFlag))
    {
        C7Vector cameraLTM(camera->getLocalTransformation());
        C7Vector cameraCTM(camera->getCumulativeTransformation());
        double ratio = (double)(activeWinSize.x / (double)activeWinSize.y);
        double scaleFactor = 2.0 * mousePositionDepth * tan((camera->getViewAngle() * 180.0 / piValT2) * degToRad) /
                             (double)activeWinSize.y;
        if (!perspective)
            scaleFactor = camera->getOrthoViewSize() / (double)activeWinSize.y;
        if (ratio > 1.0)
            scaleFactor = scaleFactor / ratio;
        int ct = (int)VDateTime::getTimeInMs();
        double zoomFactor = (double)(yPosDifference * yPosDifference * zoomSensitivity) *
                            (double(VDateTime::getTimeDiffInMs(mouseDownTimings)) / 50.0);
        mouseDownTimings = ct;
        if (yPosDifference > 0)
            zoomFactor = -zoomFactor;
        if ((navigationMode == sim_navigation_camerashift) || (navigationMode == sim_navigation_objectshift))
            zoomFactor = 0.0;
        if (navigationMode == sim_navigation_camerazoom)
            scaleFactor = 0.0;

        double deltaX = -(previousMousePosition.x - mousePosition.x) * scaleFactor;
        double deltaY = (previousMousePosition.y - mousePosition.y) * scaleFactor;
        double deltaZ = -zoomFactor;

        C3Vector relativeTransl(cameraLTM.getAxis(0) * deltaX + cameraLTM.getAxis(1) * deltaY +
                                cameraLTM.getAxis(2) * deltaZ);
        C3Vector absoluteTransl(cameraCTM.getAxis(0) * deltaX + cameraCTM.getAxis(1) * deltaY +
                                cameraCTM.getAxis(2) * deltaZ);

        if ((mousePositionDepth <= 0.0) &&
            ((navigationMode == sim_navigation_camerashift) || (navigationMode == sim_navigation_objectshift)))
        { // In case we didn't click an object
            relativeTransl.clear();
            absoluteTransl.clear();
        }

        if (((navigationMode == sim_navigation_camerashift) || (navigationMode == sim_navigation_camerazoom)))
        { // Camera shifting/zooming
            C7Vector local(camera->getFullLocalTransformation());
            camera->shiftCameraInCameraManipulationMode(cameraLTM.X + relativeTransl);
            if (cameraParentProxy != nullptr)
            { // We manipulate the parent object instead:
                C7Vector local1(camera->getFullLocalTransformation());
                camera->setLocalTransformation(local); // we reset to initial
                cameraParentProxy->setLocalTransformation(cameraParentProxy->getFullLocalTransformation() * local1 *
                                                          local.getInverse());
            }
        }
        if (navigationMode == sim_navigation_objectshift)
        { // Object/vertice shifting/zooming
            // We have to invert the translation vector:
            absoluteTransl *= -1.0;
            if (navigationMode == sim_navigation_objectshift)
            { // We have to shift the green ball
                centerPosition[0] += absoluteTransl(0);
                centerPosition[1] += absoluteTransl(1);
                centerPosition[2] += absoluteTransl(2);
            }
            if ((GuiApp::getEditModeType() & SHAPE_OR_PATH_EDIT_MODE_OLD) == 0)
            { // Object shifting/zooming
                // Prepare the object that will be shifted, and all other objects in selection appropriately:
                // There is one master object that acts as the shift pivot. That object needs to be carefully selected
                std::vector<CSceneObject*> allSelObj;
                for (size_t i = 0; i < App::currentWorld->sceneObjects->getSelectionCount(); i++)
                {
                    CSceneObject* it = App::currentWorld->sceneObjects->getObjectFromHandle(
                        App::currentWorld->sceneObjects->getObjectHandleFromSelectionIndex(i));
                    allSelObj.push_back(it);
                }
                std::vector<CSceneObject*> allSelObjects;
                std::map<CSceneObject*, bool> occ;
                CSceneObject* masterObj = nullptr;
                for (int i = int(allSelObj.size()) - 1; i >= 0; i--)
                {
                    CSceneObject* it = allSelObj[i]->getLastParentInSelection(&allSelObj);
                    if (it == nullptr)
                        it = allSelObj[i];
                    std::map<CSceneObject*, bool>::iterator it2 = occ.find(it);
                    if (it2 == occ.end())
                    {
                        occ[it] = true;
                        if (masterObj == nullptr)
                            masterObj = it;
                        else
                            allSelObjects.push_back(it);
                    }
                }
                if (masterObj != nullptr)
                {
                    bool shiftedMaster = false;
                    C7Vector oldTr(masterObj->getCumulativeTransformation());
                    double prevPos[2] = {double(previousMousePosition.x), double(previousMousePosition.y)};
                    double pos[2] = {double(mousePosition.x), double(mousePosition.y)};
                    double screenHalfSizes[2] = {double(activeWinSize.x) / 2.0, double(activeWinSize.y) / 2.0};
                    double halfSizes[2];
                    double ratio = screenHalfSizes[0] / screenHalfSizes[1];
                    if (perspective)
                    {
                        if (ratio > 1.0)
                        {
                            double a = 2.0 * atan(tan(camera->getViewAngle() / 2.0) / ratio);
                            halfSizes[0] = camera->getViewAngle() / 2.0;
                            halfSizes[1] = a / 2.0;
                        }
                        else
                        {
                            double a = 2.0 * atan(tan(camera->getViewAngle() / 2.0) * ratio);
                            halfSizes[0] = a / 2.0;
                            halfSizes[1] = camera->getViewAngle() / 2.0;
                        }
                        prevPos[0] -= screenHalfSizes[0];
                        prevPos[1] -= screenHalfSizes[1];
                        pos[0] -= screenHalfSizes[0];
                        pos[1] -= screenHalfSizes[1];
                        prevPos[0] = atan((prevPos[0] / screenHalfSizes[0]) * tan(halfSizes[0]));
                        prevPos[1] = atan((prevPos[1] / screenHalfSizes[1]) * tan(halfSizes[1]));
                        pos[0] = atan((pos[0] / screenHalfSizes[0]) * tan(halfSizes[0]));
                        pos[1] = atan((pos[1] / screenHalfSizes[1]) * tan(halfSizes[1]));
                        screenHalfSizes[0] = halfSizes[0];
                        screenHalfSizes[1] = halfSizes[1];
                    }
                    else
                    {
                        if (ratio > 1.0)
                        {
                            halfSizes[0] = camera->getOrthoViewSize() * 0.5;
                            halfSizes[1] = camera->getOrthoViewSize() * 0.5 / ratio;
                        }
                        else
                        {
                            halfSizes[1] = camera->getOrthoViewSize() * 0.5;
                            halfSizes[0] = camera->getOrthoViewSize() * 0.5 * ratio;
                        }
                        prevPos[0] -= screenHalfSizes[0];
                        prevPos[1] -= screenHalfSizes[1];
                        pos[0] -= screenHalfSizes[0];
                        pos[1] -= screenHalfSizes[1];
                    }
                    C3Vector centerPos(centerPosition);
                    if ((masterObj->getObjectType() != sim_sceneobject_path) || (allSelObjects.size() != 0) ||
                        (GuiApp::mainWindow->editModeContainer->pathPointManipulation
                             ->getSelectedPathPointIndicesSize_nonEditMode() == 0))
                    { // normal object shifting:
                        if (masterObj->setLocalTransformationFromObjectTranslationMode(
                                camera->getFullCumulativeTransformation().getMatrix(), centerPos, prevPos, pos,
                                screenHalfSizes, halfSizes, perspective, eventID))
                            shiftedMaster = true;
                    }
                    else
                    { // path point shifting (non-edit mode!):
                        ((CPath_old*)masterObj)
                            ->transformSelectedPathPoints(camera->getFullCumulativeTransformation().getMatrix(),
                                                          centerPos, prevPos, pos, screenHalfSizes, halfSizes,
                                                          perspective, eventID);
                    }

                    if (shiftedMaster)
                    {
                        C7Vector newTr(masterObj->getCumulativeTransformation());
                        C7Vector shift(newTr * oldTr.getInverse());
                        for (size_t i = 0; i < allSelObjects.size(); i++)
                        {
                            CSceneObject* obj = allSelObjects[i];
                            C7Vector oldLTr = obj->getLocalTransformation();
                            C7Vector parentTr = obj->getFullParentCumulativeTransformation();
                            obj->setLocalTransformation(parentTr.getInverse() * shift * parentTr * oldLTr);
                        }
                    }
                }
            }
            if (GuiApp::getEditModeType() & SHAPE_EDIT_MODE)
            { // Vertice shifting/zooming
                CShape* shape = GuiApp::mainWindow->editModeContainer->getEditModeShape();
                C7Vector objCTM(shape->getCumulativeTransformation());
                C7Vector objCTMI(objCTM.getInverse());
                objCTM.X += absoluteTransl;
                objCTM = objCTMI * objCTM;
                for (int i = 0; i < int(vertexSel.size()); i++)
                {
                    C3Vector v(
                        GuiApp::mainWindow->editModeContainer->getShapeEditMode()->getEditionVertex(vertexSel[i]));
                    GuiApp::mainWindow->editModeContainer->getShapeEditMode()->setEditionVertex(vertexSel[i],
                                                                                                objCTM * v);
                }
            }
            if (GuiApp::getEditModeType() & PATH_EDIT_MODE_OLD)
            { // Path point shifting
                double prevPos[2] = {double(previousMousePosition.x), double(previousMousePosition.y)};
                double pos[2] = {double(mousePosition.x), double(mousePosition.y)};
                double screenHalfSizes[2] = {double(activeWinSize.x) / 2.0, double(activeWinSize.y) / 2.0};
                double halfSizes[2];
                double ratio = screenHalfSizes[0] / screenHalfSizes[1];
                if (perspective)
                {
                    if (ratio > 1.0)
                    {
                        double a = 2.0 * atan(tan(camera->getViewAngle() / 2.0) / ratio);
                        halfSizes[0] = camera->getViewAngle() / 2.0;
                        halfSizes[1] = a / 2.0;
                    }
                    else
                    {
                        double a = 2.0 * atan(tan(camera->getViewAngle() / 2.0) * ratio);
                        halfSizes[0] = a / 2.0;
                        halfSizes[1] = camera->getViewAngle() / 2.0;
                    }
                    prevPos[0] -= screenHalfSizes[0];
                    prevPos[1] -= screenHalfSizes[1];
                    pos[0] -= screenHalfSizes[0];
                    pos[1] -= screenHalfSizes[1];
                    prevPos[0] = atan((prevPos[0] / screenHalfSizes[0]) * tan(halfSizes[0]));
                    prevPos[1] = atan((prevPos[1] / screenHalfSizes[1]) * tan(halfSizes[1]));
                    pos[0] = atan((pos[0] / screenHalfSizes[0]) * tan(halfSizes[0]));
                    pos[1] = atan((pos[1] / screenHalfSizes[1]) * tan(halfSizes[1]));
                    screenHalfSizes[0] = halfSizes[0];
                    screenHalfSizes[1] = halfSizes[1];
                }
                else
                {
                    if (ratio > 1.0)
                    {
                        halfSizes[0] = camera->getOrthoViewSize() * 0.5;
                        halfSizes[1] = camera->getOrthoViewSize() * 0.5 / ratio;
                    }
                    else
                    {
                        halfSizes[1] = camera->getOrthoViewSize() * 0.5;
                        halfSizes[0] = camera->getOrthoViewSize() * 0.5 * ratio;
                    }
                    prevPos[0] -= screenHalfSizes[0];
                    prevPos[1] -= screenHalfSizes[1];
                    pos[0] -= screenHalfSizes[0];
                    pos[1] -= screenHalfSizes[1];
                }
                C3Vector centerPos(centerPosition);

                CPath_old* path = GuiApp::mainWindow->editModeContainer->getEditModePath_old();
                path->transformSelectedPathPoints(camera->getFullCumulativeTransformation().getMatrix(), centerPos,
                                                  prevPos, pos, screenHalfSizes, halfSizes, perspective, eventID);
            }
        }
    }
    // ****************************************************************************

    // Camera opening angle...
    // ****************************************************************************
    if (navigationMode == sim_navigation_cameraangle)
    {
        double zoomFactor = (double)(previousMousePosition.y - mousePosition.y) * 0.005;
        if (perspective)
        {
            double newViewAngle = camera->getViewAngle() + zoomFactor;
            tt::limitValue(10.0 * degToRad, 135.0 * degToRad,
                           newViewAngle); // with 90 degrees, objects disappear!! Really??? Changed to 135 on 2010/11/12
            camera->setViewAngle(newViewAngle);
            if (cameraParentProxy != nullptr)
            { // We report the same camera opening to all cameras attached to cameraPrentProxy
                for (size_t i = 0; i < cameraParentProxy->getChildCount(); i++)
                {
                    CSceneObject* child = cameraParentProxy->getChildFromIndex(i);
                    if (child->getObjectType() == sim_sceneobject_camera)
                        ((CCamera*)child)->setViewAngle(newViewAngle);
                }
            }
        }
        else
        {
            camera->setOrthoViewSize(camera->getOrthoViewSize() * (1.0 + zoomFactor));
            if (cameraParentProxy != nullptr)
            { // We report the same camera opening to all cameras attached to cameraPrentProxy
                for (size_t i = 0; i < cameraParentProxy->getChildCount(); i++)
                {
                    CSceneObject* child = cameraParentProxy->getChildFromIndex(i);
                    if (child->getObjectType() == sim_sceneobject_camera)
                        ((CCamera*)child)->setOrthoViewSize(camera->getOrthoViewSize() * (1.0 + zoomFactor));
                }
            }
        }
    }
    // ****************************************************************************
}
#endif
