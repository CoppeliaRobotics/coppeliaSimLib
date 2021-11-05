#pragma once

#include "vThread.h"
#include "ser.h"
#ifdef SIM_WITH_GUI
#include "vMenubar.h"
#endif

class CSimulation
{
public:
    CSimulation();
    virtual ~CSimulation();
    void simulationAboutToStart();
    void simulationEnded();
    void setUpDefaultValues();
    bool startOrResumeSimulation();
    bool stopSimulation();
    bool pauseSimulation();
    bool isSimulationRunning() const;
    bool isSimulationPaused() const;
    bool isSimulationStopped() const;
    void advanceSimulationByOneStep();
    void setDefaultSimulationParameterIndex(int d);
    int getDefaultSimulationParameterIndex();
    void setRealTimeSimulation(bool realTime);
    bool getRealTimeSimulation();
    void serialize(CSer& ar);

    quint64 getSimulationTime_us();
    quint64 getSimulationTime_real_us();
    void clearSimulationTimeHistory_us();
    void addToSimulationTimeHistory_us(quint64 simTime,quint64 simTimeReal);
    bool getSimulationTimeHistoryDurations_us(quint64& simTime,quint64& simTimeReal);
    void setResetSceneAtSimulationEnd(bool r);
    bool getResetSceneAtSimulationEnd();
    void setRemoveNewObjectsAtSimulationEnd(bool r);
    bool getRemoveNewObjectsAtSimulationEnd();

    void setAvoidBlocking(bool avoidBlocking);
    bool getAvoidBlocking();

    void incrementStopRequestCounter();
    int getStopRequestCounter();
    bool didStopRequestCounterChangeSinceSimulationStart();

    void pushAllInitialEvents() const;


    bool canGoSlower();
    bool canGoFaster();
    bool goFasterOrSlower(int action);
    int getSpeedModifierIndexOffset();
    bool setSpeedModifierIndexOffset(int offset);
    int getSpeedModifier_forCalcPassPerRendering();

    void setSimulationTimeStep_raw_us(quint64 dt);
    quint64 getSimulationTimeStep_raw_us(int parameterIndex=-1);
    quint64 getSimulationTimeStep_speedModified_us(int parameterIndex=-1);

    void setSimulationPassesPerRendering_raw(int n);
    int getSimulationPassesPerRendering_raw();
    int getSimulationPassesPerRendering_speedModified();

    void setRealTimeCoefficient_raw(double coeff);
    double getRealTimeCoefficient_speedModified();

    bool getDynamicContentVisualizationOnly();
    void setDynamicContentVisualizationOnly(bool dynOnly);

    void adjustRealTimeTimer_us(quint64 deltaTime);

    void setFullscreenAtSimulationStart(bool f);
    bool getFullscreenAtSimulationStart();


    bool isRealTimeCalculationStepNeeded();

    void setSimulationState(int state);

    bool processCommand(int commandID);
    bool getInfo(std::string& txtLeft,std::string& txtRight,int& index);

    void setPauseAtError(bool br);
    bool getPauseAtError();
    void pauseOnErrorRequested();
    void setPauseTime_us(quint64 time);
    quint64 getPauseTime_us();
    bool getPauseAtSpecificTime();
    void setPauseAtSpecificTime(bool e);
    void setCatchUpIfLate(bool c);
    bool getCatchUpIfLate();

    int getSimulationState() const;

    bool getDisplayWarningAboutNonDefaultParameters();

    void setDisableWarningsFlags(int mask);
    int getDisableWarningsFlags();


#ifdef SIM_WITH_GUI
    bool showAndHandleEmergencyStopButton(bool showState,const char* scriptName);
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

    quint64 _simulationTime_us;
    quint64 simulationTime_real_us;
    quint64 simulationTime_real_noCatchUp_us;

    int simulationTime_real_lastInMs;
    quint64 _realTimeCorrection_us;

    int _simulationStepCount;

    bool _displayedWarningAboutNonDefaultParameters;
    int _disableWarningsFlags;

    int _defaultSimulationParameterIndex;
    int _simulationPassesPerRendering;
    quint64 _simulationTimeStep_us;
    int _speedModifierIndexOffset;
    int _desiredFasterOrSlowerSpeed;


    int timeInMsWhenStopWasPressed;

    bool _realTimeSimulation;
    double _realTimeCoefficient;

    quint64 _simulationTimeToPause_us;
    bool _pauseAtSpecificTime;
    bool _pauseAtError;
    bool _pauseOnErrorRequested;
    bool _hierarchyWasEnabledBeforeSimulation;


    volatile static int emergencyStopButtonThreadForMainScriptsLaunched;

    int _simulationState;
    bool _requestToStop; // Indicates a passage from paused to stop (but through running!). Check the simulation state diagram!!
    bool _requestToPause; // added on 2010/01/13 (simPauseSimulation didn't work in the first simulation pass)

    // Following is needed to determine if we are really in real-time mode
    std::vector<quint64> simulationTime_history_us;
    std::vector<quint64> simulationTime_real_history_us;

    bool _initialValuesInitialized;
    bool _initialPauseAtSpecificTime;
    bool _dynamicContentVisualizationOnly;

    int _stopRequestCounter;
    int _stopRequestCounterAtSimulationStart;
};
