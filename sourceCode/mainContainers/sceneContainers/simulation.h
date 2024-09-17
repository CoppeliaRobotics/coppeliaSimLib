#pragma once

#include <vThread.h>
#include <ser.h>
#include <cbor.h>
#ifdef SIM_WITH_GUI
#include <vMenubar.h>
#endif

// ----------------------------------------------------------------------------------------------
// flags: bit0: not writable, bit1: not readable, bit2: removable
#define DEFINE_PROPERTIES \
    FUNCX(propSim_resetSimulationAtEnd,                       "resetSimulationAtEnd",                       sim_propertytype_bool,      0) \
    FUNCX(propSim_removeNewObjectsAtEnd,                      "removeNewObjectsAtEnd",                      sim_propertytype_bool,      0) \
    FUNCX(propSim_realtimeSimulation,                         "realtimeSimulation",                         sim_propertytype_bool,      0) \
    FUNCX(propSim_pauseSimulationAtTime,                      "pauseSimulationAtTime",                      sim_propertytype_bool,      0) \
    FUNCX(propSim_pauseSimulationAtError,                     "pauseSimulationAtError",                     sim_propertytype_bool,      0) \
    FUNCX(propSim_simulationTime,                           "simulationTime",                               sim_propertytype_float,      sim_propertyinfo_notwritable) \
    FUNCX(propSim_timeStep,                                 "timeStep",                                     sim_propertytype_float,      0) \
    FUNCX(propSim_timeToPause,                              "timeToPause",                                  sim_propertytype_float,      0) \
    FUNCX(propSim_realtimeModifier,                         "realtimeModifier",                             sim_propertytype_float,      0) \
    FUNCX(propSim_stepCount,                                "stepCount",                                    sim_propertytype_int,    sim_propertyinfo_notwritable) \
    FUNCX(propSim_simulationState,                          "simulationState",                              sim_propertytype_int,    sim_propertyinfo_notwritable) \
    FUNCX(propSim_stepsPerRendering,                        "stepsPerRendering",                            sim_propertytype_int,    0) \
    FUNCX(propSim_speedModifier,                            "speedModifier",                                sim_propertytype_int,    0) \

#define FUNCX(name, str, v1, v2) const SProperty name = {str, v1, v2};
DEFINE_PROPERTIES
#undef FUNCX
#define FUNCX(name, str, v1, v2) name,
const std::vector<SProperty> allProps_sim = { DEFINE_PROPERTIES };
#undef FUNCX
#undef DEFINE_PROPERTIES
// ----------------------------------------------------------------------------------------------

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
    void setIsRealTimeSimulation(bool realTime);
    bool getIsRealTimeSimulation() const;
    void serialize(CSer &ar);

    double getSimulationTime() const;
    double getSimulationTime_real() const;
    void setResetSceneAtSimulationEnd(bool r);
    bool getResetSceneAtSimulationEnd() const;
    void setRemoveNewObjectsAtSimulationEnd(bool r);
    bool getRemoveNewObjectsAtSimulationEnd() const;

    void incrementStopRequestCounter();
    int getStopRequestCounter() const;
    bool didStopRequestCounterChangeSinceSimulationStart() const;

    void appendGenesisData(CCbor *ev) const;

    bool canGoSlower() const;
    bool canGoFaster() const;
    bool getSettingsAreDefault() const;

    void setSpeedModifierCount(int sm);
    int getSpeedModifierCount() const;

    void setTimeStep(double dt, bool alsoWhenRunning = false);
    double getTimeStep() const;

    void setPassesPerRendering(int n);
    int getPassesPerRendering() const;
    void setSimulationStepCount(int cnt);

    void setRealTimeCoeff(double coeff);
    double getRealTimeCoeff() const;

    bool getDynamicContentVisualizationOnly() const;
    void setDynamicContentVisualizationOnly(bool dynOnly);

    void adjustRealTimeTimer(double deltaTime);

    void setFullscreenAtSimulationStart(bool f);
    bool getFullscreenAtSimulationStart() const;

    bool isRealTimeCalculationStepNeeded() const;

    void setSimulationState(int state);

    bool getInfo(std::string &txtLeft, std::string &txtRight, int &index) const;

    void setPauseAtError(bool br);
    bool getPauseAtError() const;
    void pauseOnErrorRequested();
    void setPauseTime(double time);
    double getPauseTime() const;
    bool getPauseAtSpecificTime() const;
    void setPauseAtSpecificTime(bool e);
    int getSimulationState() const;

    int setBoolProperty(const char* pName, bool pState);
    int getBoolProperty(const char* pName, bool& pState) const;
    int setIntProperty(const char* pName, int pState);
    int getIntProperty(const char* pName, int& pState) const;
    int setFloatProperty(const char* pName, double pState);
    int getFloatProperty(const char* pName, double& pState) const;
    int getPropertyName(int& index, std::string& pName) const;
    int getPropertyInfo(const char* pName, int& info) const;


#ifdef SIM_WITH_GUI
    bool processCommand(int commandID);
    bool showAndHandleEmergencyStopButton(bool showState, const char *scriptName);
    void keyPress(int key);
    void addMenu(VMenu *menu);
#endif

  private:
    void _setSpeedModifierRaw(int sm);
    double _getNewTimeStep(int newSpeedModifierCount) const;
    bool _goFasterOrSlower(int action);
    void _setSimulationTime(double t);
    void _clearSimulationTimeHistory();
    void _addToSimulationTimeHistory(double simTime, double simTimeReal);
    bool _getSimulationTimeHistoryDurations(double &simTime, double &simTimeReal) const;

    double _getSpeedModifier_forRealTimeCoefficient() const;

    bool _resetSimulationAtEnd;
    bool _removeNewObjectsAtSimulationEnd;

    bool _fullscreenAtSimulationStart;

    double _simulationTime;
    double simulationTime_real;
    double simulationTime_real_noCatchUp;

    int simulationTime_real_lastInMs;
    double _realTimeCorrection;

    int _simulationStepCount;

    int _disableWarningsFlags;

    int _simulationPassesPerRendering;
    double _simulationTimeStep;
    int _speedModifierCount;
    int _desiredFasterOrSlowerSpeed;

    int timeInMsWhenStopWasPressed;

    bool _realTimeSimulation;
    double _realTimeCoefficient;

    double _simulationTimeToPause;
    bool _pauseAtSpecificTime;
    bool _pauseAtError;
    bool _pauseOnErrorRequested;
    bool _hierarchyWasEnabledBeforeSimulation;

    int _simulationState;
    bool _requestToStop;  // Indicates a passage from paused to stop (but through running!). Check the simulation state
                          // diagram!!
    bool _requestToPause; // added on 2010/01/13 (simPauseSimulation didn't work in the first simulation pass)

    // Following is needed to determine if we are really in real-time mode
    std::vector<double> simulationTime_history;
    std::vector<double> simulationTime_real_history;

    bool _initialValuesInitialized;
    double _initialSimulationTimeStep;
    bool _initialPauseAtSpecificTime;
    bool _dynamicContentVisualizationOnly;

    int _stopRequestCounter;
    int _stopRequestCounterAtSimulationStart;
};
