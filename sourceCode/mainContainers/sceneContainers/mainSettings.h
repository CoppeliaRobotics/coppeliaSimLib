#pragma once

#include "colorObject.h"
#include "viewableBase.h"

class CMainSettings 
{
public:
    CMainSettings();
    virtual ~CMainSettings();
    void serialize(CSer& ar);
    void setUpDefaultValues();

    void setScenePathAndName(const char* pathAndName);
    std::string getScenePathAndName() const;
    std::string getScenePath() const;
    std::string getSceneName() const;
    std::string getSceneNameForUi() const;
    std::string getSceneNameWithExt() const;

    void pushReconstructAllEvents() const;

    void setActiveLayers(unsigned short l);
    unsigned short getActiveLayers() const;

    int dynamicsBULLETStepSizeDivider_forBackwardCompatibility_03_01_2012;
    int dynamicsODEStepSizeDivider_forBackwardCompatibility_03_01_2012;
    bool forBackwardCompatibility_03_01_2012_stillUsingStepSizeDividers;

    CColorObject collisionColor;

    bool ikCalculationEnabled;
    bool gcsCalculationEnabled;
    bool collisionDetectionEnabled;
    bool distanceCalculationEnabled;
    bool jointMotionHandlingEnabled_DEPRECATED;
    bool pathMotionHandlingEnabled_DEPRECATED;
    bool proximitySensorsEnabled;
    bool visionSensorsEnabled;
    bool millsEnabled;
    bool mirrorsDisabled;
    bool clippingPlanesDisabled;
    int infoWindowColorStyle;
    bool infoWindowOpenState;
    bool statusBoxOpenState;

private:
    std::string _scenePathAndName;
    unsigned short _activeLayers;
};
