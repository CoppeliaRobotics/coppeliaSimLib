#pragma once

#include <string>
#include <map>
#include <simTypes.h>

class CCalculationInfo
{
public:
    CCalculationInfo();
    virtual ~CCalculationInfo();
    void simulationAboutToStart();
    void simulationAboutToStep();
    void simulationEnded();

    void resetInfo(bool clearDisp);
    void formatInfo();

    void setMainScriptExecutionTime(int duration);
    void setSimulationScriptExecCount(int cnt);

    void proximitySensorSimulationStart();
    void proximitySensorSimulationEnd(bool detected);

    void visionSensorSimulationStart();
    void visionSensorSimulationEnd(bool detected);

    void renderingStart();
    void renderingEnd();
    void clearRenderingTime();

    void dynamicsStart();
    void dynamicsEnd(int calcPasses,bool dynamicContent);

    void simulationPassStart();
    void simulationPassEnd();

    double getProximitySensorCalculationTime();
    double getVisionSensorCalculationTime();
    double getMainScriptExecutionTime();
    double getDynamicsCalculationTime();
    double getSimulationPassExecutionTime();
    double getRenderingDuration();

#ifdef SIM_WITH_GUI
    void printInformation();
#endif

private:
    int _mainScriptDuration;
    int _simulationScriptExecCount;

    int _simulationPassStartTime;
    int _simulationPassDuration;

    int _renderingStartTime;
    int _renderingDuration;

    int _sensCalcCount;
    int _sensDetectCount;
    int _sensStartTime;
    int _sensCalcDuration;

    int _rendSensCalcCount;
    int _rendSensDetectCount;
    int _rendSensStartTime;
    int _rendSensCalcDuration;

    int _dynamicsStartTime;
    int _dynamicsCalcDuration;
    int _dynamicsCalcPasses;
    bool _dynamicsContentAvailable;

    std::string _scriptTxt[2];
    std::string _sensTxt[2];
    std::string _visionSensTxt[2];
    std::string _dynamicsTxt[2];
};
