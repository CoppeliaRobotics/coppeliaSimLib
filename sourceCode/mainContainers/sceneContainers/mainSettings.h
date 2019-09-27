
#pragma once

#include "visualParam.h"
#include "mainCont.h"
#include "viewableBase.h"

class CMainSettings : public CMainCont 
{
public:
    CMainSettings();
    virtual ~CMainSettings();
    void serialize(CSer& ar);
    void setUpDefaultValues();
    void simulationAboutToStart();
    void simulationEnded();
    void renderYour3DStuff(CViewableBase* renderingObject,int displayAttrib);

    void setScenePathAndName(const char* pathAndName);
    std::string getScenePathAndName() const;
    std::string getScenePath() const;
    std::string getSceneName() const;
    std::string getSceneNameForUi() const;
    std::string getSceneNameWithExt() const;

    void setActiveLayers(unsigned short l);
    unsigned short getActiveLayers();

    int dynamicsBULLETStepSizeDivider_forBackwardCompatibility_03_01_2012;
    int dynamicsODEStepSizeDivider_forBackwardCompatibility_03_01_2012;
    bool forBackwardCompatibility_03_01_2012_stillUsingStepSizeDividers;

    CVisualParam collisionColor;

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
