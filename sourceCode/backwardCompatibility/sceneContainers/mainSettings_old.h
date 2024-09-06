#pragma once
#include <ser.h>

class CMainSettings
{
  public:
    CMainSettings();
    virtual ~CMainSettings();
    void serialize(CSer &ar);
    void setUpDefaultValues();

    int dynamicsBULLETStepSizeDivider_forBackwardCompatibility_03_01_2012;
    int dynamicsODEStepSizeDivider_forBackwardCompatibility_03_01_2012;
    bool forBackwardCompatibility_03_01_2012_stillUsingStepSizeDividers;

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
};
