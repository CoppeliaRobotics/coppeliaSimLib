#pragma once

#include <ser.h>
#include <simMath/3Vector.h>
#include <simMath/7Vector.h>
#include <sceneObject.h>

struct SDummyProperty
{
    const char* name;
    int type;
    int flags;
    int oldEnums[5];
    const char* shortInfoTxt;
    const char* infoTxt;
};

// ----------------------------------------------------------------------------------------------
// flags: bit0: not writable, bit1: not readable, bit2: removable
#define DEFINE_PROPERTIES                                                                                                                                                                                                                                         \
    FUNCX(propDummy_size, "dummySize", sim_propertytype_float, 0, -1, -1, -1, -1, -1, "Size", "Dummy size")                                                                                                                                                       \
    FUNCX(propDummy_linkedDummyHandle, "linkedDummyHandle", sim_propertytype_int, sim_propertyinfo_deprecated | sim_propertyinfo_modelhashexclude, -1, -1, -1, -1, -1, "", "")                                                                            \
    FUNCX(propDummy_linkedDummy, "linkedDummy", sim_propertytype_handle, sim_propertyinfo_modelhashexclude, -1, -1, -1, -1, -1, "Linked dummy", "Handle of the linked dummy")                                                                            \
    FUNCX(propDummy_dummyType, "dummyType", sim_propertytype_int, 0, -1, -1, -1, -1, -1, "Type", "Dummy type")                                                                                                                                                    \
    FUNCX(propDummy_assemblyTag, "assemblyTag", sim_propertytype_string, 0, -1, -1, -1, -1, -1, "Assembly tag", "")                                                                                                                                               \
    FUNCX(propDummy_engineProperties, "engineProperties", sim_propertytype_string, sim_propertyinfo_modelhashexclude, -1, -1, -1, -1, -1, "Engine properties", "Engine properties as JSON text")                                                                  \
    FUNCX(propDummy_mujocoLimitsEnabled, "mujoco.limitsEnabled", sim_propertytype_bool, 0, sim_mujoco_dummy_limited, -1, -1, -1, -1, "", "")                                                                                                                      \
    FUNCX(propDummy_mujocoLimitsRange, "mujoco.limitsRange", sim_propertytype_floatarray, 0, sim_mujoco_dummy_range1, sim_mujoco_dummy_range2, -1, -1, -1, "", "")                                                                                                \
    FUNCX(propDummy_mujocoLimitsSolref, "mujoco.limitsSolref", sim_propertytype_floatarray, 0, sim_mujoco_dummy_solreflimit1, sim_mujoco_dummy_solreflimit2, -1, -1, -1, "", "")                                                                                  \
    FUNCX(propDummy_mujocoLimitsSolimp, "mujoco.limitsSolimp", sim_propertytype_floatarray, 0, sim_mujoco_dummy_solimplimit1, sim_mujoco_dummy_solimplimit2, sim_mujoco_dummy_solimplimit3, sim_mujoco_dummy_solimplimit4, sim_mujoco_dummy_solimplimit5, "", "") \
    FUNCX(propDummy_mujocoMargin, "mujoco.margin", sim_propertytype_float, 0, sim_mujoco_dummy_margin, -1, -1, -1, -1, "", "")                                                                                                                                    \
    FUNCX(propDummy_mujocoSpringStiffness, "mujoco.springStiffness", sim_propertytype_float, 0, sim_mujoco_dummy_stiffness, -1, -1, -1, -1, "", "")                                                                                                               \
    FUNCX(propDummy_mujocoSpringDamping, "mujoco.springDamping", sim_propertytype_float, 0, sim_mujoco_dummy_damping, -1, -1, -1, -1, "", "")                                                                                                                     \
    FUNCX(propDummy_mujocoSpringLength, "mujoco.springLength", sim_propertytype_float, 0, sim_mujoco_dummy_springlength, -1, -1, -1, -1, "", "")                                                                                                                  \
    FUNCX(propDummy_mujocoJointProxyHandle, "mujoco.jointProxyHandle", sim_propertytype_int, sim_propertyinfo_modelhashexclude, sim_mujoco_dummy_proxyjointid, -1, -1, -1, -1, "Joint proxy", "Handle of the joint proxy (MuJoCo only)")                          \
    FUNCX(propDummy_mujocoOverlapConstrSolref, "mujoco.overlapConstrSolref", sim_propertytype_floatarray, 0, -1, -1, -1, -1, -1, "", "")                                                                                                                          \
    FUNCX(propDummy_mujocoOverlapConstrSolimp, "mujoco.overlapConstrSolimp", sim_propertytype_floatarray, 0, -1, -1, -1, -1, -1, "", "")                                                                                                                          \
    FUNCX(propDummy_mujocoOverlapConstrTorqueScale, "mujoco.overlapConstrTorquescale", sim_propertytype_float, 0, -1, -1, -1, -1, -1, "", "")

#define FUNCX(name, str, v1, v2, w0, w1, w2, w3, w4, t1, t2) const SDummyProperty name = {str, v1, v2, {w0, w1, w2, w3, w4}, t1, t2};
DEFINE_PROPERTIES
#undef FUNCX
#define FUNCX(name, str, v1, v2, w0, w1, w2, w3, w4, t1, t2) name,
const std::vector<SDummyProperty> allProps_dummy = {DEFINE_PROPERTIES};
#undef FUNCX
#undef DEFINE_PROPERTIES
// ----------------------------------------------------------------------------------------------

enum
{ /* Mujoco dummy double params */
    simi_mujoco_dummy_range1 = 0,
    simi_mujoco_dummy_range2,
    simi_mujoco_dummy_solreflimit1,
    simi_mujoco_dummy_solreflimit2,
    simi_mujoco_dummy_solimplimit1,
    simi_mujoco_dummy_solimplimit2,
    simi_mujoco_dummy_solimplimit3,
    simi_mujoco_dummy_solimplimit4,
    simi_mujoco_dummy_solimplimit5,
    simi_mujoco_dummy_margin,
    simi_mujoco_dummy_springlength,
    simi_mujoco_dummy_stiffness,
    simi_mujoco_dummy_damping,
    simi_mujoco_dummy_solrefoverlapconstr1,
    simi_mujoco_dummy_solrefoverlapconstr2,
    simi_mujoco_dummy_solimpoverlapconstr1,
    simi_mujoco_dummy_solimpoverlapconstr2,
    simi_mujoco_dummy_solimpoverlapconstr3,
    simi_mujoco_dummy_solimpoverlapconstr4,
    simi_mujoco_dummy_solimpoverlapconstr5,
    simi_mujoco_dummy_torquescaleoverlapconstr,
};

enum
{ /* Mujoco dummy int params */
    simi_mujoco_dummy_bitcoded = 0,
    simi_mujoco_dummy_proxyjointid,
};

enum
{ /* Mujoco dummy bool params */
    simi_mujoco_dummy_limited = 1,
};

class CDummy : public CSceneObject
{
  public:
    CDummy();
    virtual ~CDummy();

    void buildOrUpdate_oldIk() override;
    void connect_oldIk() override;

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
    std::string getObjectTypeInfo() const override;
    std::string getObjectTypeInfoExtended() const override;
    bool isPotentiallyCollidable() const override;
    bool isPotentiallyMeasurable() const override;
    bool isPotentiallyDetectable() const override;
    void announceObjectWillBeErased(const CSceneObject* object, bool copyBuffer) override;
    void announceIkObjectWillBeErased(int ikGroupID, bool copyBuffer) override;
    void performObjectLoadingMapping(const std::map<int, int>* map, int opType) override;
    void setIsInScene(bool s) override;

    int setBoolProperty(const char* pName, bool pState, CCbor* eev = nullptr);
    int getBoolProperty(const char* pName, bool& pState) const override;
    int setIntProperty(const char* pName, int pState, CCbor* eev = nullptr);
    int getIntProperty(const char* pName, int& pState) const override;
    int setHandleProperty(const char* pName, long long int pState, CCbor* eev = nullptr);
    int getHandleProperty(const char* pName, long long int& pState) const override;
    int setFloatProperty(const char* pName, double pState, CCbor* eev = nullptr);
    int getFloatProperty(const char* pName, double& pState) const override;
    int setStringProperty(const char* pName, const char* pState) override;
    int getStringProperty(const char* pName, std::string& pState) const override;
    int setColorProperty(const char* pName, const float* pState) override;
    int getColorProperty(const char* pName, float* pState) const override;
    int setVector2Property(const char* pName, const double* pState, CCbor* eev = nullptr);
    int getVector2Property(const char* pName, double* pState) const override;
    int setFloatArrayProperty(const char* pName, const double* v, int vL, CCbor* eev = nullptr);
    int getFloatArrayProperty(const char* pName, std::vector<double>& pState) const override;
    int getPropertyName(int& index, std::string& pName, std::string& appartenance) const override;
    static int getPropertyName_static(int& index, std::string& pName, std::string& appartenance);
    int getPropertyInfo(const char* pName, int& info, std::string& infoTxt) const override;
    static int getPropertyInfo_static(const char* pName, int& info, std::string& infoTxt);
    // Some helpers:
    bool getBoolPropertyValue(const char* pName) const;
    int getIntPropertyValue(const char* pName) const;
    double getFloatPropertyValue(const char* pName) const;

    bool getFreeOnPathTrajectory() const;
    double getVirtualDistanceOffsetOnPath() const;
    double getVirtualDistanceOffsetOnPath_variationWhenCopy() const;
    std::string getLinkedDummyLoadAlias() const;
    std::string getLinkedDummyLoadName_old() const;
    double getDummySize() const;
    bool getAssignedToParentPath() const;
    bool getAssignedToParentPathOrientation() const;
    int getLinkedDummyHandle() const;
    int getDummyType() const;
    std::string getAssemblyTag() const;

    CColorObject* getDummyColor();
    void loadUnknownObjectType(CSer& ar);

    bool setAssignedToParentPath(bool assigned);
    bool setAssignedToParentPathOrientation(bool assigned);
    void setLinkedDummyHandle(int handle, bool check);
    bool setDummyType(int lt, bool check);
    void setDummySize(double s);
    void setAssemblyTag(const char* tag);
    void setFreeOnPathTrajectory(bool isFree);
    void setVirtualDistanceOffsetOnPath(double off);
    void setVirtualDistanceOffsetOnPath_variationWhenCopy(double off);

    double getEngineFloatParam_old(int what, bool* ok) const;
    int getEngineIntParam_old(int what, bool* ok) const;
    bool getEngineBoolParam_old(int what, bool* ok) const;
    bool setEngineFloatParam_old(int what, double v);
    bool setEngineIntParam_old(int what, int v);
    bool setEngineBoolParam_old(int what, bool v);

    void copyEnginePropertiesTo(CDummy* target);

  protected:
    void _sendEngineString(CCbor* eev = nullptr);
    std::string _enumToProperty(int oldEnum, int type, int& indexWithArrays) const;

    void _reflectPropToLinkedDummy() const;
    void _setLinkedDummyHandle_sendOldIk(int h) const;
    void _setLinkType_sendOldIk(int t) const;

    bool _freeOnPathTrajectory;
    double _virtualDistanceOffsetOnPath_OLD;
    double _virtualDistanceOffsetOnPath_variationWhenCopy_OLD;
    std::string _linkedDummyLoadAlias;
    std::string _linkedDummyLoadName_old;

    CColorObject _dummyColor;
    double _dummySize;
    int _linkedDummyHandle;
    int _linkType;
    std::string _assemblyTag;
    bool _assignedToParentPath;
    bool _assignedToParentPathOrientation;

    std::vector<double> _mujocoFloatParams;
    std::vector<int> _mujocoIntParams;

#ifdef SIM_WITH_GUI
  public:
    void display(CViewableBase* renderingObject, int displayAttrib) override;
#endif
};
