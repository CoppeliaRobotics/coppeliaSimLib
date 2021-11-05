#include "simInternal.h"
#include "simulation.h"
#include "graph.h"
#include "tt.h"
#include "graphingRoutines_old.h"
#include "gV.h"
#include "threadPool_old.h"
#include "app.h"
#include "simStrings.h"
#include "vDateTime.h"
#include "persistentDataContainer.h"
#include "simFlavor.h"

const quint64 SIMULATION_DEFAULT_TIME_STEP_US[5]={200000,100000,50000,25000,10000};
const int SIMULATION_DEFAULT_PASSES_PER_RENDERING[5]={1,1,1,1,1};

const int SIMULATION_SPEED_MODIFIER_SEQUENCE[10]={1,2,5,10,20,40,80,160,320,640};
const int SIMULATION_SPEED_MODIFIER_START_INDEX[6]={5,4,3,2,1,3};

CSimulation::CSimulation()
{
    _stopRequestCounter=0;
    setUpDefaultValues();
}

CSimulation::~CSimulation()
{ // beware, the current world could be nullptr
    setUpDefaultValues();
}

void CSimulation::setUpDefaultValues()
{
    _dynamicContentVisualizationOnly=false;
    _simulationState=sim_simulation_stopped;
    _simulationTime_us=0;

    simulationTime_real_us=0;
    simulationTime_real_noCatchUp_us=0;
    clearSimulationTimeHistory_us();

    _defaultSimulationParameterIndex=2; // 2 is for default values
    _simulationTimeStep_us=SIMULATION_DEFAULT_TIME_STEP_US[_defaultSimulationParameterIndex];
    _simulationPassesPerRendering=SIMULATION_DEFAULT_PASSES_PER_RENDERING[_defaultSimulationParameterIndex]; 
    _speedModifierIndexOffset=0;
    _desiredFasterOrSlowerSpeed=0;
    _realTimeCoefficient=1.0;
    _simulationStepCount=0;
    _simulationTimeToPause_us=10000000;
    _pauseAtSpecificTime=false;
    _pauseAtError=true;
    _pauseOnErrorRequested=false;
    _hierarchyWasEnabledBeforeSimulation=false;
    _catchUpIfLate=false;
    _avoidBlocking=false;
    _initialValuesInitialized=false;
    _resetSimulationAtEnd=true;
    _removeNewObjectsAtSimulationEnd=true;
    _realTimeSimulation=false;
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
{ // careful here: this is called by this through App::wc->simulationAboutToStart!!
    _initialValuesInitialized=true;
    _initialPauseAtSpecificTime=_pauseAtSpecificTime;
    _speedModifierIndexOffset=0;
    _displayedWarningAboutNonDefaultParameters=false;
    _disableWarningsFlags=0;
    _dynamicContentVisualizationOnly=false;
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
{ // careful here: this is called by this through App::wc->simulationEnded!!
    TRACE_INTERNAL;

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
    TRACE_INTERNAL;
    if (isSimulationStopped())
    {
        App::setFullScreen(_fullscreenAtSimulationStart);
        CThreadPool_old::setSimulationEmergencyStop(false);
        CThreadPool_old::setRequestSimulationStop(false);
//        CScriptObject::emergencyStopButtonPressed=false;
        App::worldContainer->simulationAboutToStart();
        _speedModifierIndexOffset=0;
        _pauseOnErrorRequested=false;
        _realTimeCorrection_us=0;
        _setSimulationTime_us(0);
        simulationTime_real_us=0;
        simulationTime_real_noCatchUp_us=0;
        clearSimulationTimeHistory_us();
        _requestToStop=false;
        _requestToPause=false; 
        simulationTime_real_lastInMs=VDateTime::getTimeInMs();
        _simulationStepCount=0;
        setSimulationState(sim_simulation_advancing_firstafterstop);
        return(true);
    }
    else if (isSimulationPaused())
    {
        App::worldContainer->simulationAboutToResume();

        _realTimeCorrection_us=0;
        setSimulationState(sim_simulation_advancing_firstafterpause);
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
    TRACE_INTERNAL;
    if (getSimulationState()!=sim_simulation_stopped)
        App::setFullScreen(false);

    if ((getSimulationState()==sim_simulation_advancing_abouttostop)||
        (getSimulationState()==sim_simulation_advancing_lastbeforestop))
        return(true); // in this situation, we are stopping anyway!!
    if (getSimulationState()==sim_simulation_paused)
    {
        App::worldContainer->simulationAboutToResume();

        // Special case here: we have to change the state directly here (and not automatically in "advanceSimulationByOneStep")
        setSimulationState(sim_simulation_advancing_firstafterpause);
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
    if ((getSimulationState()!=sim_simulation_advancing_firstafterstop)&&
        (getSimulationState()!=sim_simulation_advancing_running)&&
        (getSimulationState()!=sim_simulation_advancing_firstafterpause))
        return(false); // in these situations, we are already about to pause or stopping anyway!!
    if (_requestToStop)
        return(false);
    _requestToPause=true;
    return(true);   
}

bool CSimulation::isSimulationRunning() const
{ 
    return((getSimulationState()&sim_simulation_advancing)!=0);
}

bool CSimulation::isSimulationStopped() const
{ 
    return(getSimulationState()==sim_simulation_stopped);
}

bool CSimulation::isSimulationPaused() const
{ 
    return(getSimulationState()==sim_simulation_paused);
}

void CSimulation::adjustRealTimeTimer_us(quint64 deltaTime)
{
    _realTimeCorrection_us+=deltaTime;
}

void CSimulation::advanceSimulationByOneStep()
{
    TRACE_INTERNAL;
    if (!isSimulationRunning())
        return;

    if ( _pauseAtError&&_pauseOnErrorRequested )
    {
        pauseSimulation();
        _pauseOnErrorRequested=false;
    }
    else
    {
        if ( _pauseAtSpecificTime&&(getSimulationTime_us()>=_simulationTimeToPause_us) )
        {
            pauseSimulation();
            _pauseAtSpecificTime=false;
        }
    }

    App::worldContainer->simulationAboutToStep();

    _simulationStepCount++;
    if (_simulationStepCount==1)
        _realTimeCorrection_us=0;

    _setSimulationTime_us(getSimulationTime_us()+getSimulationTimeStep_speedModified_us());

    int ct=VDateTime::getTimeInMs();
    quint64 drt=quint64((double(VDateTime::getTimeDiffInMs(simulationTime_real_lastInMs))*1000.0+double(_realTimeCorrection_us))*getRealTimeCoefficient_speedModified());
    simulationTime_real_us+=drt;
    simulationTime_real_noCatchUp_us+=drt;
    if ( (!_catchUpIfLate)&&(simulationTime_real_noCatchUp_us>getSimulationTime_us()+getSimulationTimeStep_speedModified_us()) )
        simulationTime_real_noCatchUp_us=getSimulationTime_us()+getSimulationTimeStep_speedModified_us();
    _realTimeCorrection_us=0;
    simulationTime_real_lastInMs=ct;
    addToSimulationTimeHistory_us(getSimulationTime_us(),simulationTime_real_us);

    if (getSimulationState()==sim_simulation_advancing_firstafterstop)
        setSimulationState(sim_simulation_advancing_running);
    else if (getSimulationState()==sim_simulation_advancing_running)
    {
        if (_requestToStop)
        {
            CThreadPool_old::setRequestSimulationStop(true);
            setSimulationState(sim_simulation_advancing_abouttostop);
            _requestToStop=false;
        }
        else
        {
            if (_requestToPause)
            {
                setSimulationState(sim_simulation_advancing_lastbeforepause);
                _requestToPause=false;
            }
        }
    }
    else if (getSimulationState()==sim_simulation_advancing_lastbeforepause)
    {
        setSimulationState(sim_simulation_paused);
        App::worldContainer->simulationPaused();
    }
    else if (getSimulationState()==sim_simulation_advancing_firstafterpause)
    {
        setSimulationState(sim_simulation_advancing_running);
    }
    else if (getSimulationState()==sim_simulation_advancing_abouttostop)
    {
        if (CThreadPool_old::getThreadPoolThreadCount()==0)
            setSimulationState(sim_simulation_advancing_lastbeforestop);
    }
    else if (getSimulationState()==sim_simulation_advancing_lastbeforestop)
    {
        App::worldContainer->simulationAboutToEnd();
        CThreadPool_old::setSimulationEmergencyStop(false);
        CThreadPool_old::setRequestSimulationStop(false);
        setSimulationState(sim_simulation_stopped);
        App::worldContainer->simulationEnded(_removeNewObjectsAtSimulationEnd);
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

int CSimulation::getSimulationState() const
{ 
    return(_simulationState);
}

void CSimulation::setSimulationTimeStep_raw_us(quint64 dt)
{
    if (isSimulationStopped()&&(_defaultSimulationParameterIndex==5))
    {
        if (dt<100)
            dt=100;
        if (dt>10000000)
            dt=10000000;
        _simulationTimeStep_us=dt;
        App::setFullDialogRefreshFlag(); // so that the recorder dlg gets correctly refreshed
    }
}

quint64 CSimulation::getSimulationTimeStep_raw_us(int parameterIndex)
{ // parameterIndex is -1 by default
    if (parameterIndex==-1)
        parameterIndex=_defaultSimulationParameterIndex;
    if (parameterIndex==5)
        return(_simulationTimeStep_us);
    return(SIMULATION_DEFAULT_TIME_STEP_US[parameterIndex]);
}

quint64 CSimulation::getSimulationTimeStep_speedModified_us(int parameterIndex)
{ // parameterIndex is -1 by default
    quint64 v=getSimulationTimeStep_raw_us(parameterIndex);

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
    quint64 crt=simulationTime_real_noCatchUp_us+quint64(double(VDateTime::getTimeDiffInMs(simulationTime_real_lastInMs))*getRealTimeCoefficient_speedModified()*1000.0);
    return (getSimulationTime_us()+getSimulationTimeStep_speedModified_us()<crt);
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

void CSimulation::pushAllInitialEvents() const
{
    auto [event,data]=App::worldContainer->prepareEvent(EVENTTYPE_SIMULATIONCHANGED,-1,nullptr,false);
    data->appendMapObject_stringInt32("state",getSimulationState());
    data->appendMapObject_stringInt32("time",getSimulationTime_us()/1000);

    App::worldContainer->pushEvent(event);
}

void CSimulation::setSimulationState(int state)
{
    bool diff=(_simulationState!=state);
    if (diff)
    {
        _simulationState=state;
        if (App::worldContainer->getEnableEvents())
        {
            const char* cmd="state";
            auto [event,data]=App::worldContainer->prepareEvent(EVENTTYPE_SIMULATIONCHANGED,-1,cmd,true);
            data->appendMapObject_stringInt32(cmd,_simulationState);
            App::worldContainer->pushEvent(event);
        }
    }
}

void CSimulation::clearSimulationTimeHistory_us()
{
    simulationTime_history_us.clear();
    simulationTime_real_history_us.clear();
}

void CSimulation::addToSimulationTimeHistory_us(quint64 simTime,quint64 simTimeReal)
{
    simulationTime_history_us.push_back(simTime);
    simulationTime_real_history_us.push_back(simTimeReal);
    if (simulationTime_history_us.size()>10)
    {
        simulationTime_history_us.erase(simulationTime_history_us.begin());
        simulationTime_real_history_us.erase(simulationTime_real_history_us.begin());
    }
}

bool CSimulation::getSimulationTimeHistoryDurations_us(quint64& simTime,quint64& simTimeReal)
{
    if (simulationTime_history_us.size()<2)
    {
        simTime=0;
        simTimeReal=0;
        return(false);
    }
    simTime=simulationTime_history_us[simulationTime_history_us.size()-1]-simulationTime_history_us[0];
    simTimeReal=simulationTime_real_history_us[simulationTime_real_history_us.size()-1]-simulationTime_real_history_us[0];
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
    if (isSimulationRunning())
    {
        if (_pauseAtError&&(!_requestToStop))
            _pauseOnErrorRequested=true;
    }
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

void CSimulation::setPauseTime_us(quint64 time)
{
    if (time<1000)
        time=1000;
    if (time>604800000000000)
        time=604800000000000;
    _simulationTimeToPause_us=time;
}

quint64 CSimulation::getPauseTime_us()
{
    return (_simulationTimeToPause_us);
}

bool CSimulation::getPauseAtSpecificTime()
{
    return(_pauseAtSpecificTime);
}

void CSimulation::setPauseAtSpecificTime(bool e)
{
    _pauseAtSpecificTime=e;
}

quint64 CSimulation::getSimulationTime_us() const
{
    return(_simulationTime_us);
}

void CSimulation::_setSimulationTime_us(quint64 t)
{
    bool diff=(_simulationTime_us!=t);
    if (diff)
    {
        _simulationTime_us=t;
        if (App::worldContainer->getEnableEvents())
        {
            const char* cmd="time";
            auto [event,data]=App::worldContainer->prepareEvent(EVENTTYPE_SIMULATIONCHANGED,-1,cmd,true);
            data->appendMapObject_stringInt32(cmd,_simulationTime_us/1000);
            App::worldContainer->pushEvent(event);
        }
    }
}

quint64 CSimulation::getSimulationTime_real_us() const
{
    return(simulationTime_real_us);
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
            if (App::currentWorld->simulation->isSimulationStopped()&&noEditMode )
            {
                App::currentWorld->simulation->setRealTimeSimulation(!App::currentWorld->simulation->getRealTimeSimulation());
                if (App::currentWorld->simulation->getRealTimeSimulation())
                    App::logMsg(sim_verbosity_msgs,IDSNS_TOGGLED_TO_REAL_TIME_MODE);
                else
                    App::logMsg(sim_verbosity_msgs,IDSNS_TOGGLED_TO_NON_REAL_TIME_MODE);
                App::setLightDialogRefreshFlag();
                App::setToolbarRefreshFlag(); // will trigger a refresh
                App::undoRedo_sceneChanged(""); // ************************** UNDO thingy **************************
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
                App::worldContainer->simulatorMessageQueue->addCommand(sim_message_simulation_start_resume_request,0,0,0,0,nullptr,0);
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
            App::worldContainer->simulatorMessageQueue->addCommand(sim_message_simulation_pause_request,0,0,0,0,nullptr,0);
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
            CThreadPool_old::forceAutomaticThreadSwitch_simulationEnding(); // 21/6/2014
            App::worldContainer->simulatorMessageQueue->addCommand(sim_message_simulation_stop_request,0,0,0,0,nullptr,0);
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
            App::currentWorld->dynamicsContainer->setDynamicEngineType(sim_physics_bullet,0);
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
            App::currentWorld->dynamicsContainer->setDynamicEngineType(sim_physics_bullet,283);
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
            App::currentWorld->dynamicsContainer->setDynamicEngineType(sim_physics_ode,0);
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
            App::currentWorld->dynamicsContainer->setDynamicEngineType(sim_physics_vortex,0);
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
            App::currentWorld->dynamicsContainer->setDynamicEngineType(sim_physics_newton,0);
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
            if (getSimulationTimeHistoryDurations_us(st_,str_))
            {
                double st=double(st_)/1000000.0;
                double str=double(str_)/1000000.0;
                if (abs((st-str)/str)>0.1)
                    txtRight="&&fg930"; // When current simulation speed is too slow
                else
                {
                    if ( abs((long long int)(getSimulationTime_us()-simulationTime_real_us)) > (long long int)(10*getSimulationTimeStep_speedModified_us()) )
                        txtRight="&&fg930"; // When simulation is behind
                }
            }
            txtRight+=gv::getHourMinuteSecondMilisecondStr(double(getSimulationTime_us())/1000000.0+0.0001)+" &&fg@@@(real time: ";
            if (abs(getRealTimeCoefficient_speedModified()-1.0)<0.01)
                txtRight+=gv::getHourMinuteSecondMilisecondStr(double(simulationTime_real_us)/1000000.0+0.0001)+")";
            else
            {
                txtRight+=gv::getHourMinuteSecondMilisecondStr(double(simulationTime_real_us)/1000000.0+0.0001)+" (x";
                txtRight+=tt::FNb(0,float(getRealTimeCoefficient_speedModified()),3,false)+"))";
            }
            if (simulationTime_real_us!=0)
                txtRight+=" (real time fact="+tt::FNb(0,double(getSimulationTime_us())/double(simulationTime_real_us),2,false)+")";
            txtRight+=" (dt="+tt::FNb(0,double(getSimulationTimeStep_speedModified_us())/1000.0,1,false)+" ms)";
        }
        else
        {
            txtRight="&&fg@@@"+gv::getHourMinuteSecondMilisecondStr(double(getSimulationTime_us())/1000000.0+0.0001);
            txtRight+=" (dt="+tt::FNb(0,double(getSimulationTimeStep_speedModified_us())/1000.0,1,false)+" ms)";
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
            ar << float(_simulationTimeStep_us)/1000000.0f;
            ar.flush();

            ar.storeDataName("St2");
            ar << _simulationTimeStep_us;
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
            ar << float(_simulationTimeToPause_us)/1000000.0f;
            ar.flush();

            ar.storeDataName("Pa2");
            ar << _simulationTimeToPause_us;
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
                        _simulationTimeStep_us=quint64(stp*1000000.1f);
                    }
                    if (theName.compare("St2")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _simulationTimeStep_us;
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
                        _simulationTimeToPause_us=quint64(w)*1000000;
                    }
                    if (theName.compare("Pa2")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _simulationTimeToPause_us;
                    }
                    if (noHit)
                        ar.loadUnknownData();
                }
            }
        }
    }
    else
    {
        bool exhaustiveXml=( (ar.getFileType()!=CSer::filetype_csim_xml_simplescene_file)&&(ar.getFileType()!=CSer::filetype_csim_xml_simplemodel_file) );
        if (ar.isStoring())
        {
            if (exhaustiveXml)
                ar.xmlAddNode_ulonglong("simulationTimeStep_ns",_simulationTimeStep_us);
            else
                ar.xmlAddNode_double("simulationTimeStep",double(_simulationTimeStep_us)/1000000.0);

            ar.xmlAddNode_int("simulationPassesPerRendering",_simulationPassesPerRendering);

            ar.xmlAddNode_comment(" 'simulationMode' tag: can be '200ms', '100ms', '50ms', '25ms', '10ms' or 'custom' ",exhaustiveXml);
            ar.xmlAddNode_enum("simulationMode",_defaultSimulationParameterIndex,0,"200ms",1,"100ms",2,"50ms",3,"25ms",4,"10ms",5,"custom");

            ar.xmlAddNode_double("realTimeCoefficient",_realTimeCoefficient);
            if (exhaustiveXml)
                ar.xmlAddNode_ulonglong("simulationTimeToPause_ns",_simulationTimeToPause_us);
            else
                ar.xmlAddNode_double("simulationTimeToPause",double(_simulationTimeToPause_us)/1000000.0);

            ar.xmlPushNewNode("switches");
            ar.xmlAddNode_bool("realTime",_realTimeSimulation);
            ar.xmlAddNode_bool("avoidBlocking",_avoidBlocking);
            ar.xmlAddNode_bool("pauseAtTime",_pauseAtSpecificTime);
            ar.xmlAddNode_bool("pauseAtError",_pauseAtError);
            ar.xmlAddNode_bool("catchUpIfLate",_catchUpIfLate);
            if (exhaustiveXml)
                ar.xmlAddNode_bool("fullScreen",_fullscreenAtSimulationStart);
            ar.xmlAddNode_bool("resetAtEnd",_resetSimulationAtEnd);
            ar.xmlAddNode_bool("removeNewObjectsAtEnd",_removeNewObjectsAtSimulationEnd);
            ar.xmlPopNode();
        }
        else
        {
            if (exhaustiveXml)
                ar.xmlGetNode_ulonglong("simulationTimeStep_ns",_simulationTimeStep_us);
            else
            {
                double d;
                if (ar.xmlGetNode_double("simulationTimeStep",d,exhaustiveXml))
                {
                    tt::limitDoubleValue(0.00001,10.0,d);
                    _simulationTimeStep_us=(unsigned long long)(d*1000000.9);
                }
            }

            if (ar.xmlGetNode_int("simulationPassesPerRendering",_simulationPassesPerRendering,exhaustiveXml))
                tt::limitValue(1,100,_simulationPassesPerRendering);

            ar.xmlGetNode_enum("simulationMode",_defaultSimulationParameterIndex,exhaustiveXml,"200ms",0,"100ms",1,"50ms",2,"25ms",3,"10ms",4,"custom",5);

            if (ar.xmlGetNode_double("realTimeCoefficient",_realTimeCoefficient,exhaustiveXml))
                tt::limitDoubleValue(0.01,100.0,_realTimeCoefficient);
            if (exhaustiveXml)
                ar.xmlGetNode_ulonglong("simulationTimeToPause_ns",_simulationTimeToPause_us);
            else
            {
                double d;
                if (ar.xmlGetNode_double("simulationTimeToPause_ns",d,exhaustiveXml))
                {
                    tt::limitDoubleValue(0.0001,10000000.0,d);
                    _simulationTimeToPause_us=(unsigned long long)(d*1000000.9);
                }
            }

            if (ar.xmlPushChildNode("switches",exhaustiveXml))
            {
                ar.xmlGetNode_bool("realTime",_realTimeSimulation,exhaustiveXml);
                ar.xmlGetNode_bool("avoidBlocking",_avoidBlocking,exhaustiveXml);
                ar.xmlGetNode_bool("pauseAtTime",_pauseAtSpecificTime,exhaustiveXml);
                ar.xmlGetNode_bool("pauseAtError",_pauseAtError,exhaustiveXml);
                ar.xmlGetNode_bool("catchUpIfLate",_catchUpIfLate,exhaustiveXml);
                if (exhaustiveXml)
                    ar.xmlGetNode_bool("fullScreen",_fullscreenAtSimulationStart,exhaustiveXml);
                ar.xmlGetNode_bool("resetAtEnd",_resetSimulationAtEnd,exhaustiveXml);
                ar.xmlGetNode_bool("removeNewObjectsAtEnd",_removeNewObjectsAtSimulationEnd,exhaustiveXml);
                ar.xmlPopNode();
            }
        }
    }
}

#ifdef SIM_WITH_GUI
bool CSimulation::showAndHandleEmergencyStopButton(bool showState,const char* scriptName)
{
    TRACE_INTERNAL;
    bool retVal=false;
    if (App::mainWindow!=nullptr)
    { // make sure we are not in headless mode
        bool res=App::uiThread->showOrHideEmergencyStop(showState,scriptName);
        if (showState&&res)
        { // stop button was pressed
            if (!isSimulationStopped())
            {
                CThreadPool_old::forceAutomaticThreadSwitch_simulationEnding(); // 21/6/2014
                CThreadPool_old::setSimulationEmergencyStop(true);
  //              if (getSimulationState()!=sim_simulation_advancing_lastbeforestop)
  //                  setSimulationState(sim_simulation_advancing_abouttostop);
            }
            retVal=true;
//            CScriptObject::emergencyStopButtonPressed=true;
        }
//        else
//            CScriptObject::emergencyStopButtonPressed=false;
    }
    return(retVal);
}

void CSimulation::addMenu(VMenu* menu)
{
    bool noEditMode=(App::getEditModeType()==NO_EDIT_MODE);
    bool simRunning=App::currentWorld->simulation->isSimulationRunning();
    bool simStopped=App::currentWorld->simulation->isSimulationStopped();
    bool simPaused=App::currentWorld->simulation->isSimulationPaused();
    bool canGoSlower=App::currentWorld->simulation->canGoSlower();
    bool canGoFaster=App::currentWorld->simulation->canGoFaster();
    if (simPaused)
        menu->appendMenuItem(App::mainWindow->getPlayViaGuiEnabled()&&noEditMode,false,SIMULATION_COMMANDS_START_RESUME_SIMULATION_REQUEST_SCCMD,IDS_RESUME_SIMULATION_MENU_ITEM);
    else
        menu->appendMenuItem(App::mainWindow->getPlayViaGuiEnabled()&&noEditMode&&(!simRunning),false,SIMULATION_COMMANDS_START_RESUME_SIMULATION_REQUEST_SCCMD,IDS_START_SIMULATION_MENU_ITEM);
    menu->appendMenuItem(App::mainWindow->getPauseViaGuiEnabled()&&noEditMode&&simRunning,false,SIMULATION_COMMANDS_PAUSE_SIMULATION_REQUEST_SCCMD,IDS_PAUSE_SIMULATION_MENU_ITEM);
    menu->appendMenuItem(App::mainWindow->getStopViaGuiEnabled()&&noEditMode&&(!simStopped),false,SIMULATION_COMMANDS_STOP_SIMULATION_REQUEST_SCCMD,IDS_STOP_SIMULATION_MENU_ITEM);
    menu->appendMenuSeparator();
    int version;
    int engine=App::currentWorld->dynamicsContainer->getDynamicEngineType(&version);
    menu->appendMenuItem(noEditMode&&simStopped,(engine==sim_physics_bullet)&&(version==0),SIMULATION_COMMANDS_TOGGLE_TO_BULLET_2_78_ENGINE_SCCMD,IDS_SWITCH_TO_BULLET_2_78_ENGINE_MENU_ITEM,true);
    menu->appendMenuItem(noEditMode&&simStopped,(engine==sim_physics_bullet)&&(version==283),SIMULATION_COMMANDS_TOGGLE_TO_BULLET_2_83_ENGINE_SCCMD,IDS_SWITCH_TO_BULLET_2_83_ENGINE_MENU_ITEM,true);
    menu->appendMenuItem(noEditMode&&simStopped,engine==sim_physics_ode,SIMULATION_COMMANDS_TOGGLE_TO_ODE_ENGINE_SCCMD,IDS_SWITCH_TO_ODE_ENGINE_MENU_ITEM,true);
    menu->appendMenuItem(noEditMode&&simStopped,engine==sim_physics_vortex,SIMULATION_COMMANDS_TOGGLE_TO_VORTEX_ENGINE_SCCMD,IDS_SWITCH_TO_VORTEX_ENGINE_MENU_ITEM,true);
    menu->appendMenuItem(noEditMode&&simStopped,engine==sim_physics_newton,SIMULATION_COMMANDS_TOGGLE_TO_NEWTON_ENGINE_SCCMD,IDS_SWITCH_TO_NEWTON_ENGINE_MENU_ITEM,true);
    if (CSimFlavor::getBoolVal(11))
    {
        menu->appendMenuSeparator();
        menu->appendMenuItem(noEditMode&&simStopped,App::currentWorld->simulation->getRealTimeSimulation(),SIMULATION_COMMANDS_TOGGLE_REAL_TIME_SIMULATION_SCCMD,IDSN_REAL_TIME_SIMULATION,true);
        menu->appendMenuItem(canGoSlower,false,SIMULATION_COMMANDS_SLOWER_SIMULATION_SCCMD,IDSN_SLOW_DOWN_SIMULATION);
        menu->appendMenuItem(canGoFaster,false,SIMULATION_COMMANDS_FASTER_SIMULATION_SCCMD,IDSN_SPEED_UP_SIMULATION);
        menu->appendMenuItem(simRunning&&(!(App::mainWindow->oglSurface->isPageSelectionActive()||App::mainWindow->oglSurface->isViewSelectionActive())),!App::mainWindow->getOpenGlDisplayEnabled(),SIMULATION_COMMANDS_TOGGLE_VISUALIZATION_SCCMD,IDSN_TOGGLE_VISUALIZATION,true);
        menu->appendMenuSeparator();
        if (App::mainWindow!=nullptr)
            menu->appendMenuItem(true,App::mainWindow->dlgCont->isVisible(SIMULATION_DLG),TOGGLE_SIMULATION_DLG_CMD,IDSN_SIMULATION_SETTINGS,true);
    }
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



