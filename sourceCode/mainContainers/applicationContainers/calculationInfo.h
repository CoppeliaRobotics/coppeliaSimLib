#pragma once

#include <string>
#include <map>

class CCalculationInfo
{
public:
    CCalculationInfo();
    virtual ~CCalculationInfo();
    void simulationAboutToStart();
    void simulationEnded();

    void resetInfo();
    void formatInfo();

    void addChildScriptCalcTime(int duration,bool threaded);
    void addChildScriptExecCnt(int cnt,bool threaded);

    void setMainScriptExecutionTime(int duration);

    void setMainScriptMessage(const char* msg);

    void collisionDetectionStart();
    void collisionDetectionEnd(bool detected);

    void distanceCalculationStart();
    void distanceCalculationEnd();

    void proximitySensorSimulationStart();
    void proximitySensorSimulationEnd(bool detected);

    void visionSensorSimulationStart();
    void visionSensorSimulationEnd(bool detected);

    void inverseKinematicsStart();
    void inverseKinematicsEnd();

    void renderingStart();
    void renderingEnd();
    void clearRenderingTime();

    void dynamicsStart();
    void dynamicsEnd(int calcPasses,bool dynamicContent);

    void millSimulationStart();
    void millSimulationEnd(float surfaceRemoved,float volumeRemoved);

    void simulationPassStart();
    void simulationPassEnd();

    float getCollisionCalculationTime();
    float getDistanceCalculationTime();
    float getProximitySensorCalculationTime();
    float getVisionSensorCalculationTime();
    float getIkCalculationTime();
    float getChildScriptExecutionTime();
    float getMillingCalculationTime();
    float getDynamicsCalculationTime();
    float getSimulationPassExecutionTime();
    float getRenderingDuration();

#ifdef SIM_WITH_GUI
    void printInformation();
#endif

private:
    bool _mainScriptExecuted;
    int _mainScriptDuration;
    int _regularScriptCount;
    int _threadedScriptCount;
    int _regularScriptDuration;
    int _threadedScriptDuration;

    std::string _mainScriptMessage;

    int _simulationPassStartTime;
    int _simulationPassDuration;

    int _renderingStartTime;
    int _renderingDuration;

    int _collCalcCount;
    int _collDetectCount;
    int _collStartTime;
    int _collCalcDuration;

    int _distCalcCount;
    int _distStartTime;
    int _distCalcDuration;

    int _sensCalcCount;
    int _sensDetectCount;
    int _sensStartTime;
    int _sensCalcDuration;

    int _rendSensCalcCount;
    int _rendSensDetectCount;
    int _rendSensStartTime;
    int _rendSensCalcDuration;

    int _ikCalcCount;
    int _ikStartTime;
    int _ikCalcDuration;

    int _millCalcCount;
    float _millRemovedSurface;
    float _millRemovedVolume;
    int _millStartTime;
    int _millCalcDuration;

    int _dynamicsStartTime;
    int _dynamicsCalcDuration;
    int _dynamicsCalcPasses;
    bool _dynamicsContentAvailable;

    std::string _scriptTxt[2];
    std::string _collTxt[2];
    std::string _distTxt[2];
    std::string _sensTxt[2];
    std::string _visionSensTxt[2];
    std::string _ikTxt[2];
    std::string _dynamicsTxt[2];
    std::string _millTxt[2];
};
