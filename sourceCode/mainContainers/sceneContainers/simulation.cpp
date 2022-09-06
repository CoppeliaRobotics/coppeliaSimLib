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
    _simulationTime=0.0;

    simulationTime_real=0.0;
    simulationTime_real_noCatchUp=0.0;
    _clearSimulationTimeHistory();

    _simulationTimeStep=0.05f;
    _simulationPassesPerRendering=1;
    _desiredFasterOrSlowerSpeed=0;
    _realTimeCoefficient=1.0;
    _simulationStepCount=0;
    _simulationTimeToPause=10.0f;
    _pauseAtSpecificTime=false;
    _pauseAtError=true;
    _pauseOnErrorRequested=false;
    _hierarchyWasEnabledBeforeSimulation=false;
    _initialValuesInitialized=false;
    _resetSimulationAtEnd=true;
    _removeNewObjectsAtSimulationEnd=true;
    _realTimeSimulation=false;
    _fullscreenAtSimulationStart=false;
    _speedModifierCount=0;
}

int CSimulation::getSpeedModifierCount() const
{
    return(_speedModifierCount);
}

void CSimulation::setSpeedModifierCount(int sm)
{
    while (sm!=_speedModifierCount)
    {
        if (sm>_speedModifierCount)
        {
            if (!_goFasterOrSlower(1))
                break;
        }
        else
        {
            if (!_goFasterOrSlower(-1))
                break;
        }
    }
}

double CSimulation::_getSpeedModifier_forRealTimeCoefficient() const
{
    float retVal=1.0;
    if (!isSimulationStopped())
    {
        if (_speedModifierCount>=0)
            retVal=pow(2.0,double(_speedModifierCount));
        else
            retVal=_simulationTimeStep/_initialSimulationTimeStep;
    }
    return(retVal);
}

void CSimulation::setFullscreenAtSimulationStart(bool f)
{
    _fullscreenAtSimulationStart=f;
}

bool CSimulation::getFullscreenAtSimulationStart() const
{
    return(_fullscreenAtSimulationStart);
}

void CSimulation::setRemoveNewObjectsAtSimulationEnd(bool r)
{
    _removeNewObjectsAtSimulationEnd=r;
}

bool CSimulation::getRemoveNewObjectsAtSimulationEnd() const
{
    return(_removeNewObjectsAtSimulationEnd);
}

void CSimulation::setResetSceneAtSimulationEnd(bool r)
{
    _resetSimulationAtEnd=r;    
}

bool CSimulation::getResetSceneAtSimulationEnd() const
{
    return(_resetSimulationAtEnd);
}

void CSimulation::simulationAboutToStart()
{ // careful here: this is called by this through App::wc->simulationAboutToStart!!
    _initialValuesInitialized=true;
    _initialPauseAtSpecificTime=_pauseAtSpecificTime;
    _speedModifierCount=0;
    _initialSimulationTimeStep=_simulationTimeStep;
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
    if ( (!App::currentWorld->dynamicsContainer->getSettingsAreDefault())||(!getSettingsAreDefault()) )
        App::logMsg(sim_verbosity_scriptwarnings,"Detected non-default settings (time steps and/or dyn. engine global settings).");
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
        _pauseAtSpecificTime=_initialPauseAtSpecificTime;
        _simulationTimeStep=_initialSimulationTimeStep;
    }
    _initialValuesInitialized=false;
    _speedModifierCount=0;
    _desiredFasterOrSlowerSpeed=0;

    #ifdef SIM_WITH_GUI
        if ( (App::mainWindow!=nullptr) && _hierarchyWasEnabledBeforeSimulation && App::userSettings->sceneHierarchyHiddenDuringSimulation)
            App::mainWindow->dlgCont->processCommand(OPEN_HIERARCHY_DLG_CMD);
    #endif
}

bool CSimulation::getDynamicContentVisualizationOnly() const
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

bool CSimulation::canGoSlower() const
{
    bool retVal=false;
    if (isSimulationRunning())
    {
        if (_speedModifierCount>0)
            retVal=true;
        else
        {
            float newDt=_getNewTimeStep(_speedModifierCount-1);
            retVal=(newDt!=0.0);
        }
    }
    return(retVal);
}

bool CSimulation::canGoFaster() const
{
    bool retVal=false;
    if (isSimulationRunning())
        retVal=(_speedModifierCount<6);
    return(retVal);
}

bool CSimulation::getSettingsAreDefault() const
{
    float dt=_simulationTimeStep;
    if (!isSimulationStopped())
        dt=_initialSimulationTimeStep;
    return(fabs(dt-0.05)<0.0001);
}

bool CSimulation::startOrResumeSimulation()
{
    TRACE_INTERNAL;
    if (isSimulationStopped())
    {
        App::setFullScreen(_fullscreenAtSimulationStart);
        CThreadPool_old::setSimulationEmergencyStop(false);
        CThreadPool_old::setRequestSimulationStop(false);
        App::worldContainer->simulationAboutToStart();
        _pauseOnErrorRequested=false;
        _realTimeCorrection=0.0;
        _setSimulationTime(0.0);
        simulationTime_real=0.0;
        simulationTime_real_noCatchUp=0.0;
        _clearSimulationTimeHistory();
        _requestToStop=false;
        _requestToPause=false; 
        simulationTime_real_lastInMs=(int)VDateTime::getTimeInMs();
        _simulationStepCount=0;
        setSimulationState(sim_simulation_advancing_firstafterstop);
        return(true);
    }
    else if (isSimulationPaused())
    {
        App::worldContainer->simulationAboutToResume();

        _realTimeCorrection=0.0;
        setSimulationState(sim_simulation_advancing_firstafterpause);
        simulationTime_real_lastInMs=(int)VDateTime::getTimeInMs();
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
        timeInMsWhenStopWasPressed=(int)VDateTime::getTimeInMs();
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

void CSimulation::adjustRealTimeTimer(float deltaTime)
{
    _realTimeCorrection+=deltaTime;
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
        if ( _pauseAtSpecificTime&&(getSimulationTime()>=_simulationTimeToPause) )
        {
            pauseSimulation();
            _pauseAtSpecificTime=false;
        }
    }

    App::worldContainer->simulationAboutToStep();

    _simulationStepCount++;
    if (_simulationStepCount==1)
        _realTimeCorrection=0.0;

    _setSimulationTime(getSimulationTime()+getTimeStep());

    int ct=(int)VDateTime::getTimeInMs();
    float drt=(float(VDateTime::getTimeDiffInMs(simulationTime_real_lastInMs))/1000.0f+_realTimeCorrection)*getRealTimeCoeff();
    simulationTime_real+=drt;
    simulationTime_real_noCatchUp+=drt;
    if (simulationTime_real_noCatchUp>getSimulationTime()+getTimeStep())
        simulationTime_real_noCatchUp=getSimulationTime()+getTimeStep();
    _realTimeCorrection=0.0f;
    simulationTime_real_lastInMs=ct;
    _addToSimulationTimeHistory(getSimulationTime(),simulationTime_real);

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
        _goFasterOrSlower(1);
        _desiredFasterOrSlowerSpeed--;
    }
    while (_desiredFasterOrSlowerSpeed<0)
    {
        _goFasterOrSlower(-1);
        _desiredFasterOrSlowerSpeed++;
    }
}

int CSimulation::getSimulationState() const
{ 
    return(_simulationState);
}

void CSimulation::setTimeStep(float dt)
{
    if (isSimulationStopped())
    {
        if (dt<0.0001f)
            dt=0.0001f;
        if (dt>10.0f)
            dt=10.0f;
        _simulationTimeStep=dt;
        App::setFullDialogRefreshFlag();
    }
}

float CSimulation::getTimeStep() const
{
    return(_simulationTimeStep);
}

int CSimulation::getPassesPerRendering() const
{
    int retVal=_simulationPassesPerRendering;
    if (_speedModifierCount>0)
        retVal*=int(pow(2.0,double(_speedModifierCount))+0.1);
    return(retVal);
}

double CSimulation::getRealTimeCoeff() const
{
    return(_realTimeCoefficient*_getSpeedModifier_forRealTimeCoefficient());
}

void CSimulation::setIsRealTimeSimulation(bool realTime)
{
    if (isSimulationStopped())
        _realTimeSimulation=realTime;
}

bool CSimulation::isRealTimeCalculationStepNeeded() const
{
    bool retVal=false;
    if (_realTimeSimulation&&isSimulationRunning())
    {
        float crt=simulationTime_real_noCatchUp+float(VDateTime::getTimeDiffInMs(simulationTime_real_lastInMs))*getRealTimeCoeff()/1000.0f;
        retVal=(getSimulationTime()+getTimeStep()<crt);
    }
    return(retVal);
}

bool CSimulation::getIsRealTimeSimulation() const
{
    return(_realTimeSimulation);
}

void CSimulation::setRealTimeCoeff(double coeff)
{
    if (coeff<0.0)
        coeff=0.0;
    if (coeff>100.0)
        coeff=100.0;
    _realTimeCoefficient=coeff;
}

void CSimulation::setPassesPerRendering(int n)
{
    if (isSimulationStopped())
    {
        tt::limitValue(1,200,n);
        _simulationPassesPerRendering=n;
    }
}

void CSimulation::pushGenesisEvents() const
{
    auto [event,data]=App::worldContainer->prepareEvent(EVENTTYPE_SIMULATIONCHANGED,-1,nullptr,false);
    data->appendMapObject_stringInt32("state",getSimulationState());
    data->appendMapObject_stringInt32("time",int(getSimulationTime()*1000.0f));

    App::worldContainer->pushEvent(event);
}

void CSimulation::setSimulationState(int state)
{
    bool diff=(_simulationState!=state);
    if (diff)
    {
        _simulationState=state;
        if (App::worldContainer->getEventsEnabled())
        {
            const char* cmd="state";
            auto [event,data]=App::worldContainer->prepareEvent(EVENTTYPE_SIMULATIONCHANGED,-1,cmd,true);
            data->appendMapObject_stringInt32(cmd,_simulationState);
            App::worldContainer->pushEvent(event);
        }
    }
}

void CSimulation::_clearSimulationTimeHistory()
{
    simulationTime_history.clear();
    simulationTime_real_history.clear();
}

void CSimulation::_addToSimulationTimeHistory(float simTime,float simTimeReal)
{
    simulationTime_history.push_back(simTime);
    simulationTime_real_history.push_back(simTimeReal);
    if (simulationTime_history.size()>10)
    {
        simulationTime_history.erase(simulationTime_history.begin());
        simulationTime_real_history.erase(simulationTime_real_history.begin());
    }
}

bool CSimulation::_getSimulationTimeHistoryDurations(float& simTime,float& simTimeReal) const
{
    bool retVal=false;
    if (simulationTime_history.size()<2)
    {
        simTime=0.0;
        simTimeReal=0.0;
    }
    else
    {
        simTime=simulationTime_history[simulationTime_history.size()-1]-simulationTime_history[0];
        simTimeReal=simulationTime_real_history[simulationTime_real_history.size()-1]-simulationTime_real_history[0];
        retVal=true;
    }
    return(retVal);
}

void CSimulation::setPauseAtError(bool br)
{
    _pauseAtError=br;
}

bool CSimulation::getPauseAtError() const
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

void CSimulation::setPauseTime(float time)
{
    if (time<0.001f)
        time=0.001f;
    if (time>604800.0f)
        time=604800.0f;
    _simulationTimeToPause=time;
}

float CSimulation::getPauseTime() const
{
    return (_simulationTimeToPause);
}

bool CSimulation::getPauseAtSpecificTime() const
{
    return(_pauseAtSpecificTime);
}

void CSimulation::setPauseAtSpecificTime(bool e)
{
    _pauseAtSpecificTime=e;
}

float CSimulation::getSimulationTime() const
{
    return(_simulationTime);
}

void CSimulation::_setSimulationTime(float t)
{
    bool diff=(_simulationTime!=t);
    if (diff)
    {
        _simulationTime=t;
        if (App::worldContainer->getEventsEnabled())
        {
            const char* cmd="time";
            auto [event,data]=App::worldContainer->prepareEvent(EVENTTYPE_SIMULATIONCHANGED,-1,cmd,true);
            data->appendMapObject_stringInt32(cmd,int(_simulationTime)*1000);
            App::worldContainer->pushEvent(event);
        }
    }
}

float CSimulation::getSimulationTime_real() const
{
    return(simulationTime_real);
}

float CSimulation::_getNewTimeStep(int newSpeedModifierCount) const
{
    float ddt=App::currentWorld->dynamicsContainer->getEffectiveStepSize();
    float dt=_simulationTimeStep;
    if (!isSimulationStopped())
        dt=_initialSimulationTimeStep;
    for (int i=0;i<-newSpeedModifierCount;i++)
    {
        dt*=0.5f;
        if ((dt*1.01f-ddt)<0.0)
            return(0.0);
        if ( fmod(dt*1.00001f,ddt)>ddt*0.01f )
        {
            float oldDt=dt;
            dt=ddt;
            while (dt+ddt<oldDt)
                dt+=ddt;
        }
    }
    return(dt);
}

bool CSimulation::_goFasterOrSlower(int action)
{
    bool retVal=false;
    if (action<0)
    { // We wanna go slower
        if (_speedModifierCount>0)
        {
            _speedModifierCount--;
            retVal=true;
        }
        else
        {
            float newDt=_getNewTimeStep(_speedModifierCount-1);
            if (newDt!=0.0)
            {
                _speedModifierCount--;
                _simulationTimeStep=newDt;
                retVal=true;
            }
        }
    }
    if (action>0)
    { // We wanna go faster
        if (canGoFaster())
        {
            _speedModifierCount++;
            if (_speedModifierCount<=0)
                _simulationTimeStep=_getNewTimeStep(_speedModifierCount);
            retVal=true;
        }
    }
    if (retVal)
    {
        App::setLightDialogRefreshFlag();
        App::setToolbarRefreshFlag();
    }
    return(retVal);
}

void CSimulation::incrementStopRequestCounter()
{
    _stopRequestCounter++;
}

int CSimulation::getStopRequestCounter() const
{
    return(_stopRequestCounter);
}

bool CSimulation::didStopRequestCounterChangeSinceSimulationStart() const
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
                App::currentWorld->simulation->setIsRealTimeSimulation(!App::currentWorld->simulation->getIsRealTimeSimulation());
                if (App::currentWorld->simulation->getIsRealTimeSimulation())
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
    if (commandID==SIMULATION_COMMANDS_TOGGLE_TO_MUJOCO_ENGINE_SCCMD)
    {
        if (!VThread::isCurrentThreadTheUiThread())
        { // we are NOT in the UI thread. We execute the command now:
            App::currentWorld->dynamicsContainer->setDynamicEngineType(sim_physics_mujoco,0);
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

bool CSimulation::getInfo(std::string& txtLeft,std::string& txtRight,int& index) const
{
    bool retVal=false;
    if (!isSimulationStopped())
    {
        if (index==0)
        {
            txtLeft="Simulation time:";
            if (_realTimeSimulation)
            {
                txtRight="";
                float st,str;
                if (_getSimulationTimeHistoryDurations(st,str))
                {
                    if (abs((st-str)/str)>0.1f)
                        txtRight="&&fg930"; // When current simulation speed is too slow
                    else
                    {
                        if ( abs(getSimulationTime()-simulationTime_real) > 10.0f*getTimeStep() )
                            txtRight="&&fg930"; // When simulation is behind
                    }
                }
                txtRight+=gv::getHourMinuteSecondMilisecondStr(getSimulationTime()+0.0001f)+" &&fg@@@(real time: ";
                if (abs(getRealTimeCoeff()-1.0)<0.01f)
                    txtRight+=gv::getHourMinuteSecondMilisecondStr(simulationTime_real+0.0001f)+")";
                else
                {
                    txtRight+=gv::getHourMinuteSecondMilisecondStr(simulationTime_real+0.0001f)+" (x";
                    txtRight+=tt::FNb(0,float(getRealTimeCoeff()),3,false)+"))";
                }
                if (simulationTime_real!=0.0)
                    txtRight+=" (real time fact="+tt::FNb(0,getSimulationTime()/simulationTime_real,2,false)+")";
                txtRight+=" (dt="+tt::FNb(0,getTimeStep()*1000.0f,1,false)+" ms)";
            }
            else
            {
                txtRight="&&fg@@@"+gv::getHourMinuteSecondMilisecondStr(getSimulationTime()+0.0001f);
                txtRight+=" (dt="+tt::FNb(0,getTimeStep()*1000.0f,1,false)+" ms, ppf="+std::to_string(getPassesPerRendering())+")";
            }
            retVal=true;
        }
        else
            index=0;
        index++;
    }
    return(retVal);
}

void CSimulation::serialize(CSer& ar)
{
    if (ar.isBinary())
    {
        if (ar.isStoring())
        {       // Storing
            ar.storeDataName("Sts"); // for backward compatibility (03/03/2016), keep before St2
            ar << _simulationTimeStep;
            ar.flush();

            ar.storeDataName("St2"); // for backward compatibility (05/09/2022), keep before St3
            ar << quint64(_simulationTimeStep*1000000.0f);
            ar.flush();

            ar.storeDataName("St3");
            ar << _simulationTimeStep;
            ar.flush();

            ar.storeDataName("Spr");
            ar << _simulationPassesPerRendering;
            ar.flush();

            ar.storeDataName("Spi"); // for backward compatibility (05/09/2022)
            ar << int(5);
            ar.flush();

            ar.storeDataName("Ss2");
            unsigned char nothing=0;
            SIM_SET_CLEAR_BIT(nothing,0,_realTimeSimulation);
            // 06.09.2022 SIM_SET_CLEAR_BIT(nothing,1,_avoidBlocking);
            SIM_SET_CLEAR_BIT(nothing,2,_pauseAtSpecificTime);
            SIM_SET_CLEAR_BIT(nothing,3,_pauseAtError);
            // 06.09.2022 SIM_SET_CLEAR_BIT(nothing,4,_catchUpIfLate);
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
            ar << _simulationTimeToPause;
            ar.flush();

            ar.storeDataName("Pa2"); // for backward compatibility (05/09/2022), keep before Pa3
            ar << quint64(_simulationTimeToPause*1000000.0f);
            ar.flush();

            ar.storeDataName("Pa3");
            ar << _simulationTimeToPause;
            ar.flush();

            ar.storeDataName(SER_END_OF_OBJECT);
        }
        else
        {       // Loading
            int byteQuantity;
            std::string theName="";
            int oldDefautParamsIndex;
            bool usingOldDefaultParams=true;
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
                        ar >> _simulationTimeStep;
                    }
                    if (theName.compare("St2")==0)
                    { // for backward compatibility (05/09/2022)
                        noHit=false;
                        ar >> byteQuantity;
                        quint64 stp;
                        ar >> stp;
                        _simulationTimeStep=float(stp)/1000000.0f;
                    }
                    if (theName.compare("St3")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _simulationTimeStep;
                        usingOldDefaultParams=false;
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
                        ar >> oldDefautParamsIndex;
                    }
                    if (theName=="Sst")
                    { // for backward compatibility (still in serialization version 15 or before)
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char nothing;
                        ar >> nothing;
                        _realTimeSimulation=SIM_IS_BIT_SET(nothing,0);
                        _pauseAtSpecificTime=SIM_IS_BIT_SET(nothing,2);
                        _pauseAtError=SIM_IS_BIT_SET(nothing,3);
                        // 06.09.2022 _catchUpIfLate=SIM_IS_BIT_SET(nothing,4);
                        bool defaultSimulationTimeStep=SIM_IS_BIT_SET(nothing,5);
                        _resetSimulationAtEnd=!SIM_IS_BIT_SET(nothing,6);
                        _removeNewObjectsAtSimulationEnd=!SIM_IS_BIT_SET(nothing,7);
                        if (defaultSimulationTimeStep)
                            oldDefautParamsIndex=2; // for default parameters
                        else
                            oldDefautParamsIndex=5; // for custom parameters
                    }
                    if (theName=="Ss2")
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        unsigned char nothing;
                        ar >> nothing;
                        _realTimeSimulation=SIM_IS_BIT_SET(nothing,0);
                        // 06.09.2022 _avoidBlocking=SIM_IS_BIT_SET(nothing,1);
                        _pauseAtSpecificTime=SIM_IS_BIT_SET(nothing,2);
                        _pauseAtError=SIM_IS_BIT_SET(nothing,3);
                        // 06.09.2022 _catchUpIfLate=SIM_IS_BIT_SET(nothing,4);
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
                        ar >> _simulationTimeToPause;
                    }
                    if (theName.compare("Pa2")==0)
                    { // for backward compatibility (05/09/2022)
                        noHit=false;
                        ar >> byteQuantity;
                        quint64 p;
                        ar >> p;
                        _simulationTimeToPause=float(p)/1000000.0f;
                    }
                    if (theName.compare("Pa3")==0)
                    {
                        noHit=false;
                        ar >> byteQuantity;
                        ar >> _simulationTimeToPause;
                    }
                    if (noHit)
                        ar.loadUnknownData();
                }
            }
            if (usingOldDefaultParams)
            {
                if ( (oldDefautParamsIndex>=0)&&(oldDefautParamsIndex<5) )
                {
                    const float SIMULATION_DEFAULT_TIME_STEP_OLD[5]={0.2f,0.1f,0.05f,0.025f,0.01f};
                    _simulationTimeStep=SIMULATION_DEFAULT_TIME_STEP_OLD[oldDefautParamsIndex];
                }
            }
        }
    }
    else
    {
        bool exhaustiveXml=( (ar.getFileType()!=CSer::filetype_csim_xml_simplescene_file)&&(ar.getFileType()!=CSer::filetype_csim_xml_simplemodel_file) );
        if (ar.isStoring())
        {
            ar.xmlAddNode_double("simTimeStep",_simulationTimeStep);

            ar.xmlAddNode_comment(" 'simulationTimeStep' tag: used for backward compatibility",exhaustiveXml);
            ar.xmlAddNode_double("simulationTimeStep",_simulationTimeStep);
            if (exhaustiveXml)
            {
                ar.xmlAddNode_comment(" 'simulationTimeStep_ns' tag: used for backward compatibility",exhaustiveXml);
                ar.xmlAddNode_ulonglong("simulationTimeStep_ns",quint64(_simulationTimeStep*1000000.0f)); // for backward compatibility (05.09.2022)
            }

            ar.xmlAddNode_int("simulationPassesPerRendering",_simulationPassesPerRendering);

            ar.xmlAddNode_comment(" 'simulationMode' tag: used for backward compatibility",exhaustiveXml);
            ar.xmlAddNode_int("simulationMode",5); // for backward compatibility (05.09.2022)

            ar.xmlAddNode_double("realTimeCoefficient",_realTimeCoefficient);

            ar.xmlAddNode_double("simulationTimeToPause",_simulationTimeToPause);
            if (exhaustiveXml)
            {
                ar.xmlAddNode_comment(" 'simulationTimeToPause_ns' tag: used for backward compatibility",exhaustiveXml);
                ar.xmlAddNode_ulonglong("simulationTimeToPause_ns",quint64(_simulationTimeToPause*1000000.0f)); // for backward compatibility (05.09.2022)
            }

            ar.xmlPushNewNode("switches");
            ar.xmlAddNode_bool("realTime",_realTimeSimulation);
            ar.xmlAddNode_bool("pauseAtTime",_pauseAtSpecificTime);
            ar.xmlAddNode_bool("pauseAtError",_pauseAtError);
            if (exhaustiveXml)
                ar.xmlAddNode_bool("fullScreen",_fullscreenAtSimulationStart);
            ar.xmlAddNode_bool("resetAtEnd",_resetSimulationAtEnd);
            ar.xmlAddNode_bool("removeNewObjectsAtEnd",_removeNewObjectsAtSimulationEnd);
            ar.xmlPopNode();
        }
        else
        {
            int oldDefautParamsIndex;
            bool usingOldDefaultParams=true;
            if (exhaustiveXml)
            { // for backward compatibility (05.09.2022)
                quint64 step;
                ar.xmlGetNode_ulonglong("simulationTimeStep_ns",step);
                _simulationTimeStep=float(step)/1000000.0f;
            }
            ar.xmlGetNode_float("simulationTimeStep",_simulationTimeStep,exhaustiveXml);
            if (ar.xmlGetNode_float("simTimeStep",_simulationTimeStep,exhaustiveXml))
                usingOldDefaultParams=false;

            if (ar.xmlGetNode_int("simulationPassesPerRendering",_simulationPassesPerRendering,exhaustiveXml))
                tt::limitValue(1,100,_simulationPassesPerRendering);

            ar.xmlGetNode_enum("simulationMode",oldDefautParamsIndex,exhaustiveXml,"200ms",0,"100ms",1,"50ms",2,"25ms",3,"10ms",4,"custom",5);

            if (ar.xmlGetNode_double("realTimeCoefficient",_realTimeCoefficient,exhaustiveXml))
                tt::limitDoubleValue(0.01,100.0,_realTimeCoefficient);

            if (exhaustiveXml)
            { // for backward compatibility (05.09.2022)
                quint64 p;
                ar.xmlGetNode_ulonglong("simulationTimeToPause_ns",p);
                _simulationTimeToPause=float(p)/1000000.0f;
            }
            ar.xmlGetNode_float("simulationTimeToPause",_simulationTimeToPause,exhaustiveXml);

            if (ar.xmlPushChildNode("switches",exhaustiveXml))
            {
                ar.xmlGetNode_bool("realTime",_realTimeSimulation,exhaustiveXml);
                ar.xmlGetNode_bool("pauseAtTime",_pauseAtSpecificTime,exhaustiveXml);
                ar.xmlGetNode_bool("pauseAtError",_pauseAtError,exhaustiveXml);
                if (exhaustiveXml)
                    ar.xmlGetNode_bool("fullScreen",_fullscreenAtSimulationStart,exhaustiveXml);
                ar.xmlGetNode_bool("resetAtEnd",_resetSimulationAtEnd,exhaustiveXml);
                ar.xmlGetNode_bool("removeNewObjectsAtEnd",_removeNewObjectsAtSimulationEnd,exhaustiveXml);
                ar.xmlPopNode();
            }
            if (usingOldDefaultParams)
            {
                if ( (oldDefautParamsIndex>=0)&&(oldDefautParamsIndex<5) )
                {
                    const float SIMULATION_DEFAULT_TIME_STEP_OLD[5]={0.2f,0.1f,0.05f,0.025f,0.01f};
                    _simulationTimeStep=SIMULATION_DEFAULT_TIME_STEP_OLD[oldDefautParamsIndex];
                }
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
            }
            retVal=true;
        }
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
    menu->appendMenuItem(noEditMode&&simStopped,engine==sim_physics_mujoco,SIMULATION_COMMANDS_TOGGLE_TO_MUJOCO_ENGINE_SCCMD,IDS_SWITCH_TO_MUJOCO_ENGINE_MENU_ITEM,true);
    menu->appendMenuSeparator();
    menu->appendMenuItem(noEditMode&&simStopped,App::currentWorld->simulation->getIsRealTimeSimulation(),SIMULATION_COMMANDS_TOGGLE_REAL_TIME_SIMULATION_SCCMD,IDSN_REAL_TIME_SIMULATION,true);
    menu->appendMenuItem(canGoSlower,false,SIMULATION_COMMANDS_SLOWER_SIMULATION_SCCMD,IDSN_SLOW_DOWN_SIMULATION);
    menu->appendMenuItem(canGoFaster,false,SIMULATION_COMMANDS_FASTER_SIMULATION_SCCMD,IDSN_SPEED_UP_SIMULATION);
    menu->appendMenuItem(simRunning&&(!(App::mainWindow->oglSurface->isPageSelectionActive()||App::mainWindow->oglSurface->isViewSelectionActive())),!App::mainWindow->getOpenGlDisplayEnabled(),SIMULATION_COMMANDS_TOGGLE_VISUALIZATION_SCCMD,IDSN_TOGGLE_VISUALIZATION,true);
    menu->appendMenuSeparator();
    if (App::mainWindow!=nullptr)
        menu->appendMenuItem(true,App::mainWindow->dlgCont->isVisible(SIMULATION_DLG),TOGGLE_SIMULATION_DLG_CMD,IDSN_SIMULATION_SETTINGS,true);
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
