#pragma once

#include <colorObject.h>
#include <thumbnail.h>
#include <cbor.h>
#ifdef SIM_WITH_GUI
#include <vMenubar.h>
#endif

// ----------------------------------------------------------------------------------------------
// flags: bit0: not writable, bit1: not readable, bit2: removable
#define DEFINE_PROPERTIES                                                                                                                                                                   \
    FUNCX(propScene_objectType, "objectType", sim_propertytype_string, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, "Object type", "")                                 \
    FUNCX(propScene_objectMetaInfo, "objectMetaInfo", sim_propertytype_string, sim_propertyinfo_silent | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, "Object meta information", "")             \
    FUNCX(propScene_finalSaveRequest, "finalSaveRequest", sim_propertytype_bool, sim_propertyinfo_modelhashexclude, "Final save", "Lock scene and models after next scene save operation")  \
    FUNCX(propScene_sceneIsLocked, "sceneIsLocked", sim_propertytype_bool, sim_propertyinfo_notwritable, "Scene is locked", "")                                                             \
    FUNCX(propScene_saveCalculationStructs, "saveCalculationStructs", sim_propertytype_bool, 0, "Save calculation structures", "Save operation also saves existing calculation structures") \
    FUNCX(propScene_sceneUid, "sceneUid", sim_propertytype_int, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, "Scene UID", "Scene unique identifier")                   \
    FUNCX(propScene_visibilityLayers, "visibilityLayers", sim_propertytype_int, 0, "Visibility layers", "Currently active visibility layers")                                               \
    FUNCX(propScene_scenePath, "scenePath", sim_propertytype_string, sim_propertyinfo_modelhashexclude, "Scene path", "")                                                                   \
    FUNCX(propScene_acknowledgment, "acknowledgment", sim_propertytype_string, 0, "Acknowledgment", "Scene acknowledgment")                                                                 \
    FUNCX(propScene_sceneUidString, "sceneUidString", sim_propertytype_string, sim_propertyinfo_notwritable, "Scene UID string", "Scene unique identifier string")                          \
    FUNCX(propScene_ambientLight, "ambientLight", sim_propertytype_color, 0, "Ambient light", "")

#define FUNCX(name, str, v1, v2, t1, t2) const SProperty name = {str, v1, v2, t1, t2};
DEFINE_PROPERTIES
#undef FUNCX
#define FUNCX(name, str, v1, v2, t1, t2) name,
const std::vector<SProperty> allProps_scene = {DEFINE_PROPERTIES};
#undef FUNCX
#undef DEFINE_PROPERTIES
// ----------------------------------------------------------------------------------------------

class CCamera;
class CViewableBase;

class CEnvironment
{
  public:
    CEnvironment();
    virtual ~CEnvironment();

    void setUpDefaultValues();

    void serialize(CSer& ar);
    void setAcknowledgement(const char* a);
    std::string getAcknowledgement() const;
    void setVisualizeWirelessEmitters(bool v);
    bool getVisualizeWirelessEmitters() const;
    void setVisualizeWirelessReceivers(bool v);
    bool getVisualizeWirelessReceivers() const;
    void setShapeTexturesEnabled(bool e);
    bool getShapeTexturesEnabled() const;

    void appendGenesisData(CCbor* ev) const;

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
    void setCalculationMaxTriangleSize(double s);
    double getCalculationMaxTriangleSize() const;
    void setCalculationMinRelTriangleSize(double s);
    double getCalculationMinRelTriangleSize() const;
    void setSaveExistingCalculationStructures(bool s);
    bool getSaveExistingCalculationStructures() const;
    void setSaveExistingCalculationStructuresTemp(bool s);
    bool getSaveExistingCalculationStructuresTemp() const;

    int getSceneUniqueID() const;

    void setSceneCanBeDiscardedWhenNewSceneOpened(bool canBeDiscarded);
    bool getSceneCanBeDiscardedWhenNewSceneOpened() const;

    std::string getExtensionString() const;
    void setExtensionString(const char* str);

    int setBoolProperty(const char* pName, bool pState);
    int getBoolProperty(const char* pName, bool& pState) const;
    int setIntProperty(const char* pName, int pState);
    int getIntProperty(const char* pName, int& pState) const;
    int setStringProperty(const char* pName, const char* pState);
    int getStringProperty(const char* pName, std::string& pState) const;
    int setColorProperty(const char* pName, const float* pState);
    int getColorProperty(const char* pName, float* pState) const;
    int getPropertyName(int& index, std::string& pName, int excludeFlags) const;
    int getPropertyInfo(const char* pName, int& info, std::string& infoTxt) const;

    void setAmbientLight(const float c[3]);
    void setFogEnabled(bool e);
    bool getFogEnabled() const;
    void setFogDensity(double d);
    double getFogDensity() const;
    void setFogStart(double s);
    double getFogStart() const;
    void setFogEnd(double e);
    double getFogEnd() const;
    void setFogType(double t);
    int getFogType() const;
    bool areNonAmbientLightsActive() const;
    void setNonAmbientLightsActive(bool a);
    void setActiveLayers(int l);
    int getActiveLayers() const;

    void setScenePathAndName(const char* pathAndName);
    std::string getScenePathAndName() const;
    std::string getScenePath() const;
    std::string getSceneName() const;
    std::string getSceneNameForUi() const;
    std::string getSceneNameWithExt() const;

    void generateNewUniquePersistentIdString();
    std::string getUniquePersistentIdString() const;
    bool getSceneIsClosingFlag() const;
    void setSceneIsClosingFlag(bool closing);

    quint64 autoSaveLastSaveTimeInSecondsSince1970;
    float fogBackgroundColor[3];
    float backGroundColor[3];
    float backGroundColorDown[3];
    float ambientLightColor[3];
    CColorObject wirelessEmissionVolumeColor;
    CColorObject wirelessReceptionVolumeColor;

    CThumbnail modelThumbnail_notSerializedHere;

  protected:
    int _activeLayers;

    bool _nonAmbientLightsAreActive;
    bool fogEnabled;
    double fogDensity;
    double fogStart;
    double fogEnd;
    int fogType; // 0=linear, 1=exp, 2=exp2

    std::string _acknowledgement;
    std::string _sceneUniquePersistentIdString;
    bool _visualizeWirelessEmitters;
    bool _visualizeWirelessReceivers;
    bool _requestFinalSave;
    bool _sceneIsLocked;
    bool _shapeTexturesEnabled;
    bool _2DElementTexturesEnabled;
    double _calculationMaxTriangleSize;
    double _calculationMinRelTriangleSize;
    bool _saveExistingCalculationStructures;
    bool _saveExistingCalculationStructuresTemp;
    bool _sceneCanBeDiscardedWhenNewSceneOpened;

    bool _sceneIsClosingFlag;
    std::string _extensionString;

    int _sceneUniqueID;

    std::string _scenePathAndName;

    static bool _shapeTexturesTemporarilyDisabled;
    static bool _shapeEdgesTemporarilyDisabled;
    static bool _customUisTemporarilyDisabled;

    static int _nextSceneUniqueID;

#ifdef SIM_WITH_GUI
  public:
    void setBackgroundColor(int viewSize[2]);
    void activateAmbientLight(bool a);
    void activateFogIfEnabled(CViewableBase* viewable, bool forDynamicContentOnly);
    void deactivateFog();
    void temporarilyDeactivateFog();
    void reactivateFogThatWasTemporarilyDisabled();
#endif
};
