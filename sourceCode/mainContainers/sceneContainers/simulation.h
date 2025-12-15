#pragma once

#include <vThread.h>
#include <ser.h>
#include <cbor.h>
#include <propertiesAndMethods.h>
#ifdef SIM_WITH_GUI
#include <vMenubar.h>
#endif

// ----------------------------------------------------------------------------------------------
#define FUNCX(name, str, v1, v2, t1, t2) extern const SProperty name;
SIMULATION_PROPERTIES
#undef FUNCX
extern const std::vector<SProperty> allProps_sim;
// ----------------------------------------------------------------------------------------------

class CSimulation
{
  public:
    CSimulation();
    virtual ~CSimulation();
    void simulationAboutToStart();
    void simulationEnded();
    void setUpDefaultValues();
    void startOrResumeSimulation();
    void stopSimulation();
    void pauseSimulation();
    bool isSimulationRunning() const;
    bool isSimulationPaused() const;
    bool isSimulationStopped() const;
    void advanceSimulationByOneStep();
    void setIsRealTimeSimulation(bool realTime);
    bool getIsRealTimeSimulation() const;
    void serialize(CSer& ar);

    double getSimulationTime() const;
    double getSimulationTime_real() const;
    void setRemoveNewObjectsAtSimulationEnd(bool r);
    bool getRemoveNewObjectsAtSimulationEnd() const;

    void incrementStopRequestCounter();
    int getStopRequestCounter() const;
    bool didStopRequestCounterChangeSinceSimulationStart() const;

    void appendGenesisData(CCbor* ev) const;

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

    bool getInfo(std::string& txtLeft, std::string& txtRight, int& index) const;

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
    int getPropertyName(int& index, std::string& pName, int excludeFlags) const;
    int getPropertyInfo(const char* pName, int& info, std::string& infoTxt) const;

#ifdef SIM_WITH_GUI
    bool processCommand(int commandID);
    bool showAndHandleEmergencyStopButton(bool showState, const char* scriptName);
    void keyPress(int key);
    void addMenu(VMenu* menu);
#endif

  private:
    void _setSpeedModifierRaw(int sm);
    double _getNewTimeStep(int newSpeedModifierCount) const;
    bool _goFasterOrSlower(int action);
    void _setSimulationTime(double t);
    void _clearSimulationTimeHistory();
    void _addToSimulationTimeHistory(double simTime, double simTimeReal);
    bool _getSimulationTimeHistoryDurations(double& simTime, double& simTimeReal) const;

    double _getSpeedModifier_forRealTimeCoefficient() const;

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
    bool _hierarchyWasEnabledBeforeSimulation;

    int _simulationState;
    bool _requestToStop;  // Indicates a passage from paused to stop (but through running!). Check the simulation state diagram!!
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
