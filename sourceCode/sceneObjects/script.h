#pragma once

#include <ser.h>
#include <sceneObject.h>
#include <scriptObject.h>

// ----------------------------------------------------------------------------------------------
// flags: bit0: not writable, bit1: not readable, bit2: removable
#define DEFINE_PROPERTIES                                                                                             \
    FUNCX(propScript_objectMetaInfo, "objectMetaInfo", sim_propertytype_string, sim_propertyinfo_silent | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Object meta information", "description": ""})===", "")             \
    FUNCX(propScript_size, "scriptSize", sim_propertytype_float, 0, R"===({"label": "Size", "description": "Size of the script's 3D representation"})===", "") \
    FUNCX(propScript_resetAfterSimError, "resetAfterSimError", sim_propertytype_bool, 0, R"===({"label": "Reset after simulation error", "description": ""})===", "") \
    FUNCX(propScript_detachedScript, "detachedScript", sim_propertytype_handle, sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "Detached script handle", "description": "", "handleType": "detachedScript"})===", "")                                                                    \
    /* FOLLOWING FOR BACKWARD COMPATIBILITY */ \
    FUNCX(propScript_scriptDisabled, "scriptDisabled", sim_propertytype_bool, sim_propertyinfo_deprecated | sim_propertyinfo_silent | sim_propertyinfo_modelhashexclude, R"===({"label": "", "description": ""})===", "")                                                         \
    FUNCX(propScript_restartOnError, "restartOnError", sim_propertytype_bool, sim_propertyinfo_deprecated | sim_propertyinfo_silent | sim_propertyinfo_modelhashexclude, R"===({"label": "", "description": ""})===", "")                                                          \
    FUNCX(propScript_execPriority, "execPriority", sim_propertytype_int, sim_propertyinfo_deprecated | sim_propertyinfo_silent | sim_propertyinfo_modelhashexclude, R"===({"label": "", "description": ""})===", "")                                                                    \
    FUNCX(propScript_scriptType, "scriptType", sim_propertytype_int, sim_propertyinfo_deprecated | sim_propertyinfo_silent | sim_propertyinfo_modelhashexclude | sim_propertyinfo_notwritable, R"===({"label": "", "description": ""})===", "")                                                \
    FUNCX(propScript_executionDepth, "executionDepth", sim_propertytype_int, sim_propertyinfo_deprecated | sim_propertyinfo_silent | sim_propertyinfo_silent | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "", "description": ""})===", "")    \
    FUNCX(propScript_scriptState, "scriptState", sim_propertytype_int, sim_propertyinfo_deprecated | sim_propertyinfo_silent | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "", "description": ""})===", "")        \
    FUNCX(propScript_language, "language", sim_propertytype_string, sim_propertyinfo_deprecated | sim_propertyinfo_silent | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "", "description": ""})===", "")                                                        \
    FUNCX(propScript_code, "code", sim_propertytype_string, sim_propertyinfo_deprecated | sim_propertyinfo_silent | sim_propertyinfo_modelhashexclude, R"===({"label": "", "description": ""})===", "")                                                                                 \
    FUNCX(propScript_scriptName, "scriptName", sim_propertytype_string, sim_propertyinfo_deprecated | sim_propertyinfo_silent | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "", "description": ""})===", "")         \
    FUNCX(propScript_addOnPath, "addOnPath", sim_propertytype_string, sim_propertyinfo_deprecated | sim_propertyinfo_silent | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "", "description": ""})===", "") \
    FUNCX(propScript_addOnMenuPath, "addOnMenuPath", sim_propertytype_string, sim_propertyinfo_deprecated | sim_propertyinfo_silent | sim_propertyinfo_notwritable | sim_propertyinfo_modelhashexclude, R"===({"label": "", "description": ""})===", "")

#define FUNCX(name, str, v1, v2, t1, t2) const SProperty name = {str, v1, v2, t1, t2};
DEFINE_PROPERTIES
#undef FUNCX
#define FUNCX(name, str, v1, v2, t1, t2) name,
const std::vector<SProperty> allProps_script = {DEFINE_PROPERTIES};
#undef FUNCX
#undef DEFINE_PROPERTIES
// ----------------------------------------------------------------------------------------------

class CScript : public CSceneObject
{
  public:
    CScript();
    CScript(CScriptObject* scrObj);
    CScript(int scriptType, const char* text, int options, const char* lang);
    virtual ~CScript();

    // Following functions are inherited from CSceneObject
    void addSpecializedObjectEventData(CCbor* ev) override;
    CSceneObject* copyYourself() override;
    void removeSceneDependencies() override;
    void scaleObject(double scalingFactor) override;
    void serialize(CSer& ar) override;
    void announceCollectionWillBeErased(int groupID, bool copyBuffer) override;
    void announceCollisionWillBeErased(int collisionID, bool copyBuffer) override;
    void announceDistanceWillBeErased(int distanceID, bool copyBuffer) override;
    void performIkLoadingMapping(const std::map<int, int>* map, int opType) override;
    void performCollectionLoadingMapping(const std::map<int, int>* map, int opType) override;
    void performCollisionLoadingMapping(const std::map<int, int>* map, int opType) override;
    void performDistanceLoadingMapping(const std::map<int, int>* map, int opType) override;
    void performTextureObjectLoadingMapping(const std::map<int, int>* map, int opType) override;
    void performDynMaterialObjectLoadingMapping(const std::map<int, int>* map) override;
    void simulationAboutToStart() override;
    void simulationEnded() override;
    void initializeInitialValues(bool simulationAlreadyRunning) override;
    void computeBoundingBox() override;
    void setIsInScene(bool s) override;
    std::string getObjectTypeInfo() const override;
    std::string getObjectTypeInfoExtended() const override;
    bool isPotentiallyCollidable() const override;
    bool isPotentiallyMeasurable() const override;
    bool isPotentiallyDetectable() const override;
    void announceObjectWillBeErased(const CSceneObject* object, bool copyBuffer) override;
    void announceIkObjectWillBeErased(int ikGroupID, bool copyBuffer) override;
    void performObjectLoadingMapping(const std::map<int, int>* map, int opType) override;
    void setObjectHandle(int newObjectHandle) override;
    bool canDestroyNow() override;
    int setBoolProperty(const char* pName, bool pState) override;
    int getBoolProperty(const char* pName, bool& pState) const override;
    int setIntProperty(const char* pName, int pState) override;
    int getIntProperty(const char* pName, int& pState) const override;
    int setLongProperty(const char* pName, long long int pState) override;
    int getLongProperty(const char* pName, long long int& pState) const override;
    int getHandleProperty(const char* pName, long long int& pState) const override;
    int setFloatProperty(const char* pName, double pState) override;
    int getFloatProperty(const char* pName, double& pState) const override;
    int setStringProperty(const char* pName, const char* pState) override;
    int getStringProperty(const char* pName, std::string& pState) const override;
    int setColorProperty(const char* pName, const float* pState) override;
    int getColorProperty(const char* pName, float* pState) const override;
    int getPropertyName(int& index, std::string& pName, std::string& appartenance, int excludeFlags) const override;
    static int getPropertyName_static(int& index, std::string& pName, std::string& appartenance, int excludeFlags);
    int getPropertyInfo(const char* pName, int& info, std::string& infoTxt) const override;
    static int getPropertyInfo_static(const char* pName, int& info, std::string& infoTxt);

    double getScriptSize() const;
    void reinitAfterSimulationIfNeeded();

    CColorObject* getScriptColor();

    void setScriptSize(double s);
    void resetAfterSimError(bool r);
    bool getResetAfterSimError() const;
    int getScriptPseudoHandle() const;

    CScriptObject* scriptObject;

  protected:
    void _commonInit(int scriptType, const char* text, int options, const char* lang);

    CColorObject _scriptColor;
    double _scriptSize;
    bool _resetAfterSimError;

#ifdef SIM_WITH_GUI
  public:
    void display(CViewableBase* renderingObject, int displayAttrib) override;
#endif
};
