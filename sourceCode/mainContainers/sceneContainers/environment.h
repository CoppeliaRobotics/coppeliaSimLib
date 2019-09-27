
#pragma once

#include "visualParam.h"
#include "mainCont.h"
#include "thumbnail.h"
#ifdef SIM_WITH_GUI
    #include "vMenubar.h"
#endif

class CCamera;

class CEnvironment : public CMainCont
{
public:
    CEnvironment();
    virtual ~CEnvironment();
    void simulationAboutToStart();
    void simulationEnded();

    void setUpDefaultValues();

    void serialize(CSer& ar);
    void setAcknowledgement(const std::string& a);
    std::string getAcknowledgement() const;
    void setVisualizeWirelessEmitters(bool v);
    bool getVisualizeWirelessEmitters() const;
    void setVisualizeWirelessReceivers(bool v);
    bool getVisualizeWirelessReceivers() const;
    void setShapeTexturesEnabled(bool e);
    bool getShapeTexturesEnabled() const;

    static int getNextSceneUniqueId();
    static void setShapeTexturesTemporarilyDisabled(bool d);
    static bool getShapeTexturesTemporarilyDisabled();
    static void setShapeEdgesTemporarilyDisabled(bool d);
    static bool getShapeEdgesTemporarilyDisabled();
    static void setCustomUisTemporarilyDisabled(bool d);
    static bool getCustomUisTemporarilyDisabled();

    void set2DElementTexturesEnabled(bool e);
    bool get2DElementTexturesEnabled() const;

    void setRequestFinalSave(bool finalSaveActivated);
    bool getRequestFinalSave() const;
    void setSceneLocked();
    bool getSceneLocked() const;
    void setCalculationMaxTriangleSize(float s);
    float getCalculationMaxTriangleSize() const;
    void setCalculationMinRelTriangleSize(float s);
    float getCalculationMinRelTriangleSize() const;
    void setSaveExistingCalculationStructures(bool s);
    bool getSaveExistingCalculationStructures() const;
    void setSaveExistingCalculationStructuresTemp(bool s);
    bool getSaveExistingCalculationStructuresTemp() const;

    int getSceneUniqueID() const;


    void setSceneCanBeDiscardedWhenNewSceneOpened(bool canBeDiscarded);
    bool getSceneCanBeDiscardedWhenNewSceneOpened() const;

    std::string getExtensionString() const;
    void setExtensionString(const char* str);

    void setFogEnabled(bool e);
    bool getFogEnabled() const;
    void setFogDensity(float d);
    float getFogDensity() const;
    void setFogStart(float s);
    float getFogStart() const;
    void setFogEnd(float e);
    float getFogEnd() const;
    void setFogType(float t);
    int getFogType() const;
    bool areNonAmbientLightsActive() const;
    void setNonAmbientLightsActive(bool a);

    void renderYour3DStuff(CViewableBase* renderingObject,int displayAttrib);
    void setBackgroundColor(int viewSize[2]);
    void activateAmbientLight(bool a);
    void activateFogIfEnabled(CViewableBase* viewable,bool forDynamicContentOnly);
    void deactivateFog();
    void temporarilyDeactivateFog();
    void reactivateFogThatWasTemporarilyDisabled();
    void generateNewUniquePersistentIdString();
    std::string getUniquePersistentIdString() const;
    bool getSceneIsClosingFlag() const;
    void setSceneIsClosingFlag(bool closing);

    std::string getCurrentJob() const;
    bool setCurrentJob(const char* jobName);
    int getJobCount();
    int getJobIndex(const std::string& name);
    std::string getJobAtIndex(int index);
    bool createNewJob(std::string newName);
    bool deleteCurrentJob();
    bool renameCurrentJob(std::string newName);
    bool switchJob(int index);
    bool getJobFunctionalityEnabled();
    void setJobFunctionalityEnabled(bool en);

    void setShowPartRepository(bool en);
    bool getShowPartRepository();
    void setShowPalletRepository(bool en);
    bool getShowPalletRepository();

    quint64 autoSaveLastSaveTimeInSecondsSince1970;
    float fogBackgroundColor[3];
    float backGroundColor[3];
    float backGroundColorDown[3];
    float ambientLightColor[3];
    CVisualParam wirelessEmissionVolumeColor;
    CVisualParam wirelessReceptionVolumeColor;

    CThumbnail modelThumbnail_notSerializedHere;

#ifdef SIM_WITH_GUI
    void addLayoutMenu(VMenu* menu);
    void addJobsMenu(VMenu* menu);
    bool processGuiCommand(int commandID);
#endif

protected:
    bool _nonAmbientLightsAreActive;
    bool fogEnabled;
    float fogDensity;
    float fogStart;
    float fogEnd;
    int fogType; // 0=linear, 1=exp, 2=exp2

    std::string _acknowledgement;
    std::string _sceneUniquePersistentIdString;
    bool _visualizeWirelessEmitters;
    bool _visualizeWirelessReceivers;
    bool _requestFinalSave;
    bool _sceneIsLocked;
    bool _shapeTexturesEnabled;
    bool _2DElementTexturesEnabled;
    float _calculationMaxTriangleSize;
    float _calculationMinRelTriangleSize;
    bool _saveExistingCalculationStructures;
    bool _saveExistingCalculationStructuresTemp;
    bool _sceneCanBeDiscardedWhenNewSceneOpened;

    bool _jobFuncEnabled; // not serialized
    bool _showPartRepository; // not serialized
    bool _showPalletRepository; // not serialized

    bool _sceneIsClosingFlag;
    std::string _extensionString;

    std::vector<std::string> _jobs;
    std::string _currentJob;

    int _sceneUniqueID;

    static bool _shapeTexturesTemporarilyDisabled;
    static bool _shapeEdgesTemporarilyDisabled;
    static bool _customUisTemporarilyDisabled;

    static int _nextSceneUniqueID;
};
