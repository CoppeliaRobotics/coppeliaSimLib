
#pragma once

#include "mainCont.h"
#include "vThread.h"
#include "ser.h"
#ifdef SIM_WITH_GUI
#include "vMenubar.h"
#endif

class CSimulation : public CMainCont
{
public:
    CSimulation();
    virtual ~CSimulation();
    void simulationAboutToStart();
    void simulationEnded();
    void renderYour3DStuff(CViewableBase* renderingObject,int displayAttrib);
    void setUpDefaultValues();
    bool startOrResumeSimulation();
    bool stopSimulation();
    bool pauseSimulation();
    bool isSimulationRunning();
    bool isSimulationPaused();
    bool isSimulationStopped();
    void advanceSimulationByOneStep();
    void setDefaultSimulationParameterIndex(int d);
    int getDefaultSimulationParameterIndex();
    void setRealTimeSimulation(bool realTime);
    bool getRealTimeSimulation();
    void setOnlineMode(bool onlineMode);
    bool getOnlineMode();
    void serialize(CSer& ar);

    quint64 getSimulationTime_ns();
    quint64 getSimulationTime_real_ns();
    void clearSimulationTimeHistory_ns();
    void addToSimulationTimeHistory_ns(quint64 simTime,quint64 simTimeReal);
    bool getSimulationTimeHistoryDurations_ns(quint64& simTime,quint64& simTimeReal);
    void setResetSceneAtSimulationEnd(bool r);
    bool getResetSceneAtSimulationEnd();
    void setRemoveNewObjectsAtSimulationEnd(bool r);
    bool getRemoveNewObjectsAtSimulationEnd();

    void setAvoidBlocking(bool avoidBlocking);
    bool getAvoidBlocking();

    void incrementStopRequestCounter();
    int getStopRequestCounter();
    bool didStopRequestCounterChangeSinceSimulationStart();

    bool canGoSlower();
    bool canGoFaster();
    bool goFasterOrSlower(int action);
    int getSpeedModifierIndexOffset();
    bool setSpeedModifierIndexOffset(int offset);
    bool canToggleThreadedRendering();
    void toggleThreadedRendering(bool noWarningMessage);
    bool getThreadedRendering();
    bool getThreadedRenderingIfSimulationWasRunning();
    int getSpeedModifier_forCalcPassPerRendering();

    void setSimulationTimeStep_raw_ns(quint64 dt);
    quint64 getSimulationTimeStep_raw_ns(int parameterIndex=-1);
    quint64 getSimulationTimeStep_speedModified_ns(int parameterIndex=-1);

    void setSimulationPassesPerRendering_raw(int n);
    int getSimulationPassesPerRendering_raw();
    int getSimulationPassesPerRendering_speedModified();

    void setRealTimeCoefficient_raw(double coeff);
    double getRealTimeCoefficient_speedModified();

    bool getDynamicContentVisualizationOnly();
    void setDynamicContentVisualizationOnly(bool dynOnly);

    void adjustRealTimeTimer_ns(quint64 deltaTime);

    void setFullscreenAtSimulationStart(bool f);
    bool getFullscreenAtSimulationStart();


    bool isRealTimeCalculationStepNeeded();

    void setSimulationStateDirect(int state);

    bool processCommand(int commandID);
    bool getInfo(std::string& txtLeft,std::string& txtRight,int& index);

    void setPauseAtError(bool br);
    bool getPauseAtError();
    void pauseOnErrorRequested();
//    bool getPauseOnErrorRequested();
    void setPauseTime_ns(quint64 time);
    quint64 getPauseTime_ns();
    bool getPauseAtSpecificTime();
    void setPauseAtSpecificTime(bool e);
    void setCatchUpIfLate(bool c);
    bool getCatchUpIfLate();

    int getSimulationState();

    bool getDisplayWarningAboutNonDefaultParameters();

    void setDisableWarningsFlags(int mask);
    int getDisableWarningsFlags();


#ifdef SIM_WITH_GUI
    void showAndHandleEmergencyStopButton(bool showState,const char* scriptName);
    void keyPress(int key);
    void addMenu(VMenu* menu);
#endif

private:
    double _getRealTimeCoefficient_raw();
    double _getSpeedModifier_forRealTimeCoefficient();

    bool _resetSimulationAtEnd;
    bool _removeNewObjectsAtSimulationEnd;

    bool _catchUpIfLate;
    bool _avoidBlocking;
    bool _fullscreenAtSimulationStart;

    quint64 _simulationTime_ns;
    quint64 simulationTime_real_ns;
    quint64 simulationTime_real_noCatchUp_ns;

    int simulationTime_real_lastInMs;
    quint64 _realTimeCorrection_ns;

    int _simulationStepCount;

    bool _threadedRenderingToggle;
    bool _threadedRenderingMessageShown;

    bool _displayedWarningAboutNonDefaultParameters;
    int _disableWarningsFlags;

    int _defaultSimulationParameterIndex;
    int _simulationPassesPerRendering;
    quint64 _simulationTimeStep_ns;
    int _speedModifierIndexOffset;
    int _desiredFasterOrSlowerSpeed;


    int timeInMsWhenStopWasPressed;

    bool _realTimeSimulation;
    bool _onlineMode; // not serialized
    double _realTimeCoefficient;

    quint64 _simulationTimeToPause_ns;
    bool _pauseAtSpecificTime;
    bool _pauseAtError;
    bool _pauseOnErrorRequested;
    bool _hierarchyWasEnabledBeforeSimulation;


    volatile static int emergencyStopButtonThreadForMainScriptsLaunched;

    int simulationState;
    bool _requestToStop; // Indicates a passage from paused to stop (but through running!). Check the simulation state diagram!!
    bool _requestToPause; // added on 2010/01/13 (simPauseSimulation didn't work in the first simulation pass)

    // Following is needed to determine if we are really in real-time mode
    std::vector<quint64> simulationTime_history_ns;
    std::vector<quint64> simulationTime_real_history_ns;

    bool _initialValuesInitialized;
    bool _initialPauseAtSpecificTime;
    bool _dynamicContentVisualizationOnly;

    int _stopRequestCounter;
    int _stopRequestCounterAtSimulationStart;
};
