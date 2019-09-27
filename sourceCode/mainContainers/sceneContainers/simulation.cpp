
#include "vrepMainHeader.h"
#include "funcDebug.h"
#include "v_rep_internal.h"
#include "simulation.h"
#include "graph.h"
#include "tt.h"
#include "graphingRoutines.h"
#include "gV.h"
#include "threadPool.h"
#include "app.h"
#include "v_repStrings.h"
#include "vDateTime.h"
#include "persistentDataContainer.h"
#include "simulationBase.h"

const quint64 SIMULATION_DEFAULT_TIME_STEP_NS[5]={200000,100000,50000,25000,10000};
const int SIMULATION_DEFAULT_PASSES_PER_RENDERING[5]={1,1,1,1,1};

const int SIMULATION_SPEED_MODIFIER_SEQUENCE[10]={1,2,5,10,20,40,80,160,320,640};
const int SIMULATION_SPEED_MODIFIER_START_INDEX[6]={5,4,3,2,1,3};

CSimulation::CSimulation()
{
    _stopRequestCounter=0;
    setUpDefaultValues();
}

CSimulation::~CSimulation()
{
    setUpDefaultValues();
}

void CSimulation::setUpDefaultValues()
{
    _dynamicContentVisualizationOnly=false;
    simulationState=sim_simulation_stopped;
    _simulationTime_ns=0;

    simulationTime_real_ns=0;
    simulationTime_real_noCatchUp_ns=0;
    clearSimulationTimeHistory_ns();

    _defaultSimulationParameterIndex=2; // 2 is for default values
    _simulationTimeStep_ns=SIMULATION_DEFAULT_TIME_STEP_NS[_defaultSimulationParameterIndex];
    _simulationPassesPerRendering=SIMULATION_DEFAULT_PASSES_PER_RENDERING[_defaultSimulationParameterIndex]; 
    _speedModifierIndexOffset=0;
    _desiredFasterOrSlowerSpeed=0;
    _realTimeCoefficient=1.0;
    _simulationStepCount=0;
    _simulationTimeToPause_ns=10000000;
    _pauseAtSpecificTime=false;
    _pauseAtError=false;
    _pauseOnErrorRequested=false;
    _hierarchyWasEnabledBeforeSimulation=false;
    _catchUpIfLate=false;
    _avoidBlocking=false;
    _initialValuesInitialized=false;
    _resetSimulationAtEnd=true;
    _removeNewObjectsAtSimulationEnd=true;
    _realTimeSimulation=false;
    _onlineMode=false;
    _threadedRenderingToggle=false;
    _fullscreenAtSimulationStart=false;
}

int CSimulation::getSpeedModifier_forCalcPassPerRendering()
{
    if (isSimulationStopped()||(_speedModifierIndexOffset<=0))
        return(1);
    return(1<<_speedModifierIndexOffset);
}

double CSimulation::_getSpeedModifier_forRealTimeCoefficient()
{
    if (isSimulationStopped()||(_speedModifierIndexOffset==0))
        return(1.0);

    int indStart=SIMULATION_SPEED_MODIFIER_START_INDEX[_defaultSimulationParameterIndex];
    int ind=indStart+_speedModifierIndexOffset;
    double init=double(SIMULATION_SPEED_MODIFIER_SEQUENCE[indStart]);
    double now=double(SIMULATION_SPEED_MODIFIER_SEQUENCE[ind]);
    return(now/init);
}

void CSimulation::setFullscreenAtSimulationStart(bool f)
{
    _fullscreenAtSimulationStart=f;
}

bool CSimulation::getFullscreenAtSimulationStart()
{
    return(_fullscreenAtSimulationStart);
}

void CSimulation::setAvoidBlocking(bool avoidBlocking)
{
    _avoidBlocking=avoidBlocking;
}

bool CSimulation::getAvoidBlocking()
{
    return(_avoidBlocking);
}

void CSimulation::setRemoveNewObjectsAtSimulationEnd(bool r)
{
    _removeNewObjectsAtSimulationEnd=r;
}

bool CSimulation::getRemoveNewObjectsAtSimulationEnd()
{
    return(_removeNewObjectsAtSimulationEnd);
}

void CSimulation::setResetSceneAtSimulationEnd(bool r)
{
    _resetSimulationAtEnd=r;    
}

bool CSimulation::getResetSceneAtSimulationEnd()
{
    return(_resetSimulationAtEnd);
}

bool CSimulation::getDisplayWarningAboutNonDefaultParameters()
{
    if (isSimulationRunning()&&(_defaultSimulationParameterIndex!=2)&&(!_displayedWarningAboutNonDefaultParameters)&&((_disableWarningsFlags&1)==0) )
    {
        _displayedWarningAboutNonDefaultParameters=true;
        return(true);
    }
    return(false);
}

void CSimulation::simulationAboutToStart()
{ // careful here: this is called by this through App::ct->simulationAboutToStart!!
    _initialValuesInitialized=true;
    _initialPauseAtSpecificTime=_pauseAtSpecificTime;
    _speedModifierIndexOffset=0;
    _displayedWarningAboutNonDefaultParameters=false;
    _disableWarningsFlags=0;
    _dynamicContentVisualizationOnly=false;
    _threadedRenderingToggle=false;
    _threadedRenderingMessageShown=false;
    _desiredFasterOrSlowerSpeed=0;
    _stopRequestCounterAtSimulationStart=_stopRequestCounter;
    #ifdef SIM_WITH_GUI
        if ( (App::mainWindow!=nullptr) && App::userSettings->sceneHierarchyHiddenDuringSimulation )
        {
            _hierarchyWasEnabledBeforeSimulation=App::mainWindow->oglSurface->isHierarchyEnabled();
            App::mainWindow->dlgCont->processCommand(CLOSE_HIERARCHY_DLG_CMD);
        }
    #endif
}

void CSimulation::simulationEnded()
{ // careful here: this is called by this through App::ct->simulationEnded!!
    FUNCTION_DEBUG;

    #ifdef SIM_WITH_GUI
        showAndHandleEmergencyStopButton(false,"");
    #endif
    _dynamicContentVisualizationOnly=false;
    #ifdef SIM_WITH_GUI
        if (App::mainWindow!=nullptr)
            App::mainWindow->simulationRecorder->stopRecording(false);
    #endif
    if (_initialValuesInitialized)
    {
        _pauseAtSpecificTime=_initialPauseAtSpecificTime; // we do not reset this!
        if (_resetSimulationAtEnd)
        {
        }
    }
    _speedModifierIndexOffset=0;
    _initialValuesInitialized=false;
    _threadedRenderingToggle=false;
    _desiredFasterOrSlowerSpeed=0;

    #ifdef SIM_WITH_GUI
        if ( (App::mainWindow!=nullptr) && _hierarchyWasEnabledBeforeSimulation && App::userSettings->sceneHierarchyHiddenDuringSimulation)
            App::mainWindow->dlgCont->processCommand(OPEN_HIERARCHY_DLG_CMD);
    #endif
}

void CSimulation::setDisableWarningsFlags(int mask)
{
    _disableWarningsFlags=mask;
}

int CSimulation::getDisableWarningsFlags()
{
    return(_disableWarningsFlags);
}

void CSimulation::setCatchUpIfLate(bool c)
{
    _catchUpIfLate=c;
}

bool CSimulation::getCatchUpIfLate()
{
    return(_catchUpIfLate);
}

void CSimulation::renderYour3DStuff(CViewableBase* renderingObject,int displayAttrib)
{

}

bool CSimulation::getDynamicContentVisualizationOnly()
{
    return(_dynamicContentVisualizationOnly);
}

void CSimulation::setDynamicContentVisualizationOnly(bool dynOnly)
{
    if ((!isSimulationStopped())||(!dynOnly))
        _dynamicContentVisualizationOnly=dynOnly;
    App::setFullDialogRefreshFlag(); // so we reflect the effect also to the toolbar button
    App::setToolbarRefreshFlag();
}

void CSimulation::setDefaultSimulationParameterIndex(int d)
{
    d=tt::getLimitedInt(0,5,d);
    _defaultSimulationParameterIndex=d;
    App::setFullDialogRefreshFlag(); // so that the recorder dlg gets correctly refreshed
}

int CSimulation::getDefaultSimulationParameterIndex()
{
    return(_defaultSimulationParameterIndex);
}


bool CSimulation::canGoSlower()
{
    if (!isSimulationRunning())
        return(false);
    if (App::getEditModeType()!=NO_EDIT_MODE)
        return(false);

    int indStart=SIMULATION_SPEED_MODIFIER_START_INDEX[_defaultSimulationParameterIndex];
    int ind=indStart+_speedModifierIndexOffset;
    return(ind>0);
}

bool CSimulation::canGoFaster()
{
    if (!isSimulationRunning())
        return(false);
    if (App::getEditModeType()!=NO_EDIT_MODE)
        return(false);

    int indStart=SIMULATION_SPEED_MODIFIER_START_INDEX[_defaultSimulationParameterIndex];
    int ind=indStart+_speedModifierIndexOffset;
    return(ind<9);
}

bool CSimulation::startOrResumeSimulation()
{
    FUNCTION_DEBUG;
    if (isSimulationStopped())
    {
        App::setFullScreen(_fullscreenAtSimulationStart);
        CThreadPool::setSimulationEmergencyStop(false);
        CThreadPool::setRequestSimulationStop(false);
        CLuaScriptObject::emergencyStopButtonPressed=false;
        App::ct->simulationAboutToStart();
        _speedModifierIndexOffset=0;
        _pauseOnErrorRequested=false;
        _realTimeCorrection_ns=0;
        _simulationTime_ns=0;
        simulationTime_real_ns=0;
        simulationTime_real_noCatchUp_ns=0;
        clearSimulationTimeHistory_ns();
        _requestToStop=false;
        _requestToPause=false; 
        simulationTime_real_lastInMs=VDateTime::getTimeInMs();
        _simulationStepCount=0;
        simulationState=sim_simulation_advancing_firstafterstop;
        return(true);
    }
    else if (isSimulationPaused())
    {
        App::ct->simulationAboutToResume();

        _realTimeCorrection_ns=0;
        simulationState=sim_simulation_advancing_firstafterpause;
        simulationTime_real_lastInMs=VDateTime::getTimeInMs();
        _requestToPause=false;
        return(true);
    }
    // Following not used anymore??
    _requestToPause=false;
    return(false);
}

bool CSimulation::stopSimulation()
{
    FUNCTION_DEBUG;
    if (simulationState!=sim_simulation_stopped)
        App::setFullScreen(false);

    if ((simulationState==sim_simulation_advancing_abouttostop)||
        (simulationState==sim_simulation_advancing_lastbeforestop))
        return(true); // in this situation, we are stopping anyway!!
    if (simulationState==sim_simulation_paused)
    {
        App::ct->simulationAboutToResume();

        // Special case here: we have to change the state directly here (and not automatically in "advanceSimulationByOneStep")
        simulationState=sim_simulation_advancing_firstafterpause;
    }
    if (!_requestToStop)
    {
        timeInMsWhenStopWasPressed=VDateTime::getTimeInMs();
        _requestToStop=true;
    }
    return(true);
}

bool CSimulation::pauseSimulation()
{
    if ((simulationState!=sim_simulation_advancing_firstafterstop)&&
        (simulationState!=sim_simulation_advancing_running)&&
        (simulationState!=sim_simulation_advancing_firstafterpause))
        return(false); // in these situations, we are already about to pause or stopping anyway!!
    if (_requestToStop)
        return(false);
    _requestToPause=true;
    return(true);   
}

bool CSimulation::isSimulationRunning()
{ 
    return((simulationState&sim_simulation_advancing)!=0);
}

bool CSimulation::isSimulationStopped()
{ 
    return(simulationState==sim_simulation_stopped); 
}

bool CSimulation::isSimulationPaused()
{ 
    return(simulationState==sim_simulation_paused); 
}

void CSimulation::adjustRealTimeTimer_ns(quint64 deltaTime)
{
    _realTimeCorrection_ns+=deltaTime;
}

void CSimulation::advanceSimulationByOneStep()
{
    FUNCTION_DEBUG;
    if (!isSimulationRunning())
        return;

    if ( _pauseAtError&&_pauseOnErrorRequested )
    {
        pauseSimulation();
        _pauseOnErrorRequested=false;
    }
    else
    {
        if ( _pauseAtSpecificTime&&(_simulationTime_ns>=_simulationTimeToPause_ns) )
        {
            pauseSimulation();
            _pauseAtSpecificTime=false;
        }
    }

    App::ct->simulationAboutToStep();

    _simulationStepCount++;
    if (_simulationStepCount==1)
        _realTimeCorrection_ns=0;

    _simulationTime_ns+=getSimulationTimeStep_speedModified_ns();

    int ct=VDateTime::getTimeInMs();
    quint64 drt=quint64((double(VDateTime::getTimeDiffInMs(simulationTime_real_lastInMs))*1000.0+double(_realTimeCorrection_ns))*getRealTimeCoefficient_speedModified());
    simulationTime_real_ns+=drt;
    simulationTime_real_noCatchUp_ns+=drt;  
    if ( (!_catchUpIfLate)&&(simulationTime_real_noCatchUp_ns>_simulationTime_ns+getSimulationTimeStep_speedModified_ns()) )
        simulationTime_real_noCatchUp_ns=_simulationTime_ns+getSimulationTimeStep_speedModified_ns();
    _realTimeCorrection_ns=0;
    simulationTime_real_lastInMs=ct;
    addToSimulationTimeHistory_ns(_simulationTime_ns,simulationTime_real_ns);

    if (simulationState==sim_simulation_advancing_firstafterstop)
        simulationState=sim_simulation_advancing_running;
    else if (simulationState==sim_simulation_advancing_running)
    {
        if (_requestToStop)
        {
            CThreadPool::setRequestSimulationStop(true);
            simulationState=sim_simulation_advancing_abouttostop;
            _requestToStop=false;
        }
        else
        {
            if (_requestToPause)
            {
                simulationState=sim_simulation_advancing_lastbeforepause;
                _requestToPause=false;
            }
        }
    }
    else if (simulationState==sim_simulation_advancing_lastbeforepause)
    {
        simulationState=sim_simulation_paused;
        App::ct->simulationPaused();
    }
    else if (simulationState==sim_simulation_advancing_firstafterpause)
    {
        simulationState=sim_simulation_advancing_running;
    }
    else if (simulationState==sim_simulation_advancing_abouttostop)
    {
        // Check if all threads have stopped
        if (CThreadPool::getThreadPoolThreadCount()==0)
            simulationState=sim_simulation_advancing_lastbeforestop;
    }
    else if (simulationState==sim_simulation_advancing_lastbeforestop)
    {
        App::ct->simulationAboutToEnd();
        CThreadPool::setSimulationEmergencyStop(false);
        CThreadPool::setRequestSimulationStop(false);
        CLuaScriptObject::emergencyStopButtonPressed=false;
        simulationState=sim_simulation_stopped;
        App::ct->simulationEnded(_removeNewObjectsAtSimulationEnd);
    }
    while (_desiredFasterOrSlowerSpeed>0)
    {
        goFasterOrSlower(1);
        _desiredFasterOrSlowerSpeed--;
    }
    while (_desiredFasterOrSlowerSpeed<0)
    {
        goFasterOrSlower(-1);
        _desiredFasterOrSlowerSpeed++;
    }
}

int CSimulation::getSimulationState()
{ 
    return(simulationState);
}

void CSimulation::setSimulationTimeStep_raw_ns(quint64 dt)
{
    if (isSimulationStopped()&&(_defaultSimulationParameterIndex==5))
    {
        if (dt<100)
            dt=100;
        if (dt>10000000)
            dt=10000000;
        _simulationTimeStep_ns=dt;
        App::setFullDialogRefreshFlag(); // so that the recorder dlg gets correctly refreshed
    }
}

quint64 CSimulation::getSimulationTimeStep_raw_ns(int parameterIndex)
{ // parameterIndex is -1 by default
    if (parameterIndex==-1)
        parameterIndex=_defaultSimulationParameterIndex;
    if (parameterIndex==5)
        return(_simulationTimeStep_ns);
    return(SIMULATION_DEFAULT_TIME_STEP_NS[parameterIndex]);
}

quint64 CSimulation::getSimulationTimeStep_speedModified_ns(int parameterIndex)
{ // parameterIndex is -1 by default
    quint64 v=getSimulationTimeStep_raw_ns(parameterIndex);

    if (isSimulationStopped()||(_speedModifierIndexOffset>=0))
        return(v);

    int indStart=SIMULATION_SPEED_MODIFIER_START_INDEX[_defaultSimulationParameterIndex];
    int ind=indStart+_speedModifierIndexOffset;
    quint64 init=SIMULATION_SPEED_MODIFIER_SEQUENCE[indStart];
    quint64 now=SIMULATION_SPEED_MODIFIER_SEQUENCE[ind];

    v*=now;
    v/=init;
    return(v);
}

int CSimulation::getSimulationPassesPerRendering_speedModified()
{
    return(getSimulationPassesPerRendering_raw()*getSpeedModifier_forCalcPassPerRendering());
}

double CSimulation::getRealTimeCoefficient_speedModified()
{
    return(_getRealTimeCoefficient_raw()*_getSpeedModifier_forRealTimeCoefficient());
}

void CSimulation::setOnlineMode(bool onlineMode)
{
    _onlineMode=onlineMode;
}

bool CSimulation::getOnlineMode()
{
    return(_onlineMode);
}

void CSimulation::setRealTimeSimulation(bool realTime)
{
    if (isSimulationStopped())
        _realTimeSimulation=realTime;
}

bool CSimulation::isRealTimeCalculationStepNeeded()
{
    if (!_realTimeSimulation)
        return(false);
    if (!isSimulationRunning())
        return(false);
    quint64 crt=simulationTime_real_noCatchUp_ns+quint64(double(VDateTime::getTimeDiffInMs(simulationTime_real_lastInMs))*getRealTimeCoefficient_speedModified()*1000.0);
    return (_simulationTime_ns+getSimulationTimeStep_speedModified_ns()<crt);
}

bool CSimulation::getRealTimeSimulation()
{
    return(_realTimeSimulation);
}

double CSimulation::_getRealTimeCoefficient_raw()
{
    return(_realTimeCoefficient);
}
void CSimulation::setRealTimeCoefficient_raw(double coeff)
{
    if (coeff<0.0)
        coeff=0.0;
    if (coeff>100.0)
        coeff=100.0;
    _realTimeCoefficient=coeff;
}

void CSimulation::setSimulationPassesPerRendering_raw(int n)
{
    if (_defaultSimulationParameterIndex==5)
    {
        tt::limitValue(1,200,n);
        _simulationPassesPerRendering=n;
    }
}

int CSimulation::getSimulationPassesPerRendering_raw()
{
    if (_defaultSimulationParameterIndex==5)
        return(_simulationPassesPerRendering);
    return(SIMULATION_DEFAULT_PASSES_PER_RENDERING[_defaultSimulationParameterIndex]);
}

void CSimulation::setSimulationStateDirect(int state)
{ // Careful with that function!
    simulationState=state;
}

void CSimulation::clearSimulationTimeHistory_ns()
{
    simulationTime_history_ns.clear();
    simulationTime_real_history_ns.clear();
}

void CSimulation::addToSimulationTimeHistory_ns(quint64 simTime,quint64 simTimeReal)
{
    simulationTime_history_ns.push_back(simTime);
    simulationTime_real_history_ns.push_back(simTimeReal);
    if (simulationTime_history_ns.size()>10)
    {
        simulationTime_history_ns.erase(simulationTime_history_ns.begin());
        simulationTime_real_history_ns.erase(simulationTime_real_history_ns.begin());
    }
}

bool CSimulation::getSimulationTimeHistoryDurations_ns(quint64& simTime,quint64& simTimeReal)
{
    if (simulationTime_history_ns.size()<2)
    {
        simTime=0;
        simTimeReal=0;
        return(false);
    }
    simTime=simulationTime_history_ns[simulationTime_history_ns.size()-1]-simulationTime_history_ns[0];
    simTimeReal=simulationTime_real_history_ns[simulationTime_real_history_ns.size()-1]-simulationTime_real_history_ns[0];
    return(true);
}

void CSimulation::setPauseAtError(bool br)
{
    _pauseAtError=br;
}

bool CSimulation::getPauseAtError()
{
    return(_pauseAtError);
}

void CSimulation::pauseOnErrorRequested()
{
    if (_pauseAtError&&(!_requestToStop))
        _pauseOnErrorRequested=true;
}

/*
bool CSimulation::getPauseOnErrorRequested()
{
    if (_pauseOnErrorRequested)
    {
        _pauseOnErrorRequested=false;
        return(true);
    }
    return(false);
}
*/

void CSimulation::setPauseTime_ns(quint64 time)
{
    if (time<1000)
        time=1000;
    if (time>604800000000000)
        time=604800000000000;
    _simulationTimeToPause_ns=time;
}

quint64 CSimulation::getPauseTime_ns()
{
    return (_simulationTimeToPause_ns);
}

bool CSimulation::getPauseAtSpecificTime()
{
    return(_pauseAtSpecificTime);
}

void CSimulation::setPauseAtSpecificTime(bool e)
{
    _pauseAtSpecificTime=e;
}

quint64 CSimulation::getSimulationTime_ns()
{
    return(_simulationTime_ns);
}

quint64 CSimulation::getSimulationTime_real_ns()
{
    return(simulationTime_real_ns);
}

bool CSimulation::goFasterOrSlower(int action)
{
    if (action==0)
        return(true);
    if (action<0)
    { // We wanna go slower
        if (canGoSlower())
        {
            _speedModifierIndexOffset--;
            App::setLightDialogRefreshFlag();
            App::setToolbarRefreshFlag();
            return(true);
        }
    }
    if (action>0)
    { // We wanna go faster
        if (canGoFaster())
        {
            _speedModifierIndexOffset++;
            App::setLightDialogRefreshFlag();
            App::setToolbarRefreshFlag();
            return(true);
        }
    }
    return(false);
}

int CSimulation::getSpeedModifierIndexOffset()
{
    return(_speedModifierIndexOffset);
}

bool CSimulation::setSpeedModifierIndexOffset(int offset)
{
    while (true)
    {
        if (!goFasterOrSlower(offset-_speedModifierIndexOffset))
            return(false);
        if (offset==_speedModifierIndexOffset)
            return(true);
    }
    return(false);
}


bool CSimulation::canToggleThreadedRendering()
{
    #ifdef SIM_WITH_GUI
        if (App::mainWindow!=nullptr)
        {
            if (App::mainWindow->simulationRecorder->getIsRecording())
                return(false);
        }
    #endif
    return(isSimulationRunning()&&(App::userSettings->threadedRenderingDuringSimulation>=0));
}

void CSimulation::toggleThreadedRendering(bool noWarningMessage)
{ // should only be called by the NON-UI thread
    #ifdef SIM_WITH_GUI
        if (App::mainWindow==nullptr)
            noWarningMessage=true;
    #else
        noWarningMessage=true;
    #endif
    _threadedRenderingToggle=!_threadedRenderingToggle;
    if (getThreadedRenderingIfSimulationWasRunning())
    {
        #ifdef SIM_WITH_GUI
            if (App::mainWindow!=nullptr)
                App::mainWindow->simulationRecorder->setRecorderEnabled(false); // video recorder not compatible with threaded rendering!
        #endif
    }
    if (getThreadedRendering()&&(!_threadedRenderingMessageShown))
    { // warning message
        _threadedRenderingMessageShown=true;
        if (!noWarningMessage)
        {
            CPersistentDataContainer cont(FILENAME_OF_USER_SETTINGS_IN_BINARY_FILE);
            std::string val;
            cont.readData("THREADEDRENDERING_WARNING_NO_SHOW",val);
            int intVal=0;
            tt::getValidInt(val,intVal);
            if (intVal<3)
            {
                #ifdef SIM_WITH_GUI
                    if (App::uiThread->messageBox_checkbox(App::mainWindow,IDSN_THREADED_RENDERING,IDSN_THREADED_RENDERING_WARNING,IDSN_DO_NOT_SHOW_THIS_MESSAGE_AGAIN_3X))
                    {
                        intVal++;
                        val=tt::FNb(intVal);
                        cont.writeData("THREADEDRENDERING_WARNING_NO_SHOW",val,!App::userSettings->doNotWritePersistentData);
                    }
                #endif
            }
        }
    }
    App::setToolbarRefreshFlag(); // will trigger a refresh
}

bool CSimulation::getThreadedRendering()
{
    return((!isSimulationStopped())&&getThreadedRenderingIfSimulationWasRunning());
}

bool CSimulation::getThreadedRenderingIfSimulationWasRunning()
{
    bool threaded=(App::userSettings->threadedRenderingDuringSimulation==1);
    if (_threadedRenderingToggle)
        threaded=!threaded;
    return(threaded);
}

void CSimulation::incrementStopRequestCounter()
{
    _stopRequestCounter++;
}

int CSimulation::getStopRequestCounter()
{
    return(_stopRequestCounter);
}

bool CSimulation::didStopRequestCounterChangeSinceSimulationStart()
{
    return(_stopRequestCounter!=_stopRequestCounterAtSimulationStart);
}

bool CSimulation::processCommand(int commandID)
{ // Return value is true if the command belonged to hierarchy menu and was executed
    if (commandID==SIMULATION_COMMANDS_TOGGLE_REAL_TIME_SIMULATION_SCCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            bool noEditMode=(App::getEditModeType()==NO_EDIT_MODE);
            if (App::ct->simulation->isSimulationStopped()&&noEditMode )
            {
                App::ct->simulation->setRealTimeSimulation(!App::ct->simulation->getRealTimeSimulation());
                if (App::ct->simulation->getRealTimeSimulation())
                    App::addStatusbarMessage(IDSNS_TOGGLED_TO_REAL_TIME_MODE);
                else
                    App::addStatusbarMessage(IDSNS_TOGGLED_TO_NON_REAL_TIME_MODE);
                App::setLightDialogRefreshFlag();
                App::setToolbarRefreshFlag(); // will trigger a refresh
                POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
            }
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }

    if (commandID==SIMULATION_COMMANDS_TOGGLE_ONLINE_SCCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            bool noEditMode=(App::getEditModeType()==NO_EDIT_MODE);
            if (App::ct->simulation->isSimulationStopped()&&noEditMode )
            {
                App::ct->simulation->setOnlineMode(!App::ct->simulation->getOnlineMode());
                if (App::ct->simulation->getOnlineMode())
                    App::addStatusbarMessage(IDSNS_TOGGLED_TO_ONLINE_MODE);
                else
                    App::addStatusbarMessage(IDSNS_TOGGLED_TO_OFFLINE_MODE);
                App::setLightDialogRefreshFlag();
                App::setToolbarRefreshFlag(); // will trigger a refresh
                POST_SCENE_CHANGED_ANNOUNCEMENT(""); // ************************** UNDO thingy **************************
            }
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }

    if (commandID==SIMULATION_COMMANDS_SLOWER_SIMULATION_SCCMD)
    { 
        _desiredFasterOrSlowerSpeed--;
        return(true);
    }
    if (commandID==SIMULATION_COMMANDS_FASTER_SIMULATION_SCCMD)
    {
        _desiredFasterOrSlowerSpeed++;
        return(true);
    }

#ifdef SIM_WITH_GUI
    if (commandID==SIMULATION_COMMANDS_TOGGLE_VISUALIZATION_SCCMD)
    {
        if (VThread::isCurrentThreadTheUiThread())
        { // We are in the UI thread. We execute the command now:
            App::mainWindow->setOpenGlDisplayEnabled(!App::mainWindow->getOpenGlDisplayEnabled());
        }
        else
        { // We are not in the UI thread. Execute the command via the UI thread:
            SUIThreadCommand cmdIn;
            SUIThreadCommand cmdOut;
            cmdIn.cmdId=TOGGLE_VISUALIZATION_UITHREADCMD;
            App::uiThread->executeCommandViaUiThread(&cmdIn,&cmdOut);
        }
        return(true);
    }
    if (commandID==SIMULATION_COMMANDS_THREADED_RENDERING_SCCMD)
    {
        if (App::mainWindow!=nullptr)
        {
            if (!App::mainWindow->simulationRecorder->getIsRecording())
            {
                if (!VThread::isCurrentThreadTheUiThread())
                { // we are NOT in the UI thread. We execute the command now:
                    toggleThreadedRendering(true);
                }
                else
                { // We are in the UI thread. Execute the command via the main thread:
                    SSimulationThreadCommand cmd;
                    cmd.cmdId=commandID;
                    App::appendSimulationThreadCommand(cmd);
                }
            }
        }
        return(true);
    }
    if (commandID==SIMULATION_COMMANDS_TOGGLE_DYNAMIC_CONTENT_VISUALIZATION_SCCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            if (!isSimulationStopped())
                setDynamicContentVisualizationOnly(!getDynamicContentVisualizationOnly());
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            App::appendSimulationThreadCommand(cmd);
        }
    }
#endif

    if (commandID==SIMULATION_COMMANDS_START_RESUME_SIMULATION_REQUEST_SCCMD)
    {
        if (App::getEditModeType()==NO_EDIT_MODE)
        {
            if (!VThread::isCurrentThreadTheUiThread())
            { // we are NOT in the UI thread. We execute the command now:
                App::ct->simulatorMessageQueue->addCommand(sim_message_simulation_start_resume_request,0,0,0,0,nullptr,0);
            }
            else
            { // We are in the UI thread. Execute the command via the main thread:
                SSimulationThreadCommand cmd;
                cmd.cmdId=commandID;
                App::appendSimulationThreadCommand(cmd);
            }
        }
        return(true);
    }
    if (commandID==SIMULATION_COMMANDS_PAUSE_SIMULATION_REQUEST_SCCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            App::ct->simulatorMessageQueue->addCommand(sim_message_simulation_pause_request,0,0,0,0,nullptr,0);
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }
    if (commandID==SIMULATION_COMMANDS_STOP_SIMULATION_REQUEST_SCCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            CThreadPool::forceAutomaticThreadSwitch_simulationEnding(); // 21/6/2014
            App::ct->simulatorMessageQueue->addCommand(sim_message_simulation_stop_request,0,0,0,0,nullptr,0);
            incrementStopRequestCounter();
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }
    if (commandID==SIMULATION_COMMANDS_TOGGLE_TO_BULLET_2_78_ENGINE_SCCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            App::ct->dynamicsContainer->setDynamicEngineType(sim_physics_bullet,0);
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }
    if (commandID==SIMULATION_COMMANDS_TOGGLE_TO_BULLET_2_83_ENGINE_SCCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            App::ct->dynamicsContainer->setDynamicEngineType(sim_physics_bullet,283);
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }
    if (commandID==SIMULATION_COMMANDS_TOGGLE_TO_ODE_ENGINE_SCCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            App::ct->dynamicsContainer->setDynamicEngineType(sim_physics_ode,0);
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }
    if (commandID==SIMULATION_COMMANDS_TOGGLE_TO_VORTEX_ENGINE_SCCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            App::ct->dynamicsContainer->setDynamicEngineType(sim_physics_vortex,0);
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }
    if (commandID==SIMULATION_COMMANDS_TOGGLE_TO_NEWTON_ENGINE_SCCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            App::ct->dynamicsContainer->setDynamicEngineType(sim_physics_newton,0);
        }
        else
        { // We are in the UI thread. Execute the command via the main thread:
            SSimulationThreadCommand cmd;
            cmd.cmdId=commandID;
            App::appendSimulationThreadCommand(cmd);
        }
        return(true);
    }

#ifdef SIM_WITH_GUI
    if (commandID==TOGGLE_SIMULATION_DLG_CMD)
    {
        if (VThread::isCurrentThreadTheUiThread())
        {
            if (App::mainWindow!=nullptr)
                App::mainWindow->dlgCont->toggle(SIMULATION_DLG);
        }
        return(true);
    }
#endif

    return(false);
}

bool CSimulation::getInfo(std::string& txtLeft,std::string& txtRight,int& index)
{
    if (isSimulationStopped())
        return(false);
    if (index==0)
    {
        txtLeft="Simulation time:";
        if (_realTimeSimulation)
        {
            txtRight="";//"&&fg060";
            quint64 st_,str_;
            if (getSimulationTimeHistoryDurations_ns(st_,str_))
            {
                double st=double(st_)/1000000.0;
                double str=double(str_)/1000000.0;
                if (abs((st-str)/str)>0.1)
                    txtRight="&&fg930"; // When current simulation speed is too slow
                else
                {
                    if ( abs((long long int)(_simulationTime_ns-simulationTime_real_ns)) > (long long int)(10*getSimulationTimeStep_speedModified_ns()) )
                        txtRight="&&fg930"; // When simulation is behind
                }
            }
            txtRight+=gv::getHourMinuteSecondMilisecondStr(double(_simulationTime_ns)/1000000.0+0.0001)+" &&fg@@@(real time: ";
            if (abs(getRealTimeCoefficient_speedModified()-1.0)<0.01)
                txtRight+=gv::getHourMinuteSecondMilisecondStr(double(simulationTime_real_ns)/1000000.0+0.0001)+")";
            else
            {
                txtRight+=gv::getHourMinuteSecondMilisecondStr(double(simulationTime_real_ns)/1000000.0+0.0001)+" (x";
                txtRight+=tt::FNb(0,float(getRealTimeCoefficient_speedModified()),3,false)+"))";
            }
            if (simulationTime_real_ns!=0)
                txtRight+=" (real time fact="+tt::FNb(0,double(_simulationTime_ns)/double(simulationTime_real_ns),2,false)+")";
            txtRight+=" (dt="+tt::FNb(0,double(getSimulationTimeStep_speedModified_ns())/1000.0,1,false)+" ms)";
        }
        else
        {
            txtRight="&&fg@@@"+gv::getHourMinuteSecondMilisecondStr(double(_simulationTime_ns)/1000000.0+0.0001);
            txtRight+=" (dt="+tt::FNb(0,double(getSimulationTimeStep_speedModified_ns())/1000.0,1,false)+" ms)";
        }
    }
    else
    {
        index=0;
        return(false);
    }
    index++;
    return(true);
}

void CSimulation::serialize(CSer& ar)
{
    if (ar.isBinary())
    {
        if (ar.isStoring())
        {       // Storing
            ar.storeDataName("Sts"); // for backward compatibility (03/03/2016), keep before St2
            ar << float(_simulationTimeStep_ns)/1000000.0f;
            ar.flush();

            ar.storeDataName("St2");
            ar << _simulationTimeStep_ns;
            ar.flush();

            ar.storeDataName("Spr");
            ar << _simulationPassesPerRendering;
            ar.flush();

            ar.storeDataName("Spi");
            ar << _defaultSimulationParameterIndex;
            ar.flush();

            ar.storeDataName("Ss2");
            unsigned char nothing=0;
            SIM_SET_CLEAR_BIT(nothing,0,_realTimeSimulation);
            SIM_SET_CLEAR_BIT(nothing,1,_avoidBlocking);
            SIM_SET_CLEAR_BIT(nothing,2,_pauseAtSpecificTime);
            SIM_SET_CLEAR_BIT(nothing,3,_pauseAtError);
            SIM_SET_CLEAR_BIT(nothing,4,_catchUpIfLate);
            SIM_SET_CLEAR_BIT(nothing,5,_fullscreenAtSimulationStart);
            SIM_SET_CLEAR_BIT(nothing,6,!_resetSimulationAtEnd);
            SIM_SET_CLEAR_BIT(nothing,7,!_removeNewObjectsAtSimulationEnd);

            ar << nothing;
            ar.flush();

            ar.storeDataName("Rtc"); // for backward compatibility (03/03/2016), keep before Rt2
            ar << float(_realTimeCoefficient);
            ar.flush();

            ar.storeDataName("Rt2");
            ar << _realTimeCoefficient;
            ar.flush();

            ar.storeDataName("Pat"); // for backward compatibility (03/03/2016), keep before Pa2
            ar << float(_simulationTimeToPause_ns)/1000000.0f;
            ar.flush();

            ar.storeDataName("Pa2");
            ar << _simulationTimeToPause_ns;
            ar.flush();

            ar.storeDataName(SER_END_OF_OBJECT);
        }
        else
        {       // Loading
            int byteQuantity;
            std::string theName="";
            while (theName.compare(SER_END_OF_OBJECT)!=0)
            {
                theName=ar.readDataName();
                if (theName.compare(SER_END_OF_OBJECT)!=0)
                {
                    bool noHit=true;
                    if (theName.compare("Sts")==0)
                    { // for backward compatibility (03/03/2016)
                        noHit=false;
                        ar >> byteQuantity;
                        float stp;
                        ar >> stp;
                        _simulationTimeStep_ns=quint64(stp*1000000.1f);
                    }
                    if (theName.compare("St2")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _simulationTimeStep_ns;
                    }
                    if (theName.compare("Spr")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _simulationPassesPerRendering;
                    }
                    if (theName.compare("Spi")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _defaultSimulationParameterIndex;
                    }
                    if (theName=="Sst")
                    { // for backward compatibility (still in serialization version 15 or before)
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char nothing;
                        ar >> nothing;
                        _realTimeSimulation=SIM_IS_BIT_SET(nothing,0);
                        _avoidBlocking=SIM_IS_BIT_SET(nothing,1);
                        _pauseAtSpecificTime=SIM_IS_BIT_SET(nothing,2);
                        _pauseAtError=SIM_IS_BIT_SET(nothing,3);
                        _catchUpIfLate=SIM_IS_BIT_SET(nothing,4);
                        bool defaultSimulationTimeStep=SIM_IS_BIT_SET(nothing,5);
                        _resetSimulationAtEnd=!SIM_IS_BIT_SET(nothing,6);
                        _removeNewObjectsAtSimulationEnd=!SIM_IS_BIT_SET(nothing,7);
                        if (defaultSimulationTimeStep)
                            _defaultSimulationParameterIndex=2; // for default parameters
                        else
                            _defaultSimulationParameterIndex=5; // for custom parameters
                    }
                    if (theName=="Ss2")
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char nothing;
                        ar >> nothing;
                        _realTimeSimulation=SIM_IS_BIT_SET(nothing,0);
                        _avoidBlocking=SIM_IS_BIT_SET(nothing,1);
                        _pauseAtSpecificTime=SIM_IS_BIT_SET(nothing,2);
                        _pauseAtError=SIM_IS_BIT_SET(nothing,3);
                        _catchUpIfLate=SIM_IS_BIT_SET(nothing,4);
                        _fullscreenAtSimulationStart=SIM_IS_BIT_SET(nothing,5);
                        _resetSimulationAtEnd=!SIM_IS_BIT_SET(nothing,6);
                        _removeNewObjectsAtSimulationEnd=!SIM_IS_BIT_SET(nothing,7);
                    }

                    if (theName.compare("Rt2")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _realTimeCoefficient;
                    }
                    if (theName.compare("Rtc")==0)
                    { // for backward compatibility (03/03/2016)
                        noHit=false;
                        ar >> byteQuantity;
                        float v;
                        ar >> v;
                        _realTimeCoefficient=double(v);
                    }
                    if (theName.compare("Pat")==0)
                    { // for backward compatibility (03/03/2016)
                        noHit=false;
                        ar >> byteQuantity;
                        float w;
                        ar >> w;
                        _simulationTimeToPause_ns=quint64(w)*1000000;
                    }
                    if (theName.compare("Pa2")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _simulationTimeToPause_ns;
                    }
                    if (noHit)
                        ar.loadUnknownData();
                }
            }
        }
    }
}

#ifdef SIM_WITH_GUI
void CSimulation::showAndHandleEmergencyStopButton(bool showState,const char* scriptName)
{
    FUNCTION_DEBUG;
    if (App::mainWindow!=nullptr)
    { // make sure we are not in headless mode
        bool res=App::uiThread->showOrHideEmergencyStop(showState,scriptName);
        if (showState&&res)
        { // stop button was pressed
            if (!isSimulationStopped())
            {
                CThreadPool::forceAutomaticThreadSwitch_simulationEnding(); // 21/6/2014
                CThreadPool::setSimulationEmergencyStop(true);
                if (getSimulationState()!=sim_simulation_advancing_lastbeforestop)
                    setSimulationStateDirect(sim_simulation_advancing_abouttostop);
            }
            CLuaScriptObject::emergencyStopButtonPressed=true;
        }
        else
            CLuaScriptObject::emergencyStopButtonPressed=false;
    }
}

void CSimulation::addMenu(VMenu* menu)
{
    CSimulationBase::handleVerSpec_addMenu(menu);
}

void CSimulation::keyPress(int key)
{
    if (key==CTRL_SPACE_KEY)
    {
        if (isSimulationRunning())
        {
            if (App::mainWindow->getStopViaGuiEnabled())
                processCommand(SIMULATION_COMMANDS_STOP_SIMULATION_REQUEST_SCCMD);
        }
        else
        {
            if (App::mainWindow->getPlayViaGuiEnabled())
                processCommand(SIMULATION_COMMANDS_START_RESUME_SIMULATION_REQUEST_SCCMD);
        }
    }
}

#endif

volatile int CSimulation::emergencyStopButtonThreadForMainScriptsLaunched=0;



